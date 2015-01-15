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
#ifdef FPZIP_COMPRESSION
#include <fpzip.h>
#endif
#ifdef ZFP_COMPRESSION
#include <zfp.h>
#endif
#include "netcdf.h"
#include "hdf5.h"
#include "nc4compress.h"

typedef struct NC_COMPRESSOR {
    NC_compressor_info info;
    const H5Z_class2_t* h5info;
    int (*_register)(const struct NC_COMPRESSOR*);
    int (*_attach)(const struct NC_COMPRESSOR*,nc_compression_t*,hid_t);
    int (*_inq)(const struct NC_COMPRESSOR*,hid_t,int*,unsigned int*,nc_compression_t*);
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
static int validate(const NC_COMPRESSOR* info);
static const NC_COMPRESSOR compressors[NC_COMPRESSORS];

/* get compressor info by enum */
const NC_compressor_info*
nc_compressor_by_index(NC_compress_enum index)
{
    return (NC_compressor_info*)&compressors[index];
}

NC_compress_enum
nc_compressor_by_name(const char* name)
{
    int e;
    for(e=0;e<NC_COMPRESSORS;e++) {
	if(strcmp(compressors[e].info.name,name) == 0)
	    return (NC_compress_enum)e;
    }
    return NC_NOZIP;
}

/*
Turn on compression for a variable's plist
*/
int
nccompress_set(const char*algorithm, hid_t plistid, int nparams, unsigned int* parms)
{
    const NC_COMPRESSOR* cmp;
    nc_compression_t* uparams = (nc_compression_t*)parms;

    for(cmp=compressors;cmp->info.name != NULL;cmp++) {
        if(strcmp(cmp->info.name,algorithm)==0) {
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

    for(cmp=compressors;cmp->info.name != NULL;cmp++) {
	if(cmp->_register != NULL && cmp->_register(cmp) != NC_NOERR)
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
    for(cmp=compressors;cmp->info.name != NULL;cmp++) {
        if(cmp->h5info != NULL && cmp->h5info->id == id)
            return cmp->h5info->name;
    }
    return NULL;
}

int
nccompress_id_for(const char* name)
{
    const NC_COMPRESSOR* cmp;
    for(cmp=compressors;cmp->info.name != NULL;cmp++) {
        if(strcmp(cmp->info.name,name)==0)
            return cmp->h5info->id;
    }
    return -1;
}

int
nccompress_inq_parameters(H5Z_filter_t filter,
                          hid_t propid,
                          int argc, /*in*/
                          unsigned int* argv, /*in*/
			  char* name, /*out; really char [NC_COMPRESS_MAX_NAME]*/
                          int* nparamsp, /*out*/
                          unsigned int* params) /*out*/
{
    const NC_COMPRESSOR* cmp;
    nc_compression_t* uparams = (nc_compression_t*)params;
    if(name == NULL || nparamsp == NULL || params == NULL)
	return NC_EINVAL;
    for(cmp=compressors;cmp->info.name != NULL;cmp++) {
        if(cmp->h5info != NULL && cmp->h5info->id == filter) {
	    if(argc < cmp->info.nelems)
		return THROW(NC_EINVAL);
	    *nparamsp = argc;
            if(cmp->_inq(cmp,propid,nparamsp,argv,uparams) != NC_NOERR)
                return THROW(NC_EHDFERR);
            strncpy(name,cmp->info.name,NC_COMPRESSION_MAX_NAME);
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

    avail = H5Zfilter_avail(info->h5info->id);
    if(!avail) {
        fprintf(stderr,"Filter not available: %s.\n",info->info.name);
        return THROW(NC_EHDFERR);
    }
    status = H5Zget_filter_info(info->h5info->id, &filter_info);
    if(!(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
        !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ) {
        fprintf(stderr,"Filter not available for encoding and decoding: %s.\n",info->info.name);
        return THROW(NC_EHDFERR);
    }
    return THROW(NC_NOERR);
}

/**
Generic inquiry function
*/
static int
generic_inq(const NC_COMPRESSOR* info,
	    hid_t propid,
	    int* argc,
            unsigned int* argv,
	    nc_compression_t* parms)
{
    int i;
    if(*argc < info->info.nelems)
       return THROW(NC_EINVAL);
    *argc = info->info.nelems;
    for(i=0;i<*argc;i++)
        parms->params[i] = argv[i];
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
szip_inq(const NC_COMPRESSOR* info, hid_t propid, int* argc, unsigned int* argv, nc_compression_t* parms)
{
   if(*argc < NC_NELEMS_SZIP)
       return THROW(NC_EINVAL);
   *argc = NC_NELEMS_SZIP;
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
    status = H5Zregister(info->h5info);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static int
bzip2_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    int status = H5Pset_filter(plistid, info->h5info->id, H5Z_FLAG_MANDATORY, (size_t)NC_NELEMS_BZIP2,parms->params);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}

static size_t
H5Z_filter_bzip2(unsigned int flags, size_t cd_nelmts,
                     const unsigned int argv[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    char *outbuf = NULL;
    size_t outbuflen, outdatalen;
    int ret;
  
    if(nbytes == 0) return 0; /* sanity check */

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
    status = H5Zregister(info->h5info);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}
    
static int
fpzip_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    int status = H5Pset_filter(plistid, info->h5info->id, H5Z_FLAG_MANDATORY, (size_t)NC_NELEMS_FPZIP, parms->params);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
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
    size_t outbuflen;
    char *outbuf = NULL;
    size_t inbytes;
    size_t bufbytes;
    size_t elemsize;
    size_t totalsize;
    size_t chunksizes[NC_MAX_VAR_DIMS];
    size_t nfsize;

    if(nbytes == 0) return 0; /* sanity check */

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

    if(flags & H5Z_FLAG_REVERSE) {
        /** Decompress data.
         **/

	/* Tell fpzip where to get is compressed data */
        fpz = fpzip_read_from_buffer(*buf);
        if(fpzip_errno != fpzipSuccess)
	    goto cleanupAndFail;

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

        if(fpzip_errno != fpzipSuccess)
	    goto cleanupAndFail;

        fpzip_read_close(fpz);
        if(fpzip_errno != fpzipSuccess)
	    goto cleanupAndFail;

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
        if(fpzip_errno != fpzipSuccess)
	    goto cleanupAndFail;

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
        if(fpzip_errno != fpzipSuccess)
	    goto cleanupAndFail;

        fpzip_write_close(fpz);
        if(fpzip_errno != fpzipSuccess)
	    goto cleanupAndFail;

        /* Replace the buffer given to us with our decompressed data buffer */
        free(*buf);
        *buf = outbuf;
        *buf_size = bufbytes;
        outbuf = NULL;
        return outbuflen; /* # valid bytes */

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

#ifdef ZFP_COMPRESSION
    
/* prec */
#define H5Z_FILTER_ZFP 257
    
/*Forward*/
static size_t H5Z_filter_zfp(unsigned flags,size_t cd_nelmts,const unsigned argv[],
                        size_t nbytes,size_t *buf_size,void**buf);
    
    
/* declare a filter function's info */
static const H5Z_class2_t H5Z_ZFP = {
    H5Z_CLASS_T_VERS,               /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_ZFP, /* Filter id number */
    1,                              /* encoder_present flag (set to true) */
    1,                              /* decoder_present flag (set to true) */
    "zfp",                        /* Filter name for debugging */
    NULL,                           /* The "can apply" callback */
    NULL,                           /* The "set local" callback */
    (H5Z_func_t)H5Z_filter_zfp,   /* The actual filter function */
};
    
static int
zfp_register(const NC_COMPRESSOR* info)
{
    herr_t status;
    status = H5Zregister(info->h5info);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}
    
static int
zfp_attach(const NC_COMPRESSOR* info, nc_compression_t* parms, hid_t plistid)
{
    int n = NC_NELEMS_ZFP;
    hid_t h = plistid;
    H5Z_filter_t id = info->h5info->id;
    unsigned int flags = H5Z_FLAG_MANDATORY;
    size_t sn = (size_t)n;
    unsigned int* params = parms->params;
    int status = H5Pset_filter(h,id,flags,n,params);
    return THROW((status ? NC_EHDFERR : NC_NOERR));
}
    
/**
Assumptions:
1. Each incoming block represents 1 complete chunk
*/
static size_t
H5Z_filter_zfp(unsigned int flags, size_t cd_nelmts,
                     const unsigned int argv[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    int i;
    zfp_params zfp;
    nc_compression_t* params;
    int rank;
    int isdouble;
    unsigned int prec;
    double rate;
    double accuracy;
    size_t outbuflen;
    char *outbuf = NULL;
    size_t inbytes;
    size_t bufbytes;
    size_t elemsize;
    size_t totalsize;
    size_t chunksizes[NC_MAX_VAR_DIMS];
    size_t nzsize;
    
    if(nbytes == 0) return 0; /* sanity check */

    params = (nc_compression_t*)argv;
    isdouble = params->zfp.isdouble;
    prec = params->zfp.prec;
    rank = params->zfp.rank;
    rate = params->zfp.rate;
    accuracy = params->zfp.tolerance;

    for(totalsize=1,i=0;i<rank;i++) {
	chunksizes[i] = params->zfp.chunksizes[i];
	totalsize *= chunksizes[i];
    }

    /* Do some computations */
    nzsize = 0;
    if(rank > 2) {
        for(nzsize=1,i=2;i<rank;i++) {
	    nzsize *= chunksizes[i];
	}
    }

    /* Element size (in bytes) */
    elemsize = (isdouble ? sizeof(double) : sizeof(float));

    if(flags & H5Z_FLAG_REVERSE) {
        /** Decompress data.
         **/

        /* Number of uncompressed bytes */
        inbytes = totalsize * elemsize;

        /* Allocated size of the target buffer */
        bufbytes = 1024 + inbytes; /* why the 1024? */

	zfp.nx = chunksizes[0];
        zfp.ny = (rank >= 2 ? chunksizes[1] : 0);
        zfp.nz = (rank >= 3 ? nzsize : 0);
        zfp.type = isdouble ? ZFP_TYPE_DOUBLE : ZFP_TYPE_FLOAT;

	zfp_set_precision(&zfp,(unsigned int)prec);
	if(rate != 0)
	    zfp_set_rate(&zfp,rate);
	if(accuracy != 0)
	    zfp_set_accuracy(&zfp,accuracy);

        /* Create the decompressed data buffer */
	outbuf = (char*)malloc(bufbytes);

        /* Decompress into the compressed data buffer */
	outbuflen = zfp_decompress(&zfp,outbuf,*buf,nbytes);
        if(outbuflen == 0)
	    goto cleanupAndFail;

        /* Replace the buffer given to us with our decompressed data buffer */
        free(*buf);
        *buf = outbuf;
        *buf_size = bufbytes;
        outbuf = NULL;
        return outbuflen; /* # valid bytes */

    } else {
  
        /** Compress data.
         **/

	/* fill in zfp */
	zfp.nx = chunksizes[0];
        zfp.ny = (rank >= 2 ? chunksizes[1] : 0);
        zfp.nz = (rank >= 3 ? nzsize : 0);
        zfp.type = isdouble ? ZFP_TYPE_DOUBLE : ZFP_TYPE_FLOAT;

	zfp_set_precision(&zfp,(unsigned int)prec);
	if(rate != 0)
	    zfp_set_rate(&zfp,rate);
	if(accuracy != 0)
	    zfp_set_accuracy(&zfp,accuracy);

        /* Create the compressed data buffer */
	bufbytes = zfp_estimate_compressed_size(&zfp);
        outbuf = (char*)malloc(bufbytes);

        /* Compress into the compressed data buffer */
	outbuflen = zfp_compress(&zfp,*buf,outbuf,bufbytes);
        if(outbuflen == 0)
	    goto cleanupAndFail;

        /* Replace the buffer given to us with our decompressed data buffer */
        free(*buf);
        *buf = outbuf;
        *buf_size = bufbytes;
        outbuf = NULL;
        return outbuflen; /* # valid bytes */
    }

cleanupAndFail:
    if(outbuf)
        free(outbuf);
    return 0;
}

#endif /*ZFP_COMPRESSION*/

/**************************************************/

/* Provide access to all the compressors */
static const NC_COMPRESSOR compressors[NC_COMPRESSORS] = {
    {{"zip", NC_NELEMS_ZIP}, &H5Z_ZIP, zip_register, zip_attach, generic_inq},
#ifdef FPZIP_COMPRESSION
    {{"zfp", NC_NELEMS_ZFP}, &H5Z_ZFP, zfp_register, zfp_attach, generic_inq},
#endif
#ifdef FPZIP_COMPRESSION
    {{"fpzip", NC_NELEMS_FPZIP}, &H5Z_FPZIP, fpzip_register, fpzip_attach, generic_inq},
#endif
#ifdef BZIP2_COMPRESSION
    {{"bzip2", NC_NELEMS_BZIP2}, &H5Z_BZIP2, bzip2_register, bzip2_attach, generic_inq},
#endif
#ifdef SZIP_COMPRESSION
    {{"szip", NC_NELEMS_SZIP}, &H5Z_SZIP, szip_register, szip_attach, szip_inq},
#endif
    {{NULL, NC_NOZIP}, NULL, NULL, NULL}
};
