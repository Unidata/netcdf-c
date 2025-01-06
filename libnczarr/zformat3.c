/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 *
 * @author Dennis Heimbigner
 */

#include "zincludes.h"
#include "zplugins.h"
#include "znc4.h"
#include "zfill.h"
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
#include "netcdf_filter_build.h"
#endif

/**************************************************/

/*Mnemonics*/
#define STRTEMPLATE "r%zu"

/**************************************************/
/* Big endian Bytes filter */
static const char* NCZ_Bytes_Big_Text = "{\"name\": \"bytes\", \"configuration\": {\"endian\": \"big\"}}";
NCjson* NCZ_Bytes_Big_Json = NULL;

/* Little endian Bytes filter */
static const char* NCZ_Bytes_Little_Text = "{\"name\": \"bytes\", \"configuration\": {\"endian\": \"little\"}}";
NCjson* NCZ_Bytes_Little_Json = NULL;

/**************************************************/
/* Forward */

static int ZF3_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
static int ZF3_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
static int ZF3_close(NC_FILE_INFO_T* file);
static int ZF3_download_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj);
static int ZF3_download_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj);
static int ZF3_decode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* jgroup, NCjson** jzgrpp, NCjson** jzsuperp);
static int ZF3_decode_superblock(NC_FILE_INFO_T* file, const NCjson* jsuper, int* zarrformat, int* nczarrformat);
static int ZF3_decode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jnczgrp, NClist* vars, NClist* subgrps, NClist* dimdefs);
static int ZF3_decode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj, NClist* jfilters, size64_t** shapep, size64_t** chunksp, NClist* dimrefs);
static int ZF3_decode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts);
static int decode_var_dimrefs(NC_FILE_INFO_T* file, size_t rank, size64_t* shapes, const NCjson* jvar, NClist* dimrefs);
static int ZF3_upload_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj);
static int ZF3_upload_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj);
static int ZF3_encode_superblock(NC_FILE_INFO_T* file, NCjson** jsuperp);
static int ZF3_encode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jzgroupp);
static int ZF3_encode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jgroupp);
static int ZF3_encode_nczarr_array(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson** jzvarp);
static int ZF3_encode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj, NCjson** jvarp);
static int ZF3_encode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson** jnczconp, NCjson** jsuperp, NCjson** jattsp);
static int ZF3_encode_filter(NC_FILE_INFO_T* file, NCZ_Filter* filter, NCjson** jfilterp);
static int ZF3_decode_filter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson* jfilter, NCZ_Filter* filter);
static int ZF3_searchobjects(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrpnames);
static int ZF3_encode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);
static int ZF3_decode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const char* chunkname, size_t* rankp, size64_t** chunkindicesp);
static int ZF3_encode_xarray(NC_FILE_INFO_T* file, size_t rank, NC_DIM_INFO_T** dims, char** xarraydims, size_t* zarr_rankp);
static char ZF3_default_dimension_separator(NC_FILE_INFO_T* file);

static int decode_dim_decls(NC_FILE_INFO_T* file, const NCjson* jdims, NClist* dimdefs);
static int dtype2nctype(const char* dtype, const char* dnczarr, nc_type* nctypep, size_t* typelenp);
static int nctype2dtype(nc_type nctype, size_t strlen, char** dtypep, char** dnczarrp);
static int computeattrinfo(NC_FILE_INFO_T* file, NC_OBJ* container, const char* aname, const NCjson* jtypes, const NCjson* jdata, struct NCZ_AttrInfo* ainfo);
static NCjson* build_named_config(const char* name, int pairs, ...);
static int extract_named_config(const NCjson* jpair, const char* field, const NCjson** jvaluep);

/**************************************************/
/* Format dispatch table */

static const NCZ_Formatter NCZ_formatter3_table =
{
    NCZARRFORMAT3,
    ZARRFORMAT3,
    NCZ_FORMATTER_VERSION,

    /*File-Level Operations*/
    ZF3_create,
    ZF3_open,
    ZF3_close,

    /*Read JSON Metadata*/
    ZF3_download_grp,
    ZF3_download_var,

    ZF3_decode_group,
    ZF3_decode_superblock,
    ZF3_decode_nczarr_group,
    ZF3_decode_var,
    ZF3_decode_attributes,

    /*Write JSON Metadata*/
    ZF3_upload_grp,
    ZF3_upload_var,

    ZF3_encode_superblock,
    ZF3_encode_nczarr_group,
    ZF3_encode_group,

    ZF3_encode_nczarr_array,
    ZF3_encode_var,

    ZF3_encode_attributes,

    /*Filter Processing*/
    ZF3_encode_filter,
    ZF3_decode_filter,

    /*Search*/
    ZF3_searchobjects,

    /*Chunkkeys*/
    ZF3_encode_chunkkey,
    ZF3_decode_chunkkey,

   /*_ARRAY_DIMENSIONS*/
   ZF3_encode_xarray,

   /* Per-format default dimension separator */
   ZF3_default_dimension_separator,
};

const NCZ_Formatter* NCZ_formatter3 = &NCZ_formatter3_table;

int
NCZF3_initialize(void)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;
    NCJcheck(NCJparse(NCZ_Bytes_Little_Text,0,&json));
    NCZ_Bytes_Little_Json = json;
    NCJcheck(NCJparse(NCZ_Bytes_Big_Text,0,&json));
    NCZ_Bytes_Big_Json = json;
done:
    return THROW(stat);
}

int
NCZF3_finalize(void)
{
    return NC_NOERR;
}

/**************************************************/

/*File-Level Operations*/

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

    NC_UNUSED(uri);
    NC_UNUSED(map);
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

    NC_UNUSED(uri);
    NC_UNUSED(map);
    ZTRACE(4,"file=%s",file->controller->path);
    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    return ZUNTRACE(THROW(stat));
}

int
ZF3_close(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NC_UNUSED(file);
    return THROW(stat);
}

/**************************************************/

/*Dowload JSON Metadata*/
int
ZF3_download_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* key = NULL;

    /* Download zarr.json */
    if((stat = NCZ_grpkey(grp,&fullpath))) goto done;
    if((stat = nczm_concat(fullpath,Z3GROUP,&key))) goto done;
    if((stat = NCZMD_fetch_json_content(file,NCZMD_GROUP,key,&zobj->jobj))) goto done;
    nullfree(key); key = NULL;
    /* Verify that group zarr.json exists */
    if(zobj->jobj == NULL) {stat = NC_ENOTZARR; goto done;}
    /* For V3, the attributes are part of the grp zarr.json */
    NCJcheck(NCJdictget(zobj->jobj,"attributes",&zobj->jatts));
    zobj->constjatts = 1;

done:
    nullfree(key);
    nullfree(fullpath);
    return THROW(stat);
}

int
ZF3_download_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* key = NULL;

    /* Download zarr.json */
    if((stat = NCZ_varkey(var,&fullpath))) goto done;
    if((stat = nczm_concat(fullpath,Z3ARRAY,&key))) goto done;
    if((stat = NCZMD_fetch_json_content(file,NCZMD_ARRAY,key,&zobj->jobj))) goto done;
    nullfree(key); key = NULL;
    /* Verify that var zarr.json exists */
    if(zobj->jobj == NULL) {stat = NC_ENOTZARR; goto done;}
    /* For V3, the attributes are part of the var zarr.json */
    NCJcheck(NCJdictget(zobj->jobj,"attributes",&zobj->jatts));
    zobj->constjatts = 1;

done:
    nullfree(key);
    nullfree(fullpath);
    return THROW(stat);
}

int
ZF3_decode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj, NCjson** jzgrpp, NCjson** jzsuperp)
{
    int stat = NC_NOERR;
    NCjson* jzgrp = NULL;
    NCjson* jzsuper = NULL;
    const NCjson* jvalue = NULL;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    
    NC_UNUSED(grp);
    /* Verify the format */
    {
	const NCjson* jvar = zobj->jobj;
	int format;
	NCJcheck(NCJdictget(jvar,"node_type",(NCjson**)&jvalue));
	assert(jvalue != NULL);
	if(strcasecmp("group",NCJstring(jvalue))!=0) {stat = THROW(NC_ENOTZARR); goto done;}
	NCJcheck(NCJdictget(jvar,"zarr_format",(NCjson**)&jvalue));
	sscanf(NCJstring(jvalue),ZARR_FORMAT_VERSION_TEMPLATE,&format);
	if(format != zinfo->zarr.zarr_format) {stat = (THROW(NC_ENCZARR)); goto done;}
    }

    if(zobj->jatts != NULL) {
	/* Extract _nczarr_group from zobj->attr */
	NCJcheck(NCJdictget(zobj->jatts,NC_NCZARR_GROUP_ATTR,&jzgrp));
	/* Extract _nczarr_superblock from zobj->attr */
	NCJcheck(NCJdictget(zobj->jatts,NC_NCZARR_SUPERBLOCK_ATTR,&jzsuper));
    }
    if(jzgrpp != NULL) *jzgrpp = jzgrp;
    if(jzsuperp != NULL) *jzsuperp = jzsuper;

done:
    return THROW(stat);
}

