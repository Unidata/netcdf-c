/* This is part of the netCDF package.
   Copyright 2008 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test for bugs found testing fill values.

   $Id: tst_fillbug.c,v 1.2 2008/10/03 16:03:24 ed Exp $
*/

#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_fillbug.nc"
#define RANK_Time 1
#define RANK_P 1
#define LEN 1

int
main() 
{
   int ncid, dimids[RANK_P], varid;
   int ndims, dimids_in[RANK_P];

   double dummy = 42.0;
   size_t start[1] = {0}, count[1] = {1};
   static float P_data[LEN];
   int i;

   printf("\n*** Testing for another netCDF-4 fill-value bug.\n");
   printf("*** Testing fill-values...");

   if (nc_create(FILENAME, NC_NETCDF4, &ncid)) ERR;
   if (nc_def_dim(ncid, "dim_name", 1, &dimids[0])) ERR;
   if (nc_def_var(ncid, "var_name", NC_FLOAT, 1, dimids, &varid)) ERR;
   if (nc_close(ncid)) ERR;
   if (nc_open(FILENAME, NC_WRITE, &ncid)) ERR;
   if (nc_put_att_double (ncid, varid, "_FillValue", NC_FLOAT, 1, &dummy)) ERR;
   if (nc_close(ncid)) ERR;
   SUMMARIZE_ERR;
   
   FINAL_RESULTS;
   return 0;
}
