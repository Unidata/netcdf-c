/*
 * Common test code for PIO C tests.
 *
 * Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>
#include <pio_tests.h>

/* The number of dimensions in the test data. */
#define NDIM_S1 1

/* The length of our test data. */
#define DIM_LEN_S1 4

/* The name of the dimension in the netCDF output file. */
#define FIRST_DIM_NAME_S1 "jojo"
#define DIM_NAME_S1 "dim_sample_s1"

/* The name of the variable in the netCDF output file. */
#define FIRST_VAR_NAME_S1 "bill"
#define VAR_NAME_S1 "var_sample_s1"

/* The number of dimensions in the sample 2 test data. */
#define NDIM_S2 1

/* The length of our sample 2 test data. */
#define DIM_LEN_S2 4

/* The name of the dimension in the sample 2 output file. */
#define FIRST_DIM_NAME_S2 "jojo"
#define DIM_NAME_S2 "dim_sample_s2"

/* The name of the variable in the sample 2 output file. */
#define FIRST_VAR_NAME_S2 "bill"
#define VAR_NAME_S2 "var_sample_s2"

/* The name of the global attribute in the sample 2 output file. */
#define FIRST_ATT_NAME_S2 "willy_gatt_sample s2"
#define ATT_NAME_S2 "gatt_sample s2"
#define SHORT_ATT_NAME_S2 "short_gatt_sample s2"
#define FLOAT_ATT_NAME_S2 "float_gatt_sample s2"
#define DOUBLE_ATT_NAME_S2 "double_gatt_sample s2"

/* The value of the global attribute in the sample 2 output file. */
#define ATT_VALUE_S2 42

/* Attribute name. */
#define GLOBAL_ATT_NAME "global_att"

/* The names of the variables created in test file. */
#define SCALAR_VAR_NAME "scalar_var"
#define TWOD_VAR_NAME "twod_var"

/* Used to create dimension names. */
#define DIM_NAME "dim"

/* Number of vars in test file. */
#define NVAR 3

/* The names of the variables created in test file. */
#define SCALAR_VAR_NAME "scalar_var"
#define THREED_VAR_NAME "threed_var"

/* Dimension lengths. */
#define DIM_X_LEN 2
#define DIM_Y_LEN 3

/* Length of all attributes. */
#define ATT_LEN 3

#ifdef _NETCDF4
int pio_type[NUM_PIO_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE,
                                       PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
int pio_type[NUM_PIO_TYPES_TO_TEST] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT, PIO_DOUBLE};
#endif /* _NETCDF4 */

/* Attribute test data. */
signed char byte_att_data[ATT_LEN] = {NC_MAX_BYTE, NC_MIN_BYTE, NC_MAX_BYTE};
char char_att_data[ATT_LEN] = {NC_MAX_CHAR, 0, NC_MAX_CHAR};
short short_att_data[ATT_LEN] = {NC_MAX_SHORT, NC_MIN_SHORT, NC_MAX_SHORT};
int int_att_data[ATT_LEN] = {NC_MAX_INT, NC_MIN_INT, NC_MAX_INT};
float float_att_data[ATT_LEN] = {NC_MAX_FLOAT, NC_MIN_FLOAT, NC_MAX_FLOAT};
double double_att_data[ATT_LEN] = {NC_MAX_DOUBLE, NC_MIN_DOUBLE, NC_MAX_DOUBLE};
#ifdef _NETCDF4
unsigned char ubyte_att_data[ATT_LEN] = {NC_MAX_UBYTE, 0, NC_MAX_UBYTE};
unsigned short ushort_att_data[ATT_LEN] = {NC_MAX_USHORT, 0, NC_MAX_USHORT};
unsigned int uint_att_data[ATT_LEN] = {NC_MAX_UINT, 0, NC_MAX_UINT};
long long int64_att_data[ATT_LEN] = {NC_MAX_INT64, NC_MIN_INT64, NC_MAX_INT64};
unsigned long long uint64_att_data[ATT_LEN] = {NC_MAX_UINT64, 0, NC_MAX_UINT64};
#endif /* _NETCDF4 */

/* Scalar variable test data. */
signed char byte_scalar_data = NC_MAX_BYTE;
char char_scalar_data = NC_MAX_CHAR;
short short_scalar_data = NC_MAX_SHORT;
int int_scalar_data = NC_MAX_INT;
float float_scalar_data = NC_MAX_FLOAT;
double double_scalar_data = NC_MAX_DOUBLE;
#ifdef _NETCDF4
unsigned char ubyte_scalar_data = NC_MAX_UBYTE;
unsigned short ushort_scalar_data = NC_MAX_USHORT;
unsigned int uint_scalar_data = NC_MAX_UINT;
long long int64_scalar_data = NC_MAX_INT64;
unsigned long long uint64_scalar_data = NC_MAX_UINT64;
#endif /* _NETCDF4 */

/* Pointers to the data. */
#ifdef _NETCDF4
void *att_data[NUM_PIO_TYPES_TO_TEST] = {byte_att_data, char_att_data, short_att_data,
                                         int_att_data, float_att_data, double_att_data,
                                         ubyte_att_data, ushort_att_data, uint_att_data,
                                         int64_att_data, uint64_att_data};
void *scalar_data[NUM_PIO_TYPES_TO_TEST] = {&byte_scalar_data, &char_scalar_data, &short_scalar_data,
                                            &int_scalar_data, &float_scalar_data, &double_scalar_data,
                                            &ubyte_scalar_data, &ushort_scalar_data, &uint_scalar_data,
                                            &int64_scalar_data, &uint64_scalar_data};
#else
void *att_data[NUM_PIO_TYPES_TO_TEST] = {byte_att_data, char_att_data, short_att_data,
                                         int_att_data, float_att_data, double_att_data};
void *scalar_data[NUM_PIO_TYPES_TO_TEST] = {&byte_scalar_data, &char_scalar_data, &short_scalar_data,
                                            &int_scalar_data, &float_scalar_data, &double_scalar_data};
#endif /* _NETCDF4 */

/* How many flavors of netCDF are available? */
int
get_iotypes(int *num_flavors, int *flavors)
{
    int num = 0;
    int fmtidx = 0;
    int format[NUM_FLAVORS];

#ifdef _PNETCDF
    num++;
    format[fmtidx++] = PIO_IOTYPE_PNETCDF;
#endif
#ifdef _NETCDF
    num++;
    format[fmtidx++] = PIO_IOTYPE_NETCDF;
#endif
#ifdef _NETCDF4
    num += 2;
    format[fmtidx++] = PIO_IOTYPE_NETCDF4C;
    format[fmtidx] = PIO_IOTYPE_NETCDF4P;
#endif

    /* Pass results back to caller. */
    if (num_flavors)
        *num_flavors = num;
    if (flavors)
        for (int f = 0; f < num; f++)
            flavors[f] = format[f];

    return PIO_NOERR;
}

