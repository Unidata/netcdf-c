/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

/* This test is derived from nc_test4/test_filter_vlen.c */
#define TESTNCZARR

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef USE_HDF5
#include <hdf5.h>
#endif
#include "netcdf.h"
#include "netcdf_aux.h"
#include "netcdf_filter.h"

#undef DEBUG

#define FILTER_ID 1 /*deflate*/

#define MAXERRS 8

#define MAXPARAMS 32

#define NPARAMS 14

static const char* testfile = NULL;

#define MAXDIMS 8

#ifdef TESTNCZARR
#define DFALT_TESTFILE "file://tmp_filter_vlen.nc#mode=nczarr,file"
#else
#define DFALT_TESTFILE "tmp_filter_vlen.nc"
#endif

#define NDIMS 4
static size_t dimsize[NDIMS] = {4,4,4,4};

static size_t ndims = NDIMS;

static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */

static int nerrs = 0;

static int ncid, varid;
static int dimids[MAXDIMS];
static char** array = NULL;

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
create(void)
{
    /* Create a file with one variable */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    return NC_NOERR;
}

static int
defvar(nc_type xtype)
{
    size_t i;

    /* Create a file with one variable-sized variable */
    for(i=0;i<ndims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%u",(unsigned)i);
        CHECK(nc_def_dim(ncid, dimname, dimsize[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", xtype, ndims, dimids, &varid));
    return NC_NOERR;
}

static int
reopen(void)
{
    size_t i;
    
    CHECK(nc_open(testfile, NC_NETCDF4, &ncid));
    for(i=0;i<ndims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%u",(unsigned)i);
        CHECK(nc_inq_dimid(ncid, dimname, &dimids[i]));
        CHECK(nc_inq_dim(ncid, dimids[i], NULL, &dimsize[i]));
    }
    CHECK(nc_inq_varid(ncid, "var", &varid));
    return NC_NOERR;
}

/* Test that a filter is a variable length var is defined */
static int
test_test1(void)
{
    int ok = 1;
    size_t nfilters = 0;
    unsigned filterids[64];
    unsigned params[NPARAMS] = {5};
    size_t nparams = 0;

    fprintf(stderr,"test_test1: filter on a variable length type.\n");
    create();
    defvar(NC_STRING);
    /* Do explicit filter; should never fail, but may produce log warning */
    CHECK(nc_def_var_filter(ncid,varid,FILTER_ID,1,params));
    /* Now see if filter was defined or not */
    memset(filterids,0,sizeof(filterids));
    params[0] = 5;
    CHECK(nc_inq_var_filter_ids(ncid,varid,&nfilters,filterids));
    fprintf(stderr,"test_test1: nc_var_filter_ids: nfilters=%u filterids[0]=%d\n",(unsigned)nfilters,filterids[0]);
    if(nfilters != 1 && filterids[0] != FILTER_ID) {
	fprintf(stderr,"test_test1: nc_var_filter_ids: failed\n");
	ok = 0;
    }
    params[0] = 0;
    CHECK(nc_inq_var_filter_info(ncid, varid, filterids[0], &nparams, params));
    fprintf(stderr,"test_test1: nc_inq_var_filter_info: nparams=%u params[0]=%u\n",(unsigned)nparams,(unsigned)params[0]);
    return ok;
}

/* Test that a filter on a variable length var is suppressed */
static int
test_test2(void)
{
    int stat = NC_NOERR;
    int ok = 1;
    size_t i;

    reset();
    fprintf(stderr,"test_test2: write with filter on a variable length type.\n");
    /* generate the data to write */
    for(i=0;i<actualproduct;i++) {
        char digits[64];
	snprintf(digits,sizeof(digits),"%u",(unsigned)i);
	array[i] = strdup(digits);
    }
    /* write the data */
    if((stat=nc_put_var(ncid,varid,(void*)array))) {
	fprintf(stderr,"test_test2: nc_put_var: error = (%d)%s\n",stat,nc_strerror(stat));
	ok = 0;
	goto done;
    }
    /* re-read the data */
    reset();
    if((stat=nc_get_var(ncid,varid,(void*)array))) {
	fprintf(stderr,"test_test2: nc_get_var: error = (%d)%s\n",stat,nc_strerror(stat));
	ok = 0;
	goto done;
    }
    /* verify the data */    
    for(i=0;i<actualproduct;i++) {
	unsigned value = 0xffffffff;
	if(array[i] != NULL)
	    sscanf(array[i],"%u",&value);
	if(array[i] == NULL || i != value) {
	    fprintf(stderr,"test_test2: nc_get_var: value mismatch at %u\n",(unsigned)i);
	    ok = 0;
	    goto done;
	}
    }
    nc_close(ncid);
done:
    return ok;
}

/* Test that a filter on a variable length var is suppressed */
static int
test_test3(void)
{
    int stat = NC_NOERR;
    int ok = 1;
    size_t i,nfilters;
    unsigned filterids[64];

    fprintf(stderr,"test_test3: re-open variable with filter on a variable length type and verify state.\n");

    reopen();    

    /* verify filter state */
    memset(filterids,0,sizeof(filterids));
    CHECK(nc_inq_var_filter_ids(ncid,varid,&nfilters,filterids));
    fprintf(stderr,"test_test3: nc_var_filter_ids: nfilters=%u filterids[0]=%d\n",(unsigned)nfilters,filterids[0]);
    if(nfilters != 1 && filterids[0] != FILTER_ID) {
	fprintf(stderr,"test_test3: nc_var_filter_ids: failed\n");
	ok = 0;
	goto done;
    }

    /* re-read the data */
    reset();
    if((stat=nc_get_var(ncid,varid,(void*)array))) {
	fprintf(stderr,"test_test3: nc_get_var: error = (%d)%s\n",stat,nc_strerror(stat));
	ok = 0;
	goto done;
    }
    /* verify the data */    
    for(i=0;i<actualproduct;i++) {
	unsigned value = 0xffffffff;
	if(array[i] != NULL)
	    sscanf(array[i],"%u",&value);
	if(array[i] == NULL || i != value) {
	    fprintf(stderr,"test_test3: nc_get_var: value mismatch at %u\n",(unsigned)i);
	    ok = 0;
	    goto done;
	}
    }
    nc_close(ncid);
done:
    return ok;
}

/**************************************************/
/* Utilities */

static void
reset()
{
    size_t i;
    for(i=0;i<actualproduct;i++) {
        if(array[i]) free(array[i]);
	array[i] = NULL;
    }
}

static void
init(int argc, char** argv)
{
    size_t i;

    /* get the testfile path */
    if(argc > 1)
        testfile = argv[1];
    else
        testfile = DFALT_TESTFILE;

    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    for(i=0;i<NDIMS;i++) {
        totalproduct *= dimsize[i];
        if(i < ndims) {
            actualproduct *= dimsize[i];
        }
    }
    /* Allocate max size */
    array = (char**)calloc(1,sizeof(char*)*actualproduct);
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
    if(!test_test2()) ERRR;
    if(!test_test3()) ERRR;
    fprintf(stderr,"*** %s\n",(nerrs > 0 ? "FAILED" : "PASS"));
    exit(nerrs > 0?1:0);
}
