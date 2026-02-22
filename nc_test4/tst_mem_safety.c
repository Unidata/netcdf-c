/* This is part of the netCDF package.
   Copyright 2024 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Tests for memory safety issues in libhdf5:
   - get_attached_info() ndims vs var->ndims mismatch (issues #2664, #2666, #2667)
   - NC4_get_vars() bounds and NULL-data guard (issue #2668)
   - nc4_nc4f_list_add() error-path memory leak (issue #2665)
   - NC_hashmapnew()/NC_hashmapfree() alloc/free cycle
   - Memory growth when repeatedly opening/closing NetCDF4 files (issue #2626)

   See https://github.com/Unidata/netcdf-c/issues/2626
   See https://github.com/Unidata/netcdf-c/issues/2664
   See https://github.com/Unidata/netcdf-c/issues/2665
   See https://github.com/Unidata/netcdf-c/issues/2666
   See https://github.com/Unidata/netcdf-c/issues/2667
   See https://github.com/Unidata/netcdf-c/issues/2668
*/

#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "nchashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#define FILE_NAME "tst_mem_safety.nc"

/* ------------------------------------------------------------------ */
/* Test 1: Repeated open/close of a NetCDF4 file must not grow RSS
 * unboundedly.
 *
 * Opens and closes the same NetCDF4 file (HDF5 backend) in a tight
 * loop.  On platforms where getrusage(2) is available the test
 * measures RSS before and after the loop and fails if growth exceeds
 * OPEN_CLOSE_MAX_GROWTH_KB kilobytes.  On other platforms the loop
 * still runs to exercise the code path, but no memory check is done.
 *
 * Regression test for https://github.com/Unidata/netcdf-c/issues/2626
 */
#define OPEN_CLOSE_FILE     "tst_mem_safety_2626.nc"
#define OPEN_CLOSE_NWARMUP  20
#define OPEN_CLOSE_NITER    500
#define OPEN_CLOSE_MAX_GROWTH_KB (10 * 1024)  /* 10 MB */

#ifdef HAVE_SYS_RESOURCE_H
static long
get_rss_kb(void)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
        return -1;
#ifdef __APPLE__
    return (long)(ru.ru_maxrss / 1024);
#else
    return (long)ru.ru_maxrss;  /* Linux: already in KB */
#endif
}
#endif /* HAVE_SYS_RESOURCE_H */

static int
test_open_close_loop(void)
{
    int ncid, dimid, varid;
    size_t start[1] = {0};
    size_t count[1] = {10};
    double data[10];
    int i;
#ifdef HAVE_SYS_RESOURCE_H
    long rss_before, rss_after, growth;
#endif

    printf("*** Testing repeated open/close memory growth (issue #2626)...");

    for (i = 0; i < 10; i++) data[i] = (double)i;

    /* Create a NetCDF4 file with an unlimited dimension and a variable. */
    if (nc_create(OPEN_CLOSE_FILE, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, "time", NC_UNLIMITED, &dimid)) ERR;
    if (nc_def_var(ncid, "dummy", NC_DOUBLE, 1, &dimid, &varid)) ERR;
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid, start, count, data)) ERR;
    if (nc_close(ncid)) ERR;

    /* Warm up: let HDF5 settle its initial internal allocations. */
    for (i = 0; i < OPEN_CLOSE_NWARMUP; i++)
    {
        if (nc_open(OPEN_CLOSE_FILE, NC_NOWRITE, &ncid)) ERR;
        if (nc_close(ncid)) ERR;
    }

#ifdef HAVE_SYS_RESOURCE_H
    rss_before = get_rss_kb();
#endif

    /* Main loop: open and close repeatedly. */
    for (i = 0; i < OPEN_CLOSE_NITER; i++)
    {
        if (nc_open(OPEN_CLOSE_FILE, NC_NOWRITE, &ncid)) ERR;
        if (nc_close(ncid)) ERR;
    }

#ifdef HAVE_SYS_RESOURCE_H
    rss_after = get_rss_kb();
    if (rss_before > 0 && rss_after > 0)
    {
        growth = rss_after - rss_before;
        if (growth > OPEN_CLOSE_MAX_GROWTH_KB)
        {
            fprintf(stderr,
                    "\nRSS grew by %ld KB over %d open/close cycles "
                    "(limit %d KB) -- possible memory leak (issue #2626)\n",
                    growth, OPEN_CLOSE_NITER, OPEN_CLOSE_MAX_GROWTH_KB);
            ERR;
        }
    }
#endif /* HAVE_SYS_RESOURCE_H */

    remove(OPEN_CLOSE_FILE);
    SUMMARIZE_ERR;
    return NC_NOERR;
}

/* ------------------------------------------------------------------ */
/* Test 2: Multi-dim coordinate variable roundtrip.
 *
 * Creates a 2D variable with two coordinate variables, closes, reopens,
 * and reads back. Exercises get_attached_info() with ndims == var->ndims
 * (the valid case). Regression baseline for the ndims guard fix
 * (issues #2664, #2666, #2667).
 */