/* Name of each flavor.
 *
 * @param iotype the IO type
 * @param name pointer that will get name of IO type. Must have enough
 * memory allocated (NC_MAX_NAME + 1 works.)
 * @returns 0 for success, error code otherwise.
 * @internal
 */
int get_iotype_name(int iotype, char *name)
{
    char flavor_name[NUM_FLAVORS][NC_MAX_NAME + 1] = {"pnetcdf", "classic",
                                                      "serial4", "parallel4"};

    /* Check inputs. */
    if (!name || iotype < PIO_IOTYPE_PNETCDF || iotype > PIO_IOTYPE_NETCDF4P)
        return PIO_EINVAL;

    /* Return name of iotype. They are numbered 1-4 in pio.h. */
    strcpy(name, flavor_name[iotype - 1]);

    return PIO_NOERR;
}

/* Initalize the test system.
 *
 * @param argc argument count from main().
 * @param argv argument array from main().
 * @param my_rank pointer that gets this tasks rank.
 * @param ntasks pointer that gets the number of tasks in WORLD
 * communicator.
 * @param target_ntasks the number of tasks this test needs to run.
 * @param log_level PIOc_set_log_level() will be called with this value.
 * @param comm a pointer to an MPI communicator that will be created
 * for this test and contain target_ntasks tasks from WORLD.
 * @returns 0 for success, error code otherwise.
 */
int pio_test_init2(int argc, char **argv, int *my_rank, int *ntasks,
                   int min_ntasks, int max_ntasks, int log_level, MPI_Comm *comm)
{
    int ret; /* Return value. */

#ifdef TIMING
    /* Initialize the GPTL timing library. */
    if ((ret = GPTLinitialize()))
        return ERR_GPTL;
#endif

    /* Initialize MPI. */
    if ((ret = MPI_Init(&argc, &argv)))
        MPIERR(ret);

    /* Learn my rank and the total number of processors. */
    if ((ret = MPI_Comm_rank(MPI_COMM_WORLD, my_rank)))
        MPIERR(ret);
    if ((ret = MPI_Comm_size(MPI_COMM_WORLD, ntasks)))
        MPIERR(ret);

    /* Check that a valid number of processors was specified. */
    if (*ntasks < min_ntasks)
    {
        fprintf(stderr, "ERROR: Number of processors must be at least %d for this test!\n",
                min_ntasks);
        return ERR_AWFUL;
    }
    else if (*ntasks > max_ntasks)
    {
        /* If more tasks are available than we need for this test,
         * create a communicator with exactly the number of tasks we
         * need. */
        int color, key;
        if (*my_rank < max_ntasks)
        {
            color = 0;
            key = *my_rank;
        }
        else
        {
            color = 1;
            key = *my_rank - max_ntasks;
        }
        if ((ret = MPI_Comm_split(MPI_COMM_WORLD, color, key, comm)))
            MPIERR(ret);
    }
    else
    {
        if ((ret = MPI_Comm_dup(MPI_COMM_WORLD, comm)))
            MPIERR(ret);
    }

    /* Turn on logging. */
    if ((ret = PIOc_set_log_level(log_level)))
        return ret;

    /* Change error handling so we can test inval parameters. */
    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    return PIO_NOERR;
}

/* Finalize a PIO C test.
 *
 * @param test_comm pointer to the test communicator.
 * @returns 0 for success, error code otherwise.
 */
int pio_test_finalize(MPI_Comm *test_comm)
{
    int ret = PIO_NOERR; /* Return value. */

    /* Wait for all processes to arrive here. */
    if (MPI_Barrier(*test_comm))
        return ERR_MPI;

    /* Free communicator. */
    if (MPI_Comm_free(test_comm))
        return ERR_MPI;

    /* Finalize MPI. */
    MPI_Finalize();

#ifdef TIMING
    /* Finalize the GPTL timing library. */
    if ((ret = GPTLfinalize()))
        return ret;
#endif

    return ret;
}

/* Test the inq_format function. */
int
test_inq_format(int ncid, int format)
{
    int myformat;
    int ret;

    /* Get the format of an open file. */
    if ((ret = PIOc_inq_format(ncid, &myformat)))
        return ret;

    /* Check the result. */
    if ((format == PIO_IOTYPE_PNETCDF || format == PIO_IOTYPE_NETCDF) && myformat != 1)
        return ERR_WRONG;
    else if ((format == PIO_IOTYPE_NETCDF4C || format == PIO_IOTYPE_NETCDF4P) &&
             myformat != 3)
        return ERR_WRONG;

    return PIO_NOERR;
}

/* Test the inq_type function for atomic types. */
int
test_inq_type(int ncid, int format)
{
#define NUM_TYPES 11
    char type_name[NC_MAX_NAME + 1];
    PIO_Offset type_size;
    nc_type xtype[NUM_TYPES] = {NC_CHAR, NC_BYTE, NC_SHORT, NC_INT, NC_FLOAT, NC_DOUBLE,
                                NC_UBYTE, NC_USHORT, NC_UINT, NC_INT64, NC_UINT64};
    int type_len[NUM_TYPES] = {1, 1, 2, 4, 4, 8, 1, 2, 4, 8, 8};
    int max_type = format == PIO_IOTYPE_NETCDF ? NC_DOUBLE : NC_UINT64;
    int ret;

    /* Check each type size. */
    for (int i = 0; i < max_type; i++)
    {
        if ((ret = PIOc_inq_type(ncid, xtype[i], type_name, &type_size)))
            return ret;
        if (type_size != type_len[i])
            return ERR_AWFUL;
    }

    return PIO_NOERR;
}

/* This creates a netCDF sample file in the specified format. */
int
create_nc_sample(int sample, int iosysid, int format, char *filename, int my_rank, int *ncid)
{
    switch(sample)
    {
    case 0:
        return create_nc_sample_0(iosysid, format, filename, my_rank, ncid);
        break;
    case 1:
        return create_nc_sample_1(iosysid, format, filename, my_rank, ncid);
        break;
    case 2:
        return create_nc_sample_2(iosysid, format, filename, my_rank, ncid);
        break;
    }
    return PIO_EINVAL;
}

/* This checks a netCDF sample file in the specified format. */
int
check_nc_sample(int sample, int iosysid, int format, char *filename, int my_rank, int *ncid)
{
    switch(sample)
    {
    case 0:
        return check_nc_sample_0(iosysid, format, filename, my_rank, ncid);
        break;
    case 1:
        return check_nc_sample_1(iosysid, format, filename, my_rank, ncid);
        break;
    case 2:
        return check_nc_sample_2(iosysid, format, filename, my_rank, ncid);
        break;
    }
    return PIO_EINVAL;
}