int
ZF3_decode_superblock(NC_FILE_INFO_T* file, const NCjson* jsuper, int* zformatp, int* nczformatp)
{
    int stat = NC_NOERR;
    const NCjson* format = NULL;
    int zformat = 0;
    int nczformat = 0;

    NC_UNUSED(file);
    assert(jsuper != NULL);
    
    if(zformatp) *zformatp = 0;
    if(nczformatp) *nczformatp = 0;
    
    /* Extract the zarr format number and the nczarr format number */
    NCJcheck(NCJdictget(jsuper,"zarr_format",(NCjson**)&format));
    if(format != NULL) {
	if(NCJsort(format) != NCJ_INT) {stat = NC_ENOTZARR; goto done;}
	if(1!=sscanf(NCJstring(format),ZARR_FORMAT_VERSION_TEMPLATE,&zformat)) {stat = NC_ENOTZARR; goto done;}
    }
    NCJcheck(NCJdictget(jsuper,"nczarr_format",(NCjson**)&format));
    if(format != NULL) {
	if(NCJsort(format) != NCJ_INT) {stat = NC_ENOTZARR; goto done;}
	if(1!=sscanf(NCJstring(format),NCZARR_FORMAT_VERSION_TEMPLATE,&nczformat)) {stat = NC_ENOTZARR; goto done;}
    }
    
    if(zformatp) *zformatp = zformat;
    if(nczformatp) *nczformatp = nczformat;

done:
    return THROW(stat);
}

int
ZF3_decode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jnczgrp, NClist* vars, NClist* subgrps, NClist* dimdefs)
{
    int stat = NC_NOERR;
    size_t i;
    const NCjson* jvalue = NULL;

    NC_UNUSED(grp);

    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);

    NCJcheck(NCJdictget(jnczgrp,"dimensions",(NCjson**)&jvalue));
    if(jvalue != NULL) {
	if(NCJsort(jvalue) != NCJ_DICT) {stat = (THROW(NC_ENCZARR)); goto done;}
	/* Decode the dimensions defined in this group */
	if((stat = decode_dim_decls(file,jvalue,dimdefs))) goto done;
    }

    NCJcheck(NCJdictget(jnczgrp,"arrays",(NCjson**)&jvalue));
    if(jvalue != NULL) {
	/* Extract the variable names in this group */
	for(i=0;i<NCJarraylength(jvalue);i++) {
	    NCjson* jname = NCJith(jvalue,i);
	    char norm_name[NC_MAX_NAME + 1];
	    /* Verify name legality */
	    if((stat = nc4_check_name(NCJstring(jname), norm_name)))
		{stat = NC_EBADNAME; goto done;}
	    nclistpush(vars,strdup(norm_name));
	}
    }

    NCJcheck(NCJdictget(jnczgrp,"groups",(NCjson**)&jvalue));
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

int
ZF3_decode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj, NClist* filtersj, size64_t** shapesp, size64_t** chunksp, NClist* dimrefs)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    /* per-variable info */
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    const NCjson* jvar = NULL;
    const NCjson* jvalue = NULL;
    const NCjson* jendian = NULL;
    const NCjson* jcodecs = NULL;
    const NCjson* jchunkgrid = NULL;
    const NCjson* jchunkkey = NULL;
    const NCjson* jdtype = NULL;
    const NCjson* jdnczarr = NULL;
    const NCjson* jnczarray = NULL;
    int varsized = 0;
    int suppress = 0; /* Abort processing of this variable */
    nc_type vtype = NC_NAT;
    size_t maxstrlen = 0;
    size_t zarr_rank = 0; /* |shape| */
    int endianness = NC_ENDIAN_NATIVE;
    struct NCZ_AttrInfo ainfo = NCZ_emptyAttrInfo();
    size64_t* shapes = NULL;
    size64_t* chunks = NULL;

    jvar = zobj->jobj;
    assert(jvar != NULL);

    /* Verify the format */
    {
	int format;
	NCJcheck(NCJdictget(jvar,"node_type",(NCjson**)&jvalue));
	assert(jvalue != NULL);
	if(strcasecmp("array",NCJstring(jvalue))!=0) {stat = THROW(NC_ENOTZARR); goto done;}
	NCJcheck(NCJdictget(jvar,"zarr_format",(NCjson**)&jvalue));
	sscanf(NCJstring(jvalue),ZARR_FORMAT_VERSION_TEMPLATE,&format);
	if(format != zinfo->zarr.zarr_format) {stat = (THROW(NC_ENCZARR)); goto done;}
    }

    /* Get _nczarr_array */
    if(zobj->jatts != NULL) {
	NCJcheck(NCJdictget(zobj->jatts,NC_NCZARR_ARRAY_ATTR,(NCjson**)&jnczarray));
    }
    
    {
	const char* dtype = NULL;
	const char* dnczarr = NULL;
	/* Get the standard dtype of the variable */
	NCJcheck(NCJdictget(jvar,"data_type",(NCjson**)&jdtype));
	if(jdtype == NULL || NCJsort(jdtype) != NCJ_STRING) {stat = NC_ENOTZARR; goto done;}
	dtype = NCJstring(jdtype);
	/* Get the dtype override from _nczarr_array */
	if(jnczarray != NULL) {
	    NCJcheck(NCJdictget(jnczarray,"nczarr_type",(NCjson**)&jdnczarr));
	    if(jdnczarr != NULL && NCJsort(jdnczarr) != NCJ_STRING) {stat = NC_ENCZARR; goto done;}
	    if(jdnczarr != NULL)
		dnczarr = NCJstring(jdnczarr);
	}
	/* Convert dtype to nc_type */
	if((stat = dtype2nctype(dtype,dnczarr,&vtype,&maxstrlen))) goto done;
	if(vtype > NC_NAT && vtype <= NC_MAX_ATOMIC_TYPE) {
	    /* Locate the NC_TYPE_INFO_T object */
	    if((stat = ncz_gettype(file,var->container,vtype,&var->type_info))) goto done;
	} else {stat = NC_EBADTYPE; goto done;}
	if(vtype == NC_STRING) {
	    zsetmaxstrlen(maxstrlen,var);
	    if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,NC_NCZARR_MAXSTRLEN_ATTR,DA_MAXSTRLEN,FIXATT))) goto done;
	}
    }

    { /* Extract the shape */
	NCJcheck(NCJdictget(jvar,"shape",(NCjson**)&jvalue));
	if(NCJsort(jvalue) != NCJ_ARRAY) {stat = THROW(NC_ENOTZARR); goto done;}
	zarr_rank = NCJarraylength(jvalue);
	if(zarr_rank > 0) {
	    if((shapes = (size64_t*)calloc(zarr_rank,sizeof(size64_t)))==NULL) {stat = NC_ENOMEM; goto done;}
	    if((stat=NCZ_decodesizet64vec(jvalue, &zarr_rank, shapes))) goto done;	
	}
	/* Set the rank of the variable */
	if((stat = nc4_var_set_ndims(var, zarr_rank))) goto done;
    }

    /* Process dimrefs (might be NULL) */
    if((stat = decode_var_dimrefs(file,zarr_rank,shapes,jvar,dimrefs))) goto done;

    /* Process chunk_grid (must precede chunk cache creation) */
    {
	NCJcheck(NCJdictget(jvar,"chunk_grid",(NCjson**)&jchunkgrid));
	if(jchunkgrid == NULL) {stat = NC_ENOTZARR; goto done;}
	if((stat = extract_named_config(jchunkgrid,"name",&jvalue))) goto done;
	if(strcmp(NCJstring(jvalue),"regular")!=0) {stat = NC_ENCZARR; goto done;}
	if((stat = extract_named_config(jchunkgrid,"chunk_shape",&jvalue))) goto done;
	/* Validate */
	if(jvalue == NULL || NCJsort(jvalue)!=NCJ_ARRAY || NCJarraylength(jvalue) != zarr_rank)
	    {stat = NC_ENOTZARR; goto done;}
	if((chunks = (size64_t*)calloc(zarr_rank,sizeof(size64_t)))==NULL) {stat = NC_ENOMEM; goto done;}
	if((stat=NCZ_decodesizet64vec(jvalue, &zarr_rank, chunks))) goto done;	
	var->storage = NC_CHUNKED;
    }	

    /* Process chunk_key_encoding (must precede chunk cache creation) */
    {
	NCglobalstate* ngs = NC_getglobalstate();
	assert(ngs != NULL);
	zvar->dimension_separator = 0;
		
	NCJcheck(NCJdictget(jvar,"chunk_key_encoding",(NCjson**)&jchunkkey));
	if(jchunkkey == NULL) {stat = NC_ENOTZARR; goto done;}
	/* Figure out what separator to use */
	if((stat = extract_named_config(jchunkkey,"name",&jvalue))) goto done;
	if(strcmp(NCJstring(jvalue),"default")!=0
	   && strcmp(NCJstring(jvalue),"v2")!=0) {stat = NC_ENOTZARR; goto done;}
	/* Get the separator character */
	if((stat = extract_named_config(jchunkkey,"separator",&jvalue))) goto done;
	if(jvalue != NULL) {
	    const char* sep = NCJstring(jvalue);
	    if(strlen(sep) != 1) {stat = NC_ENOTZARR; goto done;}
	    if(!islegaldimsep(sep[0])) {stat = NC_ENOTZARR; goto done;}
	    zvar->dimension_separator = sep[0];
	} else
	    zvar->dimension_separator = ngs->zarr.dimension_separator; /* use global default */
    }	

    /* fill_value; must precede calls to adjust cache */
    {
	NCJcheck(NCJdictget(jvar,"fill_value",(NCjson**)&jvalue));
	if(jvalue == NULL || NCJsort(jvalue) == NCJ_NULL) {
	    var->no_fill = NC_NOFILL;
	    if((stat = NCZ_disable_fill(file,var))) goto done;
	} else { /* Fill in var->fill_value */
	    var->no_fill = NC_FILL;
	    NCZ_clearAttrInfo(file,&ainfo);
	    ainfo.name = NC_FillValue;
	    ainfo.nctype = vtype;
	    if((stat = NCZ_computeattrdata(file,jvalue,&ainfo))) goto done;
	    /* Create var->fill_value */
	    assert(ainfo.nctype == vtype);
	    if((stat = NCZ_set_dual_obj_data(file,(NC_OBJ*)var,NC_FillValue,DA_FILLVALUE,ainfo.datalen,ainfo.data))) goto done;	
	    /* propagate to _FillValue attribute */
	    if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,NC_FillValue,DA_FILLVALUE,FIXATT))) goto done;
	    /* clear+reclaim ainfo */
	    NCZ_clearAttrInfo(file,&ainfo);
	}
    }

    /* Codecs key */
    /* From V3 Spec: A list of JSON objects providing codec configurations,
       or null if no filters are to be applied. Each codec configuration
       object MUST contain a "name" key identifying the codec to be used.
       Note that for V3 every array has at least one codec: the "bytes" codec
       that specifies endianness.
    */
    if(var->filters == NULL) var->filters = (void*)nclistnew();
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    if((stat = NCZ_filter_initialize())) goto done;
#endif
    {
	NCJcheck(NCJdictget(jvar,"codecs",(NCjson**)&jcodecs));
	if(jcodecs == NULL || NCJsort(jcodecs) != NCJ_ARRAY || NCJarraylength(jcodecs) == 0)
	    {stat = NC_ENOTZARR; goto done;}
	/* Get endianess from the first codec */
	jendian = NCJith(jcodecs,0);
	NCJcheck(NCJdictget(jendian,"name",(NCjson**)&jvalue));
	if(strcmp("bytes",NCJstring(jvalue))!=0) {stat = NC_ENOTZARR; goto done;}
	/* Get the configuration */
	NCJcheck(NCJdictget(jendian,"configuration",(NCjson**)&jvalue));
	if(NCJsort(jvalue) != NCJ_DICT) {stat = NC_ENOTZARR; goto done;}
	/* Get the endianness */
	NCJcheck(NCJdictget(jvalue,"endian",(NCjson**)&jendian));
	if(jendian == NULL)  {stat = NC_ENOTZARR; goto done;}
	if(strcmp("big",NCJstring(jendian))==0) endianness = NC_ENDIAN_BIG;
	else if(strcmp("little",NCJstring(jendian))==0) endianness = NC_ENDIAN_LITTLE;
	else  {stat = NC_ENOTZARR; goto done;}
	if(endianness != NC_ENDIAN_NATIVE) {
	    var->endianness = endianness;
	    var->type_info->endianness = var->endianness; /* Propagate */
	}	
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
	{
	size_t k;
	const NCjson* jfilter = NULL;
	for(k=1;k<NCJarraylength(jcodecs);k++) {
	    jfilter = NCJith(jcodecs,k);
	    if(NCJsort(jfilter) != NCJ_DICT) {stat = NC_EFILTER; goto done;}
	    nclistpush(filtersj,jfilter);
	}
	}
#endif /*NETCDF_ENABLE_NCVARR_FILTERS*/
    }

    /* Suppress variable if there are filters and var is not fixed-size */
    if(varsized && nclistlength(filtersj) > 0)
	suppress = 1;

    if(suppress) {
	NC_GRP_INFO_T* grp = var->container;
	/* Reclaim NCZarr variable specific info */
	(void)NCZ_zclose_var1(var);
	/* Remove from list of variables and reclaim the top level var object */
	(void)nc4_var_list_del(grp, var);
	var = NULL;
    }

    if(shapesp) {*shapesp = shapes; shapes = NULL;}
    if(chunksp) {*chunksp = chunks; chunks = NULL;}

