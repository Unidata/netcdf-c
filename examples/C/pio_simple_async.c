/*
 * This is a very simple example showing how to use PIO async mode.
 *
 * Ed Hartnett 12/18/17
 */
#include <config.h>
#include <netcdf.h>
#include <netcdf_par.h>
#include <stdio.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

#define TEST_NAME "pio_simple_async"

/** Global err buffer for MPI. When there is an MPI error, this buffer
 * is used to store the error message that is associated with the MPI
 * error. */
char err_buffer[MPI_MAX_ERROR_STRING];

/** This is the length of the most recent MPI error message, stored
 * int the global error string. */
int resultlen;

#define ERR_AWFUL 1111

/** Handle MPI errors. This should only be used with MPI library
 * function calls. */
#define MPIERR(e) do {                                                  \
        MPI_Error_string(e, err_buffer, &resultlen);                    \
        fprintf(stderr, "MPI error, line %d, file %s: %s\n", __LINE__, __FILE__, err_buffer); \
        MPI_Finalize();                                                 \
        return ERR_AWFUL;                                               \
    } while (0)

/** Handle non-MPI errors by finalizing the MPI library and exiting
 * with an exit code. */
#define ERR(e) do {                                                     \
        fprintf(stderr, "%d Error %d in %s, line %d\n", my_rank, e, __FILE__, __LINE__); \
        MPI_Finalize();                                                 \
        return e;                                                       \
    } while (0)

/* Initialize with task 0 as IO task, tasks 1-3 as a
 * computation component. */
#define NUM_IO_PROCS 1
#define NUM_COMPUTATION_PROCS 3
#define COMPONENT_COUNT 1

#define NDIM3 3
#define NDIM2 2
#define XLEN 6
#define YLEN 2

#define VAR_NAME "var_0"

/* This is called on the compute component (which consists of tasks 1,
 * 2, and 3). This code does not run on the IO component. NetCDF calls
 * from this code will send messages to the IO component, which will
 * perform the IO. */
int do_some_computation(int iosysid, int my_rank, int ntasks, int num_compute_tasks)
{
   int ncid;
   char filename[NC_MAX_NAME + 1];
   int dimid[NDIM3];
   size_t dim_len[NDIM3] = {NC_UNLIMITED, XLEN, YLEN};
   int varid;
   int ioid;
   int elements_per_pe;
   int ret;

   /* Determine output file name. */
   sprintf(filename, "%s.nc", TEST_NAME);
   printf("Doing computation on rank %d of %d for sample file %s\n", my_rank,
          ntasks, filename);

   /* Create a classic format data output file. The NC_PIO flag
    * indicates that PIO should be used. NC_NETCDF4 can be added to
    * create a netCDF-4 file. NC_MPIIO or NC_SHARE can be combined
    * with NC_NETCDF4 to indicate that the IO component should use
    * netCDF-4 parallel I/O. When pnetcdf support is complete,
    * NC_PNETCDF can be used to indicate that pnetcdf should be
    * used. */
   if ((ret = nc_create(filename, NC_PIO, &ncid)))
      ERR(ret);

   /* Define some dims in usual netCDF way. */
   for (int d = 0; d < NDIM3; d++)
   {
      char dim_name[NC_MAX_NAME + 1];
      sprintf(dim_name, "dim_%d", d);
      if ((ret = nc_def_dim(ncid, dim_name, dim_len[d], &dimid[d])))
         ERR(ret);
   }

   /* Create a var in the usual netCDF way. */
   if ((ret = nc_def_var(ncid, VAR_NAME, NC_INT, NDIM3, dimid, &varid)))
      ERR(ret);

   /* End define mode as we usualy would before writing data. */
   if ((ret = nc_enddef(ncid)))
      ERR(ret);

   /* How many values from this var should be on each processor? In
    * this case we have a record with 12 values, and we have 3 compute
    * processors. So each processor will get 4 elements from the
    * overall array. In general, the array need not be evenly
    * distributed, and each processor does not need to have the same
    * number of elements. But for this simple example, the math works
    * out even. */
   elements_per_pe = XLEN * YLEN / num_compute_tasks;

   /* The decomp_map array contains a mapping for this task into the
    * global data array. */
   PIO_Offset decomp_map[elements_per_pe];        /* The decomposition mapping. */   
   for (int i = 0; i < elements_per_pe; i++)
   {
      decomp_map[i] = (my_rank - 1) * elements_per_pe + i;
      printf("task %d element %d is var element %lld\n", my_rank, i, decomp_map[i]);
   }

   /* Create a decomposition to split a record from this var across
    * the three tasks of the computation component. */
   int rec_dim_len[NDIM2] = {XLEN, YLEN};
   if ((ret = nc_init_decomp(iosysid, NC_INT, NDIM2, rec_dim_len, elements_per_pe,
                             decomp_map, &ioid, NC_PIO_REARR_BOX, NULL, NULL)))
      ERR(ret);

   /* Create a local array on this processor to represent local
    * data. In a model these would be the locally-computed values that
    * will end up in the output netCDF data file. The local array only
    * has a subset of the data of the netCDF variable. For this simple
    * example, set all values to the rank. */
   int local_data[elements_per_pe];
   for (int e = 0; e < elements_per_pe; e++)
      local_data[e] = my_rank;

   /* Write one record of data to the netCDF variable. (Actually this
    * just buffers the data in memory on the IO component. It will be
    * written to disk when memory is running out, or when the file is
    * synced or closed. This makes it very fast.) */
   if ((ret = nc_setframe(ncid, varid, 0)))
      ERR(ret);
   if ((ret = nc_write_darray(ncid, varid, ioid, elements_per_pe, local_data,
                              NULL)))
      ERR(ret);

   /* Close the output file. */
   if ((ret = nc_close(ncid)))
      ERR(ret);

   /* Free the decomposition. */
   if ((ret = nc_free_decomp(iosysid, ioid)))
      ERR(ret);
   
   return 0;
}

