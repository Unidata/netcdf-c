/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test the effect of changing the chunk cache size and nelems on
   read performance. See https://github.com/Unidata/netcdf-c/issues/1757.

   Ed Hartnett
*/

#include <nc_tests.h>
#include "err_macros.h"
#include <time.h>

#define FILE_NAME "tst_chunk_cache.nc"
#define NDIM2 2
#define DIM_X 5000
#define DIM_Y 3000
#define CHUNK_X 500
#define CHUNK_Y 300
#define SMALL_CHUNK_X 50
#define SMALL_CHUNK_Y 30
#define DIM_NAME_X "x"
#define DIM_NAME_Y "y"
#define VAR_NAME "data"
#define VAR_NAME_2 "data2"
#define FILE_NAME_2 "tst_chunk_cache2.nc"
#define NUM_NELEMS_TESTS 6

/* Fill a 2D array with test data. */
static void
fill_data(float *data, int nx, int ny)
{
    int i, j;
    for (i = 0; i < nx; i++)
        for (j = 0; j < ny; j++)
            data[i * ny + j] = (float)(i * ny + j);
}

/* Verify a 2D array against expected test data. Return 0 on success. */
static int
check_data(float *data, int nx, int ny)
{
    int i, j;
    for (i = 0; i < nx; i++)
        for (j = 0; j < ny; j++)
            if (data[i * ny + j] != (float)(i * ny + j))
                return 1;
    return 0;
}

/* Create the test file with a chunked 2D variable and write data. */
static int
create_test_file(const char *file_name, float *data)
{
    int ncid, dimids[NDIM2], varid;
    size_t chunks[NDIM2] = {CHUNK_X, CHUNK_Y};

    if (nc_create(file_name, NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, DIM_NAME_X, DIM_X, &dimids[0])) ERR;
    if (nc_def_dim(ncid, DIM_NAME_Y, DIM_Y, &dimids[1])) ERR;
    if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIM2, dimids, &varid)) ERR;
    if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
    if (nc_put_var_float(ncid, varid, data)) ERR;
    if (nc_close(ncid)) ERR;
    return 0;
}

/* Read the full variable with given cache settings. Print elapsed
 * time and store it in *elapsed_out. Return 0 on success. */
static int
read_with_cache(const char *file_name, float *data_in, size_t cache_size,
                size_t cache_nelems, float cache_preemption,
                const char *label, double *elapsed_out)
{
    int ncid, varid;
    clock_t start, end;
    double elapsed;

    if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
    if (nc_set_var_chunk_cache(ncid, varid, cache_size, cache_nelems,
                               cache_preemption)) ERR;

    start = clock();
    if (nc_get_var_float(ncid, varid, data_in)) ERR;
    end = clock();

    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("    %s: %.3f seconds\n", label, elapsed);
    if (elapsed_out)
        *elapsed_out = elapsed;

    if (nc_close(ncid)) ERR;
    return 0;
}

