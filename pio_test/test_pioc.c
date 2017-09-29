/*
 * Tests for PIO Functions.
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
#define TEST_NAME "test_pioc"

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
#define ATT_NAME "foo"
#define ATT_NAME2 "bar"

/* Value to write to attributes. */
#define ATT_VAL 42

/* The meaning of life, the universe, and everything. */
#define START_DATA_VAL 42

/* Values for some netcdf-4 settings. */
#define VAR_CACHE_SIZE (1024 * 1024)
#define VAR_CACHE_NELEMS 10
#define VAR_CACHE_PREEMPTION 0.5

/* Number of NetCDF classic types. */
#define NUM_CLASSIC_TYPES 6

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"timestep", "x", "y"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/* Length of chunksizes to use in netCDF-4 files. */
PIO_Offset chunksize[NDIM] = {2, X_DIM_LEN/2, Y_DIM_LEN/2};

#define DIM_NAME "dim"
#define NDIM1 1
#define DIM_LEN 4

/* Length of the max maplen in decomp testing. */
#define MAX_MAPLEN 1

signed char custom_fill_byte = -TEST_VAL_42;
unsigned char custom_fill_char = TEST_VAL_42;
short custom_fill_short = -1000;
int custom_fill_int = -100000;
float custom_fill_float = 10.5;
double custom_fill_double = 1000000.5;
unsigned char custom_fill_ubyte = 5;
unsigned short custom_fill_ushort = 1000;
unsigned int custom_fill_uint = 100000;
long long custom_fill_int64 = -100000000;
unsigned long long custom_fill_uint64 = 100000000;

/* Create the decomposition to divide the 1-dimensional sample data
 * between the 4 tasks.
 *
 * @param ntasks the number of available tasks
 * @param my_rank rank of this task.
 * @param iosysid the IO system ID.
 * @param dim1_len the length of the dimension.
 * @param ioid a pointer that gets the ID of this decomposition.
 * @returns 0 for success, error code otherwise.
 **/
int create_decomposition(int ntasks, int my_rank, int iosysid, int dim1_len, int *ioid)
{
#define NDIM1 1
    PIO_Offset elements_per_pe;     /* Array elements per processing unit. */
    PIO_Offset *compdof;  /* The decomposition mapping. */
    int dim_len[NDIM1] = {dim1_len};
    int bad_dim_len[NDIM1] = {-50};
    int ret;

    /* How many data elements per task? */
    elements_per_pe = dim1_len / ntasks;

    /* Allocate space for the decomposition array. */
    if (!(compdof = malloc(elements_per_pe * sizeof(PIO_Offset))))
        return PIO_ENOMEM;

    /* Describe the decomposition. The new init_decomp uses a 0-based
     * array, so don't add 1! */
    for (int i = 0; i < elements_per_pe; i++)
        compdof[i] = my_rank * elements_per_pe + i;

    /* These should fail. */
    if (PIOc_init_decomp(iosysid + TEST_VAL_42, PIO_FLOAT, NDIM1, dim_len, elements_per_pe,
                         compdof, ioid, 0, NULL, NULL) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_init_decomp(iosysid, PIO_FLOAT, NDIM1, bad_dim_len, elements_per_pe,
                         compdof, ioid, 0, NULL, NULL) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_init_decomp(iosysid, PIO_FLOAT, NDIM1, dim_len, elements_per_pe,
                                compdof, ioid, 0, NULL, NULL)))
        ERR(ret);

    /* Free the mapping. */
    free(compdof);

    return 0;
}

/* Check the contents of the test file. */
int check_darray_file(int iosysid, int ntasks, int my_rank, char *filename)
{
    int ncid;
    int ndims, nvars, ngatts, unlimdimid;
    char dim_name_in[PIO_MAX_NAME + 1];
    PIO_Offset dim_len_in;
    PIO_Offset arraylen = 1;
    float data_in;
    int ioid;
    int ret;

    assert(filename);

    /* Open the file. */
    if ((ret = PIOc_open(iosysid, filename, PIO_NOWRITE, &ncid)))
        ERR(ret);

    /* Check metadata. */
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        ERR(ret);
    if (ndims != 1 || nvars != 1 || ngatts != 0 || unlimdimid != -1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_dim(ncid, 0, dim_name_in, &dim_len_in)))
        ERR(ret);
    if (strcmp(dim_name_in, DIM_NAME) || dim_len_in != DIM_LEN)
        ERR(ERR_WRONG);

    /* Decompose the data over the tasks. */
    if ((ret = create_decomposition(ntasks, my_rank, iosysid, DIM_LEN, &ioid)))
        ERR(ret);

    /* Read data. */
    if ((ret = PIOc_read_darray(ncid, 0, ioid, arraylen, &data_in)))
        ERR(ret);

    /* Try to write, but this will fail because file was opened with
     * NOWRITE. */
    float fillvalue = 0.0;
    float test_data =  my_rank * 10;
    if (PIOc_write_darray(ncid, 0, ioid, arraylen, &test_data, &fillvalue) != PIO_EPERM)
        ERR(ret);

    /* Check data. */
    if (data_in != my_rank * 10)
        ERR(ERR_WRONG);

    /* Close the file. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return PIO_NOERR;
}

/* Test the darray functionality. */
int test_darray(int iosysid, int ioid, int num_flavors, int *flavor, int my_rank,
                int fv)
{
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dim_len[NDIM1] = {DIM_LEN}; /* Length of the dimensions in the sample data. */
    int dimids[NDIM1];      /* The dimension IDs. */
    int ncid;      /* The ncid of the netCDF file. */
    int varid;     /* The ID of the netCDF varable. */
    int ret;       /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        /* Create the filename. */
        sprintf(filename, "%s_flavor_%d_fv_%d.nc", TEST_NAME, flavor[fmt], fv);

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* Define netCDF dimensions and variable. */
        if ((ret = PIOc_def_dim(ncid, DIM_NAME, (PIO_Offset)dim_len[0], &dimids[0])))
            ERR(ret);

        /* Define a variable. */
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_FLOAT, NDIM1, dimids, &varid)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* These should not work, because invalid varids are given. */
        if (PIOc_setframe(ncid, TEST_VAL_42, 0) != PIO_ENOTVAR)
            ERR(ERR_WRONG);
        if (PIOc_advanceframe(ncid, TEST_VAL_42) != PIO_ENOTVAR)
            ERR(ERR_WRONG);

        /* Write some data. */
        PIO_Offset arraylen = 1;
        float fillvalue = PIO_FILL_FLOAT;
        float *fillvaluep = fv ? &fillvalue : NULL;
        float test_data[arraylen];
        for (int f = 0; f < arraylen; f++)
            test_data[f] = my_rank * 10 + f;
        if ((ret = PIOc_write_darray(ncid, varid, ioid, arraylen, test_data, fillvaluep)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Check the file contents. */
        if ((ret = check_darray_file(iosysid, TARGET_NTASKS, my_rank, filename)))
            ERR(ret);
    }
    return PIO_NOERR;
}

/* Check the dimension names.
 *
 * @param my_rank rank of process
 * @param ncid ncid of open netCDF file
 * @returns 0 for success, error code otherwise. */
int check_dim_names(int my_rank, int ncid, MPI_Comm test_comm)
{
    char dim_name[PIO_MAX_NAME + 1];
    char zero_dim_name[PIO_MAX_NAME + 1];
    int my_test_rank;
    int ret;

    /* Find rank in test communicator. */
    if ((ret = MPI_Comm_rank(test_comm, &my_test_rank)))
        MPIERR(ret);

    for (int d = 0; d < NDIM; d++)
    {
        memset(dim_name, 0, sizeof(dim_name));
        if ((ret = PIOc_inq_dimname(ncid, d, dim_name)))
            ERR(ret);

        /* Did other ranks get the same name? */
        memset(zero_dim_name, 0, sizeof(zero_dim_name));
        if (!my_test_rank)
            strcpy(zero_dim_name, dim_name);
        if ((ret = MPI_Bcast(&zero_dim_name, strlen(dim_name) + 1, MPI_CHAR, 0,
                             test_comm)))
            MPIERR(ret);
        if (strcmp(dim_name, zero_dim_name))
            return ERR_AWFUL;
    }
    return 0;
}

/* Check the variable name.
 *
 * @param my_rank rank of process
 * @param ncid ncid of open netCDF file
 *
 * @returns 0 for success, error code otherwise. */
int check_var_name(int my_rank, int ncid, MPI_Comm test_comm)
{
    char var_name[PIO_MAX_NAME + 1];
    char zero_var_name[PIO_MAX_NAME + 1];
    int my_test_rank;
    int ret;

    /* Find rank in test communicator. */
    if ((ret = MPI_Comm_rank(test_comm, &my_test_rank)))
        MPIERR(ret);

    memset(var_name, 0, sizeof(var_name));
    if ((ret = PIOc_inq_varname(ncid, 0, var_name)))
        ERR(ret);

    /* Did other ranks get the same name? */
    memset(zero_var_name, 0, sizeof(zero_var_name));
    if (!my_test_rank)
        strcpy(zero_var_name, var_name);
    if ((ret = MPI_Bcast(&zero_var_name, strlen(var_name) + 1, MPI_CHAR, 0,
                         test_comm)))
        MPIERR(ret);
    if (strcmp(var_name, zero_var_name))
        return ERR_AWFUL;
    return 0;
}

/* Check the attribute name.
 *
 * @param my_rank rank of process
 * @param ncid ncid of open netCDF file
 * @param flavor the iotype
 * @param test_comm the MPI communicator of the test.
 * @param async 1 if we are testing async, 0 otherwise.
 * @returns 0 for success, error code otherwise.
 */
