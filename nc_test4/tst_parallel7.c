/* Copyright 2022, UCAR/Unidata See COPYRIGHT file for copying and
 * redistribution conditions.
 *
 * This parallel I/O test checks for a reported bug of netCDF hanging.
 *
 * See https://github.com/Unidata/netcdf-c/issues/1831.
 *
 * dqwu, Ed Hartnett, 5/2/22
 */

#include <nc_tests.h>
#include "err_macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>

#define FILENAME "tst_parallel7.nc"


#define NDIM 3
#define X_DIM_LEN 48000
#define Y_DIM_LEN 72
#define NVARS 80

int main(int argc, char** argv)
{
    int rank, nprocs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!rank)
        printf("\n*** Testing parallel I/O.\n");

    if (!rank)
        printf("*** testing a reported hanging problem...");
    /* See https://github.com/Unidata/netcdf-c/issues/1831 */
    {
	int dimid[NDIM];
	int elements_per_pe;
	int ncid;
	int varid;
	float *buffer;
	char varname[NC_MAX_NAME + 1];
	size_t start[NDIM];
	size_t count[NDIM];
	int i;

	if (nprocs != 16)
	    ERR;

	if (nc_create_par(FILENAME, NC_CLOBBER | NC_MPIIO | NC_NETCDF4,
			  MPI_COMM_WORLD, MPI_INFO_NULL, &ncid)) ERR;
	if (nc_def_dim(ncid, "time", NC_UNLIMITED, &dimid[0])) ERR;
	if (nc_def_dim(ncid, "x", X_DIM_LEN, &dimid[1])) ERR;
	if (nc_def_dim(ncid, "y", Y_DIM_LEN, &dimid[2])) ERR;

	for (i = 0; i < NVARS; i++)
	{
	    sprintf(varname, "var_%d", i);
	    if (nc_def_var(ncid, varname, NC_FLOAT, NDIM, dimid,
			   &varid)) ERR;
	}

	if (nc_enddef(ncid)) ERR;

	elements_per_pe = X_DIM_LEN * Y_DIM_LEN / 15;

	if (rank < 15)
	{
	    start[0] = 0;
	    count[0] = 1;

	    start[1] = rank * (X_DIM_LEN / 15);
	    count[1] = X_DIM_LEN / 15;

	    start[2] = 0;
	    count[2] = 72;

	    buffer = malloc(elements_per_pe * sizeof(float));
	    for (i = 0; i < elements_per_pe; i++)
		buffer[i] = rank;
	}
	else
	{
	    start[0] = 0;
	    count[0] = 0;

	    start[1] = 0;
	    count[1] = 0;

	    start[2] = 0;
	    count[2] = 0;

	    buffer = NULL;
	}

	for (varid = 0; varid < NVARS; varid++)
	{
	    if (!rank)
	    { 
		printf("Write varid %d start\n", varid);
		fflush(stdout);
	    }

	    if (nc_var_par_access(ncid, varid, NC_COLLECTIVE)) ERR;
	    if (nc_put_vara_float(ncid, varid, start, count, buffer)) ERR;

	    if (!rank)
	    {
		printf("Write varid %d end\n", varid);
		fflush(stdout);
	    }
	}

	if (nc_close(ncid)) ERR;
	free(buffer);
    }

    if (!rank)
        SUMMARIZE_ERR;

    MPI_Finalize();

    if (!rank)
        FINAL_RESULTS;

    return 0;
}