done:
    nullfree(chunks);
    NCZ_clearAttrInfo(file,&ainfo);
    nullfree(shapes); shapes = NULL;
    return THROW(stat);
}

int
ZF3_decode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts)
{
    int stat = NC_NOERR;
    size_t i;
    NC_ATT_INFO_T* att = NULL;
    struct NCZ_AttrInfo ainfo = NCZ_emptyAttrInfo();
    const NCjson* jtypes = NULL;
    const NCjson* jnczattr = NULL;

    /* Extract _nczarr_attrs */
    NCJcheck(NCJdictget(jatts,NC_NCZARR_ATTRS_ATTR,(NCjson**)&jnczattr));
    if(jnczattr != NULL) {
	/* See if we have jtypes */
	NCJcheck(NCJdictget(jnczattr,"attribute_types",(NCjson**)&jtypes));
    }

    if(jatts != NULL && NCJsort(jatts)==NCJ_DICT) {    
	for(i=0;i<NCJdictlength(jatts);i++) {
	    const NCjson* janame = NCJdictkey(jatts,i);
	    const NCjson* javalue = NCJdictvalue(jatts,i);
	    const NC_reservedatt* ra = NULL;
	    const char* aname = NCJstring(janame);
	    DualAtt dualatt;
	    NC_GRP_INFO_T* grp = (NC_GRP_INFO_T*)(container->sort == NCGRP ? container : NULL);

	    /* See if this is reserved attribute */
	    ra = NC_findreserved(aname);
	    if(ra != NULL) {
		/* case 1: name = _NCProperties, grp=root, varid==NC_GLOBAL */
		if(grp != NULL && file->root_grp == grp && strcmp(aname,NCPROPS)==0 && grp != NULL) {
		    /* Setup provenance */
		    if(NCJsort(javalue) != NCJ_STRING)
			{stat = (THROW(NC_ENCZARR)); goto done;} /*malformed*/
		    if((stat = NCZ_read_provenance(file,aname,NCJstring(javalue)))) goto done;
		    continue; /* Suppress _NCProperties until we write the file */
		} else {/* case other: if attribute is hidden */
		    if(ra->flags & HIDDENATTRFLAG) continue; /* ignore it */
		}
	    }

	    /* Collect the attribute's type and value  */
	    NCZ_clearAttrInfo(file,&ainfo);
	    if((stat = computeattrinfo(file,container,aname,jtypes,javalue,&ainfo))) goto done;
	    /* Create the attribute */
	    if((stat = ncz_makeattr(file,container,&ainfo,&att))) goto done;

	    /* if a dual attr sync with NC_XXX_INFO_T object */
	    dualatt = NCZ_is_dual_att(aname); /* See if this is a dual attribute */
	    if(dualatt != DA_NOT) {
		if((stat = NCZ_sync_dual_att(file,container,aname,dualatt,FIXOBJ))) goto done;
	    }		
	}
    }

done:
    NCZ_clearAttrInfo(file,&ainfo);
    return THROW(stat);
}

/**************************************************/

int
ZF3_upload_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* key = NULL;

    /* Add attributes to the group zarr.json */
    NCJcheck(NCJinsert(zobj->jobj,"attributes",zobj->jatts)); zobj->jatts = NULL;

    /* Construct grp key */
    if((stat = NCZ_grpkey(grp,&fullpath))) goto done;

    /* build ZGROUP path */
    if((stat = nczm_concat(fullpath,Z3GROUP,&key))) goto done;
    /* Write to map */
    if((stat=NCZMD_update_json_content(file,NCZMD_GROUP,key,zobj->jobj))) goto done;
    nullfree(key); key = NULL;

done:
    nullfree(fullpath);
    nullfree(key);
    return THROW(stat);
}