/* This creates an empty netCDF file in the specified format. */
int
create_nc_sample_0(int iosysid, int format, char *filename, int my_rank, int *ncidp)
{
    int ncid;
    int ret;

    /* Create the file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &format, filename, NC_CLOBBER)))
        return ret;

    /* End define mode. */
    if ((ret = PIOc_enddef(ncid)))
        return ret;

    /* Test inq_format. */
    if ((ret = test_inq_format(ncid, format)))
        return ret;

    /* Test inq_type. */
    if ((ret = test_inq_type(ncid, format)))
        return ret;

    /* Close the file if ncidp was not provided. */
    if (ncidp)
        *ncidp = ncid;
    else
    {
        if ((ret = PIOc_closefile(ncid)))
            return ret;
    }

    return PIO_NOERR;
}

/* Check sample file 1 for correctness. */
int
check_nc_sample_0(int iosysid, int format, char *filename, int my_rank, int *ncidp)
{
    int ncid;
    int ndims, nvars, ngatts, unlimdimid;
    int ndims2, nvars2, ngatts2, unlimdimid2;
    int ret;

    /* Re-open the file to check it. */
    if ((ret = PIOc_openfile2(iosysid, &ncid, &format, filename,
                              NC_NOWRITE)))
        return ret;

    /* Find the number of dimensions, variables, and global attributes.*/
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        return ret;
    if (ndims != 0 || nvars != 0 || ngatts != 0 || unlimdimid != -1)
        return ERR_WRONG;

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_ndims(ncid, &ndims2)))
        return ret;
    if (ndims2 != 0)
        return ERR_WRONG;
    if ((ret = PIOc_inq_nvars(ncid, &nvars2)))
        return ret;
    if (nvars2 != 0)
        return ERR_WRONG;
    if ((ret = PIOc_inq_natts(ncid, &ngatts2)))
        return ret;
    if (ngatts2 != 0)
        return ERR_WRONG;
    if ((ret = PIOc_inq_unlimdim(ncid, &unlimdimid2)))
        return ret;
    if (unlimdimid != -1)
        return ERR_WRONG;

    /* Close the file if ncidp was not provided. */
    if (ncidp)
        *ncidp = ncid;
    else
    {
        if ((ret = PIOc_closefile(ncid)))
            return ret;
    }

    return 0;
}

/* This creates a netCDF file in the specified format, with some
 * sample values. */
int
create_nc_sample_1(int iosysid, int format, char *filename, int my_rank, int *ncidp)
{
    /* The ncid of the netCDF file. */
    int ncid;

    /* The ID of the netCDF varable. */
    int varid;

    /* The ID of the netCDF dimension. */
    int dimid;

    /* Return code. */
    int ret;

    /* Start and count arrays for netCDF. */
    PIO_Offset start[NDIM_S1], count[NDIM_S1] = {0};

    /* The sample data. */
    int data[DIM_LEN_S1];

    /* Create the file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &format, filename, NC_CLOBBER)))
        return ret;

    /* /\* End define mode, then re-enter it. *\/ */
    if ((ret = PIOc_enddef(ncid)))
        return ret;
    if ((ret = PIOc_redef(ncid)))
        return ret;

    /* Define a dimension. */
    if ((ret = PIOc_def_dim(ncid, DIM_NAME_S1, DIM_LEN_S1, &dimid)))
        return ret;

    /* Define a 1-D variable. */
    if ((ret = PIOc_def_var(ncid, VAR_NAME_S1, NC_INT, NDIM_S1, &dimid, &varid)))
        return ret;

    /* End define mode. */
    if ((ret = PIOc_enddef(ncid)))
        return ret;

    /* Write some data. For the PIOc_put/get functions, all data must
     * be on compmaster before the function is called. Only
     * compmaster's arguments are passed to the async msg handler. All
     * other computation tasks are ignored. */
    for (int i = 0; i < DIM_LEN_S1; i++)
        data[i] = i;
    start[0] = 0;
    count[0] = DIM_LEN_S1;
    if ((ret = PIOc_put_vars_tc(ncid, varid, start, count, NULL, NC_INT, data)))
        return ret;

    /* Test inq_format. */
    if ((ret = test_inq_format(ncid, format)))
        return ret;

    /* Test inq_type. */
    if ((ret = test_inq_type(ncid, format)))
        return ret;

    /* Close the file if ncidp was not provided. */
    if (ncidp)
        *ncidp = ncid;
    else
    {
        if ((ret = PIOc_closefile(ncid)))
            return ret;
    }

    return PIO_NOERR;
}

/* Check sample file 1 for correctness. */
int
check_nc_sample_1(int iosysid, int format, char *filename, int my_rank, int *ncidp)
{
    int ncid;
    int ret;
    int ndims, nvars, ngatts, unlimdimid;
    int ndims2, nvars2, ngatts2, unlimdimid2;
    char dimname[NC_MAX_NAME + 1];
    PIO_Offset dimlen;
    char varname[NC_MAX_NAME + 1];
    nc_type vartype;
    int varndims, vardimids, varnatts;

    /* Re-open the file to check it. */
    if ((ret = PIOc_openfile2(iosysid, &ncid, &format, filename,
                              NC_NOWRITE)))
        return ret;

    /* Try to read the data. */
    PIO_Offset start[NDIM_S1] = {0}, count[NDIM_S1] = {DIM_LEN_S1};
    int data_in[DIM_LEN_S1];
    if ((ret = PIOc_get_vars_tc(ncid, 0, start, count, NULL, NC_INT, data_in)))
        return ret;
    for (int i = 0; i < DIM_LEN_S1; i++)
    {
        if (data_in[i] != i)
            return ERR_AWFUL;
    }

    /* Find the number of dimensions, variables, and global attributes.*/
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        return ret;
    if (ndims != 1 || nvars != 1 || ngatts != 0 || unlimdimid != -1)
        return ERR_WRONG;

    /* This should return PIO_NOERR. */
    if ((ret = PIOc_inq(ncid, NULL, NULL, NULL, NULL)))
        return ret;

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_ndims(ncid, &ndims2)))
        return ret;
    if (ndims2 != 1)
        return ERR_WRONG;
    if ((ret = PIOc_inq_nvars(ncid, &nvars2)))
        return ret;
    if (nvars2 != 1)
        return ERR_WRONG;
    if ((ret = PIOc_inq_natts(ncid, &ngatts2)))
        return ret;
    if (ngatts2 != 0)
        return ERR_WRONG;
    if ((ret = PIOc_inq_unlimdim(ncid, &unlimdimid2)))
        return ret;
    if (unlimdimid != -1)
        return ERR_WRONG;

    /* Check out the dimension. */
    if ((ret = PIOc_inq_dim(ncid, 0, dimname, &dimlen)))
        return ret;
    if (strcmp(dimname, DIM_NAME_S1) || dimlen != DIM_LEN_S1)
        return ERR_WRONG;

    /* Check out the variable. */
    if ((ret = PIOc_inq_var(ncid, 0, varname, &vartype, &varndims, &vardimids, &varnatts)))
        return ret;
    if (strcmp(varname, VAR_NAME_S1) || vartype != NC_INT || varndims != NDIM_S1 ||
        vardimids != 0 || varnatts != 0)
        return ERR_WRONG;

    /* Close the file if ncidp was not provided. */
    if (ncidp)
        *ncidp = ncid;
    else
    {
        if ((ret = PIOc_closefile(ncid)))
            return ret;
    }

    return 0;
}

