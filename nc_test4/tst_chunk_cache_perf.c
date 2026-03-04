/*! \file tst_chunk_cache_perf.c

Diagnostic test for GitHub issue #2721: nc_get_vars incredibly slow on Windows.

https://github.com/Unidata/netcdf-c/issues/2721

This test prints chunk cache compile-time defaults, runtime settings, and
timing results for strided vs non-strided reads of a compressed chunked
variable. It is designed to be run on both Linux and Windows via CI so that
the output can be compared to diagnose whether the Windows slowdown is caused
by a smaller-than-expected chunk cache or by zlib decompression speed.

The test always passes (returns 0) — it is purely diagnostic.

Variable dimensions are scaled down from the bug report (2000x512x512) to
keep CI fast, while preserving the same chunk/dim ratio and access pattern.

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

/* Scaled-down dimensions matching the issue #2721 ratio. */
#define D0      200   /* was 2000 */
#define D1      128   /* was 512  */
#define D2      128   /* was 512  */

/* Chunk sizes matching the bug report exactly. */
#define C0      20
#define C1      10
#define C2      10

/* Deflate level matching the bug report. */
#define DEFLATE_LEVEL 6

/* Strided read parameters: stride 2 on all dims, read half the data. */
#define S_COUNT0  (D0 / 2)
#define S_COUNT1  (D1 / 2)
#define S_COUNT2  (D2 / 2)
#define S_STRIDE  2

#define NVALS_WRITE  ((size_t)D0 * D1 * D2)
#define NVALS_READ   ((size_t)S_COUNT0 * S_COUNT1 * S_COUNT2)

#define FILE_NAME "tst_chunk_cache_perf.nc"

int
main(void)
{
    int ncid, varid, dimids[3];
    size_t chunks[3] = {C0, C1, C2};
    double *wdata = NULL;
    double *rdata = NULL;
    size_t i;
    int retval = 0;

    /* --- Compile-time settings --- */
    printf("=== Chunk Cache Diagnostic (issue #2721) ===\n");
    printf("\n--- Compile-time defaults ---\n");
#ifdef DEFAULT_CHUNK_CACHE_SIZE
    printf("  DEFAULT_CHUNK_CACHE_SIZE    = %lu bytes (%.1f MB)\n",
           (unsigned long)DEFAULT_CHUNK_CACHE_SIZE,
           (double)DEFAULT_CHUNK_CACHE_SIZE / (1024.0 * 1024.0));
#else
    printf("  DEFAULT_CHUNK_CACHE_SIZE    = (not defined in config.h)\n");
#endif
#ifdef DEFAULT_CHUNKS_IN_CACHE
    printf("  DEFAULT_CHUNKS_IN_CACHE     = %lu\n",
           (unsigned long)DEFAULT_CHUNKS_IN_CACHE);
#else
    printf("  DEFAULT_CHUNKS_IN_CACHE     = (not defined in config.h)\n");
#endif
#ifdef DEFAULT_CHUNK_CACHE_PREEMPTION
    printf("  DEFAULT_CHUNK_CACHE_PREEMPTION = %.2f\n",
           (double)DEFAULT_CHUNK_CACHE_PREEMPTION);
#else
    printf("  DEFAULT_CHUNK_CACHE_PREEMPTION = (not defined in config.h)\n");
#endif
#ifdef CHUNK_CACHE_SIZE
    printf("  CHUNK_CACHE_SIZE            = %lu bytes (%.1f MB)\n",
           (unsigned long)CHUNK_CACHE_SIZE,
           (double)CHUNK_CACHE_SIZE / (1024.0 * 1024.0));
#else
    printf("  CHUNK_CACHE_SIZE            = (not defined in config.h)\n");
#endif
#ifdef CHUNK_CACHE_NELEMS
    printf("  CHUNK_CACHE_NELEMS          = %lu\n",
           (unsigned long)CHUNK_CACHE_NELEMS);
#else
    printf("  CHUNK_CACHE_NELEMS          = (not defined in config.h)\n");
#endif

    /* --- Runtime global chunk cache settings --- */
    {
        size_t cache_size, cache_nelems;
        float cache_preemption;
        printf("\n--- Runtime global chunk cache (nc_get_chunk_cache) ---\n");
        if (nc_get_chunk_cache(&cache_size, &cache_nelems, &cache_preemption))
        {
            printf("  ERROR calling nc_get_chunk_cache\n");
        }
        else
        {
            printf("  size       = %lu bytes (%.1f MB)\n",
                   (unsigned long)cache_size,
                   (double)cache_size / (1024.0 * 1024.0));
            printf("  nelems     = %lu\n", (unsigned long)cache_nelems);
            printf("  preemption = %.2f\n", (double)cache_preemption);
        }
    }

    /* --- Variable / file parameters --- */
    printf("\n--- Test variable parameters ---\n");
    printf("  dims:       %d x %d x %d\n", D0, D1, D2);
    printf("  chunks:     %d x %d x %d\n", C0, C1, C2);
    printf("  deflate:    level %d, no shuffle\n", DEFLATE_LEVEL);
    printf("  type:       NC_DOUBLE\n");
    printf("  write size: %.1f MB\n",
           (double)NVALS_WRITE * sizeof(double) / (1024.0 * 1024.0));

    /* --- Allocate buffers --- */
    wdata = (double *)malloc(NVALS_WRITE * sizeof(double));
    rdata = (double *)malloc(NVALS_READ  * sizeof(double));
    if (!wdata || !rdata)
    {
        printf("ERROR: malloc failed\n");
        free(wdata); free(rdata);
        return 1;
    }
    for (i = 0; i < NVALS_WRITE; i++)
        wdata[i] = (double)i;

    /* --- Create and write the file --- */
    printf("\n--- Writing test file ---\n");
    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, "d0", D0, &dimids[0])) ERR;
    if (nc_def_dim(ncid, "d1", D1, &dimids[1])) ERR;
    if (nc_def_dim(ncid, "d2", D2, &dimids[2])) ERR;
    if (nc_def_var(ncid, "data", NC_DOUBLE, 3, dimids, &varid)) ERR;
    if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
    if (nc_def_var_deflate(ncid, varid, 0, 1, DEFLATE_LEVEL)) ERR;
    if (nc_put_var_double(ncid, varid, wdata)) ERR;
    if (nc_close(ncid)) ERR;
    printf("  written: %s\n", FILE_NAME);

