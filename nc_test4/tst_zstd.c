/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables.
   Ed Hartnett, 8/7/24
*/

#include "nc_tests.h"
#include "err_macros.h"
#include "netcdf.h"

#define FILE_NAME "tst_zstd.nc"
#define VAR_FLOAT_NAME "GALE_data"
#define VAR_DOUBLE_NAME "ERICA_data"
#define NDIM2 2
#define DIM1_NAME "x"
#define DIM1_LEN 2
#define DIM2_NAME "y"
#define DIM2_LEN 3

int
main(int argc, char **argv)
{
   int ncid, dimids[NDIM2];
   int float_varid, double_varid;
   float float_in[DIM1_LEN][DIM2_LEN], float_out[DIM1_LEN][DIM2_LEN] = {{-.1f, 9999.99f, 100.001f},{-.1f, 9999.99f, 100.001f}};
   double double_in[DIM1_LEN][DIM2_LEN], double_out[DIM1_LEN][DIM2_LEN] = {{0.02, .1128, 1090.1},{0.02, .1128, 1090.1}};
   int i, j, ret;


   printf("\n*** Testing netcdf-4 zstd compression.\n");

   printf("*** testing netcdf-4 zstd with float...");
   {
      int nvars_in, varids_in[2];

      /* Create a netcdf file with one dim and two vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR_FLOAT_NAME, NC_FLOAT, 2, dimids, &float_varid)) ERR;
      if (nc_def_var(ncid, VAR_DOUBLE_NAME, NC_DOUBLE, 2, dimids, &double_varid)) ERR;
      if ((ret = nc_def_var_zstandard(ncid, float_varid, 4)))
      {
	  printf("error: %s\n", nc_strerror(ret));
	  ERR;
      }
      if (nc_def_var_zstandard(ncid, double_varid, 4)) ERR;
      if (nc_put_var_float(ncid, float_varid, (float *)float_out)) ERR;
      if (nc_put_var_double(ncid, double_varid, (double *)double_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and make sure nc_inq_varids yields correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
      if (nc_get_var_float(ncid, float_varid, (float *)float_in)) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;
      for (i = 0; i < DIM1_LEN; i++)
      {
          for (j = 0; j < DIM2_LEN; j++)
          {
              if (float_in[i][j] != float_out[i][j]) ERR;
              if (double_in[i][j] != double_out[i][j]) ERR;
          }
      }
          
      
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
