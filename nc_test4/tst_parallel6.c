/* Copyright 2022, UCAR/Unidata See COPYRIGHT file for copying and
 * redistribution conditions.
 *
 * This parallel I/O test checks the behavior of nc_inq_dimlen() after
 * parallel I/O writes.
 *
 * This program taken from a PNetCDF issue:
 * https://github.com/Parallel-NetCDF/PnetCDF/issues/72, thanks
 * wkliao!
 *
 * wkliao, Ed Hartnett, 4/11/22
 */

#include <nc_tests.h>
#include "err_macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>

#define FILENAME "tst_parallel6.nc"

int main(int argc, char** argv)
{
    int err = NC_NOERR, rank, nprocs;
    int ncid, varid, dimid;
    size_t start[1], count[1], nrecs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!rank)
        printf("\n*** Testing parallel I/O.\n");

    if (!rank)
        printf("*** testing record lenth with multiple processes writing records...");

    /* nc_set_log_level(4); */
    if (nc_create_par(FILENAME, NC_CLOBBER | NC_NETCDF4, MPI_COMM_WORLD,
		      MPI_INFO_NULL, &ncid)) ERR;

    if (nc_def_dim(ncid, "time", NC_UNLIMITED, &dimid)) ERR;
    if (nc_def_var(ncid, "var", NC_INT, 1, &dimid, &varid)) ERR;
    if (nc_var_par_access(ncid, varid, NC_COLLECTIVE)) ERR;
    if (nc_enddef(ncid)) ERR;

    start[0] = rank;
    count[0] = 1;
    if (nc_put_vara_int(ncid, varid, start, count, &rank)) ERR;
    if (nc_inq_dimlen(ncid, dimid, &nrecs)) ERR;
    if (nc_close(ncid)) ERR;
    /* nc_set_log_level(-1); */

    if (nrecs != nprocs)
    {
	printf("Rank %d error at line %d of file %s:\n",rank,__LINE__,__FILE__);
	printf("\tafter writing start=%zd count=%zd\n", start[0], count[0]);
	printf("\texpecting number of records = %d but got %ld\n",
	       nprocs, nrecs);
	ERR;
    }

    if (!rank)
        SUMMARIZE_ERR;

    MPI_Finalize();

    if (!rank)
        FINAL_RESULTS;

    return 0;
}
