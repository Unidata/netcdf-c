/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "znc4.h"
#include "zfill.h"
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
#include "zfilter.h"
#include "netcdf_filter_build.h"
#endif

/**************************************************/

/* Mnemonics */
#define NOENDIAN -1

#define STRTEMPLATE "r%zu"

/**************************************************/

/* Table for converting nc_type to dtype+alias */
static const struct ZV3NCTYPE2DTYPE {
    const char* zarr; /* Must be a legitimate Zarr V3 type */
    int typelen; /* use int so we can use -1 as undefined flag; converted to size_t later */
    const char* type_alias;
} zv3nctype2dtype[N_NCZARR_TYPES] = {
/* nc_type       Pure Zarr   typelen    NCZarr */
/*NC_NAT*/      {NULL,          0,      NULL},
/*NC_BYTE*/     {"int8",        1,      NULL},
/*NC_CHAR*/     {"uint8",       1,      "char"},
/*NC_SHORT*/    {"int16",       2,      NULL},
/*NC_INT*/      {"int32",       4,      NULL},
/*NC_FLOAT*/    {"float32",     4,      NULL},
/*NC_DOUBLE*/   {"float64",     8,      NULL},
/*NC_UBYTE*/    {"uint8",       1,      NULL},
/*NC_USHORT*/   {"uint16",      2,      NULL},
/*NC_UINT*/     {"uint32",      4,      NULL},
/*NC_INT64*/    {"int64",       8,      NULL},
/*NC_UINT64*/   {"uint64",      8,      NULL},
/*NC_STRING*/   {STRTEMPLATE,	0,      "string"},
/*NC_JSON*/     {"json",	0,      "json"} /* NCZarr internal type */
};

/* Table for converting dtype to nc_type */
static const struct ZV3DTYPE2NCTYPE {
    const char* dtype;  /* Must be a legitimate Zarr V3 type */
    const char* dalias; /* attribute_types alias || NULL */
    size_t typelen;
} zv3dtype2nctype[N_NCZARR_TYPES] = {
/* nc_type       dtype   typelen  */
/*NC_NAT*/      {NULL,          NULL,	    0},
/*NC_BYTE*/     {"int8",        NULL,	    1},
/*NC_CHAR*/     {"uint8 ",	"char",	    1},
/*NC_SHORT*/    {"int16",       NULL,	    2},
/*NC_INT*/      {"int32",       NULL,	    4},
/*NC_FLOAT*/    {"float32",     NULL,	    4},
/*NC_DOUBLE*/   {"float64",     NULL,	    8},
/*NC_UBYTE*/    {"uint8",       NULL,	    1},
/*NC_USHORT*/   {"uint16",      NULL,	    2},
/*NC_UINT*/     {"uint32",      NULL,	    4},
/*NC_INT64*/    {"int64",       NULL,	    8},
/*NC_UINT64*/   {"uint64",      NULL,	    8},
/*NC_STRING*/   {STRTEMPLATE,	"string",   0},
/*NC_JSON*/     {"json",	"json",	    1}
};

/**************************************************/
/* Capture arguments for ncz4_create_var */
struct CVARGS {
	const char* varname;
	nc_type vtype;
	int storage;
	int endianness;
	size_t maxstrlen;
	char dimension_separator;
	char order;
	size_t rank;
	size64_t shape[NC_MAX_VAR_DIMS];
	size64_t chunks[NC_MAX_VAR_DIMS];
	int dimids[NC_MAX_VAR_DIMS];
	NClist* filterlist;
	int no_fill;
	void* fill_value;
};

/**************************************************/
/* Big endian Bytes filter */
static const char* NCZ_Bytes_Big_Text = "{\"name\": \"bytes\", \"configuration\": {\"endian\": \"big\"}}";
NCjson* NCZ_Bytes_Big_Json = NULL;

/* Little endian Bytes filter */
static const char* NCZ_Bytes_Little_Text = "{\"name\": \"bytes\", \"configuration\": {\"endian\": \"little\"}}";
NCjson* NCZ_Bytes_Little_Json = NULL;

/**************************************************/

/*Forward*/
static int ZF3_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
static int ZF3_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
static int ZF3_close(NC_FILE_INFO_T* file);
static int ZF3_writemeta(NC_FILE_INFO_T* file);
static int ZF3_readmeta(NC_FILE_INFO_T* file);
static int ZF3_buildchunkkey(size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);
static int ZF3_hdf2codec(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter);

static int write_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
static int write_var_meta(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_VAR_INFO_T* var);
static int write_var(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_VAR_INFO_T* var);

static int build_atts(NC_FILE_INFO_T* file, NC_OBJ* container, NCindex* attlist, NCjson** jattsp, NCjson** jnczattsp);
static int build_superblock(NC_FILE_INFO_T* file, NCjson** jsuperp);

static int read_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, NC_GRP_INFO_T** grpp);
static int read_grp_contents(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
static int read_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames);
static int read_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* subgrpnames);
static int verify_superblock(NC_FILE_INFO_T* file, const NCjson* jsuper);
static int parse_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jdims);
static int parse_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts, const NCjson* jatypes);
static int NCZ_collect_grps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCjson** jgrpsp);
static int NCZ_collect_arrays(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCjson** jarraysp);
static int NCZ_collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCjson** jdimsp);

static int subobjects_pure(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* varnames, NClist* grpnames);
static int subobjects(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* parent, const NCjson* jnczgrp, NClist* varnames, NClist* grpnames);
static NCjson* build_attr_type_dict(NCjson** dictp, const char* aname, const char* dtype);
static NCjson* build_named_config(const char* name, int pairs, ...);
static int getnextlevel(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* parent, NClist* varnames, NClist* subgrpnames);
static int nctype2dtype(nc_type nctype, int purezarr, size_t len, char** dnamep, char** tagp);
static int dtype2nctype(const char* dtype, const char* dalias, nc_type* nctypep, int* typelenp);
static int parse_rnnn(const char* dtype, size_t* typelenp);
static int jtypes2atypes(int purezarr, const NCjson* jattrs, const NCjson* jtypes, nc_type** atypesp);
static int infer_maxstrlen(const NCjson* jadata, int* typelenp);
static int NCZ_computedimrefs(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, size_t ndims, size64_t* shape, NCZ_DimInfo* diminfo);
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
static int json2filter(NC_FILE_INFO_T* file, const NCjson* jfilter, NCZ_Filter** zfilterp, NClist* filterlist);
#endif

/**************************************************/

/**
 * @internal Synchronize file metadata from internal to map.
 *
 * @param file Pointer to file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ZF3_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    ZTRACE(4,"file=%s",file->controller->path);
    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    return ZUNTRACE(THROW(stat));
}

static int
ZF3_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    ZTRACE(4,"file=%s",file->controller->path);
    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    return ZUNTRACE(THROW(stat));
}

/**************************************************/
/* Internal->Map */

/**
 * @internal Synchronize file metadata from internal => map.
 * Disabled for V1.
 *
 * @param file Pointer to file info struct.
 *
 * @return ::NC_NOERR No error
 *	   ::NC_EXXX errors
 * @author Dennis Heimbigner
 */
static int
ZF3_writemeta(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;

    ZTRACE(4,"file=%s",file->controller->path);

    /* Create the group tree recursively */
    if((stat = write_grp(file, file->root_grp)))
	goto done;

done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Recursively synchronize group from memory to map.
 *
 * @param file Pointer to file struct
 * @param zfile
 * @param map
 * @param grp Pointer to grp struct
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
write_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZMAP* map = zfile->map;
    int purezarr = 0;
    int rootgrp = 0;
    int needzarrjson = 0;
    char* fullpath = NULL;
    char* key = NULL;
    NCjson* jzarrjson = NULL; /* zarr.json */
    NCjson* jsuper = NULL;    /* _nczarr_superblock */
    NCjson* jatts = NULL;     /* group attributes */
    NCjson* jtypes = NULL;    /* group attribute types */
    NCjson* jdims = NULL;     /* group dimensions */
    NCjson* jtype = NULL;
    NCjson* jtmp = NULL;
    NCjson* jnczgrp = NULL;
    NCjson* jarrays = NULL;
    NCjson* jsubgrps = NULL;

    ZTRACE(3,"file=%s grp=%s isclose=%d",file->controller->path,grp->hdr.name,isclose);

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;
    rootgrp = (grp->parent == NULL);

    /* Do we need zarr.json for this group? */
    if(purezarr && ncindexsize(grp->att) > 0) needzarrjson = 1;
    else if(!purezarr && rootgrp) needzarrjson = 1;
    else if(ncindexsize(grp->att) > 0) needzarrjson = 1;
    else if(!purezarr && ncindexsize(grp->vars) > 0) needzarrjson = 1;
    else if(!purezarr && ncindexsize(grp->children) > 0) needzarrjson = 1;
    else needzarrjson = 0;

    if(needzarrjson) {
        /* Construct grp key */
        if((stat = NCZ_grpkey(grp,&fullpath))) goto done;

        NCJcheck(NCJnew(NCJ_DICT,&jzarrjson)); /* zarr.json */
        NCJcheck(NCJinsertstring(jzarrjson,"node_type","group"));
        NCJcheck(NCJinsertint(jzarrjson,"zarr_format",zfile->zarr.zarr_format));
	/* Disable must_understand */
	NCJinsertstring(jzarrjson,"must_understand","false");
    
        if(ncindexsize(grp->att) > 0) {
            /* Insert the group attributes */
   	    assert(grp->att);
	    if((stat = build_atts(file,(NC_OBJ*)grp, grp->att, &jatts, &jtypes))) goto done;
	}
	/* Add optional special attribute: _nczarr_attrs */
	if(!purezarr) {
	    jtype = build_attr_type_dict(&jtypes,NCZ_V3_ATTR,"json");
	    jtype = NULL;
	}

	/* Add optional special attribute: _nczarr_group */
	if(!purezarr) {
	    NCJcheck(NCJnew(NCJ_DICT,&jnczgrp));
	    /* Collect the dimensions in this group */
	    if((stat = NCZ_collect_dims(file, grp, &jdims))) goto done;
	    NCJcheck(NCJinsert(jnczgrp,"dimensions",jdims));
	    jdims = NULL;
	    /* Collect the arrays in this group */
	    if((stat = NCZ_collect_arrays(file, grp, &jarrays))) goto done;
	    NCJcheck(NCJinsert(jnczgrp,"arrays",jarrays));
	    jarrays = NULL;
	    /* Collect the subgrps in this group */
	    if((stat = NCZ_collect_grps(file, grp, &jsubgrps))) goto done;
	    NCJcheck(NCJinsert(jnczgrp,"subgroups",jsubgrps));
	    jsubgrps = NULL;
	    /* build corresponding type */
	    jtype = build_attr_type_dict(&jtypes,NCZ_V3_GROUP,"json");
	    jtype = NULL;
	    /* Insert _nczarr_group into "attributes" */
	    if(jatts == NULL) NCJcheck(NCJnew(NCJ_DICT,&jatts));
	    NCJcheck(NCJinsert(jatts,NCZ_V3_GROUP,jnczgrp));
	    jnczgrp = NULL;
	}

	/* Add optional special attribute: _nczarr_superblock */
	if(!purezarr && rootgrp) {
	    /* build superblock */
	    if((stat = build_superblock(file,&jsuper))) goto done;
	    jtype = build_attr_type_dict(&jtypes,NCZ_V3_SUPERBLOCK,"json");
	    jtype = NULL;
	    if(jatts == NULL) NCJcheck(NCJnew(NCJ_DICT,&jatts));
	    NCJcheck(NCJinsert(jatts,NCZ_V3_SUPERBLOCK,jsuper));
	    jsuper = NULL;
        }

	if(jtypes != NULL) {
	    assert(!purezarr);
	    if(jatts == NULL) NCJcheck(NCJnew(NCJ_DICT,&jatts));
	    /* Insert _nczarr_attrs into "attributes" */
	    NCJcheck(NCJnew(NCJ_DICT,&jtmp));
	    NCJcheck(NCJinsert(jtmp,"attribute_types",jtypes));	    
	    jtypes = NULL;
	    NCJcheck(NCJinsert(jatts,NCZ_V3_ATTR,jtmp));
	    jtmp = NULL;
	}	

	if(jatts != NULL) {
	    NCJcheck(NCJinsert(jzarrjson,"attributes",jatts));
	    jatts = NULL;
	}

	/* build Z3GROUP path */
	if((stat = nczm_concat(fullpath,Z3GROUP,&key))) goto done;
	/* Write to map */
	if((stat=NCZ_uploadjson(map,key,jzarrjson))) goto done;
	nullfree(key); key = NULL;
    }

    /* Now write all the variables */
    for(i=0; i<ncindexsize(grp->vars); i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	if((stat = write_var(file,zfile,map,var))) goto done;
    }

    /* Now recurse to synchronize all the subgrps */
    for(i=0; i<ncindexsize(grp->children); i++) {
	NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = write_grp(file,g))) goto done;
    }

