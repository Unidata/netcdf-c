/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "znc4.h"
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
#include "netcdf_filter_build.h"
#endif
#include "zfill.h"

/**************************************************/
/* Static zarr type name table */
/* Used to convert nc_type <=> dtype */
static const struct ZTYPESV2 {
    const char* dtype;
    int typelen;    
} znamesv2[N_NCZARR_TYPES] = {
/* nc_type       dtype */
/*NC_NAT*/	{NULL,0},
/*NC_BYTE*/	{"|i1",1},
/*NC_CHAR*/	{">S1",1},
/*NC_SHORT*/	{"|i2",2},
/*NC_INT*/	{"|i4",4},
/*NC_FLOAT*/	{"|f4",4},
/*NC_DOUBLE*/	{"|f8",8},
/*NC_UBYTE*/	{"|u1",1},
/*NC_USHORT*/	{"|u2",2},
/*NC_UINT*/	{"|u4",4},
/*NC_INT64*/	{"|i8",8},
/*NC_UINT64*/	{"|u8",8},
/*NC_STRING*/	{"|S%d",0},
/*NC_JSON*/	{"|J0",0} /* NCZarr internal type */
};

struct Ainfo {
    nc_type type;
    int endianness;
    size_t typelen;
};

/* Capture arguments for ncz4_create_var */
struct CVARGS {
	const char* varname;
	nc_type vtype;
	int storage;
	int scalar;
	int endianness;
	size_t maxstrlen;
	char dimension_separator;
	char order;
	size_t rank;
	size64_t shapes[NC_MAX_VAR_DIMS];
	size64_t chunks[NC_MAX_VAR_DIMS];
	int dimids[NC_MAX_VAR_DIMS];
	NClist* filterlist;
	int no_fill;
	void* fill_value;
};

/**************************************************/

/*Forward*/
static int ZF2_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
static int ZF2_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
static int ZF2_close(NC_FILE_INFO_T* file);
static int ZF2_readmeta(NC_FILE_INFO_T* file);
static int ZF2_writemeta(NC_FILE_INFO_T* file);
static int ZF2_buildchunkkey(size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);
static int ZF2_hdf2codec(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter);

static int write_grp(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_GRP_INFO_T* grp);
static int write_var_meta(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_VAR_INFO_T* var);
static int write_var(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_VAR_INFO_T* var);
static int build_atts(NC_FILE_INFO_T* file, NC_OBJ* container, NCindex* attlist, NCjson** jattsp, NCjson** jtypesp);

static int read_superblock(NC_FILE_INFO_T* file, const NCjson* jzsuper, int* nczarrvp);
static int read_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, NC_GRP_INFO_T** grpp);
static int read_grp_contents(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
static int read_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* varnames);
static int read_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* subgrpnames);
static int parse_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson* jatts);

static int locate_nczarr_grp_info(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson* jgroup, NCjson* jatts,
			const NCjson** jzgroupp, const NCjson** jzattsp, const NCjson** jzsuperp, int* nczv21p);
static int locate_nczarr_array_info(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson* jarray, NCjson* jatts,
			const NCjson** jzarrayp, const NCjson** jzattsp, int* nczv21p);
static int define_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* dimdefs, int** dimidsp);
static int define_dim1(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCZ_DimInfo* dinfo, int* dimidp);
static int collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jdimsp);
static int parse_group_content(const NCjson* jcontent, NClist* dimdefs, NClist* varnames, NClist* subgrps);
static int parse_group_content_pure(NC_FILE_INFO_T*  zinfo, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrps);
static int searchvars(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* varnames);
static int searchsubgrps(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* subgrpnames);
static int nctype2dtype(nc_type nctype, int endianness, int purezarr, size_t len, char** dnamep, const char** tagp);
static int dtype2nctype(const char* dtype, nc_type* nctypep, size_t* typelenp, int* endianp);
static int get_att_types(int purezarr, NCjson* jatts, struct Ainfo* ainfo);
static int collectdimrefs(const NCjson* jdimrefs, struct CVARGS* cvargs, NCZ_DimInfo* diminfo);
static int collectxarraydims(const NCjson* jxarray, struct CVARGS* cvargs, NCZ_DimInfo* diminfo);
static int NCZ_computedimrefs(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, size_t ndims, size64_t* shape, NCZ_DimInfo* diminfo, int* isscalarp);
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
ZF2_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    ZTRACE(4,"file=%s",file->controller->path);
    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    return ZUNTRACE(THROW(stat));
}

static int
ZF2_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map)
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
ZF2_writemeta(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;
    NCZMAP* map = NULL;

    ZTRACE(4,"file=%s",file->controller->path);

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    map = zfile->map;
    assert(map != NULL);

    /* Write out root group recursively */
    if((stat = write_grp(file, zfile, map, file->root_grp)))
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
write_grp(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, NC_GRP_INFO_T* grp)
{
    int stat = NC_NOERR;
    size_t i;
    char version[1024];
    int purezarr = 0;
    char* fullpath = NULL;
    char* key = NULL;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jgroup = NULL;
    NCjson* jdims = NULL;
    NCjson* jvars = NULL;
    NCjson* jsubgrps = NULL;
    NCjson* jsuper = NULL;
    NCjson* jtmp = NULL;
    NCjson* jzgrp = NULL;
    NCjson* jzatts = NULL;

    ZTRACE(3,"file=%s grp=%s isclose=%d",file->controller->path,grp->hdr.name,isclose);

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    /* Construct grp key */
    if((stat = NCZ_grpkey(grp,&fullpath)))
	goto done;

    /* build ZGROUP contents */
    NCJcheck(NCJnew(NCJ_DICT,&jgroup));
    snprintf(version,sizeof(version),"%d",zfile->zarr.zarr_format);
    NCJcheck(NCJaddstring(jgroup,NCJ_STRING,"zarr_format"));
    NCJcheck(NCJaddstring(jgroup,NCJ_INT,version));

    /* build Z2GROUP path */
    if((stat = nczm_concat(fullpath,Z2GROUP,&key))) goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jgroup))) goto done;
    nullfree(key); key = NULL;

    /* Build the Z2ATTRS object and the corresponding attribute types list */
    assert(grp->att);
    if((stat = build_atts(file,(NC_OBJ*)grp, grp->att, &jatts, &jtypes)))goto done;

    /* Add NCZarr related attributes (and types)*/
    if(!purezarr) {
	if(jtypes == NULL) NCJcheck(NCJnew(NCJ_DICT,&jtypes)); /* Make sure this exists */
        if(jzatts == NULL) {NCJcheck(NCJnew(NCJ_DICT,&jzatts));} /* ditto */

	/* Create dimensions dict */
	if((stat = collect_dims(file,grp,&jdims))) goto done;

	/* Create vars list */
	NCJnew(NCJ_ARRAY,&jvars);
	for(i=0; i<ncindexsize(grp->vars); i++) {
	    NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	    NCJaddstring(jvars,NCJ_STRING,var->hdr.name);
	}

	/* Create subgroups list */
	NCJnew(NCJ_ARRAY,&jsubgrps);
	for(i=0; i<ncindexsize(grp->children); i++) {
	    NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	    NCJaddstring(jsubgrps,NCJ_STRING,g->hdr.name);
	}

	/* Create the "_nczarr_group" dict */
	NCJcheck(NCJnew(NCJ_DICT,&jzgrp));
	/* Insert the various dicts and arrays */
	NCJcheck(NCJinsert(jzgrp,"dims",jdims));
	jdims = NULL; /* avoid memory problems */
	NCJcheck(NCJinsert(jzgrp,"vars",jvars));
	jvars = NULL; /* avoid memory problems */
	NCJcheck(NCJinsert(jzgrp,"groups",jsubgrps));
	jsubgrps = NULL; /* avoid memory problems */

	/* Insert the "_nczarr_group" attribute */
	NCJcheck(NCJinsert(jatts,NCZ_V2_GROUP,jzgrp));
	jzgrp = NULL;

	/* Add _nczarr_group type (|J0) */
	NCJcheck(NCJinsertstring(jtypes,NCZ_V2_GROUP,"|J0"));

        if(grp->parent == NULL) { /* Root group => add _nczarr_superblock and type */
            /* Track the library version that wrote this */
            strncpy(version,NCZARR_PACKAGE_VERSION,sizeof(version));
            NCJcheck(NCJnew(NCJ_DICT,&jsuper));
            NCJcheck(NCJnewstring(NCJ_STRING,version,&jtmp));
            NCJcheck(NCJinsert(jsuper,"version",jtmp));
            jtmp = NULL;
            snprintf(version,sizeof(version),"%u", (unsigned)zfile->zarr.nczarr_format);
            NCJcheck(NCJnewstring(NCJ_INT,version,&jtmp));
            NCJcheck(NCJinsert(jsuper,"format",jtmp));
            jtmp = NULL;
            NCJcheck(NCJinsert(jatts,NCZ_V2_SUPERBLOCK,jsuper));
            jsuper = NULL;
  	    if(jtypes == NULL) NCJcheck(NCJnew(NCJ_DICT,&jtypes));
	    /* Add _nczarr_superblock type (|J0) */
	    NCJcheck(NCJinsertstring(jtypes,NCZ_V2_SUPERBLOCK,"|J0"));
        }

	/* Insert _nczarr_attrs type */
	NCJcheck(NCJinsertstring(jtypes,NCZ_V2_ATTR,"|J0"));

	/* Save the types */
	NCJcheck(NCJinsert(jzatts,"types",jtypes));
	jtypes = NULL;

        /* Insert the "_nczarr_attrs" attribute */
        NCJcheck(NCJinsert(jatts,NCZ_V2_ATTR,jzatts));
	jzatts = NULL;
    }
    
    /* write .zattrs path */
    if((stat = nczm_concat(fullpath,Z2ATTRS,&key))) goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jatts))) goto done;
    nullfree(key); key = NULL;

    /* Now write all the variables */
    for(i=0; i<ncindexsize(grp->vars); i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	if((stat = write_var(file,zfile,map,var))) goto done;
    }

    /* Now recurse to synchronize all the subgrps */
    for(i=0; i<ncindexsize(grp->children); i++) {
	NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = write_grp(file,zfile,map,g))) goto done;
    }

