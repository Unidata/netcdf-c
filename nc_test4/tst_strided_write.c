/*! \file tst_strided_write.c

Test for GitHub issue #1877: NetCDF slow writes when using the stride parameter.

https://github.com/Unidata/netcdf-c/issues/1877

The issue reports that nc_put_vars_double() with a stride parameter is orders
of magnitude slower than an equivalent loop of nc_put_vara_double() calls.
The root cause is that NCDEFAULT_put_vars() writes one element at a time via
an odometer loop, while NC4_put_vars() uses HDF5's H5Sselect_hyperslab() for
a single bulk write.

This test:
  1. Verifies that strided writes produce correct data (correctness test).
  2. On platforms with sys/time.h, verifies that strided writes are not
     pathologically slow compared to looped writes (performance regression test).

The dimensions match the original bug report: 5184 x 228 x 40, written with
stride (1, 1, 5) along the third dimension.

Edward Hartnett, 2/22/26

Copyright 2026 University Corporation for Atmospheric Research/Unidata.
See COPYRIGHT file for more info.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include "netcdf.h"
#include "err_macros.h"

/* Dimensions matching the original bug report. */
#define DIM0_LEN  5184
#define DIM1_LEN  228
/* The unlimited dimension has 40 "slabs", written with stride 5. */
#define DIM2_SLABS  40
#define DIM2_STRIDE 5
/* The unlimited dimension must be large enough to hold the last strided index.
 * Last index = 0 + (DIM2_SLABS - 1) * DIM2_STRIDE = 39 * 5 = 195. */
#define DIM2_LEN    (DIM2_SLABS * DIM2_STRIDE)  /* 200 */

#define FILE_STRIDED "tst_strided_write_s.nc"
#define FILE_LOOPED  "tst_strided_write_l.nc"

/* Total number of values written (one per strided position). */
#define NVALS ((size_t)(DIM0_LEN) * (size_t)(DIM1_LEN) * (size_t)(DIM2_SLABS))

/*
 * Allocate and fill the source data buffer with recognizable values.
 * Layout: data[d0][d1][z]  (z innermost, DIM2_SLABS z-values per (d0,d1) pair).
 * This is the natural memory order for nc_put_vars with stride on dim2.
 * Returns NULL on allocation failure.
 */
static double *
make_data(void)
{
    size_t i;
    double *data = (double *)malloc(NVALS * sizeof(double));
    if (!data) return NULL;
    for (i = 0; i < NVALS; i++)
        data[i] = (double)i;
    return data;
}

/*
 * Create a NetCDF-4 file with a 3-D variable (dim0 x dim1 x unlimited).
 * Returns the open ncid and varid via output parameters.
 */
static int
create_file(const char *filename, int *ncid_out, int *varid_out)
{
    int ncid, varid;
    int dimids[3];

    if (nc_create(filename, NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, "d0", DIM0_LEN, &dimids[0])) ERR;
    if (nc_def_dim(ncid, "d1", DIM1_LEN, &dimids[1])) ERR;
    if (nc_def_dim(ncid, "d2", NC_UNLIMITED, &dimids[2])) ERR;
    if (nc_def_var(ncid, "data", NC_DOUBLE, 3, dimids, &varid)) ERR;
    if (nc_enddef(ncid)) ERR;

    *ncid_out = ncid;
    *varid_out = varid;
    return 0;
}

/*
 * Write data using nc_put_vars_double() with stride (1, 1, DIM2_STRIDE).
 * data layout is [d0][d1][z] so this maps element data[d0][d1][z] to
 * file position (d0, d1, z*DIM2_STRIDE).
 * This is the slow path reported in issue #1877.
 */
static int
write_strided(const double *data)
{
    int ncid, varid;
    size_t start[3]     = {0, 0, 0};
    size_t count[3]     = {DIM0_LEN, DIM1_LEN, DIM2_SLABS};
    ptrdiff_t stride[3] = {1, 1, DIM2_STRIDE};

    if (create_file(FILE_STRIDED, &ncid, &varid)) ERR;
    if (nc_put_vars_double(ncid, varid, start, count, stride, data)) ERR;
    if (nc_close(ncid)) ERR;
    return 0;
}

/*
 * Write data using a loop of nc_put_vara_double() calls, one per z-slab.
 * data layout is [d0][d1][z], so element (d0, d1, z) is at
 * data[d0*DIM1_LEN*DIM2_SLABS + d1*DIM2_SLABS + z].
 * For each z we need a contiguous (DIM0_LEN x DIM1_LEN) buffer, so we
 * copy the z-slice into a temporary buffer before writing.
 * This is the fast workaround described in the bug report.
 */