done:
    nullfree(key);
    nullfree(fullpath);
    NCJreclaim(jsuper);
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    NCJreclaim(jdims);
    NCJreclaim(jtype);
    NCJreclaim(jtmp);
    NCJreclaim(jzarrjson);
    NCJreclaim(jnczgrp);
    NCJreclaim(jarrays);
    NCJreclaim(jsubgrps);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Synchronize variable meta data from memory to map.
 *
 * @param file Pointer to file struct
 * @param var Pointer to var struct
 * @param isclose If this called as part of nc_close() as opposed to nc_enddef().
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
write_var_meta(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    size_t i;
    char tmpstr[1024];
    char* fullpath = NULL;
    char* key = NULL;
    char* dimpath = NULL;
    NClist* dimrefs = NULL;
    NCjson* jvar = NULL;
    NCjson* jncvar = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jdimnames = NULL;
    NCjson* jtmp = NULL;
    NCjson* jtmp3 = NULL;
    NCjson* jfill = NULL;
    NCjson* jcodecs = NULL;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jtype = NULL;
    char* dtypename = NULL;
    char* dtypehint = NULL;
    int purezarr = 0;
    size64_t shape[NC_MAX_VAR_DIMS];
    NCZ_VAR_INFO_T* zvar = var->format_var_info;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    NClist* filterchain = NULL;
    NCjson* jfilter = NULL;
#endif

    ZTRACE(3,"file=%s var=%s isclose=%d",file->controller->path,var->hdr.name,isclose);

    zfile = file->format_file_info;
    map = zfile->map;

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    /* Make sure that everything is established */
    /* ensure the fill value */
    if((stat = NCZ_ensure_fill_value(var))) goto done; /* ensure var->fill_value is set */
    assert(var->no_fill || var->fill_value != NULL);
    /* ensure the chunk cache */
    if((stat = NCZ_adjust_var_cache(var))) goto done;
    /* rebuild the fill chunk */
    if((stat = NCZ_ensure_fill_chunk(zvar->cache))) goto done;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Build the filter working parameters for any filters */
    if((stat = NCZ_filter_setup(var))) goto done;
#endif

    /* Construct var path */
    if((stat = NCZ_varkey(var,&fullpath)))
	goto done;

    /* Create the variable's metadata json dict */
    NCJcheck(NCJnew(NCJ_DICT,&jvar));

    /* build Z3ARRAY contents */
    NCJcheck(NCJinsertstring(jvar,"node_type","array"));

    if(!purezarr) {
	/* Disable must_understand */
	NCJinsertstring(jvar,"must_understand","false");
    }

    /* zarr_format key */
    NCJcheck(NCJinsertint(jvar,"zarr_format",zfile->zarr.zarr_format));

    /* Shape: Integer list defining the length of each dimension of the array.*/
    for(i=0;i<var->ndims;i++) {
	NC_DIM_INFO_T* dim = var->dim[i];
	shape[i] = dim->len;
    }

    /* shape key */
    /* Integer list defining the length of each dimension of the array.*/
    /* Create the list */
    NCJcheck(NCJnew(NCJ_ARRAY,&jtmp));
    for(i=0;i<var->ndims;i++) {
	snprintf(tmpstr,sizeof(tmpstr),"%llu",shape[i]);
	NCJaddstring(jtmp,NCJ_INT,tmpstr);
    }
    NCJcheck(NCJinsert(jvar,"shape",jtmp));
    jtmp = NULL;

    /* data_type key */
    /* A string defining a valid data type for the array. */
    {	/* compute the type name */
	int atomictype = var->type_info->hdr.id;
	assert(atomictype > 0 && atomictype <= NC_MAX_ATOMIC_TYPE);
	if((stat = nctype2dtype(atomictype,purezarr,NCZ_get_maxstrlen((NC_OBJ*)var),&dtypename,&dtypehint))) goto done;
	NCJinsertstring(jvar,"data_type",dtypename);
	nullfree(dtypename); dtypename = NULL;
    }

    /* chunk_grid key {"name": "regular", "configuration": {"chunk_shape": [n1, n2, ...]}}  */
	/* The zarr format does not support the concept
	   of contiguous (or compact), so it will never appear in the read case.
	*/
    /* create the chunk sizes list */
    NCJcheck(NCJnew(NCJ_ARRAY,&jtmp3));
    if(zvar->scalar) {
	NCJaddstring(jtmp3,NCJ_INT,"1"); /* one chunk of size 1 */
    } else for(i=0;i<var->ndims;i++) {
	size64_t len = var->chunksizes[i];
	snprintf(tmpstr,sizeof(tmpstr),"%lld",len);
	NCJaddstring(jtmp3,NCJ_INT,tmpstr);
    }
    /* chunk_shape configuration dict */
    jtmp = build_named_config("regular",1,"chunk_shape",jtmp3);
    jtmp3 = NULL;
    NCJcheck(NCJinsert(jvar,"chunk_grid",jtmp));
    jtmp = NULL;    

    /* chunk_key_encoding configuration key */
    tmpstr[0] = zvar->dimension_separator;
    tmpstr[1] = '\0';
    NCJcheck(NCJnewstring(NCJ_STRING,tmpstr,&jtmp));
    /* assemble chunk_key_encoding dict */
    jtmp = build_named_config("default",1,"separator",jtmp);
    NCJcheck(NCJinsert(jvar,"chunk_key_encoding",jtmp));
    jtmp = NULL;

    /* build "dimension_names" key */
    if(NCJnew(NCJ_ARRAY,&jdimnames)) {stat = NC_ENOTZARR; goto done;}
     for(i=0;i<var->ndims;i++) {
	NC_DIM_INFO_T* dim = var->dim[i];
	if(NCJaddstring(jdimnames,NCJ_STRING, dim->hdr.name)) {stat = NC_ENOTZARR; goto done;}
    }
    if(NCJinsert(jvar,"dimension_names",jdimnames)) {stat = NC_ENOTZARR; goto done;}
    jdimnames = NULL;

    /* Capture dimref names as FQNs; simultaneously collect the simple dim names */
    if(!purezarr && var->ndims > 0) {
	if((dimrefs = nclistnew())==NULL) {stat = NC_ENOMEM; goto done;}
	for(i=0;i<var->ndims;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    if((stat = NCZ_dimkey(dim,&dimpath))) goto done;
	    nclistpush(dimrefs,dimpath);
	    dimpath = NULL;
	}
    }

    /* Build the NCZ_V3_ARRAY dict entry */
    if(!purezarr) {
	/* Create the dimrefs json object */
	NCJnew(NCJ_ARRAY,&jdimrefs);
	for(i=0;i<nclistlength(dimrefs);i++) {
	    const char* dim = (const char*)nclistget(dimrefs,i);
	    NCJaddstring(jdimrefs,NCJ_STRING,dim);
	}

	/* create the "_nczarr_array" dict */
	NCJnew(NCJ_DICT,&jncvar);

	/* Insert dimrefs  */
	NCJinsert(jncvar,"dimension_references",jdimrefs);
	jdimrefs = NULL; /* Avoid memory problems */

	/* Insert "type_alias if necessary */
	if(dtypehint != NULL)
	    NCJinsertstring(jncvar,"type_alias",dtypehint);
    }

    /* fill_value key */
    if(var->no_fill) {
	NCJnew(NCJ_NULL,&jfill);
    } else {/*!var->no_fill*/
	int atomictype = var->type_info->hdr.id;
	if(var->fill_value == NULL) {
	     if((stat = NCZ_ensure_fill_value(var))) goto done;
	}
	/* Convert var->fill_value to a string */
	if((stat = NCZ_stringconvert(atomictype,1,var->fill_value,&jfill))) goto done;
	assert(jfill->sort != NCJ_ARRAY);
    }
    NCJcheck(NCJinsert(jvar,"fill_value",jfill));
    jfill = NULL;

    /* codecs key */
    /* A list of JSON objects providing codec configurations; note that this is never empty
       because endianness must always be included. */
    /* Add the endianness codec as first entry */
    
    /* jcodecs holds the array of filters, if any */
    NCJcheck(NCJnew(NCJ_ARRAY,&jcodecs));
    /* Insert the "bytes" codec as first (pseudo-)codec */
    {
	NCjson* bytescodec = NULL;
	int endianness = var->endianness;
	if(endianness == NC_ENDIAN_NATIVE)
	    endianness = (NC_isLittleEndian()?NC_ENDIAN_LITTLE:NC_ENDIAN_BIG);
	if(endianness == NC_ENDIAN_LITTLE) bytescodec = NCZ_Bytes_Little_Json;
	else {assert(endianness == NC_ENDIAN_BIG); bytescodec = NCZ_Bytes_Big_Json;}
	if(NCJclone(bytescodec,&jtmp) || jtmp == NULL) {stat = NC_ENOTZARR; goto done;}
	if(NCJappend(jcodecs,jtmp)) goto done;
	jtmp = NULL;
    }
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Get chain of filters for this variable */
    filterchain = (NClist*)var->filters;
    if(nclistlength(filterchain) > 0) {
	size_t k;
	for(k=0;k<nclistlength(filterchain);k++) {
	    NCZ_Filter* filter = (struct NCZ_Filter*)nclistget(filterchain,k);
	    /* encode up the filter as a string */
	    if((stat = NCZ_filter_jsonize(file,var,filter,&jfilter))) goto done;
	    NCJappend(jcodecs,jfilter);
	    jfilter = NULL;
	}
    }
#endif
    NCJcheck(NCJinsert(jvar,"codecs",jcodecs));
    jcodecs = NULL;

    /* Build the Array attributes
       Always invoke in order to handle special attributes
    */
    assert(var->att);
    if((stat = build_atts(file,(NC_OBJ*)var, var->att, &jatts, &jtypes))) goto done;

    if(!purezarr) {
	if(jncvar != NULL) {
            /*  _nczarr_array as a pseudo-attribute */
	    jtype = build_attr_type_dict(&jtypes,NCZ_V3_ARRAY,"json");
	    jtype = NULL;
	    if(jatts == NULL) NCJcheck(NCJnew(NCJ_DICT,&jatts));
	    NCJcheck(NCJinsert(jatts,NCZ_V3_ARRAY,jncvar));
	    jncvar = NULL;
	}

	if(!purezarr) {
            /*  _nczarr_attrs as a pseudo-attribute */
	    jtype = build_attr_type_dict(&jtypes,NCZ_V3_ATTR,"json");
	    jtype = NULL;
	}

	if(jtypes != NULL) {
	    if(jatts == NULL) NCJcheck(NCJnew(NCJ_DICT,&jatts));
	    /* Insert _nczarr_attrs into "attributes" */
	    NCJcheck(NCJnew(NCJ_DICT,&jtmp));
	    NCJcheck(NCJinsert(jtmp,"attribute_types",jtypes));
	    jtypes = NULL;
	    NCJcheck(NCJinsert(jatts,NCZ_V3_ATTR,jtmp));	    
	    jtmp = NULL;
	}	
    }

    if(jatts != NULL) {
	NCJcheck(NCJinsert(jvar,"attributes",jatts));
	jatts = NULL;
    }

    /* build zarr.json path */
    if((stat = nczm_concat(fullpath,Z3ARRAY,&key)))
	goto done;

    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jvar)))
	goto done;
    nullfree(key); key = NULL;
    NCJreclaim(jvar); jvar = NULL;

    var->created = 1;

