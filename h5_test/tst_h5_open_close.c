/* This is part of the netCDF package.
   Copyright 2026 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Pure-HDF5 regression test for the repeated open/close memory growth
   reported in https://github.com/Unidata/netcdf-c/issues/2626.

   This test does NOT use netCDF; it exercises HDF5 directly using the
   same H5F_CLOSE_WEAK + open/close pattern that netCDF-4 uses.  This
   isolates whether the growth originates in HDF5 itself (nothing we
   can fix in netCDF-C) or in netCDF-C's HDF5 usage.

   NOTE: The RSS memory-growth check is intended to run on Linux (Ubuntu)
   only.  On macOS, getrusage()/ru_maxrss returns the peak (high-water
   mark) RSS for the entire process lifetime and never decreases, making
   before/after comparisons meaningless.  The build system therefore skips
   registering this test on macOS CI.

   Ed Hartnett, 3/17/26
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "h5_err_macros.h"
#include <hdf5.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#define FILE_NAME "tst_h5_open_close.h5"
#define NWARMUP  20
#define NITER    500
/* 1024 KB (1 MB): absorbs OS page-granularity noise over 500 cycles
 * while still catching a real per-open leak (~2 KB/cycle would trip
 * this).  See https://github.com/Unidata/netcdf-c/issues/2626. */
#define MAX_GROWTH_KB 1024

#ifdef HAVE_SYS_RESOURCE_H
/* Linux: getrusage()/ru_maxrss is the peak RSS in KB. */
static long
get_rss_kb(void)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
        return -1;
    return (long)ru.ru_maxrss;
}
#endif /* HAVE_SYS_RESOURCE_H */

int
main()
{
    hid_t fid, sid, did, fapl;
    hsize_t dims[1] = {10};
    double data[10] = {0};
    int i;
#ifdef HAVE_SYS_RESOURCE_H
    long rss_before, rss_after, growth;
#endif

    printf("\n*** Checking pure-HDF5 open/close memory growth (issue #2626).\n");
    printf("*** Checking repeated H5Fopen/H5Fclose does not grow RSS...");

    /* Create a small HDF5 file with one dataset. */
    if ((fid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC,
                         H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
    if ((sid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
    if ((did = H5Dcreate2(fid, "dummy", H5T_NATIVE_DOUBLE, sid,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
    if (H5Dwrite(did, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                 H5P_DEFAULT, data) < 0) ERR;
    if (H5Dclose(did) < 0) ERR;
    if (H5Sclose(sid) < 0) ERR;
    if (H5Fclose(fid) < 0) ERR;

    /* Use H5F_CLOSE_WEAK, matching netCDF-4's open behaviour. */
    if ((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
    if (H5Pset_fclose_degree(fapl, H5F_CLOSE_WEAK) < 0) ERR;

    /* Warm up: let HDF5 settle its initial internal allocations. */
    for (i = 0; i < NWARMUP; i++)
    {
        if ((fid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl)) < 0) ERR;
        if (H5Fclose(fid) < 0) ERR;
    }

#ifdef HAVE_SYS_RESOURCE_H
    rss_before = get_rss_kb();
#endif

    for (i = 0; i < NITER; i++)
    {
        if ((fid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl)) < 0) ERR;
        if (H5Fclose(fid) < 0) ERR;
    }

#ifdef HAVE_SYS_RESOURCE_H
    rss_after = get_rss_kb();
    growth = (rss_before > 0 && rss_after > 0) ? rss_after - rss_before : 0;
    printf("\n    RSS growth: %ld KB over %d open/close cycles (limit %d KB)\n",
           growth, NITER, MAX_GROWTH_KB);
    if (rss_before > 0 && rss_after > 0 && growth > MAX_GROWTH_KB)
    {
        fprintf(stderr,
                "FAIL: pure-HDF5 RSS grew by %ld KB over %d open/close "
                "cycles (limit %d KB) -- growth is inside HDF5 itself "
                "(issue #2626)\n",
                growth, NITER, MAX_GROWTH_KB);
        ERR;
    }
#endif /* HAVE_SYS_RESOURCE_H */

    if (H5Pclose(fapl) < 0) ERR;
    remove(FILE_NAME);

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
