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
#include "netcdf_json.h"

/* Provide Codec information for the standard filters */

#ifndef H5Z_FILTER_BZIP2
#define H5Z_FILTER_BZIP2	1
#define H5Z_FILTER_ZSTD		2
#define H5Z_FILTER_BLOSC	3
#endif

#ifdef HAVE_BLOSC
#include "H5Zblosc.h"
#endif

/**************************************************/
/* NCZarr Filter Objects */

/* Forward */

static int NCZ_bzip2_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_bzip2_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);

#ifdef HAVE_ZSTD
static int NCZ_zstd_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_zstd_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);
#endif

#ifdef HAVE_BLOSC
static int NCZ_blosc_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_blosc_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);
static int NCZ_blosc_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);
static void NCZ_blosc_codec_finalize(void);
#endif

/**************************************************/
/* Provide the codec support for bzip2 filter */

static NCZ_codec_t NCZ_bzip2_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "bz2",	        /* Standard name/id of the codec */
  H5Z_FILTER_BZIP2,   /* HDF5 alias for bzip2 */
  NULL, /*NCZ_bzip2_codec_initialize*/
  NULL, /*NCZ_bzip2_codec_finalize*/
  NCZ_bzip2_codec_to_hdf5,
  NCZ_bzip2_hdf5_to_codec,
  NULL, /*NCZ_bzip2_modify_parameters*/
};

/* External Export API */
DLLEXPORT
const void*
NCZ_get_codec_info(void)
{
    return (void*)&NCZ_bzip2_codec;
}

static int
NCZ_bzip2_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    unsigned* params = NULL;
    struct NCJconst jc;
  
    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}
    
    if((params = (unsigned*)calloc(1,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec))
        {stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}
    /* Verify the codec ID */
    if(NCJdictget(jcodec,"id",&jtmp))
        {stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EFILTER; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_bzip2_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    /* Get Level */
    if(NCJdictget(jcodec,"level",&jtmp))
        {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc))
        {stat = NC_EFILTER; goto done;}
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EINVAL; goto done;}
    params[0] = (unsigned)jc.ival;
    *nparamsp = 1;
    *paramsp = params; params = NULL;
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_bzip2_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    unsigned level = 0;
    char json[1024];

    if(nparams == 0 || params == NULL)
        {stat = NC_EFILTER; goto done;}

    level = params[0];
    snprintf(json,sizeof(json),"{\"id\": \"%s\", \"level\": \"%u\"}",NCZ_bzip2_codec.codecid,level);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

/**************************************************/
/* Provide the codec support for zstandard filter */

#ifdef HAVE_ZSTD

static NCZ_codec_t NCZ_zstd_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "zstd",	        /* Standard name/id of the codec */
  H5Z_FILTER_ZSTD,   /* HDF5 alias for zstd */
  NULL, /*NCZ_zstd_codec_initialize*/
  NULL, /*NCZ_zstd_codec_finalize*/
  NCZ_zstd_codec_to_hdf5,
  NCZ_zstd_hdf5_to_codec,
  NULL, /*NCZ_zstd_modify_parameters*/
};

static int
NCZ_zstd_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    unsigned* params = NULL;
    struct NCJconst jc;
  
    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}
    
    if((params = (unsigned*)calloc(1,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec))
        {stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}
    /* Verify the codec ID */
    if(NCJdictget(jcodec,"id",&jtmp))
        {stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EFILTER; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_zstd_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    /* Get Level */
    if(NCJdictget(jcodec,"level",&jtmp))
        {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc))
        {stat = NC_EFILTER; goto done;}
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EINVAL; goto done;}
    params[0] = (unsigned)jc.ival;
    *nparamsp = 1;
    *paramsp = params; params = NULL;
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_zstd_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    unsigned level = 0;
    char json[1024];

    if(nparams == 0 || params == NULL)
        {stat = NC_EFILTER; goto done;}

    level = params[0];
    snprintf(json,sizeof(json),"{\"id\": \"%s\", \"level\": \"%u\"}",NCZ_zstd_codec.codecid,level);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
#endif

/**************************************************/
/* Provide the codec support for blosc filter */

#ifdef HAVE_BLOSC

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_blosc_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "blosc",	        /* Standard name/id of the codec */
  H5Z_FILTER_BLOSC,    /* HDF5 alias for blosc */
  NULL, /*NCZ_blosc_codec_initialize*/
  NCZ_blosc_codec_finalize,
  NCZ_blosc_codec_to_hdf5,
  NCZ_blosc_hdf5_to_codec,
  NCZ_blosc_modify_parameters,
};

/* NCZarr Interface Functions */

/* Create the true parameter set:
Visible parameters:
param[0] -- reserved
param[1] -- reserved
param[2] -- reserved
param[3] -- variable chunksize in bytes | 0 (=>default)
param[4] -- compression level
param[5] -- BLOSC_SHUFFLE|BLOSC_BITSHUFFLE
param[6] -- compressor index

Working parameters:
param[0] -- filter revision
param[1] -- blosc version
param[2] -- variable type size in bytes
param[3] -- variable chunksize in bytes
param[4] -- compression level
param[5] -- BLOSC_SHUFFLE|BLOSC_BITSHUFFLE
param[6] -- compressor index
*/

void blosc_destroy(void);

static int ncz_blosc_initialized = 0;

static void
NCZ_blosc_codec_finalize(void)
{
    if(ncz_blosc_initialized) {
        blosc_destroy();
	ncz_blosc_initialized = 0;
    }
}

