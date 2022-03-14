/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <hdf5.h>
#include "netcdf.h"
#include "netcdf_aux.h"
#include "netcdf_filter.h"

#undef TESTODDSIZE

#undef DEBUG

#ifndef H5Z_FILTER_FLETCHER32
#define H5Z_FILTER_FLETCHER32      3
#endif

/* The C standard apparently defines all floating point constants as double;
   we rely on that in this code.
*/
#define DBLVAL 12345678.12345678

#define TEST_ID 32768

#define MAXERRS 8

#define MAXPARAMS 32

#define NPARAMS 14

static unsigned int baseline[NPARAMS];

static const char* testfile = NULL;

#define MAXDIMS 8

#define DFALT_TESTFILE "tmp_misc.nc"

#define spec "32768, -17b, 23ub, -25S, 27US, 77, 93U, 789f, 12345678.12345678d, -9223372036854775807L, 18446744073709551615UL"

#ifdef TESTODDSIZE
#define NDIMS 1
static size_t dimsize[NDIMS] = {4};
static size_t chunksize[NDIMS] = {3};
#else
#define NDIMS 4
static size_t dimsize[NDIMS] = {4,4,4,4};
static size_t chunksize[NDIMS] = {4,4,4,4};
#endif

static size_t ndims = NDIMS;

static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */
static size_t chunkproduct = 1; /* x-product over actual chunks */

static size_t pattern[MAXDIMS];

static int nerrs = 0;

static int ncid, varid;
static int dimids[MAXDIMS];
static float* array = NULL;
static float* expected = NULL;

/* Forward */
static int test_test1(void);
static void init(int argc, char** argv);
static void reset(void);

#define ERRR do { \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n", \
        __FILE__, __LINE__);                                \
nerrs++;\
} while (0)

static int
check(int err,int line)
{
    if(err != NC_NOERR) {
        fprintf(stderr,"fail (%d): %s\n",line,nc_strerror(err));
    }
    return NC_NOERR;
}
#define CHECK(x) check(x,__LINE__)

static int
verifychunks(void)
{
    int i;
    int store = -1;
    size_t localchunks[MAXDIMS];
    memset(localchunks,0,sizeof(localchunks));
    CHECK(nc_inq_var_chunking(ncid, varid, &store, localchunks));
    if(store != NC_CHUNKED) {
        fprintf(stderr,"bad chunk store\n");
        return 0;
    }
    for(i=0;i<ndims;i++) {
        if(chunksize[i] != localchunks[i]) {
            fprintf(stderr,"bad chunk size: %d\n",i);
            return 0;
        }
    }
    return 1;
}

static int
create(void)
{
    /* Create a file with one big variable, but whose dimensions arte not a multiple of chunksize (to see what happens) */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    return NC_NOERR;
}

static int
defvar(nc_type xtype)
{
    int i;

    /* Create a file with one big variable, but whose dimensions arte not a multiple of chunksize (to see what happens) */
    for(i=0;i<ndims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, dimsize[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", xtype, ndims, dimids, &varid));
    return NC_NOERR;
}

static int
setchunking(void)
{
    int store;

    store = NC_CHUNKED;
    CHECK(nc_def_var_chunking(ncid,varid,store,chunksize));
    if(!verifychunks())
        return NC_EINVAL;
    return NC_NOERR;
}

static void
insert(int index, void* src, size_t size)
{
    unsigned char src8[8];
    void* dst = &baseline[index];
    if(size == 8) {
	memcpy(src8,src,size);
	ncaux_h5filterspec_fix8(src8,0);
	src = src8;
    }
    memcpy(dst,src,size);
}

static void
buildbaseline(unsigned int testcasenumber)
{
    unsigned int val4;
    unsigned long long val8;
    float float4;
    double float8;

    baseline[0] = testcasenumber;
    switch (testcasenumber) {
    case 1:
        val4 = ((unsigned int)-17) & 0xff;
        insert(1,&val4,sizeof(val4)); /* 1 signed int*/
	val4 = (unsigned int)23;
        insert(2,&val4,sizeof(val4)); /* 2 unsigned int*/
        val4 = ((unsigned int)-25) & 0xffff;
        insert(3,&val4,sizeof(val4)); /* 3 signed int*/
	val4 = (unsigned int)27;
        insert(4,&val4,sizeof(val4)); /* 4 unsigned int*/
	val4 = (unsigned int)77;
        insert(5,&val4,sizeof(val4)); /* 5 signed int*/
	val4 = (unsigned int)93;
        insert(6,&val4,sizeof(val4)); /* 6 unsigned int*/
	float4 = 789.0f;
        insert(7,&float4,sizeof(float4)); /* 7 float */
	float8 = DBLVAL;
        insert(8,&float8,sizeof(float8)); /* 8 double */
	val8 = -9223372036854775807L;
        insert(10,&val8,sizeof(val8)); /* 10 signed long long */
	val8 = 18446744073709551615UL;
        insert(12,&val8,sizeof(val8)); /* 12 unsigned long long */
	break;
    case 2:
    case 3:
    	break;
    default:
	fprintf(stderr,"Unknown testcase number: %d\n",testcasenumber);
	abort();
    }
}

static int
test_test1(void)
{
    int ok = 1;
    int stat = NC_NOERR;

    reset();
    buildbaseline(1);
    fprintf(stderr,"test4: filter on a variable length type.\n");
    create();
    defvar(NC_STRING);
    setchunking();
    /* Do explicit filter; should fail */
    switch (stat = nc_def_var_filter(ncid,varid,H5Z_FILTER_FLETCHER32,0,NULL)) {
    case NC_EFILTER: break; /* XFAIL */
    case NC_NOERR: CHECK(NC_EINVAL); break;
    default:  CHECK(stat); break;
    }
    CHECK(nc_abort(ncid));
    return ok;
}

/**************************************************/
/* Utilities */

static void
reset()
{
    memset(array,0,sizeof(float)*actualproduct);
}

static void
init(int argc, char** argv)
{
    int i;

    /* get the testfile path */
    if(argc > 1)
        testfile = argv[1];
    else
        testfile = DFALT_TESTFILE;

    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<NDIMS;i++) {
        if(pattern[i] == 1)
	    chunksize[i] = 1;
        totalproduct *= dimsize[i];
        if(i < ndims) {
            actualproduct *= dimsize[i];
            chunkproduct *= chunksize[i];
        }
    }
    /* Allocate max size */
    array = (float*)calloc(1,sizeof(float)*actualproduct);
    expected = (float*)calloc(1,sizeof(float)*actualproduct);
}

/**************************************************/
int
main(int argc, char **argv)
{
#ifdef DEBUG
    H5Eprint1(stderr);
    nc_set_log_level(1);
#endif
    init(argc,argv);
    if(!test_test1()) ERRR;
    exit(nerrs > 0?1:0);
}
