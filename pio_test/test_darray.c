/*
 * Tests for PIO distributed arrays.
 *
 * @author Ed Hartnett
 * @date 2/16/17
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
#define TEST_NAME "test_darray"

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

/* The names of variables in the netCDF output files. */
#define VAR_NAME "Billy-Bob"
#define VAR_NAME2 "Sally-Sue"

/* Test cases relating to PIOc_write_darray_multi(). */
#define NUM_TEST_CASES_WRT_MULTI 3

/* Test with and without specifying a fill value to
 * PIOc_write_darray(). */
#define NUM_TEST_CASES_FILLVALUE 2

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"timestep", "x", "y"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/**
 * Test the darray functionality. Create a netCDF file with 3
 * dimensions and 1 PIO_INT variable, and use darray to write some
 * data.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param pio_type the type of the data.
 * @returns 0 for success, error code otherwise.
 */
int test_darray(int iosysid, int ioid, int num_flavors, int *flavor, int my_rank,
                int pio_type)
{
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dimids[NDIM];      /* The dimension IDs. */
    int ncid;      /* The ncid of the netCDF file. */
    int ncid2;     /* The ncid of the re-opened netCDF file. */
    int varid;     /* The ID of the netCDF varable. */
    int varid2;    /* The ID of a varable of different type. */
    int wrong_varid = TEST_VAL_42;  /* A wrong ID. */
    int ret;       /* Return code. */
    PIO_Offset arraylen = 4;
    void *fillvalue;
    void *test_data;
    void *test_data_in;
    int fillvalue_int = NC_FILL_INT;
    int test_data_int[arraylen];
    int test_data_int_in[arraylen];
    float fillvalue_float = NC_FILL_FLOAT;
    float test_data_float[arraylen];
    float test_data_float_in[arraylen];
    double fillvalue_double = NC_FILL_DOUBLE;
    double test_data_double[arraylen];
    double test_data_double_in[arraylen];

    /* Initialize some data. */
    for (int f = 0; f < arraylen; f++)
    {
        test_data_int[f] = my_rank * 10 + f;
        test_data_float[f] = my_rank * 10 + f + 0.5;
        test_data_double[f] = my_rank * 100000 + f + 0.5;
    }

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {

        /* Add a couple of extra tests for the
         * PIOc_write_darray_multi() function. */
        for (int test_multi = 0; test_multi < NUM_TEST_CASES_WRT_MULTI; test_multi++)
        {
            /* Test with/without providing a fill value to PIOc_write_darray(). */
            for (int provide_fill = 0; provide_fill < NUM_TEST_CASES_FILLVALUE; provide_fill++)
            {
                /* Create the filename. */
                sprintf(filename, "data_%s_iotype_%d_pio_type_%d_test_multi_%d_provide_fill_%d.nc", TEST_NAME,
                        flavor[fmt], pio_type, test_multi, provide_fill);

                /* Select the fill value and data. */
                switch (pio_type)
                {
                case PIO_INT:
                    fillvalue = provide_fill ? &fillvalue_int : NULL;
                    test_data = test_data_int;
                    test_data_in = test_data_int_in;
                    break;
                case PIO_FLOAT:
                    fillvalue = provide_fill ? &fillvalue_float : NULL;
                    test_data = test_data_float;
                    test_data_in = test_data_float_in;
                    break;
                case PIO_DOUBLE:
                    fillvalue = provide_fill ? &fillvalue_double : NULL;
                    test_data = test_data_double;
                    test_data_in = test_data_double_in;
                    break;
                default:
                    ERR(ERR_WRONG);
                }

                /* Create the netCDF output file. */
                if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
                    ERR(ret);

                /* Define netCDF dimensions and variable. */
                for (int d = 0; d < NDIM; d++)
                    if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
                        ERR(ret);

                /* Define a variable. */
                if ((ret = PIOc_def_var(ncid, VAR_NAME, pio_type, NDIM, dimids, &varid)))
                    ERR(ret);

                /* Define a variable with a different type. */
                int other_type = pio_type == PIO_INT ? PIO_FLOAT : PIO_INT;
                if ((ret = PIOc_def_var(ncid, VAR_NAME2, other_type, NDIM, dimids, &varid2)))
                    ERR(ret);

                /* End define mode. */
                if ((ret = PIOc_enddef(ncid)))
                    ERR(ret);

                /* Set the value of the record dimension. */
                if ((ret = PIOc_setframe(ncid, varid, 0)))
                    ERR(ret);

                int frame = 0;
                int flushtodisk = test_multi - 1;
                if (!test_multi)
                {
                    /* These should not work. */
                    if (PIOc_write_darray(ncid + TEST_VAL_42, varid, ioid, arraylen, test_data, fillvalue) != PIO_EBADID)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray(ncid, varid, ioid + TEST_VAL_42, arraylen, test_data, fillvalue) != PIO_EBADID)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray(ncid, varid, ioid, arraylen - 1, test_data, fillvalue) != PIO_EINVAL)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray(ncid, TEST_VAL_42, ioid, arraylen, test_data, fillvalue) != PIO_ENOTVAR)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray(ncid, varid2, ioid, arraylen, test_data, fillvalue) != PIO_EINVAL)
                        ERR(ERR_WRONG);

                    /* Write the data. */
                    if ((ret = PIOc_write_darray(ncid, varid, ioid, arraylen, test_data, fillvalue)))
                        ERR(ret);
                }
                else
                {
                    int varid_big = NC_MAX_VARS + TEST_VAL_42;

                    /* These will not work. */
                    if (PIOc_write_darray_multi(ncid + TEST_VAL_42, &varid, ioid, 1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_EBADID)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray_multi(ncid, NULL, ioid, 1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_EINVAL)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray_multi(ncid, &varid, ioid + TEST_VAL_42, 1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_EBADID)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray_multi(ncid, &varid, ioid, -1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_EINVAL)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray_multi(ncid, &varid_big, ioid, 1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_ENOTVAR)
                        ERR(ERR_WRONG);
                    if (PIOc_write_darray_multi(ncid, &wrong_varid, ioid, 1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_ENOTVAR)
                        ERR(ERR_WRONG);

                    /* Write the data with the _multi function. */
                    if ((ret = PIOc_write_darray_multi(ncid, &varid, ioid, 1, arraylen, test_data, &frame,
                                                       fillvalue, flushtodisk)))
                        ERR(ret);
                }

                /* Close the netCDF file. */
                if ((ret = PIOc_closefile(ncid)))
                    ERR(ret);

                /* Reopen the file. */
                if ((ret = PIOc_openfile(iosysid, &ncid2, &flavor[fmt], filename, PIO_NOWRITE)))
                    ERR(ret);

                /* These should not work. */
                if (PIOc_read_darray(ncid2 + TEST_VAL_42, varid, ioid, arraylen,
                                     test_data_in) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if (PIOc_read_darray(ncid2, varid, ioid + TEST_VAL_42, arraylen,
                                     test_data_in) != PIO_EBADID)
                    ERR(ERR_WRONG);

                /* Set the record number. */
                if ((ret = PIOc_setframe(ncid2, varid, 0)))
                    ERR(ret);

                /* Read the data. */
                if ((ret = PIOc_read_darray(ncid2, varid, ioid, arraylen, test_data_in)))
                    ERR(ret);

                /* Check the results. */
                for (int f = 0; f < arraylen; f++)
                {
                    switch (pio_type)
                    {
                    case PIO_INT:
                        if (test_data_int_in[f] != test_data_int[f])
                            return ERR_WRONG;
                        break;
                    case PIO_FLOAT:
                        if (test_data_float_in[f] != test_data_float[f])
                            return ERR_WRONG;
                        break;
                    case PIO_DOUBLE:
                        if (test_data_double_in[f] != test_data_double[f])
                            return ERR_WRONG;
                        break;
                    default:
                        ERR(ERR_WRONG);
                    }
                }

                /* Try to write, but it won't work, because we opened file read-only. */
                if (!test_multi)
                {
                    if (PIOc_write_darray(ncid2, varid, ioid, arraylen, test_data, fillvalue) != PIO_EPERM)
                        ERR(ERR_WRONG);
                }
                else
                {
                    if (PIOc_write_darray_multi(ncid2, &varid, ioid, 1, arraylen, test_data, &frame,
                                                fillvalue, flushtodisk) != PIO_EPERM)
                        ERR(ERR_WRONG);
                }

                /* Close the netCDF file. */
                if ((ret = PIOc_closefile(ncid2)))
                    ERR(ret);
            } /* next fillvalue test case */
        } /* next test multi */
    } /* next iotype */

    return PIO_NOERR;
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
#define NUM_TYPES_TO_TEST 3
    int ioid;
    char filename[NC_MAX_NAME + 1];
    int pio_type[NUM_TYPES_TO_TEST] = {PIO_INT, PIO_FLOAT, PIO_DOUBLE};
    int dim_len_2d[NDIM2] = {X_DIM_LEN, Y_DIM_LEN};
    int ret; /* Return code. */

    for (int t = 0; t < NUM_TYPES_TO_TEST; t++)
    {
        /* This will be our file name for writing out decompositions. */
        sprintf(filename, "%s_decomp_rank_%d_flavor_%d_type_%d.nc", TEST_NAME, my_rank,
                *flavor, pio_type[t]);

        /* Decompose the data over the tasks. */
        if ((ret = create_decomposition_2d(TARGET_NTASKS, my_rank, iosysid, dim_len_2d,
                                           &ioid, pio_type[t])))
            return ret;

        /* Run a simple darray test. */
        if ((ret = test_darray(iosysid, ioid, num_flavors, flavor, my_rank, pio_type[t])))
            return ret;

        /* Free the PIO decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid)))
            ERR(ret);
    }

    return PIO_NOERR;
}

/* Run tests for darray functions. */
int main(int argc, char **argv)
{
#define NUM_REARRANGERS_TO_TEST 2
    int rearranger[NUM_REARRANGERS_TO_TEST] = {PIO_REARR_BOX, PIO_REARR_SUBSET};
    int my_rank;
    int ntasks;
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    MPI_Comm test_comm; /* A communicator for this test. */
    int ret;         /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              MIN_NTASKS, -1, &test_comm)))
        ERR(ERR_INIT);

    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Only do something on max_ntasks tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        int iosysid;  /* The ID for the parallel I/O system. */
        int ioproc_stride = 1;    /* Stride in the mpi rank between io tasks. */
        int ioproc_start = 0;     /* Zero based rank of first processor to be used for I/O. */
        int ret;      /* Return code. */

        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        for (int r = 0; r < NUM_REARRANGERS_TO_TEST; r++)
        {
            /* Initialize the PIO IO system. This specifies how
             * many and which processors are involved in I/O. */
            if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, ioproc_stride,
                                           ioproc_start, rearranger[r], &iosysid)))
                return ret;

            /* Run tests. */
            if ((ret = test_all_darray(iosysid, num_flavors, flavor, my_rank, test_comm)))
                return ret;

            /* Finalize PIO system. */
            if ((ret = PIOc_finalize(iosysid)))
                return ret;
        } /* next rearranger */
    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);
    return 0;
}