int check_atts(int my_rank, int ncid, int flavor, MPI_Comm test_comm, int async)
{
    char att_name[PIO_MAX_NAME + 1];
    char zero_att_name[PIO_MAX_NAME + 1];
    int my_test_rank;
    nc_type att_type;
    PIO_Offset att_len;
    signed char att_schar_value;
    short att_short_value;
    int att_int_value;
    float att_float_value;
    double att_double_value;
    unsigned char att_uchar_value;
    unsigned short att_ushort_value;
    unsigned int att_uint_value;
    long long int att_int64_value;
    unsigned long long int att_uint64_value;
    char too_long_name[PIO_MAX_NAME * 5 + 1];
    int ret;

    memset(too_long_name, 74, PIO_MAX_NAME * 5);
    too_long_name[PIO_MAX_NAME * 5] = 0;

    /* Find rank in test communicator. */
    if ((ret = MPI_Comm_rank(test_comm, &my_test_rank)))
        MPIERR(ret);

    memset(att_name, 0, sizeof(att_name));
    if ((ret = PIOc_inq_attname(ncid, NC_GLOBAL, 0, att_name)))
        ERR(ret);

    /* Did all ranks get the same name? */
    memset(zero_att_name, 0, sizeof(zero_att_name));
    if (!my_test_rank)
        strcpy(zero_att_name, att_name);
    if ((ret = MPI_Bcast(&zero_att_name, strlen(att_name) + 1, MPI_CHAR, 0,
                         test_comm)))
        MPIERR(ret);
    if (strcmp(att_name, zero_att_name))
        return ERR_AWFUL;

    /* These should not work. */
    if (PIOc_get_att_int(ncid + TEST_VAL_42, NC_GLOBAL, ATT_NAME, &att_int_value) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_get_att_int(ncid, TEST_VAL_42, ATT_NAME, &att_int_value) != PIO_ENOTVAR)
        ERR(ERR_WRONG);
    if (PIOc_get_att_int(ncid, NC_GLOBAL, NULL, &att_int_value) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_get_att_int(ncid, NC_GLOBAL, ATT_NAME, NULL) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* These should not work. */
    if (PIOc_inq_att(ncid, NC_GLOBAL, too_long_name, &att_type, &att_len) != PIO_EINVAL)
        ERR(ERR_WRONG);
    int tmp_attid;
    if (PIOc_inq_attid(ncid, NC_GLOBAL, too_long_name, &tmp_attid) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Check first att. */
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)))
        ERR(ret);
    if (att_type != PIO_INT || att_len != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_int(ncid, NC_GLOBAL, ATT_NAME, &att_int_value)))
        ERR(ret);
    if (att_int_value != ATT_VAL)
        ERR(ERR_WRONG);

    /* Try the typeless get. */
    int att_int_value2;
    if ((ret = PIOc_get_att(ncid, NC_GLOBAL, ATT_NAME, &att_int_value2)))
        ERR(ret);
    if (att_int_value2 != ATT_VAL)
        ERR(ERR_WRONG);

    /* Check second att. */
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, ATT_NAME2, &att_type, &att_len)))
        ERR(ret);
    if (att_type != PIO_FLOAT || att_len != 1)
        ERR(ERR_WRONG);

    /* Try converting to every type. */
    if ((ret = PIOc_get_att_schar(ncid, NC_GLOBAL, ATT_NAME2, &att_schar_value)))
        ERR(ret);
    if (att_schar_value != ATT_VAL)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_short(ncid, NC_GLOBAL, ATT_NAME2, &att_short_value)))
        ERR(ret);
    if (att_short_value != ATT_VAL)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_int(ncid, NC_GLOBAL, ATT_NAME2, &att_int_value)))
        ERR(ret);
    if (att_int_value != ATT_VAL)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_float(ncid, NC_GLOBAL, ATT_NAME2, &att_float_value)))
        ERR(ret);
    if (att_float_value != ATT_VAL)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_double(ncid, NC_GLOBAL, ATT_NAME2, &att_double_value)))
        ERR(ret);
    if (att_double_value != ATT_VAL)
        ERR(ERR_WRONG);
    if (flavor == PIO_IOTYPE_NETCDF4C || flavor == PIO_IOTYPE_NETCDF4P)
    {
        if ((ret = PIOc_get_att_uchar(ncid, NC_GLOBAL, ATT_NAME2, &att_uchar_value)))
            ERR(ret);
        if (att_uchar_value != ATT_VAL)
            ERR(ERR_WRONG);
        if ((ret = PIOc_get_att_ushort(ncid, NC_GLOBAL, ATT_NAME2, &att_ushort_value)))
            ERR(ret);
        if (att_ushort_value != ATT_VAL)
            ERR(ERR_WRONG);
        if ((ret = PIOc_get_att_uint(ncid, NC_GLOBAL, ATT_NAME2, &att_uint_value)))
            ERR(ret);
        if (att_uint_value != ATT_VAL)
            ERR(ERR_WRONG);
        if ((ret = PIOc_get_att_longlong(ncid, NC_GLOBAL, ATT_NAME2, &att_int64_value)))
            ERR(ret);
        if (att_int64_value != ATT_VAL)
            ERR(ERR_WRONG);
        if ((ret = PIOc_get_att_ulonglong(ncid, NC_GLOBAL, ATT_NAME2, &att_uint64_value)))
            ERR(ret);
        if (att_uint64_value != ATT_VAL)
            ERR(ERR_WRONG);
    }
    return 0;
}

/*
 * Check error strings.
 *
 * @param my_rank rank of this task.
 * @param num_tries number of errcodes to try.
 * @param errcode pointer to array of error codes, of length num_tries.
 * @param expected pointer to an array of strings, with the expected
 * error messages for each error code.
 * @returns 0 for success, error code otherwise.
 */
int check_error_strings(int my_rank, int num_tries, int *errcode,
                        const char **expected)
{
    int ret;

    /* Try each test code. */
    for (int try = 0; try < num_tries; try++)
    {
        char errstr[PIO_MAX_NAME + 1];

        /* Get the error string for this errcode. */
        if ((ret = PIOc_strerror(errcode[try], errstr)))
            ERR(ret);

        /* Check that it was as expected. */
        if (strncmp(errstr, expected[try], strlen(expected[try])))
        {
            printf("%d expected %s got %s\n", my_rank, expected[try], errstr);
            return ERR_AWFUL;
        }
    }

    return PIO_NOERR;
}

/* Check the PIOc_iotype_available() function.
 *
 * @param my_rank the rank of this process.
 * @return 0 for success, error code otherwise.
 */
int test_iotypes(int my_rank)
{
    /* This is never present. */
    if (PIOc_iotype_available(1000))
        ERR(ERR_WRONG);

    /* NetCDF is always present. */
    if (!PIOc_iotype_available(PIO_IOTYPE_NETCDF))
        ERR(ERR_WRONG);

    /* Pnetcdf may or may not be present. */
#ifdef _PNETCDF
    if (!PIOc_iotype_available(PIO_IOTYPE_PNETCDF))
        ERR(ERR_WRONG);
#else
    if (PIOc_iotype_available(PIO_IOTYPE_PNETCDF))
        ERR(ERR_WRONG);
#endif /* _PNETCDF */

    /* NetCDF-4 may or may not be present. */
#ifdef _NETCDF4
    if (!PIOc_iotype_available(PIO_IOTYPE_NETCDF4C))
        ERR(ERR_WRONG);
    if (!PIOc_iotype_available(PIO_IOTYPE_NETCDF4P))
        ERR(ERR_WRONG);
#else
    if (PIOc_iotype_available(PIO_IOTYPE_NETCDF4C))
        ERR(ERR_WRONG);
    if (PIOc_iotype_available(PIO_IOTYPE_NETCDF4P))
        ERR(ERR_WRONG);
#endif /* _NETCDF4 */

    return PIO_NOERR;
}

/* Check the PIOc_strerror() function for classic netCDF.
 *
 * @param my_rank the rank of this process.
 * @return 0 for success, error code otherwise.
 */
int check_strerror_netcdf(int my_rank)
{
#define NUM_NETCDF_TRIES 5
    int errcode[NUM_NETCDF_TRIES] = {PIO_EBADID, NC4_LAST_ERROR - 1, 0, 1, -600};
    const char *expected[NUM_NETCDF_TRIES] = {"NetCDF: Not a valid ID",
                                              "Unknown Error: Unrecognized error code", "No error",
                                              nc_strerror(1), "Unknown Error: Unrecognized error code"};
    int ret;

    if ((ret = check_error_strings(my_rank, NUM_NETCDF_TRIES, errcode, expected)))
        ERR(ret);

    /* When called with a code of 0, these functions should do nothing
     * and return 0. */
    if (check_mpi(NULL, 0, __FILE__, __LINE__))
        ERR(ERR_WRONG);
    if (check_mpi2(NULL, NULL, 0, __FILE__, __LINE__))
        ERR(ERR_WRONG);
    if (pio_err(NULL, NULL, 0, __FILE__, __LINE__))
        ERR(ERR_WRONG);
    if (check_netcdf(NULL, 0, __FILE__, __LINE__))
        ERR(ERR_WRONG);
    if (check_netcdf2(NULL, NULL, 0, __FILE__, __LINE__))
        ERR(ERR_WRONG);

    /* When called with other error messages, these functions should
     * return PIO_EIO. */
    /* if (check_mpi(NULL, MPI_ERR_OTHER, __FILE__, __LINE__) != PIO_EIO) */
    /*     ERR(ERR_WRONG); */
    /* This returns the correct result, but prints a confusing error
     * message during the test run, so I'll leave it commented out. */
    /* if (check_mpi(NULL, MPI_ERR_UNKNOWN, __FILE__, __LINE__) != PIO_EIO) */
    /*     ERR(ERR_WRONG); */

    return PIO_NOERR;
}

/* Check the PIOc_strerror() function for netCDF-4.
 *
 * @param my_rank the rank of this process.
 * @return 0 for success, error code otherwise.
 */