done:
    NCJreclaim(jatts);
    NCJreclaim(jgroup);
    NCJreclaim(jdims);
    NCJreclaim(jvars);
    NCJreclaim(jsubgrps);
    NCJreclaim(jsuper);
    NCJreclaim(jtmp);
    NCJreclaim(jzgrp);
    NCJreclaim(jzatts);
    nullfree(fullpath);
    nullfree(key);
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
    char number[1024];
    char* fullpath = NULL;
    char* key = NULL;
    char* dimpath = NULL;
    NClist* dimrefs = NULL;
    NCjson* jvar = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jtmp = NULL;
    NCjson* jfill = NULL;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jzarray = NULL;
    NCjson* jzatts = NULL;
    char* dtypename = NULL;
    int purezarr = 0;
    size64_t shape[NC_MAX_VAR_DIMS];
    NCZ_VAR_INFO_T* zvar = var->format_var_info;
    NCjson* jfilters = NULL;
    NCjson* jfilter = NULL;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    NClist* filterchain = NULL;
#endif

    ZTRACE(3,"file=%s var=%s isclose=%d",file->controller->path,var->hdr.name,isclose);

    zfile = file->format_file_info;
    map = zfile->map;

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    /* Make sure that everything is established */
    /* ensure the fill value */
    if((stat = NCZ_ensure_fill_value(var))) goto done; /* ensure var->fill_value is set */
    assert(var->no_fill || var->fill_value != NULL);
    /* rebuild the fill chunk */
    if((stat = NCZ_ensure_fill_chunk(zvar->cache))) goto done;

    /* Construct var path */
    if((stat = NCZ_varkey(var,&fullpath)))
	goto done;

    /* Create the .zarray json object */
    NCJcheck(NCJnew(NCJ_DICT,&jvar));

    /* zarr_format key */
    snprintf(number,sizeof(number),"%d",zfile->zarr.zarr_format);
    NCJcheck(NCJaddstring(jvar,NCJ_STRING,"zarr_format"));
    NCJcheck(NCJaddstring(jvar,NCJ_INT,number));

    /* Collect the shape vector */
    for(i=0;i<var->ndims;i++) {
	NC_DIM_INFO_T* dim = var->dim[i];
	shape[i] = dim->len;
    }

    /* shape key */
    /* Integer list defining the length of each dimension of the array.*/
    /* Create the list */
    NCJcheck(NCJnew(NCJ_ARRAY,&jtmp));
    if(var->ndims == 0) { /* scalar */
	NCJaddstring(jtmp,NCJ_INT,"1"); /* pretend scalar x is x[1] */
    } else for(i=0;i<var->ndims;i++) {
	snprintf(number,sizeof(number),"%llu",shape[i]);
	NCJaddstring(jtmp,NCJ_INT,number);
    }
    NCJcheck(NCJinsert(jvar,"shape",jtmp));
    jtmp = NULL;

    /* dtype key */
    /* A string or list defining a valid data type for the array. */
    NCJcheck(NCJaddstring(jvar,NCJ_STRING,"dtype"));
    {	/* Compute the type name */
	int endianness = var->endianness;
	int atomictype = var->type_info->hdr.id;
	assert(atomictype > 0 && atomictype <= NC_MAX_ATOMIC_TYPE);
	if((stat = nctype2dtype(atomictype,endianness,purezarr,NCZ_get_maxstrlen((NC_OBJ*)var),&dtypename,NULL))) goto done;
	NCJaddstring(jvar,NCJ_STRING,dtypename);
	nullfree(dtypename); dtypename = NULL;
    }

    /* chunks key */
    /* Create the list */
    NCJcheck(NCJnew(NCJ_ARRAY,&jtmp));
    if(var->ndims == 0) {/* treat scalar like chunks of [1] */
	NCJaddstring(jtmp,NCJ_INT,"1"); /* one chunk of size 1 */
    } else for(i=0;i<var->ndims;i++) {
	size64_t len = var->chunksizes[i];
	snprintf(number,sizeof(number),"%lld",len);
	NCJaddstring(jtmp,NCJ_INT,number);
    }
    NCJcheck(NCJinsert(jvar,"chunks",jtmp));
    jtmp = NULL;

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

    /* order key */
    NCJcheck(NCJaddstring(jvar,NCJ_STRING,"order"));
    /* "C" means row-major order, i.e., the last dimension varies fastest;
       "F" means column-major order, i.e., the first dimension varies fastest.*/
    /* Default to C for now */
    NCJcheck(NCJaddstring(jvar,NCJ_STRING,"C"));

    /* dimension_separator key */
    /* Single char defining the separator in chunk keys */
    if(zvar->dimension_separator != DFALT_DIM_SEPARATOR_V2) {
	char sep[2];
	sep[0] = zvar->dimension_separator;/* make separator a string*/
	sep[1] = '\0';
	NCJnewstring(NCJ_STRING,sep,&jtmp);
	NCJinsert(jvar,"dimension_separator",jtmp);
	jtmp = NULL;
    }

    /* Capture dimref names as FQNs */
    if((dimrefs = nclistnew())==NULL) {stat = NC_ENOMEM; goto done;}
    if(var->ndims == 0) {
	nclistpush(dimrefs,strdup(DIMSCALAR));
    } else {/*var->ndims > 0*/
	for(i=0;i<var->ndims;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    if((stat = NCZ_dimkey(dim,&dimpath))) goto done;
	    nclistpush(dimrefs,dimpath);
	    dimpath = NULL;
	}
    }

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Compressor and Filters */
    /* Build the filter working parameters for any filters */
    if((stat = NCZ_filter_setup(var))) goto done;
#endif

    /* compressor key */
    /* From V2 Spec: A JSON object identifying the primary compression codec and providing
       configuration parameters, or ``null`` if no compressor is to be used. */
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    filterchain = (NClist*)var->filters;
    if(nclistlength(filterchain) > 0) {
	NCZ_Filter* filter = (struct NCZ_Filter*)nclistget(filterchain,nclistlength(filterchain)-1);
	/* encode up the compressor */
	if((stat = NCZ_filter_jsonize(file,var,filter,&jfilter))) goto done;
    } else /* |filterchain| == 0 */
#endif
    { /* no filters at all */
	/* Default to null */
	NCJnew(NCJ_NULL,&jfilter);
    }
    NCJcheck(NCJinsert(jvar,"compressor",jfilter));
    jfilter = NULL;

    /* filters key */
    /* From V2 Spec: A list of JSON objects providing codec configurations,
       or null if no filters are to be applied. Each codec configuration
       object MUST contain a "id" key identifying the codec to be used. */
    /* A list of JSON objects providing codec configurations, or ``null``
       if no filters are to be applied. */
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    assert(jfilters == NULL);
    if(nclistlength(filterchain) > 1) {
	size_t k;
        NCJcheck(NCJnew(NCJ_ARRAY,&jfilters));
	for(k=0;k<nclistlength(filterchain)-1;k++) {
	    NCZ_Filter* filter = (struct NCZ_Filter*)nclistget(filterchain,k);
	    /* encode up the filter as a string */
	    if((stat = NCZ_filter_jsonize(file,var,filter,&jfilter))) goto done;
	    NCJappend(jfilters,jfilter); jfilter = NULL;
	}
    } else
#endif
    { /* no filters at all */
	NCJcheck(NCJnew(NCJ_NULL,&jfilters));
    }
    NCJcheck(NCJinsert(jvar,"filters",jfilters));
    jfilters = NULL;

    /* ensure the chunk cache */
    if((stat = NCZ_adjust_var_cache(var))) goto done;

    /* Build the Z2ATTRS object */
    assert(var->att);
    if((stat = build_atts(file,(NC_OBJ*)var, var->att, &jatts, &jtypes)))goto done;

    /* Add the NCZ_V2_ARRAY attribute */
    if(!purezarr) {
	if(jtypes == NULL) NCJcheck(NCJnew(NCJ_DICT,&jtypes)); /* Make sure this exists */
        if(jzatts == NULL) {NCJcheck(NCJnew(NCJ_DICT,&jzatts));} /* ditto */

	NCJnew(NCJ_DICT,&jzarray); /* NCZ_V2_ARRAY contents */

	/* Create the dimrefs json object */
	NCJnew(NCJ_ARRAY,&jdimrefs);
	for(i=0;i<nclistlength(dimrefs);i++) {
	    const char* dim = nclistget(dimrefs,i);
	    NCJaddstring(jdimrefs,NCJ_STRING,dim);
	}
	NCJinsert(jzarray,"dimrefs",jdimrefs);
	jdimrefs = NULL; /* Avoid memory problems */

	/* Set the storage type */
	if(var->ndims == 0) {
	    NCJnewstring(NCJ_STRING,"scalar",&jtmp);
	} else {
	    NCJnewstring(NCJ_STRING,"chunked",&jtmp);
	}
        NCJinsert(jzarray,"storage",jtmp);
	jtmp = NULL;

        NCJinsert(jatts,NCZ_V2_ARRAY,jzarray);
        jzarray = NULL;
	NCJinsertstring(jtypes,NCZ_V2_ARRAY,"|J0");

        /* Add NCZ_V2_ATTRS attribute */
	NCJinsertstring(jtypes,NCZ_V2_ATTR,"|J0");
	NCJinsert(jzatts,"types",jtypes);
	jtypes = NULL;
        NCJinsert(jatts,NCZ_V2_ATTR,jzatts);
        jzatts = NULL;
    }

    var->created = 1;

    /* build .zarray path and write */
    if((stat = nczm_concat(fullpath,Z2ARRAY,&key))) goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jvar))) goto done;
    nullfree(key); key = NULL;

    /* Build and write .zattrs object */
    /* write .zattrs path */
    if((stat = nczm_concat(fullpath,Z2ATTRS,&key))) goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jatts))) goto done;
    nullfree(key); key = NULL;

