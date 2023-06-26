/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_HDF5_H
#include <hdf5.h>
#endif
#include "netcdf.h"
#include "netcdf_aux.h"
#include "netcdf_filter.h"

#undef DEBUG

#ifndef H5Z_FILTER_BZIP2
#define H5Z_FILTER_BZIP2      307
#endif

#define TEST_ID 32768

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
test_test1(void)
{
    int ok = 1;
    int id = -1;
    size_t nparams;    

    reset();
    fprintf(stderr,"test4: filter on a variable length type.\n");
    create();
    defvar(NC_STRING);
    /* Do explicit filter; should never fail, but may produce log warning */
    CHECK(nc_def_var_filter(ncid,varid,H5Z_FILTER_BZIP2,0,NULL));
    /* Now see if filter was defined or not */
    CHECK(nc_inq_var_filter(ncid,varid,&id,&nparams,NULL));
    if(id > 0) {
	fprintf(stderr,"*** id=%d\n",id);
	ok = 0;
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
    fprintf(stderr,"*** %s\n",(nerrs > 0 ? "FAILED" : "PASS"));
    exit(nerrs > 0?1:0);
}