int check_strerror_netcdf4(int my_rank)
{
#ifdef _NETCDF4
#define NUM_NETCDF4_TRIES 2
    int errcode[NUM_NETCDF4_TRIES] = {NC_ENOTNC3, NC_ENOPAR};
    const char *expected[NUM_NETCDF4_TRIES] =
        {"NetCDF: Attempting netcdf-3 operation on netcdf-4 file",
         "NetCDF: Parallel operation on file opened for non-parallel access"};
    int ret;

    if ((ret = check_error_strings(my_rank, NUM_NETCDF4_TRIES, errcode, expected)))
        ERR(ret);

#endif /* _NETCDF4 */

    return PIO_NOERR;
}

/* Check the PIOc_strerror() function for parallel-netCDF.
 *
 * @param my_rank the rank of this process.
 * @return 0 for success, error code otherwise.
 */
int check_strerror_pnetcdf(int my_rank)
{
#ifdef _PNETCDF
#define NUM_PNETCDF_TRIES 2
    int errcode[NUM_PNETCDF_TRIES] = {NC_EMULTIDEFINE_VAR_NUM, NC_EMULTIDEFINE_ATTR_VAL};
    const char *expected[NUM_PNETCDF_TRIES] =
        {"Number of variables is",
         "Attribute value is inconsistent among processes."};
    int ret;

    if ((ret = check_error_strings(my_rank, NUM_PNETCDF_TRIES, errcode, expected)))
        ERR(ret);

#endif /* _PNETCDF */

    return PIO_NOERR;
}

/* Check the PIOc_strerror() function for PIO.
 *
 * @param my_rank the rank of this process.
 * @return 0 for success, error code otherwise.
 */
int check_strerror_pio(int my_rank)
{
#define NUM_PIO_TRIES 6
    int errcode[NUM_PIO_TRIES] = {PIO_EBADID,
                                  NC_ENOTNC3, NC4_LAST_ERROR - 1, 0, 1,
                                  PIO_EBADIOTYPE};
    const char *expected[NUM_PIO_TRIES] = {"NetCDF: Not a valid ID",
                                           "NetCDF: Attempting netcdf-3 operation on netcdf-4 file",
                                           "Unknown Error: Unrecognized error code", "No error",
                                           nc_strerror(1), "Bad IO type"};
    int ret;

    if ((ret = check_error_strings(my_rank, NUM_PIO_TRIES, errcode, expected)))
        ERR(ret);

    return PIO_NOERR;
}

/* Check the PIOc_strerror() function.
 *
 * @param my_rank the rank of this process.
 * @return 0 for success, error code otherwise.
 */
int check_strerror(int my_rank)
{
    int ret;

    if ((ret = check_strerror_netcdf(my_rank)))
        ERR(ret);

    if ((ret = check_strerror_netcdf4(my_rank)))
        ERR(ret);

    if ((ret = check_strerror_pnetcdf(my_rank)))
        ERR(ret);

    if ((ret = check_strerror_pio(my_rank)))
        ERR(ret);

    return PIO_NOERR;
}

/* Define metadata for the test file. */
int define_metadata(int ncid, int my_rank, int flavor)
{
    int dimids[NDIM]; /* The dimension IDs. */
    int varid; /* The variable ID. */
    char too_long_name[PIO_MAX_NAME * 5 + 1];
    int ret;

    /* Check invalid parameters. */
    memset(too_long_name, 74, PIO_MAX_NAME * 5);
    too_long_name[PIO_MAX_NAME * 5] = 0;
    if (PIOc_def_dim(ncid + 1, dim_name[0], (PIO_Offset)dim_len[0], &dimids[0]) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_def_dim(ncid, NULL, (PIO_Offset)dim_len[0], &dimids[0]) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_def_dim(ncid, too_long_name, (PIO_Offset)dim_len[0], &dimids[0]) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Define dimensions. */
    for (int d = 0; d < NDIM; d++)
        if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
            ERR(ret);

    /* Check invalid parameters. */
    if (PIOc_def_var(ncid + 1, VAR_NAME, PIO_INT, NDIM, dimids, &varid) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_def_var(ncid, NULL, PIO_INT, NDIM, dimids, &varid) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_def_var(ncid, too_long_name, PIO_INT, NDIM, dimids, &varid) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Define a variable. Test that varidp can be NULL. Since this is
     * the first var in the file, the varid will be 0. */
    if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_INT, NDIM, dimids, NULL)))
        ERR(ret);
    varid = 0;

    /* Set the fill mode. */
    int fillmode = PIO_NOFILL;
    int temp_mode;
    int old_mode;
    if ((ret = PIOc_set_fill(ncid, fillmode, &old_mode)))
        ERR(ERR_WRONG);
    if ((ret = PIOc_set_fill(ncid, fillmode, &temp_mode)))
        ERR(ERR_WRONG);
    if (temp_mode != PIO_NOFILL)
        ERR(ERR_WRONG);
    if ((ret = PIOc_set_fill(ncid, old_mode, NULL)))
        ERR(ret);

    /* Set the fill value for netCDF-4 files. */
    int int_fill = -999;
    int int_fill_in;
    int fill_mode;

    /* These should not work. */
    if (PIOc_def_var_fill(ncid + TEST_VAL_42, varid, NC_FILL, &int_fill) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_def_var_fill(ncid, varid + TEST_VAL_42, NC_FILL, &int_fill) != PIO_ENOTVAR)
        ERR(ERR_WRONG);
    if (PIOc_def_var_fill(ncid, varid, NC_FILL + TEST_VAL_42, &int_fill) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_def_var_fill(ncid, varid, NC_FILL, NULL) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Set the fill value. */
    if ((ret = PIOc_def_var_fill(ncid, varid, NC_FILL, &int_fill)))
        ERR(ret);

    /* These should not work. */
    if (PIOc_inq_var_fill(ncid + TEST_VAL_42, varid, &fill_mode, &int_fill_in) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_inq_var_fill(ncid, varid + TEST_VAL_42, &fill_mode, &int_fill_in) != PIO_ENOTVAR)
        ERR(ERR_WRONG);

    /* Check the fill value. */
    if ((ret = PIOc_inq_var_fill(ncid, varid, &fill_mode, &int_fill_in)))
        ERR(ret);
    if (fill_mode != NC_FILL || int_fill_in != int_fill)
        ERR(ERR_WRONG);

    /* These should also work. */
    int_fill_in = 0;

    /* This does not work for pnetcdf, but probably should. */
    if (flavor != PIO_IOTYPE_PNETCDF)
    {
        if ((ret = PIOc_inq_var_fill(ncid, varid, NULL, &int_fill_in)))
            ERR(ret);
        if (int_fill_in != int_fill)
            ERR(ERR_WRONG);
        if ((ret = PIOc_inq_var_fill(ncid, varid, NULL, NULL)))
            ERR(ret);
    }
    if ((ret = PIOc_inq_var_fill(ncid, varid, &fill_mode, NULL)))
        ERR(ret);
    if (fill_mode != NC_FILL)
        ERR(ERR_WRONG);

    return PIO_NOERR;
}

/* Check the metadata in the test file. */
int check_metadata(int ncid, int my_rank, int flavor)
{
    int ndims, nvars, ngatts, unlimdimid, natts, dimid[NDIM];
    PIO_Offset len_in;
    char name_in[PIO_MAX_NAME + 1];
    nc_type xtype_in;
    int ret;

    /* Check how many dims, vars, global atts there are, and the id of
     * the unlimited dimension. */
    if (PIOc_inq(ncid + 1, &ndims, &nvars, &ngatts, &unlimdimid) != PIO_EBADID)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq(ncid, NULL, NULL, NULL, NULL)))
        ERR(ret);
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        ERR(ret);
    if (ndims != NDIM || nvars != 1 || ngatts != 0 || unlimdimid != 0)
        return ERR_AWFUL;
    int num_unlimdims;
    int unlimdimid2;
    if ((ret = PIOc_inq_unlimdims(ncid, &num_unlimdims, &unlimdimid2)))
        ERR(ret);
    if (unlimdimid2 != 0)
        return ERR_AWFUL;

    /* Check the dimensions. */
    for (int d = 0; d < NDIM; d++)
    {
        if (PIOc_inq_dim(ncid + 1, d, name_in, &len_in) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_inq_dim(ncid, d + 40, name_in, &len_in) != PIO_EBADDIM)
            ERR(ERR_WRONG);
        if ((ret = PIOc_inq_dim(ncid, d, NULL, NULL)))
            ERR(ret);
        if ((ret = PIOc_inq_dim(ncid, d, name_in, &len_in)))
            ERR(ret);
        if (len_in != dim_len[d] || strcmp(name_in, dim_name[d]))
            return ERR_AWFUL;
    }

    /* Check the variable. */
    if (PIOc_inq_var(ncid + 1, 0, name_in, &xtype_in, &ndims, dimid, &natts) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_inq_var(ncid, 45, name_in, &xtype_in, &ndims, dimid, &natts) != PIO_ENOTVAR)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_var(ncid, 0, name_in, NULL, NULL, NULL, NULL)))
        ERR(ret);
    if ((ret = PIOc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimid, &natts)))
        ERR(ret);
    if (strcmp(name_in, VAR_NAME) || xtype_in != PIO_INT || ndims != NDIM ||
        dimid[0] != 0 || dimid[1] != 1 || dimid[2] != 2 || natts != 1)
        return ERR_AWFUL;

    return PIO_NOERR;
}

/* Test file operations.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @param test_comm the MPI communicator of the test.
 * @param async 1 if we are testing async, 0 otherwise.
 * @returns 0 for success, error code otherwise.
 */
