/*
  Copyright 2019, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This program benchmarks random access to chunked data.

  Ed Hartnett 11/25/19
*/

#include <nc_tests.h>
#include <err_macros.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* #define FILE_NAME "gfs.t00z.sfcf024.nc" */
#define FILE_NAME "tst_bm_rando.nc"
#define MILLION 1000000
#define NDIM3 3

/* These dim lengths taken from the current NOAA GFS surface data. */
/* #define DIM1_LEN 3072 */
/* #define DIM2_LEN 1536 */
#define DIM1_LEN 512
#define DIM2_LEN 512

#define DIM0_NAME "unlimited"
#define DIM1_NAME "x"
#define DIM2_NAME "y"
#define NUM_VAR 10
#define NUM_REC 100
#define NUM_TRY 10000
#define NUM_CACHE_SETTING 3

/* Prototype from tst_utils.c. */
int nc4_timeval_subtract(struct timeval *result, struct timeval *x,
                         struct timeval *y);
int
main(int argc, char **argv)
{
    printf("Benchmarking random access to  file.\n");
    printf("Reading a file randomly...\n");
    {
        int ncid;
        int dimid[NDIM3];
        int varid[NUM_VAR];
        size_t chunksize[NDIM3] = {1, 512, 512};
        char name[NC_MAX_NAME + 1];
        size_t start[NDIM3] = {0, 0, 0};
        size_t count[NDIM3] = {1, DIM1_LEN, DIM2_LEN};
        float *data;
        float *data_in;
        size_t size[NUM_CACHE_SETTING] = {4194304, (4194304), (4194304)};
        /* size_t size[NUM_CACHE_SETTING] = {4194304, (4194304 * 4), (4194304 * 16)}; */
        size_t nelems[NUM_CACHE_SETTING] = {1009, 4133, 16141};
        /* size_t nelems[NUM_CACHE_SETTING] = {1009, 1009, 1009}; */
        float preemption[NUM_CACHE_SETTING] = {0.75f, 0.75f, 0.75f};

        struct timeval start_time, end_time, diff_time;
        float read_us;

        int v, d, t, c;

        /* Set random number seed. */
        srand(time(NULL));

        /* Create the file. */
        if (nc_create(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
        if (nc_def_dim(ncid, DIM0_NAME, NC_UNLIMITED, &dimid[0])) ERR;
        if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimid[1])) ERR;
        if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimid[2])) ERR;
        for (v = 0; v < NUM_VAR; v++)
        {
            sprintf(name, "var_%d", v);
            if (nc_def_var(ncid, name, NC_FLOAT, NDIM3, dimid, &varid[v])) ERR;
            if (nc_def_var_chunking(ncid, v, NC_CHUNKED, chunksize)) ERR;
        }
        if (!(data = malloc(DIM1_LEN * DIM2_LEN * sizeof(float)))) ERR;
        for (d = 0; d < DIM1_LEN * DIM2_LEN; d++)
            data[d] = d + 1.0f/(float)(rand()%1000);
        for (v = 0; v < NUM_VAR; v++)
        {
            for (start[0] = 0; start[0] < NUM_REC; start[0]++)
            {
                if (nc_put_vara_float(ncid, v, start, count, data)) ERR;
            }
        }
        if (nc_close(ncid)) ERR;
        free(data);
        if (!(data_in = malloc(DIM1_LEN * DIM2_LEN * sizeof(float)))) ERR;

        /* nc_set_log_level(3); */
        printf("size\tnelems\tpreemption\tread time(s)\n");
        for (c = 0; c < NUM_CACHE_SETTING; c++)
        {
            char cmd[NC_MAX_NAME * 2 + 20];
            char file_2[NC_MAX_NAME + 1];

            if (c)
            {
                /* Create a copy of file_out. This will defeat any OS
                 * buffering. */
                sprintf(file_2, "tst_copy_%d_%s", c, FILE_NAME);
                sprintf(cmd, "cp %s %s\n", FILE_NAME, file_2);
                system(cmd);
            }
            else
                strcpy(file_2, FILE_NAME);

            /* Start timer. */
            if (gettimeofday(&start_time, NULL)) ERR;

            /* Set cache settings. */
            if (nc_set_chunk_cache(size[c], nelems[c], preemption[c])) ERR;

            /* Open the file. */
            if (nc_open(file_2, NC_NOWRITE, &ncid)) ERR;

            /* Read a random record of a random var. */
            for (t = 0; t < NUM_TRY; t++)
            {
                int var = rand() % NUM_VAR;
                start[0] = rand() % NUM_REC;
                if (nc_get_vara_float(ncid, var, start, count, data_in)) ERR;
            }

            /* Close the file. */
            if (nc_close(ncid)) ERR;

            /* Stop timer. */
            if (gettimeofday(&end_time, NULL)) ERR;
            if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
            read_us = (int)diff_time.tv_sec + (float)diff_time.tv_usec / MILLION ;
            printf("%zu, %ld, %g, %g\n", size[c], nelems[c], preemption[c], read_us);
        }
        /* Free data storage. */
        free(data_in);


    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
