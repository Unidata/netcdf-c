/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables.
   Ed Hartnett
*/

#include <nc_tests.h>
#include "err_macros.h"

#define FILE_NAME "tst_vars4.nc"
#define NDIM2 2
#define NUM_VARS 1
#define Y_NAME "y"
#define X_NAME "x"
#define Z_NAME "z"
#define VAR_NAME Y_NAME
#define XDIM_LEN 2
#define YDIM_LEN 5
#define ZDIM_LEN 8193
#define CLAIR "Clair"
#define JAMIE "Jamie"

int
main(int argc, char **argv)
{
    printf("\n*** Testing netcdf-4 variable functions, even more.\n");
    printf("**** testing Jeff's dimension problem...");
    {
        int varid, ncid, dims[NDIM2], dims_in[NDIM2];
        int ndims, nvars, ngatts, unlimdimid, natts;
        char name_in[NC_MAX_NAME + 1];
        nc_type type_in;
        size_t len_in;

        nc_set_log_level(-1);
        if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
        if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
        if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
        if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, 2, dims, &varid)) ERR;
        if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
        if (nvars != NUM_VARS || ndims != NDIM2 || ngatts != 0 || unlimdimid != -1) ERR;
        if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
        if (strcmp(name_in, VAR_NAME) || type_in != NC_FLOAT || ndims != NDIM2 ||
            dims_in[0] != dims[0] || dims_in[1] != dims[1] || natts != 0) ERR;
        if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
        if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
        if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
        if (strcmp(name_in, Y_NAME)) ERR;
        if (len_in != YDIM_LEN) ERR;
        if (nc_close(ncid)) ERR;
        nc_set_log_level(-1);

        /* Open the file and check. */
        if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
        if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
        if (nvars != NUM_VARS || ndims != NDIM2 || ngatts != 0 || unlimdimid != -1) ERR;
        if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
        if (strcmp(name_in, VAR_NAME) || type_in != NC_FLOAT || ndims != NDIM2 ||
            dims_in[0] != dims[0] || dims_in[1] != dims[1] || natts != 0) ERR;
        if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
        if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
        if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
        if (strcmp(name_in, Y_NAME)) ERR;
        if (len_in != YDIM_LEN) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("**** testing chunking turned on by fletcher...");
    {
        int varid, ncid, dims[NDIM2];
        int storage_in;
        size_t chunksizes_in[NDIM2];

        if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
        if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
        if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
        if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, 2, dims, &varid)) ERR;
        if (nc_def_var_fletcher32(ncid, varid, NC_FLETCHER32)) ERR;
        if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksizes_in)) ERR;
        if (chunksizes_in[0] != XDIM_LEN || chunksizes_in[1] != YDIM_LEN) ERR;
        if (nc_close(ncid)) ERR;

        /* Open the file and check. */
        if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
        if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksizes_in)) ERR;
        if (chunksizes_in[0] != XDIM_LEN || chunksizes_in[1] != YDIM_LEN) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("**** testing chunking turned on by shuffle...");
    {
        int varid, ncid, dims[NDIM2];
        int storage_in;
        size_t chunksizes_in[NDIM2];

        if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
        if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
        if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
        if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, 2, dims, &varid)) ERR;
        if (nc_def_var_deflate(ncid, varid, NC_SHUFFLE, 0, 0)) ERR;
        if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksizes_in)) ERR;
        if (chunksizes_in[0] != XDIM_LEN || chunksizes_in[1] != YDIM_LEN) ERR;
        if (nc_close(ncid)) ERR;

        /* Open the file and check. */
        if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
        if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksizes_in)) ERR;
        if (chunksizes_in[0] != XDIM_LEN || chunksizes_in[1] != YDIM_LEN) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
