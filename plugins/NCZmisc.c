/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Programmer:  Dennis Heimbigner
 */

/* Converted to NCZarr support by Dennis Heimbigner 5/1/2021 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "netcdf.h"
#include "netcdf_filter.h"
#include "netcdf_filter_build.h"

#include "h5misc.h"

/**************************************************/
/* NCZarr Filter Objects */
/* Codec Format
{
"id": "test",
"testcase": "n",
"byte": "<unsigned int>",
"ubyte": "<unsigned int>",
"short": "<unsigned int>",
"ushort": "<unsigned int>",
"int": "<unsigned int>",
"uint": "<unsigned int>",
"float": "<unsigned int>",
"double0": "<unsigned int>",
"double1": "<unsigned int>",
"int640": "<unsigned int>",
"int641": "<unsigned int>",
"uint640": "<unsigned int>",
"uint641": "<unsigned int>",
}
*/

/* Give unique dict key names for parameters */
static const char* fields[14] = {
"testcase",
"byte",
"ubyte",
"short",
"ushort",
"int",
"uint",
"float",
"double_0",
"double_1",
"int64_0",
"int64_1",
"uint64_0",
"uint64_1"
};

/* Forward */
static int NCZ_misc_codec_to_hdf5(const NCproplist* env, const char* codec, int* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_misc_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp);

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_misc_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "test",	        /* Standard name/id of the codec */
  H5Z_FILTER_TEST,     /* HDF5 alias for misc */
  NULL, /*NCZ_misc_codec_initialize*/
  NULL, /*NCZ_misc_codec_finalize*/
  NCZ_misc_codec_to_hdf5,
  NCZ_misc_hdf5_to_codec,
  NULL, /*NCZ_misc_modify_parameters*/
};

/* External Export API */
DLLEXPORT
const void*
NCZ_get_codec_info(void)
{
    return (void*)&NCZ_misc_codec;
}

/* NCZarr Interface Functions */

static int
NCZ_misc_codec_to_hdf5(const NCproplist* env, const char* codec_json, int* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    const NCjson* jparams = NULL;
    const NCjson* jtmp = NULL;
    size_t i,npairs,dictpairs;
    unsigned* params = NULL;
    int isv3 = 0;
    uintptr_t zarrformat = 0;

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(zarrformat == 3) isv3 = 1;

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec)<0)
	{stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}

    /* Verify the codec ID */

    if(zarrformat == 3) {
        if(NCJdictget(jcodec,"name",(NCjson**)&jtmp) < 0) {stat = NC_EFILTER; goto done;}
    } else {
        if(NCJdictget(jcodec,"id",(NCjson**)&jtmp) < 0) {stat = NC_EFILTER; goto done;}
    }
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_misc_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
  
    if(zarrformat == 3) {
	if(NCJdictget(jcodec,"configuration",(NCjson**)&jparams) < 0) {stat = NC_EFILTER; goto done;}
    } else { /* V2 */
        jparams = jcodec;
    }

    /* The codec will have:
        v2: (14 + 1) == 14 dict entries + id
        v3: (14) == 14 dict entries in configuration
    */
    /* verify the dict size */
    if(isv3) dictpairs = (14); else dictpairs = (14+1);
    if(NCJdictlength(jparams) != dictpairs) {
	fprintf(stderr,"(1) Incorrect no. of codec parameters: need=%d sent=%zu\n",(int)dictpairs,NCJdictlength(jparams));
	fprintf(stderr,"jparams=%s\n",NCJtotext(jparams,0));
        stat = NC_EINVAL;
	goto done;
    }
    npairs = (14); /* ignore the test key. */
    if((params = (unsigned*)calloc(npairs,sizeof(unsigned)))== NULL) {stat = NC_ENOMEM; goto done;}

    for(i=0;i<npairs;i++) {
	struct NCJconst jc;
        if(NCJdictget(jparams,fields[i],(NCjson**)&jtmp) < 0) {stat = NC_EFILTER; goto done;}
	if(NCJcvt(jtmp,NCJ_INT,&jc)<0) {stat = NC_EFILTER; goto done;}
	if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EINVAL; goto done;}
	params[i] = (unsigned)jc.ival;
    }
    if(nparamsp) *nparamsp = npairs;
    if(paramsp) {*paramsp = params; params = NULL;}
    if(idp) *idp = H5Z_FILTER_TEST;
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_misc_hdf5_to_codec(const NCproplist* env, int id, size_t nparams, const unsigned* params, char** codecp)
{
    int i,stat = NC_NOERR;
    char json[4096];
    char value[1024];
    size_t count, jlen;
    uintptr_t zarrformat = 0;
    
    NC_UNUSED(id);
    
    if(nparams == 0 || params == NULL)
        {stat = NC_EINVAL; goto done;}
    if(nparams != 14) {
	fprintf(stderr,"(2) Incorrect no. of parameters: need=14 sent=%ld\n",(unsigned long)nparams);
	stat = NC_EINVAL;
	goto done;
    }

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    jlen = sizeof(json);
    if(zarrformat == 2) {
        count = (size_t)snprintf(json,sizeof(json),"{\"id\": \"%s\"",NCZ_misc_codec.codecid);
        for(i=0;i<14;i++) {
            size_t len = (size_t)snprintf(value,sizeof(value),", \"%s\": \"%u\"",fields[i],params[i]);
	    count += len; assert(jlen > count);
	    strcat(json,value);
        }
        count += 1; assert(jlen > count);
    } else if(zarrformat == 3) {
        snprintf(json,sizeof(json),"{\"name\": \"%s\", \"configuration\": {",NCZ_misc_codec.codecid);
        for(i=0;i<14;i++) {
	    if(i > 0) strcat(json,", ");
            snprintf(value,sizeof(value),"\"%s\": \"%u\"",fields[i],params[i]);
	    strcat(json,value);
        }
	strcat(json,"}");
    }
    strcat(json,"}");
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