#ifdef HAVE_SYS_TIME_H
    /* --- Time strided read: stride (2,2,2) on all dims --- */
    {
        struct timeval t0, t1;
        long long strided_us, full_us;
        size_t start[3]     = {0, 0, 0};
        size_t count[3]     = {S_COUNT0, S_COUNT1, S_COUNT2};
        ptrdiff_t stride[3] = {S_STRIDE, S_STRIDE, S_STRIDE};
        size_t cache_size, cache_nelems;
        float cache_preemption;

        printf("\n--- Strided read: nc_get_vars, stride (%d,%d,%d) ---\n",
               S_STRIDE, S_STRIDE, S_STRIDE);
        printf("  count: %d x %d x %d = %.1f MB output\n",
               S_COUNT0, S_COUNT1, S_COUNT2,
               (double)NVALS_READ * sizeof(double) / (1024.0 * 1024.0));

        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, "data", &varid)) ERR;

        /* Print per-variable cache after open (shows auto-tune result). */
        if (nc_get_var_chunk_cache(ncid, varid, &cache_size, &cache_nelems,
                                   &cache_preemption) == NC_NOERR)
        {
            printf("  per-var cache after open: %lu bytes (%.1f MB), "
                   "%lu slots, preemption=%.2f\n",
                   (unsigned long)cache_size,
                   (double)cache_size / (1024.0 * 1024.0),
                   (unsigned long)cache_nelems,
                   (double)cache_preemption);
        }

        memset(rdata, 0, NVALS_READ * sizeof(double));
        gettimeofday(&t0, NULL);
        if (nc_get_vars_double(ncid, varid, start, count, stride, rdata)) ERR;
        gettimeofday(&t1, NULL);
        strided_us = (long long)(t1.tv_sec - t0.tv_sec) * 1000000LL
                     + (t1.tv_usec - t0.tv_usec);
        printf("  elapsed: %lld us (%.2f s)\n", strided_us,
               (double)strided_us / 1e6);

        if (nc_close(ncid)) ERR;

        /* --- Time full read: nc_get_var --- */
        {
            double *fullbuf = (double *)malloc(NVALS_WRITE * sizeof(double));
            if (!fullbuf)
            {
                printf("  WARNING: malloc for full read failed, skipping\n");
                full_us = 0;
            }
            else
            {
                printf("\n--- Full read: nc_get_var ---\n");
                printf("  reading %.1f MB\n",
                       (double)NVALS_WRITE * sizeof(double) / (1024.0 * 1024.0));

                if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
                if (nc_inq_varid(ncid, "data", &varid)) ERR;

                memset(fullbuf, 0, NVALS_WRITE * sizeof(double));
                gettimeofday(&t0, NULL);
                if (nc_get_var_double(ncid, varid, fullbuf)) ERR;
                gettimeofday(&t1, NULL);
                full_us = (long long)(t1.tv_sec - t0.tv_sec) * 1000000LL
                          + (t1.tv_usec - t0.tv_usec);
                printf("  elapsed: %lld us (%.2f s)\n", full_us,
                       (double)full_us / 1e6);

                if (nc_close(ncid)) ERR;
                free(fullbuf);
            }
        }

        /* --- Summary --- */
        printf("\n--- Summary ---\n");
        printf("  strided read time : %lld us\n", strided_us);
        printf("  full read time    : %lld us\n", full_us);
        if (full_us > 0)
            printf("  ratio (strided/full): %.1fx\n",
                   (double)strided_us / (double)full_us);
        printf("  (issue #2721: on Windows this ratio was ~5x worse than Linux)\n");
    }
#else
    printf("\n--- Timing skipped (no sys/time.h on this platform) ---\n");
    printf("  (chunk cache settings above are the key diagnostic output)\n");
#endif /* HAVE_SYS_TIME_H */

    free(wdata);
    free(rdata);
    nc_delete(FILE_NAME);

    printf("\n=== Done (always passes - diagnostic only) ===\n");
    return retval;
}
