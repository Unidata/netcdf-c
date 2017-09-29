/*
 * This program tests darrays with async and non-async.
 *
 * @author Ed Hartnett
 * @date 7/8/17
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>
#include <pio_internal.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 1

/* The name of this test. */
#define TEST_NAME "test_darray_2sync"

#define NUM_IO_PROCS 1
#define NUM_COMPUTATION_PROCS 3
#define COMPONENT_COUNT 1

#define DIM_NAME "simple_dim"
#define DIM_LEN 6
#define VAR_NAME "simple_var"
#define NDIM1 1

/* Declare and fill and array with all PIO types available. */
#ifdef _NETCDF4
#define MAX_NUM_TYPES 11
int test_type[MAX_NUM_TYPES] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE,
                                    PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
#define MAX_NUM_TYPES 6
int test_type[MAX_NUM_TYPES] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE};
#endif /* _NETCDF4 */

#define LEN2 2

/* Tests and incorrect fill values are rejected. */
int darray_fill_test(int iosysid, int my_rank, int num_iotypes, int *iotype,
                     int async)
{
    /* For each of the available IOtypes... */
    for (int iot = 0; iot < num_iotypes; iot++)
    {
        /* Test more types for netCDF4 IOTYPES. */
        int num_types_to_test = (iotype[iot] == PIO_IOTYPE_NETCDF4C ||
                                 iotype[iot] == PIO_IOTYPE_NETCDF4P) ? MAX_NUM_TYPES : NUM_CLASSIC_TYPES;

        /* Test all available types. */
        for (int t = 0; t < num_types_to_test; t++)
        {
            int ncid;
            int dimid;
            int varid;
            int ioid;
            char filename[PIO_MAX_NAME + 1];
            /* The default fill values. */
            signed char default_fill_byte = PIO_FILL_BYTE;
            unsigned char default_fill_char = PIO_FILL_CHAR;
            short default_fill_short = PIO_FILL_SHORT;
            int default_fill_int = PIO_FILL_INT;
            float default_fill_float = PIO_FILL_FLOAT;
            double default_fill_double = PIO_FILL_DOUBLE;
#ifdef _NETCDF4
            unsigned char default_fill_ubyte = PIO_FILL_UBYTE;
            unsigned short default_fill_ushort = PIO_FILL_USHORT;
            unsigned int default_fill_uint = PIO_FILL_UINT;
            long long default_fill_int64 = PIO_FILL_INT64;
            unsigned long long default_fill_uint64 = PIO_FILL_UINT64;
#endif /* _NETCDF4 */
            
            /* Some incorrect fill values. */
            signed char wrong_fill_byte = TEST_VAL_42;
            unsigned char wrong_fill_char = TEST_VAL_42;
            short wrong_fill_short = TEST_VAL_42;
            int wrong_fill_int = TEST_VAL_42;
            float wrong_fill_float = TEST_VAL_42;
            double wrong_fill_double = TEST_VAL_42;
#ifdef _NETCDF4
            unsigned char wrong_fill_ubyte = TEST_VAL_42;
            unsigned short wrong_fill_ushort = TEST_VAL_42;
            unsigned int wrong_fill_uint = TEST_VAL_42;
            long long wrong_fill_int64 = TEST_VAL_42;
            unsigned long long wrong_fill_uint64 = TEST_VAL_42;
#endif /* _NETCDF4 */

            /* Some sample data. */
            signed char test_data_byte[LEN2] = {my_rank, -my_rank};
            unsigned char test_data_char[LEN2] = {my_rank, my_rank};
            short test_data_short[LEN2] = {my_rank, -my_rank};
            int test_data_int[LEN2] = {my_rank, -my_rank};
            float test_data_float[LEN2] = {my_rank, -my_rank};
            double test_data_double[LEN2] = {my_rank, -my_rank};
#ifdef _NETCDF4
            unsigned char test_data_ubyte[LEN2] = {my_rank, my_rank};
            unsigned short test_data_ushort[LEN2] = {my_rank, my_rank};
            unsigned int test_data_uint[LEN2] = {my_rank, my_rank};
            long long test_data_int64[LEN2] = {my_rank, -my_rank};
            unsigned long long test_data_uint64[LEN2] = {my_rank, my_rank};
#endif /* _NETCDF4 */
            void *test_data;
            void *default_fillvalue;
            void *wrong_fillvalue;
            int ret;

            /* For unexplained reasons, pnetcdf code can't handle these types. */
            if (iotype[iot] == PIO_IOTYPE_PNETCDF &&
                (test_type[t] == PIO_BYTE || test_type[t] == PIO_CHAR))
                continue;

            switch(test_type[t])
            {
            case PIO_BYTE:
                test_data = test_data_byte;
                default_fillvalue = &default_fill_byte;
                wrong_fillvalue = &wrong_fill_byte;
                break;
            case PIO_CHAR:
                test_data = test_data_char;
                default_fillvalue = &default_fill_char;
                wrong_fillvalue = &wrong_fill_char;
                break;
            case PIO_SHORT:
                test_data = test_data_short;
                default_fillvalue = &default_fill_short;
                wrong_fillvalue = &wrong_fill_short;
                break;
            case PIO_INT:
                test_data = test_data_int;
                default_fillvalue = &default_fill_int;
                wrong_fillvalue = &wrong_fill_int;
                break;
            case PIO_FLOAT:
                test_data = test_data_float;
                default_fillvalue = &default_fill_float;
                wrong_fillvalue = &wrong_fill_float;
                break;
            case PIO_DOUBLE:
                test_data = test_data_double;
                default_fillvalue = &default_fill_double;
                wrong_fillvalue = &wrong_fill_double;
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                test_data = test_data_ubyte;
                default_fillvalue = &default_fill_ubyte;
                wrong_fillvalue = &wrong_fill_ubyte;
                break;
            case PIO_USHORT:
                test_data = test_data_ushort;
                default_fillvalue = &default_fill_ushort;
                wrong_fillvalue = &wrong_fill_ushort;
                break;
            case PIO_UINT:
                test_data = test_data_uint;
                default_fillvalue = &default_fill_uint;
                wrong_fillvalue = &wrong_fill_uint;
                break;
            case PIO_INT64:
                test_data = test_data_int64;
                default_fillvalue = &default_fill_int64;
                wrong_fillvalue = &wrong_fill_int64;
                break;
            case PIO_UINT64:
                test_data = test_data_uint64;
                default_fillvalue = &default_fill_uint64;
                wrong_fillvalue = &wrong_fill_uint64;
                break;
#endif /* _NETCDF4 */
            }

            /* Create test filename. */
            sprintf(filename, "%s_fill_async_%d_iotype_%d_type_%d.nc", TEST_NAME, async, iotype[iot],
                    test_type[t]);

            /* Create the test file. */
            if ((ret = PIOc_createfile(iosysid, &ncid, &iotype[iot], filename, PIO_CLOBBER)))
                ERR(ret);

            /* Define a dimension. */
            if ((ret = PIOc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)))
                ERR(ret);

            /* Define a 1D var. */
            if ((ret = PIOc_def_var(ncid, VAR_NAME, test_type[t], NDIM1, &dimid, &varid)))
                ERR(ret);

            /* End define mode. */
            if ((ret = PIOc_enddef(ncid)))
                ERR(ret);

            /* Create the PIO decomposition for this test. */
            int elements_per_pe = LEN2;
            PIO_Offset compdof[elements_per_pe];
            int gdimlen = DIM_LEN;
            if (my_rank == 0)
            {
                /* Only non-async code will reach here, for async, task 0
                 * does not run this function. */
                compdof[0] = -1;
                compdof[1] = -1;
            }
            else
            {
                compdof[0] = (my_rank - 1) * elements_per_pe;
                compdof[1] = compdof[0] + 1;
            }

            /* Initialize the decomposition. Only the subset
             * decomposition uses the fill value. */
            if ((ret = PIOc_init_decomp(iosysid, test_type[t], NDIM1, &gdimlen, elements_per_pe,
                                        compdof, &ioid, PIO_REARR_BOX, NULL, NULL)))
                ERR(ret);

            /* Set the record number for the unlimited dimension. */
            if ((ret = PIOc_setframe(ncid, varid, 0)))
                ERR(ret);

            /* This should not work, because fill value is
             * incorrect. (Test turned off until Fortran API/tests are
             * fixed.) */
            if (PIOc_write_darray(ncid, varid, ioid, LEN2, test_data, wrong_fillvalue) != PIO_EINVAL)
                ERR(ERR_WRONG);
            
            /* Write the data. There are 3 procs with data, each writes 2
             * values. */
            if ((ret = PIOc_write_darray(ncid, varid, ioid, LEN2, test_data, default_fillvalue)))
                ERR(ret);

            /* Close the test file. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);

            /* Free decomposition. */
            if ((ret = PIOc_freedecomp(iosysid, ioid)))
                ERR(ret);

            /* Check the file. */
            {
                int ncid2;

                /* Reopen the file. */
                if ((ret = PIOc_openfile2(iosysid, &ncid2, &iotype[iot], filename, PIO_NOWRITE)))
                    ERR(ret);

                /* Read the data. */
                switch(test_type[t])
                {
                case PIO_BYTE:
                {
                    signed char data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_schar(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_CHAR:
                    break;
                case PIO_SHORT:
                {
                    short data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_short(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_INT:
                {
                    int data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_int(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_FLOAT:
                {
                    float data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_float(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_DOUBLE:
                {
                    double data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_double(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
#ifdef _NETCDF4
                case PIO_UBYTE:
                {
                    unsigned char data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_uchar(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_USHORT:
                {
                    unsigned short data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_ushort(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_UINT:
                {
                    unsigned int data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_uint(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_INT64:
                {
                    long long data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_longlong(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
                case PIO_UINT64:
                {
                    unsigned long long data_in[elements_per_pe * NUM_COMPUTATION_PROCS];
                    if ((ret = PIOc_get_var_ulonglong(ncid2, 0, data_in)))
                        ERR(ret);
                    if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                        data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                        ERR(ret);
                }
                break;
#endif /* _NETCDF4 */
                }

                /* Close the test file. */
                if ((ret = PIOc_closefile(ncid2)))
                    ERR(ret);
            } /* finish checking file */
        } /* next type */
    } /* next iotype */

    return PIO_NOERR;
}

/* Tests for darray that can run on both async and non-async
 * iosysids. This is a deliberately simple test, to make debugging
 * easier. */
int darray_simple_test(int iosysid, int my_rank, int num_iotypes, int *iotype,
                       int async)
{
    /* For each of the available IOtypes... */
    for (int iot = 0; iot < num_iotypes; iot++)
    {
        int ncid;
        int dimid;
        int varid;
        int ioid;
        char filename[PIO_MAX_NAME + 1];
        int ret;

        /* Create test filename. */
        sprintf(filename, "%s_simple_async_%d_iotype_%d.nc", TEST_NAME, async, iotype[iot]);

        /* Create the test file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &iotype[iot], filename, PIO_CLOBBER)))
            ERR(ret);

        /* Define a dimension. */
        if ((ret = PIOc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)))
            ERR(ret);

        /* Define a 1D var. */
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_INT, NDIM1, &dimid, &varid)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Create the PIO decomposition for this test. */
        int elements_per_pe = 2;
        PIO_Offset compdof[elements_per_pe];
        int gdimlen = DIM_LEN;
        if (my_rank == 0)
        {
            /* Only non-async code will reach here, for async, task 0
             * does not run this function. */
            compdof[0] = -1;
            compdof[1] = -1;
        }
        else
        {
            compdof[0] = (my_rank - 1) * elements_per_pe;
            compdof[1] = compdof[0] + 1;
        }

        /* Initialize the decomposition. */
        if ((ret = PIOc_init_decomp(iosysid, PIO_INT, NDIM1, &gdimlen, elements_per_pe,
                                    compdof, &ioid, PIO_REARR_BOX, NULL, NULL)))
            ERR(ret);

        /* Set the record number for the unlimited dimension. */
        if ((ret = PIOc_setframe(ncid, varid, 0)))
            ERR(ret);

        /* Write the data. There are 3 procs with data, each writes 2
         * values. */
        int arraylen = 2;
        int test_data[2] = {my_rank, -my_rank};
        if ((ret = PIOc_write_darray(ncid, varid, ioid, arraylen, test_data, NULL)))
            ERR(ret);

        /* Close the test file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Free decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid)))
            ERR(ret);

        /* Check the file. */
        {
            int ncid2;
            int data_in[elements_per_pe * NUM_COMPUTATION_PROCS];

            /* Reopen the file. */
            if ((ret = PIOc_openfile2(iosysid, &ncid2, &iotype[iot], filename, PIO_NOWRITE)))
                ERR(ret);

            /* Read the data. */
            if ((ret = PIOc_get_var_int(ncid2, 0, data_in)))
                ERR(ret);
            if (my_rank && data_in[0] != 1 && data_in[1] != -1 && data_in[2] != 2 &&
                data_in[3] != -2 && data_in[4] != 3 && data_in[5] != -3)
                ERR(ret);

            /* Close the test file. */
            if ((ret = PIOc_closefile(ncid2)))
                ERR(ret);
        }
    }

    return PIO_NOERR;
}

/* This function can be run for both async and non async. It runs all
 * the test functions. */
int run_darray_tests(int iosysid, int my_rank, int num_iotypes, int *iotype, int async)
{
    int ret;

    /* Run the simple darray test. */
    if ((ret = darray_simple_test(iosysid, my_rank, num_iotypes, iotype, async)))
        ERR(ret);

    /* Run the darray fill value tests. */
    if ((ret = darray_fill_test(iosysid, my_rank, num_iotypes, iotype, async)))
        ERR(ret);

    return PIO_NOERR;
}

/* Initialize with task 0 as IO task, tasks 1-3 as a
 * computation component. */
int run_async_tests(MPI_Comm test_comm, int my_rank, int num_iotypes, int *iotype)
{
    int iosysid;
    int num_computation_procs = NUM_COMPUTATION_PROCS;
    MPI_Comm io_comm;              /* Will get a duplicate of IO communicator. */
    MPI_Comm comp_comm[COMPONENT_COUNT]; /* Will get duplicates of computation communicators. */
    int mpierr;
    int ret;

    if ((ret = PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                               &num_computation_procs, NULL, &io_comm, comp_comm,
                               PIO_REARR_BOX, &iosysid)))
        ERR(ERR_INIT);

    /* This code runs only on computation components. */
    if (my_rank)
    {
        /* Run the tests. */
        if ((ret = run_darray_tests(iosysid, my_rank, num_iotypes, iotype, 1)))
            ERR(ret);

        /* Finalize PIO system. */
        if ((ret = PIOc_finalize(iosysid)))
            return ret;

        /* Free the computation conomponent communicator. */
        if ((mpierr = MPI_Comm_free(comp_comm)))
            MPIERR(mpierr);
    }
    else
    {
        /* Free the IO communicator. */
        if ((mpierr = MPI_Comm_free(&io_comm)))
            MPIERR(mpierr);
    }

    return PIO_NOERR;
}

/* Initialize with task 0 as IO task, tasks 1-3 as a
 * computation component. */
int run_noasync_tests(MPI_Comm test_comm, int my_rank, int num_iotypes, int *iotype)
{
    int iosysid;
    int stride = 1;
    int base = 1;
    int ret;

    /* Initialize PIO system. */
    if ((ret = PIOc_Init_Intracomm(test_comm, NUM_IO_PROCS, stride, base, PIO_REARR_BOX,
                                   &iosysid)))
        ERR(ret);

    /* Run the tests. */
    if ((ret = run_darray_tests(iosysid, my_rank, num_iotypes, iotype, 0)))
        ERR(ret);

    /* Finalize PIO system. */
    if ((ret = PIOc_finalize(iosysid)))
        return ret;

    return PIO_NOERR;
}

/* Run Tests for darray functions. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks;  /* Number of processors involved in current execution. */
    int num_iotypes; /* Number of PIO netCDF iotypes in this build. */
    int iotype[NUM_IOTYPES]; /* iotypes for the supported netCDF IO iotypes. */
    MPI_Comm test_comm; /* A communicator for this test. */
    int ret;     /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              TARGET_NTASKS, -1, &test_comm)))
        ERR(ERR_INIT);
    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Figure out iotypes. */
    if ((ret = get_iotypes(&num_iotypes, iotype)))
        ERR(ret);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        if ((ret = run_async_tests(test_comm, my_rank, num_iotypes, iotype)))
            ERR(ret);

        if ((ret = run_noasync_tests(test_comm, my_rank, num_iotypes, iotype)))
            ERR(ret);

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