/* Run Tests for pio_spmd.c functions. */
int main(int argc, char **argv)
{
   int my_rank; /* Zero-based rank of processor. */
   int ntasks;  /* Number of processors involved in current execution. */
   MPI_Comm comm;
   int num_computation_procs = NUM_COMPUTATION_PROCS;
   MPI_Comm io_comm;                    /* Will get a duplicate of IO communicator. */
   MPI_Comm comp_comm[COMPONENT_COUNT]; /* Will get duplicates of computation communicator(s). */
   int iosysid;   
   int ret;

   /* Initialize MPI. */
   if ((ret = MPI_Init(&argc, &argv)))
      MPIERR(ret);

   /* Learn my rank and the total number of processors. */
   if ((ret = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank)))
      MPIERR(ret);
   if ((ret = MPI_Comm_size(MPI_COMM_WORLD, &ntasks)))
      MPIERR(ret);

   /* Check that the correct number of processors was specified. */
   if (ntasks != TARGET_NTASKS)
   {
      fprintf(stderr, "ERROR: Number of processors must be %d!\n", TARGET_NTASKS);
      return ERR_AWFUL;
   }

   /* Create a communicator with all four tasks. */
   if ((ret = MPI_Comm_dup(MPI_COMM_WORLD, &comm)))
      MPIERR(ret);

   /* Change error handling to return error codes on error. */
   if ((ret = nc_set_iosystem_error_handling(NC_PIO_DEFAULT, NC_PIO_RETURN_ERROR,
                                             NULL)))
      ERR(ret);

   /* Initialize the IO System. This must run on all tasks in
    * comm. The IO tasks will not return from this call. They will
    * enter a message loop, waiting to hear from the computation
    * components that some IO is desired. */
   if ((ret = nc_init_async(comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                            &num_computation_procs, NULL, &io_comm, comp_comm,
                            NC_PIO_REARR_BOX, &iosysid)))
      ERR(ret);

   /* Computation component runs compute code. This is where a model
    * would be run. In this simple example, there is onle one compute
    * component, but in general there can be many. Each would run
    * different code from this point forward. When they make netCDF
    * calls, the compute component will send the request and
    * parameters to the IO component for execution. */
   if (my_rank)
   {
      /* Do IO-producing computations here. This is where a user would
       * kick off a model run. */
      if ((ret = do_some_computation(iosysid, my_rank, ntasks, NUM_COMPUTATION_PROCS)))
         ERR(ret);
      
      /* Finalize PIO system. All IO is complete. This must be called
       * on all processes in the computational component. The IO
       * component will shut down when all IO systems have been
       * freed. */
      if ((ret = nc_free_iosystem(iosysid)))
         ERR(ret);

      /* Free the computation conomponent communicator. */
      if ((ret = MPI_Comm_free(comp_comm)))
         MPIERR(ret);
   }
   else
   {
      /* Free the IO communicator. This will be run after the
       * computation component calls nc_free_iosystem(), which will
       * cause nc_init_async() to return 0 on the IO task. */
      if ((ret = MPI_Comm_free(&io_comm)))
         MPIERR(ret);
   }

   /* Wait for all processes to arrive here. */
   if ((ret = MPI_Barrier(comm)))
      MPIERR(ret);

   /* Free communicator. */
   if ((ret = MPI_Comm_free(&comm)))
      MPIERR(ret);

   /* Finalize MPI. */
   MPI_Finalize();

   printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

   return 0;
}