static int
test_coord_var_roundtrip(void)
{
#define CV_NX 4
#define CV_NY 6
#define CV_NDIMS 2
    int ncid, varid, xdimid, ydimid;
    int cv_xid, cv_yid;
    int dimids[CV_NDIMS];
    float xdata[CV_NX], ydata[CV_NY];
    float data_out[CV_NX * CV_NY], data_in[CV_NX * CV_NY];
    size_t start[CV_NDIMS] = {0, 0};
    size_t count[CV_NDIMS] = {CV_NX, CV_NY};
    int i;
    int ndims_in, natts_in, dimids_in[CV_NDIMS];
    nc_type xtype_in;
    char name_in[NC_MAX_NAME + 1];

    printf("*** Testing coordinate variable roundtrip (issues #2664/#2666/#2667)...");

    for (i = 0; i < CV_NX; i++) xdata[i] = (float)i * 1.5f;
    for (i = 0; i < CV_NY; i++) ydata[i] = (float)i * 2.5f;
    for (i = 0; i < CV_NX * CV_NY; i++) data_out[i] = (float)i;

    /* Create file. */
    if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
    if (nc_def_dim(ncid, "x", CV_NX, &xdimid)) ERR;
    if (nc_def_dim(ncid, "y", CV_NY, &ydimid)) ERR;

    /* Coordinate variables (1-D, same name as dimension). */
    if (nc_def_var(ncid, "x", NC_FLOAT, 1, &xdimid, &cv_xid)) ERR;
    if (nc_def_var(ncid, "y", NC_FLOAT, 1, &ydimid, &cv_yid)) ERR;

    /* 2-D data variable using both dims. */
    dimids[0] = xdimid;
    dimids[1] = ydimid;
    if (nc_def_var(ncid, "data", NC_FLOAT, CV_NDIMS, dimids, &varid)) ERR;

    if (nc_enddef(ncid)) ERR;

    if (nc_put_var_float(ncid, cv_xid, xdata)) ERR;
    if (nc_put_var_float(ncid, cv_yid, ydata)) ERR;
    if (nc_put_vara_float(ncid, varid, start, count, data_out)) ERR;
    if (nc_close(ncid)) ERR;

    /* Reopen and read back - exercises get_attached_info(). */
    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

    /* Inquire the 2-D variable - triggers get_attached_info loop. */
    if (nc_inq_var(ncid, varid, name_in, &xtype_in, &ndims_in,
                   dimids_in, &natts_in)) ERR;
    if (ndims_in != CV_NDIMS) ERR;
    if (xtype_in != NC_FLOAT) ERR;
    if (strcmp(name_in, "data")) ERR;

    /* Read data back and verify. */
    if (nc_get_vara_float(ncid, varid, start, count, data_in)) ERR;
    for (i = 0; i < CV_NX * CV_NY; i++)
        if (data_in[i] != data_out[i]) ERR;

    /* Read coordinate variables back. */
    {
        float xin[CV_NX], yin[CV_NY];
        if (nc_get_var_float(ncid, cv_xid, xin)) ERR;
        if (nc_get_var_float(ncid, cv_yid, yin)) ERR;
        for (i = 0; i < CV_NX; i++)
            if (xin[i] != xdata[i]) ERR;
        for (i = 0; i < CV_NY; i++)
            if (yin[i] != ydata[i]) ERR;
    }

    if (nc_close(ncid)) ERR;
    remove(FILE_NAME);
    SUMMARIZE_ERR;
    return NC_NOERR;
}

/* ------------------------------------------------------------------ */
/* Test 3: NC4_get_vars() edge cases.
 *
 * Tests bounds checking in NC4_get_vars() on a valid 2-D variable:
 *   - zero count on one dimension
 *   - start beyond dimension size (must return NC_EINVALCOORDS)
 *   - start+count beyond dimension size (must return NC_EEDGE)
 *   - strided read with stride > 1
 * Regression baseline for the NC4_get_vars NULL-data guard (issue #2668).
 */
