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

int
main(int argc, char **argv)
{
    printf("\n*** Testing netcdf-4 variable quantization functions.\n");
    printf("**** testing simple quantization and error conditions...");
    {
	int ncid;
	int nvars_in, varids_in;

	/* Create a netcdf-4 file with one scalar var. Attempt
	 * quantization. */
	if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
	if (nc_close(ncid)) ERR;

	/* Open the file and check. */
	if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
	if (nc_inq_varids(ncid, &nvars_in, &varids_in)) ERR;
	/* if (nvars_in != 1 || varids_in != 0) ERR; */
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
