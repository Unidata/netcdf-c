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
 * Programmer:  Robb Matzke
 *              Friday, August 27, 1999
 */

/* Converted to NCZarr support by Dennis Heimbigner 5/1/2021 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "netcdf_json.h"

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
static int NCZ_misc_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_misc_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);

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
NCZ_misc_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    size_t i,nparams = 0;
    unsigned* params = NULL;

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec))
	{stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}

    /* Verify the codec ID */
    if(NCJdictget(jcodec,"id",&jtmp))
	{stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_misc_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
  
    /* The codec will have (2*14 + 1) +1 = 29 dict entries + id*/
    nparams = (2*14 + 1) + 1;
    if(NCJlength(jcodec) != nparams) {
	fprintf(stderr,"Incorrect no. of codec parameters: need=29 sent=%ld\n",(unsigned long)(nparams-1));
	stat = NC_EINVAL;
	goto done;
    }
    
    /* Actual # of parameters is 14 (ignoring the testcase number) */
    nparams = 14;
    if((params = (unsigned*)calloc(nparams,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    for(i=0;i<nparams;i++) {
	struct NCJconst jc;
        if(NCJdictget(jcodec,fields[i],&jtmp))
	    {stat = NC_EFILTER; goto done;}
	if(NCJcvt(jtmp,NCJ_INT,&jc))
	    {stat = NC_EFILTER; goto done;}
	if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EINVAL; goto done;}
	params[i] = (unsigned)jc.ival;
    }
    if(nparamsp) *nparamsp = nparams;
    if(paramsp) {*paramsp = params; params = NULL;}
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_misc_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int i,stat = NC_NOERR;
    char json[4096];
    char value[1024];

    if(nparams == 0 || params == NULL)
        {stat = NC_EINVAL; goto done;}
    if(nparams != 14) {
	fprintf(stderr,"Incorrect no. of parameters: need=14 sent=%ld\n",(unsigned long)nparams);
	stat = NC_EINVAL;
	goto done;
    }
    snprintf(json,sizeof(json),"{\"id\": \"%s\"",NCZ_misc_codec.codecid);
    for(i=0;i<14;i++) {
        snprintf(value,sizeof(value),", \"%s\": \"%u\"",fields[i],params[i]);
	strlcat(json,value,sizeof(json));
    }
    strlcat(json,"}",sizeof(json));
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
