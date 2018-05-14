/* Copyright 2018, UCAR/Unidata See COPYRIGHT file for copying and
 * redistribution conditions.
 *
 * This program tests netcdf-4 parallel I/O. In this test I write data
 * on one task, while writing 0 items on others.
 *
 * Ed Hartnett
 */

#include <nc_tests.h>
#include "err_macros.h"
#include <mpi.h>

#define FILE "tst_parallel5.nc"
#define VAR_NAME "TheIrishRover"
#define DIM_NAME "number_of_masts"
#define MASTS 27
#define NDIMS1 1
#define DIMSIZE 4
#define NUM_PROC 4
#define NUM_SLABS 10
#define NUM_ACCESS_TESTS 2

int
main(int argc, char **argv)
{
   int mpi_size, mpi_rank;
   MPI_Comm comm = MPI_COMM_WORLD;
   MPI_Info info = MPI_INFO_NULL;
   int ncid, v1id, dimid;
   size_t start[NDIMS1] = {0}, count[NDIMS1] = {0};
   int data = MASTS;
   int data_in = TEST_VAL_42;
   int acc;

   /* Initialize MPI. */
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

   /* Require exactly 4 tasks. */
   if (mpi_size != NUM_PROC) ERR;

   if (!mpi_rank)
   {
      printf("\n*** tst_parallel testing collective parallel access.\n");
      printf("*** tst_parallel testing whether we can write 0 elements from some tasks...");
   }

   for (acc = 0; acc < NUM_ACCESS_TESTS; acc++)
   {
      /* Create a parallel netcdf-4 file. */
      /*nc_set_log_level(3);*/
      if (nc_create_par(FILE, NC_NETCDF4|NC_MPIIO, comm, info, &ncid)) ERR;

      /* Create a dimension. */
      if (nc_def_dim(ncid, DIM_NAME, DIMSIZE, &dimid)) ERR;

      /* Create one var. */
      if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIMS1, &dimid, &v1id)) ERR;

      /* Write metadata to file. */
      if (nc_enddef(ncid)) ERR;

      /* Set up slab for this process. */
      if (!mpi_rank)
         count[0] = 1;

      if (nc_var_par_access(ncid, v1id, acc ? NC_COLLECTIVE : NC_INDEPENDENT)) ERR;

      /* Write phoney data. */
      if (nc_put_vara_int(ncid, v1id, start, count, &data)) ERR;

      if (nc_sync(ncid)) ERR;

      /* Read phoney data. */
      if (nc_get_vara_int(ncid, v1id, start, count, &data_in)) ERR;

      /* Task 0 has MASTS, the others have data_in remaining, as
       * initialized, at TEST_VAL_42. */
      if (data_in != (mpi_rank ? TEST_VAL_42 : MASTS)) ERR;

      /* Close the netcdf file. */
      if (nc_close(ncid)) ERR;
   }

   /* Shut down MPI. */
   MPI_Finalize();

   if (!mpi_rank)
   {
      SUMMARIZE_ERR;
      FINAL_RESULTS;
   }
   return 0;
}
