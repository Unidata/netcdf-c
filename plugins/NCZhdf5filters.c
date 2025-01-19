/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */

/*
Author: Dennis Heimbigner
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "netcdf.h"
#include "netcdf_filter.h"
#include "netcdf_filter_build.h"
#include "netcdf_vutils.h"
#include "h5misc.h"

#ifdef HAVE_SZ
#include <szlib.h>
#include "H5Zszip.h"
#endif

#define H5Z_FILTER_DEFLATE	1 	/*deflation like gzip	     	*/
#define H5Z_FILTER_SHUFFLE      2       /*shuffle the data              */
#define H5Z_FILTER_FLETCHER32   3       /*fletcher32 checksum of EDC    */

/**************************************************/
/* NCZarr Filter Objects */

/* Forward */

static int NCZ_raw_codec_to_hdf5(const NCproplist*, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_raw_hdf5_to_codec(const NCproplist*, int id, size_t nparams, const unsigned* params, char** codecp);

static int NCZ_shuffle_codec_to_hdf5(const NCproplist*, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_shuffle_hdf5_to_codec(const NCproplist*, int id, size_t nparams, const unsigned* params, char** codecp);
static int NCZ_shuffle_modify_parameters(const NCproplist*, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

static int NCZ_fletcher32_codec_to_hdf5(const NCproplist*, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_fletcher32_hdf5_to_codec(const NCproplist*, int id, size_t nparams, const unsigned* params, char** codecp);
static int NCZ_fletcher32_modify_parameters(const NCproplist*, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

static int NCZ_deflate_codec_to_hdf5(const NCproplist*, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_deflate_hdf5_to_codec(const NCproplist*, int id, size_t nparams, const unsigned* params, char** codecp);

#ifdef HAVE_SZ
static int NCZ_szip_codec_to_hdf5(const NCproplist*, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_szip_hdf5_to_codec(const NCproplist*, int id, size_t nparams, const unsigned* params, char** codecp);
static int NCZ_szip_modify_parameters(const NCproplist*, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);
#endif

/**************************************************/
/* Provide a default codec for HDF5 filters.
   This encodes the parameters of the HDF5 filter
   into one of these two codec forms:
   zarr v2: {"id": "<hdf5-id>", "hdf5raw": "<version>", "nparams": <uint-hex>, "0": <uint-hex>...,"<N>": <uint-hex>}
   zarr v3: {"name": "<hdf5-id>, "configuration": {"hdf5raw": "<version>", "nparams": <uint-hex>, "0": <uint-hex>...,"<N>": <uint-hex>}}
   This codec is called hdf5raw.
   NOTES:
   1. this cannot be used if a modify_parameters function is required.
*/

static NCZ_codec_t NCZ_hdf5_raw_codec = {
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  H5Z_CODEC_RAW,        /* Standard name/id of the codec */
  H5Z_FILTER_RAW,	/* HDF5 alias for raw */
  NULL,			/*NCZ_raw_codec_initialize*/
  NULL,			/*NCZ_raw_codec_finalize*/
  NCZ_raw_codec_to_hdf5,
  NCZ_raw_hdf5_to_codec,
  NULL
};

static int
NCZ_raw_codec_to_hdf5(const NCproplist* env, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    const NCjson* jname = NULL;
    const NCjson* jdict = NULL;
    const char* name = NULL;
    uintptr_t zarrformat = 0;
    int hdf5id = 0;
    size_t nparams = 0;
    unsigned* params = NULL;
  
    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}
    
    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    /* parse the JSON */
    if(NCJparse(codec,0,&jcodec)<0) {stat = NC_EFILTER; goto done;}
    if(jcodec == NULL || NCJsort(jcodec) != NCJ_DICT) {stat = NC_EFILTER; goto done;}

    if(zarrformat == 3) {
        if(NCJdictget(jcodec,"name",(NCjson**)&jname)<0) {stat = NC_EFILTER; goto done;}
	if(NCJdictget(jcodec,"configuration",(NCjson**)&jdict)<0) {stat = NC_EFILTER; goto done;}
    } else {
        if(NCJdictget(jcodec,"id",(NCjson**)&jname)<0) {stat = NC_EFILTER; goto done;}
        jdict = jcodec;
    }

    /* Verify the codec ID */
    if(jdict == NULL || jname == NULL || NCraw_test(jdict)<0) {stat = NC_EFILTER; goto done;}
    if(NCJsort(jname) != NCJ_INT) {stat = NC_EFILTER; goto done;}
    name = NCJstring(jname);
    if(1 != sscanf(name,"%d",&hdf5id)) {stat = NC_EINVAL; goto done;}
    /* Return the id */
    if(idp) *idp = hdf5id;

    /* verify that the codec is in rawformat */
    if(NCraw_test(jdict)<0) {stat = NC_EFILTER; goto done;}

    /* Decode */
    if((stat = NCraw_decode(jdict,&nparams,&params))) goto done;

    if(idp) *idp = hdf5id;
    *nparamsp = nparams;
    *paramsp = params; params = NULL;
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_raw_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char* jsonstr = NULL;
    uintptr_t zarrformat = 0;
    NCjson* jcodec = NULL;
    NCjson* jcfg = NULL;
    char digits[64];

    if(codecp == NULL) {stat = NC_EINTERNAL; goto done;}
    
    if(nparams != 0 && params == NULL)
        {stat = NC_EINVAL; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    snprintf(digits,sizeof(digits),"%u",id);/* Compute id/name */
    if((stat = NCraw_encode(nparams,params,&jcfg))) goto done;

    if(zarrformat == 3) {
	jcodec = jcfg; jcfg = NULL;
	NCJinsertstring(jcodec,"id",digits);
    } else {
	NCJcheck(NCJnew(NCJ_DICT,&jcodec));
	NCJinsertstring(jcodec,"name",digits);
	NCJinsert(jcodec,"configuration",jcfg);
    }

    NCJcheck(NCJunparse(jcodec,0,&jsonstr));
    if(codecp) {*codecp = jsonstr; jsonstr = NULL;}
    
done:
    if(jsonstr) free(jsonstr);
    return stat;
}

/**************************************************/

static NCZ_codec_t NCZ_shuffle_codec = {
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "shuffle",	        /* Standard name/id of the codec */
  H5Z_FILTER_SHUFFLE,   /* HDF5 alias for shuffle */
  NULL, /*NCZ_shuffle_codec_initialize*/
  NULL, /*NCZ_shuffle_codec_finalize*/
  NCZ_shuffle_codec_to_hdf5,
  NCZ_shuffle_hdf5_to_codec,
  NCZ_shuffle_modify_parameters,
};

static int
NCZ_shuffle_codec_to_hdf5(const NCproplist* env, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;

    NC_UNUSED(env);
    NC_UNUSED(codec);

    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}

    if(idp) *idp = H5Z_FILTER_SHUFFLE;
    *nparamsp = 0;
    *paramsp = NULL;
done:    
    return stat;
}

static int
NCZ_shuffle_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];
    uintptr_t zarrformat = 0;
    
    NC_UNUSED(id);
    NC_UNUSED(nparams);
    NC_UNUSED(params);
    
    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(zarrformat == 3) {
        snprintf(json,sizeof(json),"{\"name\": \"%s\"}",NCZ_shuffle_codec.codecid);
    } else {
        snprintf(json,sizeof(json),"{\"id\": \"%s\"}",NCZ_shuffle_codec.codecid);
    }

    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

static int
NCZ_shuffle_modify_parameters(const NCproplist* env, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int stat = NC_NOERR;
    nc_type vtype;
    size_t typesize;
    char vname[NC_MAX_NAME+1];
    unsigned int* params = NULL;
    uintptr_t ncid, varid;

    NC_UNUSED(idp);
    NC_UNUSED(vnparamsp);
    NC_UNUSED(vparamsp);

    /* Ignore the visible parameters */

    if(!wnparamsp || !wparamsp) {stat = NC_EINTERNAL; goto done;}

    ncproplistget(env,"ncid",&ncid,NULL);
    ncproplistget(env,"varid",&varid,NULL);

    /* Get variable info */
    if((stat = nc_inq_var((int)ncid,(int)varid,vname,&vtype,NULL,NULL,NULL))) goto done;

    /* Get the typesize */
    if((stat = nc_inq_type((int)ncid,vtype,NULL,&typesize))) goto done;

    if((params=(unsigned*)malloc(sizeof(unsigned)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    params[0] = (unsigned)typesize;

    if(wnparamsp) *wnparamsp = 1;
    if(wparamsp) {
        nullfree(*wparamsp);
        *wparamsp = params; params = NULL;
    }

done:
    nullfree(params);
    return stat;
}

/**************************************************/

static NCZ_codec_t NCZ_fletcher32_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "fletcher32",	        /* Standard name/id of the codec */
  H5Z_FILTER_FLETCHER32,   /* HDF5 alias for zlib */
  NULL, /*NCZ_fletcher32_codec_initialize*/
  NULL, /*NCZ_fletcher32_codec_finalize*/
  NCZ_fletcher32_codec_to_hdf5,
  NCZ_fletcher32_hdf5_to_codec,
  NCZ_fletcher32_modify_parameters,
};

static int
NCZ_fletcher32_codec_to_hdf5(const NCproplist* env, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;

    NC_UNUSED(codec);
    NC_UNUSED(env);
    NC_UNUSED(idp);

    if(!nparamsp || !paramsp) {stat = NC_EINTERNAL; goto done;}

    *nparamsp = 0;
    *paramsp = NULL;
    
done:
    return stat;
}

static int
NCZ_fletcher32_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];
    uintptr_t zarrformat = 0;

    NC_UNUSED(id);
    NC_UNUSED(nparams);
    NC_UNUSED(params);

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(zarrformat == 3) {
        snprintf(json,sizeof(json),"{\"name\": \"%s\"}",NCZ_fletcher32_codec.codecid);
    } else {
        snprintf(json,sizeof(json),"{\"id\": \"%s\"}",NCZ_fletcher32_codec.codecid);
    }
    
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

static int
NCZ_fletcher32_modify_parameters(const NCproplist* env, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int stat = NC_NOERR;

    NC_UNUSED(env);
    NC_UNUSED(idp);
    NC_UNUSED(vnparamsp);
    NC_UNUSED(vparamsp);

    /* Ignore the visible parameters */

    if(!wnparamsp || !wparamsp) {stat = NC_EINTERNAL; goto done;}

    *wnparamsp = 0;
    nullfree(*wparamsp);
    *wparamsp = NULL;

done:
    return stat;
}

/**************************************************/

static NCZ_codec_t NCZ_zlib_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "zlib",	        /* Standard name/id of the codec */
  H5Z_FILTER_DEFLATE,   /* HDF5 alias for zlib */
  NULL, /*NCZ_deflate_codec_initialize*/
  NULL, /*NCZ_deflate_codec_finalize*/
  NCZ_deflate_codec_to_hdf5,
  NCZ_deflate_hdf5_to_codec,
  NULL, /*NCZ_deflate_modify_parameters*/
};

static int
NCZ_deflate_codec_to_hdf5(const NCproplist* env, const char* codec_json, int* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    const NCjson* jtmp = NULL;
    const NCjson* jdict = NULL;
    unsigned* params = NULL;
    struct NCJconst jc;
    uintptr_t zarrformat = 0;
  
    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}
    
    if((params = (unsigned*)calloc(1,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec)<0)
        {stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}

    if(zarrformat == 3) {
        if(NCJdictget(jcodec,"name",(NCjson**)&jtmp)<0){stat = NC_EFILTER; goto done;}
        if(NCJdictget(jcodec,"configuration",(NCjson**)&jdict)<0){stat = NC_EFILTER; goto done;}
    } else {
        if(NCJdictget(jcodec,"id",(NCjson**)&jtmp)<0){stat = NC_EFILTER; goto done;}
	jdict = jcodec;
    }
    if(jdict == NULL){stat = NC_EFILTER; goto done;}
    
    /* Verify the codec ID */
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EFILTER; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_zlib_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    /* Get Level */
    if(NCJdictget(jdict,"level",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc)<0) {stat = NC_EFILTER; goto done;}
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EINVAL; goto done;}

    if(idp) *idp = H5Z_FILTER_DEFLATE;
    params[0] = (unsigned)jc.ival;
    *nparamsp = 1;
    *paramsp = params; params = NULL;
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_deflate_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    unsigned level = 0;
    char json[1024];
    uintptr_t zarrformat = 0;

    NC_UNUSED(id);

    if(nparams == 0 || params == NULL)
        {stat = NC_EFILTER; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    level = params[0];
    if(zarrformat == 3) {
        snprintf(json,sizeof(json),"{\"name\": \"%s\", \"configuration\": {\"level\": %u}}",NCZ_zlib_codec.codecid,level);
    } else {
        snprintf(json,sizeof(json),"{\"id\": \"%s\", \"level\": %u}",NCZ_zlib_codec.codecid,level);
    }

    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

/**************************************************/

#ifdef HAVE_SZ

static NCZ_codec_t NCZ_szip_codec = {
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "szip",	        /* Standard name/id of the codec */
  H5Z_FILTER_SZIP,   /* HDF5 alias for szip */
  NULL, /*NCZ_szip_codec_initialize*/
  NULL, /*NCZ_szip_codec_finalize*/
  NCZ_szip_codec_to_hdf5,
  NCZ_szip_hdf5_to_codec,
  NCZ_szip_modify_parameters,
};

static int
NCZ_szip_codec_to_hdf5(const NCproplist* env, const char* codec_json, int* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    unsigned* params = NULL;
    size_t nparams = 2; /* No. of visible parameters */
    NCjson* json = NULL;
    const NCjson* jdict = NULL;
    const NCjson* jtmp = NULL;
    struct NCJconst jc = {0,0,0,NULL};
    uintptr_t zarrformat = 0;
    
    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}

    if((params = (unsigned*)calloc(nparams,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(NCJparse(codec_json,0,&json)<0)
        {stat = NC_EFILTER; goto done;}

    if(zarrformat == 3) {
	if(NCJdictget(json,"configuration",(NCjson**)&jdict)<0) {stat = NC_EFILTER; goto done;}
    } else
        jdict = json;

    if(NCJdictget(jdict,"mask",(NCjson**)&jtmp) < 0 || jtmp == NULL) {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc)<0) {stat = NC_EFILTER;  goto done;}
    params[H5Z_SZIP_PARM_MASK] = (unsigned)jc.ival;

    jtmp = NULL;
    if(NCJdictget(jdict,"pixels-per-block",(NCjson**)&jtmp) < 0 || jtmp == NULL) {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc)<0) {stat = NC_EFILTER;  goto done;}
    params[H5Z_SZIP_PARM_PPB] = (unsigned)jc.ival;

    if(idp) *idp = H5Z_FILTER_SZIP;
    *nparamsp = nparams;
    *paramsp = params; params = NULL;
    
done:
    NCJreclaim(json);
    nullfree(params);
    return stat;
}

static int
NCZ_szip_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[2048];
    uintptr_t zarrformat = 0;

    NC_UNUSED(id);
    NC_UNUSED(nparams);
    
    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(zarrformat == 3) {
        snprintf(json,sizeof(json),"{\"name\": \"%s\", \"configuration\": {\"mask\": %u, \"pixels-per-block\": %u}}",
    		NCZ_szip_codec.codecid,
		params[H5Z_SZIP_PARM_MASK],
		params[H5Z_SZIP_PARM_PPB]);
    } else {
        snprintf(json,sizeof(json),"{\"id\": \"%s\", \"mask\": %u, \"pixels-per-block\": %u}",
    		NCZ_szip_codec.codecid,
		params[H5Z_SZIP_PARM_MASK],
		params[H5Z_SZIP_PARM_PPB]);
    }

    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

static int
NCZ_szip_modify_parameters(const NCproplist* env, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int i,ret_value = NC_NOERR;
    nc_type vtype;
    size_t typesize, scanline, dtype_precision, npoints;
    int ndims, storage, dtype_order;
    int dimids[NC_MAX_VAR_DIMS];
    char vname[NC_MAX_NAME+1];
    size_t chunklens[NC_MAX_VAR_DIMS];
    unsigned* params = NULL;
    unsigned* vparams = NULL;
    size_t wnparams = 4;
    uintptr_t ncid, varid;
    
    NC_UNUSED(idp);

    if(wnparamsp == NULL || wparamsp == NULL)
        {ret_value = NC_EFILTER; goto done;}
    if(vnparamsp == NULL || vparamsp == NULL)
        {ret_value = NC_EFILTER; goto done;}
    if(*vnparamsp > 0 && *vparamsp == NULL)
        {ret_value = NC_EFILTER; goto done;}

    vparams = *vparamsp;

    ncproplistget(env,"ncid",&ncid,NULL);
    ncproplistget(env,"varid",&varid,NULL);

    /* Get variable info */
    if((ret_value = nc_inq_var((int)ncid,(int)varid,vname,&vtype,&ndims,dimids,NULL))) goto done;

    /* Get the typesize */
    if((ret_value = nc_inq_type((int)ncid,vtype,NULL,&typesize))) goto done;

    /* Get datatype's precision, in case is less than full bits  */
    dtype_precision = typesize*8;

    if(dtype_precision > 24) {
        if(dtype_precision <= 32)
            dtype_precision = 32;
        else if(dtype_precision <= 64)
            dtype_precision = 64;
    } /* end if */

    if(ndims == 0) {ret_value = NC_EFILTER; goto done;}

    /* Set "local" parameter for this dataset's "pixels-per-scanline" */
    if((ret_value = nc_inq_dimlen((int)ncid,dimids[ndims-1],&scanline))) goto done;

    /* Get number of elements for the dataspace;  use
       total number of elements in the chunk to define the new 'scanline' size */
    /* Compute chunksize */
    if((ret_value = nc_inq_var_chunking((int)ncid,(int)varid,&storage,chunklens))) goto done;
    if(storage != NC_CHUNKED) {ret_value = NC_EFILTER; goto done;}
    npoints = 1;
    for(i=0;i<ndims;i++) npoints *= chunklens[i];

    /* Get datatype's endianness order */
    if((ret_value = nc_inq_var_endian((int)ncid,(int)varid,&dtype_order))) goto done;

    if((params = (unsigned*)malloc(wnparams*sizeof(unsigned)))==NULL)
        {ret_value = NC_ENOMEM; goto done;}
    params[H5Z_SZIP_PARM_MASK] = vparams[H5Z_SZIP_PARM_MASK];
    params[H5Z_SZIP_PARM_PPB] = vparams[H5Z_SZIP_PARM_PPB];

    /* Set "local" parameter for this dataset's "bits-per-pixel" */
    params[H5Z_SZIP_PARM_BPP] = (unsigned)dtype_precision;

    /* Adjust scanline if it is smaller than number of pixels per block or
       if it is bigger than maximum pixels per scanline, or there are more than
       SZ_MAX_BLOCKS_PER_SCANLINE blocks per scanline  */
    if(scanline < vparams[H5Z_SZIP_PARM_PPB]) {
        if(npoints < vparams[H5Z_SZIP_PARM_PPB])
	    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "pixels per block greater than total number of elements in the chunk")
	scanline = MIN((vparams[H5Z_SZIP_PARM_PPB] * SZ_MAX_BLOCKS_PER_SCANLINE), npoints);
    } else {
        if(scanline <= SZ_MAX_PIXELS_PER_SCANLINE)
            scanline = MIN((vparams[H5Z_SZIP_PARM_PPB] * SZ_MAX_BLOCKS_PER_SCANLINE), scanline);
        else
            scanline = vparams[H5Z_SZIP_PARM_PPB] * SZ_MAX_BLOCKS_PER_SCANLINE;
    } /* end else */
    /* Assign the final value to the scanline */
    params[H5Z_SZIP_PARM_PPS] = (unsigned)scanline;

    /* Set the correct mask flags */

    /* From H5Pdcpl.c#H5Pset_szip */
    params[H5Z_SZIP_PARM_MASK] &= (unsigned)(~H5_SZIP_CHIP_OPTION_MASK);
    params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_ALLOW_K13_OPTION_MASK;
    params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_RAW_OPTION_MASK;
    params[H5Z_SZIP_PARM_MASK] &= (unsigned)(~(H5_SZIP_LSB_OPTION_MASK | H5_SZIP_MSB_OPTION_MASK));

    /* From H5Zszip.c#H5Z__set_local_szip */
    params[H5Z_SZIP_PARM_MASK] &= (unsigned)(~(H5_SZIP_LSB_OPTION_MASK | H5_SZIP_MSB_OPTION_MASK));
    switch(dtype_order) {
    case NC_ENDIAN_LITTLE:      /* Little-endian byte order */
        params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_LSB_OPTION_MASK;
        break;
    case NC_ENDIAN_BIG:      /* Big-endian byte order */
        params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_MSB_OPTION_MASK;
        break;
    default:
        HGOTO_ERROR(H5E_PLINE, H5E_BADTYPE, FAIL, "bad datatype endianness order")
    } /* end switch */

    *wnparamsp = wnparams;
    nullfree(*wparamsp);
    *wparamsp = params; params = NULL;
    
done:
    nullfree(params);
    FUNC_LEAVE_NOAPI(ret_value)
}

#endif /*HAVE_SZ*/

/**************************************************/

NCZ_codec_t* NCZ_default_codecs[] = {
&NCZ_hdf5_raw_codec, /* default codec for hdf5 filters */
&NCZ_shuffle_codec,
&NCZ_fletcher32_codec,
&NCZ_zlib_codec,
#ifdef HAVE_SZ
&NCZ_szip_codec,
#endif
NULL
};

/* External Export API */
DLLEXPORT
const void*
NCZ_codec_info_defaults(void)
{
    return (void*)&NCZ_default_codecs;
}
