/*
 * Tests for PIO distributed arrays.
 *
 * Ed Hartnett, 2/16/17
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
#define TEST_NAME "test_darray_multivar"

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

/* The name of the variables in the netCDF output files. */
#define VAR_NAME_1 "STICKS"
#define VAR_NAME_2 "NIX"
#define VAR_NAME_3 "HICK"
#define VAR_NAME_4 "PIX"

/* Number of variables in the test file. */
#define NUM_VAR 4

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"timestep", "x", "y"};

/* The var names. */
char var_name[NUM_VAR][PIO_MAX_NAME + 1] = {"STICKS", "NIX", "HICK", "PIX"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/**
 * Test the darray functionality. Create a netCDF file with 3
 * dimensions and 4 variables, and use darray to write to one of them.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param pio_type the type of the data.
 * @param test_comm the communicator that is running this test.
 * @param rearranger the rearranger in use for this test.
 * @param use_fill 1 if fill mode should be set.
 * @param use_default 1 if default fill values should be used
 * (ignored if use_fill is 0).
 * @returns 0 for success, error code otherwise.
 */
int test_multivar_darray(int iosysid, int ioid, int num_flavors, int *flavor,
                         int my_rank, int pio_type, MPI_Comm test_comm,
                         int rearranger, int use_fill, int use_default)
{
#define NUM_MULTIVAR_TEST_CASES 2
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dimids[NDIM];     /* The dimension IDs. */
    int ncid;             /* The ncid of the netCDF file. */
    int ncid2;            /* The ncid of the re-opened netCDF file. */
    int varid[NUM_VAR];   /* The IDs of the netCDF varables. */
    PIO_Offset arraylen = 4;
    void *fillvalue;
    void *test_data;
    void *test_data_in;
    signed char fillvalue_byte = NC_FILL_BYTE;
    signed char custom_fillvalue_byte = -TEST_VAL_42;
    signed char test_data_byte[arraylen];
    signed char test_data_byte_in[arraylen];
    char fillvalue_char = NC_FILL_CHAR;
    char custom_fillvalue_char = -TEST_VAL_42;
    char test_data_char[arraylen];
    char test_data_char_in[arraylen];
    short fillvalue_short = NC_FILL_SHORT;
    short custom_fillvalue_short = -TEST_VAL_42;
    short test_data_short[arraylen];
    short test_data_short_in[arraylen];
    int fillvalue_int = NC_FILL_INT;
    int custom_fillvalue_int = -TEST_VAL_42;
    int test_data_int[arraylen];
    int test_data_int_in[arraylen];
    float fillvalue_float = NC_FILL_FLOAT;
    float custom_fillvalue_float = -TEST_VAL_42;
    float test_data_float[arraylen];
    float test_data_float_in[arraylen];
    double fillvalue_double = NC_FILL_DOUBLE;
    double custom_fillvalue_double = (-TEST_VAL_42 * 100);
    double test_data_double[arraylen];
    double test_data_double_in[arraylen];
#ifdef _NETCDF4
    unsigned char fillvalue_ubyte = NC_FILL_UBYTE;
    unsigned char custom_fillvalue_ubyte = TEST_VAL_42;
    unsigned char test_data_ubyte[arraylen];
    unsigned char test_data_ubyte_in[arraylen];
    unsigned short fillvalue_ushort = NC_FILL_USHORT;
    unsigned short custom_fillvalue_ushort = (TEST_VAL_42 * 100);
    unsigned short test_data_ushort[arraylen];
    unsigned short test_data_ushort_in[arraylen];
    unsigned int fillvalue_uint = NC_FILL_UINT;
    unsigned int custom_fillvalue_uint = (TEST_VAL_42 * 100);
    unsigned int test_data_uint[arraylen];
    unsigned int test_data_uint_in[arraylen];
    long long fillvalue_int64 = NC_FILL_INT64;
    long long custom_fillvalue_int64 = (TEST_VAL_42 * 100);
    long long test_data_int64[arraylen];
    long long test_data_int64_in[arraylen];
    unsigned long long fillvalue_uint64 = NC_FILL_UINT64;
    unsigned long long custom_fillvalue_uint64 = (TEST_VAL_42 * 100);
    unsigned long long test_data_uint64[arraylen];
    unsigned long long test_data_uint64_in[arraylen];
#endif /* _NETCDF4 */
    int ret;       /* Return code. */

    /* Initialize some data. */
    for (int f = 0; f < arraylen; f++)
    {
        test_data_byte[f] = my_rank * 10 + f;
        test_data_char[f] = my_rank * 10 + f;
        test_data_short[f] = my_rank * 10 + f;
        test_data_int[f] = my_rank * 10 + f;
        test_data_float[f] = my_rank * 10 + f + 0.5;
        test_data_double[f] = my_rank * 100000 + f + 0.5;
#ifdef _NETCDF4
        test_data_ubyte[f] = my_rank * 10 + f;
        test_data_ushort[f] = my_rank * 10 + f;
        test_data_uint[f] = my_rank * 10 + f;
        test_data_int64[f] = my_rank * 10 + f;
        test_data_uint64[f] = my_rank * 10 + f;
#endif /* _NETCDF4 */
    }

    /* Select the fill value and data. */
    switch (pio_type)
    {
    case PIO_BYTE:
        fillvalue = use_default ? &fillvalue_byte : &custom_fillvalue_byte;
        test_data = test_data_byte;
        test_data_in = test_data_byte_in;
        break;
    case PIO_CHAR:
        fillvalue = use_default ? &fillvalue_char : &custom_fillvalue_char;
        test_data = test_data_char;
        test_data_in = test_data_char_in;
        break;
    case PIO_SHORT:
        fillvalue = use_default ? &fillvalue_short : &custom_fillvalue_short;
        test_data = test_data_short;
        test_data_in = test_data_short_in;
        break;
    case PIO_INT:
        fillvalue = use_default ? &fillvalue_int : &custom_fillvalue_int;
        test_data = test_data_int;
        test_data_in = test_data_int_in;
        break;
    case PIO_FLOAT:
        fillvalue = use_default ? &fillvalue_float : &custom_fillvalue_float;
        test_data = test_data_float;
        test_data_in = test_data_float_in;
        break;
    case PIO_DOUBLE:
        fillvalue = use_default ? &fillvalue_double : &custom_fillvalue_double;
        test_data = test_data_double;
        test_data_in = test_data_double_in;
        break;
#ifdef _NETCDF4
    case PIO_UBYTE:
        fillvalue = use_default ? &fillvalue_ubyte : &custom_fillvalue_ubyte;
        test_data = test_data_ubyte;
        test_data_in = test_data_ubyte_in;
        break;
    case PIO_USHORT:
        fillvalue = use_default ? &fillvalue_ushort : &custom_fillvalue_ushort;
        test_data = test_data_ushort;
        test_data_in = test_data_ushort_in;
        break;
    case PIO_UINT:
        fillvalue = use_default ? &fillvalue_uint : &custom_fillvalue_uint;
        test_data = test_data_uint;
        test_data_in = test_data_uint_in;
        break;
    case PIO_INT64:
        fillvalue = use_default ? &fillvalue_int64 : &custom_fillvalue_int64;
        test_data = test_data_int64;
        test_data_in = test_data_int64_in;
        break;
    case PIO_UINT64:
        fillvalue = use_default ? &fillvalue_uint64 : &custom_fillvalue_uint64;
        test_data = test_data_uint64;
        test_data_in = test_data_uint64_in;
        break;
#endif /* _NETCDF4 */
    default:
        ERR(ERR_WRONG);
    }

    /* Try in pnetcdf only. This code demonstrates that pnetcdf fill
     * values do work. But not for PIO, so we have a bug somewhere. */
    {
#ifdef _PNETCDF
        int ncid;
        int varid;
        int dimid;
        char test_filename[] = "pnetcdf_test.nc";
        int ret;

        if ((ret = ncmpi_create(test_comm, test_filename, NC_CLOBBER, MPI_INFO_NULL, &ncid)))
            return ret;
        if ((ret = ncmpi_set_fill(ncid, NC_FILL, NULL)))
            return ret;
        if ((ret = ncmpi_def_dim(ncid, "dim_name", 5, &dimid)))
            return ret;
        if ((ret = ncmpi_def_var(ncid, "dim_name", NC_INT, 1, &dimid, &varid)))
            return ret;
        if ((ret = ncmpi_enddef(ncid)))
            return ret;
        if ((ret = ncmpi_close(ncid)))
            return ret;

        /* Reopen and check. */
        if ((ret = ncmpi_open(test_comm, test_filename, NC_NOWRITE, MPI_INFO_NULL, &ncid)))
            return ret;
        int datum;
        MPI_Offset start[1] = {0};
        ret = ncmpi_get_var1_int(ncid, varid, start, &datum);

        /* Not sure why this doesn't work. */
        /* if ((ret = ncmpi_get_var1_int(ncid, varid, start, &datum))) */
        /*     return ret; */
        if ((ret = ncmpi_close(ncid)))
            return ret;
#endif /* _PNETCDF */
    }

    for (int tc = 0; tc < NUM_MULTIVAR_TEST_CASES; tc++)
    {
        /* Use PIO to create the example file in each of the four
         * available ways. */
        for (int fmt = 0; fmt < num_flavors; fmt++)
        {
            /* BYTE and CHAR don't work with pnetcdf. Don't know why yet. */
            if (flavor[fmt] == PIO_IOTYPE_PNETCDF && (pio_type == PIO_BYTE || pio_type == PIO_CHAR))
                continue;

            /* NetCDF-4 types only work with netCDF-4 formats. */
            if (pio_type > PIO_DOUBLE && flavor[fmt] != PIO_IOTYPE_NETCDF4C &&
                flavor[fmt] != PIO_IOTYPE_NETCDF4P)
                continue;

            /* Create the filename. */
            sprintf(filename, "data_%s_iotype_%d_tc_%d_pio_type_%d_use_fill_%d_default_fill_%d.nc",
                    TEST_NAME, flavor[fmt], tc, pio_type, use_fill, use_default);

            /* Create the netCDF output file. */
            if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
                ERR(ret);

            /* Turn on fill mode if desired. */
            if (use_fill)
                if ((ret = PIOc_set_fill(ncid, NC_FILL, NULL)))
                    ERR(ret);

            /* Define netCDF dimensions and variable. */
            for (int d = 0; d < NDIM; d++)
                if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
                    ERR(ret);

            /* Define the variables. */
            for (int v = 0; v < NUM_VAR; v++)
            {
                if ((ret = PIOc_def_var(ncid, var_name[v], pio_type, NDIM, dimids, &varid[v])))
                    ERR(ret);
                if (use_fill && !use_default)
                    if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, fillvalue)))
                        ERR(ret);
            }

            /* End define mode. */
            if ((ret = PIOc_enddef(ncid)))
                ERR(ret);

            /* Set the value of the record dimension. */
            for (int v = 0; v < NUM_VAR; v++)
            {
                if ((ret = PIOc_setframe(ncid, varid[v], 0)))
                    ERR(ret);

                /* For the first test case we just write the first variable. */
                if (tc == 0)
                    break;
            }

            /* Write the data. */
            for (int v = 0; v < NUM_VAR; v++)
            {
                void *fp = use_fill ? fillvalue : NULL;
                if ((ret = PIOc_write_darray(ncid, varid[v], ioid, arraylen, test_data, fp)))
                    ERR(ret);

                /* For the first test case we just write the first variable. */
                if (tc == 0)
                    break;
            }

            /* Close the netCDF file. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);

            /* Reopen the file. */
            if ((ret = PIOc_openfile2(iosysid, &ncid2, &flavor[fmt], filename, PIO_NOWRITE)))
                ERR(ret);

            for (int v = 0; v < NUM_VAR; v++)
            {
                if ((ret = PIOc_setframe(ncid2, varid[v], 0)))
                    ERR(ret);

                /* Read the data. */
                if ((ret = PIOc_read_darray(ncid2, varid[v], ioid, arraylen, test_data_in)))
                    ERR(ret);

                /* Check the results. */
                for (int f = 0; f < arraylen; f++)
                {
                    switch (pio_type)
                    {
                    case PIO_BYTE:
                        if (test_data_byte_in[f] != test_data_byte[f])
                            return ERR_WRONG;
                        break;
                    case PIO_CHAR:
                        if (test_data_char_in[f] != test_data_char[f])
                            return ERR_WRONG;
                        break;
                    case PIO_SHORT:
                        if (test_data_short_in[f] != test_data_short[f])
                            return ERR_WRONG;
                        break;
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
#ifdef _NETCDF4
                    case PIO_UBYTE:
                        if (test_data_ubyte_in[f] != test_data_ubyte[f])
                            return ERR_WRONG;
                        break;
                    case PIO_USHORT:
                        if (test_data_ushort_in[f] != test_data_ushort[f])
                            return ERR_WRONG;
                        break;
                    case PIO_UINT:
                        if (test_data_uint_in[f] != test_data_uint[f])
                            return ERR_WRONG;
                        break;
                    case PIO_INT64:
                        if (test_data_int64_in[f] != test_data_int64[f])
                            return ERR_WRONG;
                        break;
                    case PIO_UINT64:
                        if (test_data_uint64_in[f] != test_data_uint64[f])
                            return ERR_WRONG;
                        break;
#endif /* _NETCDF4 */
                    default:
                        ERR(ERR_WRONG);
                    }
                }

                /* For the first test case we just write the first variable. */
                if (tc == 0)
                    break;
            } /* next var */

            /* For the first test case, if fill mode is in use the other vars
             * should have fill values. */
            if (tc == 0 && use_fill && flavor[fmt] != PIO_IOTYPE_PNETCDF)
            {
                if ((ret = PIOc_setframe(ncid2, varid[1], 0)))
                    ERR(ret);

                /* Read the data. */
                if ((ret = PIOc_read_darray(ncid2, varid[1], ioid, arraylen, test_data_in)))
                    ERR(ret);

                /* Check the results. */
                for (int f = 0; f < arraylen; f++)
                {
                    switch (pio_type)
                    {
                    case PIO_BYTE:
                        if (test_data_byte_in[f] != (use_default ? NC_FILL_BYTE : custom_fillvalue_byte))
                            return ERR_WRONG;
                        break;
                    case PIO_CHAR:
                        if (test_data_char_in[f] != (use_default ? NC_FILL_CHAR : custom_fillvalue_char))
                            return ERR_WRONG;
                        break;
                    case PIO_SHORT:
                        if (test_data_short_in[f] != (use_default ? NC_FILL_SHORT : custom_fillvalue_short))
                            return ERR_WRONG;
                        break;
                    case PIO_INT:
                        if (test_data_int_in[f] != (use_default ? NC_FILL_INT : custom_fillvalue_int))
                            return ERR_WRONG;
                        break;
                    case PIO_FLOAT:
                        if (test_data_float_in[f] != (use_default ? NC_FILL_FLOAT : custom_fillvalue_float))
                            return ERR_WRONG;
                        break;
                    case PIO_DOUBLE:
                        if (test_data_double_in[f] != (use_default ? NC_FILL_DOUBLE : custom_fillvalue_double))
                            return ERR_WRONG;
                        break;
#ifdef _NETCDF4
                    case PIO_UBYTE:
                        if (test_data_ubyte_in[f] != (use_default ? NC_FILL_UBYTE : custom_fillvalue_ubyte))
                            return ERR_WRONG;
                        break;
                    case PIO_USHORT:
                        if (test_data_ushort_in[f] != (use_default ? NC_FILL_USHORT : custom_fillvalue_ushort))
                            return ERR_WRONG;
                        break;
                    case PIO_UINT:
                        if (test_data_uint_in[f] != (use_default ? NC_FILL_UINT : custom_fillvalue_uint))
                            return ERR_WRONG;
                        break;
                    case PIO_INT64:
                        if (test_data_int64_in[f] != (use_default ? NC_FILL_INT64 : custom_fillvalue_int64))
                            return ERR_WRONG;
                        break;
                    case PIO_UINT64:
                        if (test_data_uint64_in[f] != (use_default ? NC_FILL_UINT64 : custom_fillvalue_uint64))
                            return ERR_WRONG;
                        break;
#endif /* _NETCDF4 */
                    default:
                        ERR(ERR_WRONG);
                    }
                }
            }

            /* Close the netCDF file. */
            if ((ret = PIOc_closefile(ncid2)))
                ERR(ret);
        }
    } /* next test case */

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
 * @param rearranger the rearranger in use in this test.
 * @returns 0 for success, error code otherwise.
 */
