/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>

#define FILE_NAME "tst_vars4.nc"

void
check_err(const int stat, const int line, const char *file) {
   if (stat != NC_NOERR) {
      (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
      fflush(stderr);
      exit(1);
   }
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 variable functions, even more.\n");
   printf("**** testing Jeff's dimension problem...");
   {
#define NDIMS2 2
#define NUM_VARS 1
#define Y_NAME "y"
#define X_NAME "x"
#define VAR_NAME Y_NAME
#define XDIM_LEN 2
#define YDIM_LEN 5

      int varid, ncid, dims[NDIMS2], dims_in[NDIMS2];
      int ndims, nvars, ngatts, unlimdimid, natts;
      char name_in[NC_MAX_NAME + 1];
      nc_type type_in;
      size_t len_in;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
      if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, 2, dims, &varid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS2 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME) || type_in != NC_FLOAT || ndims != NDIMS2 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
      if (strcmp(name_in, Y_NAME)) ERR;
      if (len_in != YDIM_LEN) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS2 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME) || type_in != NC_FLOAT || ndims != NDIMS2 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
      if (strcmp(name_in, Y_NAME)) ERR;
      if (len_in != YDIM_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing setting chunking without chunksizes...");
   {
#define NDIMS3 3
#define NUM_VARS 1
#define Y_NAME "y"
#define X_NAME "x"
#define Z_NAME "z"
#define JOE_NAME "joe"
#define XDIM_LEN 2
#define YDIM_LEN 5
#define ZDIM_LEN 3000

      int varid, ncid, dims[NDIMS3], dims_in[NDIMS3];
      int ndims, nvars, ngatts, unlimdimid, natts;
      char name_in[NC_MAX_NAME + 1];
      nc_type type_in;
      size_t len_in;
      int storage = 0;
      size_t chunksizes[NDIMS3];

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
      if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
      if (nc_def_dim(ncid, Z_NAME, ZDIM_LEN, &dims[2])) ERR;
      if (nc_def_var(ncid, JOE_NAME, NC_FLOAT, NDIMS3, dims, &varid)) ERR;
      if (nc_def_var_chunking(ncid, 0, NC_CHUNKED, NULL)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS3 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, JOE_NAME) || type_in != NC_FLOAT || ndims != NDIMS3 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || dims_in[2] != dims[2] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
      if (strcmp(name_in, Y_NAME) || len_in != YDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 2, name_in, &len_in)) ERR;
      if (strcmp(name_in, Z_NAME) || len_in != ZDIM_LEN) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS3 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, JOE_NAME) || type_in != NC_FLOAT || ndims != NDIMS3 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || dims_in[2] != dims[2] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
      if (strcmp(name_in, Y_NAME) || len_in != YDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 2, name_in, &len_in)) ERR;
      if (strcmp(name_in, Z_NAME) || len_in != ZDIM_LEN) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes...");
   {
#   define RANK_M 3
#   define RANK_time 1

      int ncid;
      size_t nsets_len = NC_UNLIMITED;
      size_t npoints_len = 152750;
      size_t n_variables_len = 11;
      int measurements_id, time_id, measurements_dims[RANK_M];
      int time_dims[RANK_time], time_data[1] = {1} ;
      size_t time_startset[1] = {0} ;
      size_t time_countset[1] = {1} ;
      int ndims, nvars, ngatts, unlimdimid;
      int storage;
      size_t chunksizes[RANK_M];

      /* Create a netCDF-4 file with two vars, and write some data to one of them. */
      if (nc_create(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, "nsets", nsets_len, &measurements_dims[0])) ERR;
      if (nc_def_dim(ncid, "npoints", npoints_len, &measurements_dims[1])) ERR;
      if (nc_def_dim(ncid, "n_variables", n_variables_len, &measurements_dims[2])) ERR;
/*      nc_set_log_level(4);*/
      if (nc_def_var(ncid, "measurements", NC_FLOAT, RANK_M, measurements_dims, 
		     &measurements_id)) ERR;
      time_dims[0] = measurements_dims[0];
      if (nc_def_var(ncid, "time", NC_INT, RANK_time, time_dims, &time_id)) ERR;
      if (nc_put_vara(ncid, time_id, time_startset, time_countset, time_data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen and check file. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 2 || ndims != 3 || ngatts != 0 || unlimdimid != 0) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}






