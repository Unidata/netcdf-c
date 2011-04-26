/*
  Copyright 2011, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This is part of netCDF.
   
  This program tests for a bug discovered with nofill mode that failed
  only on file systems with block size in a particular range.  It fails
  when invoked with the blksize argument between 2091953 and 2150032,
  inclusive, and succeeds for other blksizes.
*/

#include <config.h>
#include <nc_tests.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <netcdf.h>

#define FILE_NAME "tst_nofill2.nc"
#define LON_LEN 240
#define LAT_LEN 121
#define LVL_LEN 31
#define TIME_LEN 1

int
create_file(char *file_name, int fill_mode, size_t* sizehintp) 
{
   int ncid;
   int lon_dim, lat_dim, lvl_dim, time_dim;
   int time_id, zonal_wnd_id;
   int i;

   /* rank (number of dimensions) for each variable */
#  define RANK_time 1
#  define RANK_lat 1
#  define RANK_lon 1
#  define RANK_lvl 1
#  define RANK_zonal_wnd 4

   /* variable shapes */
   int zonal_wnd_dims[RANK_zonal_wnd];
   size_t zonal_wnd_start[RANK_zonal_wnd];
   size_t zonal_wnd_count[RANK_zonal_wnd];
   float zonal_wnd[LON_LEN*LAT_LEN*TIME_LEN];
   size_t default_initialsize = 0;
   double time[TIME_LEN] = {1.};

   /* To test bug on filesystem without large block size, we can get
    * the same effect by providing the desired value as sizehint to
    * nc__create() instead of calling nc_create() and getting the
    * block size reported by fstat */
   if (nc__create(file_name, NC_CLOBBER, default_initialsize, sizehintp, &ncid)) ERR;
   if (nc_set_fill(ncid, fill_mode, NULL)) ERR;

   /* define dimensions */
   if (nc_def_dim(ncid, "lon", LON_LEN, &lon_dim)) ERR;
   if (nc_def_dim(ncid, "lat", LAT_LEN, &lat_dim)) ERR;
   if (nc_def_dim(ncid, "lvl", LVL_LEN, &lvl_dim)) ERR;
   if (nc_def_dim(ncid, "time", TIME_LEN, &time_dim)) ERR;

   /* define variables */
   if (nc_def_var(ncid, "time", NC_DOUBLE, RANK_time, &time_dim, &time_id)) ERR;

   zonal_wnd_dims[0] = time_dim;
   zonal_wnd_dims[1] = lvl_dim;
   zonal_wnd_dims[2] = lat_dim;
   zonal_wnd_dims[3] = lon_dim;
   if (nc_def_var(ncid, "zonal_wnd", NC_FLOAT, RANK_zonal_wnd, zonal_wnd_dims, &zonal_wnd_id)) ERR;

   if (nc_enddef (ncid)) ERR;
   if (nc_put_var_double(ncid, time_id, time)) ERR;
   /* Bug exposed when written in reverse order. */
   for(i = LVL_LEN - 1; i>=0; i--)
      /* for(i = 0; i < LVL_LEN; i++) */
   {
      int izw;
      for(izw = 0; izw < TIME_LEN * LAT_LEN * LON_LEN; izw++) {
	 zonal_wnd[izw] = 100 + i;
      }
      zonal_wnd_start[0] = 0;
      zonal_wnd_start[1] = i;
      zonal_wnd_start[2] = 0;
      zonal_wnd_start[3] = 0;
      zonal_wnd_count[0] = TIME_LEN;
      zonal_wnd_count[1] = 1;
      zonal_wnd_count[2] = LAT_LEN;
      zonal_wnd_count[3] = LON_LEN;
      if (nc_put_vara_float(ncid, zonal_wnd_id, zonal_wnd_start, zonal_wnd_count, zonal_wnd)) ERR;
   }
   if (nc_close(ncid)) ERR;
   return 0;
}

int
main(int argc, char **argv)
{
   size_t sizehint = (1750000);	/* default if not set on command line,
				 * exposes bug.  It turns out any
				 * value between 2091953 and 2150032
				 * triggers bug, whereas all other
				 * values work fine. */

   printf("\n*** Testing nofill mode.\n");
   printf("*** Create file in nofill mode, writing all values...");
   {
#define NUM_TRIES 4000
      int ncid;
      size_t idx[1] = {0};
      double data;
      int i;

      for (i = 0; i < NUM_TRIES; i++)
      {
	 printf("...trying sizehint of %ld\n", sizehint);
	 if (create_file(FILE_NAME, NC_NOFILL, &sizehint)) ERR;
	 if (nc_open(FILE_NAME, 0, &ncid)) ERR;
	 if (nc_get_var1(ncid, 0, idx, &data)) ERR;
	 if (!data) ERR;
	 if (nc_close(ncid)) ERR;
	 sizehint += 10000;
      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
