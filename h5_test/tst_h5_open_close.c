/* This is part of the netCDF package.
   Copyright 2026 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Pure-HDF5 regression test for the repeated open/close memory growth
   reported in https://github.com/Unidata/netcdf-c/issues/2626.

   This test does NOT use netCDF; it exercises HDF5 directly using the
   same H5F_CLOSE_WEAK + open/close pattern that netCDF-4 uses.  This
   isolates whether the growth originates in HDF5 itself (nothing we
   can fix in netCDF-C) or in netCDF-C's HDF5 usage.

   The test always prints the measured RSS growth so the value is
   visible in CI logs even when the test passes.  On platforms without
   getrusage(2) the loop still runs but no RSS limit is enforced.

   Ed Hartnett
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "h5_err_macros.h"
#include <hdf5.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef __APPLE__
#include <mach/mach.h>
#endif

#define FILE_NAME "tst_h5_open_close.h5"
#define NWARMUP  20
#define NITER    500
/* 1024 KB (1 MB): absorbs OS page-granularity noise over 500 cycles
 * while still catching a real per-open leak (~2 KB/cycle would trip
 * this).  See https://github.com/Unidata/netcdf-c/issues/2626. */
#define MAX_GROWTH_KB 1024

#ifdef __APPLE__
/* On macOS, getrusage()/ru_maxrss is the peak (high-water mark) RSS for the
 * entire process lifetime -- it only ever increases and never reflects memory
 * that has been freed.  Using it to measure growth across a loop would always
 * show positive "growth" even for a perfectly leak-free library, because HDF5
 * may peak higher at some point mid-loop than at the start.
 *
 * Instead, use task_info(TASK_BASIC_INFO) which returns the *current* resident
 * set size of the task at the moment of the call, making before/after
 * comparisons meaningful. */
static long
get_rss_kb(void)
{
    struct task_basic_info info;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO,
                  (task_info_t)&info, &count) != KERN_SUCCESS)
        return -1;
    return (long)(info.resident_size / 1024);
}
#elif defined(HAVE_SYS_RESOURCE_H)
static long
get_rss_kb(void)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
        return -1;
    return (long)ru.ru_maxrss;  /* Linux: current RSS, already in KB */
}
#endif /* __APPLE__ / HAVE_SYS_RESOURCE_H */

int
main()
{
    hid_t fid, sid, did, fapl;
    hsize_t dims[1] = {10};
    double data[10] = {0};
    int i;
#if defined(__APPLE__) || defined(HAVE_SYS_RESOURCE_H)
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

#if defined(__APPLE__) || defined(HAVE_SYS_RESOURCE_H)
    rss_before = get_rss_kb();
#endif

    for (i = 0; i < NITER; i++)
    {
        if ((fid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl)) < 0) ERR;
        if (H5Fclose(fid) < 0) ERR;
    }

#if defined(__APPLE__) || defined(HAVE_SYS_RESOURCE_H)
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
#endif /* __APPLE__ / HAVE_SYS_RESOURCE_H */

    if (H5Pclose(fapl) < 0) ERR;
    remove(FILE_NAME);

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