done:
    nclistfreeall(dimrefs);
    nullfree(fullpath);
    nullfree(key);
    nullfree(dtypename);
    nullfree(dtypehint);
    nullfree(dimpath);
    NCJreclaim(jvar);
    NCJreclaim(jncvar);
    NCJreclaim(jdimrefs);
    NCJreclaim(jdimnames);
    NCJreclaim(jtmp);
    NCJreclaim(jtmp3);
    NCJreclaim(jfill);
    NCJreclaim(jcodecs);
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    NCJreclaim(jtype);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Synchronize variable meta data and data from memory to map.
 *
 * @param file Pointer to file struct
 * @param var Pointer to var struct
 * @param isclose If this called as part of nc_close() as opposed to nc_enddef().
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
write_var(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;

    ZTRACE(3,"file=%s var=%s isclose=%d",file->controller->path,var->hdr.name,isclose);

    if((stat = write_var_meta(file,zfile,map,var))) goto done;

done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Convert an objects attributes to a JSON dictionary.
 *
 * @param file
 * @param zfile
 * @param container
 * @param attlist
 * @param jattsp return dictionary in this
 * @param jtypesp array of attribute types
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
build_atts(NC_FILE_INFO_T* file, NC_OBJ* container, NCindex* attlist, NCjson** jattsp, NCjson** jtypesp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t i;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jtype = NULL;
    NCjson* jcfg = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jint = NULL;
    NCjson* jdata = NULL;
    int purezarr = 0;
    NC_VAR_INFO_T* var = NULL;
    char* dtype = NULL;
    char* dtypehint = NULL;

    ZTRACE(3,"file=%s container=%s |attlist|=%u",file->controller->path,container->name,(unsigned)ncindexsize(attlist));
    
    if(container->sort == NCVAR)
	var = (NC_VAR_INFO_T*)container;

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    /* Create the attribute dictionary */
    NCJcheck(NCJnew(NCJ_DICT,&jatts));

    if(ncindexsize(attlist) > 0) {
	/* Walk all the attributes convert to json and collect the dtype */
	for(i=0;i<ncindexsize(attlist);i++) {
	    NC_ATT_INFO_T* a = (NC_ATT_INFO_T*)ncindexith(attlist,i);
	    size_t typesize = 0;
	    nc_type internaltype = a->nc_typeid;

	    /* special cases */
	    if(container->sort == NCVAR && strcmp(a->hdr.name,NC_FillValue)==0) continue; /* Ignore _FillValue */

	    if(a->nc_typeid > NC_MAX_ATOMIC_TYPE)
		{stat = (THROW(NC_ENCZARR)); goto done;}
	    if(a->nc_typeid == NC_STRING)
		typesize = NCZ_get_maxstrlen(container);
	    else
		{if((stat = NC4_inq_atomic_type(a->nc_typeid,NULL,&typesize))) goto done;}

	    /* Track json representation*/
            if(internaltype == NC_CHAR && NCZ_iscomplexjsontext((size_t)a->len,(char*)a->data,&jdata)) {
		internaltype = NC_JSON; /* hack to remember this case */
		typesize = 0;
	    } else {
	        if((stat = NCZ_stringconvert(a->nc_typeid,(size_t)a->len,a->data,&jdata))) goto done;
 	    }
	    NCJinsert(jatts,a->hdr.name,jdata);
	    jdata = NULL;

	    if(!purezarr) {
		/* Collect the corresponding dtype */
		if((stat = nctype2dtype(internaltype,purezarr,typesize,&dtype,&dtypehint))) goto done;
		jtype = build_attr_type_dict(&jtypes,a->hdr.name,(dtypehint==NULL?dtype:dtypehint));
		nullfree(dtype); dtype = NULL;
		nullfree(dtypehint); dtypehint = NULL;
		jtype = NULL;
	    }
	}
    }

    /* Add other special attributes: Quantize */
    if(container->sort == NCVAR && var && var->quantize_mode > 0) {    
	char mode[64];
	const char* qname = NULL;
	snprintf(mode,sizeof(mode),"%d",var->nsd);
	NCJcheck(NCJnewstring(NCJ_INT,mode,&jint));
	/* Insert the quantize attribute */
	switch (var->quantize_mode) {
	case NC_QUANTIZE_BITGROOM:
	    qname = NC_QUANTIZE_BITGROOM_ATT_NAME;
	    break;
	case NC_QUANTIZE_GRANULARBR:
	    qname = NC_QUANTIZE_GRANULARBR_ATT_NAME;
	    break;
	case NC_QUANTIZE_BITROUND:
	    qname = NC_QUANTIZE_BITROUND_ATT_NAME;
	    break;
	default: break;
	}
	NCJcheck(NCJinsert(jatts,qname,jint));
	jint = NULL;
	if(!purezarr) {
	    jtype = build_attr_type_dict(&jtypes,qname,"int");
	    jtype = NULL;
	}
    }

    if(jattsp) {*jattsp = jatts; jatts = NULL;}
    if(jtypesp) {*jtypesp = jtypes; jtypes = NULL;}
done:
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    NCJreclaim(jtype);
    NCJreclaim(jcfg);
    NCJreclaim(jdimrefs);
    NCJreclaim(jint);
    NCJreclaim(jdata);
    return THROW(stat);
}

/**
The super block is a placeholder in case
combined metadata information if implemented in V3
*/

static int
build_superblock(NC_FILE_INFO_T* file, NCjson** jsuperp)
{
    int stat = NC_NOERR;
    NCjson* jsuper = NULL;
    NCjson* jgroups = NULL;
    NCjson* jgroup = NULL;    
    char version[1024];
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    NCJcheck(NCJnew(NCJ_DICT,&jsuper));
    NCJcheck(NCJnew(NCJ_DICT,&jgroups));

    /* Fill in superblock */

    /* Track the library version that wrote this */
    strncpy(version,NCZARR_PACKAGE_VERSION,sizeof(version));
    NCJcheck(NCJinsertstring(jsuper,"version",version));
    NCJcheck(NCJinsertint(jsuper,"format",zfile->zarr.nczarr_format));

    if(jsuperp) {*jsuperp = jsuper; jsuper = NULL;}

    NCJreclaim(jgroup);
    NCJreclaim(jgroups);
    NCJreclaim(jsuper);
    return THROW(stat);
}


/**************************************************/
/* Map->Internal */


/**
 * @internal Read file data from map to memory.
 *
 * @param file Pointer to file info struct.
 *
 * @return NC_NOERR If no error.
 *	   NC_EXXX for error returns
 *
 * @author Dennis Heimbigner
 */
static int
ZF3_readmeta(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;
    NC_GRP_INFO_T* root = NULL;
    NCZ_GRP_INFO_T* zroot = NULL;
    NCjson* jrootgrp = NULL;
    NClist* paths = NULL;
    char* rootkey = NULL;
    int purezarr = 0;

    ZTRACE(3,"file=%s",file->controller->path);
    
    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    root = file->root_grp;
    zroot = (NCZ_GRP_INFO_T*)root->format_grp_info;
    assert(zroot);

    /* Read the root group's metadata */
    if((stat = NCZ_downloadjson(zfile->map, Z3METAROOT, &jrootgrp))) goto done;
    if(jrootgrp == NULL) {/* not there */
	zfile->flags |= FLAG_PUREZARR;
	stat = NC_NOERR; /* reset */
	goto done;
    }

    /* get definitive flag */
    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    /* Check for superblock */
    if(!purezarr) {
        const NCjson* jsuper = NULL;
        const NCjson* jatts = NULL;
	/* Read attributes */
        NCJcheck(NCJdictget(jrootgrp,"attributes",&jatts));
	if(jatts == NULL) {stat = NC_ENCZARR; goto done;} /* must exist */
	/* read superblock attribute: _nczarr_superblock */
        NCJcheck(NCJdictget(jatts,NCZ_V3_SUPERBLOCK,&jsuper));
	if(jsuper == NULL) {stat = NC_ENCZARR; goto done;} /* must exist */
        if((stat = verify_superblock(file,jsuper))) goto done;
    }

    /* Now load the groups contents starting with root */
    assert(file->root_grp != NULL);
    if((stat = read_grp_contents(file,file->root_grp))) goto done;

done:
    NCJreclaim(jrootgrp);
    nullfree(rootkey);
    nclistfreeall(paths);
    return ZUNTRACE(THROW(stat));
}

/**
@internal Collect format specific attribute info and convert to standard form.
@param file - [in] the containing file (annotation)
@param container - [in] the containing object (var or grp)
@param ainfop - [out] the standardized attribute info.
@return NC_NOERR|NC_EXXX

@author Dennis Heimbigner
*/
static int
parse_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts, const NCjson* jatypes)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t i, natts;
    nc_type* atypes = NULL;
    int purezarr = 0;
    int rootgrp = 0;
    NC_GRP_INFO_T* grp = NULL; /* container or container->parent */
    NC_VAR_INFO_T* var = NULL;
    NCZ_VAR_INFO_T* zvar = NULL;

    ZTRACE(3,"file=%s container=%s",file->controller->path,container->name);

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    if(jatts == NULL) goto done;
    if(NCJsort(jatts) != NCJ_DICT) {stat = NC_ENOTZARR; goto done;}
    if(NCJdictlength(jatts) == 0) goto done;

    if(container->sort == NCGRP) {
	grp = (NC_GRP_INFO_T*)container;
    } else {
	var = (NC_VAR_INFO_T*)container;
	zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
	grp = var->container;
    }
    if(grp->parent == NULL) rootgrp = 1;

    /* warning: will change if Zarr V3 decides to separate the attributes */
    natts = NCJdictlength(jatts);

    /* Get _nczarr_attrs from jatts (may be null) */
    if(jatypes != NULL) {
	/* jatypes  should be a dict */
	if(NCJsort(jatypes) != NCJ_DICT) {stat = (THROW(NC_ENCZARR)); goto done;}
    }
    /* Extract "types; may not exist if purezarr or only hidden attributes are defined */
    if((stat = jtypes2atypes(purezarr, jatts, jatypes, &atypes))) goto done;