int test_names(int iosysid, int num_flavors, int *flavor, int my_rank,
               MPI_Comm test_comm, int async)
{
    char too_long_name[PIO_MAX_NAME * 5 + 1];
    int ret;    /* Return code. */

    memset(too_long_name, 74, PIO_MAX_NAME * 5);
    too_long_name[PIO_MAX_NAME * 5] = 0;

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        int ncid;
        int varid;
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];
        int dimids[NDIM];        /* The dimension IDs. */
        int att_val = ATT_VAL;
        float float_att_val = ATT_VAL;

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "%s_%s_names.nc", TEST_NAME, iotype_name);

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* Define netCDF dimensions and variable. */
        for (int d = 0; d < NDIM; d++)
        {
            if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
                ERR(ret);
        }

        /* These should not work. */
        if (PIOc_setframe(ncid + TEST_VAL_42, 0, 0) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_setframe(ncid, -1, 0) != PIO_ENOTVAR)
            ERR(ERR_WRONG);
        if (PIOc_setframe(ncid, NC_MAX_VARS + 1, 0) != PIO_ENOTVAR)
            ERR(ERR_WRONG);
        if (PIOc_advanceframe(ncid + TEST_VAL_42, 0) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_advanceframe(ncid, -1) != PIO_ENOTVAR)
            ERR(ERR_WRONG);
        if (PIOc_advanceframe(ncid, NC_MAX_VARS + 1) != PIO_ENOTVAR)
            ERR(ERR_WRONG);

        /* Check the dimension names. */
        if ((ret = check_dim_names(my_rank, ncid, test_comm)))
            ERR(ret);

        /* These should not work. */
        if (PIOc_put_att_int(ncid + TEST_VAL_42, NC_GLOBAL, ATT_NAME, PIO_INT, 1, &att_val) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, TEST_VAL_42, ATT_NAME, PIO_INT, 1, &att_val) != PIO_ENOTVAR)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, NC_GLOBAL, NULL, PIO_INT, 1, &att_val) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, NC_GLOBAL, ATT_NAME, PIO_INT, 1, NULL) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, NC_GLOBAL, too_long_name, PIO_INT, 1, &att_val) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, NC_GLOBAL, ATT_NAME, PIO_INT, -1, &att_val) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, NC_GLOBAL, ATT_NAME, TEST_VAL_42, 1, &att_val) != PIO_EBADTYPE)
            ERR(ERR_WRONG);
        if (PIOc_put_att_int(ncid, NC_GLOBAL, ATT_NAME, PIO_LONG_INTERNAL, 1, &att_val) != PIO_EBADTYPE)
            ERR(ERR_WRONG);

        /* Define a global attribute. */
        if ((ret = PIOc_put_att_int(ncid, NC_GLOBAL, ATT_NAME, PIO_INT, 1, &att_val)))
            ERR(ret);
        if ((ret = PIOc_put_att_float(ncid, NC_GLOBAL, ATT_NAME2, PIO_FLOAT, 1, &float_att_val)))
            ERR(ret);

        /* Check the attribute name. */
        if ((ret = check_atts(my_rank, ncid, flavor[fmt], test_comm, async)))
            ERR(ret);

        /* Define a variable. */
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_FLOAT, NDIM, dimids, &varid)))
            ERR(ret);

        /* Check the variable name. */
        if ((ret = check_var_name(my_rank, ncid, test_comm)))
            ERR(ret);

        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);
    }

    return PIO_NOERR;
}

/* Test file operations.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_files(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid;
    int ret;    /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];

        /* Overwrite existing test file. */
        int mode = PIO_CLOBBER;

        /* If this is netCDF-4, add the netCDF4 flag. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            mode |= NC_NETCDF4;
        }

        /* If this is pnetcdf or netCDF-4 parallel, add the MPIIO flag. */
        if (flavor[fmt] == PIO_IOTYPE_PNETCDF || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            mode |= NC_MPIIO;
        }

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "%s_%s.nc", TEST_NAME, iotype_name);

        /* Testing some invalid parameters. */
        if (PIOc_create(iosysid + 1, filename, mode, &ncid) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_create(iosysid, filename, mode, NULL) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_create(iosysid, NULL, mode, &ncid) != PIO_EINVAL)
            ERR(ERR_WRONG);

        /* Create the netCDF output file. */
        if ((ret = PIOc_create(iosysid, filename, mode, &ncid)))
            ERR(ret);

        /* Check this support function. */
        if (!PIOc_File_is_Open(ncid))
            ERR(ERR_WRONG);
        if (PIOc_File_is_Open(ncid + 1))
            ERR(ERR_WRONG);

        /* Define the test file metadata. */
        if ((ret = define_metadata(ncid, my_rank, flavor[fmt])))
            ERR(ret);

        /* End define mode. */
        if (PIOc_enddef(ncid + 1) != PIO_EBADID)
            ERR(ERR_WRONG);
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        if (PIOc_closefile(ncid + 1) != PIO_EBADID)
            ERR(ERR_WRONG);
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Check some invalid paramters. */
        if (PIOc_open(iosysid + 1, filename, mode, &ncid) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_open(iosysid, NULL, mode, &ncid) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_open(iosysid, filename, mode, NULL) != PIO_EINVAL)
            ERR(ERR_WRONG);

        /* Reopen the test file. */
        if ((ret = PIOc_open(iosysid, filename, mode, &ncid)))
            ERR(ret);

        /* Check the test file metadata. */
        if ((ret = check_metadata(ncid, my_rank, flavor[fmt])))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

    }

    return PIO_NOERR;
}

/* Test empty file operations.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_empty_files(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid, ncid2;
    int ret;    /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "%s_empty_%s.nc", TEST_NAME, iotype_name);
        
        if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], filename, PIO_CLOBBER)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Reopen the test file. */
        if ((ret = PIOc_openfile2(iosysid, &ncid2, &flavor[fmt], filename, PIO_NOWRITE)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid2)))
            ERR(ret);

    }

    return PIO_NOERR;
}

/* Check that the fill values are correctly reported by find_var_fill().
 *
 * @param ncid the ID of the open test file.
 * @param ntypes the number ot types we are testing. 
 * @param use_custom_fill true if custom fill values were used.
 * @param my_rank rank of this task.
 * @return 0 on success. 
 */
int check_fillvalues(int ncid, int num_types, int use_custom_fill, int my_rank)
{
    file_desc_t *file;
    signed char fill_byte;
    unsigned char fill_char;
    short fill_short;
    int fill_int;
    float fill_float;
    double fill_double;
    unsigned char fill_ubyte;
    unsigned short fill_ushort;
    unsigned int fill_uint;
    long long fill_int64;
    unsigned long long fill_uint64;
    int ret;

    if (use_custom_fill)
    {
        fill_byte = custom_fill_byte;
        fill_char = custom_fill_char;
        fill_short = custom_fill_short;
        fill_int = custom_fill_int;
        fill_float = custom_fill_float;
        fill_double = custom_fill_double;
        fill_ubyte = custom_fill_ubyte;
        fill_ushort = custom_fill_ushort;
        fill_uint = custom_fill_uint;
        fill_int64 = custom_fill_int64;
        fill_uint64 = custom_fill_uint64;
    }
    else
    {
        fill_byte = PIO_FILL_BYTE;
        fill_char = PIO_FILL_CHAR;
        fill_short = PIO_FILL_SHORT;
        fill_int = PIO_FILL_INT;
        fill_float = PIO_FILL_FLOAT;
        fill_double = PIO_FILL_DOUBLE;
        fill_ubyte = PIO_FILL_UBYTE;
        fill_ushort = PIO_FILL_USHORT;
        fill_uint = PIO_FILL_UINT;
        fill_int64 = PIO_FILL_INT64;
        fill_uint64 = PIO_FILL_UINT64;
    }

    if ((ret = pio_get_file(ncid, &file)))
        ERR(ret);
            
    for (int v = 0; v < num_types; v++)
    {
        var_desc_t *vdesc;

        /* Get the var info. */
        if ((ret = get_var_desc(v, &file->varlist, &vdesc)))
            ERR(ret);
                
        /* Check the fill value with this internal function. */
        if ((ret = find_var_fillvalue(file, v, vdesc)))
            ERR(ret);

        switch (vdesc->pio_type)
        {
        case PIO_BYTE:
            if (*(signed char *)vdesc->fillvalue != fill_byte)
                ERR(ERR_WRONG);
            break;
        case PIO_CHAR:
            if (*(unsigned char *)vdesc->fillvalue != fill_char)
                ERR(ERR_WRONG);
            break;
        case PIO_SHORT:
            if (*(short *)vdesc->fillvalue != fill_short)
                ERR(ERR_WRONG);
            break;
        case PIO_INT:
            if (*(int *)vdesc->fillvalue != fill_int)
                ERR(ERR_WRONG);
            break;
        case PIO_FLOAT:
            if (*(float *)vdesc->fillvalue != fill_float)
                ERR(ERR_WRONG);
            break;
        case PIO_DOUBLE:
            if (*(double *)vdesc->fillvalue != fill_double)
                ERR(ERR_WRONG);
            break;
        case PIO_UBYTE:
            if (*(unsigned char *)vdesc->fillvalue != fill_ubyte)
                ERR(ERR_WRONG);
            break;
        case PIO_USHORT:
            if (*(unsigned short *)vdesc->fillvalue != fill_ushort)
                ERR(ERR_WRONG);
            break;
        case PIO_UINT:
            if (*(unsigned int *)vdesc->fillvalue != fill_uint)
                ERR(ERR_WRONG);
            break;
        case PIO_INT64:
            if (*(long long *)vdesc->fillvalue != fill_int64)
                ERR(ERR_WRONG);
            break;
        case PIO_UINT64:
            if (*(unsigned long long *)vdesc->fillvalue != fill_uint64)
                ERR(ERR_WRONG);
            break;
        default:
            ERR(ERR_AWFUL);
        }
    }

    return PIO_NOERR;
}
        
