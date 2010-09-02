/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf files a bit. 
*/

#include <nc_tests.h>
#include "netcdf.h"

#define URL "http://test.opendap.org:8080/dods/dts/test.01"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf file functions some more.\n");
#ifdef USE_DAP
#ifdef ENABLE_DAP_REMOTE_TESTS
   printf("*** testing simple opendap open/close...");
   {
      int ncid;

      /* Test with URL. */
      if (nc_open(URL, 0, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
#endif /*ENABLE_DAP_REMOTE_TESTS*/
#endif /* USE_DAP */
   FINAL_RESULTS;
}