assert(natts == 0 || atypes != NULL);
    
    /* Fill in the attributes */
    {
        for(i=0;i<natts;i++) {
            NCjson* jkey = NULL;
            NCjson* jvalues = NULL;
            jkey = NCJdictkey(jatts,i);
	    nc_type typeid;
	    size_t len = 0;
	    void* data = NULL;

            assert(jkey != NULL && NCJisatomic(jkey));
            jvalues = NCJdictvalue(jatts,i);
   	    /*Special cases:*/
	    if(strcmp(NC_ATT_FILLVALUE,NCJstring(jkey))==0) continue; /* _FillValue: ignore */
	    else if(strcmp(NCZARR_DEFAULT_MAXSTRLEN_ATTR,NCJstring(jkey))==0) {
	        if(rootgrp && NCJsort(jvalues)==NCJ_INT && atypes[i] == NC_INT)
		    sscanf(NCJstring(jvalues),"%zu",&zfile->default_maxstrlen);
		/* leave as attribute */
	    } else if(strcmp(NCZARR_MAXSTRLEN_ATTR,NCJstring(jkey))==0) {
	        if(var && NCJsort(jvalues)==NCJ_INT && atypes[i] == NC_INT)
		    sscanf(NCJstring(jvalues),"%zu",&zvar->maxstrlen);
		/* leave as an attribute */
    	    } else if(strncmp(NCJstring(jkey),NCZ_V3_PREFIX,strlen(NCZ_V3_PREFIX))==0)
		    continue; /* do not materialize any "_nczarr..." attributes except those preceding this test */
	    /* Convert jvalues to void* data */
	    typeid = atypes[i];
	    if((stat = NCZ_computeattrdata(&typeid, jvalues, NULL, &len, &data))) goto done;
	    /* Create the attribute */
	    if((stat = ncz4_create_attr(file,container,NCJstring(jkey),typeid,len,data,NULL))) goto done;
	    (void)NC_reclaim_data_all(file->controller,typeid,data,len);
	    if(stat) goto done;
	    data = NULL;
        }
    }

    /* remember that we read the attributes */
    NCZ_setatts_read(container);

done:
     nullfree(atypes);
     return ZUNTRACE(THROW(stat));
}

/* Convert an attribute types list to an nc_type list */
static int
jtypes2atypes(int purezarr, const NCjson* jattrs, const NCjson* jtypes, nc_type** atypesp)
{
    int stat = NC_NOERR;
    size_t i;
    nc_type* atypes = NULL;
    size_t nattrs = 0;
    size_t ntypes = 0;

    nattrs = NCJdictlength(jattrs);
    ntypes = NCJdictlength(jtypes);
    if(jtypes != NULL && ntypes != nattrs) {stat = NC_ENCZARR; goto done;} /* length mismatch */
    if((atypes = (nc_type*)calloc(nattrs,sizeof(nc_type)))==NULL) {stat = NC_ENOMEM; goto done;}
    for(i=0;i<nattrs;i++) {
        const NCjson* akey = NCJdictkey(jattrs,i);
        const NCjson* jdtype = NULL;
        if(NCJsort(akey) != NCJ_STRING) {stat = NC_ENOTZARR; goto done;}
        if(jtypes != NULL) NCJcheck(NCJdictget(jtypes,NCJstring(akey),&jdtype));
        if(jdtype == NULL) {
            const NCjson* avalues = NCJdictvalue(jattrs,i);
            /* Infer the type from the values */
            if((stat = NCZ_inferattrtype(avalues,NC_NAT,&atypes[i]))) goto done;
        } else {
	    int typelen = -1;
	    const char* dalias = NCJstring(jdtype);
            /* Get the nc_type; note that alias might be a legitimate type (e.g. int32), so use as dtype also */
            if((stat = dtype2nctype(NULL,dalias,&atypes[i],&typelen))) goto done;
	    if(typelen == -1) {
	        const NCjson* jadata = NCJdictvalue(jattrs,i);
		/* we know it is a string, but must infer the max strlen */
		if((stat = infer_maxstrlen(jadata,&typelen))) goto done;
	    }
        }
    }
    if(atypesp) {*atypesp = atypes; atypes = NULL;}
done:
    nullfree(atypes);
    return stat;
}

/**
Infer the maxstrlen for a string typed attribute
by looking at the string data
@param jadata attribute data
@param typelenp return inferred size
*/

static int
infer_maxstrlen(const NCjson* jadata, int* typelenp)
{
    int stat = NC_NOERR;
    size_t i = 0;
    size_t maxstrlen = 0;
    switch (NCJsort(jadata)) {
    case NCJ_STRING:
	maxstrlen = strlen(NCJstring(jadata));
	break;
    case NCJ_ARRAY:
	for(i=0;i<NCJarraylength(jadata);i++) {
	    size_t slen = 0;
	    NCjson* jstr = NCJith(jadata,i);
	    if(NCJsort(jstr) != NCJ_STRING) {stat = NC_ENCZARR; break;}
	    slen = strlen(NCJstring(jstr));
	    if(slen > maxstrlen) maxstrlen = slen; /* compute max str size */
	}
	break;
    default: stat = NC_ENCZARR; break;
    }
    if(typelenp) {*typelenp = (int)maxstrlen;}
    return THROW(stat);
}

static int
ZF3_buildchunkkey(size_t rank, const size64_t* chunkindices, char dimsep, char** keyp)
{
    int stat = NC_NOERR;
    size_t r;
    NCbytes* key = ncbytesnew();

    if(keyp) *keyp = NULL;

    assert(islegaldimsep(dimsep));
    
    ncbytescat(key,"c");
    ncbytesappend(key,dimsep);
    for(r=0;r<rank;r++) {
	char sindex[64];
	if(r > 0) ncbytesappend(key,dimsep);
	/* Print as decimal with no leading zeros */
	snprintf(sindex,sizeof(sindex),"%lu",(unsigned long)chunkindices[r]);	
	ncbytescat(key,sindex);
    }
    ncbytesnull(key);
    if(keyp) *keyp = ncbytesextract(key);

    ncbytesfree(key);
    return THROW(stat);
}

static int
ZF3_close(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    return THROW(stat);
}

/**************************************************/

static int
verify_superblock(NC_FILE_INFO_T* file, const NCjson* jsuper)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    const NCjson* jvalue = NULL;

    NCJcheck(NCJdictget(jsuper,"version",&jvalue));
    if(jvalue != NULL) {
	sscanf(NCJstring(jvalue),"%d.0.0",&zfile->zarr.nczarr_format);
	assert(zfile->zarr.nczarr_format == NCZARRFORMAT3);
    }
    NCJcheck(NCJdictget(jsuper,"format",&jvalue));
    if(jvalue != NULL) {
	sscanf(NCJstring(jvalue),"%d",&zfile->zarr.zarr_format);
	assert(zfile->zarr.zarr_format == ZARRFORMAT3);
    }

    return THROW(stat);
}
/**
 * @internal Read group data from map to memory
 *
 * @param file Pointer to file struct
 * @param parent Pointer to parent group
 * @param name of the group to create
 * @param grpp created group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, NC_GRP_INFO_T** grpp)
{
    int stat = NC_NOERR;
    NC_GRP_INFO_T* grp = NULL;

    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);
    
    /* Build group */
    if((stat = ncz4_create_grp(file,parent,name,&grp))) goto done;

    /* Fill in the group recursively */
    if((stat = read_grp_contents(file,grp))) goto done;

    if(grpp) {*grpp = grp; grp = NULL;}

done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal fill in group content from map to memory
 *
 * @param file Pointer to file struct
 * @param grp Pointer to grp struct
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_grp_contents(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZMAP* map = zfile->map;
    char* grppath = NULL;
    char* key = NULL;
    int purezarr = 0;
    NClist* subvars = nclistnew();
    NClist* subgrps = nclistnew();
    NCjson* jgroup = NULL;
    const NCjson* jatts = NULL;
    const NCjson* jnczgrp = NULL;
    const NCjson* jdims = NULL;
    const NCjson* jncatt = NULL;
    const NCjson* jtypes = NULL;

    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);
    
    purezarr = (zfile->flags & FLAG_PUREZARR);

    /* Download the grp meta-data; might not exist for virtual groups */

    /* build Z3GROUP path */
    /* Construct grp path */
    if((stat = NCZ_grpkey(grp,&grppath))) goto done;
    /* and the grp's zarr.json */
    if((stat = nczm_concat(grppath,Z3GROUP,&key))) goto done;

    /* Read zarr.json */
    stat=NCZ_downloadjson(map,key,&jgroup);
    nullfree(key); key = NULL;
    if(stat) goto done;
   
    /* Get attributes */
    NCJcheck(NCJdictget(jgroup,"attributes",&jatts));

    if(purezarr || jgroup == NULL) {
        /* Apparently this is a virtual group; treat like purezarr to find the vars and groups */
	if((stat = subobjects_pure(zfile,grp,subvars,subgrps))) goto done;
    } else {
	assert(!purezarr);
	/* Get _nczarr_group */
	NCJcheck(NCJdictget(jatts,NCZ_V3_GROUP,&jnczgrp));
	if(jnczgrp == NULL) {stat = NC_ENCZARR; goto done;}
        /* Define dimensions */
	NCJcheck(NCJdictget(jnczgrp,"dimensions",&jdims));
        if(jdims != NULL) {
	    if((stat = parse_dims(file,grp,jdims))) goto done;
        }
        /* Get lists of subvars and subgrps */
  	if((stat = subobjects(zfile,grp,jnczgrp,subvars,subgrps))) goto done;
    }

    if(!purezarr) {
	/* Extract the _nczarr_attr values */
	NCJcheck(NCJdictget(jatts,NCZ_V3_ATTR,&jncatt));
	if(jncatt == NULL) {stat = NC_ENCZARR; goto done;}
	assert((NCJsort(jncatt) == NCJ_DICT));
	/* Extract attr type list */
	NCJcheck(NCJdictget(jncatt,"attribute_types",&jtypes));
	if(jtypes == NULL) {stat = NC_ENCZARR; goto done;}
    }
    
    /* build the attributes */
    if((stat = parse_attrs(file,(NC_OBJ*)grp,jatts,jtypes))) goto done;

    /* Fill in the group recursively */

    /* Define vars */
    if((stat = read_vars(file,grp,subvars))) goto done;

    /* Define sub-groups */
    if((stat = read_subgrps(file,grp,subgrps))) goto done;