static int
NCZ_blosc_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int i,stat = NC_NOERR;
    nc_type vtype;
    int storage, ndims;
    size_t* chunklens = NULL;
    size_t typesize, chunksize;
    char vname[NC_MAX_NAME+1];
    unsigned* params = NULL;
    size_t nparams;
    size_t vnparams = *vnparamsp;
    unsigned* vparams = *vparamsp;
    
    if(vnparams < 7)
        {stat = NC_EFILTER; goto done;}
    nparams = 7;

    if(vparams == NULL)
        {stat = NC_EFILTER; goto done;}

    if(wnparamsp == NULL || wparamsp == NULL)
        {stat = NC_EFILTER; goto done;}

    vnparams = *vnparamsp;
    vparams = *vparamsp;

    /* Get variable info */
    if((stat = nc_inq_var(ncid,varid,vname,&vtype,&ndims,NULL,NULL))) goto done;

    if(ndims == 0) {stat = NC_EFILTER; goto done;}

    /* Get the typesize */
    if((stat = nc_inq_type(ncid,vtype,NULL,&typesize))) goto done;

    /* Compute chunksize */
    if((chunklens = (size_t*)calloc(ndims,sizeof(size_t)))==NULL) goto done;
    if((stat = nc_inq_var_chunking(ncid,varid,&storage,chunklens))) goto done;
    if(storage != NC_CHUNKED) {stat = NC_EFILTER; goto done;}
    chunksize = typesize;
    for(i=0;i<ndims;i++) chunksize *= chunklens[i];

    if((params = (unsigned*)malloc(vnparams*sizeof(unsigned)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    memcpy(params,vparams,vnparams*sizeof(unsigned));

    params[0] = FILTER_BLOSC_VERSION;
    params[1] = BLOSC_VERSION_FORMAT;
    params[2] = (unsigned)typesize;

    params[3] = chunksize;
    params[4] = params[4];
    params[5] = params[5];
    params[6] = params[6];

    *wnparamsp = nparams;
    nullfree(*wparamsp);
    *wparamsp = params; params = NULL;
    
done:
    nullfree(chunklens);
    nullfree(params);
    FUNC_LEAVE_NOAPI(stat)
}

static int
NCZ_blosc_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    unsigned* params = NULL;
    struct NCJconst jc = {0,0,0,NULL};
    int compcode;

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec)) {stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}
    /* Verify the codec ID */
    if(NCJdictget(jcodec,"id",&jtmp))
        {stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_blosc_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    if((params = (unsigned*)calloc(7,sizeof(unsigned)))==NULL) {stat = NC_ENOMEM; goto done;}

    /* Get compression level*/
    if(NCJdictget(jcodec,"clevel",&jtmp)) {stat = NC_EFILTER; goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_INT,&jc)) {stat = NC_EFILTER;  goto done;}
    } else
        jc.ival = DEFAULT_LEVEL;
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EFILTER; goto done;}
    params[4] = (unsigned)jc.ival;

    /* Get blocksize */
    if(NCJdictget(jcodec,"blocksize",&jtmp)) {stat = NC_EFILTER;  goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_INT,&jc)) {stat = NC_EFILTER; goto done;}
    } else
        jc.ival = DEFAULT_BLOCKSIZE;
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EFILTER; goto done;}
    params[3] = (unsigned)jc.ival;

    /* Get shuffle */
    if(NCJdictget(jcodec,"shuffle",&jtmp)) {stat = NC_EFILTER; goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_INT,&jc)) {stat = NC_EFILTER; goto done;}
    } else
        jc.ival = BLOSC_NOSHUFFLE;
    params[5] = (unsigned)jc.ival;

    /* Get compname */
    if(NCJdictget(jcodec,"cname",&jtmp)) {stat = NC_EFILTER;  goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_STRING,&jc)) {stat = NC_EFILTER; goto done;}
        if(jc.sval == NULL || strlen(jc.sval) == 0) {stat = NC_EFILTER; goto done;}
        if((compcode = blosc_compname_to_compcode(jc.sval)) < 0) {stat = NC_EFILTER; goto done;}
    } else
        compcode = DEFAULT_COMPCODE;
    params[6] = (unsigned)compcode;

    if(nparamsp) *nparamsp = 7;
    if(paramsp) {*paramsp = params; params = NULL;}
    
done:
    if(jc.sval) {
	free(jc.sval);
    }
    if(params) {
        free(params);
    }
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_blosc_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];
    const char* compname = NULL;

    if(nparams == 0 || params == NULL)
        {stat = NC_EINVAL; goto done;}

    /* Get the sub-compressor name */
    if(blosc_compcode_to_compname((int)params[6],&compname) < 0) {stat = NC_EFILTER; goto done;}

    snprintf(json,sizeof(json),
	"{\"id\": \"blosc\",\"clevel\": %u,\"blocksize\": %u,\"cname\": \"%s\",\"shuffle\": %d}",
	params[4],params[3],compname,params[5]);

    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
#endif

/**************************************************/

NCZ_codec_t* NCZ_stdfilters_codecs[] = {
&NCZ_bzip2_codec,
#ifdef HAVE_ZSTD
&NCZ_zstd_codec,
#endif
#ifdef HAVE_BLOSC
&NCZ_blosc_codec,
#endif
NULL
};

/* External Export API */
DLLEXPORT
const void*
NCZ_codec_info_defaults(void)
{
    return (void*)&NCZ_stdfilters_codecs;
}

