#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "netcdf_filter_build.h"

#ifndef H5Z_FILTER_STRINGN
#define H5Z_FILTER_STRINGN      (H5Z_FILTER_MAX - 1)
#endif

static size_t H5Z__filter_stringn(unsigned flags, size_t cd_nelmts,
    const unsigned cd_values[], size_t nbytes, size_t *buf_size, void **buf);

/* This message derives from H5Z */
const H5Z_class2_t H5Z_STRINGN[1] = {{
    H5Z_CLASS_T_VERS,           /* H5Z_class_t version */
    H5Z_FILTER_STRINGN,		/* Filter id number		*/
    1,                          /* encoder_present flag (set to true) */
    1,                          /* decoder_present flag (set to true) */
    "fixedsizestrings",			/* Filter name for debugging	*/
    NULL,                       /* The "can apply" callback     */
    NULL,
    H5Z__filter_stringn,	/* The actual filter function	*/
}};

/* External Discovery Functions */
DLLEXPORT
H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

DLLEXPORT
const void*
H5PLget_plugin_info(void)
{
    return H5Z_STRINGN;
}

/*-------------------------------------------------------------------------
 * Function:	H5Z__filter_stringn
 *
 * Purpose:	Implement an I/O filter which converts between a block pointers
 *              to char* and a linearized set of strings of max length specified
 *              filter parameter.
 *
 * Return:	Success: Size of buffer filtered
 *		Failure: 0
 *
 * Programmer:	Dennis Heimbigner
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5Z__filter_stringn(unsigned flags, size_t cd_nelmts, const unsigned cd_values[],
                   size_t nbytes, size_t *buf_size, void** buf)
{
    size_t maxstrlen;
    size_t nelem;       /* Number of elements in buffer */
    size_t outsize;
    void* outbuf = NULL;         /* Pointer to new buffer */
    herr_t ret_value = 0;       /* Return value */

    /* Check arguments */
    if (cd_nelmts != 1 || cd_values[0]==0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "invalid stringn parameters")

    maxstrlen = (size_t)cd_values[0];

    if(flags & H5Z_FLAG_REVERSE) { /* convert char[N][maxstrlen]... to char*[N] */
	size_t i;
	char* src = (char*)(*buf);
	char** dst = (char**)(outbuf);
	if((nbytes % maxstrlen) != 0)
	    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "chunksize % maxstrlen != 0")
	nelem = nbytes / maxstrlen; /* Number of char* pointers */
	outsize = sizeof(char*)*nelem;
	if (NULL==(outbuf = H5MM_malloc(outsize)))
	    HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, 0, "memory allocation failed for stringn uncompression")
	for(i=0;i<nelem;i++,src+=maxstrlen) {
	    char* s;
	    if (NULL==(s = H5MM_malloc(maxstrlen+1)))
		HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, 0, "memory allocation failed for string uncompression")
	    memcpy(s,src,maxstrlen);
	    s[maxstrlen] = '\0'; /* nul terminate */
	    dst[i] = s; s = NULL;
	}
        /* reclaim the input buffer */
	H5MM_xfree(*buf);
    } else { /* convert from char*[N] to char[N][maxstrlen] */
	size_t i;
	char** src = (char**)(*buf);
	char* dst;
	if((nbytes % sizeof(char*)) != 0)
	    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "chunksize % maxstrlen != 0")
	nelem = nbytes / sizeof(char); /* Number of char* pointers */
	outsize = maxstrlen * nelem;
	if (NULL==(outbuf = H5MM_malloc(outsize)))
	    HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, 0, "memory allocation failed for stringn compression")
	for(dst=(char*)outbuf,i=0;i<nelem;i++,dst+=maxstrlen) {
	    size_t dstlen = strlen(src[i]);
	    if(dstlen > maxstrlen) dstlen = maxstrlen; /* truncate */
	    memcpy(dst,src,dstlen); /* Note not nul terminated */
	}
	/* Note: src strings are not reclaimed because it is assumed this filter
	   is first in the filter chain when encoding so strings are owned by caller
	   but *buf is created and owned by HDF5.
	*/
        H5MM_xfree(*buf);
    }

    /* Set the buffer information to return */
    *buf = outbuf; outbuf = NULL;
    *buf_size = outsize;

    /* Set the return value */
    ret_value = (int)outsize;

done:
    return (size_t)ret_value;
}

/**************************************************/
/* Codec Interface */

/* Forward */
static int NCZ_stringn_codec_to_hdf5(const NCproplist* env, const char* codec, unsigned* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_stringn_hdf5_to_codec(const NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp);

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_stringn_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "fixedsizestrings",   /* Standard name/id of the codec */
  H5Z_FILTER_STRINGN,   /* HDF5 alias for stringn */
  NULL, /*NCZ_stringn_codec_initialize*/
  NULL, /*NCZ_stringn_codec_finalize*/
  NCZ_stringn_codec_to_hdf5,
  NCZ_stringn_hdf5_to_codec,
  NULL, /*NCZ_stringn_modify_parameters*/
};

/* External Export API */
DLLEXPORT
const void*
NCZ_get_codec_info(void)
{
    return (void*)&NCZ_stringn_codec;
}

/* NCZarr Interface Functions */

static int
NCZ_stringn_codec_to_hdf5(const NCproplist* env, const char* codec_json, unsigned* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    const NCjson* jdict = NULL;
    const NCjson* jtmp = NULL;
    size_t nparams = 0;
    unsigned* params = NULL;
    uintptr_t zarrformat = 0;
    
    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec)<0)
	{stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}

    /* Get and Verify the codec ID */

    if(zarrformat == 3) {
        if(NCJdictget(jcodec,"name",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
        if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
        if(strcmp(NCJstring(jtmp),NCZ_stringn_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
        if(NCJdictget(jcodec,"configuration",(NCjson**)(NCjson**)&jdict)<0) {stat = NC_EFILTER; goto done;}
    } else {
        if(NCJdictget(jcodec,"id",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
        if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
        if(strcmp(NCJstring(jtmp),NCZ_stringn_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
	jdict = jcodec;
    }
    nparams = 1;
    if((params = (unsigned*)calloc(nparams,sizeof(unsigned)))== NULL) {stat = NC_ENOMEM; goto done;}
    /* This filter has one parameter named maxstrlen */
    {
        struct NCJconst jc;
        if(NCJdictget(jdict,"maxstrlen",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
        if(NCJcvt(jtmp,NCJ_INT,&jc)<0) {stat = NC_EFILTER; goto done;}
        if(jc.ival <= 0 || jc.ival > NC_MAX_INT) {stat = NC_EINVAL; goto done;}
        params[0] = (unsigned)jc.ival;
    }

    if(nparamsp) *nparamsp = nparams;
    if(paramsp) {*paramsp = params; params = NULL;}
    if(idp) *idp = (unsigned)(H5Z_FILTER_STRINGN);

done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_stringn_hdf5_to_codec(const NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[8192];
    char value[8192];
    uintptr_t zarrformat = 0;

    NC_UNUSED(id);

    if(nparams != 0 && params == NULL)
        {stat = NC_EINVAL; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(zarrformat == 2) {
        snprintf(json,sizeof(json),"{\"id\": \"%s\"",NCZ_stringn_codec.codecid);
        snprintf(value,sizeof(value),", \"maxstrlen\": \"%u\"",params[0]);
	strcat(json,value);
    } else {
        snprintf(json,sizeof(json),"{\"name\": \"%s\", \"configuration\": {\"maxstrlen\": %u}",
		NCZ_stringn_codec.codecid,params[0]);
    }    
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

