/* This is part of the netCDF package.  Copyright 2005-2011,
   University Corporation for Atmospheric Research/Unidata. See
   COPYRIGHT file for conditions of use.

   Test that NetCDF-4 can read HDF4 files.
   Ed Hartnett
*/
#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#include <mfhdf.h>
#include <netcdf_f.h>

#define FILE_NAME "tst_hdf4_extra.h4"

#define PRES_NAME "pres"
#define LAT_LEN 3
#define LON_LEN 2
#define DIMS_2 2

int
create_hdf4_file()
{
   int32 sd_id, sds_id;
   int32 dim_size[DIMS_2] = {LAT_LEN, LON_LEN};
   int32 start[DIMS_2] = {0, 0}, edge[DIMS_2] = {LAT_LEN, LON_LEN};
   int data_out[LAT_LEN][LON_LEN];
   int i, j;
   
   /* Create some data. */
   for (i = 0; i < LAT_LEN; i++)
      for (j = 0; j < LON_LEN; j++)
         data_out[i][j] = j;
   
   /* Create a file with one SDS, containing our phony data. */
   sd_id = SDstart(FILE_NAME, DFACC_CREATE);
   sds_id = SDcreate(sd_id, PRES_NAME, DFNT_INT32, DIMS_2, dim_size);
   if (SDwritedata(sds_id, start, NULL, edge, (void *)data_out)) ERR;
   if (SDendaccess(sds_id)) ERR;
   if (SDend(sd_id)) ERR;

   return 0;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF4/NetCDF-4 interoperability extra stuff...\n");

   /* Create our test file. */
   if (create_hdf4_file()) ERR;
   
   printf("*** testing abort...");
   {
      int ncid;

      /* These will not work. */
      if (nc_open(FILE_NAME, NC_MMAP, &ncid) != NC_EINVAL) ERR;
      if (nc_open(FILE_NAME, NC_64BIT_OFFSET, &ncid) != NC_EINVAL) ERR;
      if (nc_open(FILE_NAME, NC_MPIIO, &ncid) != NC_EINVAL) ERR;
      if (nc_open(FILE_NAME, NC_MPIPOSIX, &ncid) != NC_EINVAL) ERR;
      if (nc_open(FILE_NAME, NC_DISKLESS, &ncid) != NC_EINVAL) ERR;

      /* Now open with netCDF. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;

      /* Attempt to write. */
      if (nc_def_var(ncid, "hh", NC_INT, 0, NULL, NULL) != NC_EPERM) ERR;

      /* Abort!! */
      if (nc_abort(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
