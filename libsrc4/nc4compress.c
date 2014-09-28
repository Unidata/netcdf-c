#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef BZIP2_COMPRESSION
#include <bzlib.h>
#endif
#ifdef SZIP_COMPRESSION
#include <szlib.h>
#endif
#include "netcdf.h"
#include "hdf5.h"
#include "nc4compress.h"

typedef struct NC_COMPRESSOR {
    const char* name;
    const H5Z_class2_t* info;
    int (*_register)(const struct NC_COMPRESSOR*);
    int (*_attach)(const struct NC_COMPRESSOR*,nc_compression_t*,hid_t);
    int (*_inq)(const struct NC_COMPRESSOR*,hid_t,size_t,unsigned int*,nc_compression_t*);
} NC_COMPRESSOR;

#define DEBUG

#ifndef DEBUG
#define THROW(e) (e)
#else
#define THROW(e) checkerr(e,__FILE__,__LINE__)
static int checkerr(int e, const char* file, int line)
{
   if(e != 0) {
     fprintf(stderr, "Error %d in file %s, line %d.\n", e, file, line);
     fflush(stderr);
     abort();
   }
   return e;
}
#endif

/* Forward */
static const NC_COMPRESSOR compressors[];
static int validate(const NC_COMPRESSOR* info);

/*
Turn on compression for a variable's plist
*/
int
nccompress_set(const char*algorithm, hid_t plistid, nc_compression_t* parms)
{
    const NC_COMPRESSOR* cmp;

    for(cmp=compressors;cmp->name != NULL;cmp++) {
	if(strcmp(cmp->name,algorithm)==0) {
	    if(cmp->_attach(cmp,parms,plistid) != NC_NOERR)
		return THROW(NC_EHDFERR);
	    return THROW(NC_NOERR);
	}
    }
    return THROW(NC_EHDFERR);
}

/* 
Register all known filters with the library
*/
int
nccompress_register_all(void)
{
    const NC_COMPRESSOR* cmp;

    for(cmp=compressors;cmp->name != NULL;cmp++) {
	if(cmp->_register(cmp) != NC_NOERR)
	    return THROW(NC_EHDFERR);
	if(validate(cmp) != NC_NOERR)
	    return THROW(NC_EHDFERR);
    }
    return THROW(NC_NOERR);
}

const char*
nccompress_name_for(int id)
{
    const NC_COMPRESSOR* cmp;
    for(cmp=compressors;cmp->name != NULL;cmp++) {
	if(cmp->info->id == id)
	    return cmp->info->name;
    }
    return NULL;
}

int
nccompress_id_for(const char* name)
{
    const NC_COMPRESSOR* cmp;
    for(cmp=compressors;cmp->name != NULL;cmp++) {
	if(strcmp(cmp->name,name)==0)
	    return cmp->info->id;
    }
    return -1;
}

int
nccompress_inq_parameters(H5Z_filter_t filter,
                          hid_t propid,
			  size_t argc,
                          unsigned int* argv,
			  char* name,
                          nc_compression_t* parms)
{
    const NC_COMPRESSOR* cmp;

    for(cmp=compressors;cmp->name != NULL;cmp++) {
	if(cmp->info->id == filter) {
	    if(cmp->_inq(cmp,propid,argc,argv,parms) != NC_NOERR)
		return THROW(NC_EHDFERR);
	    strncpy(name,cmp->name,COMPRESSION_MAX_NAME);
	    return THROW(NC_NOERR);
	}
    }
    return THROW(NC_EHDFERR);
}

