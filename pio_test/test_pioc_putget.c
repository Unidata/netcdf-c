/*
 * Tests for PIO data reading and writing routines.
 *
 * @author Ed Hartnett
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
#define TEST_NAME "test_pioc_putget"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* The number of dimensions in the example data. In this test, we
 * are using three-dimensional data. */
#define NDIM 3

/* The length of our sample data along each dimension. */
#define X_DIM_LEN 40
#define Y_DIM_LEN 40

/* The number of timesteps of data to write. */
#define NUM_TIMESTEPS 1

/* The name of the variable in the netCDF output files. */
#define VAR_NAME "foo"

/* The name of the attribute in the netCDF output files. */
#define ATT_NAME "bar"

/* The meaning of life, the universe, and everything. */
#define START_DATA_VAL 42

/* Values for some netcdf-4 settings. */
#define VAR_CACHE_SIZE (1024 * 1024)
#define VAR_CACHE_NELEMS 10
#define VAR_CACHE_PREEMPTION 0.5

/* Number of NetCDF classic types. */
#define NUM_CLASSIC_TYPES 6

/* Number of NetCDF-4 types. */
#define NUM_NETCDF4_TYPES 12

/* Length of the attributes for each type. */
#define ATT_LEN 3

/* Text att (must be two chars long to fit in ATT_LEN with a NULL). */
#define TEXT_ATT_VALUE "hi"

/* Names for each type attribute. */
#define TEXT_ATT_NAME "text_att"
#define SCHAR_ATT_NAME "schar_att"
#define SHORT_ATT_NAME "short_att"
#define INT_ATT_NAME "int_att"
#define LONG_ATT_NAME "long_att"
#define FLOAT_ATT_NAME "float_att"
#define DOUBLE_ATT_NAME "double_att"
#define UCHAR_ATT_NAME "uchar_att"
#define USHORT_ATT_NAME "ushort_att"
#define UINT_ATT_NAME "uint_att"
#define INT64_ATT_NAME "int64_att"
#define UINT64_ATT_NAME "unit64_att"

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"timestep", "x", "y"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/* Length of chunksizes to use in netCDF-4 files. */
PIO_Offset chunksize[NDIM] = {2, X_DIM_LEN/2, Y_DIM_LEN/2};

/* Some sample data values to write. */
char text[] = "hi";
char char_data = 2;
signed char byte_data = -42;
short short_data = -300;
int int_data = -10000;
long int long_data = -20000;
float float_data = -42.42;
double double_data = -420000000000.5;
unsigned char ubyte_data = 43;
unsigned short ushort_data = 666;
unsigned int uint_data = 666666;
long long int64_data = -99999999999;
unsigned long long uint64_data = 99999999999;

char text_array[X_DIM_LEN][Y_DIM_LEN];
char char_array[X_DIM_LEN][Y_DIM_LEN];
signed char byte_array[X_DIM_LEN][Y_DIM_LEN];
short short_array[X_DIM_LEN][Y_DIM_LEN];
int int_array[X_DIM_LEN][Y_DIM_LEN];
long int long_array[X_DIM_LEN][Y_DIM_LEN];
float float_array[X_DIM_LEN][Y_DIM_LEN];
double double_array[X_DIM_LEN][Y_DIM_LEN];
unsigned char ubyte_array[X_DIM_LEN][Y_DIM_LEN];
unsigned short ushort_array[X_DIM_LEN][Y_DIM_LEN];
unsigned int uint_array[X_DIM_LEN][Y_DIM_LEN];
long long int64_array[X_DIM_LEN][Y_DIM_LEN];
unsigned long long uint64_array[X_DIM_LEN][Y_DIM_LEN];

#define DIM_NAME "dim"
#define NDIM1 1
#define DIM_LEN 4

/* Fill up the data arrays with some values. */
void init_arrays()
{
    for (int x = 0; x < X_DIM_LEN; x++)
    {
        strcpy(text_array[x], text);
        for (int y = 0; y < Y_DIM_LEN; y++)
        {
            char_array[x][y] = char_data;
            byte_array[x][y] = byte_data;
            short_array[x][y] = short_data;
            int_array[x][y] = int_data;
            long_array[x][y] = int_data;
            float_array[x][y] = float_data;
            double_array[x][y] = double_data;
            ubyte_array[x][y] = ubyte_data;
            ushort_array[x][y] = ushort_data;
            uint_array[x][y] = uint_data;
            int64_array[x][y] = int64_data;
            uint64_array[x][y] = uint64_data;
        }
    }
}

/**
 * Test the attribute conversions.
 *
 * @param ncid the ncid of the open test file.
 * @param flavor the iotype of the test file.
 * @param expected array of int, one per type. Expected return value.
 * @param expected_data array of values we expect to find in attributes.
 * @return 0 for success, error code otherwise.
 */
int test_att_conv_byte(int ncid, int flavor, char *name, int *expected, long long *expected_data)
{
    signed char byte_array_in[ATT_LEN];
    short short_array_in[ATT_LEN];
    unsigned char ubyte_array_in[ATT_LEN];
    int int_array_in[ATT_LEN];
    long long_array_in[ATT_LEN];
    float float_array_in[ATT_LEN];
    double double_array_in[ATT_LEN];
    unsigned short ushort_array_in[ATT_LEN];
    unsigned int uint_array_in[ATT_LEN];
    long long int64_array_in[ATT_LEN];
    unsigned long long uint64_array_in[ATT_LEN];

    /* Read the att and check results. */
    if (expected[PIO_BYTE] != PIOc_get_att_schar(ncid, NC_GLOBAL, name, byte_array_in))
        return ERR_WRONG;

    if (expected[PIO_BYTE] == 0)
        for (int x = 0; x < ATT_LEN; x++)
            if (byte_array_in[x] != expected_data[x])
                return ERR_WRONG;

    if (expected[PIO_SHORT] != PIOc_get_att_short(ncid, NC_GLOBAL, name, short_array_in))
        return ERR_WRONG;

    if (expected[PIO_SHORT] == 0)
        for (int x = 0; x < ATT_LEN; x++)
            if (short_array_in[x] != expected_data[x])
                return ERR_WRONG;

    if (expected[PIO_INT] != PIOc_get_att_int(ncid, NC_GLOBAL, name, int_array_in))
        return ERR_WRONG;

    if (expected[PIO_INT] == 0)
        for (int x = 0; x < ATT_LEN; x++)
            if (int_array_in[x] != expected_data[x])
                return ERR_WRONG;

    if (expected[PIO_INT] != PIOc_get_att_long(ncid, NC_GLOBAL, name, long_array_in))
        return ERR_WRONG;

    if (expected[PIO_INT] == 0)
        for (int x = 0; x < ATT_LEN; x++)
            if (long_array_in[x] != expected_data[x])
                return ERR_WRONG;

    if (expected[PIO_FLOAT] != PIOc_get_att_float(ncid, NC_GLOBAL, name, float_array_in))
        return ERR_WRONG;

    if (expected[PIO_FLOAT] == 0)
        for (int x = 0; x < ATT_LEN; x++)
            if (float_array_in[x] != expected_data[x])
                return ERR_WRONG;

    if (expected[PIO_DOUBLE] != PIOc_get_att_double(ncid, NC_GLOBAL, name, double_array_in))
        return ERR_WRONG;

    if (expected[PIO_DOUBLE] == 0)
        for (int x = 0; x < ATT_LEN; x++)
            if (double_array_in[x] != expected_data[x])
                return ERR_WRONG;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((expected[PIO_UBYTE] != PIOc_get_att_uchar(ncid, NC_GLOBAL, name, ubyte_array_in)))
            return ERR_WRONG;

        if (expected[PIO_UBYTE] == 0)
            for (int x = 0; x < ATT_LEN; x++)
                if (ubyte_array_in[x] != expected_data[x])
                    return ERR_WRONG;

        if ((expected[PIO_USHORT] != PIOc_get_att_ushort(ncid, NC_GLOBAL, name, ushort_array_in)))
            return ERR_WRONG;

        if (expected[PIO_USHORT] == 0)
            for (int x = 0; x < ATT_LEN; x++)
                if (ushort_array_in[x] != expected_data[x])
                    return ERR_WRONG;

        if ((expected[PIO_UINT] != PIOc_get_att_uint(ncid, NC_GLOBAL, name, uint_array_in)))
            return ERR_WRONG;

        if (expected[PIO_UINT] == 0)
            for (int x = 0; x < ATT_LEN; x++)
                if (uint_array_in[x] != expected_data[x])
                    return ERR_WRONG;

        if ((expected[PIO_INT64] != PIOc_get_att_longlong(ncid, NC_GLOBAL, name, int64_array_in)))
            return ERR_WRONG;

        if (expected[PIO_INT64] == 0)
            for (int x = 0; x < ATT_LEN; x++)
                if (int64_array_in[x] != expected_data[x])
                    return ERR_WRONG;

        if ((expected[PIO_UINT64] != PIOc_get_att_ulonglong(ncid, NC_GLOBAL, name, uint64_array_in)))
            return ERR_WRONG;

        if (expected[PIO_UINT64] == 0)
            for (int x = 0; x < ATT_LEN; x++)
                if (uint64_array_in[x] != expected_data[x])
                    return ERR_WRONG;
    }

    return PIO_NOERR;
}