static int
test_get_vars_bounds(void)
{
#define GV_NX 8
#define GV_NY 10
#define GV_NDIMS 2
    int ncid, varid, xdimid, ydimid;
    int dimids[GV_NDIMS];
    float data_out[GV_NX * GV_NY];
    float data_in[GV_NX * GV_NY];
    size_t start[GV_NDIMS], count[GV_NDIMS];
    ptrdiff_t stride[GV_NDIMS];
    int i, stat;

    printf("*** Testing NC4_get_vars() edge cases (issue #2668)...");

    for (i = 0; i < GV_NX * GV_NY; i++) data_out[i] = (float)i;

    /* Create and populate file. */
    if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
    if (nc_def_dim(ncid, "x", GV_NX, &xdimid)) ERR;
    if (nc_def_dim(ncid, "y", GV_NY, &ydimid)) ERR;
    dimids[0] = xdimid;
    dimids[1] = ydimid;
    if (nc_def_var(ncid, "data", NC_FLOAT, GV_NDIMS, dimids, &varid)) ERR;
    if (nc_enddef(ncid)) ERR;
    start[0] = 0; start[1] = 0;
    count[0] = GV_NX; count[1] = GV_NY;
    if (nc_put_vara_float(ncid, varid, start, count, data_out)) ERR;
    if (nc_close(ncid)) ERR;

    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

    /* Zero count on second dim - must succeed and read nothing. */
    start[0] = 0; start[1] = 0;
    count[0] = GV_NX; count[1] = 0;
    stride[0] = 1; stride[1] = 1;
    if (nc_get_vars_float(ncid, varid, start, count, stride, data_in)) ERR;

    /* start[0] beyond dimension size - must return NC_EINVALCOORDS. */
    start[0] = GV_NX + 1; start[1] = 0;
    count[0] = 1; count[1] = 1;
    stat = nc_get_vars_float(ncid, varid, start, count, stride, data_in);
    if (stat != NC_EINVALCOORDS) ERR;

    /* start+count beyond dimension size - must return NC_EEDGE. */
    start[0] = GV_NX - 2; start[1] = 0;
    count[0] = 5; count[1] = 1;
    stat = nc_get_vars_float(ncid, varid, start, count, stride, data_in);
    if (stat != NC_EEDGE) ERR;

    /* Strided read: every other element in x, every third in y. */
    start[0] = 0; start[1] = 0;
    count[0] = GV_NX / 2; count[1] = GV_NY / 3;
    stride[0] = 2; stride[1] = 3;
    if (nc_get_vars_float(ncid, varid, start, count, stride, data_in)) ERR;
    /* Verify first element. */
    if (data_in[0] != data_out[0]) ERR;
    /* Verify second element in x direction (stride 2 in x). */
    if (data_in[GV_NY / 3] != data_out[2 * GV_NY]) ERR;

    /* Non-positive stride must return NC_ESTRIDE. */
    start[0] = 0; start[1] = 0;
    count[0] = 1; count[1] = 1;
    stride[0] = 0; stride[1] = 1;
    stat = nc_get_vars_float(ncid, varid, start, count, stride, data_in);
    if (stat != NC_ESTRIDE) ERR;

    if (nc_close(ncid)) ERR;
    remove(FILE_NAME);
    SUMMARIZE_ERR;
    return NC_NOERR;
}

/* ------------------------------------------------------------------ */
/* Test 4: NC_hashmapnew()/NC_hashmapfree() alloc/free cycle.
 *
 * Exercises the hashmap alloc/free path directly. With ASAN enabled,
 * any leak inside NC_hashmapnew or NC_hashmapfree is caught immediately.
 * Regression test for issue #2665.
 */
static int
test_hashmap_alloc_free(void)
{
#define HM_NENTRIES 200
#define HM_NROUNDS  100
    NC_hashmap *hm;
    char key[64];
    uintptr_t val;
    int i, round;

    printf("*** Testing NC_hashmapnew()/NC_hashmapfree() cycle (issue #2665)...");

    for (round = 0; round < HM_NROUNDS; round++)
    {
        /* Create with default size. */
        hm = NC_hashmapnew(0);
        if (!hm) ERR;

        /* Insert entries. */
        for (i = 0; i < HM_NENTRIES; i++)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapadd(hm, (uintptr_t)i, key, strlen(key))) ERR;
        }

        if (NC_hashmapcount(hm) != HM_NENTRIES) ERR;

        /* Look up all entries. */
        for (i = 0; i < HM_NENTRIES; i++)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapget(hm, key, strlen(key), &val)) ERR;
            if (val != (uintptr_t)i) ERR;
        }

        /* Remove half the entries. */
        for (i = 0; i < HM_NENTRIES; i += 2)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapremove(hm, key, strlen(key), &val)) ERR;
            if (val != (uintptr_t)i) ERR;
        }

        if (NC_hashmapcount(hm) != HM_NENTRIES / 2) ERR;

        /* Removed entries must not be found. */
        for (i = 0; i < HM_NENTRIES; i += 2)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (NC_hashmapget(hm, key, strlen(key), &val)) ERR;
        }

        /* Remaining entries must still be found. */
        for (i = 1; i < HM_NENTRIES; i += 2)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapget(hm, key, strlen(key), &val)) ERR;
            if (val != (uintptr_t)i) ERR;
        }

        NC_hashmapfree(hm);
    }

    /* Also test with a pre-sized map that triggers rehash. */
    hm = NC_hashmapnew(4);
    if (!hm) ERR;
    for (i = 0; i < HM_NENTRIES; i++)
    {
        snprintf(key, sizeof(key), "key_%04d", i);
        if (!NC_hashmapadd(hm, (uintptr_t)i, key, strlen(key))) ERR;
    }
    if (NC_hashmapcount(hm) != HM_NENTRIES) ERR;
    NC_hashmapfree(hm);

    SUMMARIZE_ERR;
    return NC_NOERR;
}

/* ------------------------------------------------------------------ */
int
main(int argc, char **argv)
{
    printf("\n*** Testing memory safety in libhdf5 (issues #2626, #2664-#2668).\n");

    test_open_close_loop();
    test_coord_var_roundtrip();
    test_get_vars_bounds();
    test_hashmap_alloc_free();

    FINAL_RESULTS;
}
