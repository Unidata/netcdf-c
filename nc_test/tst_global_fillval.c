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

int
main(int argc, char **argv)
{
   int format[MAX_NUM_FORMATS];
   int num_formats;

   /* How many formats to be tested? */
   determine_test_formats(&num_formats, format);

   printf("*** testing proper elatefill return...");
   {

      int i;

      for (i = 0; i < num_formats; i++)
      {
         int ncid, fillv = 9;

         if (nc_set_default_format(format[i], NULL)) ERR;
         if (nc_create(FILE_NAME, 0, &ncid)) ERR;
         if (nc_put_att_int(ncid, NC_GLOBAL, "_FillValue", NC_INT, 1, &fillv)) ERR;
         if (nc_close(ncid)) ERR;

      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