done:
    NCJreclaim(jgroup);
    nclistfreeall(subvars);
    nclistfreeall(subgrps);
    nullfree(grppath);
    nullfree(key);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Materialize dimensions into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param jdims "dimensions" key from _nczarr_group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
parse_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jdims)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    const NCjson* jdim = NULL;
    const NCjson* jname = NULL;	   
    const NCjson* jsize = NULL;
    const NCjson* junlim = NULL;
    const NCjson* jcfg = NULL;
    int purezarr = 0;
    
    ZTRACE(3,"file=%s grp=%s |diminfo|=%u",file->controller->path,grp->hdr.name,nclistlength(diminfo));

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    if(purezarr) goto done; /* Dims will be created as needed */

    assert(NCJsort(jdims)==NCJ_ARRAY);
    /* Reify each dim in turn */
    for(i = 0; i < NCJarraylength(jdims); i++) {
	NC_DIM_INFO_T* dim = NULL;
	size64_t dimlen = 0;
	int isunlim = 0;
	/* Extract info */
	jdim = NCJith(jdims,i);
	assert(NCJsort(jdim) == NCJ_DICT);
	NCJcheck(NCJdictget(jdim,"name",&jname));
	assert(jname != NULL);
	NCJcheck(NCJdictget(jdim,"configuration",&jcfg));
	assert(jcfg != NULL);
	NCJcheck(NCJdictget(jcfg,"size",&jsize));
	assert(jsize != NULL);
	NCJcheck(NCJdictget(jcfg,"unlimited",&junlim)); /* might be null */
	/* Create the NC_DIM_INFO_T object */
	{
	    const char* name = NCJstring(jname);
	    unsigned long data;
	    sscanf(NCJstring(jsize),"%lu",&data); /* Get length */
	    dimlen = (size64_t)data;
	    if(junlim != NULL) {
		sscanf(NCJstring(junlim),"%ld",&data); /* Get unlimited flag */
		isunlim = (data == 0 ? (int)0 : (int)1);
	    } else
		isunlim = 0;
	    if((stat = nc4_dim_list_add(grp, name, (size_t)dimlen, -1, &dim))) goto done;
	    dim->unlimited = isunlim;
	    if((dim->format_dim_info = calloc(1,sizeof(NCZ_DIM_INFO_T))) == NULL) {stat = NC_ENOMEM; goto done;}
	    ((NCZ_DIM_INFO_T*)dim->format_dim_info)->common.file = file;
	}
    }

done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Materialize a single var into memory;
 * Take xarray and purezarr into account.
 *
 * @param file Pointer to file info struct.
 * @param zfile Zarr specific file info
 * @param map Zarr storage map handler
 * @param grp Pointer to grp info struct.
 * @param var variable name
 * @param suppressp flat to suppress variable
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_var1(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const char* varname)
{
    int stat = NC_NOERR;
    size_t i,j;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NC_VAR_INFO_T* var = NULL;
    int purezarr = 0;
    int atypeid = NC_NAT;
    size_t fvlen;
    NCjson* jvar = NULL;
    /* per-variable info */
    const NCjson* jatts = NULL;
    const NCjson* jncvar = NULL;
    const NCjson* jncatt = NULL;
    const NCjson* jdimrefs = NULL;
    const NCjson* jtypes = NULL;
    const NCjson* jvalue = NULL;
    const NCjson* jtmp = NULL;
    const NCjson* jtmp2 = NULL;
    const NCjson* jsep = NULL;
    const NCjson* jcodecs = NULL;
    const NCjson* jcodec = NULL;
    const NCjson* jtalias = NULL;
    char* grppath = NULL;
    char* varpath = NULL;
    char* key = NULL;
    int suppress = 0; /* Abort processing of this variable */
    int vtypelen = 0;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    int varsized = 0;
    size_t k;
#endif
    NCZ_DimInfo diminfo[NC_MAX_VAR_DIMS];
    /* Capture arguments for ncz4_create_var */
    struct CVARGS cvargs;

    /* initialize cvargs defaults */
    memset(&cvargs,0,sizeof(cvargs));
    cvargs.varname = varname;
    cvargs.endianness = (NC_isLittleEndian()?NC_ENDIAN_LITTLE:NC_ENDIAN_BIG);
    cvargs.storage = NC_CHUNKED;
    cvargs.no_fill = 1;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    cvargs.filterlist = nclistnew();
#endif
    memset(diminfo,0,sizeof(diminfo));

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;
 
    /* Construct parent path */
    if((stat = NCZ_grpkey(grp,&grppath))) goto done;
    /* Construct the path to the var */
    if((stat = nczm_concat(grppath,varname,&varpath))) goto done;
    /* Construct the path to the var zarr.json object */
    if((stat = nczm_concat(varpath,Z3ARRAY,&key))) goto done;

    /* Download the zarray object */
    if((stat=NCZ_readdict(zfile->map,key,&jvar))) goto done;
    nullfree(key); key = NULL;
    assert(NCJsort(jvar) == NCJ_DICT);

    /* Extract the metadata from jvar */

    /* Verify the format */
    {
	int version;
	NCJcheck(NCJdictget(jvar,"node_type",&jvalue));
	if(strcasecmp("array",NCJstring(jvalue))!=0) {stat = THROW(NC_ENOTZARR); goto done;}
	NCJcheck(NCJdictget(jvar,"zarr_format",&jvalue));
	sscanf(NCJstring(jvalue),"%d",&version);
	if(version != zfile->zarr.zarr_format)
	    {stat = (THROW(NC_ENCZARR)); goto done;}
    }

    /* shape */
    {
	NCJcheck(NCJdictget(jvar,"shape",&jvalue));
	if(NCJsort(jvalue) != NCJ_ARRAY) {stat = (THROW(NC_ENCZARR)); goto done;}
        cvargs.rank = NCJarraylength(jvalue);
	    
	if(cvargs.rank > 0) {
	    /* extract the shape */
	    if((stat = NCZ_decodesizet64vec(jvalue, cvargs.shape))) goto done;
	}
	/* Set storage flag */
	cvargs.storage = (cvargs.rank==0?NC_CONTIGUOUS:NC_CHUNKED);
    }
    
    /* Get dimension_names */
    {
        const NCjson* jdimnames = NULL;
	/* get the "dimension_names" */
	NCJcheck(NCJdictget(jvar,"dimension_names",&jdimnames));
	if(jdimnames != NULL) {
	    for(i=0;i<cvargs.rank;i++) {
	        const NCjson* dimpath = NCJith(jdimnames,i);
	        assert(NCJisatomic(dimpath));
	        diminfo[i].name = strdup(NCJstring(dimpath));
		diminfo[i].shape = cvargs.shape[i]; /* while we are at it */
		diminfo[i].unlimited = 0; /* until we know for certain */
	    }
	}
    }    

    /* Extract the attributes; might not exist */
    NCJcheck(NCJdictget(jvar,"attributes",&jatts));

    if(!purezarr) {
	if(jatts == NULL) {stat = NC_ENCZARR; goto done;} /*must exist*/
	/* Extract the _NCZARR_ARRAY values */
	NCJcheck(NCJdictget(jatts,NCZ_V3_ARRAY,&jncvar));
	if(jncvar == NULL) {stat = NC_ENCZARR; goto done;}
	assert((NCJsort(jncvar) == NCJ_DICT));
	/* Extract the _NCZARR_ATTR values */
	NCJcheck(NCJdictget(jatts,NCZ_V3_ATTR,&jncatt));
	if(jncatt == NULL) {stat = NC_ENCZARR; goto done;}
	assert((NCJsort(jncatt) == NCJ_DICT));
	/* Extract attr type list */
	NCJcheck(NCJdictget(jncatt,"attribute_types",&jtypes));
	if(jtypes == NULL) {stat = NC_ENCZARR; goto done;}
	/* Extract dimfqns list	 */
	NCJcheck(NCJdictget(jncvar,"dimension_references",&jdimrefs));
	if(jdimrefs != NULL) {
	    /* Extract the dimref names */
	    assert((NCJsort(jdimrefs) == NCJ_ARRAY));
            for(j=0;j<cvargs.rank;j++) {
		const NCjson* dimpath = NCJith(jdimrefs,j);
		assert(NCJisatomic(dimpath));
		assert(diminfo[j].fqn == NULL);
		diminfo[j].fqn = strdup(NCJstring(dimpath));
	    }
	}
	/* Extract the type_alias hint for use instead of the variables' type */
	NCJcheck(NCJdictget(jncvar,"type_alias",&jtalias));
    }

    /* Complete the diminfo construction */
    if((stat = NCZ_computedimrefs(file, grp, cvargs.rank, cvargs.shape, diminfo))) goto done;

    /* Get the type of the variable */
    {
	const char* dalias = (jtalias == NULL?NULL:NCJstring(jtalias));
	NCJcheck(NCJdictget(jvar,"data_type",&jvalue));
	/* Convert dtype to nc_type */
	if((stat = dtype2nctype(NCJstring(jvalue),dalias,&cvargs.vtype,&vtypelen))) goto done;
	assert(vtypelen >= 0);
	if(cvargs.vtype == NC_STRING) {
	    cvargs.maxstrlen = (size_t)vtypelen;
	    if(cvargs.maxstrlen <= 0) cvargs.maxstrlen = NCZ_get_maxstrlen((NC_OBJ*)var);
	}
    }

    /* Capture dimension_separator (must precede chunk cache creation) */
    {
	NCglobalstate* ngs = NC_getglobalstate();
	assert(ngs != NULL);
	cvargs.dimension_separator = '\0';
	NCJcheck(NCJdictget(jvar,"chunk_key_encoding",&jvalue));
	if(jvalue == NULL) {
	    /* If value is invalid, then use global default */
	    if(!islegaldimsep(cvargs.dimension_separator))
	    cvargs.dimension_separator = ngs->zarr.dimension_separator; /* use global value */
	    /* Verify its value */
	} else {
	    if(NCJsort(jvalue) != NCJ_DICT) {stat = NC_ENOTZARR; goto done;}
	    NCJcheck(NCJdictget(jvalue,"name",&jtmp));
	    if(strcasecmp("default",NCJstring(jtmp))==0) {
		NCJcheck(NCJdictget(jvalue,"configuration",&jtmp));
		if(jtmp != NULL) {
		    NCJcheck(NCJdictget(jtmp,"separator",&jsep));
		    if(jsep != NULL) {
			if(NCJisatomic(jsep) && NCJstring(jsep) != NULL && strlen(NCJstring(jsep)) == 1)
			    cvargs.dimension_separator = NCJstring(jsep)[0];
		    } else
			cvargs.dimension_separator = DFALT_DIM_SEPARATOR_V3;
		} else
		    cvargs.dimension_separator = DFALT_DIM_SEPARATOR_V3;
	    } else if(strcasecmp("v2",NCJstring(jtmp))==0) {
		NCJcheck(NCJdictget(jvalue,"separator",&jsep));
		if(jsep != NULL) {
		    if(NCJsort(jsep) == NCJ_STRING && NCJstring(jsep) != NULL && strlen(NCJstring(jsep)) == 1)
			cvargs.dimension_separator = NCJstring(jsep)[0];
		    else {stat = NC_ENOTZARR; goto done;}
		} else
		    cvargs.dimension_separator = '.';
	    } else {stat = NC_ENOTZARR; goto done;}
	 }
	assert(islegaldimsep(cvargs.dimension_separator)); /* we are hosed */
    }

    /* chunks */
    {
	if(cvargs.rank > 0) {
	    NCJcheck(NCJdictget(jvar,"chunk_grid",&jvalue));
	    if(jvalue == NULL) {stat = (THROW(NC_ENOTBUILT)); goto done;}
	    NCJcheck(NCJdictget(jvalue,"name",&jtmp));
	    if(jtmp == NULL) {stat = (THROW(NC_ENOTZARR)); goto done;}
	    if(strcasecmp("regular",NCJstring(jtmp))!=0) {stat = THROW(NC_ENOTZARR); goto done;}
	    NCJcheck(NCJdictget(jvalue,"configuration",&jtmp));
	    if(jtmp == NULL) {stat = (THROW(NC_ENOTZARR)); goto done;}
	    NCJcheck(NCJdictget(jtmp,"chunk_shape",&jtmp2));
	    if(jtmp2 == NULL) {stat = (THROW(NC_ENOTZARR)); goto done;}
	    if(NCJsort(jtmp2) != NCJ_ARRAY) {stat = (THROW(NC_ENOTZARR)); goto done;}

	    if(cvargs.rank != NCJarraylength(jtmp2)) {stat = (THROW(NC_ENCZARR)); goto done;}
    
	    /* Get the chunks and chunkproduct */
	    if((stat = NCZ_decodesizet64vec(jtmp2, cvargs.chunks))) goto done;
	}
    }
	
    /* codecs key */
    /* From V3 Spec: A list of JSON objects providing codec configurations,
       or null if no filters are to be applied. Each codec configuration
       object MUST contain a "name" and a "configuration" key identifying the codec to be used.
       Note that the first codec must be the "bytes" codec and it specifies variable endianness */
    {
	NCJcheck(NCJdictget(jvar,"codecs",&jcodecs));
	assert(jcodecs != NULL);
	/* Process the first codec which must be bytes, to get the endianness */
	jcodec = NCJith(jcodecs,0);
	if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_ENOTZARR; goto done;}	    
	NCJcheck(NCJdictget(jcodec,"name",&jvalue));
	if(jvalue == NULL || strcmp(ZBYTES3,NCJstring(jvalue))!=0) {stat = NC_ENCZARR; goto done;}	      
	NCJcheck(NCJdictget(jcodec,"configuration",&jvalue));
	if(jvalue == NULL || NCJsort(jvalue) != NCJ_DICT) {stat = NC_ENCZARR; goto done;}	     
	NCJcheck(NCJdictget(jvalue,"endian",&jtmp));
	if(strcasecmp("big",NCJstring(jtmp))==0)
	    cvargs.endianness = NC_ENDIAN_BIG;
	else if(strcasecmp("little",NCJstring(jtmp))==0)
	    cvargs.endianness = NC_ENDIAN_LITTLE;
	else {stat = NC_EINVAL; goto done;}

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
	if((stat = NCZ_filter_initialize())) goto done;
	NCJcheck(NCJdictget(jvar,"codecs",&jcodecs));
	if(jcodecs == NULL || NCJsort(jcodecs) == NCJ_NULL)
	    {stat = NC_ENOTZARR; goto done;}
	if(NCJsort(jcodecs) != NCJ_ARRAY) {stat = NC_ENOTZARR; goto done;}
	if(NCJarraylength(jcodecs) == 0) {stat = NC_ENOTZARR; goto done;}
	 /* Process remaining filters */
	for(k=1;;k++) {
	    NCZ_Filter* filter = NULL;
	    jcodec = NULL;
	    jcodec = NCJith(jcodecs,k);
	    if(jcodec == NULL) break; /* done */
	    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EFILTER; goto done;}
	    if((stat = json2filter(file,jcodec,&filter,cvargs.filterlist))) goto done;
	}

	/* Suppress variable if there are filters and var is not fixed-size */
	if(varsized && nclistlength(cvargs.filterlist) > 0) suppress = 1;

