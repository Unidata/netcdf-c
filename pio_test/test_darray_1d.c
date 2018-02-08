/*
 * Tests for PIO distributed arrays. This test uses 1 dimension,
 * everything very simple. ;-)
 *
 * @author Ed Hartnett
 * @date 2/27/17
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
#define TEST_NAME "test_darray_1d"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* The number of dimensions in the example data. In this test, we
 * are using three-dimensional data. */
#define NDIM 1

/* The length of our sample data along the dimension. */
#define DIM_LEN 8

/* This is the length of the map for each task. */
#define EXPECTED_MAPLEN 2

/* The number of timesteps of data to write. */
#define NUM_TIMESTEPS 2

/* The name of the variable in the netCDF output files. */
#define VAR_NAME "RedShirtSurvival"

/* The dimension names. */
#define DIM_NAME "episode"
#define DIM_NAME_2 "phaser_draws"

/* Create a 1D decomposition.
 *
 * @param ntasks the number of available tasks
 * @param my_rank rank of this task.
 * @param iosysid the IO system ID.
 * @param dim_len an array of length 3 with the dimension sizes.
 * @param ioid a pointer that gets the ID of this decomposition.
 * @param pio_type the type that will be used for basetype.
 * @returns 0 for success, error code otherwise.
 **/
int create_decomposition_1d(int ntasks, int my_rank, int iosysid, int pio_type, int *ioid)
{
    PIO_Offset elements_per_pe;     /* Array elements per processing unit. */
    int dim_len_1d[NDIM] = {DIM_LEN};
    int ret;

    /* How many data elements per task? In this example we will end up
     * with 2. */
    elements_per_pe = DIM_LEN / ntasks;

    PIO_Offset compdof[elements_per_pe];

    /* Don't forget to add 1! */
    compdof[0] = my_rank + 1;

    /* This means fill value will be used here. */
    compdof[1] = 0;

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_InitDecomp(iosysid, pio_type, NDIM, dim_len_1d, elements_per_pe,
                               compdof, ioid, NULL, NULL, NULL)))
        ERR(ret);

    return 0;
}

/**
 * Test fill values and darrays.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param pio_type the type of the data.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param test_comm the MPI communicator running the test.
 * @returns 0 for success, error code otherwise.
 */
