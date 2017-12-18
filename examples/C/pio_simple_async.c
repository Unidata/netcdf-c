/*
 * This is a very simple example showing how to use PIO async mode.
 *
 * Ed Hartnett 12/18/17
 */
#include <config.h>
#include <netcdf_par.h>
#include <stdio.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 1

/* The name of this test. */
#define TEST_NAME "tst_darray_async"

/* For 1-D use. */
#define NDIM1 1

/* For 2-D use. */
#define NDIM2 2

/* For 3-D use. */
#define NDIM3 3

/* For maplens of 2. */
#define MAPLEN2 2

/* Lengths of non-unlimited dimensions. */
#define LAT_LEN 2
#define LON_LEN 3

/* Number of vars in test file. */
#define NVAR 4

/* Number of records written for record var. */
#define NREC 4

/* Name of record test var. */
#define REC_VAR_NAME "surface_temperature"
#define REC_VAR_NAME2 "surface_temperature2"

/* Name of non-record test var. */
#define NOREC_VAR_NAME "surface_height"
#define NOREC_VAR_NAME2 "surface_height2"

/* char dim_name[NDIM3][PIO_MAX_NAME + 1] = {"unlim", "lat", "lon"}; */

/* Length of the dimension. */
#define LEN3 3

#define NUM_VAR_SETS 2

#define NTASKS 4

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

/* Run Tests for pio_spmd.c functions. */
int main(int argc, char **argv)
{
   int my_rank; /* Zero-based rank of processor. */
   int ntasks;  /* Number of processors involved in current execution. */
   MPI_Comm comm;
   int ret;

   /* Initialize MPI. */
   if ((ret = MPI_Init(&argc, &argv)))
      MPIERR(ret);

   /* Learn my rank and the total number of processors. */
   if ((ret = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank)))
      MPIERR(ret);
   if ((ret = MPI_Comm_size(MPI_COMM_WORLD, &ntasks)))
      MPIERR(ret);

   /* Check that a valid number of processors was specified. */
   if (ntasks != NTASKS)
   {
      fprintf(stderr, "ERROR: Number of processors must be %d!\n", NTASKS);
      return ERR_AWFUL;
   }

   /* Create a communicator with all tasks. */
   if ((ret = MPI_Comm_dup(MPI_COMM_WORLD, &comm)))
      MPIERR(ret);

   /* Turn on logging. */
   /* if ((ret = nc_set_log_level(3))) */
   /*    return ret; */

   /* Change error handling to return error codes on error. */
   if ((ret = nc_set_iosystem_error_handling(NC_PIO_DEFAULT, NC_PIO_RETURN_ERROR, NULL)))
      return ret;

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