#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
    }

    /* fill_value; must precede calls to adjust cache */
    {
	NCJcheck(NCJdictget(jvar,"fill_value",&jvalue));
	if(jvalue == NULL || NCJsort(jvalue) == NCJ_NULL)
	    cvargs.no_fill = 1;
	else {
	    atypeid = cvargs.vtype;
	    if((stat = NCZ_computeattrdata(&atypeid, jvalue, NULL, &fvlen, &cvargs.fill_value))) goto done;
	    assert(atypeid == cvargs.vtype && fvlen == 1);
	    cvargs.no_fill = 0;
	}
    }

    {
	size_t i;
	/* Capture the dimids for the dimensions */
	for(i=0;i<cvargs.rank;i++) {
	    NC_OBJ* obj = NULL;
	    NCZ_DimInfo* dinfo = &diminfo[i];
	    assert(dinfo->name != NULL && dinfo->fqn != NULL);
	    if((stat = NCZ_locateFQN(file->root_grp,dinfo->fqn,NCDIM,&obj,NULL))) goto done;
	    if(obj == NULL) {stat = NC_EBADDIM; goto done;}
	    cvargs.dimids[i] = obj->id;
	}
    }

    if(!suppress) {
        /* Create the variable */
        if((stat = ncz4_create_var(file,grp,
					cvargs.varname,
					cvargs.vtype,
					cvargs.storage,
					(cvargs.rank==0?1:0),
					cvargs.endianness,
					cvargs.maxstrlen,
					cvargs.dimension_separator,
					cvargs.order,
					cvargs.rank,
					cvargs.shape,
					cvargs.chunks,
					cvargs.dimids,
					cvargs.filterlist,
					cvargs.no_fill,
					cvargs.fill_value,
					&var)))
	    goto done;

        if((stat = NCZ_adjust_var_cache(var))) goto done;

        /* Now that we have the variable, create its attributes */
        if((stat = parse_attrs(file,(NC_OBJ*)var,jatts,jtypes))) goto done;
    
	/* Push the fill value attribute */
	if((stat = NCZ_copy_var_to_fillatt(file,var,NULL))) goto done;
    }

    if(suppress) {
	/* Reclaim NCZarr variable specific info */
	(void)NCZ_zclose_var1(var);
	/* Remove from list of variables and reclaim the top level var object */
	(void)nc4_var_list_del(grp, var);
	var = NULL;
    }

done:
    /* Clean up cvargs */
    nclistfree(cvargs.filterlist);
    if(cvargs.fill_value != NULL)
	(void)NC_reclaim_data_all(file->controller,atypeid,cvargs.fill_value,fvlen);
    NCZ_clear_diminfo(cvargs.rank,diminfo);
    /* Clean up	 */
    nullfree(grppath);
    nullfree(varpath);
    nullfree(key);
    NCJreclaim(jvar);
    return THROW(stat);
}

/**
 * @internal Materialize vars into memory;
 * Take purezarr into account.
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param varlist list of var names in this group
 * @param varnames List of names of variables in this group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames)
{
    int stat = NC_NOERR;
    size_t i;

    ZTRACE(3,"file=%s grp=%s |varnames|=%u",file->controller->path,grp->hdr.name,nclistlength(varnames));

    if(nclistlength(varnames) == 0) goto done; /* Nothing to create */

    /* Reify each array in turn */
    for(i = 0; i < nclistlength(varnames); i++) {
	const char* varname = (const char*)nclistget(varnames,i);
	if((stat = read_var1(file,grp,varname))) goto done;
    }

done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Materialize subgroups into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param subgrpnames List of names of subgroups in this group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* subgrpnames)
{
    int stat = NC_NOERR;
    size_t i;
    
    ZTRACE(3,"file=%s parent=%s |subgrpnames|=%u",file->controller->path,parent->hdr.name,nclistlength(subgrpnames));

    /* Recurse to fill in subgroups */
    for(i=0;i<nclistlength(subgrpnames);i++) {
	NC_GRP_INFO_T* g = NULL;
	const char* gname = nclistget(subgrpnames,i);
        char norm_name[NC_MAX_NAME];
        /* Check and normalize the name. */
        if((stat = nc4_check_name(gname, norm_name)))
            goto done;
	if((stat = read_grp(file,parent,gname,&g))) goto done;
    }

done:
    return ZUNTRACE(THROW(stat));
}


/**************************************************/
/* Potentially shared functions */

/**
 * @internal JSONize dims in a group
 *
 * @param  file pointer to file struct
 * @param  parent group
 * @param jdimsp return the array of jsonized dimensions
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
NCZ_collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCjson** jdimsp)
{
    int stat=NC_NOERR;
    size_t i;
    NCjson* jdims = NULL;
    NCjson* jdim = NULL;
    NCjson* jdimname = NULL;
    NCjson* jdimsize = NULL;
    NCjson* jdimunlimited = NULL;
    NCjson* jcfg = NULL;
    char slen[64];

    ZTRACE(3,"file=%s parent=%s",file->controller->path,parent->hdr.name);

    NCJcheck(NCJnew(NCJ_ARRAY,&jdims));

    for(i=0; i<ncindexsize(parent->dim); i++) {
	NC_DIM_INFO_T* dim = (NC_DIM_INFO_T*)ncindexith(parent->dim,i);
	jdimsize = NULL;
	jdimname = NULL;
	jdimunlimited = NULL;
	jdim = NULL;
	jcfg = NULL;
	
	/* Collect the dimension info */

	/* jsonize dim name */
	NCJcheck(NCJnewstring(NCJ_STRING,dim->hdr.name,&jdimname));
	/* Jsonize the size */
	snprintf(slen,sizeof(slen),"%llu",(unsigned long long)dim->len);
	NCJcheck(NCJnewstring(NCJ_INT,slen,&jdimsize));
	/* And the unlimited flag*/
	if(dim->unlimited) {
	    NCJcheck(NCJnewstring(NCJ_INT,"1",&jdimunlimited));
	}

	/* Assemble the dimension dict */
	
	/* Create configuration dict */
	NCJcheck(NCJnew(NCJ_DICT,&jcfg));
	NCJcheck(NCJinsert(jcfg,"size",jdimsize));
	jdimsize = NULL;
	if(jdimunlimited != NULL) NCJinsert(jcfg,"unlimited",jdimunlimited);
	jdimunlimited = NULL;

	/* Create dim dict */
	NCJcheck(NCJnew(NCJ_DICT,&jdim));

	/* Add the dimension name*/
	NCJcheck(NCJinsert(jdim,"name",jdimname));
	jdimname = NULL;

	/* Add the configuration */
	NCJcheck(NCJinsert(jdim,"configuration",jcfg));
	jcfg = NULL;

	/* Add to jdims */
	NCJcheck(NCJappend(jdims,jdim));
	jdim = NULL;
    }

    if(jdimsp) {*jdimsp = jdims; jdims = NULL;}

    NCJreclaim(jdims);
    NCJreclaim(jdim);
    NCJreclaim(jdimsize);
    NCJreclaim(jdimname);
    NCJreclaim(jdimunlimited);
    NCJreclaim(jcfg);
    return ZUNTRACE(THROW(stat));
}


