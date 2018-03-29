/*
 * Tests for PIO distributed arrays. This program tests the
 * PIOc_write_darray_multi() function with more than one variable.
 *
 * @author Ed Hartnett
 * @date 3/7/17
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
#define TEST_NAME "test_darray_multi"

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

/* Number of variables. */
#define NVAR 3

/* For attributes. */
#define NOTE_NAME "note"
#define NOTE "This is a test file for the PIO library, and may be deleted."

/* Who would have thought? */
#define TOTAL_NUMBER_OF_STOOGES_NAME "Total_Number_of_Stooges"
#define TOTAL_NUMBER_OF_STOOGES 6

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"year", "Stooge_popularity", "face_smacks"};

/* The variable names. */
char var_name[NVAR][PIO_MAX_NAME + 1] = {"Larry", "Curly", "Moe"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/**
 * Test the darray functionality. Create a netCDF file with 3
 * dimensions and 3 variable, and use PIOc_write_darray_multi() to
 * write one record of data to all three vars at once.
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
#define NUM_TEST_CASES_WRT_MULTI 2
#define NUM_TEST_CASES_FILLVALUE 2

    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dimids[NDIM];      /* The dimension IDs. */
    int ncid;              /* The ncid of the netCDF file. */
    int ncid2;             /* The ncid of the re-opened netCDF file. */
    int varid[NVAR];       /* The IDs of the netCDF varables. */
    int other_varid;       /* The IDs of a var of different type. */
    int wrong_varid[NVAR]; /* These will not work. */
    PIO_Offset arraylen = 4; /* Amount of data from each task. */
    void *fillvalue;       /* Pointer to fill value. */
    void *test_data;       /* Pointer to test data we will write. */
    void *test_data_in;    /* Pointer to buffer we will read into. */
    int ret;               /* Return code. */

    /* Default fill value array for each type. */
    signed char byte_fill[NVAR] = {NC_FILL_BYTE, NC_FILL_BYTE, NC_FILL_BYTE};
    char char_fill[NVAR] = {NC_FILL_CHAR, NC_FILL_CHAR, NC_FILL_CHAR};
    short short_fill[NVAR] = {NC_FILL_SHORT, NC_FILL_SHORT, NC_FILL_SHORT};
    int int_fill[NVAR] = {NC_FILL_INT, NC_FILL_INT, NC_FILL_INT};
    float float_fill[NVAR] = {NC_FILL_FLOAT, NC_FILL_FLOAT, NC_FILL_FLOAT};
    double double_fill[NVAR] = {NC_FILL_DOUBLE, NC_FILL_DOUBLE, NC_FILL_DOUBLE};
#ifdef _NETCDF4
    unsigned char ubyte_fill[NVAR] = {NC_FILL_UBYTE, NC_FILL_UBYTE, NC_FILL_UBYTE};
    unsigned short ushort_fill[NVAR] = {NC_FILL_USHORT, NC_FILL_USHORT, NC_FILL_USHORT};
    unsigned int uint_fill[NVAR] = {NC_FILL_UINT, NC_FILL_UINT, NC_FILL_UINT};
    long long int64_fill[NVAR] = {NC_FILL_INT64, NC_FILL_INT64, NC_FILL_INT64};
    unsigned long long uint64_fill[NVAR] = {NC_FILL_UINT64, NC_FILL_UINT64, NC_FILL_UINT64};
#endif /* _NETCDF4 */

    /* Test data we will write. */
    signed char test_data_byte[arraylen * NVAR];
    char test_data_char[arraylen * NVAR];
    short test_data_short[arraylen * NVAR];
    int test_data_int[arraylen * NVAR];
    float test_data_float[arraylen * NVAR];
    double test_data_double[arraylen * NVAR];
#ifdef _NETCDF4
    unsigned char test_data_ubyte[arraylen * NVAR];
    unsigned short test_data_ushort[arraylen * NVAR];
    unsigned int test_data_uint[arraylen * NVAR];
    long long test_data_int64[arraylen * NVAR];
    unsigned long long test_data_uint64[arraylen * NVAR];
#endif /* _NETCDF4 */

    /* We will read test data into these buffers. */
    signed char test_data_byte_in[arraylen];
    char test_data_char_in[arraylen];
    short test_data_short_in[arraylen];
    int test_data_int_in[arraylen];
    float test_data_float_in[arraylen];
    double test_data_double_in[arraylen];
#ifdef _NETCDF4
    unsigned char test_data_ubyte_in[arraylen];
    unsigned short test_data_ushort_in[arraylen];
    unsigned int test_data_uint_in[arraylen];
    long long test_data_int64_in[arraylen];
    unsigned long long test_data_uint64_in[arraylen];
#endif /* _NETCDF4 */

    /* Initialize a big blob of test data for NVAR vars. */
    for (int f = 0; f < arraylen * NVAR; f++)
    {
        test_data_byte[f] = my_rank * 1 + f;
        test_data_char[f] = my_rank * 2 + f;
        test_data_short[f] = my_rank * 5 + f;
        test_data_int[f] = my_rank * 10 + f;
        test_data_float[f] = my_rank * 10 + f + 0.5;
        test_data_double[f] = my_rank * 100000 + f + 0.5;
#ifdef _NETCDF4
        test_data_ubyte[f] = my_rank * 3 + f;
        test_data_ushort[f] = my_rank * 9 + f;
        test_data_uint[f] = my_rank * 100 + f;
        test_data_int64[f] = my_rank * 10000 + f;
        test_data_uint64[f] = my_rank * 100000 + f;
#endif /* _NETCDF4 */
    }

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        /* 1-byte types not working with pnetcdf. */
        if (flavor[fmt] == PIO_IOTYPE_PNETCDF && (pio_type == PIO_BYTE || pio_type == PIO_CHAR))
            continue;

        /* NetCDF-4 types only work with netCDF-4. */
        if (pio_type > PIO_DOUBLE && (flavor[fmt] != PIO_IOTYPE_NETCDF4C &&
                                      flavor[fmt] != PIO_IOTYPE_NETCDF4P))
            continue;

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
                case PIO_BYTE:
                    fillvalue = provide_fill ? byte_fill : NULL;
                    test_data = test_data_byte;
                    test_data_in = test_data_byte_in;
                    break;
                case PIO_CHAR:
                    fillvalue = provide_fill ? char_fill : NULL;
                    test_data = test_data_char;
                    test_data_in = test_data_char_in;
                    break;
                case PIO_SHORT:
                    fillvalue = provide_fill ? short_fill : NULL;
                    test_data = test_data_short;
                    test_data_in = test_data_short_in;
                    break;
                case PIO_INT:
                    fillvalue = provide_fill ? int_fill : NULL;
                    test_data = test_data_int;
                    test_data_in = test_data_int_in;
                    break;
                case PIO_FLOAT:
                    fillvalue = provide_fill ? float_fill : NULL;
                    test_data = test_data_float;
                    test_data_in = test_data_float_in;
                    break;
                case PIO_DOUBLE:
                    fillvalue = provide_fill ? double_fill : NULL;
                    test_data = test_data_double;
                    test_data_in = test_data_double_in;
                    break;
#ifdef _NETCDF4
                case PIO_UBYTE:
                    fillvalue = provide_fill ? ubyte_fill : NULL;
                    test_data = test_data_ubyte;
                    test_data_in = test_data_ubyte_in;
                    break;
                case PIO_USHORT:
                    fillvalue = provide_fill ? ushort_fill : NULL;
                    test_data = test_data_ushort;
                    test_data_in = test_data_ushort_in;
                    break;
                case PIO_UINT:
                    fillvalue = provide_fill ? uint_fill : NULL;
                    test_data = test_data_uint;
                    test_data_in = test_data_uint_in;
                    break;
                case PIO_INT64:
                    fillvalue = provide_fill ? int64_fill : NULL;
                    test_data = test_data_int64;
                    test_data_in = test_data_int64_in;
                    break;
                case PIO_UINT64:
                    fillvalue = provide_fill ? uint64_fill : NULL;
                    test_data = test_data_uint64;
                    test_data_in = test_data_uint64_in;
                    break;
#endif /* _NETCDF4 */
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
                for (int v = 0; v < NVAR; v++)
                    if ((ret = PIOc_def_var(ncid, var_name[v], pio_type, NDIM, dimids, &varid[v])))
                        ERR(ret);

                /* Define a variable of a different type, to test error handling. */
                int other_pio_type = pio_type < 5 ? pio_type + 1 : PIO_INT;
                if ((ret = PIOc_def_var(ncid, "OTHER_VAR", other_pio_type, NDIM, dimids, &other_varid)))
                    ERR(ret);

                /* Leave a note. */
                if ((ret = PIOc_put_att_text(ncid, NC_GLOBAL, NOTE_NAME, strlen(NOTE), NOTE)))
                    ERR(ret);
                int num_stooges = TOTAL_NUMBER_OF_STOOGES;
                if ((ret = PIOc_put_att_int(ncid, NC_GLOBAL, TOTAL_NUMBER_OF_STOOGES_NAME, PIO_INT, 1, &num_stooges)))
                    ERR(ret);

                /* End define mode. */
                if ((ret = PIOc_enddef(ncid)))
                    ERR(ret);

                /* Set the value of the record dimension. */
                if ((ret = PIOc_setframe(ncid, varid[0], 0)))
                    ERR(ret);

                int frame[NVAR] = {0, 0, 0};
                int flushtodisk = test_multi;

                /* This will not work, because we mix var types. */
                wrong_varid[0] = varid[0];
                wrong_varid[1] = varid[1];
                wrong_varid[0] = other_varid;
                if (PIOc_write_darray_multi(ncid, wrong_varid, ioid, NVAR, arraylen, test_data, frame,
                                            fillvalue, flushtodisk) != PIO_EINVAL)
                    ERR(ERR_WRONG);

                /* Write the data with the _multi function. */
                if ((ret = PIOc_write_darray_multi(ncid, varid, ioid, NVAR, arraylen, test_data, frame,
                                                   fillvalue, flushtodisk)))
                    ERR(ret);

                /* Close the netCDF file. */
                if ((ret = PIOc_closefile(ncid)))
                    ERR(ret);

                /* Reopen the file. */
                if ((ret = PIOc_openfile(iosysid, &ncid2, &flavor[fmt], filename, PIO_NOWRITE)))
                    ERR(ret);

                /* Now use read_darray on each var in turn and make
                 * sure we get correct data. */
                for (int v = 0; v < NVAR; v++)
                {
                    /* Set the value of the record dimension. */
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
                            if (test_data_byte_in[f] != test_data_byte[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_CHAR:
                            if (test_data_char_in[f] != test_data_char[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_SHORT:
                            if (test_data_short_in[f] != test_data_short[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_INT:
                            if (test_data_int_in[f] != test_data_int[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_FLOAT:
                            if (test_data_float_in[f] != test_data_float[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_DOUBLE:
                            if (test_data_double_in[f] != test_data_double[f + arraylen * v])
                                return ERR_WRONG;
                            break;
#ifdef _NETCDF4
                        case PIO_UBYTE:
                            if (test_data_ubyte_in[f] != test_data_ubyte[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_USHORT:
                            if (test_data_ushort_in[f] != test_data_ushort[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_UINT:
                            if (test_data_uint_in[f] != test_data_uint[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_INT64:
                            if (test_data_int64_in[f] != test_data_int64[f + arraylen * v])
                                return ERR_WRONG;
                            break;
                        case PIO_UINT64:
                            if (test_data_uint64_in[f] != test_data_uint64[f + arraylen * v])
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
#ifdef _NETCDF4
#define NUM_TYPES_TO_TEST 11
    int pio_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE,
                                       PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
#define NUM_TYPES_TO_TEST 6
    int pio_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE};
#endif /* _NETCDF4 */
    int ioid;
    char filename[NC_MAX_NAME + 1];
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
