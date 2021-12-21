#include "config.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "netcdf_filter_build.h"

/* WARNING:
Starting with HDF5 version 1.10.x, the plugin code MUST be
careful when using the standard *malloc()*, *realloc()*, and
*free()* function.

In the event that the code is allocating, reallocating, or
free'ing memory that either came from or will be exported to the
calling HDF5 library, then one MUST use the corresponding HDF5
functions *H5allocate_memory()*, *H5resize_memory()*,
*H5free_memory()* [5] to avoid memory failures.

Additionally, if your filter code leaks memory, then the HDF5 library
will generate an error.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "netcdf_filter_build.h"
#include <netcdf_json.h>

#include "h5bzip2.h"

/* Forward */
static htri_t H5Z_bzip2_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static size_t H5Z_filter_bzip2(unsigned flags,size_t cd_nelmts,const unsigned cd_values[],
                    size_t nbytes,size_t *buf_size,void**buf);

const H5Z_class2_t H5Z_BZIP2[1] = {{
    H5Z_CLASS_T_VERS,       /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_BZIP2,         /* Filter id number             */
    1,              /* encoder_present flag (set to true) */
    1,              /* decoder_present flag (set to true) */
    "bzip2",                  /* Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_bzip2_can_apply, /* The "can apply" callback  */
    NULL,                       /* The "set local" callback     */
    (H5Z_func_t)H5Z_filter_bzip2,         /* The actual filter function   */
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
    return H5Z_BZIP2;
}

/* Make this explicit */
/*
 * The "can_apply" callback returns positive a valid combination, zero for an
 * invalid combination and negative for an error.
 */
static htri_t
H5Z_bzip2_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* Assume it can always apply */
}

static size_t
H5Z_filter_bzip2(unsigned int flags, size_t cd_nelmts,
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

    bz_stream stream;
    char *newbuf = NULL;
    size_t newbuflen;

    /* Prepare the output buffer. */
    outbuflen = nbytes * 3 + 1;  /* average bzip2 compression ratio is 3:1 */
    outbuf = H5allocate_memory(outbuflen,0);
    if (outbuf == NULL) {
      fprintf(stderr, "memory allocation failed for bzip2 decompression\n");
      goto cleanupAndFail;
    }

    /* Use standard malloc()/free() for internal memory handling. */
    stream.bzalloc = NULL;
    stream.bzfree = NULL;
    stream.opaque = NULL;

    /* Start decompression. */
    ret = BZ2_bzDecompressInit(&stream, 0, 0);
    if (ret != BZ_OK) {
      fprintf(stderr, "bzip2 decompression start failed with error %d\n", ret);
      goto cleanupAndFail;
    }

    /* Feed data to the decompression process and get decompressed data. */
    stream.next_out = outbuf;
    stream.avail_out = outbuflen;
    stream.next_in = *buf;
    stream.avail_in = nbytes;
    do {
      ret = BZ2_bzDecompress(&stream);
      if (ret < 0) {
	fprintf(stderr, "BUG: bzip2 decompression failed with error %d\n", ret);
	goto cleanupAndFail;
      }

      if (ret != BZ_STREAM_END && stream.avail_out == 0) {
        /* Grow the output buffer. */
        newbuflen = outbuflen * 2;
        newbuf = H5resize_memory(outbuf, newbuflen);
        if (newbuf == NULL) {
          fprintf(stderr, "memory allocation failed for bzip2 decompression\n");
          goto cleanupAndFail;
        }
        stream.next_out = newbuf + outbuflen;  /* half the new buffer behind */
        stream.avail_out = outbuflen;  /* half the new buffer ahead */
        outbuf = newbuf;
        outbuflen = newbuflen;
      }
    } while (ret != BZ_STREAM_END);

    /* End compression. */
    outdatalen = stream.total_out_lo32;
    ret = BZ2_bzDecompressEnd(&stream);
    if (ret != BZ_OK) {
      fprintf(stderr, "bzip2 compression end failed with error %d\n", ret);
      goto cleanupAndFail;
    }

  } else {

    /** Compress data.
     **
     ** This is quite simple, since the size of compressed data in the worst
     ** case is known and it is not much bigger than the size of uncompressed
     ** data.  This allows us to use the simplified one-shot interface to
     ** compression.
     **/

    unsigned int odatalen;  /* maybe not the same size as outdatalen */
    int blockSize100k = 9;

    /* Get compression block size if present. */
    if (cd_nelmts > 0) {
      blockSize100k = cd_values[0];
      if (blockSize100k < 1 || blockSize100k > 9) {
	fprintf(stderr, "invalid compression block size: %d\n", blockSize100k);
	goto cleanupAndFail;
      }
    }

    /* Prepare the output buffer. */
    outbuflen = nbytes + nbytes / 100 + 600;  /* worst case (bzip2 docs) */
    outbuf = H5allocate_memory(outbuflen,0);

    if (outbuf == NULL) {
      fprintf(stderr, "memory allocation failed for bzip2 compression\n");
      goto cleanupAndFail;
    }

    /* Compress data. */
    odatalen = outbuflen;
    ret = BZ2_bzBuffToBuffCompress(outbuf, &odatalen, *buf, nbytes,
                                   blockSize100k, 0, 0);
    outdatalen = odatalen;
    if (ret != BZ_OK) {
      fprintf(stderr, "bzip2 compression failed with error %d\n", ret);
      goto cleanupAndFail;
    }
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
/* NCZarr Filter Objects */

/* Provide the codec support for the HDF5 bzip library */

static int NCZ_bzip2_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_bzip2_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);

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

