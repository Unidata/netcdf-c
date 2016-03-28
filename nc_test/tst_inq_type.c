/* This is part of the netCDF package. Copyright 2016 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test nc_inq_type

   Added in support of https://github.com/Unidata/netcdf/issues/240

*/

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <nc_tests.h>
#include <netcdf.h>

#ifdef USE_PNETCDF
#include <netcdf_par.h>
#endif

#define FILE_NAME "tst_inq_type.nc"

void
check_err(const int stat, const int line, const char *file) {
   if (stat != NC_NOERR) {
      (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
      fflush(stderr);
      exit(1);
   }
}


int main(int argc, char **argv) {

  int ncid=0;

  printf("\n* Testing nc_inq_type\n");

  if(nc_create(FILE_NAME,NC_WRITE,&ncid)) ERR;

  printf("\t* Testing NC_INT:\t");
  if(nc_inq_type(0,NC_INT,NULL,NULL))ERR;
  else printf("success.\n");

  printf("\t* Testing NC_INT64:\t");
  if(nc_inq_type(0,NC_INT64,NULL,NULL))ERR;
  else printf("success.\n");

  printf("* Finished.\n");
  return 0;
}
