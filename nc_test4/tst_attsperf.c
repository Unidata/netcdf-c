/* This is part of the netCDF package. Copyright 2018 University
 * Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
 * for conditions of use.
 *
 * Test the netCDF-4 attribute code.
 *
 * WARNING: do not attempt to run this under windows because of the use
 * of gettimeofday().
 *
 * Ed Hartnett 6/19/18
*/

#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "nc4internal.h"
#include <sys/time.h>

#define TEST "tst_attsperf"
#define VAR "bigvar"
#define NDIMS 2
#define DIM0 "d0"
#define DIM1 "d1"
#define DIMSIZE0 16
#define DIMSIZE1 512
#define TOTALSIZE (DIMSIZE0 * DIMSIZE1)
#define NUM_ATTS 100
#define ATT_LEN 10
#define NUM_VARS 100

int
add_attributes(int ncid, int varid)
{
   char att_name[NC_MAX_NAME + 1];
   double att_data[ATT_LEN];
   int i, a;

   /* Fill up data. */
   for (i = 0; i < ATT_LEN; i++)
      att_data[i] = i;

   /* Write a bunch of attributes. */
   for (a = 0; a < NUM_ATTS; a++)
   {
      sprintf(att_name, "%s_varid_%d_att_%d", TEST, varid, a);
      if (nc_put_att_double(ncid, varid, att_name, NC_DOUBLE,
                            ATT_LEN, att_data)) ERR;
   }

   return 0;
}

int
buildfile(int file_no)
{
   int ncid, varid;
   int dimids[NDIMS];
   char file_name[NC_MAX_NAME + 1];
   int v;

   sprintf(file_name, "%s_%d.nc", TEST, file_no);

   if (nc_create(file_name, NC_NETCDF4, &ncid)) ERR;

   if (nc_def_dim(ncid, DIM0, DIMSIZE0, &dimids[0])) ERR;
   if (nc_def_dim(ncid, DIM1, DIMSIZE1, &dimids[1])) ERR;
   for (v = 0; v < NUM_VARS; v++)
   {
      char var_name[NC_MAX_NAME + 1];
      sprintf(var_name, "%s_var_%d", TEST, v);
      if (nc_def_var(ncid, var_name, NC_INT, NDIMS, dimids, &varid)) ERR;
      if (add_attributes(ncid, v)) ERR;
   }
   if (add_attributes(ncid, NC_GLOBAL)) ERR;
   if (nc_enddef(ncid)) ERR;

   if (nc_close(ncid)) ERR;
   return 0;
}

long long
readfile(int inq_all)
{
   int ncid;
   struct timeval starttime, endtime;
   long long delta;
   long long startt, endt;
   char file_name[NC_MAX_NAME + 1];

   sprintf(file_name, "%s_%d.nc", TEST, inq_all);

   /* Start the clock. */
   gettimeofday(&starttime, NULL);

   /* Open the file. */
   if (nc_open(file_name, NC_NETCDF4, &ncid)) ERR;

   /* Simulate old open by triggering attribute reads, if desired. */
   if (inq_all)
   {
      int natts;
      int v;

      /* When checking the number of atts, we trigger the read. */
      if (nc_inq(ncid, NULL, NULL, &natts, NULL)) ERR;
      for (v = 0; v < NUM_VARS; v++)
         if (nc_inq_varnatts(ncid, v, &natts)) ERR;
   }
   gettimeofday(&endtime, NULL);

   /* Close the file. */
   if (nc_close(ncid)) ERR;

   /* Compute the time delta */
   startt = (1000000 * starttime.tv_sec) + starttime.tv_usec;
   endt = (1000000 * endtime.tv_sec) + endtime.tv_usec;
   delta = endt - startt;
   return delta;
}

int
main(int argc, char **argv)
{
   long long zerodelta, onedelta, factor;

   printf("testing speed of open with files with lots of metadata...\n");
   if (buildfile(0)) ERR;
   if (buildfile(1)) ERR;
   if ((zerodelta = readfile(0)) == -1) ERR;
   if ((onedelta = readfile(1)) == -1) ERR;

   /* Print results to the millisec */
   factor = onedelta / zerodelta;
   printf("Lazy Atts time=%lld Read Atts at Open time=%lld Speedup=%lld\n",
          zerodelta, onedelta, factor);
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
