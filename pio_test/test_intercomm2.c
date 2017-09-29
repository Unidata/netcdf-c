/*
 * Tests for PIOc_Intercomm. This tests the Init_Intercomm()
 * function, and basic asynch I/O capability.
 *
 * To run with valgrind, use this command:
 * <pre>mpiexec -n 4 valgrind -v --leak-check=full --suppressions=../../../tests/unit/valsupp_test.supp
 * --error-exitcode=99 --track-origins=yes ./test_intercomm2</pre>
 *
 * @author Ed Hartnett
 *
 */
#include <pio.h>
#include <pio_tests.h>
#include <pio_internal.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_intercomm2"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 2

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* The number of dimensions in the test data. */
#define NDIM 1

/* The length of our test data. */
#define DIM_LEN 4

/* Number of netCDF atomic types. */
#define NUM_TYPES 11

/* The name of the dimension in the netCDF output file. */
#define FIRST_DIM_NAME "jojo"
#define DIM_NAME "dim_test_intercomm2"

/* The name of the variable in the netCDF output file. */
#define FIRST_VAR_NAME "bill"
#define VAR_NAME "var_test_intercomm2"

/* The name of the global attribute in the netCDF output file. */
#define FIRST_ATT_NAME "willy_gatt_test_intercomm2"
#define ATT_NAME "gatt_test_intercomm2"
#define SHORT_ATT_NAME "short_gatt_test_intercomm2"
#define FLOAT_ATT_NAME "float_gatt_test_intercomm2"
#define DOUBLE_ATT_NAME "double_gatt_test_intercomm2"

/* The value of the global attribute in the netCDF output file. */
#define ATT_VALUE 42

