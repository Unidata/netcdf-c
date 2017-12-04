/*
  Copyright 2007, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This is part of netCDF.

  This program runs some extra tests.

  $Id: tst_misc.c,v 1.6 2010/05/05 22:15:36 dmh Exp $
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "netcdf.h"
#include "nc_tests.h"
#include "err_macros.h"
#ifdef USE_PARALLEL
#include "netcdf_par.h"
#endif

#define FILE_NAME "tst_misc.nc"

int
main(int argc, char **argv)
{
#ifdef TEST_PNETCDF
   MPI_Init(&argc, &argv);
#endif
   printf("\n*** Testing some extra stuff.\n");
   printf("*** Trying to open non-netCDF files of tiny length...");
   {
#define DATA_LEN 32
     int ncid,openstat;
      char dummy_data[DATA_LEN];
      FILE *file;
      int i;

      /* Appease valgrind by initializing our data. */
      for (i = 0; i < DATA_LEN; i++)
	 dummy_data[i] = i;

      for (i = DATA_LEN; i >= 0; i--)
      {
	 /* Create a small file which is not a netCDF file. */
	 if (!(file = fopen(FILE_NAME, "w+"))) ERR;
	 if (fwrite(dummy_data, 1, i, file) != i) ERR;
	 if (fclose(file)) ERR;

	 /* Make sure that netCDF rejects this file politely. */
#ifdef TEST_PNETCDF
        openstat = nc_open_par(FILE_NAME, NC_PNETCDF, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid);
#else
         openstat = nc_open(FILE_NAME, 0, &ncid);
#endif
	 /* Some platforms (OSX, buddy) return stat = 2 (file not found)
	    for index i == 2.  Not sure why, but this is a work around. */
	 if(openstat != NC_ENOTNC && openstat != 2) ERR;

      }
   }

   SUMMARIZE_ERR;
#ifndef USE_NETCDF4   
   printf("*** Trying to create netCDF-4 file without netCDF-4...");
   {
       int ncid;
       
       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid) != NC_ENOTBUILT)
	   ERR;
   }
   SUMMARIZE_ERR;
#endif /* USE_NETCDF4 undefined */
#ifndef USE_DISKLESS   
   printf("*** Trying to create diskless file without diskless...");
   {
       int ncid;
       
       if (nc_create(FILE_NAME, NC_DISKLESS, &ncid) != NC_ENOTBUILT)
	   ERR;
   }
   SUMMARIZE_ERR;
#endif /* USE_DISKLESS undefined */
   
#ifdef TEST_PNETCDF
   MPI_Finalize();
#endif
   
   FINAL_RESULTS;
}