/* This creates a netCDF file in the specified format, with some
 * sample values. */
int
create_nc_sample_2(int iosysid, int format, char *filename, int my_rank, int *ncidp)
{
    int ncid, varid, dimid;
    PIO_Offset start[NDIM_S2], count[NDIM_S2] = {0};
    int data[DIM_LEN_S2];
    int ret;

    /* Create a netCDF file with one dimension and one variable. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &format, filename, NC_CLOBBER)))
        return ret;

    /* End define mode, then re-enter it. */
    if ((ret = PIOc_enddef(ncid)))
        return ret;
    if ((ret = PIOc_redef(ncid)))
        return ret;

    /* Define a dimension. */
    char dimname2[NC_MAX_NAME + 1];
    if ((ret = PIOc_def_dim(ncid, FIRST_DIM_NAME_S2, DIM_LEN_S2, &dimid)))
        return ret;
    if ((ret = PIOc_inq_dimname(ncid, 0, dimname2)))
        return ret;
    if (strcmp(dimname2, FIRST_DIM_NAME_S2))
        return ERR_WRONG;
    if ((ret = PIOc_rename_dim(ncid, 0, DIM_NAME_S2)))
        return ret;

    /* Define a 1-D variable. */
    char varname2[NC_MAX_NAME + 1];
    if ((ret = PIOc_def_var(ncid, FIRST_VAR_NAME_S2, NC_INT, NDIM_S2, &dimid, &varid)))
        return ret;
    if ((ret = PIOc_inq_varname(ncid, 0, varname2)))
        return ret;
    if (strcmp(varname2, FIRST_VAR_NAME_S2))
        return ERR_WRONG;
    if ((ret = PIOc_rename_var(ncid, 0, VAR_NAME_S2)))
        return ret;

    /* Add a global attribute. */
    int att_data = ATT_VALUE_S2;
    short short_att_data = ATT_VALUE_S2;
    float float_att_data = ATT_VALUE_S2;
    double double_att_data = ATT_VALUE_S2;
    char attname2[NC_MAX_NAME + 1];
    /* Write an att and rename it. */
    if ((ret = PIOc_put_att_int(ncid, NC_GLOBAL, FIRST_ATT_NAME_S2, NC_INT, 1, &att_data)))
        return ret;
    if ((ret = PIOc_inq_attname(ncid, NC_GLOBAL, 0, attname2)))
        return ret;
    if (strcmp(attname2, FIRST_ATT_NAME_S2))
        return ERR_WRONG;
    if ((ret = PIOc_rename_att(ncid, NC_GLOBAL, FIRST_ATT_NAME_S2, ATT_NAME_S2)))
        return ret;

    /* Write an att and delete it. */
    if ((ret = PIOc_put_att_int(ncid, NC_GLOBAL, FIRST_ATT_NAME_S2, NC_INT, 1, &att_data)))
        return ret;
    if ((ret = PIOc_del_att(ncid, NC_GLOBAL, FIRST_ATT_NAME_S2)))
        return ret;
    /* if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, FIRST_ATT_NAME_S2, NULL, NULL)) != PIO_ENOTATT) */
    /* { */
    /*  printf("ret = %d\n", ret); */
    /*  return ERR_AWFUL; */
    /* } */

    /* Write some atts of different types. */
    if ((ret = PIOc_put_att_short(ncid, NC_GLOBAL, SHORT_ATT_NAME_S2, NC_SHORT, 1, &short_att_data)))
        return ret;
    if ((ret = PIOc_put_att_float(ncid, NC_GLOBAL, FLOAT_ATT_NAME_S2, NC_FLOAT, 1, &float_att_data)))
        return ret;
    if ((ret = PIOc_put_att_double(ncid, NC_GLOBAL, DOUBLE_ATT_NAME_S2, NC_DOUBLE, 1, &double_att_data)))
        return ret;

    /* End define mode. */
    if ((ret = PIOc_enddef(ncid)))
        return ret;

    /* Write some data. For the PIOc_put/get functions, all data must
     * be on compmaster before the function is called. Only
     * compmaster's arguments are passed to the async msg handler. All
     * other computation tasks are ignored. */
    for (int i = 0; i < DIM_LEN_S2; i++)
        data[i] = i;
    start[0] = 0;
    count[0] = DIM_LEN_S2;
    if ((ret = PIOc_put_vars_tc(ncid, varid, start, count, NULL, NC_INT, data)))
        return ret;

    /* Close the file if ncidp was not provided. */
    if (ncidp)
        *ncidp = ncid;
    else
    {
        if ((ret = PIOc_closefile(ncid)))
            return ret;
    }

    return PIO_NOERR;
}

/* Check sample file 2 for correctness.
 *
 * @param ncidp if NULL, close file, otherwise return ncid of still-open file.
 * @reaturns 0 for success and error code otherwise.
 */