done:
    nclistfreeall(dimrefs);
    nullfree(fullpath);
    nullfree(key);
    nullfree(dtypename);
    nullfree(dimpath);
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    NCJreclaim(jfilters);
    NCJreclaim(jfilter);
#endif
    NCJreclaim(jdimrefs);
    NCJreclaim(jtmp);
    NCJreclaim(jfill);
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    NCJreclaim(jzarray);
    NCJreclaim(jzatts);
    NCJreclaim(jvar);
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
 * @internal Synchronize attribute data to JSON representation.
 * Additionally, compute the attribute types.
 *
 * @param file Pointer to file struct
 * @param container Pointer to grp|var struct containing the attributes
 * @param attlist list of attributes from the container
 * @param jattsp return list of attributes in JSON form
 * @param jtypesp return list of attribute types in JSON form
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
build_atts(NC_FILE_INFO_T* file, NC_OBJ* container, NCindex* attlist, NCjson** jattsp, NCjson** jtypesp)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jtype = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jdict = NULL;
    NCjson* jint = NULL;
    NCjson* jdata = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    char* content = NULL;
    char* dimpath = NULL;
    int isxarray = 0;
    int purezarr = 0;
    int inrootgroup = 0;
    NC_VAR_INFO_T* var = NULL;
    char* dtype = NULL;
    int endianness = (NC_isLittleEndian()?NC_ENDIAN_LITTLE:NC_ENDIAN_BIG);

    ZTRACE(3,"file=%s container=%s |attlist|=%u",file->controller->path,container->name,(unsigned)ncindexsize(attlist));

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;
    if(zfile->flags & FLAG_XARRAYDIMS) isxarray = 1;

    if(container->sort == NCVAR) {
	var = (NC_VAR_INFO_T*)container;
	if(var->container && var->container->parent == NULL)
	    inrootgroup = 1;
    }

    /* Create the attribute dictionary */
    NCJcheck(NCJnew(NCJ_DICT,&jatts));

    if(!purezarr) {
  	/* Create the types dict */
	NCJnew(NCJ_DICT,&jtypes);
    }

    if(ncindexsize(attlist) > 0) {
	/* Walk all the attributes convert to json and compute the dtype */
	for(i=0;i<ncindexsize(attlist);i++) {
	    NC_ATT_INFO_T* a = (NC_ATT_INFO_T*)ncindexith(attlist,i);
	    size_t typesize = 0;
	    nc_type internaltype = a->nc_typeid;

	    /* Special cases */	
	    if(container->sort == NCVAR && strcmp(a->hdr.name,NC_FillValue)==0) continue; /* Ignore _FillValue */
	
	    if(a->nc_typeid > NC_MAX_ATOMIC_TYPE)
		{stat = (THROW(NC_ENCZARR)); goto done;}
	    if(a->nc_typeid == NC_STRING)
		typesize = NCZ_get_maxstrlen(container);
	    else
		{if((stat = NC4_inq_atomic_type(a->nc_typeid,NULL,&typesize))) goto done;}

	    /* Track complex json representation*/
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
		if((stat = nctype2dtype(internaltype,endianness,purezarr,typesize,&dtype,NULL))) goto done;
		NCJnewstring(NCJ_STRING,dtype,&jtype);
		nullfree(dtype); dtype = NULL;
		NCJinsert(jtypes,a->hdr.name,jtype); /* add {name: type} */
		jtype = NULL;
	    }
	}
    }

    /* Add Quantize Attribute */
    if(container->sort == NCVAR && var && var->quantize_mode > 0) {
        char mode[64];
        const char* qattname = NULL;
        snprintf(mode,sizeof(mode),"%d",var->nsd);
        NCJcheck(NCJnewstring(NCJ_INT,mode,&jint));
        /* Insert the quantize attribute */
        switch (var->quantize_mode) {
        case NC_QUANTIZE_BITGROOM:
            qattname = NC_QUANTIZE_BITGROOM_ATT_NAME;
            break;
        case NC_QUANTIZE_GRANULARBR:
            qattname = NC_QUANTIZE_GRANULARBR_ATT_NAME;
            break;
        case NC_QUANTIZE_BITROUND:
            qattname = NC_QUANTIZE_BITROUND_ATT_NAME;
            break;
        default: {stat = NC_ENCZARR; goto done;}
        }
        if(!purezarr) {
            NCJcheck(NCJnewstring(NCJ_STRING,"<u4",&jtype));
            NCJcheck(NCJinsert(jtypes,qattname,jtype));
            jtype = NULL;
	    }
        NCJcheck(NCJinsert(jatts,qattname,jint));
        jint = NULL;
    }

    /* Insert optional XARRAY attribute */
    if(container->sort == NCVAR) {
	if(inrootgroup && isxarray) {
	    int dimsinroot = 1;
	    /* Insert the xarray _ARRAY_ATTRIBUTE attribute */
	    NCJnew(NCJ_ARRAY,&jdimrefs);
	    /* Fake the scalar case */
	    if(var->ndims == 0) {
		NCJaddstring(jdimrefs,NCJ_STRING,XARRAYSCALAR);
		dimsinroot = 1; /* define XARRAYSCALAR in root group */
	    } else { /* Walk the dimensions and capture the names */
	        for(i=0;i<var->ndims;i++) {
		    NC_DIM_INFO_T* dim = var->dim[i];
		    /* Verify that the dimension is in the root group */
		    if(dim->container && dim->container->parent != NULL) {
		        dimsinroot = 0; /* dimension is not in root */
		        break;
		    }
	        }
	    }
            if(dimsinroot) {
                /* Walk the dimensions and capture the names */
                for(i=0;i<var->ndims;i++) {
                    char* dimname;
                    NC_DIM_INFO_T* dim = var->dim[i];
                    dimname = strdup(dim->hdr.name);
                    if(dimname == NULL) {stat = NC_ENOMEM; goto done;}
                    NCJcheck(NCJaddstring(jdimrefs,NCJ_STRING,dimname));
                    nullfree(dimname); dimname = NULL;
                }
                /* Add the _ARRAY_DIMENSIONS attribute */
                NCJcheck(NCJinsert(jatts,NC_XARRAY_DIMS,jdimrefs));
                jdimrefs = NULL;
                /* And a fake type */
                if(!purezarr) {
                    NCJcheck(NCJnewstring(NCJ_STRING,"|J0",&jtype));
                    NCJcheck(NCJinsert(jtypes,NC_XARRAY_DIMS,jtype)); /* add {name: type} */
                    jtype = NULL;
                }
            }
        }
    }

    if(jattsp) {*jattsp = jatts; jatts = NULL;}
    if(jtypesp) {*jtypesp = jtypes; jtypes = NULL;}

done:
    nullfree(fullpath);
    nullfree(key);
    nullfree(content);
    nullfree(dimpath);
    nullfree(dtype);
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    NCJreclaim(jtype);
    NCJreclaim(jdimrefs);
    NCJreclaim(jdict);
    NCJreclaim(jint);
    NCJreclaim(jdata);
    return ZUNTRACE(THROW(stat));
}

/**************************************************/

/**************************************************/
/* Map->Internal */


/**
 * @internal Read file data from map to memory.
 *
 * @param file Pointer to file info struct.
 *
 * @return NC_NOERR If no error.
 *         NC_EXXX for error returns
 *
 * @author Dennis Heimbigner
 */
static int
ZF2_readmeta(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;

    ZTRACE(3,"file=%s",file->controller->path);

    /* Now load the groups starting with root */
    assert(file->root_grp != NULL);
    if((stat = read_grp_contents(file,file->root_grp))) goto done;

done:
    return ZUNTRACE(THROW(stat));
}