/* Test the internal function that determins a var's fillvalue.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_find_var_fillvalue(int iosysid, int num_flavors, int *flavor,
                            int my_rank, int async)
{
#define NUM_FILL_TESTS 2
    int ncid;
    int dimid;
    int test_type[NUM_NETCDF4_TYPES - 1] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT,
                                            PIO_FLOAT, PIO_DOUBLE, PIO_UBYTE, PIO_USHORT,
                                            PIO_UINT, PIO_INT64, PIO_UINT64};
    int ret;    /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];
        int num_types = NUM_CLASSIC_TYPES;

        /* Overwrite existing test file. */
        int mode = PIO_CLOBBER;

        /* If this is netCDF-4, add the netCDF4 flag. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            mode |= NC_NETCDF4;
            num_types = NUM_NETCDF4_TYPES - 1;
        }
        else
            num_types = NUM_CLASSIC_TYPES;

        /* If this is pnetcdf or netCDF-4 parallel, add the MPIIO flag. */
        if (flavor[fmt] == PIO_IOTYPE_PNETCDF || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            mode |= NC_MPIIO;
        }

        /* Get memory for varids. */
        int varid[num_types];

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "%s_find_var_fillvalue_%s.nc", TEST_NAME, iotype_name);

        /* Test with and without custom fill values. */
        for (int fvt = 0; fvt < NUM_FILL_TESTS; fvt++)
        {
            /* Create the netCDF output file. */
            if ((ret = PIOc_create(iosysid, filename, mode, &ncid)))
                ERR(ret);

            /* Create a dimension. */
            if ((ret = PIOc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)))
                ERR(ret);

            /* Create a var of each type. */
            for (int v = 0; v < num_types; v++)
            {
                char var_name[PIO_MAX_NAME + 1];
                sprintf(var_name, "var_of_type_%d", test_type[v]);
                if ((ret = PIOc_def_var(ncid, var_name, test_type[v], NDIM1, &dimid, &varid[v])))
                    ERR(ret);

                /* Use custom fill values for this test? */
                if (fvt)
                {
                    switch(test_type[v])
                    {
                    case PIO_BYTE:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_byte)))
                            ERR(ret);
                        break;
                    case PIO_CHAR:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_char)))
                            ERR(ret);
                        break;
                    case PIO_SHORT:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_short)))
                            ERR(ret);
                        break;
                    case PIO_INT:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_int)))
                            ERR(ret);
                        break;
                    case PIO_FLOAT:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_float)))
                            ERR(ret);
                        break;
                    case PIO_DOUBLE:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_double)))
                            ERR(ret);
                        break;
                    case PIO_UBYTE:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_ubyte)))
                            ERR(ret);
                        break;
                    case PIO_USHORT:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_ushort)))
                            ERR(ret);
                        break;
                    case PIO_UINT:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_uint)))
                            ERR(ret);
                        break;
                    case PIO_INT64:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_int64)))
                            ERR(ret);
                        break;
                    case PIO_UINT64:
                        if ((ret = PIOc_def_var_fill(ncid, varid[v], NC_FILL, &custom_fill_uint64)))
                            ERR(ret);
                        break;
                    default:
                        ERR(ERR_AWFUL);
                    }
                } /* endif fvt */
            }

            /* Check the fill values. */
            if ((ret = check_fillvalues(ncid, num_types, fvt, my_rank)))
                ERR(ret);

            /* End define mode. */
            if ((ret = PIOc_enddef(ncid)))
                ERR(ret);

            /* Close the netCDF file. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);

            /* Reopen the test file. */
            /* if ((ret = PIOc_open(iosysid, filename, NC_WRITE, &ncid))) */
            /*     ERR(ret); */
            if ((ret = PIOc_openfile2(iosysid, &ncid, &flavor[fmt], filename, NC_WRITE)))
                ERR(ret);

            /* Check the fill values. */
            if ((ret = check_fillvalues(ncid, num_types, fvt, my_rank)))
                ERR(ret);

            /* Close the netCDF file. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);
        } /* next fill value test */
    }

    return PIO_NOERR;
}

/* Test the deletion of files.
 *
 * @param iosysid the iosystem ID that will be used for the test.
 * @param num_flavors the number of different IO types that will be tested.
 * @param flavor an array of the valid IO types.
 * @param my_rank 0-based rank of task.
 * @returns 0 for success, error code otherwise.
 */
int test_deletefile(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid;
    int ret;    /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];
        int old_method;

        /* These should fail. */
        if (PIOc_set_iosystem_error_handling(iosysid + TEST_VAL_42, PIO_RETURN_ERROR, &old_method) != PIO_EBADID)
            ERR(ERR_WRONG);
        if (PIOc_set_iosystem_error_handling(iosysid, PIO_RETURN_ERROR + TEST_VAL_42, &old_method) != PIO_EINVAL)
            ERR(ERR_WRONG);

        /* Set error handling. */
        if ((ret = PIOc_set_iosystem_error_handling(iosysid, PIO_RETURN_ERROR, &old_method)))
            ERR(ret);
        if (old_method != PIO_INTERNAL_ERROR && old_method != PIO_RETURN_ERROR)
            ERR(ERR_WRONG);

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "delete_me_%s_%s.nc", TEST_NAME, iotype_name);

        int bad_iotype = TEST_VAL_42;
        if (PIOc_createfile(iosysid, &ncid, &bad_iotype, filename, PIO_CLOBBER) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* This should not work. */
        if (PIOc_deletefile(iosysid + TEST_VAL_42, filename) != PIO_EBADID)
            ERR(ERR_WRONG);

        /* Now delete the file. */
        if ((ret = PIOc_deletefile(iosysid, filename)))
            ERR(ret);

        /* Make sure it is gone. Openfile will now return an error
         * code when I try to open the file. */
        if (!PIOc_openfile2(iosysid, &ncid, &(flavor[fmt]), filename, PIO_NOWRITE))
            ERR(ERR_WRONG);
    }

    return PIO_NOERR;
}

