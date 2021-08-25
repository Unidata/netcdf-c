/* This is part of the netCDF package.
   Copyright 2021 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test quantization of netcdf-4 variables. Quantization is the
   zeroing-out of bits in float or double data beyond a desired
   precision.

   Ed Hartnett, 8/19/21
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"

#define FILE_NAME "tst_quantize.nc"
#define NDIMS1 1
#define DIM_NAME_1 "meters_along_canal"
#define DIM_LEN_1 10
#define VAR_NAME_1 "Amsterdam_houseboat_location"
#define NSD_1 3

int
main(int argc, char **argv)
{
    printf("\n*** Testing netcdf-4 variable quantization functions.\n");
    printf("**** testing simple quantization and error conditions...");
    {
	int ncid, dimid, varid;
	int quantize_mode_in, nsd_in;

	/* Create a netcdf classic file with one var. Attempt
	 * quantization. It will not work. */
	if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
	if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_1, &dimid)) ERR;
	if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIMS1, &dimid, &varid)) ERR;
	if (nc_def_var_quantize(ncid, varid, NC_QUANTIZE_BITGROOM, NSD_1) != NC_ENOTNC4) ERR;
	if (nc_inq_var_quantize(ncid, varid, &quantize_mode_in, &nsd_in) != NC_ENOTNC4) ERR;
	if (nc_close(ncid)) ERR;

	/* Create a netcdf-4 file with one var. Attempt
	 * quantization. It will work, eventually... */
	if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
	if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_1, &dimid)) ERR;
	if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIMS1, &dimid, &varid)) ERR;

	/* Bad varid. */
	if (nc_def_var_quantize(ncid, NC_GLOBAL, NC_QUANTIZE_BITGROOM, NSD_1) != NC_EGLOBAL) ERR;
	if (nc_def_var_quantize(ncid, 1, NC_QUANTIZE_BITGROOM, NSD_1) != NC_ENOTVAR) ERR;
	/* Invalid values. */
	if (nc_def_var_quantize(ncid, varid, NC_QUANTIZE_BITGROOM + 1, NSD_1) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid, NC_QUANTIZE_BITGROOM, -1) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid, NC_QUANTIZE_BITGROOM, 8) != NC_EINVAL) ERR;

	/* This will work. */
	if (nc_def_var_quantize(ncid, varid, NC_QUANTIZE_BITGROOM, NSD_1)) ERR;
	if (nc_inq_var_quantize(ncid, varid, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_QUANTIZE_BITGROOM) ERR;
	if (nsd_in != NSD_1) ERR;
	if (nc_close(ncid)) ERR;

	/* Open the file and check. */
	if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
	if (nc_inq_var_quantize(ncid, 0, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_QUANTIZE_BITGROOM) ERR;
	if (nsd_in != NSD_1) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
