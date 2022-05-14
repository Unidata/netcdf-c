#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>

#include "netcdf_filter_build.h"

/* use a temporary filter id*/
#define H5Z_FILTER_UNKNOWN 33000

#undef DEBUG

static htri_t H5Z_unknown_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static size_t H5Z_filter_unknown(unsigned int, size_t, const unsigned int cd_values[], size_t, size_t*, void**);

static H5Z_class2_t H5Z_UNKNOWN[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_UNKNOWN), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "unknown",                          /*  Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_unknown_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_unknown,     /* The actual filter function   */
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
    return H5Z_UNKNOWN;
}

/* Make this explicit */
/*
 * The "can_apply" callback returns positive a valid combination, zero for an
 * invalid combination and negative for an error.
 */
static htri_t
H5Z_unknown_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* Assume it can always apply */
}

static size_t
H5Z_filter_unknown(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    void* newbuf;
    
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
"id": "unknown",
}
*/

/* Forward */
static int NCZ_unknown_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_unknown_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_unknown_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "unknown",	        /* Standard name/id of the codec */
  H5Z_FILTER_UNKNOWN,     /* HDF5 alias for unknown */
  NULL, /*NCZ_unknown_codec_initialize*/
  NULL, /*NCZ_unknown_codec_finalize*/
  NCZ_unknown_codec_to_hdf5,
  NCZ_unknown_hdf5_to_codec,
  NULL, /*NCZ_unknown_modify_parameters*/
};

/* External Export API */
DLLEXPORT
const void*
NCZ_get_codec_info(void)
{
    return (void*)&NCZ_unknown_codec;
}

/* NCZarr Interface Functions */

static int
NCZ_unknown_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;

    *nparamsp = 0;
    *paramsp = NULL;
    
    return stat;
}

static int
NCZ_unknown_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[8192];

    if(nparams != 0 || params != NULL)
        {stat = NC_EINVAL; goto done;}

    snprintf(json,sizeof(json),"{\"id\": \"%s\"}",NCZ_unknown_codec.codecid);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
