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

#define H5Z_FILTER_DEFLATE	1 	/*deflation like gzip	     	*/
#define H5Z_FILTER_SHUFFLE      2       /*shuffle the data              */
#define H5Z_FILTER_FLETCHER32   3       /*fletcher32 checksum of EDC    */

/**************************************************/
/* NCZarr Filter Objects */

/* Forward */
static int NCZ_shuffle_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_shuffle_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);
static int NCZ_shuffle_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

static int NCZ_fletcher32_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_fletcher32_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);
static int NCZ_fletcher32_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

static int NCZ_deflate_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_deflate_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);

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
NCZ_shuffle_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    
    /* Ignore any incoming elementsize */

    if(nparamsp) *nparamsp = 0;
    
    return stat;
}

static int
NCZ_shuffle_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];
    unsigned typesize = 0;
    
    if(nparams > 0)
        typesize = params[0];
    snprintf(json,sizeof(json),"{\"id\": \"%s\", \"elementsize\": \"%u\"}",NCZ_shuffle_codec.codecid,typesize);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

static int
NCZ_shuffle_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int stat = NC_NOERR;
    nc_type vtype;
    size_t typesize;
    char vname[NC_MAX_NAME+1];
    unsigned int* params = NULL;

    /* Ignore the visible parameters */

    if(!wnparamsp || !wparamsp) {stat = NC_EINTERNAL; goto done;}

    /* Get variable info */
    if((stat = nc_inq_var(ncid,varid,vname,&vtype,NULL,NULL,NULL))) goto done;

    /* Get the typesize */
    if((stat = nc_inq_type(ncid,vtype,NULL,&typesize))) goto done;

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

#if 0
static int
NCZ_shuffle_visible_parameters(int ncid, int varid, size_t nparamsin, const unsigned int* paramsin, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    unsigned* params = NULL;
    size_t typesize = 0;
    
    if(!nparamsp || !paramsp) {stat = NC_EINTERNAL; goto done;}

    if(nparamsin > 0)
        typesize = paramsin[0];

    if((params=(unsigned*)malloc(sizeof(unsigned)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    
    params[0] = (unsigned)typesize;

    /* add the typesize as a new parameter */
    nullfree(*paramsp);
    *paramsp = params; params = NULL;
    *nparamsp = 1;

done:
    nullfree(params);
    return stat;
}
#endif

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
NCZ_fletcher32_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;

    NC_UNUSED(codec);
    if(!nparamsp || !paramsp) {stat = NC_EINTERNAL; goto done;}

    *nparamsp = 0;
    nullfree(*paramsp);
    *paramsp = NULL;
    
done:
    return stat;
}

static int
NCZ_fletcher32_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];

    snprintf(json,sizeof(json),"{\"id\": \"%s\"}",NCZ_fletcher32_codec.codecid);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

static int
NCZ_fletcher32_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int stat = NC_NOERR;

    /* Ignore the visible parameters */

    if(!wnparamsp || !wparamsp) {stat = NC_EINTERNAL; goto done;}

    *wnparamsp = 0;
    nullfree(*wparamsp);
    *wparamsp = NULL;

done:
    return stat;
}

#if 0
static int
NCZ_fletcher32_visible_parameters(int ncid, int varid, size_t nparamsin, const unsigned int* paramsin, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;

    if(!nparamsp || !paramsp) {stat = NC_EINTERNAL; goto done;}

    nullfree(*paramsp); *paramsp = NULL;
    *nparamsp = 0;

done:
    return stat;
}
#endif

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
NCZ_deflate_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
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
    if(strcmp(NCJstring(jtmp),NCZ_zlib_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

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
NCZ_deflate_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    unsigned level = 0;
    char json[1024];

    if(nparams == 0 || params == NULL)
        {stat = NC_EFILTER; goto done;}

    level = params[0];
    snprintf(json,sizeof(json),"{\"id\": \"%s\", \"level\": \"%u\"}",NCZ_zlib_codec.codecid,level);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

/**************************************************/

NCZ_codec_t* NCZ_default_codecs[] = {
&NCZ_shuffle_codec,
&NCZ_fletcher32_codec,
&NCZ_zlib_codec,
NULL
};

/* External Export API */
DLLEXPORT
const void*
NCZ_codec_info_defaults(void)
{
    return (void*)&NCZ_default_codecs;
}

