/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "netcdf.h"
#include "netcdf_filter.h"

#undef TESTODDSIZE

#undef DEBUG

#define FILTER_ID 40000

#define MAXERRS 8

#define MAXPARAMS 32

#define MAXDIMS 8

#define DFALT_TESTFILE "tmp_filter_order.nc"

#define NPARAMS 1
#define PARAMVAL 17

#define NDIMS 4
static size_t dimsize[NDIMS] = {4,4,4,4};
static size_t chunksize[NDIMS] = {4,4,4,4};

static size_t ndims = NDIMS;

static int creating = 1; /* Default is to do filter test 1 */
static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */
static size_t chunkproduct = 1; /* x-product over actual chunks */

static int nerrs = 0;

static const char* testfile = NULL;

static int ncid, varid;
static int dimids[MAXDIMS];
static size_t odom[MAXDIMS];
static float* array = NULL;
static float* expected = NULL;

/* Forward */
static int filter_test1(void);
static void init(int argc, char** argv);
static void reset(void);
static void odom_reset(void);
static int odom_more(void);
static int odom_next(void);
static int odom_offset(void);
static float expectedvalue(void);

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

static void
report(const char* msg, int lineno)
{
    fprintf(stderr,"fail: line=%d %s\n",lineno,msg);
    exit(1);
}

#define CHECK(x) check(x,__LINE__)
#define REPORT(x) report(x,__LINE__)

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
    int i;

    /* Create a file with one big variable */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    for(i=0;i<ndims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, dimsize[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, ndims, dimids, &varid));
    return NC_NOERR;
}

static void
deffilters(void)
{
    unsigned int params[1];

    /* Register filter 0 */
    params[0] = 0;    
    printf("def filter id[0]=%d nparams=1 params[0]=%d\n",(int)FILTER_ID,(int)params[0]); fflush(stdout);
    CHECK(nc_def_var_filter(ncid,varid,FILTER_ID,1,params));
    params[0] = 1;
    printf("def filter id[1]=%d nparams=1 params[0]=%d\n",(int)FILTER_ID+1,(int)params[0]); fflush(stdout);
    CHECK(nc_def_var_filter(ncid,varid,FILTER_ID+1,1,params));
}

static void
inqfilters(void)
{
    unsigned int params[1];
    unsigned int filterids[2];
    size_t nfilters = 0;
    size_t nparams = 0;

    CHECK(nc_inq_var_filter_ids(ncid,varid,&nfilters,filterids));
    printf("inq filter ids(%d)=[%d,%d]\n",(int)nfilters,(int)filterids[0],(int)filterids[1]); fflush(stdout);
    if(nfilters != 2) REPORT("nfilters mismatch");
    if(filterids[0] != FILTER_ID+0) REPORT("0: filterids mismatch");
    if(filterids[1] != FILTER_ID+1) REPORT("1: filterids mismatch");
    CHECK(nc_inq_var_filter_info(ncid,varid,filterids[0],&nparams,params));;
    printf("inq filter id[0]=%d nparams=%d params[0]=%d\n",(int)filterids[0],(int)nparams,(int)params[0]); fflush(stdout);
    if(nparams != 1) REPORT("0: nparams mismatch");
    if(params[0] != 0) REPORT("0: param mismatch");
    CHECK(nc_inq_var_filter_info(ncid,varid,filterids[1],&nparams,params));
    printf("inq filter id[1]=%d nparams=%d params[0]=%d\n",(int)filterids[1],(int)nparams,(int)params[0]); fflush(stdout);
    if(nparams != 1) REPORT("1: nparams mismatch");
    if(params[0] != 1) REPORT("1: param mismatch");
}