/**
 * Test the attribute conversions.
 *
 * @param ncid the ncid of the open test file.
 * @param flavor the iotype of the test file.
 * @param expected array of int, one per type. Expected return value.
 * @param expected_data array of values we expect to find in attributes.
 * @return 0 for success, error code otherwise.
 */
int test_att_conv_int64(int ncid, int flavor, char *name, int *expected, long long *expected_data)
{
    float float_array_in[ATT_LEN];
    double double_array_in[ATT_LEN];
    unsigned char ubyte_array_in[ATT_LEN];
    unsigned short ushort_array_in[ATT_LEN];
    unsigned int uint_array_in[ATT_LEN];
    long long int64_array_in[ATT_LEN];

    /* Read the att and check results. */
    if (expected[PIO_FLOAT] != PIOc_get_att_float(ncid, NC_GLOBAL, name, float_array_in))
        return ERR_WRONG;

    if (expected[PIO_DOUBLE] != PIOc_get_att_double(ncid, NC_GLOBAL, name, double_array_in))
        return ERR_WRONG;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((expected[PIO_UBYTE] != PIOc_get_att_uchar(ncid, NC_GLOBAL, name, ubyte_array_in)))
            return ERR_WRONG;
        if ((expected[PIO_USHORT] != PIOc_get_att_ushort(ncid, NC_GLOBAL, name, ushort_array_in)))
            return ERR_WRONG;
        if ((expected[PIO_UINT] != PIOc_get_att_uint(ncid, NC_GLOBAL, name, uint_array_in)))
            return ERR_WRONG;

        if ((expected[PIO_INT64] != PIOc_get_att_longlong(ncid, NC_GLOBAL, name, int64_array_in)))
            return ERR_WRONG;
    }

    return PIO_NOERR;
}

/* Test attribute read/write operations with NC_BYTE data.
 *
 * This function creates a file with 3 dimensions, with a var of each
 * type. Then it uses the var/var1/vars/vars functions to write, and
 * then read data from the test file.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_atts_byte(int iosysid, int num_flavors, int *flavor, int my_rank,
                   MPI_Comm test_comm)
{
    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char iotype_name[PIO_MAX_NAME + 1];
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        int ncid;
        int ret;    /* Return code. */

        /* Create test file with dims and vars defined. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            return ret;
        sprintf(filename, "%s_att_byte_%s.nc", TEST_NAME, iotype_name);

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
            return ret;

        if ((ret = PIOc_put_att_schar(ncid, NC_GLOBAL, SCHAR_ATT_NAME, PIO_BYTE, ATT_LEN, (signed char *)byte_array)))
            return ret;
        if ((ret = PIOc_put_att_schar(ncid, NC_GLOBAL, SHORT_ATT_NAME, PIO_SHORT, ATT_LEN, (signed char *)byte_array)))
            return ret;
        if ((ret = PIOc_put_att_schar(ncid, NC_GLOBAL, INT_ATT_NAME, PIO_INT, ATT_LEN, (signed char *)byte_array)))
            return ret;
        if ((ret = PIOc_put_att_schar(ncid, NC_GLOBAL, FLOAT_ATT_NAME, PIO_FLOAT, ATT_LEN, (signed char *)byte_array)))
            return ret;
        if ((ret = PIOc_put_att_schar(ncid, NC_GLOBAL, DOUBLE_ATT_NAME, PIO_DOUBLE, ATT_LEN, (signed char *)byte_array)))
            return ret;
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            if (PIOc_put_att_schar(ncid, NC_GLOBAL, UCHAR_ATT_NAME, PIO_UBYTE, ATT_LEN, (signed char *)byte_array) != PIO_ERANGE)
                return ERR_WRONG;
            if (PIOc_put_att_schar(ncid, NC_GLOBAL, USHORT_ATT_NAME, PIO_USHORT, ATT_LEN, (signed char *)byte_array) != PIO_ERANGE)
                return ERR_WRONG;
            if (PIOc_put_att_schar(ncid, NC_GLOBAL, UINT_ATT_NAME, PIO_UINT, ATT_LEN, (signed char *)byte_array) != PIO_ERANGE)
                return ERR_WRONG;
            if ((ret = PIOc_put_att_schar(ncid, NC_GLOBAL, INT64_ATT_NAME, PIO_INT64, ATT_LEN, (signed char *)byte_array)))
                return ret;
            if (PIOc_put_att_schar(ncid, NC_GLOBAL, UINT64_ATT_NAME, PIO_UINT64, ATT_LEN, (signed char *)byte_array) != PIO_ERANGE)
                return ERR_WRONG;
        }
        if ((ret = PIOc_enddef(ncid)))
            return ret;

        /* Test the attribute conversions. */
        long long expected_data[ATT_LEN];
        for (int x = 0; x < ATT_LEN; x++)
            expected_data[x] = byte_array[x][0];

        int schar_expected[NUM_NETCDF_TYPES + 1] = {0, 0, PIO_ERANGE, 0, 0, 0, 0, PIO_ERANGE, PIO_ERANGE, PIO_ERANGE,
                                                    0, PIO_ERANGE, PIO_ERANGE};
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], SCHAR_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], SHORT_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], INT_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], FLOAT_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], DOUBLE_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
            if ((ret = test_att_conv_byte(ncid, flavor[fmt], INT64_ATT_NAME, schar_expected, expected_data)))
                ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Reopen the file. */
        if ((ret = PIOc_openfile2(iosysid, &ncid, &(flavor[fmt]), filename, PIO_NOWRITE)))
            ERR(ret);

        /* Test the attribute conversions. */
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], SCHAR_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], SHORT_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], INT_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], FLOAT_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if ((ret = test_att_conv_byte(ncid, flavor[fmt], DOUBLE_ATT_NAME, schar_expected, expected_data)))
            ERR(ret);
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
            if ((ret = test_att_conv_byte(ncid, flavor[fmt], INT64_ATT_NAME, schar_expected, expected_data)))
                ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

    } /* next flavor */

    return PIO_NOERR;
}