int
check_nc_sample_2(int iosysid, int format, char *filename, int my_rank, int *ncidp)
{
    int ncid;
    int ret;
    int ndims, nvars, ngatts, unlimdimid;
    int ndims2, nvars2, ngatts2, unlimdimid2;
    int dimid2;
    char dimname[NC_MAX_NAME + 1];
    PIO_Offset dimlen;
    char dimname2[NC_MAX_NAME + 1];
    PIO_Offset dimlen2;
    char varname[NC_MAX_NAME + 1];
    nc_type vartype;
    int varndims, vardimids, varnatts;
    char varname2[NC_MAX_NAME + 1];
    nc_type vartype2;
    int varndims2, vardimids2, varnatts2;
    int varid2;
    int att_data;
    short short_att_data;
    float float_att_data;
    double double_att_data;
    nc_type atttype;
    PIO_Offset attlen;
    char myattname[NC_MAX_NAME + 1];
    int myid;
    PIO_Offset start[NDIM_S2] = {0}, count[NDIM_S2] = {DIM_LEN_S2};
    int data_in[DIM_LEN_S2];

    /* Re-open the file to check it. */
    if ((ret = PIOc_openfile2(iosysid, &ncid, &format, filename, NC_NOWRITE)))
        return ERR_CHECK;

    /* Try to read the data. */
    if ((ret = PIOc_get_vars_tc(ncid, 0, start, count, NULL, NC_INT, data_in)))
        return ERR_CHECK;
    for (int i = 0; i < DIM_LEN_S2; i++)
    {
        if (data_in[i] != i)
            return ERR_AWFUL;
    }

    /* Find the number of dimensions, variables, and global attributes.*/
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        return ERR_CHECK;
    if (ndims != 1 || nvars != 1 || ngatts != 4 || unlimdimid != -1)
        return ERR_WRONG;

    /* This should return PIO_NOERR. */
    if ((ret = PIOc_inq(ncid, NULL, NULL, NULL, NULL)))
        return ERR_CHECK;

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_ndims(ncid, &ndims2)))
        return ERR_CHECK;
    if (ndims2 != 1)
        return ERR_WRONG;
    if ((ret = PIOc_inq_nvars(ncid, &nvars2)))
        return ERR_CHECK;
    if (nvars2 != 1)
        return ERR_WRONG;
    if ((ret = PIOc_inq_natts(ncid, &ngatts2)))
        return ERR_CHECK;
    if (ngatts2 != 4)
        return ERR_WRONG;
    if ((ret = PIOc_inq_unlimdim(ncid, &unlimdimid2)))
        return ERR_CHECK;
    if (unlimdimid != -1)
        return ERR_WRONG;
    /* Should succeed, do nothing. */
    if ((ret = PIOc_inq_unlimdim(ncid, NULL)))
        return ERR_CHECK;

    /* Check out the dimension. */
    if ((ret = PIOc_inq_dim(ncid, 0, dimname, &dimlen)))
        return ERR_CHECK;
    if (strcmp(dimname, DIM_NAME_S2) || dimlen != DIM_LEN_S2)
        return ERR_WRONG;

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_dimname(ncid, 0, dimname2)))
        return ERR_CHECK;
    if (strcmp(dimname2, DIM_NAME_S2))
        return ERR_WRONG;
    if ((ret = PIOc_inq_dimlen(ncid, 0, &dimlen2)))
        return ERR_CHECK;
    if (dimlen2 != DIM_LEN_S2)
        return ERR_WRONG;
    if ((ret = PIOc_inq_dimid(ncid, DIM_NAME_S2, &dimid2)))
        return ERR_CHECK;
    if (dimid2 != 0)
        return ERR_WRONG;

    /* Check out the variable. */
    if ((ret = PIOc_inq_var(ncid, 0, varname, &vartype, &varndims, &vardimids, &varnatts)))
        return ERR_CHECK;
    if (strcmp(varname, VAR_NAME_S2) || vartype != NC_INT || varndims != NDIM_S2 ||
        vardimids != 0 || varnatts != 0)
        return ERR_WRONG;

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_varname(ncid, 0, varname2)))
        return ERR_CHECK;
    if (strcmp(varname2, VAR_NAME_S2))
        return ERR_WRONG;
    if ((ret = PIOc_inq_vartype(ncid, 0, &vartype2)))
        return ERR_CHECK;
    if (vartype2 != NC_INT)
        return ERR_WRONG;
    if ((ret = PIOc_inq_varndims(ncid, 0, &varndims2)))
        return ERR_CHECK;
    if (varndims2 != NDIM_S2)
        return ERR_WRONG;
    if ((ret = PIOc_inq_vardimid(ncid, 0, &vardimids2)))
        return ERR_CHECK;
    if (vardimids2 != 0)
        return ERR_WRONG;
    if ((ret = PIOc_inq_varnatts(ncid, 0, &varnatts2)))
        return ERR_CHECK;
    if (varnatts2 != 0)
        return ERR_WRONG;
    if ((ret = PIOc_inq_varid(ncid, VAR_NAME_S2, &varid2)))
        return ERR_CHECK;
    if (varid2 != 0)
        return ERR_WRONG;

    /* Check out the global attributes. */
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, ATT_NAME_S2, &atttype, &attlen)))
        return ERR_CHECK;
    if (atttype != NC_INT || attlen != 1)
        return ERR_WRONG;
    if ((ret = PIOc_inq_attlen(ncid, NC_GLOBAL, ATT_NAME_S2, &attlen)))
        return ERR_CHECK;
    if (attlen != 1)
        return ERR_WRONG;
    if ((ret = PIOc_inq_attname(ncid, NC_GLOBAL, 0, myattname)))
        return ERR_CHECK;
    if (strcmp(ATT_NAME_S2, myattname))
        return ERR_WRONG;
    if ((ret = PIOc_inq_attid(ncid, NC_GLOBAL, ATT_NAME_S2, &myid)))
        return ERR_CHECK;
    if (myid != 0)
        return ERR_WRONG;
    if ((ret = PIOc_get_att_int(ncid, NC_GLOBAL, ATT_NAME_S2, &att_data)))
        return ERR_CHECK;
    if (att_data != ATT_VALUE_S2)
        return ERR_WRONG;
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, SHORT_ATT_NAME_S2, &atttype, &attlen)))
        return ERR_CHECK;
    if (atttype != NC_SHORT || attlen != 1)
        return ERR_WRONG;
    if ((ret = PIOc_get_att_short(ncid, NC_GLOBAL, SHORT_ATT_NAME_S2, &short_att_data)))
        return ERR_CHECK;
    if (short_att_data != ATT_VALUE_S2)
        return ERR_WRONG;
    if ((ret = PIOc_get_att_float(ncid, NC_GLOBAL, FLOAT_ATT_NAME_S2, &float_att_data)))
        return ERR_CHECK;
    if (float_att_data != ATT_VALUE_S2)
        return ERR_WRONG;
    if ((ret = PIOc_get_att_double(ncid, NC_GLOBAL, DOUBLE_ATT_NAME_S2, &double_att_data)))
        return ERR_CHECK;
    if (double_att_data != ATT_VALUE_S2)
        return ERR_WRONG;

    /* Close the file if ncidp was not provided. */
    if (ncidp)
        *ncidp = ncid;
    else
    {
        if ((ret = PIOc_closefile(ncid)))
            return ERR_CHECK;
    }

    return 0;
}

