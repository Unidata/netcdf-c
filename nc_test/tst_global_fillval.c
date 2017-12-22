/* This is part of the netCDF package. Copyright 2017 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test proper elatefill return when fillvalue is assigned outside of
   the initial define.

   Contributed by wkliao, see the following for more information:

   * https://github.com/Unidata/netcdf-c/issues/388
   * https://github.com/Unidata/netcdf-c/pull/389

   Modified by Ed Hartnett, see:
   https://github.com/Unidata/netcdf-c/issues/392
   */

#include "config.h"
#include <nc_tests.h>
#include "err_macros.h"

#define FILE_NAME "tst_global_fillval.nc"

/* Unweildy, but currently this structure must be used
   to accomodate Visual Studio */
#if defined USE_NETCDF4 && USE_CDF5
#define num_formats 5
#elif USE_NETCDF4
#define num_formats 4
#elif USE_CDF5
#define num_formats 3
#else
#define num_formats 2
#endif

int
main(int argc, char **argv)
{
   printf("*** testing proper elatefill return on %d formats %d...", num_formats, USE_CDF5);
    {

	int n = 0;
        int i;

	int formats[num_formats];
	formats[n++] = 0;
	formats[n++] = NC_64BIT_OFFSET;
#ifdef USE_CDF5
	formats[n++] = NC_64BIT_DATA;
#endif
#ifdef USE_NETCDF4
	formats[n++] = NC_NETCDF4;
	formats[n++] = NC_CLASSIC_MODEL | NC_NETCDF4;
#endif

	for (i = 0; i < num_formats; i++)
	{
	    int ncid, cmode, fillv = 9;

	    cmode = NC_CLOBBER | formats[i];
	    if (nc_create(FILE_NAME, cmode, &ncid)) ERR;
	    if (nc_put_att_int(ncid, NC_GLOBAL, "_FillValue", NC_INT, 1, &fillv)) ERR;
	    if (nc_close(ncid)) ERR;

	}
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