/* Test the netCDF-4 optimization functions. */
int test_nc4(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid;    /* The ncid of the netCDF file. */
    int dimids[NDIM];    /* The dimension IDs. */
    int varid;    /* The ID of the netCDF varable. */

    /* For setting the chunk cache. */
    PIO_Offset chunk_cache_size = 1024*1024;
    PIO_Offset chunk_cache_nelems = 1024;
    float chunk_cache_preemption = 0.5;

    /* For reading the chunk cache. */
    PIO_Offset chunk_cache_size_in;
    PIO_Offset chunk_cache_nelems_in;
    float chunk_cache_preemption_in;

    int storage = NC_CHUNKED; /* Storage of netCDF-4 files (contiguous vs. chunked). */
    PIO_Offset my_chunksize[NDIM]; /* Chunksizes we get from file. */
    int shuffle;    /* The shuffle filter setting in the netCDF-4 test file. */
    int deflate;    /* Non-zero if deflate set for the variable in the netCDF-4 test file. */
    int deflate_level;    /* The deflate level set for the variable in the netCDF-4 test file. */
    int endianness;    /* Endianness of variable. */
    PIO_Offset var_cache_size;    /* Size of the var chunk cache. */
    PIO_Offset var_cache_nelems; /* Number of elements in var cache. */
    float var_cache_preemption;     /* Var cache preemption. */
    char varname_in[PIO_MAX_NAME];
    int expected_ret; /* The return code we expect to get. */
    int ret;    /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "%s_%s.nc", TEST_NAME, iotype_name);

        /* Try to set the chunk cache. */
        chunk_cache_preemption = 0.5;
        ret = PIOc_set_chunk_cache(iosysid, flavor[fmt], chunk_cache_size,
                                   chunk_cache_nelems, chunk_cache_preemption);

        /* What result did we expect to get? */
        expected_ret = flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P ?
            PIO_NOERR : PIO_ENOTNC4;
        if (ret != expected_ret)
            ERR(ERR_AWFUL);

        /* Try to set the chunk cache for netCDF-4 iotypes. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
            if ((ret = PIOc_set_chunk_cache(iosysid, flavor[fmt], chunk_cache_size,
                                            chunk_cache_nelems, chunk_cache_preemption)))
                ERR(ERR_AWFUL);

        /* Now check the chunk cache. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            if ((ret = PIOc_get_chunk_cache(iosysid, flavor[fmt], &chunk_cache_size_in,
                                            &chunk_cache_nelems_in, &chunk_cache_preemption_in)))
                ERR(ERR_AWFUL);

            /* Check that we got the correct values. */
            if (chunk_cache_size_in != chunk_cache_size || chunk_cache_nelems_in != chunk_cache_nelems ||
                chunk_cache_preemption_in != chunk_cache_preemption)
                ERR(ERR_AWFUL);
        }

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* Define netCDF dimensions and variable. */
        for (int d = 0; d < NDIM; d++)
        {
            if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
                ERR(ret);
        }
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_FLOAT, NDIM, dimids, &varid)))
            ERR(ret);

        /* Check that invalid arguments are properly rejected. */
        if (PIOc_def_var_chunking(ncid + TEST_VAL_42, 0, NC_CHUNKED, chunksize) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_inq_var_chunking(ncid + TEST_VAL_42, 0, &storage, my_chunksize) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_inq_var_deflate(ncid + TEST_VAL_42, 0, &shuffle, &deflate, &deflate_level) != PIO_EBADID)
            ERR(ret);
        if (PIOc_def_var_endian(ncid + TEST_VAL_42, 0, 1) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_def_var_deflate(ncid + TEST_VAL_42, 0, 0, 0, 0) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_inq_var_endian(ncid + TEST_VAL_42, 0, &endianness) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_set_var_chunk_cache(ncid + TEST_VAL_42, 0, VAR_CACHE_SIZE, VAR_CACHE_NELEMS,
                                     VAR_CACHE_PREEMPTION) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_get_var_chunk_cache(ncid + TEST_VAL_42, 0, &var_cache_size, &var_cache_nelems,
                                     &var_cache_preemption) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_set_chunk_cache(iosysid + TEST_VAL_42, flavor[fmt], chunk_cache_size, chunk_cache_nelems,
                                 chunk_cache_preemption) != PIO_EBADID)
            ERR(ERR_AWFUL);
        if (PIOc_get_chunk_cache(iosysid + TEST_VAL_42, flavor[fmt], &chunk_cache_size,
                                 &chunk_cache_nelems, &chunk_cache_preemption) != PIO_EBADID)
            ERR(ERR_AWFUL);

        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            if (PIOc_def_var_chunking(ncid, TEST_VAL_42, NC_CHUNKED, chunksize) != PIO_ENOTVAR)
                ERR(ERR_AWFUL);
            if (PIOc_inq_var_chunking(ncid, TEST_VAL_42, &storage, my_chunksize) != PIO_ENOTVAR)
                ERR(ERR_AWFUL);
            if (PIOc_set_var_chunk_cache(ncid, TEST_VAL_42, VAR_CACHE_SIZE, VAR_CACHE_NELEMS,
                                         VAR_CACHE_PREEMPTION) != PIO_ENOTVAR)
                ERR(ERR_AWFUL);
            if (PIOc_get_var_chunk_cache(ncid, TEST_VAL_42, &var_cache_size, &var_cache_nelems,
                                         &var_cache_preemption) != PIO_ENOTVAR)
                ERR(ERR_AWFUL);
        }
        else
        {
            if (PIOc_def_var_chunking(ncid, TEST_VAL_42, NC_CHUNKED, chunksize) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_inq_var_chunking(ncid, TEST_VAL_42, &storage, my_chunksize) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_set_var_chunk_cache(ncid, TEST_VAL_42, VAR_CACHE_SIZE, VAR_CACHE_NELEMS,
                                         VAR_CACHE_PREEMPTION) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_get_var_chunk_cache(ncid, TEST_VAL_42, &var_cache_size, &var_cache_nelems,
                                         &var_cache_preemption) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
        }

        /* For netCDF-4 files, set the chunksize to improve performance. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            unsigned long long too_big_chunksize[NDIM] = {(unsigned long long)NC_MAX_INT64 + TEST_VAL_42, X_DIM_LEN/2, Y_DIM_LEN/2};
            if (PIOc_def_var_chunking(ncid, 0, NC_CHUNKED, (MPI_Offset *)too_big_chunksize) == PIO_NOERR)
                ERR(ret);

            if ((ret = PIOc_def_var_chunking(ncid, 0, NC_CHUNKED, chunksize)))
                ERR(ret);

            /* Setting deflate should not work with parallel iotype. */
            ret = PIOc_def_var_deflate(ncid, 0, 0, 1, 1);
            if (flavor[fmt] == PIO_IOTYPE_NETCDF4P)
            {
                if (ret == PIO_NOERR)
                    ERR(ERR_WRONG);
            }
            else
            {
                if (ret != PIO_NOERR)
                    ERR(ERR_WRONG);
            }

            /* Check that the inq_varname function works. */
            if ((ret = PIOc_inq_varname(ncid, 0, NULL)))
                ERR(ret);
            if ((ret = PIOc_inq_varname(ncid, 0, varname_in)))
                ERR(ret);

            /* Check that the inq_var_chunking function works. */
            if ((ret = PIOc_inq_var_chunking(ncid, 0, NULL, NULL)))
                ERR(ret);
            if ((ret = PIOc_inq_var_chunking(ncid, 0, &storage, my_chunksize)))
                ERR(ret);

            /* Check the answers. */
            if (storage != NC_CHUNKED)
                ERR(ERR_AWFUL);
            for (int d1 = 0; d1 < NDIM; d1++)
                if (my_chunksize[d1] != chunksize[d1])
                    ERR(ERR_AWFUL);

            /* Check that the inq_var_deflate functions works. */
            if ((ret = PIOc_inq_var_deflate(ncid, 0, &shuffle, &deflate, &deflate_level)))
                ERR(ret);

            /* For serial netCDF-4 deflate is turned on by default */
            if (flavor[fmt] == PIO_IOTYPE_NETCDF4C)
                if (shuffle || !deflate || deflate_level != 1)
                    ERR(ERR_AWFUL);

            /* For parallel netCDF-4, no compression available. :-( */
            if (flavor[fmt] == PIO_IOTYPE_NETCDF4P)
                if (shuffle || deflate)
                    ERR(ERR_AWFUL);

            /* Check setting the chunk cache for the variable. */
            if ((ret = PIOc_set_var_chunk_cache(ncid, 0, VAR_CACHE_SIZE, VAR_CACHE_NELEMS,
                                                VAR_CACHE_PREEMPTION)))
                ERR(ret);

            /* Check getting the chunk cache values for the variable. */
            if ((ret = PIOc_get_var_chunk_cache(ncid, 0, &var_cache_size, &var_cache_nelems,
                                                &var_cache_preemption)))
                ERR(ret);

            /* Check that we got expected values. */
            if (var_cache_size != VAR_CACHE_SIZE)
                ERR(ERR_AWFUL);
            if (var_cache_nelems != VAR_CACHE_NELEMS)
                ERR(ERR_AWFUL);
            if (var_cache_preemption != VAR_CACHE_PREEMPTION)
                ERR(ERR_AWFUL);

            if ((ret = PIOc_def_var_endian(ncid, 0, 1)))
                ERR(ERR_AWFUL);
            if ((ret = PIOc_inq_var_endian(ncid, 0, &endianness)))
                ERR(ERR_AWFUL);
            if (endianness != 1)
                ERR(ERR_WRONG);
        }
        else
        {
            /* Trying to set or inq netCDF-4 settings for non-netCDF-4
             * files results in the PIO_ENOTNC4 error. */
            if (PIOc_def_var_chunking(ncid, 0, NC_CHUNKED, chunksize) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_inq_var_chunking(ncid, 0, &storage, my_chunksize) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_inq_var_deflate(ncid, 0, &shuffle, &deflate, &deflate_level) != PIO_ENOTNC4)
                ERR(ret);
            if (PIOc_def_var_endian(ncid, 0, 1) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_inq_var_endian(ncid, 0, &endianness) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_set_var_chunk_cache(ncid, 0, VAR_CACHE_SIZE, VAR_CACHE_NELEMS,
                                         VAR_CACHE_PREEMPTION) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_get_var_chunk_cache(ncid, 0, &var_cache_size, &var_cache_nelems,
                                         &var_cache_preemption) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_set_chunk_cache(iosysid, flavor[fmt], chunk_cache_size, chunk_cache_nelems,
                                     chunk_cache_preemption) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
            if (PIOc_get_chunk_cache(iosysid, flavor[fmt], &chunk_cache_size,
                                     &chunk_cache_nelems, &chunk_cache_preemption) != PIO_ENOTNC4)
                ERR(ERR_AWFUL);
        }

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);
    }
    return PIO_NOERR;
}

/* This function is part of test_scalar(). It tests the contents of
 * the scalar var. */
int check_scalar_var(int ncid, int varid, int flavor, int my_rank)
{
    char var_name_in[PIO_MAX_NAME + 1];
    int var_type_in;
    int ndims_in;
    int natts_in;
    int val_in;
    int ret;

    /* Learn the var metadata. */
    if ((ret = PIOc_inq_var(ncid, varid, var_name_in, &var_type_in, &ndims_in, NULL,
                            &natts_in)))
        ERR(ret);

    /* Is the metadata correct? */
    if (strcmp(var_name_in, VAR_NAME) || var_type_in != PIO_INT || ndims_in != 0 || natts_in != 0)
        ERR(ERR_WRONG);

    /* Get the value. */
    if ((ret = PIOc_get_var_int(ncid, varid, &val_in)))
        ERR(ret);

    /* Is the value correct? */
    if (val_in != TEST_VAL_42)
        ERR(ERR_WRONG);

    return 0;
}

