/* Copyright 2007-2011, UCAR/Unidata. See COPYRIGHT file for copying
 * and redistribution conditions.
 *
 * This is part of the netCDF package.
 *
 * This test is for parallel IO and the collective access of metadata
 * with HDF5.
 *
 * Ward Fisher, Ed Hartnett
 */

#include "config.h"
#include "nc_tests.h"
#include "err_macros.h"

#define TEST_NAME "tst_parallel4_simplerw_coll"
#define NDIMS 3
#define DIMSIZE 16
#define NUM_SLABS 16
#define DIM1_NAME "slab"
#define DIM2_NAME "x"
#define DIM3_NAME "y"
#define VAR_NAME "Bond_James_Bond"

int
main(int argc, char **argv)
{
   int mpi_namelen;
   char mpi_name[MPI_MAX_PROCESSOR_NAME];
   int mpi_size, mpi_rank;
   MPI_Comm comm = MPI_COMM_WORLD;
   MPI_Info info = MPI_INFO_NULL;
   double start_time = 0, total_time;
   int ncid, varid, dimids[NDIMS];
   size_t start[NDIMS] = {0, 0, 0};
   size_t count[NDIMS] = {1, DIMSIZE, DIMSIZE};
   int int_data[DIMSIZE * DIMSIZE], int_data_in[DIMSIZE * DIMSIZE];
   void *data, *data_in;
   int ndims_in, nvars_in, natts_in, unlimdimid_in;
   int int_fill_value = TEST_VAL_42;
   int int_fill_value_in;
   int fill_mode_in;
   int mpi_size_in;
#define NUM_TEST_TYPES 1
   nc_type test_type[1] = {NC_INT};
   int tt;
   int j, i, ret;

   /* Initialize MPI. */
   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
   MPI_Get_processor_name(mpi_name, &mpi_namelen);

   /* Must be able to evenly divide my slabs between processors. */
   if (NUM_SLABS % mpi_size)
   {
      if (!mpi_rank)
         printf("NUM_SLABS (%d) is not evenly divisible by mpi_size(%d)\n",
                NUM_SLABS, mpi_size);
      ERR;
   }

   if (!mpi_rank)
   {
      printf("\n*** Testing parallel I/O some more.\n");
      printf("*** writing a %d x %d x %d file from %d processors...\n", NUM_SLABS,
             DIMSIZE, DIMSIZE, mpi_size);
   }

   for (tt = 0; tt < NUM_TEST_TYPES; tt++)
   {
      char file_name[NC_MAX_NAME + 1];
      
      /* Initialize test data. */
      switch(test_type[tt])
      {
      case NC_INT:
         for (i = 0; i < DIMSIZE * DIMSIZE; i++)
            int_data[i] = mpi_rank;
         data = int_data;
         data_in = int_data_in;
         break;
      }
      
      /* Create a file name. */
      sprintf(file_name, "%s_type_%d.nc", TEST_NAME, test_type[tt]);

      /* Create a parallel netcdf-4 file. */
      if (nc_create_par(file_name, NC_NETCDF4|NC_MPIIO, comm, info, &ncid)) ERR;

      /* A global attribute holds the number of processors that created
       * the file. */
      if (nc_put_att_int(ncid, NC_GLOBAL, "num_processors", NC_INT, 1, &mpi_size)) ERR;

      /* Create three dimensions. */
      if (nc_def_dim(ncid, DIM1_NAME, NUM_SLABS, dimids)) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIMSIZE, &dimids[1])) ERR;
      if (nc_def_dim(ncid, DIM3_NAME, DIMSIZE, &dimids[2])) ERR;

      /* Create one var. */
      if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIMS, dimids, &varid)) ERR;
      if (nc_put_att_int(ncid, varid, "var_num_processors", NC_INT, 1, &mpi_size)) ERR;
      if (nc_def_var_fill(ncid, varid, NC_FILL, &int_fill_value)) ERR;
      if (nc_inq_var_fill(ncid, varid, &fill_mode_in, &int_fill_value_in)) ERR;
      if (fill_mode_in != NC_FILL || int_fill_value_in != TEST_VAL_42) ERR;

      /* Write metadata to file. */
      if (nc_enddef(ncid)) ERR;

      /* Change access mode to collective, then back to independent. */
      if (nc_var_par_access(ncid, varid, NC_COLLECTIVE)) ERR;
      if (nc_var_par_access(ncid, varid, NC_INDEPENDENT)) ERR;

      if (!mpi_rank)
         start_time = MPI_Wtime();

      /* Write all the slabs this process is responsible for. */
      for (i = 0; i < NUM_SLABS / mpi_size; i++)
      {
         start[0] = NUM_SLABS / mpi_size * mpi_rank + i;

         /* Write one slab of data. */
         if (nc_put_vara(ncid, varid, start, count, data)) ERR;
      }

      /* On rank 0, keep track of time. */
      if (!mpi_rank)
      {
         total_time = MPI_Wtime() - start_time;
         printf("%d\t%g\t%g\n", mpi_size, total_time, DIMSIZE * DIMSIZE * NUM_SLABS *
                sizeof(int) / total_time);
      }

      /* Close the netcdf file. */
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it. */
      if ((ret = nc_open_par(file_name, NC_NOWRITE|NC_MPIIO, comm, info, &ncid))) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != NDIMS || nvars_in != 1 || natts_in != 1 ||
          unlimdimid_in != -1) ERR;

      /* Check the attributes. */
      if (nc_get_att_int(ncid, NC_GLOBAL, "num_processors", &mpi_size_in)) ERR;
      if (mpi_size_in != mpi_size) ERR;
      if (nc_get_att_int(ncid, 0, "var_num_processors", &mpi_size_in)) ERR;
      if (mpi_size_in != mpi_size) ERR;
      if (nc_inq_var_fill(ncid, varid, &fill_mode_in, &int_fill_value_in)) ERR;
      if (fill_mode_in != NC_FILL || int_fill_value_in != TEST_VAL_42) ERR;

      /* Read all the slabs this process is responsible for. */
      for (i = 0; i < NUM_SLABS / mpi_size; i++)
      {
         start[0] = NUM_SLABS / mpi_size * mpi_rank + i;
         printf("mpi_rank %d i %d start[0] %ld\n", mpi_rank, i, start[0]);

         /* Read one slab of data. */
         if (nc_get_vara(ncid, varid, start, count, data_in)) ERR;

         /* Check data. */
         switch (test_type[tt])
         {
         case NC_INT:
            for (j = 0; j < DIMSIZE * DIMSIZE; j++)
               if (int_data_in[j] != mpi_rank) ERR;
            break;
         }
      } /* next slab */

      /* Close the netcdf file. */
      if (nc_close(ncid))  ERR;
   } /* next test type */
   
   /* Shut down MPI. */
   MPI_Finalize();

   if (!mpi_rank)
   {
      SUMMARIZE_ERR;
      FINAL_RESULTS;
   }
   return 0;
}