/* Check the file for correctness. */
int check_file(int iosysid, int format, char *filename, int my_rank)
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
    char too_long_name[PIO_MAX_NAME * 5 + 1];

    /* Create a name that is too long. */
    memset(too_long_name, 74, PIO_MAX_NAME * 5);
    too_long_name[PIO_MAX_NAME * 5] = 0;

    /* Re-open the file to check it. */
    if ((ret = PIOc_openfile(iosysid, &ncid, &format, filename,
                             NC_NOWRITE)))
        ERR(ret);

    /* Try to read the data. */
    PIO_Offset start[NDIM] = {0}, count[NDIM] = {DIM_LEN};
    int data_in[DIM_LEN];
    /* Check with various combinations of start/count */
    for (int start_index = 0; start_index < DIM_LEN; start_index++)
    {
        start[0] = start_index;
        count[0] = DIM_LEN - start_index;
        if ((ret = PIOc_get_vars_tc(ncid, 0, start, count, NULL, NC_INT, data_in)))
            ERR(ret);
        for (int i = 0; i < count[0]; i++)
        {
            if (data_in[i] != (i + start_index))
                ERR(ERR_AWFUL);
        }
    }

    /* Find the number of dimensions, variables, and global attributes.*/
    if ((ret = PIOc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)))
        ERR(ret);
    if (ndims != 1 || nvars != 1 || ngatts != 4 || unlimdimid != -1)
        ERR(ERR_WRONG);

    /* This should return PIO_NOERR. */
    if ((ret = PIOc_inq(ncid, NULL, NULL, NULL, NULL)))
        ERR(ret);

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_ndims(ncid, &ndims2)))
        ERR(ret);
    if (ndims2 != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_nvars(ncid, &nvars2)))
        ERR(ret);
    if (nvars2 != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_natts(ncid, &ngatts2)))
        ERR(ret);
    if (ngatts2 != 4)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_unlimdim(ncid, &unlimdimid2)))
        ERR(ret);
    if (unlimdimid != -1)
        ERR(ERR_WRONG);
    /* Should succeed, do nothing. */
    if ((ret = PIOc_inq_unlimdim(ncid, NULL)))
        ERR(ret);

    /* Check out the dimension. */
    if ((ret = PIOc_inq_dim(ncid, 0, dimname, &dimlen)))
        ERR(ret);
    if (strcmp(dimname, DIM_NAME) || dimlen != DIM_LEN)
        ERR(ERR_WRONG);

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_dimname(ncid, 0, dimname2)))
        ERR(ret);
    if (strcmp(dimname2, DIM_NAME))
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_dimlen(ncid, 0, &dimlen2)))
        ERR(ret);
    if (dimlen2 != DIM_LEN)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_dimid(ncid, DIM_NAME, &dimid2)))
        ERR(ret);
    if (dimid2 != 0)
        ERR(ERR_WRONG);

    /* These should not work. */
    if (PIOc_inq_dimid(ncid + TEST_VAL_42, DIM_NAME, &dimid2) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_inq_dimid(ncid, too_long_name, &dimid2) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Check out the variable. */
    if ((ret = PIOc_inq_var(ncid, 0, varname, &vartype, &varndims, &vardimids, &varnatts)))
        ERR(ret);
    if (strcmp(varname, VAR_NAME) || vartype != NC_INT || varndims != NDIM ||
        vardimids != 0 || varnatts != 0)
        ERR(ERR_WRONG);

    /* Check the other functions that get these values. */
    if ((ret = PIOc_inq_varname(ncid, 0, varname2)))
        ERR(ret);
    if (strcmp(varname2, VAR_NAME))
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_vartype(ncid, 0, &vartype2)))
        ERR(ret);
    if (vartype2 != NC_INT)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_varndims(ncid, 0, &varndims2)))
        ERR(ret);
    if (varndims2 != NDIM)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_vardimid(ncid, 0, &vardimids2)))
        ERR(ret);
    if (vardimids2 != 0)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_varnatts(ncid, 0, &varnatts2)))
        ERR(ret);
    if (varnatts2 != 0)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_varid(ncid, VAR_NAME, &varid2)))
        ERR(ret);
    if (varid2 != 0)
        ERR(ERR_WRONG);

    /* These should not work. */
    if (PIOc_inq_varid(ncid + TEST_VAL_42, VAR_NAME, &varid2) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_inq_varid(ncid, NULL, &varid2) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_inq_varid(ncid, too_long_name, &varid2) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Check out the global attributes. */
    nc_type atttype;
    PIO_Offset attlen;
    char myattname[NC_MAX_NAME + 1];
    int myid;
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &atttype, &attlen)))
        ERR(ret);
    if (atttype != NC_INT || attlen != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_attlen(ncid, NC_GLOBAL, ATT_NAME, &attlen)))
        ERR(ret);
    if (attlen != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_attname(ncid, NC_GLOBAL, 0, myattname)))
        ERR(ret);
    if (strcmp(ATT_NAME, myattname))
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_attid(ncid, NC_GLOBAL, ATT_NAME, &myid)))
        ERR(ret);
    if (PIOc_inq_attid(ncid + TEST_VAL_42, NC_GLOBAL, ATT_NAME, &myid) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (myid != 0)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_int(ncid, NC_GLOBAL, ATT_NAME, &att_data)))
        ERR(ret);
    if (att_data != ATT_VALUE)
        ERR(ERR_WRONG);
    if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, SHORT_ATT_NAME, &atttype, &attlen)))
        ERR(ret);
    if (atttype != NC_SHORT || attlen != 1)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_short(ncid, NC_GLOBAL, SHORT_ATT_NAME, &short_att_data)))
        ERR(ret);
    if (short_att_data != ATT_VALUE)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_float(ncid, NC_GLOBAL, FLOAT_ATT_NAME, &float_att_data)))
        ERR(ret);
    if (float_att_data != ATT_VALUE)
        ERR(ERR_WRONG);
    if ((ret = PIOc_get_att_double(ncid, NC_GLOBAL, DOUBLE_ATT_NAME, &double_att_data)))
        ERR(ret);
    if (double_att_data != ATT_VALUE)
        ERR(ERR_WRONG);

    /* These should not work. */
    if (PIOc_inq_att(ncid + TEST_VAL_42, NC_GLOBAL, ATT_NAME, &atttype, &attlen) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_inq_att(ncid + TEST_VAL_42, NC_GLOBAL, NULL, &atttype, &attlen) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_inq_att(ncid + TEST_VAL_42, NC_GLOBAL, too_long_name, &atttype, &attlen) != PIO_EBADID)
        ERR(ERR_WRONG);
    if (PIOc_get_att(ncid, NC_GLOBAL, TEST_NAME, &att_data) != PIO_ENOTATT)
        ERR(ERR_WRONG);
    if (PIOc_get_att(ncid, NC_GLOBAL, NULL, &att_data) != PIO_EINVAL)
        ERR(ERR_WRONG);
    if (PIOc_get_att(ncid, NC_GLOBAL, too_long_name, &att_data) != PIO_EINVAL)
        ERR(ERR_WRONG);

    /* Close the file. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);

    return 0;
}

/* Run Tests for Init_Intercomm. */
int main(int argc, char **argv)
{
    /* Zero-based rank of processor. */
    int my_rank;

    /* Number of processors involved in current execution. */
    int ntasks;

    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */

    /* Names for the output files. */
    char filename[NUM_FLAVORS][NC_MAX_NAME + 1];

    /* The ID for the parallel I/O system. */
    int iosysid[COMPONENT_COUNT];

    /* Return code. */
    int ret;

    MPI_Comm test_comm;

    char too_long_name[PIO_MAX_NAME * 5 + 1];

    /* Create a name that is too long. */
    memset(too_long_name, 74, PIO_MAX_NAME * 5);
    too_long_name[PIO_MAX_NAME * 5] = 0;

    /* Set up test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, TARGET_NTASKS, TARGET_NTASKS,
                              -1, &test_comm)))
        ERR(ERR_INIT);

    /* Figure out iotypes. */
    if ((ret = get_iotypes(&num_flavors, flavor)))
        ERR(ret);

    if (my_rank < TARGET_NTASKS)
    {

        /* How many processors will be used for our IO and 2 computation components. */
        int num_procs[COMPONENT_COUNT] = {2};

        /* Is the current process a computation task? */
        int comp_task = my_rank < 2 ? 0 : 1;

        /* Index of computation task in iosysid array. Varies by rank and
         * does not apply to IO component processes. */
        int my_comp_idx = comp_task ? 0 : -1;

        /* Initialize the IO system. */
        if ((ret = PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                                   num_procs, NULL, NULL, NULL, PIO_REARR_BOX, iosysid)))
            ERR(ERR_AWFUL);

        /* All the netCDF calls are only executed on the computation
         * tasks. The IO tasks have not returned from PIOc_Init_Intercomm,
         * and when the do, they should go straight to finalize. */
        if (comp_task)
        {
            for (int fmt = 0; fmt < num_flavors; fmt++)
            {
                int ncid, varid, dimid;
                PIO_Offset start[NDIM], count[NDIM] = {0};
                int data[DIM_LEN];

                /* Create the filename for this flavor. */
                sprintf(filename[fmt], "test_intercomm2_%d.nc", flavor[fmt]);

                /* Create a netCDF file with one dimension and one variable. */
                if ((ret = PIOc_createfile(iosysid[my_comp_idx], &ncid, &flavor[fmt], filename[fmt],
                                           NC_CLOBBER)))
                    ERR(ret);

                /* End define mode, then re-enter it. */
                if ((ret = PIOc_enddef(ncid)))
                    ERR(ret);
                if ((ret = PIOc_redef(ncid)))
                    ERR(ret);

                /* Test the inq_format function. */
                int myformat;
                if (PIOc_inq_format(ncid + TEST_VAL_42, &myformat) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if ((ret = PIOc_inq_format(ncid, &myformat)))
                    ERR(ret);
                if ((flavor[fmt] == PIO_IOTYPE_PNETCDF || flavor[fmt] == PIO_IOTYPE_NETCDF) &&
                    myformat != 1)
                    ERR(ERR_AWFUL);
                else if ((flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P) &&
                         myformat != 3)
                    ERR(ERR_AWFUL);

                /* Test the inq_type function for atomic types. */
                char type_name[NC_MAX_NAME + 1];
                PIO_Offset type_size;
                nc_type xtype[NUM_TYPES] = {NC_CHAR, NC_BYTE, NC_SHORT, NC_INT, NC_FLOAT, NC_DOUBLE,
                                            NC_UBYTE, NC_USHORT, NC_UINT, NC_INT64, NC_UINT64};
                int type_len[NUM_TYPES] = {1, 1, 2, 4, 4, 8, 1, 2, 4, 8, 8};
                int max_type = flavor[fmt] == PIO_IOTYPE_NETCDF ? NC_DOUBLE : NC_UINT64;

                /* This should not work. */
                if (PIOc_inq_type(ncid + TEST_VAL_42, xtype[0], type_name, &type_size) != PIO_EBADID)
                    ERR(ERR_WRONG);

                /* These should work. */
                for (int i = 0; i < max_type; i++)
                {
                    if ((ret = PIOc_inq_type(ncid, xtype[i], type_name, &type_size)))
                        ERR(ret);
                    if (type_size != type_len[i])
                        ERR(ERR_AWFUL);
                }

                /* Define a dimension. */
                char dimname2[NC_MAX_NAME + 1];
                if ((ret = PIOc_def_dim(ncid, FIRST_DIM_NAME, DIM_LEN, &dimid)))
                    ERR(ret);
                if ((ret = PIOc_inq_dimname(ncid, 0, dimname2)))
                    ERR(ret);
                if (strcmp(dimname2, FIRST_DIM_NAME))
                    ERR(ERR_WRONG);
                if ((ret = PIOc_rename_dim(ncid, 0, DIM_NAME)))
                    ERR(ret);

                /* These should not work. */
                if (PIOc_rename_dim(ncid + TEST_VAL_42, 0, DIM_NAME) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if (PIOc_rename_dim(ncid, 0, NULL) != PIO_EINVAL)
                    ERR(ERR_WRONG);
                if (PIOc_rename_dim(ncid, 0, too_long_name) != PIO_EINVAL)
                    ERR(ERR_WRONG);

                /* Define a 1-D variable. */
                char varname2[NC_MAX_NAME + 1];
                if ((ret = PIOc_def_var(ncid, FIRST_VAR_NAME, NC_INT, NDIM, &dimid, &varid)))
                    ERR(ret);
                if ((ret = PIOc_inq_varname(ncid, 0, varname2)))
                    ERR(ret);
                if (strcmp(varname2, FIRST_VAR_NAME))
                    ERR(ERR_WRONG);
                if ((ret = PIOc_rename_var(ncid, 0, VAR_NAME)))
                    ERR(ret);

                /* These should not work. */
                if (PIOc_rename_var(ncid + TEST_VAL_42, 0, VAR_NAME) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if (PIOc_rename_var(ncid, 0, NULL) != PIO_EINVAL)
                    ERR(ERR_WRONG);
                if (PIOc_rename_var(ncid, 0, too_long_name) != PIO_EINVAL)
                    ERR(ERR_WRONG);

                /* Add a global attribute. */
                int att_data = ATT_VALUE;
                short short_att_data = ATT_VALUE;
                float float_att_data = ATT_VALUE;
                double double_att_data = ATT_VALUE;
                char attname2[NC_MAX_NAME + 1];

                /* Write an att and rename it. */
                if ((ret = PIOc_put_att_int(ncid, NC_GLOBAL, FIRST_ATT_NAME, NC_INT, 1, &att_data)))
                    ERR(ret);
                if ((ret = PIOc_inq_attname(ncid, NC_GLOBAL, 0, attname2)))
                    ERR(ret);
                if (strcmp(attname2, FIRST_ATT_NAME))
                    ERR(ERR_WRONG);
                if ((ret = PIOc_rename_att(ncid, NC_GLOBAL, FIRST_ATT_NAME, ATT_NAME)))
                    ERR(ret);

                /* These should not work. */
                if (PIOc_inq_attname(ncid + TEST_VAL_42, NC_GLOBAL, 0, attname2) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if (PIOc_rename_att(ncid + TEST_VAL_42, NC_GLOBAL, FIRST_ATT_NAME, ATT_NAME) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if (PIOc_rename_att(ncid, NC_GLOBAL, FIRST_ATT_NAME, NULL) != PIO_EINVAL)
                    ERR(ERR_WRONG);
                if (PIOc_rename_att(ncid, NC_GLOBAL, FIRST_ATT_NAME, too_long_name) != PIO_EINVAL)
                    ERR(ERR_WRONG);
                if (PIOc_del_att(ncid + TEST_VAL_42, NC_GLOBAL, FIRST_ATT_NAME) != PIO_EBADID)
                    ERR(ERR_WRONG);
                if (PIOc_del_att(ncid, NC_GLOBAL, NULL) != PIO_EINVAL)
                    ERR(ERR_WRONG);
                if (PIOc_del_att(ncid, NC_GLOBAL, too_long_name) != PIO_EINVAL)
                    ERR(ERR_WRONG);

                /* Write an att and delete it. */
                if ((ret = PIOc_put_att_int(ncid, NC_GLOBAL, FIRST_ATT_NAME, NC_INT, 1, &att_data)))
                    ERR(ret);
                if ((ret = PIOc_del_att(ncid, NC_GLOBAL, FIRST_ATT_NAME)))
                    ERR(ret);
                /* if ((ret = PIOc_inq_att(ncid, NC_GLOBAL, FIRST_ATT_NAME, NULL, NULL)) != PIO_ENOTATT) */
                /* { */
                /*      printf("ret = %d\n", ret); */
                /*      ERR(ERR_AWFUL); */
                /* } */

                /* Write some atts of different types. */
                if ((ret = PIOc_put_att_short(ncid, NC_GLOBAL, SHORT_ATT_NAME, NC_SHORT, 1, &short_att_data)))
                    ERR(ret);
                if ((ret = PIOc_put_att_float(ncid, NC_GLOBAL, FLOAT_ATT_NAME, NC_FLOAT, 1, &float_att_data)))
                    ERR(ret);
                if ((ret = PIOc_put_att_double(ncid, NC_GLOBAL, DOUBLE_ATT_NAME, NC_DOUBLE, 1, &double_att_data)))
                    ERR(ret);

                /* Check some att types. */
                nc_type myatttype;
                if ((ret = PIOc_inq_atttype(ncid, NC_GLOBAL, SHORT_ATT_NAME, &myatttype)))
                    ERR(ret);
                if (myatttype != NC_SHORT)
                    ERR(ERR_WRONG);
                if ((ret = PIOc_inq_atttype(ncid, NC_GLOBAL, FLOAT_ATT_NAME, &myatttype)))
                    ERR(ret);
                if (myatttype != NC_FLOAT)
                    ERR(ERR_WRONG);
                if ((ret = PIOc_inq_atttype(ncid, NC_GLOBAL, DOUBLE_ATT_NAME, &myatttype)))
                    ERR(ret);
                if (myatttype != NC_DOUBLE)
                    ERR(ERR_WRONG);

                /* End define mode. */
                if ((ret = PIOc_enddef(ncid)))
                    ERR(ret);

                /* Write some data. For the PIOc_put/get functions, all
                 * data must be on compmaster before the function is
                 * called. Only compmaster's arguments are passed to the
                 * async msg handler. All other computation tasks are
                 * ignored. */
                for (int i = 0; i < DIM_LEN; i++)
                    data[i] = i;
                start[0] = 0;
                count[0] = DIM_LEN;
                if ((ret = PIOc_put_vars_tc(ncid, varid, start, count, NULL, NC_INT, data)))
                    ERR(ret);

                /* Close the file. */
                if ((ret = PIOc_closefile(ncid)))
                    ERR(ret);

                /* Check the file for correctness. */
                if ((ret = check_file(iosysid[my_comp_idx], flavor[fmt], filename[fmt], my_rank)))
                    ERR(ret);

                /* Now delete the file. */
                /* if ((ret = PIOc_deletefile(iosysid, filename[fmt]))) */
                /*      ERR(ret); */
                /* if ((ret = PIOc_openfile(iosysid, &ncid, &flavor[fmt], filename[fmt], */
                /*                           NC_NOWRITE)) != PIO_ENFILE) */
                /*      ERR(ERR_AWFUL); */

            } /* next netcdf flavor */

            /* Finalize the IO system. Only call this from the computation tasks. */
            if ((ret = PIOc_finalize(iosysid[my_comp_idx])))
                ERR(ret);
        }
    } /* my_rank < TARGET_NTASKS */

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
