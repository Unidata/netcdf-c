/*
 * Tests for PIO distributed arrays.
 *
 * Ed Hartnett, Jim Edwards, 4/20/17
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_darray_multivar2"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* The number of dimensions in the example data. In this test, we
 * are using three-dimensional data. */
#define NDIM 3

/* But sometimes we need arrays of the non-record dimensions. */
#define NDIM2 2

/* The length of our sample data along each dimension. */
#define X_DIM_LEN 4
#define Y_DIM_LEN 4

/* The number of timesteps of data to write. */
#define NUM_TIMESTEPS 2

/* Number of variables in the test file. */
#define NUM_VAR 2

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"timestep", "x", "y"};

/* The var names. */
char var_name[NUM_VAR][PIO_MAX_NAME + 1] = {"Aubery", "Martin"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/**
 * Test the darray functionality. Create a netCDF file with 3
 * dimensions and 2 variables. One of the vars uses the record
 * dimension, the other does not. Then use darray to write to them.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param pio_type the type of the data.
 * @param test_comm the communicator that is running this test.
 * @returns 0 for success, error code otherwise.
 */
int test_multivar_darray(int iosysid, int ioid, int num_flavors, int *flavor,
                         int my_rank, int pio_type, MPI_Comm test_comm)
{
    /* char filename[PIO_MAX_NAME + 1]; /\* Name for the output files. *\/ */
    /* int dimids[NDIM];     /\* The dimension IDs. *\/ */
    /* int ncid;             /\* The ncid of the netCDF file. *\/ */
    /* int varid[NUM_VAR];   /\* The IDs of the netCDF varables. *\/ */
    /* PIO_Offset arraylen = 4; */
    /* int custom_fillvalue_int = -TEST_VAL_42; */
    /* int test_data_int[arraylen]; */
    /* int ret;       /\* Return code. *\/ */

    /* /\* Initialize some data. *\/ */
    /* for (int f = 0; f < arraylen; f++) */
    /*     test_data_int[f] = my_rank * 10 + f; */

    /* /\* Use PIO to create the example file in each of the four */
    /*  * available ways. *\/ */
    /* for (int fmt = 0; fmt < num_flavors; fmt++) */
    /* { */
    /*     /\* Create the filename. *\/ */
    /*     sprintf(filename, "data_%s_iotype_%d_pio_type_%d.nc", TEST_NAME, flavor[fmt], pio_type); */

    /*     /\* Create the netCDF output file. *\/ */
    /*     printf("rank: %d Creating sample file %s with format %d type %d\n", my_rank, filename, */
    /*            flavor[fmt], pio_type); */
    /*     if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER))) */
    /*         ERR(ret); */

    /*     /\* Define netCDF dimensions and variable. *\/ */
    /*     printf("%d Defining netCDF metadata...\n", my_rank); */
    /*     for (int d = 0; d < NDIM; d++) */
    /*         if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d]))) */
    /*             ERR(ret); */

    /*     /\* Var 0 does not have a record dim, varid 1 is a record var. *\/ */
    /*     if ((ret = PIOc_def_var(ncid, var_name[0], pio_type, NDIM - 1, &dimids[1], &varid[0]))) */
    /*         ERR(ret); */
    /*     if ((ret = PIOc_def_var(ncid, var_name[1], pio_type, NDIM, dimids, &varid[1]))) */
    /*         ERR(ret); */

    /*     /\* End define mode. *\/ */
    /*     if ((ret = PIOc_enddef(ncid))) */
    /*         ERR(ret); */

    /*     /\* Write the data. *\/ */
    /*     for (int v = 0; v < NUM_VAR; v++) */
    /*     { */
    /*         if ((ret = PIOc_setframe(ncid, varid[v], 0))) */
    /*             ERR(ret); */
    /*         if ((ret = PIOc_write_darray(ncid, varid[v], ioid, arraylen, test_data_int, &custom_fillvalue_int))) */
    /*             ERR(ret); */
    /*     } */

    /*     /\* Close the netCDF file. *\/ */
    /*     if ((ret = PIOc_closefile(ncid))) */
    /*         ERR(ret); */

    /*     /\* Check the file contents. *\/ */
    /*     { */
    /*         int ncid2;            /\* The ncid of the re-opened netCDF file. *\/ */
    /*         int test_data_int_in[arraylen]; */

    /*         /\* Reopen the file. *\/ */
    /*         if ((ret = PIOc_openfile(iosysid, &ncid2, &flavor[fmt], filename, PIO_NOWRITE))) */
    /*             ERR(ret); */

    /*         for (int v = 0; v < NUM_VAR; v++) */
    /*         { */
    /*             if ((ret = PIOc_setframe(ncid2, varid[v], 0))) */
    /*                 ERR(ret); */

    /*             /\* Read the data. *\/ */
    /*             if ((ret = PIOc_read_darray(ncid2, varid[v], ioid, arraylen, test_data_int_in))) */
    /*                 ERR(ret); */

    /*             /\* Check the results. *\/ */
    /*             for (int f = 0; f < arraylen; f++) */
    /*                 if (test_data_int_in[f] != test_data_int[f]) */
    /*                 { */
    /*                     printf("my_rank %d test_data_int_in[%d] = %d expected %d\n", my_rank, */
    /*                            f, test_data_int_in[f], test_data_int[f]); */
    /*                     return ERR_WRONG; */
    /*                 } */
    /*         } /\* next var *\/ */

    /*         /\* Close the netCDF file. *\/ */
    /*         if ((ret = PIOc_closefile(ncid2))) */
    /*             ERR(ret); */
    /*     } */
    /* } */

    return PIO_NOERR;
}

/* Create the decomposition to divide the 3-dimensional sample data
 * between the 4 tasks. For the purposes of decomposition we are only
 * concerned with 2 dimensions - we ignore the unlimited dimension.
 *
 * @param ntasks the number of available tasks
 * @param my_rank rank of this task.
 * @param iosysid the IO system ID.
 * @param dim_len_2d an array of length 2 with the dim lengths.
 * @param ioid a pointer that gets the ID of this decomposition.
 * @param pio_type the data type to use for the decomposition.
 * @returns 0 for success, error code otherwise.
 **/
int create_decomposition_2d_2(int ntasks, int my_rank, int iosysid, int *dim_len_2d,
                              int *ioid, int pio_type)
{
    PIO_Offset elements_per_pe;     /* Array elements per processing unit. */
    PIO_Offset *compdof;  /* The decomposition mapping. */
    int ret;

    /* How many data elements per task? In this example we will end up
     * with 4. */
    elements_per_pe = dim_len_2d[0] * dim_len_2d[1] / ntasks;

    /* Allocate space for the decomposition array. */
    if (!(compdof = malloc(elements_per_pe * sizeof(PIO_Offset))))
        return PIO_ENOMEM;

    /* Describe the decomposition. This is a 1-based array, so add 1! */
    for (int i = 0; i < elements_per_pe; i++)
        compdof[i] = my_rank * elements_per_pe + i + 1;

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_InitDecomp(iosysid, pio_type, NDIM2, dim_len_2d, elements_per_pe,
                               compdof, ioid, NULL, NULL, NULL)))
        ERR(ret);

    /* Free the mapping. */
    free(compdof);

    return 0;
}

