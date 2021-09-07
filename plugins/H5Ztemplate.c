#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <netcdf_filter_build.h>
#include "xxxx.h"

/*
Provide a textual template (not a C++ template)
from which one can construct a new filter.
The filter "name is marked with "XXXX" or "xxxx"
*/

static htri_t H5Z_can_apply_xxxx(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static herr_t H5Z_set_local_xxxx)(long long, long long, long long);
static size_t H5Z_filter_xxxx(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf);

const H5Z_class2_t H5Z_XXXX[1] = {{
    H5Z_CLASS_T_VERS,              /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_XXXX, /* Filter id number */
    1,                             /* encoder_present flag (set to true) */
    1,                             /* decoder_present flag (set to true) */
    "xxxx",                        /* Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_can_apply_xxxx, /* The "can apply" callback | NULL */
    (H5Z_set_local_func_t)H5Z_set_local_xxxx, /* The "set local" callback | NULL */
    (H5Z_func_t)H5Z_filter_xxxx,              /* The actual filter function   */
}};

/* External Discovery Functions */
H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

const void*
H5PLget_plugin_info(void)
{
    return H5Z_FILTER_XXXX;
}

static htri_t
H5Z_can_apply_xxxx(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* always applicable */
}

static herr_t
H5Z_set_local_xxxx)(long long, long long, long long)
{
    return 0; /* always succeed */
}

static size_t
H5Z_filter_xxxx(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
  char *outbuf = NULL;
  size_t outbuflen, outdatalen;
  int ret;

  if (flags & H5Z_FLAG_REVERSE) {

    /** Decompress data.
     **
     ** This process is troublesome since the size of uncompressed data
     ** is unknown, so the low-level interface must be used.
     ** Data is decompressed to the output buffer (which is sized
     ** for the average case); if it gets full, its size is doubled
     ** and decompression continues.  This avoids repeatedly trying to
     ** decompress the whole block, which could be really inefficient.
     **/

    char *newbuf = NULL;
    size_t newbuflen;

  } else {

    /** Compress data.
     **
     ** This is quite simple, since the size of compressed data in the worst
     ** case is known and it is not much bigger than the size of uncompressed
     ** data.  This allows us to use the simplified one-shot interface to
     ** compression.
     **/

    unsigned int odatalen;  /* maybe not the same size as outdatalen */

    /* Prepare the output buffer. */
    outbuflen = M;  /* worst case */
    outbuf = H5allocate_memory(outbuflen,0);
    if (outbuf == NULL) {
      fprintf(stderr, "memory allocation failed for xxxx compression\n");
      goto cleanupAndFail;
    }

    /* Compress data. */

  }

  /* Always replace the input buffer with the output buffer. */
  H5free_memory(*buf);
  *buf = outbuf;
  *buf_size = outbuflen;
  return outdatalen;

 cleanupAndFail:
  if (outbuf)
    H5free_memory(outbuf);
  return 0;
}


/**************************************************/
/* The NCZarr API */

/* Forward */
static int NCZ_xxxx_codec_setup(int ncid, int varid, int* nparamsp, unsigned** paramsp);
static int NCZ_xxxx_codec_shutdown(int nparams, unsigned* params);
static int NCZ_xxxx_codec_to_hdf5(const char* codec, int* nparamsp, unsigned** paramsp);
static int NCZ_xxxx_hdf5_to_codec(int nparams, unsigned* params, char** codecp);

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_xxxx_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "xxxx",	        /* Standard name/id of the codec */
  H5Z_FILTER_XXXX,      /* HDF5 alias for xxxx */
  NCZ_xxxx_codec_to_hdf5,
  NCZ_xxxx_hdf5_to_codec
  NCZ_xxxx_codec_setup,
  NCZ_xxxx_codec_shutdown,
};

/* External Export API */
const void*
NCZ_get_plugin_info(void)
{
    return (void*)&NCZ_xxxx_codec;
}

/* NCZarr Interface Functions */

static int
NCZ_xxxx_codec_setup(int ncid, int varid, int* nparamsp, unsigned** paramsp)
{
    return NC_NOERR;
}

static int
NCZ_xxxx_codec_shutdown(int nparams, unsigned* params)
{
    return NC_NOERR;
}

static int
NCZ_xxxx_codec_to_hdf5(const char* codec_json, int* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    int nparams = 0;
    unsigned* params = NULL;
    struct NCJconst jc;

    if((params = (unsigned*)malloc(sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    /* parse the JSON */
    if((stat = NCJparse(codec_json,0,&jcodec))) goto done;
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}
    /* Verify the codec ID */
    if((stat = NCJdictget(jcodec,"id",&jtmp))) goto done;
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_xxxx_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    /* Get unsigned integer param */
    if((stat = NCJdictget(jcodec,"param0",&jtmp))) goto done;
    if((stat = NCJcvt(jtmp,NCJ_INT,&jc))) goto done;
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EINVAL; goto done;}
    params[i] = (unsigned)jc.ival;
    if(nparamsp) *nparamsp = 1;
    if(paramsp) {*paramsp = params; params = NULL;}
    
done:
    if(params) free(params);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_xxxx_hdf5_to_codec(int nparams, unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];

    if(nparams == 0 || params == NULL)
        {stat = NC_EINVAL; goto done;}

    snprintf(json,sizeof(json),"{\"id\": \"%s\", \"param0\": \"%u\"}",NCZ_xxxx_codec.codecid,param[0]);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

