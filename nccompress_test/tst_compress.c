/*
  Copyright 2008, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This program tests the large file bug in netCDF 3.6.2,
  creating byte and short variables larger than 4 GiB.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>

#ifdef SZIP_COMPRESSION
#include <szlib.h>
#else
#define SZ_NN_OPTION_MASK 0
#define SZ_EC_OPTION_MASK 0
#endif

#ifdef FPZIP_COMPRESSION
#include <fpzip.h>
#endif

#ifdef ZFP_COMPRESSION
#include <zfp.h>
#endif

#include "netcdf.h"
#include "nc_tests.h"
#include "nc4compress.h"

/* Write using var1_T instead of var_T */
#undef VAR1

#define DEFLATE_LEVEL 9

#if 0
#define RATE 32
#define TOLERANCE (1e-9)
#else
#define RATE 0
#define TOLERANCE (1e-9)
#endif

#define MAXERRS 8

/* Following 3 must be consistent */
#define T float
#define NC_PUT_VAR1 nc_put_var1_float
#define NC_PUT_VAR nc_put_var_float

/* Created Meta-data 
netcdf zip {
dimensions:
	dim1 = .. ;
	dim2 = ... ;
	dim3 = ... ;
	...
	dimn = ... ;
variables:
	int var(dim1, dim2, dim3,...dimn) ;
}
*/

typedef enum XZIP {
NOZIP = 0,
ZIP   = 1,
SZIP  = 2,
BZIP2 = 3,
FPZIP = 4,
ZFP   = 5,
NZIP  = 6,
} XZIP;

/* These need to be parallel with XZIP */
static char* zipnames[NZIP+1] = {"nozip","zip","szip","bzip2","fpzip","zfp",NULL};

static int supported[NZIP];
static int test[NZIP];

/* Following must be consistent */
static const int MAXDIMS = 5;
static size_t DIM[5] = {32,32,32,32,32};
static size_t CHUNKS[5]
//= {8,8,8,8,8};
= {32,32,32,32,32};

/* Actual # dims to use */
static const int NDIMS = 3;

static size_t total = 0;
static int ncid, varid;
static int dimids[NC_MAX_VAR_DIMS];
static int odom[NC_MAX_VAR_DIMS];
static T* array = NULL;

static int tagndims = 0;

/* Forward */
static int test_zfp(void);
static int test_fpzip(void);
static int test_bzip2(void);
static int test_szip(void);
static int test_zip(void);
static int test_nozip(void);
static void init(int argc, char** argv);
static void reset(void);
static void odom_reset(void);
static int odom_more(void);
static int odom_next(void);
static int odom_offset(void);
static T expected(void);
static size_t product(int ndims, size_t* dims);

static int
check(int err,int line)
{
    if(err != NC_NOERR) {
	fprintf(stderr,"fail (%d): %s\n",line,nc_strerror(err));
	ERR;
    }
    return NC_NOERR;
}

#define CHECK(x) check(x,__LINE__)

static char*
filenamefor(XZIP encoder)
{
    static char testfile[2048];
    char* name = zipnames[(int)encoder];
    if(tagndims)
        snprintf(testfile,sizeof(testfile),"%s_%d.nc",name,NDIMS);
    else
        snprintf(testfile,sizeof(testfile),"%s.nc",name);
    return testfile;
}

static int
verifychunks(void)
{
    int i;
    int store = -1;
    size_t chunksizes[MAXDIMS];
    memset(chunksizes,0,sizeof(chunksizes));
    CHECK(nc_inq_var_chunking(ncid, varid, &store, chunksizes));
    if(store != NC_CHUNKED) {
	fprintf(stderr,"bad chunk store\n");
	return 0;
    }
    for(i=0;i<NDIMS;i++) {
        if(chunksizes[i] != CHUNKS[i]) {
	    fprintf(stderr,"bad chunk size: %d\n",i);
	    return 0;
	}
    }
    return 1;
}

static int
create(XZIP encoder)
{
    int i;
    int store;
    size_t chunksizes[NDIMS];
    char* testfile = filenamefor(encoder);

    /* Create a file with one big variable. */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    for(i=0;i<NDIMS;i++) {
	char dimname[1024];
	snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, DIM[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, NDIMS, dimids, &varid));
    /* Force a specific chunking */
    for(i=0;i<NDIMS;i++)
	chunksizes[i] = CHUNKS[i];	
    store = NC_CHUNKED;
    CHECK(nc_def_var_chunking(ncid,varid,store,chunksizes));
    if(!verifychunks())
	return NC_EINVAL;
    return NC_NOERR;
}