#define DIM_NAME "Distance_from_Mayo"
#define VAR_NAME_2 "Rocky_Road_to_Dublin"
#define NDIMS1 1
#define NUM_RECORDS 3
    printf("**** testing extending var along unlimited dim with no coord var...");
    {
        int varid, ncid, dimid;
        int ndims, nvars, natts, unlimdimid;
        size_t dim_len_in, index;
        int data = TEST_VAL_42;

        /* Create the test file with one var, one unlimited dim. */
        if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
        if (nc_def_dim(ncid, DIM_NAME, NC_UNLIMITED, &dimid)) ERR;
        if (nc_def_var(ncid, VAR_NAME_2, NC_INT, NDIMS1, &dimid, &varid)) ERR;

        /* Write some records. */
        for (index = 0; index < NUM_RECORDS; index++)
            if (nc_put_var1_int(ncid, varid, &index, &data)) ERR;
        if (nc_close(ncid)) ERR;

        /* Open the file and check. */
        if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
        if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
        if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != 0) ERR;
        if (nc_inq_dim(ncid, dimid, NULL, &dim_len_in)) ERR;
        if (dim_len_in != NUM_RECORDS) ERR;

        /* Now add more records. */
        for (index = 3; index < NUM_RECORDS * 2; index++)
            if (nc_put_var1_int(ncid, varid, &index, &data)) ERR;
        if (nc_inq_dim(ncid, dimid, NULL, &dim_len_in)) ERR;

        if (dim_len_in != NUM_RECORDS * 2) ERR;

        /* Close the file. */
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("**** testing type creation and destruction for atomic types...");
    {
        int varid1, varid2, ncid;
        int ndims, nvars, natts, unlimdimid;

        /* Create the test file with two scalar vars. */
        if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
        if (nc_def_var(ncid, CLAIR, NC_INT, 0, NULL, &varid1)) ERR;
        if (nc_def_var(ncid, JAMIE, NC_INT, 0, NULL, &varid2)) ERR;
        if (nc_close(ncid)) ERR;

        /* Open the file and check. */
        if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
        if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
        if (ndims != 0 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("**** testing scalar big endian vars...");
    {
        int varid1, varid2, ncid;
        int ndims, nvars, natts, unlimdimid;
        int test_val = TEST_VAL_42;
        int test_val2 = TEST_VAL_42 * 2;
        int data_in;

        /* Create the test file with two scalar vars. */
        if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
        if (nc_def_var(ncid, CLAIR, NC_INT, 0, NULL, &varid1)) ERR;
        if (nc_def_var_endian(ncid, varid1, NC_ENDIAN_BIG)) ERR;
        if (nc_def_var(ncid, JAMIE, NC_INT, 0, NULL, &varid2)) ERR;
        if (nc_def_var_endian(ncid, varid2, NC_ENDIAN_BIG)) ERR;
        if (nc_enddef(ncid)) ERR;
        if (nc_put_var(ncid, varid1, &test_val)) ERR;
        if (nc_put_var(ncid, varid2, &test_val2)) ERR;
        if (nc_close(ncid)) ERR;

        /* Open the file and check. */
        if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
        if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
        if (ndims != 0 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
        if (nc_get_var(ncid, varid1, &data_in)) ERR;
        if (data_in != TEST_VAL_42) ERR;
        if (nc_get_var(ncid, varid2, &data_in)) ERR;
        if (data_in != TEST_VAL_42 * 2) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("**** testing scalar big endian vars...");
    {
        int ncid, enumid;
        int bigid, littleid;
        int endian_in;
        /* Note: if no zero valued enum, then causes ncdump error */
        int econst0 = 0;
        int econst1 = 1;

        if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;

        if (nc_def_enum(ncid, NC_INT, "enum_t", &enumid)) ERR;
        if (nc_insert_enum(ncid, enumid, "econst0", &econst0)) ERR;
        if (nc_insert_enum(ncid, enumid, "econst1", &econst1)) ERR;

        if (nc_def_var(ncid, "little", enumid, 0, NULL, &littleid)) ERR;
        if (nc_def_var(ncid, "big", enumid, 0, NULL, &bigid)) ERR;

        if (nc_def_var_endian(ncid, littleid, NC_ENDIAN_LITTLE) != NC_EINVAL) ERR;
        if (nc_def_var_endian(ncid, bigid, NC_ENDIAN_BIG) != NC_EINVAL) ERR;

        /* Note that it is important to set endian ness before testing it */
        if (nc_inq_var_endian(ncid, littleid, &endian_in)) ERR;
        if (endian_in) ERR;
        if (nc_inq_var_endian(ncid, bigid, &endian_in)) ERR;
        if (endian_in) ERR;

        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("**** testing compact storage with one scalar var...");
    {
        int ncid, varid;
        int storage_in;
        int data = TEST_VAL_42;

        /* Create a file with one var which is compact scalar. */
        if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;

        /* Define a scalar. Scalars can also be compact. */
        if (nc_def_var(ncid, JAMIE, NC_INT, 0, NULL, &varid)) ERR;
        if (nc_def_var_chunking(ncid, varid, NC_COMPACT, NULL)) ERR;

        /* Write data. */
        if (nc_put_var_int(ncid, varid, &data)) ERR;

        /* Close file. */
        if (nc_close(ncid)) ERR;

        /* Open the file and check it. */
        {
            int ndims, nvars;

            if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
            if (nc_inq(ncid, &ndims, &nvars, NULL, NULL)) ERR;
            if (ndims != 0 || nvars != 1) ERR;
            if (nc_inq_var_chunking(ncid, 0, &storage_in, NULL)) ERR;
            if (storage_in != NC_COMPACT) ERR;
            if (nc_close(ncid)) ERR;
        }
    }
    SUMMARIZE_ERR;
    printf("**** testing compact storage...");
    {
        int ncid, dimid[NDIM2], varid, varid2, varid3;
        int data[XDIM_LEN];
        int storage_in;
        int x;

        /* Create some data. */
        for (x = 0; x < XDIM_LEN; x++)
            data[x] = x;

        /* Create a file with one var with compact storage. */
        if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;

        /* Define dims. */
        if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dimid[0])) ERR;
        if (nc_def_dim(ncid, Z_NAME, ZDIM_LEN, &dimid[1])) ERR;

        /* Define vars1 to be compact. */
        if (nc_def_var(ncid, Y_NAME, NC_INT, 1, dimid, &varid)) ERR;
        if (nc_def_var_chunking(ncid, varid, NC_COMPACT, NULL)) ERR;
        if (nc_inq_var_chunking(ncid, varid, &storage_in, NULL)) ERR;
        if (storage_in != NC_COMPACT) ERR;

        /* Define var2 - it's too big for compact. */
        if (nc_def_var(ncid, CLAIR, NC_INT, NDIM2, dimid, &varid2)) ERR;
        /* This won't work, the var is too big for compact! */
        if (nc_def_var_chunking(ncid, varid2, NC_COMPACT, NULL) != NC_EVARSIZE) ERR;

        /* Define var3, a scalar. Scalars can also be compact. */
        if (nc_def_var(ncid, JAMIE, NC_INT, 0, NULL, &varid3)) ERR;
        if (nc_def_var_chunking(ncid, varid3, NC_COMPACT, NULL)) ERR;

        /* Write data. */
        if (nc_put_var_int(ncid, varid3, data)) ERR;

        /* Close file. */
        if (nc_close(ncid)) ERR;

        /* Open the file and check it. */
        {
            int ndims, nvars;
            int storage_in;
            if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
            if (nc_inq(ncid, &ndims, &nvars, NULL, NULL)) ERR;
            if (ndims != 2 || nvars != 3) ERR;
            if (nc_inq_var_chunking(ncid, varid, &storage_in, NULL)) ERR;
            if (storage_in != NC_COMPACT) ERR;
            if (nc_inq_var_chunking(ncid, varid2, &storage_in, NULL)) ERR;
            if (storage_in != NC_CONTIGUOUS) ERR;
            if (nc_inq_var_chunking(ncid, varid3, &storage_in, NULL)) ERR;
            if (storage_in != NC_COMPACT) ERR;
            if (nc_close(ncid)) ERR;
        }
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
