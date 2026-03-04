/* This is part of the netCDF package.
   Copyright 2024 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Regression test for GitHub issue #2750:
   nc_put_var_double execution time increases in subsequent runs when a
   variable is written with chunking and compression.

   The original reporter observed monotonically increasing write times
   across repeated open/write/close/delete cycles on Windows, only when
   both chunking and deflate compression were enabled together.

   This test runs NUM_ITERS such cycles and prints the elapsed CPU time
   for each nc_put_var_double call.  It never fails on timing alone; it
   only fails if a netCDF API call returns an error.  The printed output
   lets CI logs show whether the regression is still present.

   Uses clock() from <time.h> for portable timing on all platforms
   including Windows MSVC (which lacks sys/time.h).

   See https://github.com/Unidata/netcdf-c/issues/2750

   Edward Hartnett, 2024
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "netcdf.h"
#include "err_macros.h"

#define FILE_NAME "tst_chunked_deflate_perf.nc"
#define NDIMS 3
#define NX 200
#define NY 64
#define NZ 64
#define CHUNKX 20
#define CHUNKY 10
#define CHUNKZ 10
#define DEFLATE_LEVEL 5
#define NUM_ITERS 10

int
main(int argc, char **argv)
{
    double *data = NULL;
    int i, iter;
    clock_t t_start, t_end;
    double elapsed;

    printf("\n*** Testing issue 2750: write time stability with chunking+deflate.\n");
    printf("    %d x %d x %d doubles, chunks %d x %d x %d, deflate level %d\n",
           NX, NY, NZ, CHUNKX, CHUNKY, CHUNKZ, DEFLATE_LEVEL);
    printf("    Running %d iterations of create/write/close/delete:\n\n", NUM_ITERS);

    data = (double *)malloc(sizeof(double) * NX * NY * NZ);
    if (!data)
    {
        fprintf(stderr, "malloc failed\n");
        return 2;
    }
    for (i = 0; i < NX * NY * NZ; i++)
        data[i] = (double)i;

    printf("    %8s  %12s\n", "iter", "write_sec");

    for (iter = 0; iter < NUM_ITERS; iter++)
    {
        int ncid, varid, dimids[NDIMS];
        int x_dimid, y_dimid, z_dimid;
        size_t chunks[NDIMS] = {CHUNKX, CHUNKY, CHUNKZ};

        if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
        if (nc_def_dim(ncid, "x", NX, &x_dimid)) ERR;
        if (nc_def_dim(ncid, "y", NY, &y_dimid)) ERR;
        if (nc_def_dim(ncid, "z", NZ, &z_dimid)) ERR;
        dimids[0] = x_dimid;
        dimids[1] = y_dimid;
        dimids[2] = z_dimid;
        if (nc_def_var(ncid, "data", NC_DOUBLE, NDIMS, dimids, &varid)) ERR;
        if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
        if (nc_def_var_deflate(ncid, varid, 0, 1, DEFLATE_LEVEL)) ERR;

        t_start = clock();
        if (nc_put_var_double(ncid, varid, data)) ERR;
        t_end = clock();

        elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
        printf("    %8d  %12.4f\n", iter, elapsed);
        fflush(stdout);

        if (nc_close(ncid)) ERR;
        remove(FILE_NAME);
    }

    printf("\n");
    free(data);
    FINAL_RESULTS;
}
