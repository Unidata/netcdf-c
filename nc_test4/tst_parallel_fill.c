/*
   Copyright 2020, UCAR/Unidata See COPYRIGHT file for copying and
   redistribution conditions.

   This program tests netcdf-4 parallel I/O using fill values.

   Ed Hartnett, 8/20/20
*/

#include <nc_tests.h>
#include "err_macros.h"
#include <mpi.h>

#define FILE_NAME "tst_parallel_fill.nc"
#define DIM_NAME "Elephants"
#define DIM_LEN 4
#define VAR_NAME "Hannible"
#define NDIM1 1

int
main(int argc, char **argv)
{
    /* MPI stuff. */
    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Netcdf-4 stuff. */
    int ncid, dimid, varid;

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    /* Create a test data file. */
    if (nc_create_par(FILE_NAME, NC_NETCDF4, comm, info, &ncid)) ERR;
    if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
    if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIM1, &dimid, &varid)) ERR;
    if (nc_close(ncid)) ERR;

    /* Open the file and check it. */
    {
	int ndims, nvars, natts, unlimdimid;
	
	if (nc_open_par(FILE_NAME, NC_NOWRITE, comm, info, &ncid)) ERR;
	if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
	if (nvars != 0 || ndims != 1 || natts != 0 || unlimdimid != -1) ERR;
	if (nc_close(ncid)) ERR;
    }

    /* Shut down MPI. */
    MPI_Finalize();

    if (!mpi_rank)
       FINAL_RESULTS;

    return 0;
}
