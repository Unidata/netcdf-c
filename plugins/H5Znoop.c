#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>

#include <netcdf_json.h>

#include "netcdf_filter_build.h"

#ifndef NOOP_INSTANCE
#define NOOP_INSTANCE 0
#endif

#if NOOP_INSTANCE == 1
const static int instance[1] = {1};
#endif
#if NOOP_INSTANCE == 0
const static int instance[1] = {0};
#endif

/* use a temporary */
#define H5Z_FILTER_NOOP 40000

/* WARNING:
Starting with HDF5 version 1.10.x, the plugin code MUST be
careful when using the standard *malloc()*, *realloc()*, and
*free()* function.

In the event that the code is allocating, reallocating, for
free'ing memory that either came from or will be exported to the
calling HDF5 library, then one MUST use the corresponding HDF5
functions *H5allocate_memory()*, *H5resize_memory()*,
*H5free_memory()* [5] to avoid memory failures.

Additionally, if your filter code leaks memory, then the HDF5 library
will generate an error.

*/

#undef DEBUG

extern void NC_h5filterspec_fix8(void* mem, int decode);

static htri_t H5Z_noop_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static size_t H5Z_filter_noop(unsigned int, size_t, const unsigned int cd_values[], size_t, size_t*, void**);

static H5Z_class2_t H5Z_NOOP[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
#if NOOP_INSTANCE == 0
    (H5Z_filter_t)(H5Z_FILTER_NOOP), /* Filter id number */
#else
    (H5Z_filter_t)(H5Z_FILTER_NOOP+1), /* Filter id number */
#endif
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
#if NOOP_INSTANCE == 0
    "noop",                          /*  Filter name for debugging    */
#else
    "noop1",                         /*  Filter name for debugging    */
#endif
    (H5Z_can_apply_func_t)H5Z_noop_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_noop,     /* The actual filter function   */
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
    return H5Z_NOOP;
}

/* Make this explicit */
/*
 * The "can_apply" callback returns positive a valid combination, zero for an
 * invalid combination and negative for an error.
 */
static htri_t
H5Z_noop_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* Assume it can always apply */
}

static size_t
H5Z_filter_noop(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    void* newbuf;
    size_t i;    
    const char* direction = (flags & H5Z_FLAG_REVERSE) ? "decompress" : "compress";
    
    NC_UNUSED(instance);

    printf("direction=%s id=%lu cd_nelmts=%lu cd_values=",direction,(unsigned long)H5Z_NOOP[0].id,(unsigned long)cd_nelmts);
    for(i=0;i<cd_nelmts;i++)
	printf(" %u",cd_values[i]);
    printf("\n");
    fflush(stdout);

    if (flags & H5Z_FLAG_REVERSE) {
        /* Replace buffer */
        newbuf = H5allocate_memory(*buf_size,0);
        if(newbuf == NULL) abort();
        memcpy(newbuf,*buf,*buf_size);
        /* reclaim old buffer */
        H5free_memory(*buf);
        *buf = newbuf;

    } else {
    /* Replace buffer */
      newbuf = H5allocate_memory(*buf_size,0);
      if(newbuf == NULL) abort();
        memcpy(newbuf,*buf,*buf_size);
	/* reclaim old buffer */
        H5free_memory(*buf);
        *buf = newbuf;

    }

    return *buf_size;
}

/**************************************************/
/* NCZarr Codec API */

/* Codec Format
{
"id": "test",
"p0": "<unsigned int>",
"p1": "<unsigned int>",
"p2": "<unsigned int>",
...
"pn": "<unsigned int>",
}
*/

/* Forward */
static int NCZ_noop_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_noop_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_noop_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
#if NOOP_INSTANCE == 0
  "noop",	        /* Standard name/id of the codec */
  H5Z_FILTER_NOOP,     /* HDF5 alias for noop */
#else
  "noop1",	        /* Standard name/id of the codec */
  H5Z_FILTER_NOOP+1,     /* HDF5 alias for noop */
#endif
  NULL, /*NCZ_noop_codec_initialize*/
  NULL, /*NCZ_noop_codec_finalize*/
  NCZ_noop_codec_to_hdf5,
  NCZ_noop_hdf5_to_codec,
  NULL, /*NCZ_noop_modify_parameters*/
};

/* External Export API */
DLLEXPORT
const void*
NCZ_get_codec_info(void)
{
    return (void*)&NCZ_noop_codec;
}

/* NCZarr Interface Functions */

static int
NCZ_noop_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    int i,nparams = 0;
    unsigned* params = NULL;
    char field[1024];

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec))
	{stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}

    /* Verify the codec ID */
    if(NCJdictget(jcodec,"id",&jtmp))
	{stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_noop_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
  
    nparams = (NCJlength(jcodec) - 1) / 2; /* -1 for id each param is key+value */

    if((params = (unsigned*)calloc(nparams,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    /* This filter has an arbitrary number of parameters named p0...pn */

    for(i=0;i<nparams;i++) {
	struct NCJconst jc;
	snprintf(field,sizeof(field),"p%d",i);
        if(NCJdictget(jcodec,field,&jtmp))
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
NCZ_noop_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int i,stat = NC_NOERR;
    char json[8192];
    char value[1024];

    if(nparams != 0 && params == NULL)
        {stat = NC_EINVAL; goto done;}

    snprintf(json,sizeof(json),"{\"id\": \"%s\"",NCZ_noop_codec.codecid);
    for(i=0;i<nparams;i++) {
        snprintf(value,sizeof(value),", \"p%d\": \"%u\"",i,params[i]);
	strlcat(json,value,sizeof(json));
    }
    strlcat(json,"}",sizeof(json));
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