int
ZF3_upload_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* key = NULL;

    /* Add attributes to the var zarr.json */
    NCJcheck(NCJinsert(zobj->jobj,"attributes",zobj->jatts)); zobj->jatts = NULL;

    /* Construct var key */
    if((stat = NCZ_varkey(var,&fullpath))) goto done;

    /* build ZARRAY path */
    if((stat = nczm_concat(fullpath,Z3ARRAY,&key))) goto done;
    /* Write to map */
    if((stat=NCZMD_update_json_content(file,NCZMD_ARRAY,key,zobj->jobj))) goto done;
    nullfree(key); key = NULL;

done:
    nullfree(fullpath);
    nullfree(key);
    return THROW(stat);
    return THROW(stat);
}

/*Write JSON Metadata*/
int
ZF3_encode_superblock(NC_FILE_INFO_T* file, NCjson** jsuperp)
{
    int stat = NC_NOERR;
    char version[64];
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCjson* jsuper = NULL;

    /* create superblock */
    snprintf(version,sizeof(version),NCZARR_FORMAT_VERSION_TEMPLATE, zinfo->zarr.nczarr_format);
    NCJnew(NCJ_DICT,&jsuper);
    NCJcheck(NCJinsertstring(jsuper,"version",version));
    if(jsuperp) {*jsuperp = jsuper; jsuper = NULL;}
done:
    NCJreclaim(jsuper);
    return THROW(stat);
}

int
ZF3_encode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jnczgrpp)
{
    int stat = NC_NOERR;
    size_t i;
    NCjson* jnczgrp = NULL;
    NCjson* jdims = NULL;
    NCjson* jvars = NULL;
    NCjson* jsubgrps = NULL;
    NCjson* jsize = NULL;
    NCjson* junlim = NULL;
    NCjson* jdim = NULL;

    NC_UNUSED(file);
    /* Create the NCZ_GROUP dict */
    NCJnew(NCJ_DICT,&jnczgrp);

    /* Collect and encode the grp dimension declarations */
    NCJnew(NCJ_DICT,&jdims);
    for(i=0;i<ncindexsize(grp->dim);i++) {
	NC_DIM_INFO_T* dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
	char digits[64];
	snprintf(digits,sizeof(digits),"%zu",dim->len);
	NCJcheck(NCJnewstring(NCJ_INT,digits,&jsize));
	NCJcheck(NCJnewstring(NCJ_INT,(dim->unlimited?"1":"0"),&junlim));
	NCJnew(NCJ_DICT,&jdim);
	NCJcheck(NCJinsert(jdim,"size",jsize)); jsize = NULL;
	NCJcheck(NCJinsert(jdim,"unlimited",junlim)); junlim = NULL;
	NCJcheck(NCJinsert(jdims,dim->hdr.name,jdim)); jdim = NULL;
    }
    NCJcheck(NCJinsert(jnczgrp,"dimensions",jdims)); jdims = NULL;

    /* Collect and insert the variable names in this group */
    NCJnew(NCJ_ARRAY,&jvars);
    for(i=0;i<ncindexsize(grp->vars);i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	NCJcheck(NCJaddstring(jvars,NCJ_STRING,var->hdr.name));
    }
    NCJcheck(NCJinsert(jnczgrp,"arrays",jvars)); jvars = NULL;

    /* Collect and insert the variable names in this group */
    NCJnew(NCJ_ARRAY,&jsubgrps);
    for(i=0;i<ncindexsize(grp->children);i++) {
	NC_GRP_INFO_T* child = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	NCJcheck(NCJaddstring(jsubgrps,NCJ_STRING,child->hdr.name));
    }
    NCJcheck(NCJinsert(jnczgrp,"groups",jsubgrps)); jsubgrps = NULL;

    if(jnczgrpp) {*jnczgrpp = jnczgrp; jnczgrp = NULL;}
done:
    NCJreclaim(jnczgrp);
    NCJreclaim(jdims);
    NCJreclaim(jvars);
    NCJreclaim(jsubgrps);
    NCJreclaim(jsize);
    NCJreclaim(junlim);
    NCJreclaim(jdim);
    return THROW(stat);
}

int
ZF3_encode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jgroupp)
{
    int stat = NC_NOERR;
    NCjson* jgroup = NULL;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    
    NC_UNUSED(grp);

    /* Add standard group fields */
    NCJcheck(NCJnew(NCJ_DICT,&jgroup)); /* zarr.json */
    NCJcheck(NCJinsertstring(jgroup,"node_type","group"));
    NCJcheck(NCJinsertint(jgroup,"zarr_format",zfile->zarr.zarr_format));

    if(jgroupp) {*jgroupp = jgroup; jgroup = NULL;}
done:
    NCJreclaim(jgroup);
    return THROW(stat);
}

int
ZF3_encode_nczarr_array(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson** jnczarrayp)
{
    int stat = NC_NOERR;
    NClist* dimrefs = NULL;
    NCjson* jnczarray = NULL;
    NCjson* jdimrefs = NULL;
    NCbytes* dimfqn = ncbytesnew();
    size_t i;
    char* dnczarr = NULL;

    NC_UNUSED(file);

    NCJnew(NCJ_DICT,&jnczarray);

    if((dimrefs = nclistnew())==NULL) {stat = NC_ENOMEM; goto done;}
    if(var->ndims > 0) {
	for(i=0;i<var->ndims;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    if((stat = NCZ_makeFQN((NC_OBJ*)dim,dimfqn))) goto done;
	    nclistpush(dimrefs,ncbytesextract(dimfqn));
	}
    }
    
    /* Create the dimrefs json object */
    NCJnew(NCJ_ARRAY,&jdimrefs);
    while(nclistlength(dimrefs)>0) {
	char* fqn = (char*)nclistremove(dimrefs,0);
	NCJaddstring(jdimrefs,NCJ_STRING,fqn);
	nullfree(fqn); fqn = NULL;
    }
    /* Insert dimension_references  */
    NCJcheck(NCJinsert(jnczarray,"dimension_references",jdimrefs)); jdimrefs = NULL;
     
    /* Add the _Storage flag */
    /* Record if this is a scalar */
    if(var->ndims == 0) {
	NCJcheck(NCJinsertint(jnczarray,"scalar",1));
    }

    /* everything looks like it is chunked */
    NCJcheck(NCJinsertstring(jnczarray,"storage","chunked"));

    /* insert nczarr_type */
    if((stat = nctype2dtype(var->type_info->hdr.id,NCZ_get_maxstrlen((NC_OBJ*)var),NULL,&dnczarr))) goto done;
    if(dnczarr != NULL)
	NCJcheck(NCJinsertstring(jnczarray,"nczarr_type",dnczarr));

    if(jnczarrayp) {*jnczarrayp = jnczarray; jnczarray = NULL;}
done:
    nullfree(dnczarr);
    nclistfreeall(dimrefs);
    ncbytesfree(dimfqn);
    NCJreclaim(jnczarray);
    NCJreclaim(jdimrefs);
    return THROW(stat);
}

int
ZF3_encode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj, NCjson** jvarp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCjson* jvar = NULL;
    NCjson* jshape = NULL;
    NCjson* jchunks = NULL;
    NCjson* jfill = NULL;
    NCjson* jcodecs = NULL;
    NCjson* jcodec = NULL;
    NCjson* jdimnames = NULL;
    NCjson* jchunkgrid = NULL;
    NCjson* jchunkkey = NULL;
    NCjson* jcfg = NULL;
    NCjson* jtmp = NULL;
    size_t zarr_rank = 0;
    size_t i;
    char tmpstr[1024];
    char* dtypename = NULL;
    
    NC_UNUSED(file);
#ifndef NETCDF_ENABLE_NCZARR_FILTERS
    NC_UNUSED(filtersj);