/**
@internal Create attributes from info stored in NCZ_{GRP|VAR}_INFO_T object.
@param file - [in] the containing file (annotation)
@param container - [in] the containing object (var or grp) into which to store the attributes
@param ainfop - [out] the standardized attribute info.
@return NC_NOERR|NC_EXXX

@author Dennis Heimbigner
*/
static int
parse_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson* jatts)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t natts = 0;
    size_t i;
    size_t len = 0;
    void* data = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NCZ_VAR_INFO_T* zvar = NULL;
    struct Ainfo* ainfo = NULL;
    int purezarr = 0;
    
    ZTRACE(3,"map=%p container=%s nczarrv1=%d",map,container->name,nczarrv1);

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    if(container->sort == NCGRP) {
        grp = (NC_GRP_INFO_T*)container;
    } else {
        var = (NC_VAR_INFO_T*)container;
	zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    }

    if(jatts != NULL) {
        if(NCJsort(jatts) != NCJ_DICT) {stat = THROW(NC_ENCZARR); goto done;}
        natts = NCJdictlength(jatts);
	if((ainfo = (struct Ainfo*)calloc(natts,sizeof(struct Ainfo)))==NULL) {stat = NC_ENOMEM; goto done;}
        /* Compute the attribute types */
        if((stat = get_att_types(purezarr,jatts,ainfo))) goto done;

        /* Fill in the attributes */
        {
            for(i=0;i<natts;i++) {
                NCjson* jkey = NULL;
                NCjson* jvalues = NULL;
                jkey = NCJdictkey(jatts,i);
		nc_type typeid;
                assert(jkey != NULL && NCJisatomic(jkey));
                jvalues = NCJdictvalue(jatts,i);
		/*Special cases:*/
		if(strcmp(NC_ATT_FILLVALUE,NCJstring(jkey))==0) continue; /* _FillValue: ignore */
		else if(strcmp(NCZARR_DEFAULT_MAXSTRLEN_ATTR,NCJstring(jkey))==0) {
		    if(grp != NULL && grp->parent == NULL && NCJsort(jvalues)==NCJ_INT && ainfo[i].type == NC_INT) {
			size_t strlen;
			sscanf(NCJstring(jvalues),"%zu",&strlen);
			zdfaltstrlen(&zfile->default_maxstrlen,strlen);
		    }
		    /* Leave as attribute */
		} else if(strcmp(NCZARR_MAXSTRLEN_ATTR,NCJstring(jkey))==0) {
		    if(var != NULL && NCJsort(jvalues)==NCJ_INT && ainfo[i].type == NC_INT) {
			size_t strlen;
			sscanf(NCJstring(jvalues),"%zu",&strlen);
			zmaxstrlen(&zvar->maxstrlen,strlen);
		    }
		    /* Leave as attribute */
		} else if(strcmp(NC_XARRAY_DIMS,NCJstring(jkey))==0) {
		    continue; /* ignore */
		} else if(strncmp(NCJstring(jkey),NCZ_V2_PREFIX,strlen(NCZ_V2_PREFIX))==0)
		    continue; /* do not materialize any "_nczarr..." attributes */
		/* Convert jvalues to void* data */
		typeid = ainfo[i].type;
		if((stat = NCZ_computeattrdata(&typeid, jvalues, NULL, &len, &data))) goto done;
		/* Create the attribute */
		stat = ncz4_create_attr(file,container,NCJstring(jkey),typeid,len,data,NULL);
	        (void)NC_reclaim_data_all(file->controller,typeid,data,len);
		if(stat) goto done;
		data = NULL;
            }
        }
    }

    /* remember that we read the attributes */
    NCZ_setatts_read(container);

done:
    nullfree(ainfo);
    return ZUNTRACE(THROW(stat));
}

static int
ZF2_close(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    return THROW(stat);
}

/**************************************************/
/**
 * @internal Read superblock data from map to memory
 *
 * @param file Pointer to file struct
 * @param nczarrvp (out) the nczarr version
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_superblock(NC_FILE_INFO_T* file, const NCjson* jzsuper, int* nczarrvp)
{
    int stat = NC_NOERR;
    int nczarr_format = 0;
    const NCjson* jtmp = NULL;

    ZTRACE(3,"file=%s",file->controller->path);

    if(jzsuper != NULL) {
        /* in any case this is nczarr format 2 */
        nczarr_format = 2;
        /* See if superblock has version and (optionally) format */
        NCJcheck(NCJdictget(jzsuper,"version",&jtmp));
        if(jtmp == NULL) {stat = NC_ENCZARR; goto done;} /* Malformed */
        NCJcheck(NCJdictget(jzsuper,"format",&jtmp));
        if(jtmp != NULL)
            sscanf(NCJstring(jtmp),"%d",&nczarr_format);
    }
 
    if(nczarrvp) *nczarrvp = nczarr_format;
done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Read group data from map to memory
 *
 * @param file Pointer to file struct
 * @param parent Pointer to parent grp | NULL if creating root
 * @param name of the grp
 * @parem grpp return the created group
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

    /* Fill in the group */
    if((stat = read_grp_contents(file,grp))) goto done;

    if(grpp) {*grpp = grp; grp = NULL;}

done:
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Read group contents from map to memory
 *
 * @param file Pointer to file struct
 * @param grp Pointer to grp
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
read_grp_contents(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp)
{
    int stat = NC_NOERR;
    int ncv21 = 1;
    char* grppath = NULL;
    char* key = NULL;
    int purezarr = 0;
    NClist* dimdefs = nclistnew();
    NClist* varnames = nclistnew();
    NClist* subgrps = nclistnew();
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    int* dimids = NULL;
    NCjson* jgroup = NULL;
    NCjson* jatts = NULL;
    const NCjson* jzgroup = NULL;
    const NCjson* jzatts = NULL;
    const NCjson* jzsuper = NULL;
    
    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);

    purezarr = (zfile->flags & FLAG_PUREZARR);

    /* Read .zgroup (might be NULL) */
    if((stat = NCZ_grpkey(grp,&grppath))) goto done;
    if((stat = nczm_concat(grppath,Z2GROUP,&key))) goto done;
    stat=NCZ_downloadjson(zfile->map,key,&jgroup);
    nullfree(key); key = NULL;
    if(stat) goto done;

    /* Read .zattrs (might be NULL) */
    if((stat = nczm_concat(grppath,Z2ATTRS,&key))) goto done;
    stat=NCZ_downloadjson(zfile->map,key,&jatts);
    nullfree(key); key = NULL;
    if(stat) goto done;

    /* Extract jatts and _nczarr_XXX values */
    if((stat = locate_nczarr_grp_info(file, grp, jgroup, jatts, &jzgroup, &jzatts, &jzsuper, &ncv21))) goto done;

    if(jzsuper != NULL) {
        int nczarr_format = 0;
	if((stat = read_superblock(file,jzsuper,&nczarr_format))) goto done;
    }

    if(purezarr) {
        if((stat = parse_group_content_pure(file,grp,varnames,subgrps))) goto done;
    } else { /*!purezarr*/
        /* Pull out lists about group content */
        if((stat = parse_group_content(jzgroup,dimdefs,varnames,subgrps))) goto done;
    }

    /* build the attributes */
    if((stat = parse_attrs(file,(NC_OBJ*)grp,jatts))) goto done;

    /* Fill in the group recursively */

    if(nclistlength(dimdefs) > 0) {
        /* Define dimensions */
        if((stat = define_dims(file,grp,dimdefs,&dimids))) goto done;
    }
    
    /* Define vars taking xarray into account */
    if((stat = read_vars(file,grp,varnames))) goto done;

    /* Read sub-groups */
    if((stat = read_subgrps(file,grp,subgrps))) goto done;