static int
write_looped(const double *data)
{
    int ncid, varid;
    int z, d0, d1;
    size_t start[3] = {0, 0, 0};
    size_t count[3] = {DIM0_LEN, DIM1_LEN, 1};
    double *slab = (double *)malloc((size_t)DIM0_LEN * DIM1_LEN * sizeof(double));
    if (!slab) ERR;

    if (create_file(FILE_LOOPED, &ncid, &varid)) { free(slab); ERR; }
    for (z = 0; z < DIM2_SLABS; z++)
    {
        /* Copy z-slice: slab[d0*DIM1_LEN + d1] = data[d0*DIM1_LEN*DIM2_SLABS + d1*DIM2_SLABS + z] */
        for (d0 = 0; d0 < DIM0_LEN; d0++)
            for (d1 = 0; d1 < DIM1_LEN; d1++)
                slab[d0 * DIM1_LEN + d1] =
                    data[(size_t)d0 * DIM1_LEN * DIM2_SLABS + (size_t)d1 * DIM2_SLABS + z];
        start[2] = (size_t)(z * DIM2_STRIDE);
        if (nc_put_vara_double(ncid, varid, start, count, slab))
            { free(slab); ERR; }
    }
    if (nc_close(ncid)) { free(slab); ERR; }
    free(slab);
    return 0;
}

/*
 * Read back a file written with stride (1,1,DIM2_STRIDE) and verify
 * values match the expected data (layout [d0][d1][z]).
 */
static int
verify_file(const char *filename, const double *expected)
{
    int ncid, varid;
    size_t start[3]     = {0, 0, 0};
    size_t count[3]     = {DIM0_LEN, DIM1_LEN, DIM2_SLABS};
    ptrdiff_t stride[3] = {1, 1, DIM2_STRIDE};
    size_t i;
    double *readbuf = (double *)malloc(NVALS * sizeof(double));
    if (!readbuf) ERR;

    if (nc_open(filename, NC_NOWRITE, &ncid)) { free(readbuf); ERR; }
    if (nc_inq_varid(ncid, "data", &varid)) { free(readbuf); ERR; }
    if (nc_get_vars_double(ncid, varid, start, count, stride, readbuf))
        { free(readbuf); ERR; }
    if (nc_close(ncid)) { free(readbuf); ERR; }

    for (i = 0; i < NVALS; i++)
    {
        if (readbuf[i] != expected[i])
        {
            printf("MISMATCH in %s at index %zu: expected %g got %g\n",
                   filename, i, expected[i], readbuf[i]);
            free(readbuf);
            return 1;
        }
    }
    free(readbuf);
    return 0;
}

int
main(void)
{
    double *data;

    printf("Testing strided writes (issue #1877)...\n");

    data = make_data();
    if (!data)
    {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* --- Correctness tests (always run) --- */

    printf("  writing with nc_put_vars_double() stride=%d...\n", DIM2_STRIDE);
    if (write_strided(data)) ERR;
    printf("  verifying strided write...\n");
    if (verify_file(FILE_STRIDED, data)) ERR;
    printf("  strided write: CORRECT\n");

    printf("  writing with looped nc_put_vara_double()...\n");
    if (write_looped(data)) ERR;
    printf("  verifying looped write...\n");
    if (verify_file(FILE_LOOPED, data)) ERR;
    printf("  looped write: CORRECT\n");

    /* --- Performance regression test (requires sys/time.h) --- */

#ifdef HAVE_SYS_TIME_H
    {
        struct timeval t0, t1;
        long long strided_us, looped_us;

        /* Time the strided write. */
        gettimeofday(&t0, NULL);
        if (write_strided(data)) ERR;
        gettimeofday(&t1, NULL);
        strided_us = (long long)(t1.tv_sec - t0.tv_sec) * 1000000LL
                     + (t1.tv_usec - t0.tv_usec);

        /* Time the looped write. */
        gettimeofday(&t0, NULL);
        if (write_looped(data)) ERR;
        gettimeofday(&t1, NULL);
        looped_us = (long long)(t1.tv_sec - t0.tv_sec) * 1000000LL
                    + (t1.tv_usec - t0.tv_usec);

        printf("  strided write: %lld us\n", strided_us);
        printf("  looped  write: %lld us\n", looped_us);

        if (looped_us > 0)
        {
            long long ratio = strided_us / looped_us;
            printf("  ratio (strided/looped): %lld\n", ratio);

            /*
             * Before the fix, strided writes were 500x-1000x slower than
             * looped writes (see issue #1877 timings). After the fix,
             * NC4_put_vars() uses a single HDF5 hyperslab write, so the
             * ratio should be well under 10x. We allow up to 50x to
             * accommodate I/O variability.
             */
            if (ratio > 50)
            {
                printf("FAIL: strided write is %lldx slower than looped "
                       "write (issue #1877 not fixed)\n", ratio);
                free(data);
                return 1;
            }
        }
        printf("  performance regression check: PASSED\n");
    }
#else
    printf("  (skipping performance timing: no sys/time.h)\n");
#endif /* HAVE_SYS_TIME_H */

    free(data);
    nc_delete(FILE_STRIDED);
    nc_delete(FILE_LOOPED);

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
