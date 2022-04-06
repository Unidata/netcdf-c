/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

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

#define MAXPARAMS 32

#ifdef TESTNCZARR
#define DFALT_TESTFILE "file://tmp_filter_avail.file#mode=nczarr,file"
#else
#define DFALT_TESTFILE "tmp_filter_avail.nc"
#endif

static const char* testfile = NULL;
static int nerrs = 0;

static int ncid;

/* Forward */
static int test_test1(void);
static void init(int argc, char** argv);

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
test_test1(void)
{
    int stat = NC_NOERR;

    printf("test1: bzip2 availability\n");
    CHECK(nc_create(testfile,NC_NETCDF4|NC_CLOBBER,&ncid));
    CHECK(nc_enddef(ncid));
    switch (stat = nc_inq_filter_avail(ncid,H5Z_FILTER_BZIP2)) {
    case NC_NOERR: break;
    case NC_ENOFILTER: break;
    default: CHECK(stat); goto done;
    }
    if(stat == NC_ENOFILTER) {
	printf("*** FAIL: filter %d not available\n",H5Z_FILTER_BZIP2);
    } else {
	printf("*** PASS: filter %d available\n",H5Z_FILTER_BZIP2);
    }

    CHECK(nc_abort(ncid));
done:
    return stat;
}

/**************************************************/
/* Utilities */

static void
init(int argc, char** argv)
{
    /* get the testfile path */
    if(argc > 1)
        testfile = argv[1];
    else
        testfile = DFALT_TESTFILE;
}

/**************************************************/
int
main(int argc, char **argv)
{
#ifdef USE_HDF5
#ifdef DEBUG
    H5Eprint1(stderr);
    nc_set_log_level(1);
#endif
#endif
    init(argc,argv);
    if(test_test1() != NC_NOERR) ERRR;
    exit(nerrs > 0?1:0);
}