done:
    NCZ_reclaim_diminfo_list(dimdefs);
    nclistfreeall(varnames);
    nclistfreeall(subgrps);
    nullfree(dimids);
    nullfree(grppath);
    nullfree(key);
    NCJreclaim(jgroup);
    NCJreclaim(jatts);		      
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Materialize dimensions into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param diminfo List of (name,length,isunlimited) triples
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
define_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* diminfo, int** dimidsp)
{
    int stat = NC_NOERR;
    size_t i, rank;
    int* dimids = NULL;

    ZTRACE(3,"file=%s grp=%s |diminfo|=%u",file->controller->path,grp->hdr.name,nclistlength(diminfo));

    assert(nclistlength(diminfo) > 0);
    rank = nclistlength(diminfo);
    if((dimids = (int*)calloc(rank,sizeof(int)))==NULL) {stat = NC_ENOMEM; goto done;}

    /* Reify each dim in turn */
    for(i = 0; i < rank; i++) {
	NCZ_DimInfo* dinfo = NULL;
        dinfo = (NCZ_DimInfo*)nclistget(diminfo,i);
	if((stat = define_dim1(file,grp,dinfo,&dimids[i]))) goto done;
    }
    if(dimidsp) {*dimidsp = dimids; dimids = NULL;}

done:
    nullfree(dimids);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Materialize a specific dimension into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param dimdata info about dimension to materialize
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
define_dim1(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCZ_DimInfo* dinfo, int* dimidp)
{
    int stat = NC_NOERR;
    int dimid = 0;
    NC_DIM_INFO_T* dim = NULL;

    ZTRACE(3,"file=%s grp=%s dimdata=%s",file->controller->path,grp->hdr.name,dinfo->name);

    if((stat = ncz4_create_dim(file,grp,dinfo->name,dinfo->shape,dinfo->unlimited,&dim))) goto done;
    dimid = dim->hdr.id;

    if(dimidp) *dimidp = dimid;

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
    int purezarr = 0;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NC_VAR_INFO_T* var = NULL;
    NCjson* jvar = NULL;
    const NCjson* jzarray = NULL;
    const NCjson* jzatts = NULL;
    int nczv21 = 1;
    const NCjson* jdimrefs = NULL;
    const NCjson* jvalue = NULL;
    NCjson* jatts = NULL;
    char* grppath = NULL;
    char* varpath = NULL;
    char* key = NULL;
    int suppress = 0; /* Abort processing of this variable */
    size_t vtypelen = 0;
    size_t zarr_rank = 0; /* == cvargs.rank + 1 if cvargs.scalar is true */
    nc_type atypeid = NC_NAT;
    size_t fvlen = 0;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    int varsized = 0;
    NClist* codecs = nclistnew(); /* NClist<const NCjson*> */
    const NCjson* jfilter = NULL;
#endif
    /* Capture dim info to make/find dims */
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

    /* Construct var path */
    if((stat = NCZ_grpkey(grp,&grppath))) goto done;
    if((stat = nczm_concat(grppath,varname,&varpath))) goto done;
    /* Construct the path to the zarray object */
    if((stat = nczm_concat(varpath,Z2ARRAY,&key))) goto done;
    /* Download the .zarray object */
    if((stat=NCZ_readdict(zfile->map,key,&jvar))) goto done;
    nullfree(key); key = NULL;
    if(jvar == NULL) {stat = NC_ENOTZARR; goto done;}
    /* Download the .zattrs object */
    /* Construct the path to .zattrs object */
    if((stat = nczm_concat(varpath,Z2ATTRS,&key))) goto done;
    if((stat=NCZ_readdict(zfile->map,key,&jatts))) goto done;
    nullfree(key); key = NULL;
    
    /* locate the _nczarr_XXX values */
    if((stat = locate_nczarr_array_info(file,grp,jvar,jatts,&jzarray,&jzatts,&nczv21))) goto done;

    /* Verify the format */
    {
        int version;
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
	zarr_rank = cvargs.rank;
        /* extract the shapes */
        if((stat = NCZ_decodesizet64vec(jvalue, cvargs.shapes))) goto done;
	cvargs.storage = NC_CHUNKED;
        /* Note: we detect and adjust for scalar later */
    }

    /* chunks */
    {
        NCJcheck(NCJdictget(jvar,"chunks",&jvalue));
        if(jvalue != NULL && NCJsort(jvalue) != NCJ_ARRAY)
            {stat = (THROW(NC_ENCZARR)); goto done;}
        /* Verify the rank */
        if(cvargs.rank != NCJarraylength(jvalue)) {stat = NC_ENCZARR; goto done;}
	/* get chunk sizes */
        if((stat = NCZ_decodesizet64vec(jvalue, cvargs.chunks))) goto done;
    }

    /* Set the type and endianness of the variable */
    {
        NCJcheck(NCJdictget(jvar,"dtype",&jvalue));
        /* Convert dtype to nc_type + endianness */
        if((stat = dtype2nctype(NCJstring(jvalue),&cvargs.vtype,&vtypelen,&cvargs.endianness))) goto done;
        if(cvargs.vtype == NC_STRING) {
            zmaxstrlen(&cvargs.maxstrlen,vtypelen);
            vtypelen = sizeof(char*); /* in-memory len */
            if(cvargs.maxstrlen == 0) {
	        zmaxstrlen(&cvargs.maxstrlen,NCZ_get_maxstrlen((NC_OBJ*)var));
	    }
        }
    }

    if(!purezarr) {
        /* Extract the NCZ_V2_ARRAY dict */
        if(jzarray == NULL) {stat = NC_ENCZARR; goto done;}
        assert((NCJsort(jzarray) == NCJ_DICT));
    }

    if(!purezarr) {/* Extract storage flag and adjust accordingly */
	NCJcheck(NCJdictget(jzarray,"storage",&jvalue));
	if(jvalue == NULL) {
	    cvargs.scalar = 0;
	} else if(strcmp(NCJstring(jvalue),"scalar")==0) {
	    cvargs.scalar = 1;
	}
        cvargs.storage = NC_CHUNKED; /* even for scalars */
    }
    
    /* Look for _ARRAY_DIMENSIONS attribute and save the dim names */
    if(!cvargs.scalar) {
        size_t i;
	if(jatts != NULL) {
	    const NCjson* jxarray = NULL;
	    NCJcheck(NCJdictget(jatts,NC_XARRAY_DIMS,&jxarray));
	    if((stat = collectxarraydims(jxarray,&cvargs,diminfo))) goto done;
	}
        /* fill in shape and unlimited */
        for(i=0;i<cvargs.rank;i++) {
	    diminfo[i].unlimited = 0;
	    diminfo[i].shape = cvargs.shapes[i];
        }
    }

    if(!purezarr) {
	/* set the fqns for the dims, if possible */
        assert(jzarray != NULL);
        assert((NCJsort(jzarray) == NCJ_DICT));
        /* Extract dimrefs list  */
        NCJcheck(NCJdictget(jzarray,"dimrefs",&jdimrefs));
	if(jdimrefs != NULL) {
            /* Extract the dimref FQNs and validate names*/
            assert((NCJsort(jdimrefs) == NCJ_ARRAY));
	    if((stat = collectdimrefs(jdimrefs,&cvargs,diminfo))) goto done;
	}
    }

    /* Complete the diminfo construction */
    if((stat = NCZ_computedimrefs(file, grp, cvargs.rank, cvargs.shapes, diminfo, &cvargs.scalar))) goto done;

    /* Capture dimension_separator (must precede chunk cache creation) */
    {
        NCglobalstate* ngs = NC_getglobalstate();
        assert(ngs != NULL);
        cvargs.dimension_separator = ngs->zarr.dimension_separator; /* use global value */
        NCJcheck(NCJdictget(jvar,"dimension_separator",&jvalue));
        if(jvalue != NULL) {
            /* Verify its value */
            if(NCJisatomic(jvalue) && NCJstring(jvalue) != NULL && strlen(NCJstring(jvalue)) == 1)
               cvargs.dimension_separator = NCJstring(jvalue)[0];
        }
        /* If value is invalid, then use global default */
        if(!islegaldimsep(cvargs.dimension_separator))
            cvargs.dimension_separator = DFALT_DIM_SEPARATOR_V2;
        assert(islegaldimsep(cvargs.dimension_separator)); /* we are hosed */
    }

    /* Capture row vs column major; currently, column major not used*/
    {
	char* sorder = "C";
        NCJcheck(NCJdictget(jvar,"order",&jvalue));
	if(jvalue != NULL)
	   sorder = NCJstring(jvalue);	    
	cvargs.order = sorder[0];
 	if(cvargs.order != 'C' && cvargs.order != 'F')
	   {stat = NC_ENOTZARR; goto done;}
	if(cvargs.order == 'F') suppress = 1; /* suppress this variable */
    }

    /* filters and compressor keys: merge into a single list */

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    {
        /* From V2 Spec: A list of JSON objects providing codec configurations,
           or null if no filters are to be applied. Each codec configuration
           object MUST contain a "id" key identifying the codec to be used. */
        /* Do filters key before compressor key so final filter chain is in correct order */
        NCJcheck(NCJdictget(jvar,"filters",&jvalue));
        if(jvalue != NULL && NCJsort(jvalue) != NCJ_NULL) {
            size_t k;
            if(NCJsort(jvalue) != NCJ_ARRAY) {stat = NC_EFILTER; goto done;}
            for(k=0;;k++) {
                jfilter = NCJith(jvalue,k);
		if(jfilter == NULL) break; /* done */
		if(NCJsort(jfilter) != NCJ_DICT) {stat = NC_EFILTER; goto done;}
		nclistpush(codecs,jfilter);
	    }
	}
    }

    /* add compressor key at the end */
    /* From V2 Spec: A JSON object identifying the primary compression codec and providing
       configuration parameters, or ``null`` if no compressor is to be used. */
    {
        if((stat = NCZ_filter_initialize())) goto done;
        NCJcheck(NCJdictget(jvar,"compressor",&jfilter));
        if(jfilter != NULL && NCJsort(jfilter) != NCJ_NULL) {
            if(NCJsort(jfilter) != NCJ_DICT) {stat = NC_EFILTER; goto done;}
   	    nclistpush(codecs,jfilter);
	}    
    }

    {
	size_t k;
	/* Compile all the filters+compressor */
        if((stat = NCZ_filter_initialize())) goto done;
        for(k=0;k<nclistlength(codecs);k++) {
	    NCZ_Filter* filter = NULL;
	    jfilter = nclistget(codecs,k);
            if((stat = json2filter(file,jfilter,&filter,cvargs.filterlist))) goto done;
        }
    }

    /* Suppress variable if there are filters and var is not fixed-size */
    if(varsized && nclistlength(cvargs.filterlist) > 0) suppress = 1;

#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/

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

    /* Make final scalar adjustments */
    if(!purezarr && cvargs.scalar) {
	NC_OBJ* obj = NULL;
	char* basename = NULL;
	/* Find the scalar dimension */
	if((stat = NCZ_locateFQN(file->root_grp,DIMSCALAR,NCDIM,&obj,&basename))) goto done;
	/* Verify */
	assert(obj != NULL && strcmp(basename,XARRAYSCALAR)==0);
	if((stat=NCZ_reclaim_dim((NC_DIM_INFO_T*)obj))) goto done;
        cvargs.rank = 0;
	cvargs.storage = NC_CONTIGUOUS;
	nullfree(basename);
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
					cvargs.scalar,
					cvargs.endianness,
					cvargs.maxstrlen,
					cvargs.dimension_separator,
					cvargs.order,
					cvargs.rank,
					cvargs.shapes,
					cvargs.chunks,
					cvargs.dimids,
					cvargs.filterlist,
					cvargs.no_fill,
					cvargs.fill_value,
					&var)))
	    goto done;

        if((stat = NCZ_adjust_var_cache(var))) goto done;

        /* Now that we have the variable, create its attributes */
        if((stat = parse_attrs(file,(NC_OBJ*)var,jatts))) goto done;
        
	/* Push the fill value attribute */
	if((stat = NCZ_copy_var_to_fillatt(file,var,NULL))) goto done;
    }