int test_darray_fill(int iosysid, int ioid, int pio_type, int num_flavors, int *flavor,
                     int my_rank, MPI_Comm test_comm)
{
#define NUM_FILLVALUE_PRESENT_TESTS 2
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dimid;     /* The dimension ID. */
    int ncid;      /* The ncid of the netCDF file. */
    int varid;     /* The ID of the netCDF varable. */
    PIO_Offset arraylen = 2;
    void *test_data;
    void *fillvalue;
    void *test_data_in;
    void *expected_in;
    PIO_Offset type_size;             /* Size of the data type. */
    /* My rank as each type. */
    signed char my_byte_rank = my_rank;
    char my_char_rank = my_rank;
    short my_short_rank = my_rank;
    float my_float_rank = my_rank;
    double my_double_rank = my_rank;
#ifdef _NETCDF4
    unsigned char my_ubyte_rank = my_rank;
    unsigned short my_ushort_rank = my_rank;
    unsigned int my_uint_rank = my_rank;
    long long my_int64_rank = my_rank;
    unsigned long long my_uint64_rank = my_rank;
#endif /* _NETCDF4 */

    /* Default fill value for each type. */
    signed char byte_fill = NC_FILL_BYTE;
    char char_fill = NC_FILL_CHAR;
    short short_fill = NC_FILL_SHORT;
    int int_fill = NC_FILL_INT;
    float float_fill = NC_FILL_FLOAT;
    double double_fill = NC_FILL_DOUBLE;
#ifdef _NETCDF4
    unsigned char ubyte_fill = NC_FILL_UBYTE;
    unsigned short ushort_fill = NC_FILL_USHORT;
    unsigned int uint_fill = NC_FILL_UINT;
    long long int64_fill = NC_FILL_INT64;
    unsigned long long uint64_fill = NC_FILL_UINT64;
#endif /* _NETCDF4 */

    void *bufr;
    int ret; /* Return code. */

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

        for (int with_fillvalue = 0; with_fillvalue < NUM_FILLVALUE_PRESENT_TESTS; with_fillvalue++)
        {
            /* Create the filename. */
            sprintf(filename, "data_%s_iotype_%d_pio_type_%d_with_fillvalue_%d.nc", TEST_NAME, flavor[fmt],
                    pio_type, with_fillvalue);

            /* Create the netCDF output file. */
            if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
                ERR(ret);

            /* Turn on fill mode. */
            if ((ret = PIOc_set_fill(ncid, NC_FILL, NULL)))
                ERR(ret);

            /* Define netCDF dimensions and variable. */
            if ((ret = PIOc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)))
                ERR(ret);

            /* Define a variable. */
            if ((ret = PIOc_def_var(ncid, VAR_NAME, pio_type, NDIM, &dimid, &varid)))
                ERR(ret);

            /* End define mode. */
            if ((ret = PIOc_enddef(ncid)))
                ERR(ret);

            /* Get the size of the type. */
            if ((ret = PIOc_inq_type(ncid, pio_type, NULL, &type_size)))
                return ret;

            /* Initialize some data. */
            signed char byte_test_data[2] = {my_rank, my_rank};
            char char_test_data[2] = {my_rank, my_rank};
            short short_test_data[2] = {my_rank, my_rank};
            int int_test_data[2] = {my_rank, my_rank};
            float float_test_data[2] = {my_rank, my_rank};
            double double_test_data[2] = {my_rank, my_rank};
#ifdef _NETCDF4
            unsigned char ubyte_test_data[2] = {my_rank, my_rank};
            unsigned short ushort_test_data[2] = {my_rank, my_rank};
            unsigned int uint_test_data[2] = {my_rank, my_rank};
            long long int64_test_data[2] = {my_rank, my_rank};
            unsigned long long uint64_test_data[2] = {my_rank, my_rank};
#endif /* _NETCDF4 */

            switch (pio_type)
            {
            case PIO_BYTE:
                test_data = byte_test_data;
                fillvalue = with_fillvalue ? &byte_fill : NULL;
                expected_in = &my_byte_rank;
                break;
            case PIO_CHAR:
                test_data = char_test_data;
                fillvalue = with_fillvalue ? &char_fill : NULL;
                expected_in = &my_char_rank;
                break;
            case PIO_SHORT:
                test_data = short_test_data;
                fillvalue = with_fillvalue ? &short_fill : NULL;
                expected_in = &my_short_rank;
                break;
            case PIO_INT:
                test_data = int_test_data;
                fillvalue = with_fillvalue ? &int_fill : NULL;
                expected_in = &my_rank;
                break;
            case PIO_FLOAT:
                test_data = float_test_data;
                fillvalue = with_fillvalue ? &float_fill : NULL;
                expected_in = &my_float_rank;
                break;
            case PIO_DOUBLE:
                test_data = double_test_data;
                fillvalue = with_fillvalue ? &double_fill : NULL;
                expected_in = &my_double_rank;
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                test_data = ubyte_test_data;
                fillvalue = with_fillvalue ? &ubyte_fill : NULL;
                expected_in = &my_ubyte_rank;
                break;
            case PIO_USHORT:
                test_data = ushort_test_data;
                fillvalue = with_fillvalue ? &ushort_fill : NULL;
                expected_in = &my_ushort_rank;
                break;
            case PIO_UINT:
                test_data = uint_test_data;
                fillvalue = with_fillvalue ? &uint_fill : NULL;
                expected_in = &my_uint_rank;
                break;
            case PIO_INT64:
                test_data = int64_test_data;
                fillvalue = with_fillvalue ? &int64_fill : NULL;
                expected_in = &my_int64_rank;
                break;
            case PIO_UINT64:
                test_data = uint64_test_data;
                fillvalue = with_fillvalue ? &uint64_fill : NULL;
                expected_in = &my_uint64_rank;
                break;
#endif /* _NETCDF4 */
            default:
                return ERR_WRONG;
            }

            /* Write the data. Our test_data contains only one real
             * value (instead of 2, as indicated by arraylen), but due
             * to the decomposition, only the first value is used in
             * the output. */
            if ((ret = PIOc_write_darray(ncid, varid, ioid, arraylen, test_data, fillvalue)))
                ERR(ret);

            /* Close the netCDF file. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);

            /* Reopen the file. */
            if ((ret = PIOc_openfile(iosysid, &ncid, &flavor[fmt], filename, PIO_NOWRITE)))
                ERR(ret);

            /* Allocate space for data. */
            if (!(test_data_in = malloc(type_size * arraylen)))
                ERR(PIO_ENOMEM);

            /* Read the data. */
            if ((ret = PIOc_read_darray(ncid, varid, ioid, arraylen, test_data_in)))
                ERR(ret);

            /* Check the (first) result. */
            if (memcmp(test_data_in, expected_in, type_size))
                return ERR_WRONG;

            /* Free resources. */
            free(test_data_in);

            /* Get a buffer big enough to hold the global array. */
            if (!(bufr = malloc(DIM_LEN * type_size)))
                return PIO_ENOMEM;

            /* Get the whole array with good old get_var(). */
            if ((ret = PIOc_get_var(ncid, varid, bufr)))
                return ret;

            /* Check the results. The first four values are 0, 1, 2, 3,
             * and the rest are the default fill value of the type. */
            for (int e = 0; e < DIM_LEN; e++)
            {
                switch (pio_type)
                {
                case PIO_BYTE:
                    if (((signed char *)bufr)[e] != (e < 4 ? e : NC_FILL_BYTE))
                        return ERR_WRONG;
                    break;
                case PIO_CHAR:
                    if (((char *)bufr)[e] != (e < 4 ? e : NC_FILL_CHAR))
                        return ERR_WRONG;
                    break;
                case PIO_SHORT:
                    if (((short *)bufr)[e] != (e < 4 ? e : NC_FILL_SHORT))
                        return ERR_WRONG;
                    break;
                case PIO_INT:
                    if (((int *)bufr)[e] != (e < 4 ? e : NC_FILL_INT))
                        return ERR_WRONG;
                    break;
                case PIO_FLOAT:
                    if (((float *)bufr)[e] != (e < 4 ? e : NC_FILL_FLOAT))
                        return ERR_WRONG;
                    break;
                case PIO_DOUBLE:
                    if (((double *)bufr)[e] != (e < 4 ? e : NC_FILL_DOUBLE))
                        return ERR_WRONG;
                    break;
#ifdef _NETCDF4
                case PIO_UBYTE:
                    if (((unsigned char *)bufr)[e] != (e < 4 ? e : NC_FILL_UBYTE))
                        return ERR_WRONG;
                    break;
                case PIO_USHORT:
                    if (((unsigned short *)bufr)[e] != (e < 4 ? e : NC_FILL_USHORT))
                        return ERR_WRONG;
                    break;
                case PIO_UINT:
                    if (((unsigned int *)bufr)[e] != (e < 4 ? e : NC_FILL_UINT))
                        return ERR_WRONG;
                    break;
                case PIO_INT64:
                    if (((long long *)bufr)[e] != (e < 4 ? e : NC_FILL_INT64))
                        return ERR_WRONG;
                    break;
                case PIO_UINT64:
                    if (((unsigned long long *)bufr)[e] != (e < 4 ? e : NC_FILL_UINT64))
                        return ERR_WRONG;
                    break;
#endif /* _NETCDF4 */
                default:
                    return ERR_WRONG;
                }
            }

            /* Release buffer. */
            free(bufr);

            /* Close the netCDF file. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);
        } /* with_fillvalue */
    } /* next iotype */

    return PIO_NOERR;
}

