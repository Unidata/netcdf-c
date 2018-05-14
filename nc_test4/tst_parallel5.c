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
#define NDIMS 3
#define DIMSIZE 4
#define NUM_PROC 4
#define NUM_SLABS 10

int
main(int argc, char **argv)
{
   int mpi_size, mpi_rank;
   MPI_Comm comm = MPI_COMM_WORLD;
   MPI_Info info = MPI_INFO_NULL;
   int ncid, v1id, dimids[NDIMS];
   size_t start[NDIMS], count[NDIMS];
   int data = TEST_VAL_42;
   int i, res;
   int slab_data[DIMSIZE * DIMSIZE / 4]; /* one slab */
   char file_name[NC_MAX_NAME + 1];

   /* Initialize MPI. */
   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

   /* Require exactly 4 tasks. */
   if (mpi_size != NUM_PROC) ERR;

   if (!mpi_rank)
   {
      printf("\n*** tst_parallel testing collective parallel access.\n");
      printf("*** tst_parallel testing whether we can write 0 elements from some tasks...");
   }

   /* Create a parallel netcdf-4 file. */
   /*nc_set_log_level(3);*/
   sprintf(file_name, "%s/%s", TEMP_LARGE, FILE);
   if (nc_create_par(file_name, NC_NETCDF4|NC_MPIIO, comm, info, &ncid)) ERR;

   /* Create a dimension. */
   if (nc_def_dim(ncid, "d1", DIMSIZE, dimids)) ERR;
   if (nc_def_dim(ncid, "d2", DIMSIZE, &dimids[1])) ERR;
   if (nc_def_dim(ncid, "d3", NUM_SLABS, &dimids[2])) ERR;

   /* Create one var. */
   if (nc_def_var(ncid, "v1", NC_INT, NDIMS, dimids, &v1id)) ERR;

   /* Write metadata to file. */
   if (nc_enddef(ncid)) ERR;

/*    /\* Set up slab for this process. *\/ */
/*    start[0] = mpi_rank * DIMSIZE/mpi_size; */
/*    start[1] = 0; */
/*    count[0] = DIMSIZE/mpi_size; */
/*    count[1] = DIMSIZE; */
/*    count[2] = 1; */
/*    /\*printf("mpi_rank=%d start[0]=%d start[1]=%d count[0]=%d count[1]=%d\n", */
/*      mpi_rank, start[0], start[1], count[0], count[1]);*\/ */

/*    if (nc_var_par_access(ncid, v1id, NC_COLLECTIVE)) ERR; */
/* /\*    if (nc_var_par_access(ncid, v1id, NC_INDEPENDENT)) ERR;*\/ */

/*    /\* Write slabs of phoney data. *\/ */
/*    for (start[2] = 0; start[2] < NUM_SLABS; start[2]++) */
/*       if (nc_put_vara_int(ncid, v1id, start, count, slab_data)) ERR; */

   /* Close the netcdf file. */
   if (nc_close(ncid)) ERR;

   /* Shut down MPI. */
   MPI_Finalize();

   if (!mpi_rank)
   {
      SUMMARIZE_ERR;
      FINAL_RESULTS;
   }
   return 0;
}