done:
    /* Clean up cvargs */
    nclistfree(cvargs.filterlist);
    if(cvargs.fill_value != NULL)
	(void)NC_reclaim_data_all(file->controller,atypeid,cvargs.fill_value,fvlen);
    /* Clean up */
    NCZ_clear_diminfo(zarr_rank,diminfo);
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    nclistfree(codecs);
#endif
    nullfree(grppath);
    nullfree(varpath);
    nullfree(key);
    NCJreclaim(jvar);
    NCJreclaim(jatts);
    return THROW(stat);
}

static int
collectdimrefs(const NCjson* jdimrefs, struct CVARGS* cvargs, NCZ_DimInfo* diminfo)
{
    int stat = NC_NOERR;
    NClist* segments = NULL;
    size_t j;

    if(jdimrefs == NULL) goto done;

    /* Extract the dimref FQNs */
    if(cvargs->scalar) {
        assert(NCJarraylength(jdimrefs) == 1);
	/* Verify that it is _scalar_ */
	NCjson* jscalar = NCJith(jdimrefs,0);
	assert(strcmp(NCJstring(jscalar),DIMSCALAR)==0);
	cvargs->rank = 1;
        diminfo[0].name = strdup(XARRAYSCALAR);
    } else {
        if(cvargs->rank != NCJarraylength(jdimrefs)) {stat = NC_ENCZARR; goto done;}
        for(j=0;j<cvargs->rank;j++) {
	    char* fqn = NULL;
	    char* name = NULL;
	    char* lastseg = NULL;
            const NCjson* dimpath = NCJith(jdimrefs,j);
            assert(NCJisatomic(dimpath));
	    assert(diminfo[j].fqn == NULL);
	    diminfo[j].fqn = strdup(NCJstring(dimpath));
	    fqn = diminfo[j].fqn;
	    name = diminfo[j].name;
	    /* Verify WRT simple dim names */
	    segments = nclistnew();
	    (void)ncz_splitkey(fqn,segments);
	    assert(nclistlength(segments) > 0);
	    lastseg = nclistget(segments,nclistlength(segments)-1);
	    if(name == NULL) name = strdup(lastseg); /* make consistent */
	    if(strcmp(name,lastseg) != 0) {stat = NC_ENCZARR; goto done;}
	    diminfo[j].name = name; /* save */		    
  	    nclistfreeall(segments); segments = NULL;
	}
    } /* else will simulate it from the shape of the variable */
done:
    nclistfreeall(segments);
    return THROW(stat);
}

static int
collectxarraydims(const NCjson* jxarray, struct CVARGS* cvargs, NCZ_DimInfo* diminfo)
{
    int stat = NC_NOERR;
    size_t i;
    const NCjson* jdimname = NULL;

    /* Compute the set of simple dim ref names for this variable from _ARRAY_DIMENSIONS */
    if(!cvargs->scalar) {
        if(jxarray != NULL) {
            assert(cvargs->rank == NCJarraylength(jxarray));
            for(i=0;i<cvargs->rank;i++) {
	        assert(diminfo[i].name == NULL);
                jdimname = NCJith(jxarray,i);
		assert(jdimname != NULL);
	        diminfo[i].name = strdup(NCJstring(jdimname));
	    }
	} /* else must be anonymous, so leave name NULL */
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
   1.1 the xarray "_ARRAY_DIMENSIONS" (V2) attribute as the simple dimension names.
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
static int
NCZ_computedimrefs(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp,
               size_t ndims,
	       size64_t* shape,
	       NCZ_DimInfo* diminfo,
	       int* isscalarp)
{
    int stat = NC_NOERR;
    size_t i;
    int isscalar;
    int purezarr;
    char digits[NC_MAX_NAME];
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCbytes* newname = ncbytesnew();
    NCbytes* fqn = ncbytesnew();
    
    ZTRACE(3,"file=%s var=%s purezarr=%d ndims=%d shape=%s",
        file->controller->path,var->hdr.name,purezarr,ndims,nczprint_vector(ndims,shape));

    assert(diminfo != NULL);

    purezarr = (zfile->flags & FLAG_PUREZARR)?1:0;

    /* Check for scalar */
    if(!purezarr && ndims == 1 && shape[0] == 1 && diminfo[0].name != NULL && strcmp(diminfo[0].name,XARRAYSCALAR) == 0) {
	isscalar = 1;
	/* Always create in root_grp */
	grp = file->root_grp;
    } else
        isscalar = 0;

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

    if(isscalarp) *isscalarp = (isscalar?1:0);

done:
    ncbytesfree(newname);
    ncbytesfree(fqn);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Materialize vars into memory;
 * Take xarray and purezarr into account.
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
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

    /* Load each var in turn */
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

    ZTRACE(3,"file=%s grp=%s |subgrpnames|=%u",file->controller->path,grp->hdr.name,nclistlength(subgrpnames));

    /* Load each subgroup name in turn */
    for(i = 0; i < nclistlength(subgrpnames); i++) {
        NC_GRP_INFO_T* g = NULL;
        const char* gname = nclistget(subgrpnames,i);
	if((stat = read_grp(file,parent,gname,&g))) goto done;
    }

done:
    return ZUNTRACE(THROW(stat));
}

/**************************************************/
/* Potentially shared functions */

/**
 * @internal Synchronize dimension data from memory to map.
 *
 * @param grp Pointer to grp struct containing the dims.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jdimsp)
{
    int stat=NC_NOERR;
    size_t i;
    NCjson* jdims = NULL;
    NCjson* jdimsize = NULL;
    NCjson* jdimargs = NULL;

    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);

    NCJcheck(NCJnew(NCJ_DICT,&jdims));
    for(i=0; i<ncindexsize(grp->dim); i++) {
        NC_DIM_INFO_T* dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
        char slen[128];

        snprintf(slen,sizeof(slen),"%llu",(unsigned long long)dim->len);
        NCJcheck(NCJnewstring(NCJ_INT,slen,&jdimsize));

        /* If dim is not unlimited, then write in the old format to provide
           maximum back compatibility.
        */
        if(dim->unlimited) {
            NCJcheck(NCJnew(NCJ_DICT,&jdimargs));
            NCJcheck(NCJaddstring(jdimargs,NCJ_STRING,"size"));
            NCJcheck(NCJappend(jdimargs,jdimsize));
            jdimsize = NULL;
            NCJcheck(NCJaddstring(jdimargs,NCJ_STRING,"unlimited"));
            NCJcheck(NCJaddstring(jdimargs,NCJ_INT,"1"));
        } else { /* !dim->unlimited */
            jdimargs = jdimsize;
            jdimsize = NULL;
        }
        NCJcheck(NCJaddstring(jdims,NCJ_STRING,dim->hdr.name));
        NCJcheck(NCJappend(jdims,jdimargs));
    }
    if(jdimsp) {*jdimsp = jdims; jdims = NULL;}

    NCJreclaim(jdims);
    return ZUNTRACE(THROW(stat));
}

static int
parse_group_content(const NCjson* jzgroup, NClist* dimdefs, NClist* varnames, NClist* subgrps)
{
    int stat = NC_NOERR;
    size_t i;
    const NCjson* jvalue = NULL;

    ZTRACE(3,"jcontent=|%s| |dimdefs|=%u |varnames|=%u |subgrps|=%u",NCJtotext(jcontent),(unsigned)nclistlength(dimdefs),(unsigned)nclistlength(varnames),(unsigned)nclistlength(subgrps));

    /* Now get nczarr specific keys */
    NCJcheck(NCJdictget(jzgroup,"dims",&jvalue));
    if(jvalue != NULL) {
        if(NCJsort(jvalue) != NCJ_DICT) {stat = (THROW(NC_ENCZARR)); goto done;}
        /* Extract the dimensions defined in this group */
        for(i=0;i<NCJdictlength(jvalue);i++) {
            const NCjson* jname = NCJdictkey(jvalue,i);
            const NCjson* jleninfo = NCJdictvalue(jvalue,i);
            const NCjson* jtmp = NULL;
            const char* slen = "0";
            const char* sunlim = "0";
            char norm_name[NC_MAX_NAME + 1];
            /* Verify name legality */
            if((stat = nc4_check_name(NCJstring(jname), norm_name)))
                {stat = NC_EBADNAME; goto done;}
            /* check the length */
            if(NCJsort(jleninfo) == NCJ_DICT) {
                NCJcheck(NCJdictget(jleninfo,"size",&jtmp));
                if(jtmp== NULL)
                    {stat = NC_EBADNAME; goto done;}
                slen = NCJstring(jtmp);
                /* See if unlimited */
                NCJcheck(NCJdictget(jleninfo,"unlimited",&jtmp));
                if(jtmp == NULL) sunlim = "0"; else sunlim = NCJstring(jtmp);
            } else if(jleninfo != NULL && NCJsort(jleninfo) == NCJ_INT) {
                slen = NCJstring(jleninfo);
            } else
                {stat = NC_ENCZARR; goto done;}
	    NCZ_DimInfo* dinfo = (NCZ_DimInfo*)calloc(1,sizeof(NCZ_DimInfo));
	    if(dinfo == NULL) {stat = NC_ENOMEM; goto done;}
	    dinfo->name = strdup(norm_name);
	    sscanf(slen,"%llu",&dinfo->shape);
    	    sscanf(sunlim,"%d",&dinfo->unlimited);
	    if(dinfo->unlimited) dinfo->unlimited = 1; else dinfo->unlimited = 0; /*cleanup*/
	    nclistpush(dimdefs,dinfo);
	    dinfo = NULL;
        }
    }

    NCJcheck(NCJdictget(jzgroup,"vars",&jvalue));
    if(jvalue != NULL) {
        /* Extract the variable names in this group */
        for(i=0;i<NCJarraylength(jvalue);i++) {
            NCjson* jname = NCJith(jvalue,i);
            char norm_name[NC_MAX_NAME + 1];
            /* Verify name legality */
            if((stat = nc4_check_name(NCJstring(jname), norm_name)))
                {stat = NC_EBADNAME; goto done;}
            nclistpush(varnames,strdup(norm_name));
        }
    }

    NCJcheck(NCJdictget(jzgroup,"groups",&jvalue));
    if(jvalue != NULL) {
        /* Extract the subgroup names in this group */
        for(i=0;i<NCJarraylength(jvalue);i++) {
            NCjson* jname = NCJith(jvalue,i);
            char norm_name[NC_MAX_NAME + 1];
            /* Verify name legality */
            if((stat = nc4_check_name(NCJstring(jname), norm_name)))
                {stat = NC_EBADNAME; goto done;}
            nclistpush(subgrps,strdup(norm_name));
        }
    }

done:
    return ZUNTRACE(THROW(stat));
}