/**
 * @internal JSONize array name within a group
 *
 * @param  file pointer to file struct
 * @param  parent group
 * @param jarrayp return the array of jsonized arrays
 * @return ::NC_NOERR No error.
 *
 * @author Dennis Heimbigner
 */
static int
NCZ_collect_arrays(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCjson** jarraysp)
{
    int stat=NC_NOERR;
    size_t i;
    NCjson* jarrays = NULL;
    NCjson* jarrayname = NULL;

    ZTRACE(3,"file=%s parent=%s",file->controller->path,parent->hdr.name);

    NCJcheck(NCJnew(NCJ_ARRAY,&jarrays));

    for(i=0; i<ncindexsize(parent->vars); i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(parent->vars,i);
	jarrayname = NULL;
	
	/* jsonize array name */
	NCJcheck(NCJnewstring(NCJ_STRING,var->hdr.name,&jarrayname));

	/* Add the array name*/
	NCJcheck(NCJappend(jarrays,jarrayname));
	jarrayname = NULL;
    }
    if(jarraysp) {*jarraysp = jarrays; jarrays = NULL;}

    NCJreclaim(jarrays);
    NCJreclaim(jarrayname);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal JSONize subgrps in a group
 *
 * @param  file pointer to file struct
 * @param  parent the group to jsonize
 * @param  jgrpp return the array of jsonized grps
 * @return ::NC_NOERR No error.
 *
 * @author Dennis Heimbigner
 */
static int
NCZ_collect_grps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCjson** jsubgrpsp)
{
    int stat=NC_NOERR;
    size_t i;
    NCjson* jsubgrps = NULL;
    NCjson* jsubgrpname = NULL;

    ZTRACE(3,"file=%s parent=%s",file->controller->path,parent->hdr.name);

    NCJcheck(NCJnew(NCJ_ARRAY,&jsubgrps));

    for(i=0; i<ncindexsize(parent->children); i++) {
	NC_GRP_INFO_T* grp = (NC_GRP_INFO_T*)ncindexith(parent->children,i);
	jsubgrpname = NULL;
	
	/* jsonize subgrp name */
	NCJcheck(NCJnewstring(NCJ_STRING,grp->hdr.name,&jsubgrpname));

	/* Add the subgrp name*/
	NCJcheck(NCJappend(jsubgrps,jsubgrpname));
	jsubgrpname = NULL;
    }
    if(jsubgrpsp) {*jsubgrpsp = jsubgrps; jsubgrps = NULL;}

    NCJreclaim(jsubgrps);
    NCJreclaim(jsubgrpname);
    return ZUNTRACE(THROW(stat));
}

static int
subobjects_pure(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* varnames, NClist* grpnames)
{
    int stat = NC_NOERR;
    char* grpkey = NULL;

    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(grp,&grpkey))) goto done;
    /* Get the map and search group */
    if((stat = getnextlevel(zfile,grp,varnames,grpnames))) goto done;

done:
    nullfree(grpkey);
    return stat;
}

/**
Extract the child vars, groups, and dimensions for this group.
@param zfile
@param parent group
@param jnczgrp JSON for _nczarr_group attribute 
@param varnames list of names of variables in this group
@param grpnames list of names of subgroups in this group
@param dims list of NCZ_DimInfo representing the dimensions to be defined in this group.
@return NC_NOERR|NC_EXXX
*/
static int
subobjects(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* parent, const NCjson* jnczgrp, NClist* varnames, NClist* grpnames)
{
    int stat = NC_NOERR;
    size_t i;
    const NCjson* jsubgrps = NULL;
    const NCjson* jarrays = NULL;
    
    assert(jnczgrp != NULL);
    NCJcheck(NCJdictget(jnczgrp,"arrays",&jarrays));
    NCJcheck(NCJdictget(jnczgrp,"subgroups",&jsubgrps));

    for(i=0;i<NCJarraylength(jarrays);i++) {
	const NCjson* jname = NCJith(jarrays,i);
	nclistpush(varnames,strdup(NCJstring(jname)));
    }

    for(i=0;i<NCJarraylength(jsubgrps);i++) {
	const NCjson* jname = NCJith(jsubgrps,i);
	nclistpush(grpnames,strdup(NCJstring(jname)));
    }

    return stat;
}

/**
Given the following:
1. shape of a variable as a vector of integers [in]
2. rank of the variable [in[
3. set of dim names as simple names [in]
Produce the following:
4. set of fqns for each dimension [out]
If the dimension does not exist, then use an anonymous name.

We have two sources for the dimension names for this variable.
1. the "dimension_names":
   1.1 inside the zarr.json (V3) dictionary; this is the simple dimension name.
2. the "dimension_references" key inside the _nczarr_array dictionary;
   this contains FQNs for the dimensions.
Note that we may have both 1.1 and 2.

If purezarr, then we only have 1.1. In that case, for each name in "dimension_names",
we need to do the following:
1. get the i'th size from the "shape" vector.
2. if the i'th simple dimension name is null, then set the name to "_Anonymous_Dim<n>",
   where n is the size from the shape vector.
3. compute an equivalent of "dimension_references" by assuming that each simple dimension
   name maps to an FQN in the current group containing the given variable.

In any case, we now have an FQN for each dimension reference for the var.

@param file
@param grp containing var
@param ndims rank of the variable
@param shape the shape of the var
@param diminfo info about each dim
@return NC_NOERR|NC_EXXX
*/
int
NCZ_computedimrefs(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp,
               size_t ndims,
	       size64_t* shape,
	       NCZ_DimInfo* diminfo)
{
    int stat = NC_NOERR;
    size_t i;
    char digits[NC_MAX_NAME];
    NCbytes* newname = ncbytesnew();
    NCbytes* fqn = ncbytesnew();
    
    ZTRACE(3,"file=%s var=%s purezarr=%d ndims=%d shape=%s",
        file->controller->path,var->hdr.name,purezarr,ndims,nczprint_vector(ndims,shape));

    assert(diminfo != NULL);

    {
	/* Fill in dimnames */
        for(i=0;i<ndims;i++) {
	    NC_DIM_INFO_T* dim = NULL;	    
	    NC_GRP_INFO_T* parent = grp; /* create dim in this group */
	    NCZ_DimInfo* dimdata = &diminfo[i];

	    ncbytesclear(newname);
	    ncbytesclear(fqn);

            if(dimdata->name == NULL) { /* convert null name to anonymous name */
                /* create anonymous dimension name WRT root group and using the loopcounter */
                snprintf(digits,sizeof(digits),"_%llu",shape[i]);
		ncbytescat(newname,NCDIMANON);
		ncbytescat(newname,digits);
		dimdata->name = ncbytesextract(newname);
		parent = file->root_grp; /* anonymous are always created in the root group */
	    }

	    /* Find a consistent name and return a usable FQN for the dim */
	    if((stat = NCZ_uniquedimname(file,parent,dimdata,&dim,newname))) goto done;
	    if(dim == NULL) {
                /* Create the dimension */
	        if((stat = ncz4_create_dim(file,parent,ncbytescontents(newname),dimdata->shape,dimdata->unlimited,&dim))) goto done;
	    }
            nullfree(dimdata->fqn); dimdata->fqn = NULL;
  	    assert(dim != NULL);
	    {
		/* Get the dim's FQN */
		if((stat=NCZ_makeFQN(dim->container,ncbytescontents(newname),fqn))) goto done;
		dimdata->fqn = ncbytesextract(fqn);
	    }
	}
    }

done:
    ncbytesfree(newname);
    ncbytesfree(fqn);
    return ZUNTRACE(THROW(stat));
}

/**************************************************/
/* Format Filter Support Functions */

/* JSON Parse/unparse of filter codecs */

int
ZF3_hdf2codec(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Convert the HDF5 id + visible parameters to the codec form */

    /* Clear any previous codec */
    nullfree(filter->codec.id); filter->codec.id = NULL;
    nullfree(filter->codec.codec); filter->codec.codec = NULL;
    filter->codec.id = strdup(filter->plugin->codec.codec->codecid);
    if(filter->plugin->codec.codec->NCZ_hdf5_to_codec) {
	stat = filter->plugin->codec.codec->NCZ_hdf5_to_codec(NCplistzarrv3,filter->hdf5.id,filter->hdf5.visible.nparams,filter->hdf5.visible.params,&filter->codec.codec);
#ifdef DEBUGF
	fprintf(stderr,">>> DEBUGF: NCZ_hdf5_to_codec: visible=%s codec=%s\n",printnczparams(filter->hdf5.visible),filter->codec.codec);
#endif
	if(stat) goto done;
    } else
	{stat = NC_EFILTER; goto done;}

done:
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
    return THROW(stat);
}

/**************************************************/
/* Type Converters */

/**
@internal Given an nc_type+purezarr+MAXSTRLEN, produce the corresponding Zarr v3 dtype string.
@param nctype     - [in] nc_type
@param purezarr   - [in] 1=>pure zarr, 0 => nczarr
@param strlen     - [in] max string length
@param dtypep     - [out] pointer to hold pointer to the dtype; user frees
@param daliasp    - [out] pointer to hold pointer to the nczarr tag
@return NC_NOERR
@return NC_EINVAL
@author Dennis Heimbigner
*/

static int
nctype2dtype(nc_type nctype, int purezarr, size_t strlen, char** dtypep, char** daliasp)
{
    char dtype[NC_MAX_NAME];
    const char* dalias = NULL;
    const struct ZV3NCTYPE2DTYPE* zd = &zv3nctype2dtype[nctype];
    
    if(nctype <= NC_NAT || nctype > N_NCZARR_TYPES) return NC_EINVAL;
    dalias = zd->type_alias;
    snprintf(dtype,sizeof(dtype),zd->zarr,strlen*8); /* should work even if dtype does not have % char */
    if(dtypep) *dtypep = nulldup(dtype);
    if(daliasp) *daliasp = nulldup(dalias);
    return NC_NOERR;		
}

