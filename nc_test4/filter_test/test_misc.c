/*
  Copyright 2008, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netcdf.h"

#define TEST_ID 32768

#define MAXERRS 8

#define MAXPARAMS 32

#define NBASELINE 14

static const unsigned int baseline[NBASELINE] = {
1,   /* 0 testcase # */
-17, /* 1 signed int*/
23,  /* 2 unsigned int*/
-25, /* 3 signed int*/
27,  /* 4 unsigned int*/
77,  /* 5 signed int*/
93,  /* 65 unsigned int*/
1145389056U, /* 7 float*/
697067329, 2723935171,    /* 8-9 double*/
128, 16777216,            /* 10-11 signed long long*/
4294967295, 4294967295,   /* 12-13 unsigned long long*/
};

#define MAXDIMS 8

#define DEFAULTACTUALDIMS 4
#define DEFAULTDIMSIZE 4
#define DEFAULTCHUNKSIZE 4

static size_t dimsize = DEFAULTDIMSIZE;
static size_t chunksize = DEFAULTCHUNKSIZE;
static size_t actualdims = DEFAULTACTUALDIMS;
static size_t pattern[MAXDIMS];

static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */
static size_t chunkproduct = 1; /* x-product over actual chunks */

static size_t dims[MAXDIMS];
static size_t chunks[MAXDIMS];

static int nerrs = 0;

static int ncid, varid;
static int dimids[MAXDIMS];
static size_t odom[MAXDIMS];
static float* array = NULL;
static float* expected = NULL;

static unsigned int filterid = 0;
static size_t nparams = 0;
static unsigned int params[MAXPARAMS];

/* Forward */
static int test_test1(void);
static void init(int argc, char** argv);
static void reset(void);
static void odom_reset(void);
static int odom_more(void);
static int odom_next(void);
static int odom_offset(void);
static float expectedvalue(void);
static void verifyparams(void);

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

static char*
filenamefor(void)
{
    return strdup("testmisc.nc");
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
    for(i=0;i<actualdims;i++) {
        if(chunksizes[i] != chunks[i]) {
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
    char* testfile = filenamefor();

    /* Create a file with one big variable. */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    for(i=0;i<actualdims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, dims[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, actualdims, dimids, &varid));
    return NC_NOERR;
}

static void
setvarfilter(void)
{
    size_t i;
    CHECK(nc_def_var_filter(ncid,varid,TEST_ID,NBASELINE,baseline));
    verifyparams();
}

static void
verifyparams(void)
{
    size_t i;
    CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
    if(filterid != TEST_ID) REPORT("id mismatch");
    if(nparams != NBASELINE) REPORT("nparams mismatch");
    for(i=0;i<nparams;i++) {
        if(params[i] != baseline[i])
            REPORT("param mismatch");
    }
}

static int
open(void)
{
    char* testfile = filenamefor();
    unsigned int* params;

    /* Open the file and check it. */
    CHECK(nc_open(testfile, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    /* Check the compression algorithm */
    CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,NULL));
    if(nparams > 0) {
        params = (unsigned int*)malloc(sizeof(unsigned int)*nparams);
        if(params == NULL)
            return NC_ENOMEM;
        CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
    }
    if(filterid != TEST_ID) {
        printf("open: test id mismatch: %d\n",filterid);
        return NC_EFILTER;
    }
    if(nparams != NBASELINE) {
	size_t i;
	unsigned int inqparams[MAXPARAMS];
        printf("nparams  mismatch\n");
        for(nerrs=0,i=0;i<nparams;i++) {
            if(inqparams[i] != baseline[i]) {
                printf("open: testparam mismatch: %d\n",i);
		nerrs++;
	    }
	}
    }
    if(nerrs > 0) return NC_EFILTER; 

    /* Verify chunking */
    if(!verifychunks())
        return 0;
    fflush(stderr);
    return 1;
}

static int
setchunking(void)
{
    int i;
    int store;

    store = NC_CHUNKED;
    CHECK(nc_def_var_chunking(ncid,varid,store,chunks));
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
    fprintf(stderr,"data comparison: |array|=%d\n",actualproduct);
    if(1)
    {
        int i;
        for(i=0;i<actualproduct;i++) {
            if(expected[i] != array[i]) {
                fprintf(stderr,"mismatch: array[%d]=%f expected[%d]=%f\n",
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
                fprintf(stderr,"mismatch: array[%d]=%f expected=%f\n",
                            offset,array[offset],expect);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
            odom_next();
       }
   }

   if(errs == 0)
        fprintf(stderr,"no data errors\n");
   return (errs == 0);
}

static void
showparameters(void)
{
    int i;
    printf("test: nparams=%ld: params=",(unsigned long)nparams);
    for(i=0;i<nparams;i++) {
        printf(" %u",params[i]);
    }
    printf("\n");
    for(i=0;i<actualdims;i++)
        printf("%s%d",(i==0?" chunks=":","),chunks[i]);
    printf("\n");
}

static int
test_test1(void)
{
    int ok = 1;

    reset();

    printf("test1: compression.\n");
    create();
    setchunking();
    setvarfilter();
    showparameters();
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(nc_put_var(ncid,varid,expected));
    CHECK(nc_close(ncid));

    printf("test1: decompression.\n");
    reset();
    open();
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
    return (odom[0] < dims[0]);
}

static int
odom_next(void)
{
    int i; /* do not make unsigned */
    for(i=actualdims-1;i>=0;i--) {
        odom[i] += 1;
        if(odom[i] < dims[i]) break;
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
    for(i=0;i<actualdims;i++) {
        offset *= dims[i];
        offset += odom[i];
    } 
    return offset;
}

static float
expectedvalue(void)
{
    int i;
    float offset = 0;

    for(i=0;i<actualdims;i++) {
        offset *= dims[i];
        offset += odom[i];
    } 
    return offset;
}

static void
init(int argc, char** argv)
{
    int i;
    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<MAXDIMS;i++) {
        dims[i] = dimsize;
        chunks[i] = (pattern[i] == 1 ? 1 : chunksize);
        totalproduct *= dims[i];
        if(i < actualdims) {
            actualproduct *= dims[i];
            chunkproduct *= chunks[i];
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
    init(argc,argv);
    if(!test_test1()) ERRR;
    exit(nerrs > 0?1:0);
}

