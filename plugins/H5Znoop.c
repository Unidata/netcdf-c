#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>

#include "netcdf_filter_build.h"
#include "h5misc.h"

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
    NC_UNUSED(dcpl_id);
    NC_UNUSED(type_id);
    NC_UNUSED(space_id);
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
    NC_UNUSED(nbytes);

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

/* This codec test various things:
test=1
Test the use of hdf5raw plugin (see H5Zutil.c).
*/

/* Forward */
static int NCZ_noop_codec_to_hdf5(const NCproplist* env, const char* codec, unsigned* idp, size_t* nparamsp, unsigned** paramsp);
static int NCZ_noop_hdf5_to_codec(const NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp);

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
NCZ_noop_codec_to_hdf5(const NCproplist* env, const char* codec_json, unsigned* idp, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    const NCjson* jdict = NULL;
    const NCjson* jtmp = NULL;
    size_t nparams = 0;
    unsigned* params = NULL;
    uintptr_t zarrformat = 0;
    
    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    /* Parse the codec */
    NCJcheck(NCJparse(codec_json,0,&jcodec));
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EFILTER; goto done;}

    /* Get and Verify the codec ID and parse the params*/
    if(zarrformat == 3) {
        if(NCJdictget(jcodec,"name",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
        if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
        if(strcmp(NCJstring(jtmp),NCZ_noop_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
        if(NCJdictget(jcodec,"configuration",(NCjson**)(NCjson**)&jdict)<0) {stat = NC_EFILTER; goto done;}
	if((stat = NCraw_decode(jdict,&nparams,&params))) goto done;
    } else { /* v2 */
        if(NCJdictget(jcodec,"id",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
        if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
        if(strcmp(NCJstring(jtmp),NCZ_noop_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}
	if((stat = NCraw_decode(jcodec,&nparams,&params))) goto done;
    }
    if(nparamsp) *nparamsp = nparams;
    if(paramsp) {*paramsp = params; params = NULL;}
    if(idp) *idp = (unsigned)(H5Z_FILTER_NOOP + instance[0]);

done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_noop_hdf5_to_codec(const NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char* jsonstr = NULL;
    uintptr_t zarrformat = 0;
    NCjson* jcodec = NULL;
    NCjson* jcfg = NULL;

    NC_UNUSED(id);

    if(nparams != 0 && params == NULL)
        {stat = NC_EINVAL; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if((stat = NCraw_encode(nparams,params,&jcfg))) goto done;
    if(zarrformat == 2) {
	jcodec = jcfg; jcfg = NULL;
	NCJinsertstring(jcodec,"id",NCZ_noop_codec.codecid);
    } else {
	NCJcheck(NCJnew(NCJ_DICT,&jcodec));
	NCJinsertstring(jcodec,"name",NCZ_noop_codec.codecid);
	NCJinsert(jcodec,"configuration",jcfg);
    }
    NCJcheck(NCJunparse(jcodec,0,&jsonstr));
    if(codecp) {*codecp = jsonstr; jsonstr = NULL;}
    
done:
    NCJreclaim(jcodec);
    if(jsonstr) free(jsonstr);
    return stat;
}
