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
   /* int ushort_varid, uint_varid, int64_varid, uint64_varid; */
   /* int i, j; */

   /* unsigned char ubyte_out[DIM1_LEN][DIM2_LEN] = {{1, 128, 255},{1, 128, 255}}; */
   /* signed char byte_in[DIM1_LEN][DIM2_LEN], byte_out[DIM1_LEN][DIM2_LEN] = {{-127, 1, 127},{-127, 1, 127}}; */
   /* unsigned short ushort_out[DIM1_LEN][DIM2_LEN] = {{110, 128, 255},{110, 128, 255}}; */
   /* short short_in[DIM1_LEN][DIM2_LEN], short_out[DIM1_LEN][DIM2_LEN] = {{-110, -128, 255},{-110, -128, 255}}; */
   /* int int_in[DIM1_LEN][DIM2_LEN], int_out[DIM1_LEN][DIM2_LEN] = {{0, 128, 255},{0, 128, 255}}; */
   /* float float_in[DIM1_LEN][DIM2_LEN], float_out[DIM1_LEN][DIM2_LEN] = {{-.1f, 9999.99f, 100.001f},{-.1f, 9999.99f, 100.001f}}; */
   /* double double_in[DIM1_LEN][DIM2_LEN], double_out[DIM1_LEN][DIM2_LEN] = {{0.02, .1128, 1090.1},{0.02, .1128, 1090.1}}; */
   /* unsigned int uint_in[DIM1_LEN][DIM2_LEN], uint_out[DIM1_LEN][DIM2_LEN] = {{0, 128, 255},{0, 128, 255}}; */
   /* long long int64_in[DIM1_LEN][DIM2_LEN], int64_out[DIM1_LEN][DIM2_LEN] = {{-111, 777, 100},{-111, 777, 100}}; */
   /* unsigned long long uint64_in[DIM1_LEN][DIM2_LEN]; */
   /* unsigned long long uint64_out[DIM1_LEN][DIM2_LEN] = {{0, 10101, 9999999},{0, 10101, 9999999}}; */
   /* char char_out[DIM1_LEN][DIM2_LEN][DIM3_LEN] = {{"lalala", "lololo", "lelele"}, {"lalala", "lololo", "lelele"}}; */

   printf("\n*** Testing netcdf-4 zstd compression.\n");

   printf("*** testing netcdf-4 zstd with float...");
   {
      int nvars_in, varids_in[2];

      /* Create a netcdf-3 file with one dim and two vars. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR_FLOAT_NAME, NC_FLOAT, 2, dimids, &float_varid)) ERR;
      if (nc_def_var(ncid, VAR_DOUBLE_NAME, NC_DOUBLE, 2, dimids, &double_varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and make sure nc_inq_varids yields correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
