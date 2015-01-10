#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef BZIP2_COMPRESSION
#include <bzlib.h>
#endif
#ifdef FPZIP_COMPRESSION
#include <fpzip.h>
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
nccompress_set(const char*algorithm, hid_t plistid, unsigned int* parms)
{
    const NC_COMPRESSOR* cmp;
    nc_compression_t* uparams = (nc_compression_t*)parms;

    for(cmp=compressors;cmp->name != NULL;cmp++) {
        if(strcmp(cmp->name,algorithm)==0) {
            if(cmp->_attach(cmp,uparams,plistid) != NC_NOERR)
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
                          unsigned int* parms)
{
    const NC_COMPRESSOR* cmp;
    nc_compression_t* uparams = (nc_compression_t*)parms;

    for(cmp=compressors;cmp->name != NULL;cmp++) {
        if(cmp->info->id == filter) {
            if(cmp->_inq(cmp,propid,argc,argv,uparams) != NC_NOERR)
                return THROW(NC_EHDFERR);
            strncpy(name,cmp->name,NC_COMPRESSION_MAX_NAME);
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
    int status = H5Pset_deflate(plistid, parms->zip.level);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
zip_inq(const NC_COMPRESSOR* info, hid_t propid, size_t argc, unsigned int* argv, nc_compression_t* parms)
{
   parms->zip.level = argv[0];
   return THROW(NC_NOERR);
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
    parms->szip.options_mask = argv[0];
    parms->szip.bits_per_pixel = argv[1];
#if 0
    parms->szip.pixels_per_block = argv[2];
    parms->szip.pixels_per_scanline = argv[3];
#endif
    return THROW(NC_NOERR);
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
    int status = H5Pset_filter(plistid, info->info->id, H5Z_FLAG_MANDATORY, (size_t)CD_NELEMS_BZIP2,(unsigned int*)parms->params);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
bzip2_inq(const NC_COMPRESSOR* info, hid_t propid, size_t argc, unsigned int* argv, nc_compression_t* parms)
{
    parms->bzip2.level = argv[0];
    return THROW(NC_NOERR);
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
        outbuflen = nbytes * 3 + 1;/* average bzip2 compression ratio is 3:1 */
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
#ifdef FPZIP_COMPRESSION
    
/* prec */
#define H5Z_FILTER_FPZIP 256
    
/*Forward*/
static size_t H5Z_filter_fpzip(unsigned flags,size_t cd_nelmts,const unsigned argv[],
                        size_t nbytes,size_t *buf_size,void**buf);
    
    
/* declare a filter function's info */
static const H5Z_class2_t H5Z_FPZIP = {
    H5Z_CLASS_T_VERS,               /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_FPZIP, /* Filter id number */
    1,                              /* encoder_present flag (set to true) */
    1,                              /* decoder_present flag (set to true) */
    "fpzip",                        /* Filter name for debugging */
    NULL,                           /* The "can apply" callback */
    NULL,                           /* The "set local" callback */
    (H5Z_func_t)H5Z_filter_fpzip,   /* The actual filter function */
};
    
static int
fpzip_register(const NC_COMPRESSOR* info)
{
    herr_t status;
    status = H5Zregister(info->info);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}
    
static int
fpzip_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    int status = H5Pset_filter(plistid, info->info->id, H5Z_FLAG_MANDATORY, (size_t)CD_NELEMS_FPZIP, (unsigned int*)parms->params);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}
    
static int
fpzip_inq(const NC_COMPRESSOR* info, hid_t propid, size_t argc, unsigned int* argv, nc_compression_t* parms)
{
    int i;
    if(argc < 0 || argc > CD_NELEMS_FPZIP)
	argc = CD_NELEMS_FPZIP;
    for(i=0;i<argc;i++)
        parms->params[i] = argv[i];
    return THROW(NC_NOERR);
}

/**
Assumptions:
1. Each incoming block represents 1 complete chunk
*/
static size_t
H5Z_filter_fpzip(unsigned int flags, size_t cd_nelmts,
                     const unsigned int argv[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    int i;
    FPZ* fpz;
    nc_compression_t* params;
    int rank;
    int isdouble;
    int prec;
    int lossless;
    size_t outbuflen;
    char *outbuf = NULL;
    size_t inbytes;
    size_t bufbytes;
    size_t elemsize;
    size_t totalsize;
    size_t chunksizes[NC_MAX_VAR_DIMS];
    size_t nfsize;
    
#if 0
    size_t piecesize,size,inbytes,bufbytes,npieces;
    dimbase, prec;
#endif

    params = (nc_compression_t*)argv;
    isdouble = params->fpzip.isdouble;
    prec = params->fpzip.prec;
    rank = params->fpzip.rank;
    for(totalsize=1,i=0;i<rank;i++) {
	chunksizes[i] = params->fpzip.chunksizes[i];
	totalsize *= chunksizes[i];
    }

    /* Do some computations */
    nfsize = 0;
    if(rank > 3) {
        for(nfsize=1,i=3;i<rank;i++) {
	    nfsize *= chunksizes[i];
	}
    }

    /* Element size (in bytes) */
    elemsize = (isdouble ? sizeof(double) : sizeof(float));

    /* Number of array bytes */
    inbytes = totalsize * elemsize;

    /* Allocated size of the new buffer;
       used for both decompression and compression */
    bufbytes = 1024 + inbytes; /* why the 1024? */

    /* precision */
    if(prec == 0)
        prec = CHAR_BIT * elemsize;
    lossless = (prec == CHAR_BIT * elemsize);

    if(flags & H5Z_FLAG_REVERSE) {
        /** Decompress data.
         **/

	/* Tell fpzip where to get is compressed data */
        fpz = fpzip_read_from_buffer(*buf);
        if(fpzip_errno != fpzipSuccess) goto cleanupAndFail;

        fpz->type = isdouble ? FPZIP_TYPE_DOUBLE : FPZIP_TYPE_FLOAT;
	fpz->prec = prec;
	fpz->nx = chunksizes[0];
        fpz->ny = (rank >= 2 ? chunksizes[1] : 1);
        fpz->nz = (rank >= 3 ? chunksizes[2] : 1);
        fpz->nf = (rank >= 4 ? nfsize : 1);

        /* Create the decompressed data buffer */
	outbuf = (char*)malloc(bufbytes);

        /* Decompress into the compressed data buffer */
        outbuflen = fpzip_read(fpz,outbuf);

        if(fpzip_errno == fpzipSuccess && outbuflen == 0)
            fpzip_errno = fpzipErrorReadStream;

        if(fpzip_errno != fpzipSuccess) goto cleanupAndFail;

        fpzip_read_close(fpz);
        if(fpzip_errno != fpzipSuccess) goto cleanupAndFail;

        /* Replace the buffer given to us with our decompressed data buffer */
        free(*buf);
        *buf = outbuf;
        *buf_size = bufbytes;
        outbuf = NULL;
        return outbuflen; /* # valid bytes */

    } else {
  
        /** Compress data.
         **/

        /* Create the compressed data buffer */
        outbuf = (char*)malloc(bufbytes); /* overkill */

        /* Compress into the decompressed data buffer */
        fpz = fpzip_write_to_buffer(outbuf,bufbytes);
        if(fpzip_errno != fpzipSuccess) goto cleanupAndFail;

        fpz->type = isdouble ? FPZIP_TYPE_DOUBLE : FPZIP_TYPE_FLOAT;
	fpz->prec = prec;
	fpz->nx = chunksizes[0];
        fpz->ny = (rank >= 2 ? chunksizes[1] : 1);
        fpz->nz = (rank >= 3 ? chunksizes[2] : 1);
        fpz->nf = (rank >= 4 ? nfsize : 1);

    /* Compress to the compressed data buffer from decompressed data in *buf*/
    outbuflen = fpzip_write(fpz,*buf);

    if(outbuflen == 0 && fpzip_errno  == fpzipSuccess)
        fpzip_errno = fpzipErrorWriteStream;
    if(fpzip_errno != fpzipSuccess) goto cleanupAndFail;

    fpzip_write_close(fpz);
    if(fpzip_errno != fpzipSuccess) goto cleanupAndFail;
  }

cleanupAndFail:
    if(outbuf)
        free(outbuf);
    if(fpzip_errno != fpzipSuccess) {
	fprintf(stderr,"fpzip error: %s\n",fpzip_errstr[fpzip_errno]);
        fflush(stderr);
    }
    return 0;
}

#endif /*FPZIP_COMPRESSION*/

/**************************************************/

/* Provide access to all the compressors */
static const NC_COMPRESSOR compressors[] = {
    {"zip", &H5Z_ZIP, zip_register, zip_attach, zip_inq},
#ifdef FPZIP_COMPRESSION
    {"fpzip", &H5Z_FPZIP, fpzip_register, fpzip_attach, fpzip_inq},
#endif
#ifdef BZIP2_COMPRESSION
    {"bzip2", &H5Z_BZIP2, bzip2_register, bzip2_attach, bzip2_inq},
#endif
#ifdef SZIP_COMPRESSION
    {"szip", &H5Z_SZIP, szip_register, szip_attach, szip_inq},
#endif
    {NULL, NULL, NULL, NULL}
};