/*
@internal Convert a Zarr v3 data_type spec to a corresponding nc_type.
@param dtype    - [in] dtype to convert
@param dalias   - [in] alias of dtype
@param nctypep  - [out] hold corresponding type
@param typelenp - [out] hold corresponding type size (esp. for fixed length strings)
@return NC_NOERR
@return NC_EINVAL
@author Dennis Heimbigner
*/

static int
dtype2nctype(const char* dtype, const char* dalias, nc_type* nctypep, int* typelenp)
{
    int stat = NC_NOERR;
    nc_type nctype = NC_NAT;
    int typelen = 0; /* use int so we can return -1 to indicate undefined */
    size_t len = 0;
    int i, match;
    const struct ZV3DTYPE2NCTYPE* zd = NULL;
	
    if(nctypep) *nctypep = NC_NAT;
    if(typelenp) *typelenp = 0;

    assert(dtype != NULL || dalias != NULL);

    match = 0;
    /* Special cases */
    if(dalias != NULL) {
	if(strcmp(dalias,"string")==0) {
	    nctype = NC_STRING;
	    if(dtype != NULL) {
		if(!parse_rnnn(dtype,&len)) {stat = NC_ENCZARR; goto done;}
		typelen = (int)len;
	    } else
	        typelen = -1; /* Signal that we do not know the maxstrlen */
	    match = 1;
	} else if(strcmp(dalias,"char")==0) {
	    nctype = NC_CHAR;
	    typelen = 1;
	    match = 1;
	} else if(strcmp(dalias,"json")==0) {
	    nctype = NC_JSON; /* pseudo-type */
	    typelen = 1;
	    match = 1;
	}
    }
    if(dtype == NULL) dtype = dalias; /* dalias might be a simple type like int32 */
    if(!match) { /* try the dtype directly */
	if(parse_rnnn(dtype,&len)) { /* Special case => string */
	    nctype = NC_STRING;
	    match = 1;
	    typelen = (int)len;
	} else { /* should be a simple numeric type */
	    for(i=1;i<N_NCZARR_TYPES;i++) { /* Including NC_JSON */
	        zd = &zv3dtype2nctype[i];
	        if(strcmp(dtype,zd->dtype)==0)
		    {nctype = i; typelen = zd->typelen; match = 1; break;}
	    }
            if(!match || zd == NULL || typelen == -1) {stat = NC_ENOTZARR; goto done;}
	}
    }
    if(nctypep) *nctypep = nctype;
    if(typelenp) *typelenp = typelen;

done:
    return THROW(stat);
}

/* Check for rnnn and compute the value of nnn as chars
@param dtype to check for rnnn form
@param typelenp return computed nnn as chars
@return 1 if dtype rnnn form 0 otherwise
*/
static int
parse_rnnn(const char* dtype, size_t* typelenp)
{
    size_t typelen;
    assert(dtype != NULL);
    if(1 != sscanf(dtype,STRTEMPLATE,&typelen)) return 0;
    if((typelen % 8) != 0) return 0;
    typelen = typelen / 8; /* convert bits to bytes */
    if(typelenp) *typelenp = typelen;
    return 1;
}

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
/**
* Convert a JSON codec to an instance of NCZ_Filter
* @param jfilter json codec
* @param zfilterp return NCZ_Filter instance
* @return NC_NOERR | NC_EXXX
*/
static int
json2filter(NC_FILE_INFO_T* file, const NCjson* jfilter, NCZ_Filter** zfilterp, NClist* filterlist)
{
    int stat = NC_NOERR;
    const NCjson* jvalue = NULL;
    NCZ_Filter* filter = NULL;
    NCZ_Plugin* plugin = NULL;
    NCZ_Codec codec;
    NCZ_HDF5 hdf5;

    ZTRACE(6,"file=%s var=%s jfilter=%s",file->hdr.name,var->hdr.name,NCJtrace(jfilter));

    memset(&codec,0,sizeof(NCZ_Codec));
    memset(&hdf5,0,sizeof(NCZ_HDF5));

    /* Get the id of this codec filter */
    if(NCJdictget(jfilter,"name",&jvalue)<0) {stat = NC_EFILTER; goto done;}
    if(!NCJisatomic(jvalue)) {stat = THROW(NC_ENOFILTER); goto done;}
    codec.id = strdup(NCJstring(jvalue));
    /* Save the codec for this filter */
    NCJcheck(NCJunparse(jfilter, 0, &codec.codec));

    if((stat = NCZ_codec_plugin_lookup(codec.id,&plugin))) goto done;
    if(plugin != NULL && !plugin->incomplete) {
	/* Save the hdf5 id */
	assert(plugin->hdf5.filter != NULL);
	hdf5.id = (unsigned)plugin->hdf5.filter->id;
        /* Convert the codec to hdf5 form visible parameters */
        if(plugin->codec.codec->NCZ_codec_to_hdf5) {
            if((stat = plugin->codec.codec->NCZ_codec_to_hdf5(NCplistzarrv3,codec.codec,&hdf5.id,&hdf5.visible.nparams,&hdf5.visible.params))) goto done;
#ifdef DEBUGF
            fprintf(stderr,">>> DEBUGF: json2filter: codec=%s, hdf5=%s\n",printcodec(codec),printhdf5(hdf5));
#endif
        }
    } /* else plugin == NULL || plugin->incomplete */

    /* Create the filter */
    if((stat = ncz4_create_filter(file,&hdf5,&codec,filterlist,&filter))) goto done;
    if(zfilterp) *zfilterp = filter;
    filter = NULL; /* its in filterchain if we get here */

done:
    NCZ_filter_hdf5_clear(&hdf5);
    NCZ_filter_codec_clear(&codec);    
    NCZ_filter_free(filter);
    return THROW(stat);
}
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/

/*************************************************/


/**
Given a group path, collect the immediate
descendant information.
The procedure is as follows:
1. Let X be the set of names and objects just below the parent grp kkey.
2. assert zarr.json is in X representing the parent grp.
3. For each name N in X see if N/zarr.json exists.
   If so, then read zarr.json and see if it is a group vs array zarr.json
4. For all M in X s.t. M/zarr.json does not exist, assume M is a virtual group.

@param
@return
*/
static int
getnextlevel(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* parent, NClist* varnames, NClist* subgrpnames)
{
    int stat = NC_NOERR;
    size_t i;
    char* grpkey = NULL;
    char* subkey = NULL;
    char* zobject = NULL;
    NClist* matches = nclistnew();
    void* content = NULL;
    NCjson* jzarrjson = NULL;

    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(parent,&grpkey))) goto done;
    /* Get the map and search group for nextlevel of objects */
    if((stat = nczmap_list(zfile->map,grpkey,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
	size64_t zjlen;
        const char* name = (const char*)nclistget(matches,i);
	const NCjson* jnodetype;
	if(strcmp(name,Z3OBJECT)==0) {continue;}
        /* See if name/zarr.json exists */
        nullfree(subkey); subkey = NULL;
        if((stat = nczm_concat(grpkey,name,&subkey))) goto done;
	nullfree(zobject); zobject = NULL;
        if((stat = nczm_concat(subkey,Z3OBJECT,&zobject))) goto done;
        switch(stat = nczmap_len(zfile->map,zobject,&zjlen)) {
	case NC_NOERR: break;
	case NC_ENOOBJECT: nclistpush(subgrpnames,name); continue; /* assume a virtual group */
	default: goto done;
	}
	nullfree(content); content = NULL;
	if((content = malloc(zjlen))==NULL) {stat = NC_ENOMEM; goto done;}
        if((stat = nczmap_read(zfile->map,zobject,0,zjlen,content))) goto done; /* read the zarr.json */
	/* parse it */
	NCJreclaim(jzarrjson); jzarrjson = NULL;
	NCJcheck(NCJparsen((size_t)zjlen,(char*)content,0,&jzarrjson));
	if(jzarrjson == NULL) {stat = NC_ENOTZARR; goto done;}
	/* See what the node_type says */
	NCJcheck(NCJdictget(jzarrjson,"node_type",&jnodetype));
	if(strcmp("array",NCJstring(jnodetype))==0)
	    nclistpush(varnames,strdup(name));
	else if(strcmp("group",NCJstring(jnodetype))==0)
	    nclistpush(subgrpnames,strdup(name));
	else
	    {stat = NC_ENOTZARR; goto done;}
    }

done:
    nullfree(content);
    nullfree(grpkey);
    nullfree(subkey);
    nullfree(zobject);
    nclistfreeall(matches);
    NCJreclaim(jzarrjson);
    return stat;
}

/**************************************************/
/* Utilities */

/* Build an attribute type json dict */
static NCjson*
build_attr_type_dict(NCjson** jdictp, const char* aname, const char* dtype)
{
    NCjson* jdict = *jdictp;
    NCjson* jtype = NULL;
    if(jdict == NULL) NCJcheck(NCJnew(NCJ_DICT,&jdict));
    NCJcheck(NCJinsertstring(jdict,aname,dtype));
    *jdictp = jdict;
    return jtype;
}

/* Build a {name,configuration} dict */
static NCjson*
build_named_config(const char* name, int pairs, ...)
{
    NCjson* jdict = NULL;
    NCjson* jcfg = NULL;
    va_list ap;
    int i;

    NCJcheck(NCJnew(NCJ_DICT,&jdict));
    NCJcheck(NCJinsertstring(jdict,"name",name));
    NCJcheck(NCJnew(NCJ_DICT,&jcfg));
    /* Get the varargs */
    va_start(ap, pairs);
    for(i=0;i<pairs;i++) {
	char* key = va_arg(ap, char*);
	NCjson* value = va_arg(ap, NCjson*);
        NCJinsert(jcfg,key,value);
    }
    NCJinsert(jdict,"configuration",jcfg);
    va_end(ap);
    return jdict;
}

/**************************************************/
/* Format Dispatch table */

static const NCZ_Formatter NCZ_formatter3_table = {
    NCZARRFORMAT3,
    ZARRFORMAT3,
    NCZ_FORMATTER_VERSION,

    ZF3_create,
    ZF3_open,
    ZF3_close,
    ZF3_readmeta,
    ZF3_writemeta,
    ZF3_hdf2codec,
    ZF3_buildchunkkey
};

const NCZ_Formatter* NCZ_formatter3 = &NCZ_formatter3_table;

int
NCZF3_initialize(void)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;
    if(NCJparse(NCZ_Bytes_Little_Text,0,&json) < 0) {stat = NC_EINTERNAL; goto done;}
    NCZ_Bytes_Little_Json = json;
    if(NCJparse(NCZ_Bytes_Big_Text,0,&json) < 0) {stat = NC_EINTERNAL; goto done;}
    NCZ_Bytes_Big_Json = json;
done:
    return THROW(stat);
}

int
NCZF3_finalize(void)
{
    return NC_NOERR;
}