static int
open(XZIP encoder)
{
    char* algorithm;
    nc_compression_t parms;
    char* testfile = filenamefor(encoder);
    const char* compressor = zipnames[(int)encoder];

    /* Open the file and check it. */
    CHECK(nc_open(testfile, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    if(strcmp(compressor,"nozip") != 0) {
	int nparams = NC_COMPRESSION_MAX_PARAMS;	
        /* Check the compression algorithm */
        CHECK(nc_inq_var_compress(ncid,varid,NULL,&algorithm,&nparams,parms.params));
        if(strcmp(algorithm,compressor) != 0) {
	    printf("Compression algorithm mismatch: %s\n",algorithm);
	    return 0;
        } else	
	    printf("Compression algorithm verified: %s\n",algorithm);
    }

    /* Verify chunking */
    if(!verifychunks())
	return 0;
    fflush(stderr);
    return 1;
}

static void
fill(void)
{
   odom_reset();
   while(odom_more()) {
	int offset = odom_offset();
	T expect = expected();
	array[offset] = expect;
	odom_next();
   }
}

static int
write(void)
{
   int stat = NC_NOERR;
#ifdef VAR1
   odom_reset();
   while(odom_more()) {
	int offset = odom_offset();
	NC_PUT_VAR1(ncid,varid,offset,&array[offset]);
	odom_next();
   }
#else
   stat = NC_PUT_VAR(ncid,varid,array);
#endif
   return stat;
}


static int
compare(void)
{
   int errs = 0;
   odom_reset();
   while(odom_more()) {
	int offset = odom_offset();
	float expect = expected();
	if(array[offset] != expect) {
	    fprintf(stderr,"mismatch: array[%d]=%f expected=%f\n",
			offset,array[offset],expect);
	    errs++;
	    if(errs >= MAXERRS)
		break;
	}
	odom_next();
   }
   return (errs == 0);
}

static int
test_zfp(void)
{
    int ok = 1;
    int i;
    nc_compression_t parms;

    printf("\n*** Testing zfp compression.\n");

    /* Use zfp compression */
    parms.zfp.isdouble  = 0; /* single (0) or double (1) precision */
    parms.zfp.prec      = 0; /* number of bits of precision (zero = full) */
    parms.zfp.rate      = RATE;
    parms.zfp.tolerance = TOLERANCE;
    parms.zfp.rank      = NDIMS;
    for(i=0;i<NDIMS;i++)
	parms.zfp.chunksizes[i] = CHUNKS[i];

    create(ZFP);
    CHECK(nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "zfp", NC_NELEMS_ZFP,parms.params));
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    reset();
    open(ZFP);
    memset(array,0,sizeof(T)*total);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_fpzip(void)
{
    int i;
    int ok = 1;
    nc_compression_t parms;

    printf("\n*** Testing fpzip compression.\n");
    reset();

    /* Use fpzip compression */
    parms.fpzip.isdouble = 0; /* single (0) or double (1) precision */
    parms.fpzip.prec     = 0; /* number of bits of precision (zero = full) */
    parms.fpzip.rank     = NDIMS;
    for(i=0;i<NDIMS;i++)
	parms.fpzip.chunksizes[i] = CHUNKS[i];

    create(FPZIP);
    CHECK(nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "fpzip", NC_NELEMS_FPZIP, parms.params));
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    reset();
    open(FPZIP);
    memset(array,0,sizeof(T)*total);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_bzip2(void)
{
    int ok = 1;
    nc_compression_t parms;

    printf("\n*** Testing bzip2 compression.\n");
    reset();

    parms.bzip2.level = DEFLATE_LEVEL;

    create(BZIP2);
    CHECK(nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "bzip2", NC_NELEMS_BZIP2, parms.params));
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    reset();
    open(BZIP2);
    memset(array,0,sizeof(T)*total);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_szip(void)
{
    int ok = 1;
    nc_compression_t parms;

    printf("\n*** Testing szip compression.\n");
    reset();

    /* Use szip compression */
    parms.szip.bits_per_pixel = 0;
    parms.szip.pixels_per_scanline = 0;
#if 0
    parms.szip.options_mask = SZ_NN_OPTION_MASK;
    parms.szip.pixels_per_block = 16;
#else
    parms.szip.options_mask = SZ_EC_OPTION_MASK;
    parms.szip.pixels_per_block = 32;
#endif

    create(SZIP);
    CHECK(nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "szip", NC_NELEMS_SZIP, parms.params));
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    reset();
    open(SZIP);
    memset(array,0,sizeof(T)*total);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_zip(void)
{
    int ok = 1;
    nc_compression_t parms;

    printf("\n*** Testing zip compression.\n");
    reset();

    /* Use zip compression */
    parms.zip.level = DEFLATE_LEVEL;

    create(ZIP);
    CHECK(nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "zip", NC_NELEMS_ZIP, parms.params));
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    reset();
    open(ZIP);
    memset(array,0,sizeof(T)*total);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_nozip(void)
{
    int ok = 1;

    printf("\n*** Testing nozip compression.\n");
    reset();

    create(NOZIP);
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    reset();
    open(NOZIP);
    memset(array,0,sizeof(T)*total);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

int
main(int argc, char **argv)
{

    init(argc,argv);

    if(test[ZFP]) {
	test_zfp();
	SUMMARIZE_ERR;
    }

    if(test[FPZIP]) {
	test_fpzip();
	SUMMARIZE_ERR;
    }

    if(test[BZIP2]) {
	test_bzip2();
	SUMMARIZE_ERR;
    }

    if(test[SZIP]) {
	test_szip();
	SUMMARIZE_ERR;
    }

    if(test[ZIP]) {
	test_zip();
	SUMMARIZE_ERR;
    }

    if(test[NOZIP]) {
	test_nozip();
	SUMMARIZE_ERR;
    }

    FINAL_RESULTS;
}

/**************************************************/
/* Utilities */

static void
reset()
{
    int i;
    size_t count = 1;
    for(i=0;i<MAXDIMS;i++)
	count *= DIM[i];
    memset(array,0,sizeof(T)*count);
}

static void
init(int argc, char** argv)
{
    int i,j;
    unsigned int nelem = 0;
    supported[NOZIP] = 1;
    supported[ZIP] = 1;
#ifdef SZIP_COMPRESSION
    supported[SZIP] = 1;
#endif
#ifdef BZIP2_COMPRESSION
    supported[BZIP2] = 1;
#endif
#ifdef FPZIP_COMPRESSION
    supported[FPZIP] = 1;
    /* Validate nc_compression_t.fpzip size */
    nelem = (sizeof(((nc_compression_t*)0)->fpzip)/sizeof(unsigned int));
    if(NC_NELEMS_FPZIP < nelem) {
	fprintf(stderr,"NC_NELEMS_FPZIP=%d < |nc_compression_t.fpzip|=%d\n",
	        NC_NELEMS_FPZIP,nelem);
	abort();	
    }
#endif
#ifdef ZFP_COMPRESSION
    supported[ZFP] = 1;
    /* Validate nc_compression_t.zfp size */
    nelem = (sizeof(((nc_compression_t*)0)->zfp)/sizeof(unsigned int));
    if(NC_NELEMS_ZFP < nelem) {
	fprintf(stderr,"NC_NELEMS_ZFP=%d < |nc_compression_t.zfp|=%d\n",
	        NC_NELEMS_ZFP,nelem);
	abort();	
    }
#endif

    nelem = sizeof(nc_compression_t) / sizeof(unsigned int);
    if(NC_COMPRESSION_MAX_PARAMS < nelem) {
	fprintf(stderr,"NC_COMPRESSION_MAX_PARAMS=%d < |nc_compression_t|=%d\n",
	        NC_COMPRESSION_MAX_PARAMS,nelem);
	abort();	
    }

    for(i=1;i<argc;i++) {
        for(j=0;j<NZIP;j++) {
	    if(strcmp(argv[i],zipnames[j])==0 && supported[j])
		test[j] = 1;
	}
    }
    /* Allocate max size */
    array = (T*)malloc(sizeof(T)*product(MAXDIMS,DIM));
    /* use only NDIMS worth */
    total = product(NDIMS,DIM);
}

static void
odom_reset(void)
{
    memset(odom,0,sizeof(odom));
}

static int
odom_more(void)
{
    return (odom[0] < DIM[0]);
}

static int
odom_next(void)
{
    int i; /* do not make unsigned */
    for(i=NDIMS-1;i>=0;i--) {
        odom[i] += 1;
        if(odom[i] < DIM[i]) break;
	if(i == 0) return 0; /* leave the 0th entry if it overflows*/
	odom[i] = 0; /* reset this position*/
    }
    return 1;
}

static int
odom_offset(void)
{
    int i;
    int offset = 0;
    for(i=0;i<NDIMS;i++) {
	offset *= DIM[i];
	offset += odom[i];
    } 
    return offset;
}

static size_t
product(int ndims, size_t* dims)
{
    size_t prod = 1;
    int i;
    for(i=0;i<ndims;i++)
	prod *= dims[i];
    return prod;
}

static T
expected(void)
{
    int i;
    T offset = 0;

    for(i=0;i<NDIMS;i++) {
	offset *= DIM[i];
	offset += odom[i];
    } 
    return offset;
}
