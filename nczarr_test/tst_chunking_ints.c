/* Copyright 2018, University Corporation for Atmospheric Research.
 * See COPYRIGHT file for copying and redistribution conditions.
 */
/**
 * @file
 * Test nc_def_var_chunking_ints with NCZarr.
 *
 * Regression test for GitHub issue
 * https://github.com/Unidata/netcdf-fortran/issues/487
 *
 * nc_def_var_chunking_ints is the Fortran-compatible wrapper that
 * accepts int* chunksizes. Prior to the fix, it either was a no-op
 * stub (NCZarr-only builds) or called HDF5-specific code that did
 * not update NCZarr cache state. This test verifies that chunk
 * data files are written at the correct size.
 *
 * @author Generated from explore_487 investigation
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <netcdf.h>

#define DIM 256
#define CHUNK 128
#define FILE_NAME "file://tst_chunking_ints.zarr#mode=zarr,file"

#define CHECK(expr) do { \
    int _r = (expr); \
    if (_r != NC_NOERR) { \
        fprintf(stderr, "Error at %s:%d: %s\n", __FILE__, __LINE__, nc_strerror(_r)); \
        return 2; \
    } \
} while(0)

static long
get_file_size(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

int
main(void)
{
    int ncid, dimid_x, dimid_y, varid;
    int chunks[2] = {CHUNK, CHUNK};
    float data[DIM][DIM];
    long expected_chunk_size = (long)CHUNK * CHUNK * sizeof(float);
    int nerrs = 0;

    printf("\n*** Testing nc_def_var_chunking_ints with NCZarr.\n");
    printf("*** Expected chunk file size: %ld bytes\n", expected_chunk_size);

    /* Create file with chunked variable */
    CHECK(nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid));
    CHECK(nc_def_dim(ncid, "y", DIM, &dimid_y));
    CHECK(nc_def_dim(ncid, "x", DIM, &dimid_x));
    CHECK(nc_def_var(ncid, "v", NC_FLOAT, 2,
                   (int[]){dimid_y, dimid_x}, &varid));

    /* Use the Fortran-compatible int* wrapper */
    CHECK(nc_def_var_chunking_ints(ncid, varid, NC_CHUNKED, chunks));

    /* Verify chunking was set correctly */
    {
        int contiguous;
        size_t chunks_out[2];
        CHECK(nc_inq_var_chunking(ncid, varid, &contiguous, chunks_out));
        if (contiguous != 0) {
            printf("FAIL: contiguous != 0\n");
            nerrs++;
        }
        if (chunks_out[0] != CHUNK || chunks_out[1] != CHUNK) {
            printf("FAIL: chunks=[%zu,%zu] expected [%d,%d]\n",
                   chunks_out[0], chunks_out[1], CHUNK, CHUNK);
            nerrs++;
        }
    }

    /* Write data */
    {
        int i, j;
        for (i = 0; i < DIM; i++)
            for (j = 0; j < DIM; j++)
                data[i][j] = 42.0f;
    }
    CHECK(nc_enddef(ncid));
    CHECK(nc_put_var_float(ncid, varid, &data[0][0]));
    CHECK(nc_close(ncid));

    /* Verify each chunk file has the correct size */
    {
        int i, j;
        char path[256];
        for (i = 0; i < DIM / CHUNK; i++) {
            for (j = 0; j < DIM / CHUNK; j++) {
                long sz;
                snprintf(path, sizeof(path),
                         "tst_chunking_ints.zarr/v/%d.%d", i, j);
                sz = get_file_size(path);
                if (sz != expected_chunk_size) {
                    printf("FAIL: %s size=%ld, expected=%ld\n",
                           path, sz, expected_chunk_size);
                    nerrs++;
                }
            }
        }
    }

    /* Cleanup */
    {
        (void)system("rm -rf tst_chunking_ints.zarr");
    }

    if (nerrs) {
        printf("FAILED: %d errors\n", nerrs);
        return 2;
    }
    printf("PASSED\n");
    return 0;
}