#endif

    NCJnew(NCJ_DICT,&jvar);

    /* Add standard group fields */
    NCJcheck(NCJinsertstring(jvar,"node_type","array"));
    NCJcheck(NCJinsertint(jvar,"zarr_format",zfile->zarr.zarr_format));

    /* data_type key */
    /* A string or list defining a valid data type for the array. */
    {	/* Add the type name */
	int atomictype = var->type_info->hdr.id;
	assert(atomictype > 0 && atomictype <= NC_MAX_ATOMIC_TYPE);
	if((stat = nctype2dtype(atomictype,NCZ_get_maxstrlen((NC_OBJ*)var),&dtypename,NULL))) goto done;
	NCJcheck(NCJinsertstring(jvar,"data_type",dtypename));
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
    NCJcheck(NCJinsert(jvar,"fill_value",jfill)); jfill = NULL;

    /* shape key */
    /* Integer list defining the length of each dimension of the array.*/
    /* Create the list */
    zarr_rank = var->ndims;
    NCJnew(NCJ_ARRAY,&jshape);
    if(var->ndims > 0) {
	for(i=0;i<zarr_rank;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    snprintf(tmpstr,sizeof(tmpstr),"%zu",dim->len);
	    NCJaddstring(jshape,NCJ_INT,tmpstr);
	}
    }
    NCJcheck(NCJinsert(jvar,"shape",jshape)); jshape = NULL;

    /* chunks key */
    /* The zarr format does not support the concept
       of contiguous (or compact), so it will never appear in the read case.
    */
    /* Create the list of chunksizes */
    NCJnew(NCJ_ARRAY,&jchunks);
    if(var->ndims > 0) {
	for(i=0;i<zarr_rank;i++) {
	    size64_t len = var->chunksizes[i];
	    snprintf(tmpstr,sizeof(tmpstr),"%lld",len);
	    NCJaddstring(jchunks,NCJ_INT,tmpstr);
	}
    }

    /* Build the chunk_grid configuration */
    jchunkgrid = build_named_config("regular",1,"chunk_shape",jchunks); jchunks = NULL;
    NCJcheck(NCJinsert(jvar,"chunk_grid",jchunkgrid)); jchunkgrid = NULL;

    /* Build the chunk_key_encoding configuration */
    {
	char sepstr[2];
	sepstr[0] = NCZ_get_dimsep(var);
	sepstr[1] = '\0';
	NCJcheck(NCJnewstring(NCJ_STRING,sepstr,(NCjson**)&jtmp));
	jchunkkey = build_named_config("default",1,"separator",jtmp); jtmp = NULL;
	NCJcheck(NCJinsert(jvar,"chunk_key_encoding",jchunkkey)); jchunkkey = NULL;
    }

    /* Collect dimension_names, even if they are meaningless */
    NCJnew(NCJ_ARRAY,&jdimnames);
    for(i=0;i<var->ndims;i++) {
	NC_DIM_INFO_T* dim = (NC_DIM_INFO_T*)var->dim[i];
	NCJcheck(NCJnewstring(NCJ_STRING,dim->hdr.name,&jtmp));
	NCJcheck(NCJappend(jdimnames,jtmp)); jtmp = NULL;
    }
    NCJcheck(NCJinsert(jvar,"dimension_names",jdimnames)); jdimnames = NULL;

    /* There is always at least the bytes codec */
    NCJnew(NCJ_ARRAY,&jcodecs);
    {
	switch (var->endianness) {
	case NC_ENDIAN_BIG: NCJcheck(NCJnewstring(NCJ_STRING,"big",&jtmp)); break;
	case NC_ENDIAN_LITTLE: NCJcheck(NCJnewstring(NCJ_STRING,"little",&jtmp)); break;
	default: stat = NC_EINVAL; goto done;
	}
	jcodec = build_named_config("bytes",1,"endian",jtmp); jtmp = NULL;
	NCJcheck(NCJappend(jcodecs,jcodec)); jcodec = NULL;
    }

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    while(nclistlength(filtersj) > 0) {
	NCjson* jcodec = nclistremove(filtersj,0);
	NCJcheck(NCJappend(jcodecs,jcodec)); jcodec = NULL;
    }
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
    NCJcheck(NCJinsert(jvar,"codecs",jcodecs)); jcodecs = NULL;

    if(jvarp) {*jvarp = jvar; jvar = NULL;}

done:
    nullfree(dtypename);
    NCJreclaim(jvar);
    NCJreclaim(jshape);
    NCJreclaim(jchunks);
    NCJreclaim(jfill);
    NCJreclaim(jcodecs);
    NCJreclaim(jcodec);
    NCJreclaim(jdimnames);
    NCJreclaim(jchunkgrid);
    NCJreclaim(jchunkkey);
    NCJreclaim(jcfg);
    NCJreclaim(jtmp);
    return THROW(stat);
}

int
ZF3_encode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson** jnczconp, NCjson** jsuperp, NCjson** jattsp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t i;
    NCindex* atts = NULL;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jdata = NULL;
    NCjson* jnczatt = NULL;
    NC_VAR_INFO_T* var = NULL;
    NC_GRP_INFO_T* grp = NULL;
    char* d2name = NULL;
    char* dnczarr = NULL;
    const char* nczname = NULL;
    int purezarr = 0;
    size_t typesize = 0;

    TESTPUREZARR;

    if(container->sort == NCVAR) {
	var = (NC_VAR_INFO_T*)container;
	atts = var->att;
	nczname = NC_NCZARR_ARRAY_ATTR;
    } else if(container->sort == NCGRP) {
	grp = (NC_GRP_INFO_T*)container;
	atts = grp->att;
	nczname = NC_NCZARR_GROUP_ATTR;
    }
    
    NCJnew(NCJ_DICT,&jatts);
    if(ncindexsize(atts) > 0) {
	NCJnew(NCJ_DICT,&jtypes);

	/* Walk all the attributes convert to json and collect the dtype */
	for(i=0;i<ncindexsize(atts);i++) {
	    NC_ATT_INFO_T* a = (NC_ATT_INFO_T*)ncindexith(atts,i);

	    if(a->nc_typeid > NC_MAX_ATOMIC_TYPE) {stat = (THROW(NC_ENCZARR)); goto done;}
	    if(a->nc_typeid == NC_STRING)
		typesize = (size_t)NCZ_get_maxstrlen(container);
	    else {
		if((stat = NC4_inq_atomic_type(a->nc_typeid,NULL,&typesize))) goto done;
	    }

	    /* Convert to storable json */

	    if(a->nc_typeid == NC_CHAR && NCZ_iscomplexjsonstring(a->hdr.name,a->len,(char*)a->data,&jdata)) {
		dnczarr = strdup(NC_JSON_DTYPE_V3);
		d2name = strdup(dnczarr);
	    } else {
		if((stat = NCZ_stringconvert(a->nc_typeid,a->len,a->data,&jdata))) goto done;
		/* Collect the corresponding dtype and the alias type to use for attributes */
		if((stat = nctype2dtype(a->nc_typeid,typesize,&d2name,&dnczarr))) goto done;
	    }
	    /* Overwrite the zarr type with the nczarr type */
	    if(dnczarr != NULL) {nullfree(d2name); d2name = dnczarr; dnczarr = NULL;}	    
	    /* Insert the attribute; optionally consumes jdata */
	    if((stat = ncz_insert_attr(jatts,jtypes,a->hdr.name,&jdata,d2name))) goto done;

	    /* cleanup */
	    NCJreclaim(jdata); jdata = NULL;
	    nullfree(d2name); d2name = NULL;
    	    nullfree(dnczarr); dnczarr = NULL;
	}
    }

    /* Finalize the contents of jtypes and jatts */
    if(!purezarr) {
	if(jtypes == NULL) NCJnew(NCJ_DICT,&jtypes);
	/* Insert _nczarr_group|_nczarr_var + type */
	if(jnczconp != NULL && *jnczconp != NULL) {
	    if((stat = ncz_insert_attr(jatts,jtypes,nczname,jnczconp,NC_JSON_DTYPE_V3))) goto done;
	    *jnczconp = NULL;
	}
	/* Insert _nczarr_super (if root group) + type */
	if(jsuperp != NULL && *jsuperp != NULL) {
	    if((stat=ncz_insert_attr(jatts,jtypes,NC_NCZARR_SUPERBLOCK_ATTR,jsuperp,NC_JSON_DTYPE_V3))) goto done;
	    *jsuperp = NULL;
	}
	
	/* Build _nczarr_attrs */
	NCJnew(NCJ_DICT,&jnczatt);
	NCJcheck(NCJinsert(jnczatt,"attribute_types",jtypes));
	/* WARNING, jtypes may undergo further changes */
	/* Insert _nczarr_attrs + type */	
	if((stat=ncz_insert_attr(jatts,jtypes,NC_NCZARR_ATTRS_ATTR,&jnczatt,NC_JSON_DTYPE_V3))) goto done;
	jtypes = NULL;
	assert(*jnczconp == NULL && jnczatt == NULL && jtypes == NULL);
    }

    if(jattsp) {*jattsp = jatts; jatts = NULL;}

done:
    nullfree(d2name);
    nullfree(dnczarr);
    NCJreclaim(jdata);
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    return THROW(stat);
}

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
/*Filter Processing*/
static int
ZF3_encode_filter(NC_FILE_INFO_T* file, NCZ_Filter* filter, NCjson** jfilterp)
{
    int stat = NC_NOERR;
    NCjson* jfilter = NULL;
    
    NC_UNUSED(file);

    /* assumptions */
    assert(filter->flags & FLAG_WORKING);

    /* Convert the HDF5 id + parameters to the codec form */

    /* We need to ensure the the current visible parameters are defined and had the opportunity to come
       from the working parameters */
    assert((filter->flags & (FLAG_VISIBLE | FLAG_WORKING)) == (FLAG_VISIBLE | FLAG_WORKING));

    /* Convert the visible parameters back to codec */
    /* Clear any previous codec */
    nullfree(filter->codec.id); filter->codec.id = NULL;
    nullfree(filter->codec.codec); filter->codec.codec = NULL;
    filter->codec.id = strdup(filter->plugin->codec.codec->codecid);
    if(filter->plugin->codec.codec->NCZ_hdf5_to_codec) {
	if((stat = filter->plugin->codec.codec->NCZ_hdf5_to_codec(NCplistzarrv3,filter->hdf5.id,filter->hdf5.visible.nparams,filter->hdf5.visible.params,&filter->codec.codec))) goto done;
    } else
	{stat = NC_EFILTER; goto done;}

    /* Parse the codec as the return */
    NCJcheck(NCJparse(filter->codec.codec,0,&jfilter));
    if(jfilterp) {*jfilterp = jfilter; jfilter = NULL;}

done:
    NCJreclaim(jfilter);
    return THROW(stat);
}