static int
parse_group_content_pure(NC_FILE_INFO_T*  file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrps)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    ZTRACE(3,"zfile=%s grp=%s |varnames|=%u |subgrps|=%u",zfile->common.file->controller->path,grp->hdr.name,(unsigned)nclistlength(varnames),(unsigned)nclistlength(subgrps));

    nclistclear(varnames);
    if((stat = searchvars(zfile,grp,varnames))) goto done;
    nclistclear(subgrps);
    if((stat = searchsubgrps(zfile,grp,subgrps))) goto done;

done:
    return ZUNTRACE(THROW(stat));
}

static int
searchvars(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* varnames)
{
    int stat = NC_NOERR;
    size_t i;
    char* grpkey = NULL;
    char* varkey = NULL;
    char* zarray = NULL;
    NClist* matches = nclistnew();

    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(grp,&grpkey))) goto done;
    /* Get the map and search group */
    if((stat = nczmap_list(zfile->map,grpkey,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
        const char* name = nclistget(matches,i);
        if(name[0] == NCZM_DOT) continue; /* zarr/nczarr specific */
        /* See if name/.zarray exists */
        if((stat = nczm_concat(grpkey,name,&varkey))) goto done;
        if((stat = nczm_concat(varkey,Z2ARRAY,&zarray))) goto done;
        switch(stat = nczmap_exists(zfile->map,zarray)) {
	case NC_NOERR: nclistpush(varnames,strdup(name)); break;
	case NC_ENOOBJECT: stat = NC_NOERR; break; /* ignore */
	default: goto done;
        }
        nullfree(varkey); varkey = NULL;
        nullfree(zarray); zarray = NULL;
    }

done:
    nullfree(grpkey);
    nullfree(varkey);
    nullfree(zarray);
    nclistfreeall(matches);
    return stat;
}

static int
searchsubgrps(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* subgrpnames)
{
    int stat = NC_NOERR;
    size_t i;
    char* grpkey = NULL;
    char* subkey = NULL;
    char* zgroup = NULL;
    NClist* matches = nclistnew();

    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(grp,&grpkey))) goto done;
    /* Get the map and search group */
    if((stat = nczmap_list(zfile->map,grpkey,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
        const char* name = nclistget(matches,i);
        if(name[0] == NCZM_DOT) continue; /* zarr/nczarr specific */
        /* See if name/.zgroup exists */
        if((stat = nczm_concat(grpkey,name,&subkey))) goto done;
        if((stat = nczm_concat(subkey,Z2GROUP,&zgroup))) goto done;
        switch(stat = nczmap_exists(zfile->map,zgroup)) {
	case NC_NOERR: nclistpush(subgrpnames,strdup(name)); break;
	case NC_ENOOBJECT: stat = NC_NOERR; break;
	default: goto done;
	}
        nullfree(subkey); subkey = NULL;
        nullfree(zgroup); zgroup = NULL;
    }

done:
    nullfree(grpkey);
    nullfree(subkey);
    nullfree(zgroup);
    nclistfreeall(matches);
    return stat;
}

static int
ZF2_buildchunkkey(size_t rank, const size64_t* chunkindices, char dimsep, char** keyp)
{
    int stat = NC_NOERR;
    size_t r;
    NCbytes* key = ncbytesnew();

    if(keyp) *keyp = NULL;

    assert(islegaldimsep(dimsep));

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

/**************************************************/
/* Format Filter Support Functions */

/* JSON Parse/unparse of filter codecs */

int
ZF2_hdf2codec(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Convert the HDF5 id + visible parameters to the codec form */

    /* Clear any previous codec */
    nullfree(filter->codec.id); filter->codec.id = NULL;
    nullfree(filter->codec.codec); filter->codec.codec = NULL;
    filter->codec.id = strdup(filter->plugin->codec.codec->codecid);
    if(filter->plugin->codec.codec->NCZ_hdf5_to_codec) {
        stat = filter->plugin->codec.codec->NCZ_hdf5_to_codec(NCplistzarrv2,filter->hdf5.id,filter->hdf5.visible.nparams,filter->hdf5.visible.params,&filter->codec.codec);
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
@internal Zarr V2: Given an nc_type+endianness+purezarr+MAXSTRLEN, produce the corresponding Zarr type string.
@param nctype     - [in] nc_type
@param endianness - [in] endianness
@param purezarr   - [in] 1=>pure zarr, 0 => nczarr
@param len        - [in] max string length
@param dnamep     - [out] pointer to hold pointer to the dtype; user frees
@return NC_NOERR
@return NC_EINVAL
@author Dennis Heimbigner
*/

int
nctype2dtype(nc_type nctype, int endianness, int purezarr, size_t len, char** dnamep, const char** tagp)
{
    char dname[64];
    const char* dtype = NULL;

    if(nctype <= NC_NAT || nctype > N_NCZARR_TYPES) return NC_EINVAL;
    dtype = znamesv2[nctype].dtype;
    snprintf(dname,sizeof(dname),dtype,len);
    /* Set endianness */
    switch (nctype) {
    case NC_STRING:
    case NC_CHAR:
    case NC_JSON:
	break;    
    default:
	switch (endianness) {
	case NC_ENDIAN_LITTLE: dname[0] = '<'; break;
	case NC_ENDIAN_BIG: dname[0] = '>'; break;
	case NC_ENDIAN_NATIVE: default: break;
	}
    }
    if(dnamep) *dnamep = strdup(dname);
    return NC_NOERR;		
}

/*
@internal Convert a numcodecs Zarr v2 dtype spec to a corresponding nc_type.
@param nctype   - [in] dtype the dtype to convert
@param typehint - [in] typehint available if type comes from _nczarr_attr
@param purezarr - [in] 1=>pure zarr, 0 => nczarr
@param nctypep  - [out] hold corresponding type
@param endianp  - [out] hold corresponding endianness
@param typelenp - [out] hold corresponding type size (for fixed length strings)
@return NC_NOERR
@return NC_EINVAL
@author Dennis Heimbigner
*/

static int
dtype2nctype(const char* dtype, nc_type* nctypep, size_t* typelenp, int* endianp)
{
    int stat = NC_NOERR;
    size_t typelen = 0;
    char tchar;
    nc_type nctype = NC_NAT;
    int endianness = -1;
    const char* p;
    int n,count;

    if(endianp) *endianp = NC_ENDIAN_NATIVE;
    if(nctypep) *nctypep = NC_NAT;
    if(typelenp) *typelenp = 0;

    if(dtype == NULL) {stat = NC_ENCZARR; goto done;}

    /* Handle special cases */
    if(strcmp(dtype,"|J0")==0) {
        nctype = NC_JSON;
	typelen = 1;
	goto exit;
    } else if(strcmp(dtype,">S1")==0) {
	nctype = NC_CHAR;
	typelen = 1;
	goto exit;
    } else if(memcmp(dtype,"|S",2)==0) {
	nctype = NC_STRING;
	sscanf(dtype,"|S%zu",&typelen);
	goto exit;
    }

    /* Parse the dtype; should be a numeric type */
    p = dtype;
    switch (*p++) {
    case '<': endianness = NC_ENDIAN_LITTLE; break;
    case '>': endianness = NC_ENDIAN_BIG; break;
    case '|': endianness = NC_ENDIAN_NATIVE; break;
    default: p--; endianness = NC_ENDIAN_NATIVE; break;
    }
    tchar = *p++; /* get the base type */
    /* Decode the type length */
    count = sscanf(p,"%zu%n",&typelen,&n);
    if(count == 0) {stat = NC_ENCZARR; goto done;}
    p += n;

    /* Numeric cases */
    switch(typelen) {
    case 1:
        switch (tchar) {
        case 'i': nctype = NC_BYTE; break;
        case 'u': nctype = NC_UBYTE; break;
        default: {stat = NC_ENCZARR; goto done;}
        }
        break;
    case 2:
        switch (tchar) {
        case 'i': nctype = NC_SHORT; break;
        case 'u': nctype = NC_USHORT; break;
        default: {stat = NC_ENCZARR; goto done;}
        }
        break;
    case 4:
        switch (tchar) {
        case 'i': nctype = NC_INT; break;
        case 'u': nctype = NC_UINT; break;
        case 'f': nctype = NC_FLOAT; break;
        default: {stat = NC_ENCZARR; goto done;}
        }
        break;
    case 8:
        switch (tchar) {
        case 'i': nctype = NC_INT64; break;
        case 'u': nctype = NC_UINT64; break;
        case 'f': nctype = NC_DOUBLE; break;
        default: {stat = NC_ENCZARR; goto done;}
        }
        break;
    default: {stat = NC_ENCZARR; goto done;}
    }

exit:
    if(nctypep) *nctypep = nctype;
    if(typelenp) *typelenp = typelen;
    if(endianp) *endianp = endianness;

done:
    return stat;
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
    if(NCJdictget(jfilter,"id",&jvalue)<0) {stat = NC_EFILTER; goto done;}
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
            if((stat = plugin->codec.codec->NCZ_codec_to_hdf5(NCplistzarrv2,codec.codec,&hdf5.id,&hdf5.visible.nparams,&hdf5.visible.params))) goto done;
#ifdef DEBUGF
            fprintf(stderr,">>> DEBUGF: json2filter: codec=%s, hdf5=%s\n",printcodec(codec),printhdf5(hdf5));
#endif
        }
    } /* else plugin == NULL || plugin->incomplete */

    /* Create the filter */
    if((stat = ncz4_create_filter(file,&hdf5,&codec,filterlist,&filter))) goto done;
    if(zfilterp) *zfilterp = filter;
    filter = NULL;

done:
    NCZ_filter_hdf5_clear(&hdf5);
    NCZ_filter_codec_clear(&codec);    
    NCZ_filter_free(filter);
    return THROW(stat);
}
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/

/**************************************************/
/* Format Dispatch table */

static const NCZ_Formatter NCZ_formatter2_table = {
    NCZARRFORMAT2,
    ZARRFORMAT2,
    NCZ_FORMATTER_VERSION,

    ZF2_create,
    ZF2_open,
    ZF2_close,
    ZF2_readmeta,
    ZF2_writemeta,
    ZF2_hdf2codec,
    ZF2_buildchunkkey
};

const NCZ_Formatter* NCZ_formatter2 = &NCZ_formatter2_table;

int
NCZF2_initialize(void)
{
    return NC_NOERR;
}

int
NCZF2_finalize(void)
{
    return NC_NOERR;
}

/**************************************************/
/* NCZarr V2 Attributed Support */

/**
Given a .zgroup content and the corresponding .zattrs (possibly NULL),
Locate and return the values for:
* "_nczarr_superblock"
* "_nczarr_group"
* "_nczarr_attrs"
Some may be NULL.

@param file	[in]  File object
@param grp	[in]  Group object
@param jgroup	[in]  .zgroup contents
@param jatts	[in]  corresponding .zattrs contents
@param jzgroupp	[out] _nczarr_group contents
@param jzattsp	[out] _nczarr_attrs contents
@param jzsuperp	[out] _nczarr_superblock contents (if root group)
@param nczv21p	[out] return 0 if this NCZARR 2.0.0 else 1.
@return NC_NOERR || NC_EXXX error
*/
static int
locate_nczarr_grp_info(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson* jgroup, NCjson* jatts,
			const NCjson** jzgroupp, const NCjson** jzattsp, const NCjson** jzsuperp, int* nczv21p)
{
    int stat = NC_NOERR;
    int nczv21 = 0;
    const NCjson* jzgroup = NULL;
    const NCjson* jzatts = NULL;
    const NCjson* jzsuper = NULL;

    if(jatts != NULL) { /* Look in jatts first */
        if(grp->parent == NULL) { /* Root group, look for superblock */
            NCJcheck(NCJdictget(jatts,NCZ_V2_SUPERBLOCK,&jzsuper));
	}
        NCJcheck(NCJdictget(jatts,NCZ_V2_ATTR,&jzatts));
        NCJcheck(NCJdictget(jatts,NCZ_V2_GROUP,&jzgroup));
	nczv21 = 1;
    }
    if(jgroup != NULL) { /* Try jgroup */
        if(jzsuper == NULL && grp->parent == NULL) { /* Root group, look for superblock */
            NCJcheck(NCJdictget(jgroup,NCZ_V2_SUPERBLOCK,&jzsuper));
	}
        if(jzatts == NULL) NCJcheck(NCJdictget(jgroup,NCZ_V2_ATTR,&jzatts));
        if(jzgroup == NULL) NCJcheck(NCJdictget(jgroup,NCZ_V2_GROUP,&jzgroup));
    }

    if(jzgroupp) *jzgroupp = jzgroup;
    if(jzattsp) *jzattsp = jzatts;
    if(jzsuperp) *jzsuperp = jzsuper;
    if(nczv21p) *nczv21p = nczv21;

    return THROW(stat);
}

/**
Given a .zarray content and the corresponding .zattrs (possibly NULL),
Locate and return the values for:
* "_nczarr_array"
* "_nczarr_attrs"
Some may be NULL.

@param file	[in]  File object
@param grp	[in]  Array object
@param jarray	[in]  .zarray contents
@param jatts	[in]  corresponding .zattrs contents
@param jzarrayp	[out] _nczarr_array contents
@param jzattsp	[out] _nczarr_attrs contents
@param nczv21p	[out] return 0 if this NCZARR 2.0.0 else 1.
@return NC_NOERR || NC_EXXX error
*/
static int
locate_nczarr_array_info(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson* jarray, NCjson* jatts,
			const NCjson** jzarrayp, const NCjson** jzattsp, int* nczv21p)
{
    int stat = NC_NOERR;
    int nczv21 = 0;
    const NCjson* jzarray = NULL;
    const NCjson* jzatts = NULL;

    if(jatts != NULL) { /* Look in jatts first */
        NCJcheck(NCJdictget(jatts,NCZ_V2_ATTR,&jzatts));
        NCJcheck(NCJdictget(jatts,NCZ_V2_ARRAY,&jzarray));
	nczv21 = 1;
    }
    if(jarray != NULL) { /* Try jarray */
        if(jzatts == NULL) NCJcheck(NCJdictget(jarray,NCZ_V2_ATTR,&jzatts));
        if(jzarray == NULL) NCJcheck(NCJdictget(jarray,NCZ_V2_ARRAY,&jzarray));
    }    

    if(jzarrayp) *jzarrayp = jzarray;
    if(jzattsp) *jzattsp = jzatts;
    if(nczv21p) *nczv21p = nczv21;
    
    return THROW(stat);
}

/**
@internal Extract attribute types for _nzarr_attrs
and return them as a list. If _nczarr_attrs does not exist,
then infer them.
@param purezarr - [in] zarr vs nczarr
@param natts - [in] number of atts
@param jatts - [in] the json for attributes
@param ainfo - [in/out] per-attribute type+endianness+length
@return NC_NOERR||NC_EXXX
*/

static int
get_att_types(int purezarr, NCjson* jatts, struct Ainfo* ainfo)
{
    int stat = NC_NOERR;
    nc_type typeid;
    int endianness;
    size_t typelen;
    size_t natts = 0;
    const NCjson* jzatts = NULL;
    const NCjson* jtypes = NULL;

    if(jatts == NULL) goto done; /* nothing to do */
    if(jatts != NULL) natts = NCJdictlength(jatts);

    if(!purezarr) {
        /* Get _nczarr_attrs from .zattrs */
        NCJcheck(NCJdictget(jatts,NCZ_V2_ATTR,&jzatts));
        /* Get types list */
        if(jzatts != NULL) {
	    NCJcheck(NCJdictget(jzatts,"types",&jtypes));
	}
    }

    if(jtypes != NULL) {
	size_t i;
	const NCjson* jtype;
	if(natts != NCJdictlength(jtypes)) {stat = NC_ENCZARR; goto done;}
	for(i=0;i<natts;i++) {
	    NCjson* akey = NCJdictkey(jatts,i); /* get attr name */
	    NCJcheck(NCJdictget(jtypes,NCJstring(akey),&jtype)); /* get corresponding type */
	    if(jtype == NULL) {stat = NC_ENCZARR; goto done;} /* fail if there is no type */	
	    /* Convert to nc_type */
	    if((stat = dtype2nctype(NCJstring(jtype),&typeid,&typelen,&endianness))) goto done;
	    ainfo[i].type = typeid;
	    ainfo[i].endianness = endianness;
    	    ainfo[i].typelen = typelen;
	}
    } else { /* Infer the types */
	size_t i;
	for(i=0;i<natts;i++) {
	    size_t typelen;
    	    NCjson* avalues = NCJdictvalue(jatts,i); /* get attr value */
	    /* Infer the type */
	    if((stat = NCZ_inferattrtype(avalues, NC_NAT, &typeid))) goto done;
	    ainfo[i].type = typeid;
    	    ainfo[i].endianness = (NCZ_isLittleEndian()?NC_ENDIAN_LITTLE:NC_ENDIAN_BIG);
	    if(typeid == NC_JSON)
	        typelen = 1;
	    else
	        {if((stat = NC4_inq_atomic_type(typeid,NULL,&typelen))) goto done;}
	    ainfo[i].typelen = typelen;
	}
    }

done:
    return THROW(stat);
}
