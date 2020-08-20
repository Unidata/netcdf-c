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
#define NUM_TYPES 2
#define MAX_SIZE 4
#define NUM_DF_TESTS 2

int
main(int argc, char **argv)
{
    /* MPI stuff. */
    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    int elements_per_pe;

    /* Netcdf-4 stuff. */
    int ncid, dimid, varid[NUM_TYPES];
    int type[NUM_TYPES] = {NC_BYTE, NC_INT};
    int t, df;

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    if (mpi_size != 1 && mpi_size != 4) ERR;
    elements_per_pe = DIM_LEN / mpi_size;

    /* Run the test with or without default fill value. */
    for (df = 0; df < NUM_DF_TESTS; df++)
    {
	/* Run the test for each type in the list. */
	for (t = 0; t < NUM_TYPES; t++)
	{
	    /* Create a test data file. */
	    if (nc_create_par(FILE_NAME, NC_NETCDF4, comm, info, &ncid)) ERR;
	    if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
	    if (nc_def_var(ncid, VAR_NAME, type[t], NDIM1, &dimid, &varid[t])) ERR;
	    if (nc_close(ncid)) ERR;
	    
	    /* Open the file and check it. */
	    {
		int ndims, nvars, natts, unlimdimid;
		int xtype_in, dimid_in;
		char name_in[NC_MAX_NAME +1];
		signed char dbyte[MAX_SIZE];
		int dint[MAX_SIZE];
		size_t start[NDIM1] = {0}, count[NDIM1];
		int d;
		
		if (nc_open_par(FILE_NAME, NC_NOWRITE, comm, info, &ncid)) ERR;
		if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
		if (nvars != 1 || ndims != 1 || natts != 0 || unlimdimid != -1) ERR;
		if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, &dimid_in, &natts)) ERR;
		if (strcmp(name_in, VAR_NAME) || xtype_in != type[t] || ndims != 1 ||
		    dimid_in != 0 || natts != 0) ERR;
		
		/* Read the data. */
		count[0] = elements_per_pe;
		switch (type[t])
		{
		case NC_BYTE:
		    if (nc_get_vara_schar(ncid, 0, start, count, dbyte)) ERR;
		    for (d = 0; d < elements_per_pe; d++)
			if (dbyte[d] != NC_FILL_BYTE) ERR;
		    break;
		case NC_INT:
		    if (nc_get_vara_int(ncid, 0, start, count, dint)) ERR;
		    for (d = 0; d < elements_per_pe; d++)
			if (dint[d] != NC_FILL_INT) ERR;
		    break;
		}
		if (nc_close(ncid)) ERR;
	    }
	} /* next t */
    } /* next df */
    
    /* Shut down MPI. */
    MPI_Finalize();

    if (!mpi_rank)
       FINAL_RESULTS;

    return 0;
}