static int
ZF3_decode_filter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson* jfilter, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
    const NCjson* jvalue = NULL;
    NCZ_Plugin* plugin = NULL;
    NCZ_Codec codec = NCZ_codec_empty();
    NCZ_HDF5 hdf5 = NCZ_hdf5_empty();

    NC_UNUSED(file);

    if(var->filters == NULL) var->filters = nclistnew();

    /* Get the name of this codec filter */
    NCJcheck(NCJdictget(jfilter,"name",(NCjson**)&jvalue));
    if(NCJsort(jvalue) != NCJ_STRING) {stat = THROW(NC_ENOFILTER); goto done;}

    /* Save the codec */
    if((codec.id = strdup(NCJstring(jvalue)))==NULL) {stat = NC_ENOMEM; goto done;}
    NCJcheck(NCJunparse(jfilter,0,&codec.codec));

    /* Find the plugin for this filter */
    if((stat = NCZ_plugin_lookup(codec.id,&plugin))) goto done;

    if(plugin != NULL) {
	/* Save the hdf5 id */
	hdf5.id = plugin->codec.codec->hdf5id;
	/* Convert the codec to hdf5 form visible parameters */
	if(plugin->codec.codec->NCZ_codec_to_hdf5) {
	    if((stat = plugin->codec.codec->NCZ_codec_to_hdf5(NCplistzarrv3,codec.codec,&hdf5.id,&hdf5.visible.nparams,&hdf5.visible.params)))
		goto done;
	}
	filter->flags |= FLAG_VISIBLE;
	filter->hdf5 = hdf5; hdf5 = NCZ_hdf5_empty();
	filter->codec = codec; codec = NCZ_codec_empty();
	filter->plugin = plugin; plugin = NULL;
    } else {
	/* Create a fake filter so we do not forget about this codec */
	filter->hdf5 = NCZ_hdf5_empty();
	filter->codec = codec; codec = NCZ_codec_empty();
    }

done:
    ncz_hdf5_clear(&hdf5);
    ncz_codec_clear(&codec);
    return THROW(stat);
}
#else /*!NETCDF_ENABLE_NCZARR_FILTERS*/
static int
ZF3_encode_filter(NC_FILE_INFO_T* file, NCZ_Filter* filter, NCjson** jfilterp)
{
    NC_UNUSED(file);
    NC_UNUSED(filter);
    if(jfilterp) *jfilterp = NULL;
    return NC_NOERR;
}

static int
ZF3_decode_filter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson* jfilter, NCZ_Filter* filter)
{
    NC_UNUSED(file);
    NC_UNUSED(var);
    NC_UNUSED(jfilter);
    NC_UNUSED(filter);
    return NC_NOERR;
}
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/


/*Search*/
int
ZF3_searchobjects(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrpnames)
{
    int stat = NC_NOERR;
    size_t i;
    char* grpkey = NULL;
    NClist* matches = nclistnew();
    char* subkey = NULL;
    char* objkey = NULL;
    NCjson* jcontents = NULL;
    const NCjson* jnodetype = NULL;

    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(grp,&grpkey))) goto done;
    if((stat = NCZMD_list(file,grpkey,matches))) goto done; /* Shallow listing */
    /* Search grp for zarr.json objects and for chunk objects */
    /* In order to tell if the name refers to an array, there are two ways to do it.
       1. we extend the objkey with "/c/ or "/c." to see if it exists as a prefix.
       2. we read the zarr.json and look at the node_type field.
       In the absence of consolidated metadat, (1) is slightly faster, but requires
       extending the zmap interface.
       So, for now, we implement case (2).
    */
    for(i=0;i<nclistlength(matches);i++) {
	const char* name = nclistget(matches,i);
	const char* ndtype = NULL;
	if(strcmp(name,Z3OBJECT) == 0) continue; /* current group metadata */
	/* Look for a zarr.json */
	if((stat = nczm_concat(grpkey,name,&subkey))) goto done;
	if((stat = nczm_concat(subkey,Z3OBJECT,&objkey))) goto done;
	/* Read the zarr.json object */
	switch (stat = NCZMD_fetch_json_content(file,NCZMD_GROUP,objkey,&jcontents)) {
	case NC_NOERR: { /* We found a zarr.json object */
	    if(jcontents == NULL || NCJsort(jcontents) != NCJ_DICT) break;
	    NCJcheck(NCJdictget(jcontents,"node_type",(NCjson**)&jnodetype));
	    if(jnodetype == NULL || NCJsort(jnodetype) != NCJ_STRING) {stat = NC_ENOTZARR; goto done;}
	    ndtype = NCJstring(jnodetype);
	    if(strcmp("array",ndtype)==0)
		nclistpush(varnames,strdup(name));
	    else if(strcmp("group",ndtype)==0)
		nclistpush(subgrpnames,strdup(name));
	    else
	        {stat = NC_ENOTZARR; goto done;}
	    } break;
	case NC_ENOOBJECT: break; /* keep searching */
	default: goto done;
	}
	stat = NC_NOERR;
	nullfree(subkey); subkey = NULL;
	nullfree(objkey); objkey = NULL;
	NCJreclaim(jcontents); jcontents = NULL;
    }

done:
    nullfree(subkey);
    nullfree(objkey);
    nullfree(grpkey);
    nclistfreeall(matches);
    NCJreclaim(jcontents);
    return stat;
}

/*Chunkkeys*/

/*
From Zarr V3 Specification:
"The compressed sequence of bytes for each chunk is stored under a key
formed from the index of the chunk within the grid of chunks
representing the array and prefixed with the string "c".  To form a
string key for a chunk, the indices are converted to strings and
concatenated with the dimension_separator character ('/' by default)
separating each index. For example, given an array with shape (10000,
10000) and chunk shape (1000, 1000) there will be 100 chunks laid out
in a 10 by 10 grid. The chunk with indices (0, 0) provides data for
rows 0-1000 and columns 0-1000 and is stored under the key "c/0/0"; the
chunk with indices (2, 4) provides data for rows 2000-3000 and columns
4000-5000 and is stored under the key "c/2/4"; etc."  If the rank is 0
(i.e. scalar) then use the key "c".
*/

int
ZF3_encode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, size_t rank, const size64_t* chunkindices, char dimsep, char** keyp)
{
    int stat = NC_NOERR;
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    NCbytes* key = ncbytesnew();
    size_t r;

    NC_UNUSED(file);

    if(keyp) *keyp = NULL;
    assert(islegaldimsep(dimsep));
    
    if(zvar->nonstdchunkkey) {
	if(rank == 0) {/*scalar*/
	    ncbytescat(key,"0");
	} else for(r=0;r<rank;r++) {
	    char sindex[64];
	    if(r > 0) ncbytesappend(key,dimsep);
	    /* Print as decimal with no leading zeros */
	    snprintf(sindex,sizeof(sindex),"%lu",(unsigned long)chunkindices[r]);	
	    ncbytescat(key,sindex);
	}
    } else { /* !zvar->nonstdchunkkey */
	ncbytescat(key,"c");
	for(r=0;r<rank;r++) {
	    char sindex[64];
	    ncbytesappend(key,dimsep);
	    /* Print as decimal with no leading zeros */
	    snprintf(sindex,sizeof(sindex),"%lu",(unsigned long)chunkindices[r]);	
	    ncbytescat(key,sindex);
	}
    }
    ncbytesnull(key);
    if(keyp) *keyp = ncbytesextract(key);
    ncbytesfree(key);
    return THROW(stat);
}