/* Test scalar vars. */
int test_scalar(int iosysid, int num_flavors, int *flavor, int my_rank, int async,
                MPI_Comm test_comm)
{
    int ncid;    /* The ncid of the netCDF file. */
    int varid;   /* The ID of the netCDF varable. */
    int ret;     /* Return code. */

    /* Use netCDF classic to create a file with a scalar var, then set
     * and read the value. */
    if (my_rank == 0)
    {
        char test_file[] = "netcdf_test.nc";
        int test_val = TEST_VAL_42;
        int test_val_in;

        if ((ret = nc_create(test_file, NC_CLOBBER, &ncid)))
            ERR(ret);
        if ((ret = nc_def_var(ncid, VAR_NAME, NC_INT, 0, NULL, &varid)))
            ERR(ret);
        if ((ret = nc_enddef(ncid)))
            ERR(ret);
        if ((ret = nc_put_var(ncid, varid, &test_val)))
            ERR(ret);
        if ((ret = nc_close(ncid)))
            ERR(ret);
        if ((ret = nc_open(test_file, NC_NOWRITE, &ncid)))
            ERR(ret);
        /* if ((ret = nc_get_var(ncid, varid, &test_val_in))) */
        /*     ERR(ret); */
        /* if (test_val_in != test_val) */
        /*     ERR(ERR_WRONG); */
        if ((ret = nc_get_vars(ncid, varid, NULL, NULL, NULL, &test_val_in)))
            ERR(ret);
        if (test_val_in != test_val)
            ERR(ERR_WRONG);
        if ((ret = nc_close(ncid)))
            ERR(ret);
    }

    /* Use pnetCDF to create a file with a scalar var, then set and
     * read the value. */
#ifdef _PNETCDF
    {
        char test_file[] = "pnetcdf_test.nc";
        int test_val = TEST_VAL_42;
        int test_val_in;

        if ((ret = ncmpi_create(test_comm, test_file, NC_CLOBBER, MPI_INFO_NULL, &ncid)))
            ERR(ret);
        if ((ret = ncmpi_def_var(ncid, VAR_NAME, NC_INT, 0, NULL, &varid)))
            ERR(ret);
        if ((ret = ncmpi_enddef(ncid)))
            ERR(ret);
        if ((ret = ncmpi_put_var_int_all(ncid, varid, &test_val)))
            ERR(ret);
        if ((ret = ncmpi_close(ncid)))
            ERR(ret);
        if ((ret = ncmpi_open(test_comm, test_file, NC_NOWRITE, MPI_INFO_NULL, &ncid)))
            ERR(ret);
        /* Turn on independent access for pnetcdf file. */
        if ((ret = ncmpi_begin_indep_data(ncid)))
            ERR(ret);
        /* if ((ret = ncmpi_get_var_int(ncid, varid, &test_val_in))) */
        /*     ERR(ret); */
        if ((ret = ncmpi_get_vars_int(ncid, varid, NULL, NULL, NULL, &test_val_in)))
            ERR(ret);
        if ((ret = ncmpi_end_indep_data(ncid)))
            ERR(ret);
        if (test_val_in != test_val)
            ERR(ERR_WRONG);
        if (test_val_in != test_val)
            ERR(ERR_WRONG);
        if ((ret = ncmpi_close(ncid)))
            ERR(ret);
    }
#endif /* _PNETCDF */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[PIO_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[PIO_MAX_NAME + 1];

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            ERR(ret);
        sprintf(filename, "%s_%s_scalar_async_%d.nc", TEST_NAME, iotype_name, async);

        /* Create the netCDF output file. */
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* Define a scalar variable. */
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_INT, 0, NULL, &varid)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Write a scalar value. */
        int test_val = TEST_VAL_42;
        if ((ret = PIOc_put_var_int(ncid, varid, &test_val)))
            ERR(ret);

        /* Check the scalar var. */
        if ((ret = check_scalar_var(ncid, varid, flavor[fmt], my_rank)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Reopen the file. */
        if ((ret = PIOc_openfile2(iosysid, &ncid, &(flavor[fmt]), filename, PIO_NOWRITE)))
            ERR(ret);

        /* Check the scalar var again. */
        if ((ret = check_scalar_var(ncid, varid, flavor[fmt], my_rank)))
            ERR(ret);

        /* Close the netCDF file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);
    }
    return PIO_NOERR;
}

/** Test the malloc_iodesc() function.
 *
 * @param my_rank rank of this task.
 * @returns 0 for success, error code otherwise.
 */
int test_malloc_iodesc2(int iosysid, int my_rank)
{
    /* More types are available for netCDF-4 builds. */
#ifdef _NETCDF4
    /* String type not supported. */
    int num_types = NUM_NETCDF_TYPES - 1;
#else
    int num_types = NUM_CLASSIC_TYPES;
#endif /* _NETCDF4 */
    int test_type[NUM_NETCDF_TYPES] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT,
                                       PIO_FLOAT, PIO_DOUBLE, PIO_UBYTE, PIO_USHORT,
                                       PIO_UINT, PIO_INT64, PIO_UINT64};
    MPI_Datatype mpi_type[NUM_NETCDF_TYPES] = {MPI_BYTE, MPI_CHAR, MPI_SHORT, MPI_INT,
                                               MPI_FLOAT, MPI_DOUBLE, MPI_UNSIGNED_CHAR,
                                               MPI_UNSIGNED_SHORT, MPI_UNSIGNED, MPI_LONG_LONG,
                                               MPI_UNSIGNED_LONG_LONG, MPI_CHAR};
    iosystem_desc_t *ios;
    io_desc_t *iodesc;
    int ret;

    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);
    /* Test with each type. */
    for (int t = 0; t < num_types; t++)
    {

        if ((ret = malloc_iodesc(ios, test_type[t], 1, &iodesc)))
            ERR(ret);
        if (iodesc->mpitype != mpi_type[t])
            ERR(ERR_WRONG);
        if (iodesc->ndims != 1)
            ERR(ERR_WRONG);
        pio_add_to_iodesc_list(iodesc);
        if (iodesc->firstregion)
            free_region_list(iodesc->firstregion);
        if ((ret = pio_delete_iodesc_from_list(iodesc->ioid)))
            ERR(ret);
    }
    return 0;
}

/* Test some decomp internal functions. */
int test_decomp_internal(int my_test_size, int my_rank, int iosysid, int dim_len,
                         MPI_Comm test_comm, int async)
{
    int ioid;
    char filename[NC_MAX_NAME + 1];    /* Test decomp filename. */
    char nc_filename[NC_MAX_NAME + 1]; /* Test decomp filename (netcdf version). */
    iosystem_desc_t *ios; /* IO system info. */
    int ret;

    /* This will be our file name for writing out decompositions. */
    sprintf(filename, "decomp_%s_rank_%d_async_%d.txt", TEST_NAME, my_rank, async);
    sprintf(nc_filename, "nc_decomp_internal_%s_rank_%d_async_%d.nc", TEST_NAME, my_rank, async);

    /* Decompose the data over the tasks. */
    if ((ret = create_decomposition(my_test_size, my_rank, iosysid, dim_len, &ioid)))
        ERR(ret);

    /* Write the decomp file (on appropriate tasks). */
    if ((ret = PIOc_write_decomp(filename, iosysid, ioid, test_comm)))
        ERR(ret);

    /* Some values for the netCDF decomp file for this iosystem. */
    char *title = "Very Simple Test Decompositon";
    char *history = "Added to PIO automatic testing by Ed in February 2017.";
    int global_dimlen[] = {DIM_LEN};
    int task_maplen[TARGET_NTASKS] = {1, 1, 1, 1};
    int map[TARGET_NTASKS][1] = {{0},{1},{2},{3}};

    /* Get the IO system info. */
    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    /* Write the decomposition file. */
    if ((ret = pioc_write_nc_decomp_int(ios, nc_filename, 0, NDIM1, global_dimlen,
                                        TARGET_NTASKS, task_maplen, (int *)map, title,
                                        history, 0)))
        ERR(ret);

    int ndims_in;
    int num_tasks_in;
    int max_maplen_in;
    char title_in[PIO_MAX_NAME + 1];
    char history_in[PIO_MAX_NAME + 1];
    char source_in[PIO_MAX_NAME + 1];
    char version_in[PIO_MAX_NAME + 1];
    char expected_source[] = "Decomposition file produced by PIO library.";
    int *global_dimlen_in;
    int *task_maplen_in;
    int *map_in;
    int fortran_order_in;

    /* These should not work. */
    if (pioc_read_nc_decomp_int(iosysid + TEST_VAL_42, nc_filename, &ndims_in, &global_dimlen_in,
                                &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                history_in, source_in, version_in, &fortran_order_in) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (pioc_read_nc_decomp_int(iosysid, NULL, &ndims_in, &global_dimlen_in,
                                &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                history_in, source_in, version_in, &fortran_order_in) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (!pioc_read_nc_decomp_int(iosysid, "no_file", &ndims_in, &global_dimlen_in,
                                &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                history_in, source_in, version_in, &fortran_order_in))
        ERR(ERR_WRONG);

    /* Read the decomp file. */
    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);


    /* Did we get the correct answers? */
    if (strcmp(title, title_in) || strcmp(history, history_in) ||
        strcmp(source_in, expected_source))
        ERR(ERR_WRONG);
    if (ndims_in != NDIM1 || num_tasks_in != TARGET_NTASKS || max_maplen_in != 1 ||
        fortran_order_in)
        ERR(ERR_WRONG);
    for (int d = 0; d < ndims_in; d++)
        if (global_dimlen_in[d] != global_dimlen[d])
            ERR(ERR_WRONG);
    for (int t = 0; t < num_tasks_in; t++)
        if (task_maplen_in[t] != 1)
            ERR(ERR_WRONG);
    for (int t = 0; t < num_tasks_in; t++)
        for (int l = 0; l < max_maplen_in; l++)
            if (map_in[t * max_maplen_in + l] != map[t][l])
                ERR(ERR_WRONG);

    /* Free resources. */
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    /* These should also work. */
    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, NULL, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, NULL,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       NULL, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, NULL, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, NULL, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, NULL, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, NULL,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       NULL, source_in, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, NULL, version_in, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, NULL, &fortran_order_in)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, NULL)))
        ERR(ret);
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return 0;
}