int
main(int argc, char **argv)
{
    float *data = NULL;
    float *data_in = NULL;

    /* Timing results for summary table. */
    double t_fullread_small = 0, t_fullread_large = 0;
    double t_nelems1 = 0, t_nelems1009 = 0;
    double t_row_small = 0, t_col_small = 0;
    double t_row_large = 0, t_col_large = 0;
    double t_nelems_stress[NUM_NELEMS_TESTS];
    size_t nelems_values[NUM_NELEMS_TESTS] = {1, 10, 100, 1000, 4133, 10007};
    int ne;

    printf("\n*** Testing chunk cache effects on performance.\n");

    /* Allocate data buffers used by multiple tests. */
    data = (float *)malloc(sizeof(float) * DIM_X * DIM_Y);
    data_in = (float *)malloc(sizeof(float) * DIM_X * DIM_Y);
    if (!data || !data_in)
    {
        printf("Memory allocation failed\n");
        return 2;
    }
    fill_data(data, DIM_X, DIM_Y);

    printf("**** testing cache settings round-trip...");
    {
#define RT_CACHE_SIZE 32000000
#define RT_CACHE_NELEMS 2003
#define RT_CACHE_PREEMPTION 0.80f
        int ncid, dimids[NDIM2], varid;
        size_t chunks[NDIM2] = {CHUNK_X, CHUNK_Y};
        size_t cache_size_in, cache_nelems_in;
        float cache_preemption_in;

        if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
        if (nc_def_dim(ncid, DIM_NAME_X, DIM_X, &dimids[0])) ERR;
        if (nc_def_dim(ncid, DIM_NAME_Y, DIM_Y, &dimids[1])) ERR;
        if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIM2, dimids, &varid)) ERR;
        if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;

        /* Set cache and verify round-trip. */
        if (nc_set_var_chunk_cache(ncid, varid, RT_CACHE_SIZE,
                                   RT_CACHE_NELEMS,
                                   RT_CACHE_PREEMPTION)) ERR;
        if (nc_get_var_chunk_cache(ncid, varid, &cache_size_in,
                                   &cache_nelems_in,
                                   &cache_preemption_in)) ERR;
        if (cache_size_in != RT_CACHE_SIZE) ERR;
        if (cache_nelems_in != RT_CACHE_NELEMS) ERR;
        if (cache_preemption_in != RT_CACHE_PREEMPTION) ERR;

        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("**** testing effect of cache size on read (%dx%d floats, %dx%d chunks)...\n",
           DIM_X, DIM_Y, CHUNK_X, CHUNK_Y);
    {
        /* Create the test file. */
        if (create_test_file(FILE_NAME, data)) ERR;

        /* Read with a tiny cache. */
        memset(data_in, 0, sizeof(float) * DIM_X * DIM_Y);
        if (read_with_cache(FILE_NAME, data_in, 1, 1, 0.75f,
                            "small cache (size=1, nelems=1)",
                            &t_fullread_small)) ERR;
        if (check_data(data_in, DIM_X, DIM_Y)) ERR;

        /* Read with a large cache. */
        memset(data_in, 0, sizeof(float) * DIM_X * DIM_Y);
        if (read_with_cache(FILE_NAME, data_in, THIRTY_TWO_MEG, 1009,
                            0.75f,
                            "large cache (size=32MB, nelems=1009)",
                            &t_fullread_large)) ERR;
        if (check_data(data_in, DIM_X, DIM_Y)) ERR;
    }
    SUMMARIZE_ERR;

    printf("**** testing effect of nelems on read...\n");
    {
        /* File already exists from previous test. */

        /* Read with large size but nelems=1 (forces hash collisions). */
        memset(data_in, 0, sizeof(float) * DIM_X * DIM_Y);
        if (read_with_cache(FILE_NAME, data_in, THIRTY_TWO_MEG, 1,
                            0.75f,
                            "large cache, nelems=1 (hash collisions)",
                            &t_nelems1)) ERR;
        if (check_data(data_in, DIM_X, DIM_Y)) ERR;

        /* Read with large size and nelems=1009 (good hash spread). */
        memset(data_in, 0, sizeof(float) * DIM_X * DIM_Y);
        if (read_with_cache(FILE_NAME, data_in, THIRTY_TWO_MEG, 1009,
                            0.75f,
                            "large cache, nelems=1009 (good hash)",
                            &t_nelems1009)) ERR;
        if (check_data(data_in, DIM_X, DIM_Y)) ERR;
    }
    SUMMARIZE_ERR;

    printf("**** testing global vs per-variable cache...");
    {
#define GV_GLOBAL_SIZE 16000000
#define GV_GLOBAL_NELEMS 503
#define GV_GLOBAL_PREEMPTION 0.50f
#define GV_VAR_SIZE 64000000
#define GV_VAR_NELEMS 2003
#define GV_VAR_PREEMPTION 0.25f
        int ncid, dimids[NDIM2], varid1, varid2;
        size_t chunks[NDIM2] = {CHUNK_X, CHUNK_Y};
        size_t cache_size_in, cache_nelems_in;
        float cache_preemption_in;

        /* Set global chunk cache. */
        if (nc_set_chunk_cache(GV_GLOBAL_SIZE, GV_GLOBAL_NELEMS,
                               GV_GLOBAL_PREEMPTION)) ERR;

        /* Create file with two variables. */
        if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
        if (nc_def_dim(ncid, DIM_NAME_X, DIM_X, &dimids[0])) ERR;
        if (nc_def_dim(ncid, DIM_NAME_Y, DIM_Y, &dimids[1])) ERR;
        if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIM2, dimids,
                       &varid1)) ERR;
        if (nc_def_var_chunking(ncid, varid1, NC_CHUNKED, chunks)) ERR;
        if (nc_def_var(ncid, VAR_NAME_2, NC_FLOAT, NDIM2, dimids,
                       &varid2)) ERR;
        if (nc_def_var_chunking(ncid, varid2, NC_CHUNKED, chunks)) ERR;

        /* Override cache for var1 only. */
        if (nc_set_var_chunk_cache(ncid, varid1, GV_VAR_SIZE,
                                   GV_VAR_NELEMS,
                                   GV_VAR_PREEMPTION)) ERR;

        /* Check var1 has overridden settings. */
        if (nc_get_var_chunk_cache(ncid, varid1, &cache_size_in,
                                   &cache_nelems_in,
                                   &cache_preemption_in)) ERR;
        if (cache_size_in != GV_VAR_SIZE) ERR;
        if (cache_nelems_in != GV_VAR_NELEMS) ERR;
        if (cache_preemption_in != GV_VAR_PREEMPTION) ERR;

        /* Check var2 still has the global cache settings (not overridden). */
        if (nc_get_var_chunk_cache(ncid, varid2, &cache_size_in,
                                   &cache_nelems_in,
                                   &cache_preemption_in)) ERR;
        if (cache_size_in != GV_GLOBAL_SIZE) ERR;
        if (cache_nelems_in != GV_GLOBAL_NELEMS) ERR;
        if (cache_preemption_in != GV_GLOBAL_PREEMPTION) ERR;

        if (nc_close(ncid)) ERR;

        /* Restore default global cache. */
        if (nc_set_chunk_cache(CHUNK_CACHE_SIZE, CHUNK_CACHE_NELEMS,
                               CHUNK_CACHE_PREEMPTION)) ERR;
    }
    SUMMARIZE_ERR;

    printf("**** testing cache with different access patterns (%dx%d floats, %dx%d chunks)...\n",
           DIM_X, DIM_Y, CHUNK_X, CHUNK_Y);
    {
        int ncid, varid;
        size_t start[NDIM2], count[NDIM2];
        float *row_buf = NULL;
        float *col_buf = NULL;
        clock_t t_start, t_end;
        double elapsed;
        size_t i, j;

        /* Create the test file. */
        if (create_test_file(FILE_NAME, data)) ERR;

        row_buf = (float *)malloc(sizeof(float) * DIM_Y);
        col_buf = (float *)malloc(sizeof(float) * DIM_X);
        if (!row_buf || !col_buf)
        {
            printf("Memory allocation failed\n");
            return 2;
        }

        /* Row-by-row read (chunk-aligned) with small cache. */
        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
        if (nc_set_var_chunk_cache(ncid, varid, 1, 1, 0.75f)) ERR;

        t_start = clock();
        for (i = 0; i < DIM_X; i++)
        {
            start[0] = i;
            start[1] = 0;
            count[0] = 1;
            count[1] = DIM_Y;
            if (nc_get_vara_float(ncid, varid, start, count, row_buf)) ERR;
            for (j = 0; j < DIM_Y; j++)
                if (row_buf[j] != (float)(i * DIM_Y + j)) ERR;
        }
        t_end = clock();
        elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
        t_row_small = elapsed;
        printf("    row-by-row read, small cache: %.3f seconds\n", elapsed);
        if (nc_close(ncid)) ERR;

        /* Column-by-column read (cross-chunk) with small cache. */
        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
        if (nc_set_var_chunk_cache(ncid, varid, 1, 1, 0.75f)) ERR;

        t_start = clock();
        for (j = 0; j < DIM_Y; j++)
        {
            start[0] = 0;
            start[1] = j;
            count[0] = DIM_X;
            count[1] = 1;
            if (nc_get_vara_float(ncid, varid, start, count, col_buf)) ERR;
            for (i = 0; i < DIM_X; i++)
                if (col_buf[i] != (float)(i * DIM_Y + j)) ERR;
        }
        t_end = clock();
        elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
        t_col_small = elapsed;
        printf("    column-by-column read, small cache: %.3f seconds\n",
               elapsed);
        if (nc_close(ncid)) ERR;

        /* Row-by-row read with large cache. */
        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
        if (nc_set_var_chunk_cache(ncid, varid, THIRTY_TWO_MEG, 1009,
                                   0.75f)) ERR;

        t_start = clock();
        for (i = 0; i < DIM_X; i++)
        {
            start[0] = i;
            start[1] = 0;
            count[0] = 1;
            count[1] = DIM_Y;
            if (nc_get_vara_float(ncid, varid, start, count, row_buf)) ERR;
            for (j = 0; j < DIM_Y; j++)
                if (row_buf[j] != (float)(i * DIM_Y + j)) ERR;
        }
        t_end = clock();
        elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
        t_row_large = elapsed;
        printf("    row-by-row read, large cache: %.3f seconds\n", elapsed);
        if (nc_close(ncid)) ERR;

        /* Column-by-column read with large cache. */
        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
        if (nc_set_var_chunk_cache(ncid, varid, THIRTY_TWO_MEG, 1009,
                                   0.75f)) ERR;

        t_start = clock();
        for (j = 0; j < DIM_Y; j++)
        {
            start[0] = 0;
            start[1] = j;
            count[0] = DIM_X;
            count[1] = 1;
            if (nc_get_vara_float(ncid, varid, start, count, col_buf)) ERR;
            for (i = 0; i < DIM_X; i++)
                if (col_buf[i] != (float)(i * DIM_Y + j)) ERR;
        }
        t_end = clock();
        elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
        t_col_large = elapsed;
        printf("    column-by-column read, large cache: %.3f seconds\n",
               elapsed);
        if (nc_close(ncid)) ERR;

        free(row_buf);
        free(col_buf);
    }
    SUMMARIZE_ERR;

    printf("**** testing nelems effect with many chunks (%dx%d floats, %dx%d chunks = %d chunks)...\n",
           DIM_X, DIM_Y, SMALL_CHUNK_X, SMALL_CHUNK_Y,
           (DIM_X / SMALL_CHUNK_X) * (DIM_Y / SMALL_CHUNK_Y));
    {
        int ncid, dimids[NDIM2], varid;
        size_t chunks[NDIM2] = {SMALL_CHUNK_X, SMALL_CHUNK_Y};
        size_t start[NDIM2], count[NDIM2];
        float *col_buf = NULL;
        clock_t t_start, t_end;
        double elapsed;
        size_t i, j;

        /* Create file with small chunks. */
        if (nc_create(FILE_NAME_2, NC_NETCDF4, &ncid)) ERR;
        if (nc_def_dim(ncid, DIM_NAME_X, DIM_X, &dimids[0])) ERR;
        if (nc_def_dim(ncid, DIM_NAME_Y, DIM_Y, &dimids[1])) ERR;
        if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIM2, dimids, &varid)) ERR;
        if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
        if (nc_put_var_float(ncid, varid, data)) ERR;
        if (nc_close(ncid)) ERR;

        col_buf = (float *)malloc(sizeof(float) * DIM_X);
        if (!col_buf)
        {
            printf("Memory allocation failed\n");
            return 2;
        }

        /* Test column-by-column read with varying nelems.
         * Cache size is large (64MB) so all chunks fit;
         * only the hash table size (nelems) varies. */
        for (ne = 0; ne < NUM_NELEMS_TESTS; ne++)
        {
            if (nc_open(FILE_NAME_2, NC_NOWRITE, &ncid)) ERR;
            if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
            if (nc_set_var_chunk_cache(ncid, varid, SIXTY_FOUR_MEG,
                                       nelems_values[ne], 0.75f)) ERR;

            t_start = clock();
            for (j = 0; j < DIM_Y; j++)
            {
                start[0] = 0;
                start[1] = j;
                count[0] = DIM_X;
                count[1] = 1;
                if (nc_get_vara_float(ncid, varid, start, count,
                                     col_buf)) ERR;
                for (i = 0; i < DIM_X; i++)
                    if (col_buf[i] != (float)(i * DIM_Y + j)) ERR;
            }
            t_end = clock();
            elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
            t_nelems_stress[ne] = elapsed;
            printf("    nelems=%5zu: %.3f seconds\n",
                   nelems_values[ne], elapsed);
            if (nc_close(ncid)) ERR;
        }

        free(col_buf);
    }
    SUMMARIZE_ERR;

    free(data);
    free(data_in);

    /* Print summary table. */
    printf("\n    %-40s %10s\n", "Test", "Time (s)");
    printf("    %-40s %10s\n",
           "----------------------------------------",
           "----------");
    printf("    %-40s %10.3f\n", "Full read, small cache", t_fullread_small);
    printf("    %-40s %10.3f\n", "Full read, large cache", t_fullread_large);
    printf("    %-40s %10.3f\n", "Full read, nelems=1", t_nelems1);
    printf("    %-40s %10.3f\n", "Full read, nelems=1009", t_nelems1009);
    printf("    %-40s %10.3f\n", "Row-by-row, small cache", t_row_small);
    printf("    %-40s %10.3f\n", "Row-by-row, large cache", t_row_large);
    printf("    %-40s %10.3f\n", "Column-by-column, small cache", t_col_small);
    printf("    %-40s %10.3f\n", "Column-by-column, large cache", t_col_large);
    printf("    %-40s %10s\n",
           "----------------------------------------",
           "----------");
    for (ne = 0; ne < NUM_NELEMS_TESTS; ne++)
    {
        char label[64];
        snprintf(label, sizeof(label),
                 "10K chunks, col read, nelems=%zu",
                 nelems_values[ne]);
        printf("    %-40s %10.3f\n", label, t_nelems_stress[ne]);
    }
    printf("\n");

    FINAL_RESULTS;
}