int
ZF3_decode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const char* chunkname, size_t* rankp, size64_t** chunkindicesp)
{
    int stat = NC_NOERR;
    char* oldp;
    char* newp;
    size64_t* chunkindices = NULL;
    size_t rank,r;
    char* chunkkey = strdup(chunkname);
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    char* base = NULL;
    char sep;
    
    NC_UNUSED(file);

    assert(strlen(chunkname) > 0);

    /* Figure out the separator char by looking for the first non digit in the chunkkey */
    if(zvar->nonstdchunkkey) {
	oldp = chunkkey;
	sep = '\0';
	for(;*oldp;oldp++) {char c = *oldp; if(c < '0' || c > '9') {sep = c; break;}}
    } else {
	sep = chunkkey[1]; /* might be trailing nul */
    }
    assert(sep=='\0' || islegaldimsep(sep));

    if(zvar->nonstdchunkkey) {
	/* Pass 1 to get rank and separate the indices*/
	oldp = (base = chunkkey);
	for(rank=0;;) {
	    newp = strchr(oldp,sep); /* look for next sep or eos */
	    rank++;
	    if(newp == NULL) break;
	    *newp = '\0';
	    oldp = newp+1;
	}
    } else {/*!zvar->nonstdchunkkey*/
	if(chunkname[0] != 'c') {stat = NC_ENOTZARR; goto done;}
	base = &chunkkey[1];
	/* Handle scalar case */
	if(strlen(base)==0) {
	    rank = 0;
	} else {
	    if(*base != sep) {stat = NC_ENOTZARR; goto done;}
	    base++; /* skip leading sep */
	    /* Pass 1 to get rank and separate the indices*/
	    oldp = base;
	    for(rank=0;;) {
		newp = strchr(oldp,sep); /* look for next sep or eos */
		rank++;
		if(newp == NULL) break;
		*newp = '\0';
		oldp = newp+1;
	    }
	    assert(rank > 0);
	    /* Create index vector */
	    if((chunkindices = (size64_t*)malloc(rank*sizeof(size64_t)))==NULL) {stat = NC_ENOMEM; goto done;}
	    /* Pass 2 to get indices */
	    oldp = base;
	    for(r=0;r<rank;r++) {
		sscanf(oldp,"%llu",&chunkindices[r]);
		oldp += (strlen(oldp)+1);
	    }
	}
    }
    if(rankp) *rankp = rank;
    if(chunkindicesp) {*chunkindicesp = chunkindices; chunkindices = NULL;}
done:
    nullfree(chunkkey);
    nullfree(chunkindices);
    return THROW(stat);
}

static int
ZF3_encode_xarray(NC_FILE_INFO_T* file, size_t rank, NC_DIM_INFO_T** dims, char** xarraydimsp, size_t* zarr_rankp)
{
    int stat = NC_NOERR;
    size_t i;
    size_t zarr_rank = rank;
    NCbytes* buf = ncbytesnew();    

    NC_UNUSED(file);

    ncbytescat(buf,"[");
    if(rank == 0) {
	ncbytescat(buf,XARRAYSCALAR);
	zarr_rank = 1;
    } else for(i=0;i<rank;i++) {
	const char* dimname = dims[i]->hdr.name;
	if(i > 0) ncbytescat(buf,",");
	ncbytescat(buf,dimname);
    }
    ncbytescat(buf,"]");
    if(xarraydimsp) {*xarraydimsp = ncbytesextract(buf);}
    if(zarr_rankp) {*zarr_rankp = zarr_rank;}

    ncbytesfree(buf);
    return THROW(stat);
}

static char
ZF3_default_dimension_separator(NC_FILE_INFO_T* file)
{
    NC_UNUSED(file);
    return DFALT_DIM_SEPARATOR_V3;
}

/**************************************************/
/* Support Functions */

static int
decode_dim_decls(NC_FILE_INFO_T* file, const NCjson* jdims, NClist* dimdefs)
{
    int stat = NC_NOERR;
    size_t i;
    struct NCZ_DimInfo* dimdef = NULL;

    NC_UNUSED(file);

    assert(NCJsort(jdims) == NCJ_DICT);
    for(i=0;i<NCJdictlength(jdims);i++) {
	const NCjson* jname = NCJdictkey(jdims,i);
	const NCjson* jdim = NCJdictvalue(jdims,i);
	struct NCJconst cvt;

	memset(&cvt,0,sizeof(cvt));
	if((dimdef = (struct NCZ_DimInfo*)calloc(1,sizeof(struct NCZ_DimInfo)))==NULL) goto done;

	strncpy(dimdef->norm_name,NCJstring(jname),sizeof(dimdef->norm_name));

	if(NCJisatomic(jdim)) { /* old-style length only dimension spec */
	    NCJcheck(NCJcvt(jdim,NCJ_INT,&cvt));	    
	    dimdef->shape = (size64_t)cvt.ival;
	    dimdef->unlimited = 0;
	} else {
	    const NCjson* jsize = NULL;
	    const NCjson* junlim = NULL;
	    assert(NCJsort(jdim) == NCJ_DICT);
	    NCJcheck(NCJdictget(jdim,"size",(NCjson**)&jsize));
	    NCJcheck(NCJdictget(jdim,"unlimited",(NCjson**)&junlim));
	    NCJcheck(NCJcvt(jsize,NCJ_INT,&cvt));
	    dimdef->shape = (size64_t)cvt.ival;
	    memset(&cvt,0,sizeof(cvt));
	    NCJcheck(NCJcvt(junlim,NCJ_INT,&cvt));
	    dimdef->unlimited = (cvt.ival == 0 ? 0 : 1);
	}
	nclistpush(dimdefs,dimdef); dimdef = NULL;
    }	 

done:
    NCZ_reclaim_diminfo(dimdef);
    return THROW(stat);
}

/*
 * Collect/create the dimension names for this variable.
 * In order of preference:
 * 1. _nczarr_var.dimensions -- the name are FQNs. Only available if !purezarr.
 * 2. dimension_names -- relative names scoped to parent group
 * 3. _ARRAY_DIMENSIONS -- (xarray) treat as a replacement for dimension_names if the latter is not defined
 * 4. _Anonymous_Dim_n -- scoped to root group and n is the length of the dimensions.
 */

/**
@param [in] file
@param [in] var
@param [in] jvar the JSON metadata for var
@param [out] dimrefs the list of FQNs for the variable's dimrefs
*/
static int
decode_var_dimrefs(NC_FILE_INFO_T* file, size_t rank, size64_t* shapes, const NCjson* jvar, NClist* dimrefs)
{
    int stat = NC_NOERR;
    size_t j;
    size_t i;
    int purezarr = 0;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    const NCjson* jatts = NULL;
    const NCjson* jdimnames = NULL;
    const NCjson* jxarray = NULL;
    const NCjson* jnczarray = NULL;
    const NCjson* jdimfqns = NULL;
    
    TESTPUREZARR;

    assert(dimrefs != NULL);

    /* Get the "dimension_names" from jvar */
    NCJcheck(NCJdictget(jvar,"dimension_names",(NCjson**)&jdimnames));

    /* Get the set of attributes */
    NCJcheck(NCJdictget(jvar,"attributes",(NCjson**)&jatts));

    if(jatts != NULL) {
	/* Get the xarray _ARRAY_DIMENSIONS */
	if(!(zinfo->flags & FLAG_XARRAYDIMS)) {
	    NCJcheck(NCJdictget(jatts,NC_XARRAY_DIMS,(NCjson**)&jxarray));
	}
	/* Get _nczarr_array then "dimension_references" */
	NCJcheck(NCJdictget(jatts,NC_NCZARR_ARRAY_ATTR,(NCjson**)&jnczarray));
	if(jnczarray != NULL) {
	    NCJcheck(NCJdictget(jatts,"dimension_references",(NCjson**)&jdimfqns));
	}
    }

    if(!purezarr && jdimfqns != NULL) { /* Use the NCZarr dimension fqns */
	for(i=0;i<rank;i++) {
	    const NCjson* jfqn = NCJith(jdimfqns,i);
	    assert(NCJsort(jfqn) == NCJ_STRING);
	    nclistpush(dimrefs,strdup(NCJstring(jfqn)));
	}
    } else if(jdimnames != NULL) { /* use "dimension_names" */
	assert(NCJsort(jdimnames) == NCJ_ARRAY);
	for(i=0;i<rank;i++) {
	    const NCjson* jname = NCJith(jdimnames,i);
	    nclistpush(dimrefs,strdup(NCJstring(jname)));
	}
    } else if(jxarray != NULL) { /* use xarray names */
	assert(NCJsort(jxarray) == NCJ_ARRAY);
	for(i=0;i<rank;i++) {
	    const NCjson* jname = NCJith(jxarray,i);
	    nclistpush(dimrefs,strdup(NCJstring(jname)));
	}
    } else { /* Last chance, simulate it from the shape of the variable */
	char anonname[NC_MAX_NAME];
	for(j=0;j<rank;j++) {
	    snprintf(anonname,sizeof(anonname),"/%s_%lld",NCDIMANON,shapes[j]);
	    nclistpush(dimrefs,strdup(anonname));
	}
    }
done:
    return THROW(stat);
}

/* Type Converters */

/**
@internal Given an nc_type+purezarr+MAXSTRLEN, produce the corresponding Zarr v3 dtype string.
@param nctype	  - [in] nc_type
@param strlen	  - [in] max string length
@param dtypep	  - [out] pointer to hold pointer to the dtype; user frees
@param dnczarrp	    - [out] pointer to hold pointer to the type alternative; user frees
@return NC_NOERR
@return NC_EINVAL
@author Dennis Heimbigner
*/