/*
* Check if compression is available and can be used for both
* compression and decompression.  Normally we do not perform error
* checking in these examples for the sake of clarity, but in this
* case we will make an exception because this filter is an
* optional part of the hdf5 library.
*/
static int
validate(const NC_COMPRESSOR* info)
{
    htri_t avail;
    unsigned int filter_info;
    herr_t status;

    avail = H5Zfilter_avail(info->info->id);
    if(!avail) {
        fprintf(stderr,"Filter not available: %s.\n",info->name);
        return THROW(NC_EHDFERR);
    }
    status = H5Zget_filter_info(info->info->id, &filter_info);
    if(!(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
	!(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ) {
        fprintf(stderr,"Filter not available for encoding and decoding: %s.\n",info->name);
        return THROW(NC_EHDFERR);
    }
    return THROW(NC_NOERR);
}

/**************************************************/
/*#ifdef ZIP (DEFLATE) compression always defined */

static const H5Z_class2_t H5Z_ZIP = {
    H5Z_CLASS_T_VERS,                 /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_DEFLATE, /* Filter id number */
    1,                                /* encoder_present flag (set to true) */
    1,                                /* decoder_present flag (set to true) */
    "zip",                            /* Filter name for debugging    */
    NULL,                             /* The "can apply" callback     */
    NULL,                             /* The "set local" callback     */
    (H5Z_func_t)NULL,                 /* The actual filter function   */
};

static int
zip_register(const NC_COMPRESSOR* info)
{
    return THROW(NC_NOERR); /* no-op */
}

static int
zip_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    int status = H5Pset_deflate(plistid, parms->level);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
zip_inq(const NC_COMPRESSOR* info, hid_t propid, size_t argc, unsigned int* argv, nc_compression_t* parms)
{
#define CD_NELEMS_ZIP 1
   parms->level = argv[0];
   return THROW(NC_NOERR);
#undef CD_NELEMS_ZIP
}

/**************************************************/
#ifdef SZIP_COMPRESSION

/* declare a filter function's info */
static const H5Z_class2_t H5Z_SZIP = {
    H5Z_CLASS_T_VERS,       /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_SZIP,         /* Filter id number             */
    1,              /* encoder_present flag (set to true) */
    1,              /* decoder_present flag (set to true) */
    "szip",                  /* Filter name for debugging    */
    NULL,                       /* The "can apply" callback     */ 
    NULL,        /* The "set local" callback     */
    (H5Z_func_t)NULL, /* The actual filter function */
};

static int
szip_register(const NC_COMPRESSOR* info)
{
    return THROW(NC_NOERR); /* no-op */
}

static int
szip_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    htri_t avail;
    herr_t status;

    /* See if already in the hdf5 library */
    avail = H5Zfilter_avail(H5Z_FILTER_SZIP);
    if(avail) {
        status = H5Pset_szip(plistid, parms->szip.options_mask, parms->szip.pixels_per_block);
    } else {
	fprintf(stderr,"szip compression not available\n");
	return NC_EHDFERR;
    }
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
szip_inq(const NC_COMPRESSOR* info, hid_t propid, size_t argc, unsigned int* argv, nc_compression_t* parms)
{
#define CD_NELEMS_SZIP 2
   parms->szip.options_mask = argv[0];
   parms->szip.bits_per_pixel = argv[1];
#if 0
   parms->szip.pixels_per_block = argv[2];
   parms->szip.pixels_per_scanline = argv[3];
#endif
   return THROW(NC_NOERR);
#undef CD_NELEMS_SZIP
}

/* end H5Z_set_local_szip() */

#endif /*SZIP_COMPRESSION*/

/**************************************************/
#ifdef BZIP2_COMPRESSION

#define H5Z_FILTER_BZIP2 307

/*Forward*/
static size_t H5Z_filter_bzip2(unsigned flags,size_t cd_nelmts,const unsigned argv[],
                    size_t nbytes,size_t *buf_size,void**buf);

/* declare a filter function's info */
static const H5Z_class2_t H5Z_BZIP2 = {
    H5Z_CLASS_T_VERS,       /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_BZIP2,         /* Filter id number             */
    1,              /* encoder_present flag (set to true) */
    1,              /* decoder_present flag (set to true) */
    "bzip2",                  /* Filter name for debugging    */
    NULL,                       /* The "can apply" callback     */
    NULL,                       /* The "set local" callback     */
    (H5Z_func_t)H5Z_filter_bzip2,         /* The actual filter function   */
};

static int
bzip2_register(const NC_COMPRESSOR* info)
{
    herr_t status;
    status = H5Zregister(info->info);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
bzip2_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    unsigned int argv[1];
    argv[0] = parms->level;
    int status = H5Pset_filter(plistid, info->info->id, H5Z_FLAG_MANDATORY, (size_t)1, argv);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
bzip2_inq(const NC_COMPRESSOR* info, hid_t propid, size_t argc, unsigned int* argv, nc_compression_t* parms)
{
#define CD_NELEMS_BZIP2 1
   parms->level = argv[0];
   return THROW(NC_NOERR);
#undef CD_NELEMS_BZIP2
}

static size_t
H5Z_filter_bzip2(unsigned int flags, size_t cd_nelmts,
                     const unsigned int argv[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
  char *outbuf = NULL;
  size_t outbuflen, outdatalen;
  int ret;

  if(flags & H5Z_FLAG_REVERSE) {

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
    outbuf = malloc(outbuflen);
    if(outbuf == NULL) {
      fprintf(stderr, "memory allocation failed for bzip2 decompression\n");
      goto cleanupAndFail;
    }

    /* Use standard malloc()/free() for internal memory handling. */
    stream.bzalloc = NULL;
    stream.bzfree = NULL;
    stream.opaque = NULL;

    /* Start decompression. */
    ret = BZ2_bzDecompressInit(&stream, 0, 0);
    if(ret != BZ_OK) {
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
      if(ret < 0) {
	fprintf(stderr, "BUG: bzip2 decompression failed with error %d\n", ret);
	goto cleanupAndFail;
      }

      if(ret != BZ_STREAM_END && stream.avail_out == 0) {
        /* Grow the output buffer. */
        newbuflen = outbuflen * 2;
        newbuf = realloc(outbuf, newbuflen);
        if(newbuf == NULL) {
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
    if(ret != BZ_OK) {
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
    if(cd_nelmts > 0) {
      blockSize100k = argv[0];
      if(blockSize100k < 1 || blockSize100k > 9) {
	fprintf(stderr, "invalid compression block size: %d\n", blockSize100k);
	goto cleanupAndFail;
      }
    }

    /* Prepare the output buffer. */
    outbuflen = nbytes + nbytes / 100 + 600;  /* worst case (bzip2 docs) */
    outbuf = malloc(outbuflen);
    if(outbuf == NULL) {
      fprintf(stderr, "memory allocation failed for bzip2 compression\n");
      goto cleanupAndFail;
    }

    /* Compress data. */
    odatalen = outbuflen;
    ret = BZ2_bzBuffToBuffCompress(outbuf, &odatalen, *buf, nbytes,
                                   blockSize100k, 0, 0);
    outdatalen = odatalen;
    if(ret != BZ_OK) {
      fprintf(stderr, "bzip2 compression failed with error %d\n", ret);
      goto cleanupAndFail;
    }
  }

  /* Always replace the input buffer with the output buffer. */
  free(*buf);
  *buf = outbuf;
  *buf_size = outbuflen;
  return outdatalen;

 cleanupAndFail:
  if(outbuf)
    free(outbuf);
  return 0;
}
#endif /*BZIP2_COMPRESSION*/

/**************************************************/

/* Provide access to all the compressors */
static const NC_COMPRESSOR compressors[] = {
    {"zip", &H5Z_ZIP, zip_register, zip_attach, zip_inq},
#ifdef BZIP2_COMPRESSION
    {"bzip2", &H5Z_BZIP2, bzip2_register, bzip2_attach, bzip2_inq},
#endif
#ifdef SZIP_COMPRESSION
    {"szip", &H5Z_SZIP, szip_register, szip_attach, szip_inq},
#endif
    {NULL, NULL, NULL, NULL}
};