/* Create the decomposition to divide the 3-dimensional sample data
 * between tasks. For the purposes of decomposition we are only
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
int create_decomposition_2d(int ntasks, int my_rank, int iosysid, int *dim_len_2d,
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

/*
 * This creates a test netCDF file in the specified format. This file
 * is simple, with a global attribute, 2 dimensions, a scalar var, and
 * a 2D var.
 *
 * @param iosysid identifies the IO system.
 * @param iotype the iotype to be used to create the file.
 * @param my_rank rank of this task in world (for debugging messages
 * only).
 * @param my_comp_idx the index of the computational component
 * creating the file.
 * @param filename pointer to buffer that will get filename. Must be
 * PIO_MAX_NAME + 1 in size.
 * @param test_name name of the test program.
 * @param verbose non-zero to turn on printf statements.
 * @param use_darray if non-zero, use darray functions to write data,
 * otherwise use PIOc_put_var().
 * @param ioid the decomposition ID to use if darrays are used to
 * write data.
 * @returns 0 for success, error code otherwise.
 */
int create_nc_sample_3(int iosysid, int iotype, int my_rank, int my_comp_idx,
                       char *filename, char *test_name, int verbose, int use_darray,
                       int ioid)
{
    char iotype_name[NC_MAX_NAME + 1];
    int ncid;
    signed char my_char_comp_idx = my_comp_idx;
    int varid[NVAR];
    char att_name[PIO_MAX_NAME + 1];
    char var_name[PIO_MAX_NAME + 1];
    char dim_name[PIO_MAX_NAME + 1];
    int dimid[NDIM3];
    int dim_len[NDIM3] = {NC_UNLIMITED, DIM_X_LEN, DIM_Y_LEN};
    short data_2d[DIM_X_LEN * DIM_Y_LEN];
    int ret;

    /* Learn name of IOTYPE. */
    if ((ret = get_iotype_name(iotype, iotype_name)))
        ERR(ret);

    /* Create a filename. */
    sprintf(filename, "%s_%s_cmp_%d_darray_%d.nc", test_name, iotype_name, my_comp_idx,
            use_darray);
    if (verbose)
        printf("my_rank %d creating test file %s for iosysid %d\n", my_rank, filename, iosysid);

    /* Create the file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &iotype, filename, NC_CLOBBER)))
        ERR(ret);

    /* Create a global attribute. */
    sprintf(att_name, "%s_%d", GLOBAL_ATT_NAME, my_comp_idx);
    if ((ret = PIOc_put_att_schar(ncid, PIO_GLOBAL, att_name, PIO_BYTE, 1, &my_char_comp_idx)))
        ERR(ret);

    /* Define a scalar variable. */
    sprintf(var_name, "%s_%d", SCALAR_VAR_NAME, my_comp_idx);
    if ((ret = PIOc_def_var(ncid, var_name, PIO_INT, 0, NULL, &varid[0])))
        ERR(ret);

    /* Define dimensions. */
    for (int d = 0; d < NDIM3; d++)
    {
        sprintf(dim_name, "%s_%d_cmp_%d", DIM_NAME, d, my_comp_idx);
        if ((ret = PIOc_def_dim(ncid, dim_name, dim_len[d], &dimid[d])))
            ERR(ret);
    }

    /* Define a 2D variable. */
    sprintf(var_name, "%s_%d", TWOD_VAR_NAME, my_comp_idx);
    if ((ret = PIOc_def_var(ncid, var_name, PIO_SHORT, NDIM2, &dimid[1], &varid[1])))
        ERR(ret);

    /* Define a 3D variable. */
    sprintf(var_name, "%s_%d", THREED_VAR_NAME, my_comp_idx);
    if ((ret = PIOc_def_var(ncid, var_name, PIO_SHORT, NDIM3, dimid, &varid[2])))
        ERR(ret);
    
    /* End define mode. */
    if ((ret = PIOc_enddef(ncid)))
        ERR(ret);

    /* Write the scalar variable. */
    if ((ret = PIOc_put_var_int(ncid, 0, &my_comp_idx)))
        ERR(ret);

    /* Create some 2D data. */
    for (int i = 0; i < DIM_X_LEN * DIM_Y_LEN; i++)
        data_2d[i] = my_comp_idx + i;

    /* Write the 2-D variable with put_var(). */
    if ((ret = PIOc_put_var_short(ncid, 1, data_2d)))
        ERR(ret);
    
    /* Write the 3D data. */
    if (use_darray)
    {
        /* Write the records of data with PIOc_write_darray(). */
        if ((ret = PIOc_setframe(ncid, varid[2], 0)))
            ERR(ret);
        if ((ret = PIOc_write_darray(ncid, varid[2], ioid, DIM_X_LEN * DIM_Y_LEN, data_2d, NULL)))
            ERR(ret);
        if ((ret = PIOc_setframe(ncid, varid[2], 1)))
            ERR(ret);
        if ((ret = PIOc_write_darray(ncid, varid[2], ioid, DIM_X_LEN * DIM_Y_LEN, data_2d, NULL)))
            ERR(ret);
    }
    else
    {
        PIO_Offset start[NDIM3] = {0, 0, 0};
        PIO_Offset count[NDIM3] = {1, DIM_X_LEN, DIM_Y_LEN};
        
        /* Write a record of the 3-D variable with put_vara(). */
        if ((ret = PIOc_put_vara_short(ncid, varid[2], start, count, data_2d)))
            ERR(ret);
        
        /* Write another record of the 3-D variable with put_vara(). */
        start[0] = 1;
        if ((ret = PIOc_put_vara_short(ncid, varid[2], start, count, data_2d)))
            ERR(ret);
    }

    /* Close the file if ncidp was not provided. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);

    return PIO_NOERR;
}

/*
 * Check the file produced by create_nc_sample_3() for
 * correctness. This checks all the metadata and data in the test
 * file.
 *
 * @param iosysid identifies the IO system.
 * @param iotype the iotype to be used to create the file.
 * @param my_rank rank of this task in world (for debugging messages
 * only).
 * @param my_comp_idx the index of the computational component
 * creating the file.
 * @param filename the name of the file to check.
 * @param test_name name of the test program.
 * @param verbose non-zero to turn on printf statements.
 * @param use_darray if non-zero, use darray functions to write data,
 * otherwise use PIOc_put_var().
 * @param ioid the decomposition ID to use if darrays are used to
 * write data.
 * @returns 0 for success, error code otherwise.
 */
int check_nc_sample_3(int iosysid, int iotype, int my_rank, int my_comp_idx,
                      const char *filename, int verbose, int use_darray, int ioid)
{
    int ncid;
    int nvars;
    int ndims;
    int ngatts;
    int unlimdimid;
    PIO_Offset att_len;
    char att_name[PIO_MAX_NAME + 1];
    char var_name[PIO_MAX_NAME + 1];
    char var_name_expected[PIO_MAX_NAME + 1];
    int dimid[NDIM2];
    int xtype;
    int natts;
    int comp_idx_in;
    short data_2d[DIM_X_LEN * DIM_Y_LEN];
    signed char att_data;
    int ret;

    /* Open the test file. */
    if ((ret = PIOc_openfile2(iosysid, &ncid, &iotype, filename, PIO_NOWRITE)))
        ERR(ret);

    /* Check file metadata. */
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        ERR(ret);
    if (ndims != NDIM3 || nvars != NVAR || ngatts != 1 || unlimdimid != 0)
        ERR(ERR_WRONG);

    /* Check the global attribute. */
    sprintf(att_name, "%s_%d", GLOBAL_ATT_NAME, my_comp_idx);
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, att_name, &xtype, &att_len)))
        ERR(ret);
    if (xtype != PIO_BYTE || att_len != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_schar(ncid, PIO_GLOBAL, att_name, &att_data)))
        ERR(ret);
    if (att_data != my_comp_idx)
        ERR(ERR_WRONG);

    /* Check the scalar variable metadata. */
    if ((ret = PIOc_inq_var(ncid, 0, var_name, &xtype, &ndims, NULL, &natts)))
        ERR(ret);
    sprintf(var_name_expected, "%s_%d", SCALAR_VAR_NAME, my_comp_idx);
    if (strcmp(var_name, var_name_expected) || xtype != PIO_INT || ndims != 0 || natts != 0)
        ERR(ERR_WRONG);

    /* Check the scalar variable data. */
    if ((ret = PIOc_get_var_int(ncid, 0, &comp_idx_in)))
        ERR(ret);
    if (comp_idx_in != my_comp_idx)
        ERR(ERR_WRONG);

    /* Check the 2D variable metadata. */
    if ((ret = PIOc_inq_var(ncid, 1, var_name, &xtype, &ndims, dimid, &natts)))
        ERR(ret);
    sprintf(var_name_expected, "%s_%d", TWOD_VAR_NAME, my_comp_idx);
    if (strcmp(var_name, var_name_expected) || xtype != PIO_SHORT || ndims != 2 || natts != 0)
        ERR(ERR_WRONG);

    /* Read the 2-D variable. */
    if ((ret = PIOc_get_var_short(ncid, 1, data_2d)))
        ERR(ret);

    /* Check 2D data for correctness. */
    for (int i = 0; i < DIM_X_LEN * DIM_Y_LEN; i++)
        if (data_2d[i] != my_comp_idx + i)
            ERR(ERR_WRONG);

    /* Check the 3-D variable. */
    if (use_darray)
    {
        /* Read the record of data with PIOc_read_darray(). */
        if ((ret = PIOc_setframe(ncid, 2, 0)))
            ERR(ret);
        if ((ret = PIOc_read_darray(ncid, 2, ioid, DIM_X_LEN * DIM_Y_LEN, data_2d)))
            ERR(ret);
        if ((ret = PIOc_setframe(ncid, 2, 1)))
            ERR(ret);
        if ((ret = PIOc_read_darray(ncid, 2, ioid, DIM_X_LEN * DIM_Y_LEN, data_2d)))
            ERR(ret);
    }
    else
    {
        PIO_Offset start[NDIM3] = {0, 0, 0};
        PIO_Offset count[NDIM3] = {1, DIM_X_LEN, DIM_Y_LEN};
        
        /* Read a record of the 3-D variable with get_vara(). */
        if ((ret = PIOc_get_vara_short(ncid, 2, start, count, data_2d)))
            ERR(ret);
        for (int i = 0; i < DIM_X_LEN * DIM_Y_LEN; i++)
            if (data_2d[i] != my_comp_idx + i)
                ERR(ERR_WRONG);
        
        /* Read another record of the 3-D variable with get_vara(). */
        start[0] = 1;
        if ((ret = PIOc_get_vara_short(ncid, 2, start, count, data_2d)))
            ERR(ret);
        for (int i = 0; i < DIM_X_LEN * DIM_Y_LEN; i++)
            if (data_2d[i] != my_comp_idx + i)
                ERR(ERR_WRONG);
    }

    /* Close the test file. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);
    return 0;
}

/*
 * This creates a test netCDF file in the specified format. This file
 * is more complex. It has a global attribute of every type, 3
 * dimensions, including unlimited dimension, a scalar var of each
 * type, and a 3D var of each type.
 *
 * @param iosysid identifies the IO system.
 * @param iotype the iotype to be used to create the file.
 * @param my_rank rank of this task in world (for debugging messages
 * only).
 * @param my_comp_idx the index of the computational component
 * creating the file.
 * @param filename pointer to buffer that will get filename. Must be
 * PIO_MAX_NAME + 1 in size.
 * @param test_name name of the test program.
 * @param varbose non-zero to turn on printf statements.
 * @returns 0 for success, error code otherwise.
 */
