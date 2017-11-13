/*
 * This program tests darrays with async.
 *
 * @author Ed Hartnett
 * @date 5/4/17
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
#define TEST_NAME "test_darray_async"

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

char dim_name[NDIM3][PIO_MAX_NAME + 1] = {"unlim", "lat", "lon"};

/* Length of the dimension. */
#define LEN3 3

#define NUM_VAR_SETS 2

/* Check the file that was created in this test. */
int check_darray_file(int iosysid, char *data_filename, int iotype, int my_rank,
                      int piotype)
{
    int ncid;
    int varid[NVAR] = {0, 1, 2, 3};
    void *data_in;
    void *data_in_norec;
    PIO_Offset type_size;
    int ret;

    /* Reopen the file. */
    if ((ret = PIOc_openfile(iosysid, &ncid, &iotype, data_filename, NC_NOWRITE)))
        ERR(ret);

    /* Get the size of the type. */
    if ((ret = PIOc_inq_type(ncid, piotype, NULL, &type_size)))
        ERR(ret);

    /* Allocate memory to read data. */
    if (!(data_in = malloc(LAT_LEN * LON_LEN * type_size * NREC)))
        ERR(PIO_ENOMEM);
    if (!(data_in_norec = malloc(LAT_LEN * LON_LEN * type_size)))
        ERR(PIO_ENOMEM);

    /* We have two sets of variables, those with unlimted, and those
     * without unlimited dimension. */
    for (int vs = 0; vs < NUM_VAR_SETS; vs++)
    {
        int rec_varid = vs ? varid[0] : varid[1];
        int norec_varid = vs ? varid[2] : varid[3];

        /* Read the record data. The values we expect are: 10, 11, 20, 21, 30,
         * 31, in each of three records. */
        if ((ret = PIOc_get_var(ncid, rec_varid, data_in)))
            ERR(ret);

        /* Read the non-record data. The values we expect are: 10, 11, 20, 21, 30,
         * 31. */
        if ((ret = PIOc_get_var(ncid, norec_varid, data_in_norec)))
            ERR(ret);

        /* Check the results. */
        for (int r = 0; r < LAT_LEN * LON_LEN * NREC; r++)
        {
            int tmp_r = r % (LAT_LEN * LON_LEN);
            switch (piotype)
            {
            case PIO_BYTE:
                if (((signed char *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_CHAR:
                if (((char *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_SHORT:
                if (((short *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_INT:
                if (((int *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_FLOAT:
                if (((float *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_DOUBLE:
                if (((double *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                if (((unsigned char *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_USHORT:
                if (((unsigned short *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_UINT:
                if (((unsigned int *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_INT64:
                if (((long long *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
            case PIO_UINT64:
                if (((unsigned long long *)data_in)[r] != (tmp_r/2 + 1) * 10.0 + tmp_r % 2)
                    ERR(ret);
                break;
#endif /* _NETCDF4 */
            default:
                ERR(ERR_WRONG);
            }
        }

        /* Check the results. */
        for (int r = 0; r < LAT_LEN * LON_LEN; r++)
        {
            switch (piotype)
            {
            case PIO_BYTE:
                if (((signed char *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_CHAR:
                if (((char *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_SHORT:
                if (((short *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_INT:
                if (((int *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_FLOAT:
                if (((float *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_DOUBLE:
                if (((double *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                if (((unsigned char *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_USHORT:
                if (((unsigned short *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_UINT:
                if (((unsigned int *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_INT64:
                if (((long long *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
            case PIO_UINT64:
                if (((unsigned long long *)data_in_norec)[r] != (r/2 + 1) * 20.0 + r%2)
                    ERR(ret);
                break;
#endif /* _NETCDF4 */
            default:
                ERR(ERR_WRONG);
            }
        }
    } /* next var set */

    /* Free resources. */
    free(data_in);
    free(data_in_norec);

    /* Close the file. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);

    return 0;
}

/* Run a simple test using darrays with async. */
int run_darray_async_test(int iosysid, int my_rank, MPI_Comm test_comm, MPI_Comm comp_comm,
                          int num_flavors, int *flavor, int piotype)
{
    int ioid;
    int dim_len[NDIM3] = {NC_UNLIMITED, 2, 3};
    PIO_Offset elements_per_pe = LAT_LEN;
    PIO_Offset compdof[LAT_LEN] = {my_rank * 2 - 2, my_rank * 2 - 1};
    char decomp_filename[PIO_MAX_NAME + 1];
    int ret;

    sprintf(decomp_filename, "decomp_rdat_%s_.nc", TEST_NAME);

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_init_decomp(iosysid, piotype, NDIM2, &dim_len[1], elements_per_pe,
                                compdof, &ioid, PIO_REARR_BOX, NULL, NULL)))
        ERR(ret);

    /* Write the decomp file (on appropriate tasks). */
    if ((ret = PIOc_write_nc_decomp(iosysid, decomp_filename, 0, ioid, NULL, NULL, 0)))
        return ret;

    int fortran_order;
    int ioid2;
    if ((ret = PIOc_read_nc_decomp(iosysid, decomp_filename, &ioid2, comp_comm,
                                   PIO_INT, NULL, NULL, &fortran_order)))
        return ret;

    /* Free the decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid2)))
        ERR(ret);

    /* Test each available iotype. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        int ncid;
        PIO_Offset type_size;
        int dimid[NDIM3];
        int varid[NVAR];
        char data_filename[PIO_MAX_NAME + 1];
        void *my_data;
        void *my_data_multi;
        void *my_data_norec;
        signed char my_data_byte[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        char my_data_char[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        short my_data_short[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        int my_data_int[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        float my_data_float[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        double my_data_double[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
#ifdef _NETCDF4
        unsigned char my_data_ubyte[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        unsigned short my_data_ushort[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        unsigned int my_data_uint[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        long long my_data_int64[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
        unsigned long long my_data_uint64[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
#endif /* _NETCDF4 */
        signed char my_data_byte_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        char my_data_char_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        short my_data_short_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        int my_data_int_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        float my_data_float_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        double my_data_double_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
#ifdef _NETCDF4
        unsigned char my_data_ubyte_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        unsigned short my_data_ushort_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        unsigned int my_data_uint_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        long long my_data_int64_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
        unsigned long long my_data_uint64_norec[LAT_LEN] = {my_rank * 20, my_rank * 20 + 1};
#endif /* _NETCDF4 */

        /* Only netCDF-4 can handle extended types. */
        if (piotype > PIO_DOUBLE && flavor[fmt] != PIO_IOTYPE_NETCDF4C && flavor[fmt] != PIO_IOTYPE_NETCDF4P)
            continue;

        /* BYTE and CHAR don't work with pnetcdf. Don't know why yet. */
        if (flavor[fmt] == PIO_IOTYPE_PNETCDF && (piotype == PIO_BYTE || piotype == PIO_CHAR))
            continue;

        /* Select the correct data to write, depending on type. */
        switch (piotype)
        {
        case PIO_BYTE:
            my_data = my_data_byte;
            my_data_norec = my_data_byte_norec;
            break;
        case PIO_CHAR:
            my_data = my_data_char;
            my_data_norec = my_data_char_norec;
            break;
        case PIO_SHORT:
            my_data = my_data_short;
            my_data_norec = my_data_short_norec;
            break;
        case PIO_INT:
            my_data = my_data_int;
            my_data_norec = my_data_int_norec;
            break;
        case PIO_FLOAT:
            my_data = my_data_float;
            my_data_norec = my_data_float_norec;
            break;
        case PIO_DOUBLE:
            my_data = my_data_double;
            my_data_norec = my_data_double_norec;
            break;
#ifdef _NETCDF4
        case PIO_UBYTE:
            my_data = my_data_ubyte;
            my_data_norec = my_data_ubyte_norec;
            break;
        case PIO_USHORT:
            my_data = my_data_ushort;
            my_data_norec = my_data_ushort_norec;
            break;
        case PIO_UINT:
            my_data = my_data_uint;
            my_data_norec = my_data_uint_norec;
            break;
        case PIO_INT64:
            my_data = my_data_int64;
            my_data_norec = my_data_int64_norec;
            break;
        case PIO_UINT64:
            my_data = my_data_uint64;
            my_data_norec = my_data_uint64_norec;
            break;
#endif /* _NETCDF4 */
        default:
            ERR(ERR_WRONG);
        }

        /* Create sample output file. */
        sprintf(data_filename, "data_%s_iotype_%d_piotype_%d.nc", TEST_NAME, flavor[fmt],
                piotype);
        if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], data_filename,
                                   NC_CLOBBER)))
            ERR(ret);

        /* Find the size of the type. */
        if ((ret = PIOc_inq_type(ncid, piotype, NULL, &type_size)))
            ERR(ret);

        /* Create the data for the darray_multi call by making two
         * copies of the data. */
        if (!(my_data_multi = malloc(2 * type_size * elements_per_pe)))
            ERR(PIO_ENOMEM);
        memcpy(my_data_multi, my_data, type_size * elements_per_pe);
        memcpy((char *)my_data_multi + type_size * elements_per_pe, my_data, type_size * elements_per_pe);

        /* Define dimensions. */
        for (int d = 0; d < NDIM3; d++)
            if ((ret = PIOc_def_dim(ncid, dim_name[d], dim_len[d], &dimid[d])))
                ERR(ret);

        /* Define variables. */
        if ((ret = PIOc_def_var(ncid, REC_VAR_NAME, piotype, NDIM3, dimid, &varid[0])))
            ERR(ret);
        if ((ret = PIOc_def_var(ncid, REC_VAR_NAME2, piotype, NDIM3, dimid, &varid[1])))
            ERR(ret);
        if ((ret = PIOc_def_var(ncid, NOREC_VAR_NAME, piotype, NDIM2, &dimid[1],
                                &varid[2])))
            ERR(ret);
        if ((ret = PIOc_def_var(ncid, NOREC_VAR_NAME2, piotype, NDIM2, &dimid[1],
                                &varid[3])))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Set the record number for the record vars. */
        if ((ret = PIOc_setframe(ncid, varid[0], 0)))
            ERR(ret);
        if ((ret = PIOc_setframe(ncid, varid[1], 0)))
            ERR(ret);

        /* Write some data to the record vars. */
        if ((ret = PIOc_write_darray(ncid, varid[0], ioid, elements_per_pe, my_data, NULL)))
            ERR(ret);
        if ((ret = PIOc_write_darray(ncid, varid[1], ioid, elements_per_pe, my_data, NULL)))
            ERR(ret);

        /* Write some data to the non-record vars. */
        if ((ret = PIOc_write_darray(ncid, varid[2], ioid, elements_per_pe, my_data_norec, NULL)))
            ERR(ret);
        if ((ret = PIOc_write_darray(ncid, varid[3], ioid, elements_per_pe, my_data_norec, NULL)))
            ERR(ret);

        /* Sync the file. */
        if ((ret = PIOc_sync(ncid)))
            ERR(ret);

        /* Increment the record number for the record vars. */
        if ((ret = PIOc_advanceframe(ncid, varid[0])))
            ERR(ret);
        if ((ret = PIOc_advanceframe(ncid, varid[1])))
            ERR(ret);

        /* Write another record. */
        if ((ret = PIOc_write_darray(ncid, varid[0], ioid, elements_per_pe, my_data, NULL)))
            ERR(ret);
        if ((ret = PIOc_write_darray(ncid, varid[1], ioid, elements_per_pe, my_data, NULL)))
            ERR(ret);

        /* Sync the file. */
        if ((ret = PIOc_sync(ncid)))
            ERR(ret);

        /* Increment the record number for the record var. */
        if ((ret = PIOc_advanceframe(ncid, varid[0])))
            ERR(ret);
        if ((ret = PIOc_advanceframe(ncid, varid[1])))
            ERR(ret);

        /* Write a third record. */
        if ((ret = PIOc_write_darray(ncid, varid[0], ioid, elements_per_pe, my_data, NULL)))
            ERR(ret);
        if ((ret = PIOc_write_darray(ncid, varid[1], ioid, elements_per_pe, my_data, NULL)))
            ERR(ret);

        /* Increment the record number for the record var. */
        if ((ret = PIOc_advanceframe(ncid, varid[0])))
            ERR(ret);
        if ((ret = PIOc_advanceframe(ncid, varid[1])))
            ERR(ret);

        /* Write a forth record, using darray_multi(). */
        int frame[2] = {3, 3};
        if ((ret = PIOc_write_darray_multi(ncid, varid, ioid, 2, elements_per_pe, my_data_multi, frame, NULL, 0)))
            ERR(ret);

        /* Close the file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Free resources. */
        free(my_data_multi);

        /* Check the file for correctness. */
        if ((ret = check_darray_file(iosysid, data_filename, PIO_IOTYPE_NETCDF, my_rank, piotype)))
            ERR(ret);

    } /* next iotype */

    /* Free the decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return 0;
}

/* Run Tests for pio_spmd.c functions. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks;  /* Number of processors involved in current execution. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    MPI_Comm test_comm; /* A communicator for this test. */
#ifdef _NETCDF4
#define NUM_TYPES_TO_TEST 11
    int test_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE,
                                        PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
#define NUM_TYPES_TO_TEST 6
    int test_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE};
#endif /* _NETCDF4 */
    int ret;     /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              TARGET_NTASKS, -1, &test_comm)))
        ERR(ERR_INIT);
    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Figure out iotypes. */
    if ((ret = get_iotypes(&num_flavors, flavor)))
        ERR(ret);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        int iosysid;

        /* Initialize with task 0 as IO task, tasks 1-3 as a
         * computation component. */
#define NUM_IO_PROCS 1
#define NUM_COMPUTATION_PROCS 3
#define COMPONENT_COUNT 1
        int num_computation_procs = NUM_COMPUTATION_PROCS;
        MPI_Comm io_comm;              /* Will get a duplicate of IO communicator. */
        MPI_Comm comp_comm[COMPONENT_COUNT]; /* Will get duplicates of computation communicators. */
        int mpierr;

        /* Run the test for each data type. */
        for (int t = 0; t < NUM_TYPES_TO_TEST; t++)
        {
            if ((ret = PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                                       &num_computation_procs, NULL, &io_comm, comp_comm,
                                       PIO_REARR_BOX, &iosysid)))
                ERR(ERR_INIT);

            /* This code runs only on computation components. */
            if (my_rank)
            {
                /* Run the simple darray async test. */
                if ((ret = run_darray_async_test(iosysid, my_rank, test_comm, comp_comm[0], num_flavors,
                                                 flavor, test_type[t])))
                    return ret;

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
        } /* next type */
    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