/* Test some decomp public API functions. */
int test_decomp_public(int my_test_size, int my_rank, int iosysid, int dim_len,
                       MPI_Comm test_comm, int async)
{
    int ioid;
    char nc_filename[NC_MAX_NAME + 1]; /* Test decomp filename (netcdf version). */
    int ret;

    /* This will be our file name for writing out decompositions. */
    sprintf(nc_filename, "nc_decomp_%s_rank_%d_async_%d.nc", TEST_NAME, my_rank, async);

    /* Decompose the data over the tasks. */
    if ((ret = create_decomposition(my_test_size, my_rank, iosysid, dim_len, &ioid)))
        ERR(ret);

    /* We will document our decomp file with metadata, like good
     * netCDF users should. */
    char *title = "Very Simple Test Decompositon";
    char *history = "Added to PIO automatic testing by Ed in February 2017.";

    /* These should not work. */
    char too_long_name[PIO_MAX_NAME * 5 + 1];
    memset(too_long_name, 74, PIO_MAX_NAME * 5);
    too_long_name[PIO_MAX_NAME * 5] = 0;

    if (PIOc_write_nc_decomp(iosysid + TEST_VAL_42, nc_filename, 0, ioid,
                             title, history, 0) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_write_nc_decomp(iosysid, NULL, 0, ioid, title, history, 0) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_write_nc_decomp(iosysid, nc_filename, 0, ioid + TEST_VAL_42,
                             title, history, 0) != PIO_EBADID)
        ERR(ERR_WRONG);

    if (PIOc_write_nc_decomp(iosysid, nc_filename, 0, ioid,
                             too_long_name, history, 0) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_write_nc_decomp(iosysid, nc_filename, 0, ioid,
                             title, too_long_name, 0) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Write a netCDF decomp file for this iosystem. */
    if ((ret = PIOc_write_nc_decomp(iosysid, nc_filename, 0, ioid, title,
                                    history, 0)))
        ERR(ret);

    int ioid_in;
    char title_in[PIO_MAX_NAME + 1];
    char history_in[PIO_MAX_NAME + 1];
    int fortran_order_in;

    /* These should not work. */
    if (PIOc_read_nc_decomp(iosysid + TEST_VAL_42, nc_filename, &ioid_in, test_comm,
                            PIO_INT, title_in, history_in, &fortran_order_in) != PIO_EBADID)
        ERR(ret);
    if (PIOc_read_nc_decomp(iosysid, NULL, &ioid_in, test_comm, PIO_INT, title_in,
                            history_in, &fortran_order_in) != PIO_EINVAL)
        ERR(ret);
    if (PIOc_read_nc_decomp(iosysid, nc_filename, NULL, test_comm, PIO_INT, title_in,
                            history_in, &fortran_order_in) != PIO_EINVAL)
        ERR(ret);

    /* Read it using the public read function. */
    if ((ret = PIOc_read_nc_decomp(iosysid, nc_filename, &ioid_in, test_comm, PIO_INT,
                                   title_in, history_in, &fortran_order_in)))
        ERR(ret);

    /* Did we get expected results? */
    if (strcmp(title, title_in) || strcmp(history, history_in))
        ERR(ERR_WRONG);

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid_in)))
        ERR(ret);

    /* These should also work. */
    if ((ret = PIOc_read_nc_decomp(iosysid, nc_filename, &ioid_in, test_comm, PIO_CHAR, NULL,
                                   history_in, &fortran_order_in)))
        ERR(ret);
    if ((ret = PIOc_freedecomp(iosysid, ioid_in)))
        ERR(ret);

    if ((ret = PIOc_read_nc_decomp(iosysid, nc_filename, &ioid_in, test_comm, PIO_BYTE, title_in,
                                   NULL, &fortran_order_in)))
        ERR(ret);
    if ((ret = PIOc_freedecomp(iosysid, ioid_in)))
        ERR(ret);

    if ((ret = PIOc_read_nc_decomp(iosysid, nc_filename, &ioid_in, test_comm, PIO_SHORT, title_in,
                                  history_in, NULL)))
        ERR(ret);
    if ((ret = PIOc_freedecomp(iosysid, ioid_in)))
        ERR(ret);

    /* Read it using the intertal function. */
    int ndims_in;
    int num_tasks_in;
    int max_maplen_in;
    char source_in[PIO_MAX_NAME + 1];
    char version_in[PIO_MAX_NAME + 1];
    char expected_source[] = "Decomposition file produced by PIO library.";
    int *global_dimlen_in;
    int *task_maplen_in;
    int *map_in;

    /* Read the decomp file. */
    if ((ret = pioc_read_nc_decomp_int(iosysid, nc_filename, &ndims_in, &global_dimlen_in,
                                       &num_tasks_in, &task_maplen_in, &max_maplen_in, &map_in, title_in,
                                       history_in, source_in, version_in, &fortran_order_in)))
        ERR(ret);

    /* Did we get the correct answers? */
    if (strcmp(title, title_in) || strcmp(history, history_in) ||
        strcmp(source_in, expected_source))
        ERR(ERR_WRONG);
    if (ndims_in != NDIM1 || num_tasks_in != TARGET_NTASKS || max_maplen_in != 1 ||
        fortran_order_in)
        ERR(ERR_WRONG);
    for (int d = 0; d < ndims_in; d++)
        if (global_dimlen_in[d] != DIM_LEN)
            ERR(ERR_WRONG);
    for (int t = 0; t < num_tasks_in; t++)
        if (task_maplen_in[t] != 1)
            ERR(ERR_WRONG);
    for (int t = 0; t < num_tasks_in; t++)
        for (int l = 0; l < max_maplen_in; l++)
            if (map_in[t * max_maplen_in + l] != t)
                ERR(ERR_WRONG);

    /* Free resources. */
    free(global_dimlen_in);
    free(task_maplen_in);
    free(map_in);

    /* /\* These should also work. *\/ */
    /* if ((ret = PIOc_write_nc_decomp(iosysid, nc_filename, 0, ioid, title, history, 0))) */
    /*     ERR(ret); */

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return 0;
}

/* Test some decomp public API functions. */
int test_decomp_public_2(int my_test_size, int my_rank, int iosysid, int dim_len,
                         MPI_Comm test_comm, int async)
{
    int ioid;
    char nc_filename[NC_MAX_NAME + 1]; /* Test decomp filename (netcdf version). */
    int ret;

    /* This will be our file name for writing out decompositions. */
    sprintf(nc_filename, "nc_decomp_%s_rank_%d_async_%d.nc", TEST_NAME, my_rank, async);

    /* Decompose the data over the tasks. */
    if ((ret = create_decomposition(my_test_size, my_rank, iosysid, dim_len, &ioid)))
        ERR(ret);

    /* Write a netCDF decomp file for this iosystem. */
    if ((ret = PIOc_write_nc_decomp(iosysid, nc_filename, 0, ioid, NULL, NULL, 0)))
        ERR(ret);

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return 0;
}

/* Test some decomp public API functions. */
int test_decomp_2(int my_test_size, int my_rank, int iosysid, int dim_len,
                  MPI_Comm test_comm, int async)
{
    int ioid;
    char nc_filename[NC_MAX_NAME + 1]; /* Test decomp filename (netcdf version). */
    int ret;

    /* This will be our file name for writing out decompositions. */
    sprintf(nc_filename, "nc_decomp_%s_rank_%d_async_%d.nc", TEST_NAME, my_rank, async);

    /* Decompose the data over the tasks. */
    if ((ret = create_decomposition(my_test_size, my_rank, iosysid, dim_len, &ioid)))
        ERR(ret);

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return 0;
}

/* Test some decomp public API functions with async. */
int test_decomp_public_async(int my_test_size, int my_rank, int iosysid, MPI_Comm test_comm,
                             int async)
{
#define ELEM1 1
#define LEN3 3
    int ioid;
    int dim_len = LEN3;
    PIO_Offset elements_per_pe = ELEM1;
    PIO_Offset compdof[ELEM1] = {my_rank + 1};
    char filename[PIO_MAX_NAME + 1];
    int ret;

    sprintf(filename, "async_decomp_%s_rank_%d_async_%d.nc", TEST_NAME, my_rank, async);

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_init_decomp(iosysid, PIO_FLOAT, NDIM1, &dim_len, elements_per_pe,
                                compdof, &ioid, PIO_REARR_BOX, NULL, NULL)))
        ERR(ret);

    /* Write the decomp file (on appropriate tasks). */
    if ((ret = PIOc_write_nc_decomp(iosysid, filename, 0, ioid, NULL, NULL, 0)))
        ERR(ret);

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);

    return 0;
}

/* Run all the tests. */
int test_all(int iosysid, int num_flavors, int *flavor, int my_rank, MPI_Comm test_comm,
             int async)
{
    int ioid;
    int my_test_size;
    char filename[NC_MAX_NAME + 1];
    char nc_filename[NC_MAX_NAME + 1];
    int ret; /* Return code. */

    if ((ret = MPI_Comm_size(test_comm, &my_test_size)))
        MPIERR(ret);

    /* This will be our file name for writing out decompositions. */
    sprintf(filename, "decomp_%d.txt", my_rank);
    sprintf(nc_filename, "decomp_%d.nc", my_rank);

    /* This is a simple test that just creates the decomp with
     * async. */
    if (async)
        if ((ret = test_decomp_public_async(my_test_size, my_rank, iosysid, test_comm, async)))
            ERR(ret);

    /* Check iotypes. */
    if ((ret = test_iotypes(my_rank)))
        ERR(ret);

    /* Test file deletes. */
    if ((ret = test_deletefile(iosysid, num_flavors, flavor, my_rank)))
        ERR(ret);

    /* Test file stuff. */
    if ((ret = test_files(iosysid, num_flavors, flavor, my_rank)))
        ERR(ret);

    /* Test some misc stuff. */
    if ((ret = test_malloc_iodesc2(iosysid, my_rank)))
        ERR(ret);

    /* Test internal function find_var_fillvalue(). */
    if ((ret = test_find_var_fillvalue(iosysid, num_flavors, flavor, my_rank, async)))
        ERR(ret);

    /* Run these tests for non-async cases only. */
    if (!async)
    {

        /* Test empty file stuff. */
        if ((ret = test_empty_files(iosysid, num_flavors, flavor, my_rank)))
            ERR(ret);

        /* Test decomposition internal functions. */
        if ((ret = test_decomp_internal(my_test_size, my_rank, iosysid, DIM_LEN, test_comm, async)))
            ERR(ret);

        /* Test decomposition public API functions. */
        if ((ret = test_decomp_public(my_test_size, my_rank, iosysid, DIM_LEN, test_comm, async)))
            ERR(ret);

        /* This is a simple test that just creates a decomp. */
        if ((ret = test_decomp_2(my_test_size, my_rank, iosysid, DIM_LEN, test_comm, async)))
            ERR(ret);

        /* This is a simple test that just writes the decomp. */
        if ((ret = test_decomp_public_2(my_test_size, my_rank, iosysid, DIM_LEN, test_comm, async)))
            ERR(ret);

        /* Decompose the data over the tasks. */
        if ((ret = create_decomposition(my_test_size, my_rank, iosysid, DIM_LEN, &ioid)))
            ERR(ret);

        /* Run the darray tests. */
        for (int fv = 0; fv < 2; fv++)
            if ((ret = test_darray(iosysid, ioid, num_flavors, flavor, my_rank, fv)))
                ERR(ret);

        /* Free the PIO decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid)))
            ERR(ret);
    }

    /* Check the error string function. */
    if ((ret = check_strerror(my_rank)))
        ERR(ret);

    /* Test name stuff. */
    if ((ret = test_names(iosysid, num_flavors, flavor, my_rank, test_comm, async)))
        ERR(ret);

    /* Test netCDF-4 functions. */
    if ((ret = test_nc4(iosysid, num_flavors, flavor, my_rank)))
        ERR(ret);

    /* Test scalar var. */
    if ((ret = test_scalar(iosysid, num_flavors, flavor, my_rank, async, test_comm)))
        ERR(ret);

    return PIO_NOERR;
}

/* Run all tests. */
int main(int argc, char **argv)
{
    /* Change the 5th arg to 3 to turn on logging. */
    return run_test_main(argc, argv, MIN_NTASKS, TARGET_NTASKS, -1,
                         TEST_NAME, dim_len, COMPONENT_COUNT, NUM_IO_PROCS);
}