/**
 * Run all the tests.
 *
 * @param iosysid the IO system ID.
 * @param num_flavors number of available iotypes in the build.
 * @param flavor pointer to array of the available iotypes.
 * @param my_rank rank of this task.
 * @param test_comm the communicator the test is running on.
 * @returns 0 for success, error code otherwise.
 */
int test_all_darray(int iosysid, int num_flavors, int *flavor, int my_rank,
                    MPI_Comm test_comm)
{
    int ioid;
    int dim_len_2d[NDIM2] = {X_DIM_LEN, Y_DIM_LEN};
    int ret; /* Return code. */

    /* Decompose the data over the tasks. */
    if ((ret = create_decomposition_2d_2(TARGET_NTASKS, my_rank, iosysid, dim_len_2d,
                                         &ioid, PIO_INT)))
        return ret;

    /* Run the multivar darray tests. */
    if ((ret = test_multivar_darray(iosysid, ioid, num_flavors, flavor, my_rank, PIO_INT,
                                    test_comm)))
        return ret;

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return PIO_NOERR;
}

/* Run tests for darray functions. */
int main(int argc, char **argv)
{
    int my_rank;
    int ntasks;
    int num_flavors;         /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    MPI_Comm test_comm;      /* A communicator for this test. */
    int ret;                 /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS, MIN_NTASKS,
                              -1, &test_comm)))
        ERR(ERR_INIT);

    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Only do something on max_ntasks tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        int iosysid;              /* The ID for the parallel I/O system. */
        int ioproc_stride = 1;    /* Stride in the mpi rank between io tasks. */
        int ioproc_start = 0;     /* Zero based rank of first processor to be used for I/O. */
        int ret;                  /* Return code. */

        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        /* Initialize the PIO IO system. This specifies how
         * many and which processors are involved in I/O. */
        if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, ioproc_stride,
                                       ioproc_start, PIO_REARR_BOX, &iosysid)))
            return ret;

        /* Run tests. */
        if ((ret = test_all_darray(iosysid, num_flavors, flavor, my_rank, test_comm)))
            return ret;

        /* Finalize PIO system. */
        if ((ret = PIOc_finalize(iosysid)))
            return ret;

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);
    return 0;
}
