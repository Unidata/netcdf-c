/* Copyright 2024
   University Corporation for Atmospheric Research/Unidata. */

/* This program tests for undesired HDF5 output in parallel
   programs. See https://github.com/Unidata/netcdf-c/issues/3003.

   This is intended to be run on one processor only, but with MPI.
   
   Edward Hartnett, 9/2/24
*/
#include "nc_tests.h"
#include "err_macros.h"
#include "netcdf_par.h"

#define FILE_NAME "tst_warn.nc"

int
main(int argc, char** argv)
{
    int ncid,varid,dimid;
    int retval;

    printf("\n*** Testing for HDF5 extra output\n");

    MPI_Init(&argc,&argv);

    printf("*** Testing create of parallel I/O file...");
    if ((retval = nc_create_par(FILE_NAME, NC_CLOBBER|NC_NETCDF4, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid))) ERR;
    if ((retval = nc_close(ncid))) ERR;

    MPI_Finalize();

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