int create_nc_sample_4(int iosysid, int iotype, int my_rank, int my_comp_idx,
                       char *filename, char *test_name, int verbose, int num_types)
{
    char iotype_name[NC_MAX_NAME + 1];
    int ncid;
    int scalar_varid[num_types];
    int varid[num_types];
    char att_name[PIO_MAX_NAME + 1];
    char var_name[PIO_MAX_NAME + 1];
    char dim_name[PIO_MAX_NAME + 1];
    int dimid[NDIM3];
    int dim_len[NDIM3] = {PIO_UNLIMITED, DIM_X_LEN, DIM_Y_LEN};
    /* short data_2d[DIM_X_LEN * DIM_Y_LEN]; */
    int ret;

    /* Learn name of IOTYPE. */
    if ((ret = get_iotype_name(iotype, iotype_name)))
        ERR(ret);

    /* Create a filename. */
    sprintf(filename, "%s_%s_cmp_%d.nc", test_name, iotype_name, my_comp_idx);
    if (verbose)
        printf("my_rank %d creating test file %s for iosysid %d\n", my_rank, filename, iosysid);

    /* Create the file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &iotype, filename, NC_CLOBBER)))
        ERR(ret);

    /* Create a global attributes of all types. */
    for (int t = 0; t < num_types; t++)
    {
        sprintf(att_name, "%s_cmp_%d_type_%d", GLOBAL_ATT_NAME, my_comp_idx, pio_type[t]);
        if ((ret = PIOc_put_att(ncid, PIO_GLOBAL, att_name, pio_type[t], ATT_LEN, att_data[t])))
            ERR(ret);
    }

    /* Define a scalar variable of each type. */
    for (int t = 0; t < num_types; t++)
    {
        sprintf(var_name, "%s_cmp_%d_type_%d", SCALAR_VAR_NAME, my_comp_idx, pio_type[t]);
        if ((ret = PIOc_def_var(ncid, var_name, pio_type[t], 0, NULL, &scalar_varid[t])))
            ERR(ret);
    }

    /* Define dimensions. */
    for (int d = 0; d < NDIM3; d++)
    {
        sprintf(dim_name, "%s_%d_cmp_%d", DIM_NAME, d, my_comp_idx);
        if ((ret = PIOc_def_dim(ncid, dim_name, dim_len[d], &dimid[d])))
            ERR(ret);
    }

    /* Define a 3D variable for each type. */
    for (int t = 0; t < num_types; t++)
    {
        sprintf(var_name, "%s_cmp_%d_type_%d", THREED_VAR_NAME, my_comp_idx, pio_type[t]);
        if ((ret = PIOc_def_var(ncid, var_name, pio_type[t], NDIM3, dimid, &varid[t])))
            ERR(ret);
    }

    /* End define mode. */
    if ((ret = PIOc_enddef(ncid)))
        ERR(ret);

    /* Write the scalar variables. */
    for (int t = 0; t < num_types; t++)
        if ((ret = PIOc_put_var(ncid, scalar_varid[t], scalar_data[t])))
            ERR(ret);

    /* Write the 3-D variables. */
    /* for (int t = 0; t < num_types; t++) */
    /* { */
    /*     for (int i = 0; i < DIM_X_LEN * DIM_Y_LEN; i++) */
    /*         data_2d[i] = my_comp_idx + i; */
    /*     if ((ret = PIOc_put_var_short(ncid, 1, data_2d))) */
    /*         ERR(ret); */
    /* } */

    /* Close the file if ncidp was not provided. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);

    return PIO_NOERR;
}

/* Check a test file for correctness. */
int check_nc_sample_4(int iosysid, int iotype, int my_rank, int my_comp_idx,
                      const char *filename, int verbose, int num_types)
{
    int ncid;
    int nvars;
    int ndims;
    int ngatts;
    int unlimdimid;
    /* PIO_Offset att_len; */
    /* char att_name[PIO_MAX_NAME + 1]; */
    char var_name[PIO_MAX_NAME + 1];
    /* int dimid[NDIM2]; */
    int xtype;
    int natts;
    /* int comp_idx_in; */
    /* short data_2d[DIM_X_LEN * DIM_Y_LEN]; */
    int ret;

    /* Open the test file. */
    if ((ret = PIOc_openfile2(iosysid, &ncid, &iotype, filename, PIO_NOWRITE)))
        ERR(ret);

    /* Check file metadata. */
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        ERR(ret);
    if (ndims != NDIM3 || nvars != num_types * 2 || ngatts != num_types || unlimdimid != 0)
        ERR(ERR_WRONG);

    /* Check the global attributes. */
    for (int t = 0; t < num_types; t++)
    {
        PIO_Offset type_size;
        PIO_Offset att_len_in;
        void *att_data_in;
        char att_name[PIO_MAX_NAME + 1];

        sprintf(att_name, "%s_cmp_%d_type_%d", GLOBAL_ATT_NAME, my_comp_idx, pio_type[t]);
        if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, att_name, &xtype, &att_len_in)))
            ERR(ret);
        if (xtype != pio_type[t] || att_len_in != ATT_LEN)
            ERR(ERR_WRONG);
        if ((ret = PIOc_inq_type(ncid, xtype, NULL, &type_size)))
            ERR(ret);
        if (!(att_data_in = malloc(type_size * ATT_LEN)))
            ERR(ERR_AWFUL);
        if (verbose)
            printf("my_rank %d t %d pio_type[t] %d type_size %lld\n", my_rank, t, pio_type[t],
                   type_size);
        if ((ret = PIOc_get_att(ncid, PIO_GLOBAL, att_name, att_data_in)))
            ERR(ret);
        if (memcmp(att_data_in, att_data[t], type_size * ATT_LEN))
            ERR(ERR_WRONG);
        free(att_data_in);
    }

    /* Check the scalar variables. */
    for (int t = 0; t < num_types; t++)
    {
        int vid;
        PIO_Offset type_size;
        void *scalar_data_in;

        sprintf(var_name, "%s_cmp_%d_type_%d", SCALAR_VAR_NAME, my_comp_idx, pio_type[t]);
        if ((ret = PIOc_inq_varid(ncid, var_name, &vid)))
            ERR(ret);
        if ((ret = PIOc_inq_var(ncid, vid, var_name, &xtype, &ndims, NULL, &natts)))
            ERR(ret);
        if (xtype != pio_type[t] || ndims != 0 || natts != 0)
            ERR(ERR_WRONG);

        /* Check the data. */
        if ((ret = PIOc_inq_type(ncid, xtype, NULL, &type_size)))
            ERR(ret);
        if (!(scalar_data_in = malloc(type_size)))
            ERR(ERR_AWFUL);

        if ((ret = PIOc_get_var(ncid, vid, scalar_data_in)))
            ERR(ret);
        /* if (comp_idx_in != my_comp_idx) */
        /*     ERR(ERR_WRONG); */
        free(scalar_data_in);
    }

    /* Check the 3D variables. */
    for (int t = 0; t < num_types; t++)
    {
        int vid;
        /* PIO_Offset type_size; */
        /* void *threed_data_in; */
        int var_dimids[NDIM3];

        sprintf(var_name, "%s_cmp_%d_type_%d", THREED_VAR_NAME, my_comp_idx, pio_type[t]);
        if ((ret = PIOc_inq_varid(ncid, var_name, &vid)))
            ERR(ret);
        if ((ret = PIOc_inq_var(ncid, vid, var_name, &xtype, &ndims, var_dimids, &natts)))
            ERR(ret);
        if (xtype != pio_type[t] || ndims != NDIM3 || natts != 0)
            ERR(ERR_WRONG);

        /* if ((ret = PIOc_inq_var(ncid, 1, var_name, &xtype, &ndims, dimid, &natts))) */
        /*     ERR(ret); */
        /* sprintf(var_name_expected, "%s_%d", THREED_VAR_NAME, my_comp_idx); */
        /* if (strcmp(var_name, var_name_expected) || xtype != PIO_SHORT || ndims != 2 || natts != 0) */
        /*     ERR(ERR_WRONG); */

        /* /\* Read the 2-D variable. *\/ */
        /* if ((ret = PIOc_get_var_short(ncid, 1, data_2d))) */
        /*     ERR(ret); */

        /* /\* Check 2D data for correctness. *\/ */
        /* for (int i = 0; i < DIM_X_LEN * DIM_Y_LEN; i++) */
        /*     if (data_2d[i] != my_comp_idx + i) */
        /*         ERR(ERR_WRONG); */
    }

    /* Close the test file. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);
    return 0;
}