static int
nctype2dtype(nc_type nctype, size_t strlen, char** dtypep, char** dnczarrp)
{
    int stat = NC_NOERR;
    char* dtype = NULL;
    char* dnczarr = NULL;
    
    if(nctype <= NC_NAT || nctype > N_NCZARR_TYPES) return NC_EINVAL;

    if(dtypep) *dtypep = NULL;
    if(dnczarrp) *dnczarrp = NULL;

    switch (nctype) {
    case NC_BYTE: {dtype = strdup("int8"); break;}
    case NC_CHAR: {dtype = strdup("uint8"); dnczarr = strdup("char"); break;}
    case NC_SHORT: {dtype = strdup("int16"); break;}
    case NC_INT: {dtype = strdup("int32"); break;}
    case NC_FLOAT: {dtype = strdup("float32"); break;}
    case NC_DOUBLE: {dtype = strdup("float64"); break;}
    case NC_UBYTE: {dtype = strdup("uint8"); break;}
    case NC_USHORT: {dtype = strdup("uint16"); break;}
    case NC_UINT: {dtype = strdup("uint32"); break;}
    case NC_INT64: {dtype = strdup("int64"); break;}
    case NC_UINT64: {dtype = strdup("uint64"); break;}
    case NC_STRING: {
	char rstring[64];
	size_t bits = 8*strlen;
	snprintf(rstring,sizeof(rstring),STRTEMPLATE,bits);
	dtype = strdup(rstring);
	dnczarr = strdup("string");
	break;
	}
    case NC_NAT: case NC_JSON: default: stat = NC_ENOTZARR; goto done;
    }

    if(dtypep) {*dtypep = dtype; dtype = NULL;}
    if(dnczarrp) {*dnczarrp = dnczarr; dnczarr = NULL;}

done:
    nullfree(dtype);
    nullfree(dnczarr);
    return THROW(stat);
}

/*
@internal Convert a numcodecs Zarr v3 dtype spec to a corresponding nc_type.
@param dtype	- [in] dtype to convert; NULL => the type came from an attribute type
@param dnczarr	- [in] dtype override
@param nctypep	- [out] hold corresponding type
@param typelenp - [out] hold corresponding type size (esp. for fixed length strings)
@return NC_NOERR
@return NC_EINVAL
@author Dennis Heimbigner
*/

static int
dtype2nctype(const char* dtype, const char* dnczarr, nc_type* nctypep, size_t* typelenp)
{
    int stat = NC_NOERR;
    nc_type nctype = NC_NAT;
    size_t typelen = 0; /* use int so we can return -1 to indicate undefined */
    char testname[16];
	
    if(nctypep) *nctypep = NC_NAT;
    if(typelenp) *typelenp = 0;

    /* At least one must be defined */
    if((dtype == NULL || strlen(dtype) == 0) && (dnczarr == NULL || strlen(dnczarr) == 0))
	{stat = NC_ENOTZARR; goto done;}

    /* parse the dnczarr type first */
    nctype = NC_NAT;
    if(dnczarr != NULL) {
	if(strcmp(dnczarr,"char")==0) {nctype = NC_CHAR; typelen = 1;}
	else if(strcmp(dnczarr,"string")==0) {nctype = NC_STRING; typelen = 0;}
	else if(strcmp(dnczarr,NC_JSON_DTYPE_V3)==0) {nctype = NC_JSON; typelen = 0;}
    }
    /* Not an nczarr type, try a zarr type */
    if(nctype == NC_NAT) {
	if(dtype == NULL) {dtype = dnczarr; dnczarr = NULL; /* override */}
	/* make it possible to use memcmp */
	memset(testname,0,sizeof(testname));
	strncpy(testname,dtype,sizeof(testname));
	     if(memcmp(testname,"int8",4)==0) {nctype = NC_BYTE; typelen = 1;}
	else if(memcmp(testname,"int16",5)==0) {nctype = NC_SHORT; typelen = 1;}
	else if(memcmp(testname,"int32",5)==0) {nctype = NC_INT; typelen = 1;}
	else if(memcmp(testname,"int64",5)==0) {nctype = NC_INT64; typelen = 1;}
	else if(memcmp(testname,"uint8",5)==0) {nctype = NC_UBYTE; typelen = 1;}
	else if(memcmp(testname,"uint16",6)==0) {nctype = NC_USHORT; typelen = 1;}
	else if(memcmp(testname,"uint32",6)==0) {nctype = NC_UINT; typelen = 1;}
	else if(memcmp(testname,"uint64",6)==0) {nctype = NC_UINT64; typelen = 1;}
	else if(memcmp(testname,"float32",7)==0) {nctype = NC_FLOAT; typelen = 1;}
	else if(memcmp(testname,"float64",7)==0) {nctype = NC_DOUBLE; typelen = 1;}
	else if(1==sscanf(testname,STRTEMPLATE,&typelen)) {
	    if((typelen % 8) == 0) {
		typelen = typelen / 8; /* convert bits to bytes */
		nctype = NC_STRING;
	    }
	} else {
	    stat = NC_ENOTZARR;
	    goto done;
	}
    }
    if(nctypep) *nctypep = nctype;
    if(typelenp) *typelenp = typelen;

done:
    return THROW(stat);
}

/*
Extract type and data for an attribute from json
*/
static int
computeattrinfo(NC_FILE_INFO_T* file, NC_OBJ* container, const char* aname, const NCjson* jtypes, const NCjson* jdata, struct NCZ_AttrInfo* ainfo)
{
    int stat = NC_NOERR;
    const NCjson* jatype = NULL;

    ZTRACE(3,"container=%s aname=%s",container->name,aname);

    assert(aname != NULL);

    ainfo->name = aname;
    ainfo->nctype = NC_NAT;

    /* Infer the attribute data's type */
    if(jtypes == NULL) {
	if(strcmp(aname,NC_FillValue)==0 && container->sort == NCVAR)
	    ainfo->nctype = ((NC_VAR_INFO_T*)container)->type_info->hdr.id; /* Use the fill value var type */	
	else {
	    if((stat = NCZ_inferattrtype(ainfo->name,ainfo->nctype,jdata,&ainfo->nctype))) goto done;
	}
    } else {
	const char* atype = NULL;
	/* Search the jtypes for the type of this attribute */
	NCJcheck(NCJdictget(jtypes,aname,(NCjson**)&jatype));
	if(jatype == NULL || NCJsort(jatype) != NCJ_STRING) {stat = NC_ENCZARR; goto done;}
	atype = NCJstring(jatype);
	/* d2type == NULL signals that this is an attribute => we only have the dnczarr type */
	if((stat=dtype2nctype(NULL,atype,&ainfo->nctype,&ainfo->typelen))) goto done;
	if(ainfo->nctype >= N_NCZARR_TYPES) {stat = NC_EINTERNAL; goto done;}
    }
    if((stat = NCZ_computeattrdata(file,jdata,ainfo))) goto done;

done:
    return ZUNTRACEX(THROW(stat),"typeid=%d typelen=%d datalen=%u",ainfo->nctype,ainfo->typelen,ainfo->datalen);
}

/* Build a {name,configuration} dict */
static NCjson*
build_named_config(const char* name, int pairs, ...)
{
    int stat = NC_NOERR;
    NCjson* jdict = NULL;
    NCjson* jcfg = NULL;
    va_list ap;
    int i;

    NCJcheck(NCJnew(NCJ_DICT,&jcfg));
    /* Get the varargs */
    va_start(ap, pairs);
    for(i=0;i<pairs;i++) {
	char* key = va_arg(ap, char*);
	NCjson* value = va_arg(ap, NCjson*);
	NCJinsert(jcfg,key,value);
    }
    va_end(ap);
    NCJcheck(NCJnew(NCJ_DICT,&jdict));
    NCJcheck(NCJinsertstring(jdict,"name",name));
    NCJinsert(jdict,"configuration",jcfg); jcfg = NULL;

done:
    NCJreclaim(jcfg);
    if(stat) {NCJreclaim(jdict); jdict = NULL;}
    return jdict;
}

static int
extract_named_config(const NCjson* jpair, const char* field, const NCjson** jvaluep)
{
    int stat = NC_NOERR;
    const NCjson* jname = NULL;
    const NCjson* jcfg = NULL;
    const NCjson* jvalue = NULL;
    assert(NCJsort(jpair)==NCJ_DICT);
    /* Verify that this is a (name,configuration) pair */
    NCJcheck(NCJdictget(jpair,"name",(NCjson**)&jname));
    NCJcheck(NCJdictget(jpair,"configuration",(NCjson**)&jcfg));
    if(jname == NULL || jcfg == NULL || NCJsort(jcfg)!=NCJ_DICT) {stat = NC_ENOTZARR; goto done;}
    /* Allow getting name or configuration */
    if(strcmp("name",field)==0) {jvalue = jname;}
    else if(strcmp("configuratio",field)==0) {jvalue = jcfg;}
    else { /* get a field from the configuration */
	NCJcheck(NCJdictget(jcfg,field,(NCjson**)&jvalue));
    }
    if(jvaluep) {*jvaluep = jvalue;}
done:
    return THROW(stat);
}