/* Test attribute read/write operations with NC_INT64 data.
 *
 * This function creates a file with 3 dimensions, with a var of each
 * type. Then it uses the var/var1/vars/vars functions to write, and
 * then read data from the test file.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_atts_int64(int iosysid, int num_flavors, int *flavor, int my_rank,
                    MPI_Comm test_comm)
{
    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char iotype_name[PIO_MAX_NAME + 1];
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        int ncid;
        int ret;    /* Return code. */

        /* Only do these tests on netCDF-4 files. */
        if (flavor[fmt] != PIO_IOTYPE_NETCDF4C && flavor[fmt] != PIO_IOTYPE_NETCDF4P)
            continue;

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            return ret;
        sprintf(filename, "%s_att_int64_%s.nc", TEST_NAME, iotype_name);

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
            return ret;

        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, SCHAR_ATT_NAME, PIO_BYTE, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, SHORT_ATT_NAME, PIO_SHORT, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, INT_ATT_NAME, PIO_INT, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if ((ret = PIOc_put_att_longlong(ncid, NC_GLOBAL, FLOAT_ATT_NAME, PIO_FLOAT, ATT_LEN, (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_att_longlong(ncid, NC_GLOBAL, DOUBLE_ATT_NAME, PIO_DOUBLE, ATT_LEN, (long long *)int64_array)))
            return ret;
        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, UCHAR_ATT_NAME, PIO_UBYTE, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, USHORT_ATT_NAME, PIO_USHORT, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, UINT_ATT_NAME, PIO_UINT, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if ((ret = PIOc_put_att_longlong(ncid, NC_GLOBAL, INT64_ATT_NAME, PIO_INT64, ATT_LEN, (long long *)int64_array)))
            return ret;
        if (PIOc_put_att_longlong(ncid, NC_GLOBAL, UINT64_ATT_NAME, PIO_UINT64, ATT_LEN, (long long *)int64_array) != PIO_ERANGE)
            return ERR_WRONG;
        if ((ret = PIOc_enddef(ncid)))
            return ret;

        /* Test the attribute conversions. */
        long long expected_data[ATT_LEN];
        for (int x = 0; x < ATT_LEN; x++)
            expected_data[x] = int64_array[x][0];

        int int64_expected[NUM_NETCDF_TYPES + 1] = {0, PIO_ERANGE, PIO_ERANGE, PIO_ERANGE, PIO_ERANGE, 0, 0, 0, 0, 0,
                                                    0, PIO_ERANGE, PIO_ERANGE};
        if ((ret = test_att_conv_int64(ncid, flavor[fmt], SCHAR_ATT_NAME, int64_expected, expected_data)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Reopen the file. */
        if ((ret = PIOc_openfile2(iosysid, &ncid, &(flavor[fmt]), filename, PIO_NOWRITE)))
            ERR(ret);

        if ((ret = test_att_conv_int64(ncid, flavor[fmt], SCHAR_ATT_NAME, int64_expected, expected_data)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

    } /* next flavor */

    return PIO_NOERR;
}

/* Use the var1 functions to write some data to an open test file. */
int putget_write_var1(int ncid, int *varid, PIO_Offset *index, int flavor)
{
    int ret;

    /* These should not work. */
    if (PIOc_put_var1_text(ncid, varid[1], index, NULL) != PIO_EINVAL)
        return ERR_WRONG;
    if (PIOc_put_var1_text(ncid + TEST_VAL_42, varid[1], index, text) != PIO_EBADID)
        return ERR_WRONG;
    if (PIOc_put_var1_text(ncid, varid[1] + TEST_VAL_42, index, text) != PIO_ENOTVAR)
        return ERR_WRONG;

    if ((ret = PIOc_put_var1_text(ncid, varid[1], index, text)))
        return ret;

    if ((ret = PIOc_put_var1_schar(ncid, varid[0], index, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_var1_short(ncid, varid[2], index, &short_data)))
        return ret;

    if ((ret = PIOc_put_var1_int(ncid, varid[3], index, &int_data)))
        return ret;

    if ((ret = PIOc_put_var1_long(ncid, varid[4], index, &long_data)))
        return ret;

    if ((ret = PIOc_put_var1_float(ncid, varid[5], index, &float_data)))
        return ret;

    if ((ret = PIOc_put_var1_double(ncid, varid[6], index, &double_data)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_var1_uchar(ncid, varid[7], index, &ubyte_data)))
            return ret;
        if ((ret = PIOc_put_var1_ushort(ncid, varid[8], index, &ushort_data)))
            return ret;
        if ((ret = PIOc_put_var1_uint(ncid, varid[9], index, &uint_data)))
            return ret;
        if ((ret = PIOc_put_var1_longlong(ncid, varid[10], index, &int64_data)))
            return ret;
        if ((ret = PIOc_put_var1_ulonglong(ncid, varid[11], index, &uint64_data)))
            return ret;
    }

    return 0;
}

/* Use the no type versions of the var1 functions to write some data
 * to an open test file. */
int putget_write_var1_nt(int ncid, int *varid, PIO_Offset *index, int flavor)
{
    int ret;

    /* These should not work. */
    if (PIOc_put_var1(ncid, varid[1], index, NULL) != PIO_EINVAL)
        return ERR_WRONG;
    if (PIOc_put_var1(ncid + TEST_VAL_42, varid[1], index, text) != PIO_EBADID)
        return ERR_WRONG;
    if (PIOc_put_var1(ncid, varid[1] + TEST_VAL_42, index, text) != PIO_ENOTVAR)
        return ERR_WRONG;

    if ((ret = PIOc_put_var1(ncid, varid[0], index, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_var1(ncid, varid[1], index, text)))
        return ret;

    if ((ret = PIOc_put_var1(ncid, varid[2], index, &short_data)))
        return ret;

    if ((ret = PIOc_put_var1(ncid, varid[3], index, &int_data)))
        return ret;

    if ((ret = PIOc_put_var1(ncid, varid[4], index, &int_data)))
        return ret;

    if ((ret = PIOc_put_var1(ncid, varid[5], index, &float_data)))
        return ret;

    if ((ret = PIOc_put_var1(ncid, varid[6], index, &double_data)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_var1(ncid, varid[7], index, &ubyte_data)))
            return ret;
        if ((ret = PIOc_put_var1(ncid, varid[8], index, &ushort_data)))
            return ret;
        if ((ret = PIOc_put_var1(ncid, varid[9], index, &uint_data)))
            return ret;
        if ((ret = PIOc_put_var1(ncid, varid[10], index, &int64_data)))
            return ret;
        if ((ret = PIOc_put_var1(ncid, varid[11], index, &uint64_data)))
            return ret;
    }

    return 0;
}

/* Use the var functions to write some data to an open test file. */
int putget_write_var(int ncid, int *varid, int flavor)
{
    int ret;

    if ((ret = PIOc_put_var_text(ncid, varid[1], (char *)text_array)))
        return ret;

    if ((ret = PIOc_put_var_schar(ncid, varid[0], (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_var_short(ncid, varid[2], (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_var_int(ncid, varid[3], (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_var_long(ncid, varid[4], (long int *)long_array)))
        return ret;

    if ((ret = PIOc_put_var_float(ncid, varid[5], (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_var_double(ncid, varid[6], (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_var_uchar(ncid, varid[7], (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_var_ushort(ncid, varid[8], (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_var_uint(ncid, varid[9], (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_var_longlong(ncid, varid[10], (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_var_ulonglong(ncid, varid[11], (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the var functions to write some data to an open test file. */
int putget_write_var_nt(int ncid, int *varid, int flavor)
{
    int ret;

    if ((ret = PIOc_put_var(ncid, varid[1], (char *)text_array)))
        return ret;

    if ((ret = PIOc_put_var(ncid, varid[0], (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_var(ncid, varid[2], (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_var(ncid, varid[3], (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_var(ncid, varid[4], (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_var(ncid, varid[5], (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_var(ncid, varid[6], (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_var(ncid, varid[7], (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_var(ncid, varid[8], (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_var(ncid, varid[9], (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_var(ncid, varid[10], (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_var(ncid, varid[11], (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the att functions to write some data to attributes in an open
 * test file.
 *
 * @param ncid the ncid of the test file to read.
 * @param varid an array of varids in the file.
 * @param flavor the PIO IO type of the test file.
 * @returns 0 for success, error code otherwise.
 */
int test_write_atts(int ncid, int *varid, int flavor)
{
    int ret;

    /* Test some invalid parameters. (Type is irrelevant here.) */
    if (PIOc_put_att_schar(ncid + 1, varid[0], SCHAR_ATT_NAME, PIO_BYTE,
                           ATT_LEN, (signed char *)byte_array) != PIO_EBADID)
        return ERR_WRONG;

    if ((ret = PIOc_put_att_text(ncid, varid[0], TEXT_ATT_NAME, ATT_LEN,
                                 TEXT_ATT_VALUE)))
        return ret;

    /* Use put_att() for the schar. */
    if ((ret = PIOc_put_att(ncid, varid[0], SCHAR_ATT_NAME, PIO_BYTE,
                            ATT_LEN, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_att_text(ncid, varid[1], TEXT_ATT_NAME, ATT_LEN,
                                 TEXT_ATT_VALUE)))
        return ret;

    if ((ret = PIOc_put_att_short(ncid, varid[2], SHORT_ATT_NAME, PIO_SHORT,
                                  ATT_LEN, (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_att_int(ncid, varid[3], INT_ATT_NAME, PIO_INT,
                                ATT_LEN, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_att_long(ncid, varid[4], LONG_ATT_NAME, PIO_INT,
                                 ATT_LEN, (long int *)long_array)))
        return ret;

    if ((ret = PIOc_put_att_float(ncid, varid[5], FLOAT_ATT_NAME, PIO_FLOAT,
                                  ATT_LEN, (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_att_double(ncid, varid[6], DOUBLE_ATT_NAME, PIO_DOUBLE,
                                   ATT_LEN, (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_att_uchar(ncid, varid[7], UCHAR_ATT_NAME, PIO_UBYTE,
                                      ATT_LEN, (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_att_ushort(ncid, varid[8], USHORT_ATT_NAME, PIO_SHORT,
                                       ATT_LEN, (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_att_uint(ncid, varid[9], UINT_ATT_NAME, PIO_UINT,
                                     ATT_LEN, (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_att_longlong(ncid, varid[10], INT64_ATT_NAME, PIO_INT64,
                                         ATT_LEN, (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_att_ulonglong(ncid, varid[11], UINT64_ATT_NAME, PIO_UINT64,
                                          ATT_LEN, (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the att functions to read some attributes from an open test
 * file.
 *
 * @param ncid the ncid of the test file to read.
 * @param varid an array of varids in the file.
 * @param flavor the PIO IO type of the test file.
 * @returns 0 for success, error code otherwise.
 */
int test_read_att(int ncid, int *varid, int flavor)
{
    char text_in[ATT_LEN];
    signed char byte_array_in[ATT_LEN];
    short short_array_in[ATT_LEN];
    unsigned char ubyte_array_in[ATT_LEN];
    int int_array_in[ATT_LEN];
    long int long_array_in[ATT_LEN];
    float float_array_in[ATT_LEN];
    double double_array_in[ATT_LEN];
    unsigned short ushort_array_in[ATT_LEN];
    unsigned int uint_array_in[ATT_LEN];
    long long int64_array_in[ATT_LEN];
    unsigned long long uint64_array_in[ATT_LEN];
    int x;
    int ret;

    if ((ret = PIOc_get_att_schar(ncid, varid[0], SCHAR_ATT_NAME, byte_array_in)))
        return ret;
    if ((ret = PIOc_get_att_text(ncid, varid[1], TEXT_ATT_NAME, text_in)))
        return ret;
    if ((ret = PIOc_get_att_short(ncid, varid[2], SHORT_ATT_NAME, short_array_in)))
        return ret;
    if ((ret = PIOc_get_att_int(ncid, varid[3], INT_ATT_NAME, int_array_in)))
        return ret;
    if ((ret = PIOc_get_att_long(ncid, varid[4], LONG_ATT_NAME, long_array_in)))
        return ret;
    if ((ret = PIOc_get_att_float(ncid, varid[5], FLOAT_ATT_NAME, float_array_in)))
        return ret;
    if ((ret = PIOc_get_att_double(ncid, varid[6], DOUBLE_ATT_NAME, double_array_in)))
        return ret;
    for (x = 0; x < ATT_LEN; x++)
    {
        if (strncmp(text_in, TEXT_ATT_VALUE, ATT_LEN))
            return ERR_WRONG;
        if (byte_array_in[x] != byte_array[x][0])
            return ERR_WRONG;
        if (short_array_in[x] != short_array[x][0])
            return ERR_WRONG;
        if (int_array_in[x] != int_array[x][0])
            return ERR_WRONG;
        if (long_array_in[x] != long_array[x][0])
            return ERR_WRONG;
        if (float_array_in[x] != float_array[x][0])
            return ERR_WRONG;
        if (double_array_in[x] != double_array[x][0])
            return ERR_WRONG;
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_att_uchar(ncid, varid[7], UCHAR_ATT_NAME, ubyte_array_in)))
            return ret;
        if ((ret = PIOc_get_att_ushort(ncid, varid[8], USHORT_ATT_NAME, ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_att_uint(ncid, varid[9], UINT_ATT_NAME, uint_array_in)))
            return ret;
        if ((ret = PIOc_get_att_longlong(ncid, varid[10], INT64_ATT_NAME, int64_array_in)))
            return ret;
        if ((ret = PIOc_get_att_ulonglong(ncid, varid[11], UINT64_ATT_NAME, uint64_array_in)))
            return ret;
        for (x = 0; x < ATT_LEN; x++)
        {
            if (ubyte_array_in[x] != ubyte_array[x][0])
                return ERR_WRONG;
            if (ushort_array_in[x] != ushort_array[x][0])
                return ERR_WRONG;
            if (uint_array_in[x] != uint_array[x][0])
                return ERR_WRONG;
            if (int64_array_in[x] != int64_array[x][0])
                return ERR_WRONG;
            if (uint64_array_in[x] != uint64_array[x][0])
                return ERR_WRONG;
        }
    }

    return 0;
}

/* Use the vara functions to write some data to an open test file. */
int putget_write_vara(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                      int flavor)
{
    int ret;

    if ((ret = PIOc_put_vara_text(ncid, varid[1], start, count, (char *)text_array)))
        return ret;

    if ((ret = PIOc_put_vara_schar(ncid, varid[0], start, count, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_vara_short(ncid, varid[2], start, count, (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_vara_int(ncid, varid[3], start, count, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_vara_long(ncid, varid[4], start, count, (long int *)long_array)))
        return ret;

    if ((ret = PIOc_put_vara_float(ncid, varid[5], start, count, (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_vara_double(ncid, varid[6], start, count, (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_vara_uchar(ncid, varid[7], start, count, (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_vara_ushort(ncid, varid[8], start, count, (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_vara_uint(ncid, varid[9], start, count, (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_vara_longlong(ncid, varid[10], start, count, (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_vara_ulonglong(ncid, varid[11], start, count, (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the no-type vara functions to write some data to an open test file. */
int putget_write_vara_nt(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                         int flavor)
{
    int ret;

    if ((ret = PIOc_put_vara(ncid, varid[1], start, count, (char *)text_array)))
        return ret;

    if ((ret = PIOc_put_vara(ncid, varid[0], start, count, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_vara(ncid, varid[2], start, count, (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_vara(ncid, varid[3], start, count, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_vara(ncid, varid[4], start, count, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_vara(ncid, varid[5], start, count, (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_vara(ncid, varid[6], start, count, (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_vara(ncid, varid[7], start, count, (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_vara(ncid, varid[8], start, count, (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_vara(ncid, varid[9], start, count, (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_vara(ncid, varid[10], start, count, (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_vara(ncid, varid[11], start, count, (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the vars functions to write some data to an open test file. */
int putget_write_vars(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                      PIO_Offset *stride, int flavor)
{
    int ret;

    if ((ret = PIOc_put_vara_text(ncid, varid[1], start, count, (char *)text_array)))
        return ret;

    if ((ret = PIOc_put_vars_schar(ncid, varid[0], start, count, stride, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_vars_short(ncid, varid[2], start, count, stride, (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_vars_int(ncid, varid[3], start, count, stride, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_vars_long(ncid, varid[4], start, count, stride, (long int *)long_array)))
        return ret;

    if ((ret = PIOc_put_vars_float(ncid, varid[5], start, count, stride, (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_vars_double(ncid, varid[6], start, count, stride, (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_vars_uchar(ncid, varid[7], start, count, stride, (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_vars_ushort(ncid, varid[8], start, count, stride, (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_vars_uint(ncid, varid[9], start, count, stride, (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_vars_longlong(ncid, varid[10], start, count, stride, (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_vars_ulonglong(ncid, varid[11], start, count, stride, (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the no-type vars functions to write some data to an open test file. */
int putget_write_vars_nt(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                         PIO_Offset *stride, int flavor)
{
    int ret;

    if ((ret = PIOc_put_vara(ncid, varid[1], start, count, (char *)text_array)))
        return ret;

    if ((ret = PIOc_put_vars(ncid, varid[0], start, count, stride, (signed char *)byte_array)))
        return ret;

    if ((ret = PIOc_put_vars(ncid, varid[2], start, count, stride, (short *)short_array)))
        return ret;

    if ((ret = PIOc_put_vars(ncid, varid[3], start, count, stride, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_vars(ncid, varid[4], start, count, stride, (int *)int_array)))
        return ret;

    if ((ret = PIOc_put_vars(ncid, varid[5], start, count, stride, (float *)float_array)))
        return ret;

    if ((ret = PIOc_put_vars(ncid, varid[6], start, count, stride, (double *)double_array)))
        return ret;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_put_vars(ncid, varid[7], start, count, stride, (unsigned char *)ubyte_array)))
            return ret;
        if ((ret = PIOc_put_vars(ncid, varid[8], start, count, stride, (unsigned short *)ushort_array)))
            return ret;
        if ((ret = PIOc_put_vars(ncid, varid[9], start, count, stride, (unsigned int *)uint_array)))
            return ret;
        if ((ret = PIOc_put_vars(ncid, varid[10], start, count, stride, (long long *)int64_array)))
            return ret;
        if ((ret = PIOc_put_vars(ncid, varid[11], start, count, stride, (unsigned long long *)uint64_array)))
            return ret;
    }

    return 0;
}

/* Use the var1 functions to read some data from an open test file. */
int putget_read_var1(int ncid, int *varid, PIO_Offset *index, int flavor)
{
    signed char byte_data_in;
    char text_data_in[PIO_MAX_NAME];
    short short_data_in;
    unsigned char ubyte_data_in;
    int int_data_in;
    long int long_data_in;
    float float_data_in;
    double double_data_in;
    unsigned short ushort_data_in;
    unsigned int uint_data_in;
    long long int64_data_in;
    unsigned long long uint64_data_in;
    int ret;

    /* These should not work. */
    if (PIOc_get_var1_schar(ncid + TEST_VAL_42, varid[0], index, &byte_data_in) != PIO_EBADID)
        return ERR_WRONG;
    if (PIOc_get_var1_schar(ncid, varid[0] + TEST_VAL_42, index, &byte_data_in) != PIO_ENOTVAR)
        return ERR_WRONG;
    if (PIOc_get_var1_schar(ncid, varid[0], index, NULL) != PIO_EINVAL)
        return ERR_WRONG;

    /* Get and check the data. */
    if ((ret = PIOc_get_var1_schar(ncid, varid[0], index, &byte_data_in)))
        return ret;
    if (byte_data_in != byte_data)
        return ERR_WRONG;

    memset(text_data_in, 0, sizeof(text_data_in));
    if ((ret = PIOc_get_var1_text(ncid, varid[1], index, text_data_in)))
        return ret;
    if (strncmp(text_data_in, "h", 1))
        return ERR_WRONG;

    if ((ret = PIOc_get_var1_short(ncid, varid[2], index, &short_data_in)))
        return ret;
    if (short_data_in != short_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1_int(ncid, varid[3], index, &int_data_in)))
        return ret;
    if (int_data_in != int_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1_long(ncid, varid[4], index, &long_data_in)))
        return ret;
    if (long_data_in != long_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1_float(ncid, varid[5], index, &float_data_in)))
        return ret;
    if (float_data_in != float_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1_double(ncid, varid[6], index, &double_data_in)))
        return ret;
    if (double_data_in != double_data)
        return ERR_WRONG;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_var1_uchar(ncid, varid[7], index, &ubyte_data_in)))
            return ret;
        if (ubyte_data_in != ubyte_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1_ushort(ncid, varid[8], index, &ushort_data_in)))
            return ret;
        if (ushort_data_in != ushort_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1_uint(ncid, varid[9], index, &uint_data_in)))
            return ret;
        if (uint_data_in != uint_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1_longlong(ncid, varid[10], index, &int64_data_in)))
            return ret;
        if (int64_data_in != int64_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1_ulonglong(ncid, varid[11], index, &uint64_data_in)))
            return ret;
        if (uint64_data_in != uint64_data)
            return ERR_WRONG;
    }

    return 0;
}

/* Use the var1 functions to read some data from an open test file. */
int putget_read_var1_nt(int ncid, int *varid, PIO_Offset *index, int flavor)
{
    signed char byte_data_in;
    char text_data_in[PIO_MAX_NAME];
    short short_data_in;
    unsigned char ubyte_data_in;
    int int_data_in;
    float float_data_in;
    double double_data_in;
    unsigned short ushort_data_in;
    unsigned int uint_data_in;
    long long int64_data_in;
    unsigned long long uint64_data_in;
    int ret;

    /* These should not work. */
    if (PIOc_get_var1(ncid + TEST_VAL_42, varid[0], index, &byte_data_in) != PIO_EBADID)
        return ERR_WRONG;
    if (PIOc_get_var1(ncid, varid[0] + TEST_VAL_42, index, &byte_data_in) != PIO_ENOTVAR)
        return ERR_WRONG;
    if (PIOc_get_var1(ncid, varid[0], index, NULL) != PIO_EINVAL)
        return ERR_WRONG;

    /* Get and check the data. */
    if ((ret = PIOc_get_var1(ncid, varid[0], index, &byte_data_in)))
        return ret;
    if (byte_data_in != byte_data)
        return ERR_WRONG;

    memset(text_data_in, 0, sizeof(text_data_in));
    if ((ret = PIOc_get_var1(ncid, varid[1], index, text_data_in)))
        return ret;
    if (strncmp(text_data_in, "h", 1))
        return ERR_WRONG;

    if ((ret = PIOc_get_var1(ncid, varid[2], index, &short_data_in)))
        return ret;
    if (short_data_in != short_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1(ncid, varid[3], index, &int_data_in)))
        return ret;
    if (int_data_in != int_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1(ncid, varid[4], index, &int_data_in)))
        return ret;
    if (int_data_in != int_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1(ncid, varid[5], index, &float_data_in)))
        return ret;
    if (float_data_in != float_data)
        return ERR_WRONG;

    if ((ret = PIOc_get_var1(ncid, varid[6], index, &double_data_in)))
        return ret;
    if (double_data_in != double_data)
        return ERR_WRONG;

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_var1(ncid, varid[7], index, &ubyte_data_in)))
            return ret;
        if (ubyte_data_in != ubyte_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1(ncid, varid[8], index, &ushort_data_in)))
            return ret;
        if (ushort_data_in != ushort_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1(ncid, varid[9], index, &uint_data_in)))
            return ret;
        if (uint_data_in != uint_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1(ncid, varid[10], index, &int64_data_in)))
            return ret;
        if (int64_data_in != int64_data)
            return ERR_WRONG;
        if ((ret = PIOc_get_var1(ncid, varid[11], index, &uint64_data_in)))
            return ret;
        if (uint64_data_in != uint64_data)
            return ERR_WRONG;
    }

    return 0;
}

/* Use the var functions to read some data from an open test file.
 *
 * @param ncid the ncid of the test file to read.
 * @param varid an array of varids in the file.
 * @param unlim non-zero if unlimited dimension is in use.
 * @param flavor the PIO IO type of the test file.
 * @returns 0 for success, error code otherwise.
 */
int putget_read_var(int ncid, int *varid, int unlim, int flavor)
{
    signed char byte_array_in[X_DIM_LEN][Y_DIM_LEN];
    char text_array_in[X_DIM_LEN][Y_DIM_LEN];
    short short_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned char ubyte_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in[X_DIM_LEN][Y_DIM_LEN];
    long int long_array_in[X_DIM_LEN][Y_DIM_LEN];
    float float_array_in[X_DIM_LEN][Y_DIM_LEN];
    double double_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned short ushort_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned int uint_array_in[X_DIM_LEN][Y_DIM_LEN];
    long long int64_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned long long uint64_array_in[X_DIM_LEN][Y_DIM_LEN];
    int x, y;
    int ret;

    /* When using the unlimited dimension, no data are wrtten by the
     * put_var_TYPE() functions, since the length of the unlimited
     * dimension is still 0. */
    if (unlim)
    {
        return 0;
    }

    if ((ret = PIOc_get_var_schar(ncid, varid[0], (signed char *)byte_array_in)))
        return ret;
    if ((ret = PIOc_get_var_text(ncid, varid[1], (char *)text_array_in)))
        return ret;
    if ((ret = PIOc_get_var_short(ncid, varid[2], (short *)short_array_in)))
        return ret;
    if ((ret = PIOc_get_var_int(ncid, varid[3], (int *)int_array_in)))
        return ret;
    if ((ret = PIOc_get_var_long(ncid, varid[4], (long int *)long_array_in)))
        return ret;
    if ((ret = PIOc_get_var_float(ncid, varid[5], (float *)float_array_in)))
        return ret;
    if ((ret = PIOc_get_var_double(ncid, varid[6], (double *)double_array_in)))
        return ret;
    for (x = 0; x < X_DIM_LEN; x++)
    {
        if (strncmp(text_array_in[x], text, strlen(text)))
            return ERR_WRONG;
        for (y = 0; y < Y_DIM_LEN; y++)
        {
            if (byte_array_in[x][y] != byte_array[x][y])
                return ERR_WRONG;
            if (short_array_in[x][y] != short_array[x][y])
                return ERR_WRONG;
            if (int_array_in[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (long_array_in[x][y] != long_array[x][y])
                return ERR_WRONG;
            if (float_array_in[x][y] != float_array[x][y])
                return ERR_WRONG;
            if (double_array_in[x][y] != double_array[x][y])
                return ERR_WRONG;
        }
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_var_uchar(ncid, varid[7], (unsigned char *)ubyte_array_in)))
            return ret;
        if ((ret = PIOc_get_var_ushort(ncid, varid[8], (unsigned short *)ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_var_uint(ncid, varid[9], (unsigned int *)uint_array_in)))
            return ret;
        if ((ret = PIOc_get_var_longlong(ncid, varid[10], (long long *)int64_array_in)))
            return ret;
        if ((ret = PIOc_get_var_ulonglong(ncid, varid[11], (unsigned long long *)uint64_array_in)))
            return ret;
        for (x = 0; x < X_DIM_LEN; x++)
            for (y = 0; y < Y_DIM_LEN; y++)
            {
                if (ubyte_array_in[x][y] != ubyte_array[x][y])
                    return ERR_WRONG;
                if (ushort_array_in[x][y] != ushort_array[x][y])
                    return ERR_WRONG;
                if (uint_array_in[x][y] != uint_array[x][y])
                    return ERR_WRONG;
                if (int64_array_in[x][y] != int64_array[x][y])
                    return ERR_WRONG;
                if (uint64_array_in[x][y] != uint64_array[x][y])
                    return ERR_WRONG;
            }
    }

    return 0;
}

/* Use the no-type var functions to read some data from an open test file.
 *
 * @param ncid the ncid of the test file to read.
 * @param varid an array of varids in the file.
 * @param unlim non-zero if unlimited dimension is in use.
 * @param flavor the PIO IO type of the test file.
 * @returns 0 for success, error code otherwise.
 */
int putget_read_var_nt(int ncid, int *varid, int unlim, int flavor)
{
    signed char byte_array_in[X_DIM_LEN][Y_DIM_LEN];
    char text_array_in[X_DIM_LEN][Y_DIM_LEN];
    short short_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned char ubyte_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in2[X_DIM_LEN][Y_DIM_LEN];
    float float_array_in[X_DIM_LEN][Y_DIM_LEN];
    double double_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned short ushort_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned int uint_array_in[X_DIM_LEN][Y_DIM_LEN];
    long long int64_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned long long uint64_array_in[X_DIM_LEN][Y_DIM_LEN];
    int x, y;
    int ret;

    /* When using the unlimited dimension, no data are wrtten by the
     * put_var_TYPE() functions, since the length of the unlimited
     * dimension is still 0. */
    if (unlim)
    {
        return 0;
    }

    if ((ret = PIOc_get_var(ncid, varid[0], (signed char *)byte_array_in)))
        return ret;
    if ((ret = PIOc_get_var(ncid, varid[1], (char *)text_array_in)))
        return ret;
    if ((ret = PIOc_get_var(ncid, varid[2], (short *)short_array_in)))
        return ret;
    if ((ret = PIOc_get_var(ncid, varid[3], (int *)int_array_in)))
        return ret;
    if ((ret = PIOc_get_var(ncid, varid[4], (int *)int_array_in2)))
        return ret;
    if ((ret = PIOc_get_var(ncid, varid[5], (float *)float_array_in)))
        return ret;
    if ((ret = PIOc_get_var(ncid, varid[6], (double *)double_array_in)))
        return ret;
    for (x = 0; x < X_DIM_LEN; x++)
    {
        if (strncmp(text_array_in[x], text, strlen(text)))
            return ERR_WRONG;
        for (y = 0; y < Y_DIM_LEN; y++)
        {
            if (byte_array_in[x][y] != byte_array[x][y])
                return ERR_WRONG;
            if (short_array_in[x][y] != short_array[x][y])
                return ERR_WRONG;
            if (int_array_in[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (int_array_in2[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (float_array_in[x][y] != float_array[x][y])
                return ERR_WRONG;
            if (double_array_in[x][y] != double_array[x][y])
                return ERR_WRONG;
        }
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_var(ncid, varid[7], (unsigned char *)ubyte_array_in)))
            return ret;
        if ((ret = PIOc_get_var(ncid, varid[8], (unsigned short *)ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_var(ncid, varid[9], (unsigned int *)uint_array_in)))
            return ret;
        if ((ret = PIOc_get_var(ncid, varid[10], (long long *)int64_array_in)))
            return ret;
        if ((ret = PIOc_get_var(ncid, varid[11], (unsigned long long *)uint64_array_in)))
            return ret;
        for (x = 0; x < X_DIM_LEN; x++)
            for (y = 0; y < Y_DIM_LEN; y++)
            {
                if (ubyte_array_in[x][y] != ubyte_array[x][y])
                    return ERR_WRONG;
                if (ushort_array_in[x][y] != ushort_array[x][y])
                    return ERR_WRONG;
                if (uint_array_in[x][y] != uint_array[x][y])
                    return ERR_WRONG;
                if (int64_array_in[x][y] != int64_array[x][y])
                    return ERR_WRONG;
                if (uint64_array_in[x][y] != uint64_array[x][y])
                    return ERR_WRONG;
            }
    }

    return 0;
}

/* Use the vara functions to read some data from an open test file. */
int putget_read_vara(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                     int flavor)
{
    signed char byte_array_in[X_DIM_LEN][Y_DIM_LEN];
    char text_array_in[X_DIM_LEN][Y_DIM_LEN];
    short short_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned char ubyte_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in[X_DIM_LEN][Y_DIM_LEN];
    long int long_array_in[X_DIM_LEN][Y_DIM_LEN];
    float float_array_in[X_DIM_LEN][Y_DIM_LEN];
    double double_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned short ushort_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned int uint_array_in[X_DIM_LEN][Y_DIM_LEN];
    long long int64_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned long long uint64_array_in[X_DIM_LEN][Y_DIM_LEN];
    int x, y;
    int ret;

    if ((ret = PIOc_get_vara_schar(ncid, varid[0], start, count, (signed char *)byte_array_in)))
        return ret;
    if ((ret = PIOc_get_vara_text(ncid, varid[1], start, count, (char *)text_array_in)))
        return ret;
    if ((ret = PIOc_get_vara_short(ncid, varid[2], start, count, (short *)short_array_in)))
        return ret;
    if ((ret = PIOc_get_vara_int(ncid, varid[3], start, count, (int *)int_array_in)))
        return ret;
    if ((ret = PIOc_get_vara_long(ncid, varid[4], start, count, (long int *)long_array_in)))
        return ret;
    if ((ret = PIOc_get_vara_float(ncid, varid[5], start, count, (float *)float_array_in)))
        return ret;
    if ((ret = PIOc_get_vara_double(ncid, varid[6], start, count, (double *)double_array_in)))
        return ret;

    for (x = 0; x < X_DIM_LEN; x++)
    {
        if (strncmp(text_array_in[x], text, strlen(text)))
            return ERR_WRONG;
        for (y = 0; y < Y_DIM_LEN; y++)
        {
            if (byte_array_in[x][y] != byte_array[x][y])
                return ERR_WRONG;
            if (short_array_in[x][y] != short_array[x][y])
                return ERR_WRONG;
            if (int_array_in[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (long_array_in[x][y] != long_array[x][y])
                return ERR_WRONG;
            if (float_array_in[x][y] != float_array[x][y])
                return ERR_WRONG;
            if (double_array_in[x][y] != double_array[x][y])
                return ERR_WRONG;
        }
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_vara_uchar(ncid, varid[7], start, count, (unsigned char *)ubyte_array_in)))
            return ret;

        if ((ret = PIOc_get_vara_ushort(ncid, varid[8], start, count, (unsigned short *)ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_vara_uint(ncid, varid[9], start, count, (unsigned int *)uint_array_in)))
            return ret;
        if ((ret = PIOc_get_vara_longlong(ncid, varid[10], start, count, (long long *)int64_array_in)))
            return ret;
        if ((ret = PIOc_get_vara_ulonglong(ncid, varid[11], start, count, (unsigned long long *)uint64_array_in)))
            return ret;
        for (x = 0; x < X_DIM_LEN; x++)
            for (y = 0; y < Y_DIM_LEN; y++)
            {
                if (ubyte_array_in[x][y] != ubyte_array[x][y])
                    return ERR_WRONG;
                if (ushort_array_in[x][y] != ushort_array[x][y])
                    return ERR_WRONG;
                if (uint_array_in[x][y] != uint_array[x][y])
                    return ERR_WRONG;
                if (int64_array_in[x][y] != int64_array[x][y])
                    return ERR_WRONG;
                if (uint64_array_in[x][y] != uint64_array[x][y])
                    return ERR_WRONG;
            }
    }

    return 0;
}

/* Use the vars functions to read some data from an open test file. */
int putget_read_vars(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                     PIO_Offset *stride, int flavor)
{
    signed char byte_array_in[X_DIM_LEN][Y_DIM_LEN];
    char text_array_in[X_DIM_LEN][Y_DIM_LEN];
    short short_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned char ubyte_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in[X_DIM_LEN][Y_DIM_LEN];
    long int long_array_in[X_DIM_LEN][Y_DIM_LEN];
    float float_array_in[X_DIM_LEN][Y_DIM_LEN];
    double double_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned short ushort_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned int uint_array_in[X_DIM_LEN][Y_DIM_LEN];
    long long int64_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned long long uint64_array_in[X_DIM_LEN][Y_DIM_LEN];
    int x, y;
    int ret;

    if ((ret = PIOc_get_vars_schar(ncid, varid[0], start, count, stride, (signed char *)byte_array_in)))
        return ret;
    if ((ret = PIOc_get_vars_text(ncid, varid[1], start, count, stride, (char *)text_array_in)))
        return ret;
    if ((ret = PIOc_get_vars_short(ncid, varid[2], start, count, stride, (short *)short_array_in)))
        return ret;
    if ((ret = PIOc_get_vars_int(ncid, varid[3], start, count, stride, (int *)int_array_in)))
        return ret;
    if ((ret = PIOc_get_vars_long(ncid, varid[4], start, count, stride, (long int *)long_array_in)))
        return ret;
    if ((ret = PIOc_get_vars_float(ncid, varid[5], start, count, stride, (float *)float_array_in)))
        return ret;
    if ((ret = PIOc_get_vars_double(ncid, varid[6], start, count, stride, (double *)double_array_in)))
        return ret;

    for (x = 0; x < X_DIM_LEN; x++)
    {
        if (strncmp(text_array_in[x], text, strlen(text)))
            return ERR_WRONG;
        for (y = 0; y < Y_DIM_LEN; y++)
        {
            if (byte_array_in[x][y] != byte_array[x][y])
                return ERR_WRONG;
            if (short_array_in[x][y] != short_array[x][y])
                return ERR_WRONG;
            if (int_array_in[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (long_array_in[x][y] != long_array[x][y])
                return ERR_WRONG;
            if (float_array_in[x][y] != float_array[x][y])
                return ERR_WRONG;
            if (double_array_in[x][y] != double_array[x][y])
                return ERR_WRONG;
        }
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_vars_uchar(ncid, varid[7], start, count, stride, (unsigned char *)ubyte_array_in)))
            return ret;

        if ((ret = PIOc_get_vars_ushort(ncid, varid[8], start, count, stride, (unsigned short *)ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_vars_uint(ncid, varid[9], start, count, stride, (unsigned int *)uint_array_in)))
            return ret;
        if ((ret = PIOc_get_vars_longlong(ncid, varid[10], start, count, stride, (long long *)int64_array_in)))
            return ret;
        if ((ret = PIOc_get_vars_ulonglong(ncid, varid[11], start, count, stride, (unsigned long long *)uint64_array_in)))
            return ret;
        for (x = 0; x < X_DIM_LEN; x++)
            for (y = 0; y < Y_DIM_LEN; y++)
            {
                if (ubyte_array_in[x][y] != ubyte_array[x][y])
                    return ERR_WRONG;
                if (ushort_array_in[x][y] != ushort_array[x][y])
                    return ERR_WRONG;
                if (uint_array_in[x][y] != uint_array[x][y])
                    return ERR_WRONG;
                if (int64_array_in[x][y] != int64_array[x][y])
                    return ERR_WRONG;
                if (uint64_array_in[x][y] != uint64_array[x][y])
                    return ERR_WRONG;
            }
    }

    return 0;
}

/* Use the no-type vara functions to read some data from an open test file. */
int putget_read_vara_nt(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                        int flavor)
{
    signed char byte_array_in[X_DIM_LEN][Y_DIM_LEN];
    char text_array_in[X_DIM_LEN][Y_DIM_LEN];
    short short_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned char ubyte_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in2[X_DIM_LEN][Y_DIM_LEN];
    float float_array_in[X_DIM_LEN][Y_DIM_LEN];
    double double_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned short ushort_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned int uint_array_in[X_DIM_LEN][Y_DIM_LEN];
    long long int64_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned long long uint64_array_in[X_DIM_LEN][Y_DIM_LEN];
    int x, y;
    int ret;

    if ((ret = PIOc_get_vara(ncid, varid[0], start, count, (signed char *)byte_array_in)))
        return ret;
    if ((ret = PIOc_get_vara(ncid, varid[1], start, count, (char *)text_array_in)))
        return ret;
    if ((ret = PIOc_get_vara(ncid, varid[2], start, count, (short *)short_array_in)))
        return ret;
    if ((ret = PIOc_get_vara(ncid, varid[3], start, count, (int *)int_array_in)))
        return ret;
    if ((ret = PIOc_get_vara(ncid, varid[4], start, count, (int *)int_array_in2)))
        return ret;
    if ((ret = PIOc_get_vara(ncid, varid[5], start, count, (float *)float_array_in)))
        return ret;
    if ((ret = PIOc_get_vara(ncid, varid[6], start, count, (double *)double_array_in)))
        return ret;

    for (x = 0; x < X_DIM_LEN; x++)
    {
        if (strncmp(text_array_in[x], text, strlen(text)))
            return ERR_WRONG;
        for (y = 0; y < Y_DIM_LEN; y++)
        {
            if (byte_array_in[x][y] != byte_array[x][y])
                return ERR_WRONG;
            if (short_array_in[x][y] != short_array[x][y])
                return ERR_WRONG;
            if (int_array_in[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (int_array_in2[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (float_array_in[x][y] != float_array[x][y])
                return ERR_WRONG;
            if (double_array_in[x][y] != double_array[x][y])
                return ERR_WRONG;
        }
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_vara(ncid, varid[7], start, count, (unsigned char *)ubyte_array_in)))
            return ret;

        if ((ret = PIOc_get_vara(ncid, varid[8], start, count, (unsigned short *)ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_vara(ncid, varid[9], start, count, (unsigned int *)uint_array_in)))
            return ret;
        if ((ret = PIOc_get_vara(ncid, varid[10], start, count, (long long *)int64_array_in)))
            return ret;
        if ((ret = PIOc_get_vara(ncid, varid[11], start, count, (unsigned long long *)uint64_array_in)))
            return ret;
        for (x = 0; x < X_DIM_LEN; x++)
            for (y = 0; y < Y_DIM_LEN; y++)
            {
                if (ubyte_array_in[x][y] != ubyte_array[x][y])
                    return ERR_WRONG;
                if (ushort_array_in[x][y] != ushort_array[x][y])
                    return ERR_WRONG;
                if (uint_array_in[x][y] != uint_array[x][y])
                    return ERR_WRONG;
                if (int64_array_in[x][y] != int64_array[x][y])
                    return ERR_WRONG;
                if (uint64_array_in[x][y] != uint64_array[x][y])
                    return ERR_WRONG;
            }
    }

    return 0;
}

/* Use the vars functions to read some data from an open test file. */
int putget_read_vars_nt(int ncid, int *varid, PIO_Offset *start, PIO_Offset *count,
                        PIO_Offset *stride, int flavor)
{
    signed char byte_array_in[X_DIM_LEN][Y_DIM_LEN];
    char text_array_in[X_DIM_LEN][Y_DIM_LEN];
    short short_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned char ubyte_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in[X_DIM_LEN][Y_DIM_LEN];
    int int_array_in2[X_DIM_LEN][Y_DIM_LEN];
    float float_array_in[X_DIM_LEN][Y_DIM_LEN];
    double double_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned short ushort_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned int uint_array_in[X_DIM_LEN][Y_DIM_LEN];
    long long int64_array_in[X_DIM_LEN][Y_DIM_LEN];
    unsigned long long uint64_array_in[X_DIM_LEN][Y_DIM_LEN];
    int x, y;
    int ret;

    if ((ret = PIOc_get_vars(ncid, varid[0], start, count, stride, (signed char *)byte_array_in)))
        return ret;
    if ((ret = PIOc_get_vars(ncid, varid[1], start, count, stride, (char *)text_array_in)))
        return ret;
    if ((ret = PIOc_get_vars(ncid, varid[2], start, count, stride, (short *)short_array_in)))
        return ret;
    if ((ret = PIOc_get_vars(ncid, varid[3], start, count, stride, (int *)int_array_in)))
        return ret;
    if ((ret = PIOc_get_vars(ncid, varid[4], start, count, stride, (int *)int_array_in2)))
        return ret;
    if ((ret = PIOc_get_vars(ncid, varid[5], start, count, stride, (float *)float_array_in)))
        return ret;
    if ((ret = PIOc_get_vars(ncid, varid[6], start, count, stride, (double *)double_array_in)))
        return ret;

    for (x = 0; x < X_DIM_LEN; x++)
    {
        if (strncmp(text_array_in[x], text, strlen(text)))
            return ERR_WRONG;
        for (y = 0; y < Y_DIM_LEN; y++)
        {
            if (byte_array_in[x][y] != byte_array[x][y])
                return ERR_WRONG;
            if (short_array_in[x][y] != short_array[x][y])
                return ERR_WRONG;
            if (int_array_in[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (int_array_in2[x][y] != int_array[x][y])
                return ERR_WRONG;
            if (float_array_in[x][y] != float_array[x][y])
                return ERR_WRONG;
            if (double_array_in[x][y] != double_array[x][y])
                return ERR_WRONG;
        }
    }

    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_vars(ncid, varid[7], start, count, stride, (unsigned char *)ubyte_array_in)))
            return ret;

        if ((ret = PIOc_get_vars(ncid, varid[8], start, count, stride, (unsigned short *)ushort_array_in)))
            return ret;
        if ((ret = PIOc_get_vars(ncid, varid[9], start, count, stride, (unsigned int *)uint_array_in)))
            return ret;
        if ((ret = PIOc_get_vars(ncid, varid[10], start, count, stride, (long long *)int64_array_in)))
            return ret;
        if ((ret = PIOc_get_vars(ncid, varid[11], start, count, stride, (unsigned long long *)uint64_array_in)))
            return ret;
        for (x = 0; x < X_DIM_LEN; x++)
            for (y = 0; y < Y_DIM_LEN; y++)
            {
                if (ubyte_array_in[x][y] != ubyte_array[x][y])
                    return ERR_WRONG;
                if (ushort_array_in[x][y] != ushort_array[x][y])
                    return ERR_WRONG;
                if (uint_array_in[x][y] != uint_array[x][y])
                    return ERR_WRONG;
                if (int64_array_in[x][y] != int64_array[x][y])
                    return ERR_WRONG;
                if (uint64_array_in[x][y] != uint64_array[x][y])
                    return ERR_WRONG;
            }
    }

    return 0;
}

/* Create a test file for the putget tests to write data to and check
 * by reading it back. In this function we create the file, define the
 * dims and vars, and pass back the ncid.
 *
 * @param iosysid the IO system ID.
 * @param try the number of the test run, 0 for var, 1 for var1, 2 for
 * vara, 3 for vars.
 * @param unlim non-zero if unlimited dimension should be used.
 * @param flavor the PIO IO type.
 * @param dim_len array of length NDIM of the dimension lengths.
 * @param varid array of varids for the variables in the test file.
 * @param filename the name of the test file to create.
 * @param ncidp pointer that gets the ncid of the created file.
 * @returns 0 for success, error code otherwise.
 */
int create_putget_file(int iosysid, int access, int unlim, int flavor, int *dim_len,
                       int *varid, const char *filename, int *ncidp)
{
    int dimids[NDIM];        /* The dimension IDs. */
    int num_vars = NUM_CLASSIC_TYPES + 1;
    int xtype[NUM_NETCDF4_TYPES + 1] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_LONG_INTERNAL,
                                        PIO_FLOAT, PIO_DOUBLE, PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64,
                                        PIO_UINT64, PIO_STRING};
    int ncid;
    int ret;

    /* This should not work. */
    if (PIOc_createfile(iosysid + TEST_VAL_42, &ncid, &flavor, filename, PIO_CLOBBER) != PIO_EBADID)
        return ERR_WRONG;

    /* Create the netCDF output file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &flavor, filename, PIO_CLOBBER)))
        return ret;

    /* Are we using unlimited dimension? */
    if (!unlim)
        dim_len[0] = NUM_TIMESTEPS;

    /* Define netCDF dimensions and variable. */
    for (int d = 0; d < NDIM; d++)
        if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
            return ret;

    /* For netcdf-4, there are extra types. */
    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
        num_vars = NUM_NETCDF4_TYPES + 1;

    /* Define variables. */
    for (int v = 0; v < num_vars; v++)
    {
        char var_name[PIO_MAX_NAME + 1];
        snprintf(var_name, PIO_MAX_NAME, "%s_%d", VAR_NAME, xtype[v]);
        /*nc_type my_type = xtype[v] == PIO_LONG_INTERNAL ? PIO_INT : xtype[v];*/
        nc_type my_type;
        if (xtype[v] == PIO_LONG_INTERNAL)
            my_type = PIO_INT;
        else
            my_type = xtype[v];
        if ((ret = PIOc_def_var(ncid, var_name, my_type, NDIM, dimids, &varid[v])))
            return ret;
    }

    /* For the first access, also test attributes. */
    if (access == 0)
        if ((ret = test_write_atts(ncid, varid, flavor)))
            return ret;

    if ((ret = PIOc_enddef(ncid)))
        return ret;

    /* Pass back the ncid. */
    *ncidp = ncid;

    return 0;
}

/** Check the contents of the test file. */
int check_file(int access, int ncid, int *varid, int flavor, PIO_Offset *index, PIO_Offset *start,
               PIO_Offset *count, PIO_Offset *stride, int unlim)
{
    int ret;

    switch (access)
    {
    case 0:
        /* Use the att functions to read some data. */
        if ((ret = test_read_att(ncid, varid, flavor)))
            return ret;

        /* Use the vara functions to read some data. */
        if ((ret = putget_read_var(ncid, varid, unlim, flavor)))
            return ret;
        break;

    case 1:
        /* Use the var1 functions to read some data. */
        if ((ret = putget_read_var1(ncid, varid, index, flavor)))
            return ret;
        break;

    case 2:
        /* Use the vara functions to read some data. */
        if ((ret = putget_read_vara(ncid, varid, start, count, flavor)))
            return ret;
        break;

    case 3:
        /* Use the vara functions to read some data. */
        if ((ret = putget_read_vars(ncid, varid, start, count, stride, flavor)))
            return ret;
        break;

    case 4:
        /* Use the var1 functions to read some data. */
        if ((ret = putget_read_var_nt(ncid, varid, unlim, flavor)))
            return ret;
        break;

    case 5:
        /* Use the var1 functions to read some data. */
        if ((ret = putget_read_var1_nt(ncid, varid, index, flavor)))
            return ret;
        break;

    case 6:
        /* Use the vara functions to read some data. */
        if ((ret = putget_read_vara_nt(ncid, varid, start, count, flavor)))
            return ret;
        break;

    case 7:
        /* Use the vara functions to read some data. */
        if ((ret = putget_read_vars_nt(ncid, varid, start, count, stride, flavor)))
            return ret;
        break;

    }

    return PIO_NOERR;
}

/* Test data read/write operations.
 *
 * This function creates a file with 3 dimensions, with a var of each
 * type. Then it uses the var/var1/vars/vars functions to write, and
 * then read data from the test file.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_putget(int iosysid, int num_flavors, int *flavor, int my_rank,
                MPI_Comm test_comm)
{
    int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

#define NUM_ACCESS 8
    for (int unlim = 0; unlim < 2; unlim++)
        for (int access = 0; access < NUM_ACCESS; access++)
        {
            /* Use PIO to create the example file in each of the four
             * available ways. */
            for (int fmt = 0; fmt < num_flavors; fmt++)
            {
                char filename[PIO_MAX_NAME + 1]; /* Test filename. */
                char iotype_name[PIO_MAX_NAME + 1];
                int ncid;
                int varid[NUM_NETCDF4_TYPES + 1];
                int ret;    /* Return code. */

                /* Create a filename. */
                if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
                    return ret;
                snprintf(filename, PIO_MAX_NAME, "%s_putget_access_%d_unlim_%d_%s.nc", TEST_NAME,
                         access, unlim, iotype_name);

                /* Create test file with dims and vars defined. */
                if ((ret = create_putget_file(iosysid, access, unlim, flavor[fmt], dim_len, varid,
                                              filename, &ncid)))
                    return ret;

                /* Write some data. */
                PIO_Offset index[NDIM] = {0, 0, 0};
                PIO_Offset start[NDIM] = {0, 0, 0};
                PIO_Offset count[NDIM] = {1, X_DIM_LEN, Y_DIM_LEN};
                PIO_Offset stride[NDIM] = {1, 1, 1};

                switch (access)
                {
                case 0:
                    /* Use the var functions to write some data. */
                    if ((ret = putget_write_var(ncid, varid, flavor[fmt])))
                        return ret;
                    break;

                case 1:
                    /* Use the var1 functions to write some data. */
                    if ((ret = putget_write_var1(ncid, varid, index, flavor[fmt])))
                        return ret;
                    break;

                case 2:
                    /* Use the vara functions to write some data. */
                    if ((ret = putget_write_vara(ncid, varid, start, count, flavor[fmt])))
                        return ret;
                    break;

                case 3:
                    /* Use the vara functions to write some data. */
                    if ((ret = putget_write_vars(ncid, varid, start, count, stride, flavor[fmt])))
                        return ret;
                    break;

                case 4:
                    /* Use the no-type var functions to write some data. */
                    if ((ret = putget_write_var_nt(ncid, varid, flavor[fmt])))
                        return ret;
                    break;

                case 5:
                    /* Use the no-type var1 functions to write some data. */
                    if ((ret = putget_write_var1_nt(ncid, varid, index, flavor[fmt])))
                        return ret;
                    break;

                case 6:
                    /* Use the no-type vara functions to write some data. */
                    if ((ret = putget_write_vara_nt(ncid, varid, start, count, flavor[fmt])))
                        return ret;
                    break;

                case 7:
                    /* Use the no-type vars functions to write some data. */
                    if ((ret = putget_write_vars_nt(ncid, varid, start, count, stride, flavor[fmt])))
                        return ret;
                    break;

                }

                /* Check for bad input handling. */
                if (PIOc_sync(ncid + TEST_VAL_42) != PIO_EBADID)
                    return ret;

                /* Make sure all data are written (pnetcdf needs this). */
                if ((ret = PIOc_sync(ncid)))
                    return ret;

                /* Check contents of the file. */
                if ((ret = check_file(access, ncid, varid, flavor[fmt], index, start, count, stride, unlim)))
                    return ret;

                /* Close the netCDF file. */
                if ((ret = PIOc_closefile(ncid)))
                    ERR(ret);

                /* /\* Access to read it. *\/ */
                if ((ret = PIOc_openfile2(iosysid, &ncid, &(flavor[fmt]), filename, PIO_NOWRITE)))
                    ERR(ret);

                /* Check contents of the file. */
                if ((ret = check_file(access, ncid, varid, flavor[fmt], index, start, count, stride, unlim)))
                    return ret;

                /* Close the netCDF file. */
                if ((ret = PIOc_closefile(ncid)))
                    ERR(ret);

            } /* next flavor */
        } /* next access */

    return PIO_NOERR;
}

/* Run all the tests. */
int test_all(int iosysid, int num_flavors, int *flavor, int my_rank, MPI_Comm test_comm,
             int async)
{
    int my_test_size;
    int ret; /* Return code. */

    if ((ret = MPI_Comm_size(test_comm, &my_test_size)))
        MPIERR(ret);

    /* Test attribute stuff. */
    if ((ret = test_atts_byte(iosysid, num_flavors, flavor, my_rank, test_comm)))
        return ret;

    if ((ret = test_atts_int64(iosysid, num_flavors, flavor, my_rank, test_comm)))
        return ret;

    /* Test read/write stuff. */
    if ((ret = test_putget(iosysid, num_flavors, flavor, my_rank, test_comm)))
        return ret;

    return PIO_NOERR;
}

/* Run Tests for NetCDF-4 Functions. */
int main(int argc, char **argv)
{
    /* Initialize data arrays with sample data. */
    init_arrays();

    return run_test_main(argc, argv, MIN_NTASKS, TARGET_NTASKS, -1,
                         TEST_NAME, dim_len, COMPONENT_COUNT, NUM_IO_PROCS);

    return 0;
}