/**
 * Test fill values and darrays with an unlimited dim.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param pio_type the type of the data.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param test_comm the MPI communicator running the test.
 * @returns 0 for success, error code otherwise.
 */
int test_darray_fill_unlim(int iosysid, int ioid, int pio_type, int num_flavors,
                           int *flavor, int my_rank, MPI_Comm test_comm)
{
#define NDIM2 2
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dimid[NDIM2];     /* The dimension ID. */
    int ncid;      /* The ncid of the netCDF file. */
    int varid;     /* The ID of the netCDF varable. */
    PIO_Offset arraylen = 2;
    void *test_data;
    void *fillvalue;
    void *test_data_in;
    void *expected_in;
    PIO_Offset type_size;             /* Size of the data type. */

    /* My rank as each type. */
    signed char my_byte_rank = my_rank;
    char my_char_rank = my_rank;
    short my_short_rank = my_rank;
    float my_float_rank = my_rank;
    double my_double_rank = my_rank;
#ifdef _NETCDF4
    unsigned char my_ubyte_rank = my_rank;
    unsigned short my_ushort_rank = my_rank;
    unsigned int my_uint_rank = my_rank;
    long long my_int64_rank = my_rank;
    unsigned long long my_uint64_rank = my_rank;
#endif /* _NETCDF4 */

    /* Default fill value for each type. */
    signed char byte_fill = NC_FILL_BYTE;
    char char_fill = NC_FILL_CHAR;
    short short_fill = NC_FILL_SHORT;
    int int_fill = NC_FILL_INT;
    float float_fill = NC_FILL_FLOAT;
    double double_fill = NC_FILL_DOUBLE;
#ifdef _NETCDF4
    unsigned char ubyte_fill = NC_FILL_UBYTE;
    unsigned short ushort_fill = NC_FILL_USHORT;
    unsigned int uint_fill = NC_FILL_UINT;
    long long int64_fill = NC_FILL_INT64;
    unsigned long long uint64_fill = NC_FILL_UINT64;
#endif /* _NETCDF4 */
    void *bufr;
    int ret;                        /* Return code. */

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
        sprintf(filename, "data_%s_iotype_%d_pio_type_%d_unlim.nc", TEST_NAME, flavor[fmt],
                pio_type);

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
            ERR(ret);

        /* Turn on fill mode. */
        if ((ret = PIOc_set_fill(ncid, NC_FILL, NULL)))
            ERR(ret);

        /* Define netCDF dimensions. */
        if ((ret = PIOc_def_dim(ncid, DIM_NAME, NC_UNLIMITED, &dimid[0])))
            ERR(ret);
        if ((ret = PIOc_def_dim(ncid, DIM_NAME_2, DIM_LEN, &dimid[1])))
            ERR(ret);

        /* Define a variable. */
        if ((ret = PIOc_def_var(ncid, VAR_NAME, pio_type, NDIM2, dimid, &varid)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Get the size of the type. */
        if ((ret = PIOc_inq_type(ncid, pio_type, NULL, &type_size)))
            return ret;

        /* Initialize some data. */
        signed char byte_test_data[2] = {my_rank, my_rank};
        char char_test_data[2] = {my_rank, my_rank};
        short short_test_data[2] = {my_rank, my_rank};
        int int_test_data[2] = {my_rank, my_rank};
        float float_test_data[2] = {my_rank, my_rank};
        double double_test_data[2] = {my_rank, my_rank};
#ifdef _NETCDF4
        unsigned char ubyte_test_data[2] = {my_rank, my_rank};
        unsigned short ushort_test_data[2] = {my_rank, my_rank};
        unsigned int uint_test_data[2] = {my_rank, my_rank};
        long long int64_test_data[2] = {my_rank, my_rank};
        unsigned long long uint64_test_data[2] = {my_rank, my_rank};
#endif /* _NETCDF4 */
        switch (pio_type)
        {
        case PIO_BYTE:
            test_data = byte_test_data;
            fillvalue = &byte_fill;
            expected_in = &my_byte_rank;
            break;
        case PIO_CHAR:
            test_data = char_test_data;
            fillvalue = &char_fill;
            expected_in = &my_char_rank;
            break;
        case PIO_SHORT:
            test_data = short_test_data;
            fillvalue = &short_fill;
            expected_in = &my_short_rank;
            break;
        case PIO_INT:
            test_data = int_test_data;
            fillvalue = &int_fill;
            expected_in = &my_rank;
            break;
        case PIO_FLOAT:
            test_data = float_test_data;
            fillvalue = &float_fill;
            expected_in = &my_float_rank;
            break;
        case PIO_DOUBLE:
            test_data = double_test_data;
            fillvalue = &double_fill;
            expected_in = &my_double_rank;
            break;
#ifdef _NETCDF4
        case PIO_UBYTE:
            test_data = ubyte_test_data;
            fillvalue = &ubyte_fill;
            expected_in = &my_ubyte_rank;
            break;
        case PIO_USHORT:
            test_data = ushort_test_data;
            fillvalue = &ushort_fill;
            expected_in = &my_ushort_rank;
            break;
        case PIO_UINT:
            test_data = uint_test_data;
            fillvalue = &uint_fill;
            expected_in = &my_uint_rank;
            break;
        case PIO_INT64:
            test_data = int64_test_data;
            fillvalue = &int64_fill;
            expected_in = &my_int64_rank;
            break;
        case PIO_UINT64:
            test_data = uint64_test_data;
            fillvalue = &uint64_fill;
            expected_in = &my_uint64_rank;
            break;
#endif /* _NETCDF4 */
        default:
            return ERR_WRONG;
        }

        /* Set the record number for the unlimited dimension. */
        if ((ret = PIOc_setframe(ncid, varid, 0)))
            ERR(ret);

        /* Write the data. Our test_data contains only one real value
         * (instead of 2, as indicated by arraylen), but due to the
         * decomposition, only the first value is used in the
         * output. */
        if ((ret = PIOc_write_darray(ncid, varid, ioid, arraylen, test_data, fillvalue)))
            ERR(ret);

        /* Set the record number for the unlimited dimension. */
        if ((ret = PIOc_setframe(ncid, varid, 1)))
            ERR(ret);

        /* Write the data. Our test_data contains only one real value
         * (instead of 2, as indicated by arraylen), but due to the
         * decomposition, only the first value is used in the
         * output. */
        if ((ret = PIOc_write_darray(ncid, varid, ioid, arraylen, test_data, fillvalue)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Reopen the file. */
        if ((ret = PIOc_openfile(iosysid, &ncid, &flavor[fmt], filename, PIO_NOWRITE)))
            ERR(ret);

        /* Allocate space for data. */
        if (!(test_data_in = malloc(type_size * arraylen)))
            ERR(PIO_ENOMEM);

        /* Set the record number for the unlimited dimension. */
        if ((ret = PIOc_setframe(ncid, varid, 0)))
            ERR(ret);

        /* Read the data. */
        if ((ret = PIOc_read_darray(ncid, varid, ioid, arraylen, test_data_in)))
            ERR(ret);

        /* Check the (first) result. */
        if (memcmp(test_data_in, expected_in, type_size))
            return ERR_WRONG;

        /* Free resources. */
        free(test_data_in);

        /* Get a buffer big enough to hold the global array. */
        if (!(bufr = malloc(DIM_LEN * type_size * 2)))
            return PIO_ENOMEM;

        /* Get the whole array with good old get_var(). */
        if ((ret = PIOc_get_var(ncid, varid, bufr)))
            return ret;

        /* Check the results. The first four values in each record are
         * 0, 1, 2, 3, and the rest are the default fill value of the
         * type. There are two records. */
        for (int e = 0; e < DIM_LEN * 2; e++)
        {
            switch (pio_type)
            {
            case PIO_BYTE:
                if (((signed char *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_BYTE))
                    return ERR_WRONG;
                break;
            case PIO_CHAR:
                if (((char *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_CHAR))
                    return ERR_WRONG;
                break;
            case PIO_SHORT:
                if (((short *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_SHORT))
                    return ERR_WRONG;
                break;
            case PIO_INT:
                if (((int *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_INT))
                    return ERR_WRONG;
                break;
            case PIO_FLOAT:
                if (((float *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_FLOAT))
                    return ERR_WRONG;
                break;
            case PIO_DOUBLE:
                if (((double *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_DOUBLE))
                    return ERR_WRONG;
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                if (((unsigned char *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_UBYTE))
                    return ERR_WRONG;
                break;
            case PIO_USHORT:
                if (((unsigned short *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_USHORT))
                    return ERR_WRONG;
                break;
            case PIO_UINT:
                if (((unsigned int *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_UINT))
                    return ERR_WRONG;
                break;
            case PIO_INT64:
                if (((long long *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_INT64))
                    return ERR_WRONG;
                break;
            case PIO_UINT64:
                if (((unsigned long long *)bufr)[e] != (e % 8 < 4 ? e % 8 : NC_FILL_UINT64))
                    return ERR_WRONG;
                break;
#endif /* _NETCDF4 */
            default:
                return ERR_WRONG;
            }
        }

        /* Release buffer. */
        free(bufr);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);
    } /* next iotype */

    return PIO_NOERR;
}

/**
 * Test the decomp read/write functionality.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param pio_type the type involved in this decompositon.
 * @param rearranger the rearranger in use.
 * @param test_comm the MPI communicator for this test.
 * @returns 0 for success, error code otherwise.
 */
int test_decomp_read_write(int iosysid, int ioid, int num_flavors, int *flavor, int my_rank,
                           int pio_type, int rearranger, MPI_Comm test_comm)
{
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int ioid2;             /* ID for decomposition we will create from file. */
    char title_in[PIO_MAX_NAME + 1];   /* Optional title. */
    char history_in[PIO_MAX_NAME + 1]; /* Optional history. */
    int fortran_order_in; /* Indicates fortran vs. c order. */
    int ret;              /* Return code. */

    /* Use PIO to create the decomp file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        /* Create the filename. */
        sprintf(filename, "decomp_%s_iotype_%d.nc", TEST_NAME, flavor[fmt]);

        if ((ret = PIOc_write_nc_decomp(iosysid, filename, 0, ioid, NULL, NULL, 0)))
            return ret;

        /* Read the data. */
        if ((ret = PIOc_read_nc_decomp(iosysid, filename, &ioid2, test_comm, pio_type,
                                       title_in, history_in, &fortran_order_in)))
            return ret;

        /* Check the results. */
        {
            iosystem_desc_t *ios;
            io_desc_t *iodesc;
            MPI_Datatype expected_basetype;

            switch (pio_type)
            {
            case PIO_BYTE:
                expected_basetype = MPI_BYTE;
                break;
            case PIO_CHAR:
                expected_basetype = MPI_CHAR;
                break;
            case PIO_SHORT:
                expected_basetype = MPI_SHORT;
                break;
            case PIO_INT:
                expected_basetype = MPI_INT;
                break;
            case PIO_FLOAT:
                expected_basetype = MPI_FLOAT;
                break;
            case PIO_DOUBLE:
                expected_basetype = MPI_DOUBLE;
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                expected_basetype = MPI_UNSIGNED_CHAR;
                break;
            case PIO_USHORT:
                expected_basetype = MPI_UNSIGNED_SHORT;
                break;
            case PIO_UINT:
                expected_basetype = MPI_UNSIGNED;
                break;
            case PIO_INT64:
                expected_basetype = MPI_LONG_LONG;
                break;
            case PIO_UINT64:
                expected_basetype = MPI_UNSIGNED_LONG_LONG;
                break;
#endif /* _NETCDF4 */
            default:
                return ERR_WRONG;
            }

            /* Get the IO system info. */
            if (!(ios = pio_get_iosystem_from_id(iosysid)))
                return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

            /* Get the IO desc, which describes the decomposition. */
            if (!(iodesc = pio_get_iodesc_from_id(ioid2)))
                return pio_err(ios, NULL, PIO_EBADID, __FILE__, __LINE__);
            if (iodesc->ioid != ioid2 || iodesc->maplen != EXPECTED_MAPLEN || iodesc->ndims != NDIM)
                return ERR_WRONG;
            /* if (iodesc->nrecvs != 1) */
            /*     return ERR_WRONG; */
            /* if (iodesc->num_aiotasks != TARGET_NTASKS) */
            /*     return ERR_WRONG; */
            if (iodesc->ndof != EXPECTED_MAPLEN)
                return ERR_WRONG;
            if (iodesc->rearranger != rearranger || iodesc->maxregions != 1)
                return ERR_WRONG;
            if (!iodesc->needsfill || iodesc->mpitype != expected_basetype)
                return ERR_WRONG;
            /* Don't forget to add 1!! */
            if (iodesc->map[0] != my_rank + 1 || iodesc->map[1] != 0)
                return ERR_WRONG;
            if (iodesc->dimlen[0] != DIM_LEN)
                return ERR_WRONG;
        }

        /* Free the PIO decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid2)))
            ERR(ret);
    }
    return PIO_NOERR;
}

/* Run tests for darray functions. */
int main(int argc, char **argv)
{
#define NUM_REARRANGERS_TO_TEST 2
    int rearranger[NUM_REARRANGERS_TO_TEST] = {PIO_REARR_BOX, PIO_REARR_SUBSET};
#ifdef _NETCDF4
#define NUM_TYPES_TO_TEST 11
    int test_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE,
                                        PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
#define NUM_TYPES_TO_TEST 6
    int test_type[NUM_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE};
#endif /* _NETCDF4 */
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
        int ioid;     /* Decomposition ID. */
        int ioproc_stride = 1;    /* Stride in the mpi rank between io tasks. */
        int ioproc_start = 0;     /* Rank of first processor to be used for I/O. */
        int ret;      /* Return code. */

        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        for (int r = 0; r < NUM_REARRANGERS_TO_TEST; r++)
        {
            /* Initialize the PIO IO system. This specifies how many and
             * which processors are involved in I/O. */
            if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, ioproc_stride,
                                           ioproc_start, rearranger[r], &iosysid)))
                return ret;

            /* Run tests for each data type. */
            for (int t = 0; t < NUM_TYPES_TO_TEST; t++)
            {
                /* Decompose the data over the tasks. */
                if ((ret = create_decomposition_1d(TARGET_NTASKS, my_rank, iosysid, test_type[t],
                                                   &ioid)))
                    return ret;

                /* Test decomposition read/write. */
                if ((ret = test_decomp_read_write(iosysid, ioid, num_flavors, flavor, my_rank,
                                                  test_type[t], rearranger[r], test_comm)))
                    return ret;

                /* Run tests. */
                if ((ret = test_darray_fill(iosysid, ioid, test_type[t], num_flavors, flavor,
                                            my_rank, test_comm)))
                    return ret;

                /* Run tests. */
                if ((ret = test_darray_fill_unlim(iosysid, ioid, test_type[t], num_flavors,
                                                  flavor, my_rank, test_comm)))
                    return ret;

                /* Free the PIO decomposition. */
                if ((ret = PIOc_freedecomp(iosysid, ioid)))
                    ERR(ret);
            }

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