int test_all_darray(int iosysid, int num_flavors, int *flavor, int my_rank,
                    MPI_Comm test_comm, int rearranger)
{
#define NUM_FILL_TESTS 3
#ifdef _NETCDF4
#define NUM_TYPES_TO_TEST 11
    int test_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE,
                                        PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
#define NUM_TYPES_TO_TEST 6
    int test_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE};
#endif /* _NETCDF4 */
    int ioid;
    int dim_len_2d[NDIM2] = {X_DIM_LEN, Y_DIM_LEN};
    int ret; /* Return code. */

    for (int t = 0; t < NUM_TYPES_TO_TEST; t++)
    {
        int use_fill = 0;
        int use_default = 0;

        /* Decompose the data over the tasks. */
        if ((ret = create_decomposition_2d(TARGET_NTASKS, my_rank, iosysid, dim_len_2d,
                                           &ioid, test_type[t])))
            return ret;

        /* Run the different combinations of use_fill and use_default. */
        for (int f = 0; f < NUM_FILL_TESTS; f++)
        {
            /* Set flags for this test case. */
            if (f == 1)
                use_fill++;
            if (f == 2)
                use_default++;

            /* Run the multivar darray tests. */
            if ((ret = test_multivar_darray(iosysid, ioid, num_flavors, flavor, my_rank, test_type[t],
                                            test_comm, rearranger, use_fill, use_default)))
                return ret;
        }

        /* Free the PIO decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid)))
            ERR(ret);
    }

    return PIO_NOERR;
}

/* Run tests for darray functions. */
int main(int argc, char **argv)
{
#define NUM_REARRANGERS 2
    int rearranger[NUM_REARRANGERS] = {PIO_REARR_BOX, PIO_REARR_SUBSET};
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

        /* Test for both arrangers. */
        for (int r = 0; r < NUM_REARRANGERS; r++)
        {

            /* Initialize the PIO IO system. This specifies how
             * many and which processors are involved in I/O. */
            if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, ioproc_stride,
                                           ioproc_start, rearranger[r], &iosysid)))
                return ret;

            /* Run tests. */
            if ((ret = test_all_darray(iosysid, num_flavors, flavor, my_rank, test_comm,
                                       rearranger[r])))
                return ret;

            /* Finalize PIO system. */
            if ((ret = PIOc_finalize(iosysid)))
                return ret;
        }

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);
    return 0;
}
