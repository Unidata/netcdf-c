/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.


 * Test some illegal mode combinations
 *
 */

#include "nc_tests.h"
#include "err_macros.h"
#include "netcdf_par.h"

#define FILE_NAME "tst_mode.nc"

int
main(int argc, char** argv)
{
   int ncid,varid;
   int retval;

   printf("\n*** Testing illegal mode combinations\n");

   MPI_Init(&argc,&argv);

   printf("*** Testing create + MPIO + fletcher32\n");
   if ((retval = nc_create_par(FILE_NAME, NC_CLOBBER|NC_NETCDF4, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid))) ERR;
   if ((retval = nc_def_var(ncid,"whatever",NC_INT,0,NULL,&varid))) ERR;
   retval = nc_def_var_fletcher32(ncid,varid,NC_FLETCHER32);
   if(retval != NC_EINVAL) ERR;
   if ((retval = nc_abort(ncid)))
{
fprintf(stderr,"XXX: err=%d\n",retval);
fflush(stderr);
ERR;
}

   printf("*** Testing create + MPIO + deflation\n");
   if ((retval = nc_create_par(FILE_NAME, NC_CLOBBER|NC_NETCDF4, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid))) ERR;
   if ((retval = nc_def_var(ncid,"whatever",NC_INT,0,NULL,&varid))) ERR;
   retval = nc_def_var_deflate(ncid,varid, NC_NOSHUFFLE, 1, 1);
   if(retval != NC_EINVAL) ERR;
   if ((retval = nc_abort(ncid))) {
	fprintf(stderr,"XXX: nc_abort: %d\n",retval); fflush(stderr);
	ERR;
   }

   MPI_Finalize();

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