static int
openfile(void)
{
    unsigned int filterids[2];
    unsigned int params[1];
    size_t nfilters = 0;
    size_t nparams = 0;
    int k;

    /* Open the file and check it. */
    CHECK(nc_open(testfile, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    /* Check the compression algorithms */
    CHECK(nc_inq_var_filter_ids(ncid,varid,&nfilters,filterids));
    if(nfilters != 2)
        return NC_EINVAL;
    for(k=0;k<nfilters;k++) {
        CHECK(nc_inq_var_filter_info(ncid,varid,filterids[k],&nparams,params));
        if(nparams != 1) {
            fprintf(stderr,"open: test nparams mismatch: %u\n",(unsigned)nparams);
            return NC_EFILTER;
	}
    }

    /* Verify chunking */
    if(!verifychunks())
        return 0;
    fflush(stderr);
    return 1;
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
fill(void)
{
   odom_reset();
   if(1) {
        int i;
        if(actualproduct <= 1) abort();
        for(i=0;i<actualproduct;i++)
            expected[i] = (float)i;
   } else {
       while(odom_more()) {
            int offset = odom_offset();
            float expect = expectedvalue();
            expected[offset] = expect;
            odom_next();
        }
   }
}

static int
compare(void)
{
    int errs = 0;
    printf("data comparison: |array|=%ld\n",(unsigned long)actualproduct); fflush(stdout);
    if(1)
    {
        int i;
        for(i=0;i<actualproduct;i++) {
            if(expected[i] != array[i]) {
                fprintf(stderr,"data mismatch: array[%d]=%f expected[%d]=%f\n",
                            i,array[i],i,expected[i]);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
        }
   } else
   {
       odom_reset();
       while(odom_more()) {
            int offset = odom_offset();
            float expect = expectedvalue();
            if(array[offset] != expect) {
                fprintf(stderr,"data mismatch: array[%d]=%f expected=%f\n",
                            offset,array[offset],expect);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
            odom_next();
       }
   }

   if(errs == 0)
        {printf("no data errors\n"); fflush(stdout);}
   return (errs == 0);
}

/* Test filter order on creation */
static int
filter_test1(void)
{
    int ok = 1;

    reset();

    printf("test1: filter order: create\n"); fflush(stdout);
    create();
    setchunking();
    deffilters();
    inqfilters();
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();

    nc_sync(ncid);

    printf("test1: compression.\n"); fflush(stdout);
    /* write array */
    CHECK(nc_put_var(ncid,varid,expected));

    printf("test1: decompression.\n"); fflush(stdout);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();

    CHECK(nc_close(ncid));

    return ok;
}

/* Test filter order on read */
static int
filter_test2(void)
{
    int ok = 1;

    reset();

    printf("test2: filter order: read\n"); fflush(stdout);

    /* Fill in the array */
    fill();

    printf("test2: decompression.\n"); fflush(stdout);
    reset();
    openfile();
    inqfilters();

    printf("test2: decompression.\n"); fflush(stdout);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();

    CHECK(nc_close(ncid));
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
odom_reset(void)
{
    memset(odom,0,sizeof(odom));
}

static int
odom_more(void)
{
    return (odom[0] < dimsize[0]);
}

static int
odom_next(void)
{
    int i; /* do not make unsigned */
    for(i=ndims-1;i>=0;i--) {
        odom[i] += 1;
        if(odom[i] < dimsize[i]) break;
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
    for(i=0;i<ndims;i++) {
        offset *= dimsize[i];
        offset += odom[i];
    }
    return offset;
}

static float
expectedvalue(void)
{
    int i;
    float offset = 0;

    for(i=0;i<ndims;i++) {
        offset *= dimsize[i];
        offset += odom[i];
    }
    return offset;
}

static void
usage(void)
{
    fprintf(stderr,"usage: test_filter_order read|create [path]\n");
    exit(1);
}

static void
init(int argc, char** argv)
{
    int i;

    if(argc == 1)
	usage();

    if(strcmp(argv[1],"read")==0) creating = 0;
    else if(strcmp(argv[1],"create")==0) creating = 1;
    else usage();

    /* get the testfile path */
    if(argc > 2)
        testfile = argv[2];
    else
        testfile = DFALT_TESTFILE;

    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<NDIMS;i++) {
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
    if(creating) {
        if(!filter_test1()) ERRR;
    } else {
        if(!filter_test2()) ERRR;
    }
    exit(nerrs > 0?1:0);
}
