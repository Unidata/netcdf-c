#include "netcdf.h"

int
main() {			/* create example.nc, uses old netCDF-2 interface */

   int  ncid;			/* netCDF id */

   /* dimension ids */
   int  lat_dim, lon_dim, frtime_dim, timelen_dim;

   /* variable ids */
   int  P_id, lat_id, lon_id, frtime_id, reftime_id, scalarv_id;

   /* variable shapes */
   int dims[3];

   /* containers for scalar attributes */
   float  float_val;
   double  double_val;

   /* attribute vectors */
   float  P_valid_range[2];

   /* enter define mode */
   ncid = nccreate("example.nc", NC_CLOBBER);

   /* define dimensions */
   lat_dim = ncdimdef(ncid, "lat", 4L);
   lon_dim = ncdimdef(ncid, "lon", 3L);
   frtime_dim = ncdimdef(ncid, "frtime", NC_UNLIMITED);
   timelen_dim = ncdimdef(ncid, "timelen", 20L);

   /* define variables and assign attributes */

   dims[0] = frtime_dim;
   dims[1] = lat_dim;
   dims[2] = lon_dim;
   P_id = ncvardef (ncid, "P", NC_FLOAT, 3, dims);
   ncattput (ncid, P_id, "long_name", NC_CHAR, 24,
	     (void *)"pressure at maximum wind");
   ncattput (ncid, P_id, "units", NC_CHAR, 12,
	     (void *)"hectopascals");
   P_valid_range[0] = 0;
   P_valid_range[1] = 1500;
   ncattput (ncid, P_id, "valid_range", NC_FLOAT, 2,
	     (void *) P_valid_range);
   float_val = -9999;
   ncattput (ncid, P_id, "_FillValue", NC_FLOAT, 1,
	     (void *) &float_val);

   dims[0] = lat_dim;
   lat_id = ncvardef (ncid, "lat", NC_FLOAT, 1, dims);
   ncattput (ncid, lat_id, "long_name", NC_CHAR, 8,
	     (void *)"latitude");
   ncattput (ncid, lat_id, "units", NC_CHAR, 13,
	     (void *)"degrees_north");

   dims[0] = lon_dim;
   lon_id = ncvardef (ncid, "lon", NC_FLOAT, 1, dims);
   ncattput (ncid, lon_id, "long_name", NC_CHAR, 9,
	     (void *)"longitude");
   ncattput (ncid, lon_id, "units", NC_CHAR, 12,
	     (void *)"degrees_east");

   dims[0] = frtime_dim;
   frtime_id = ncvardef (ncid, "frtime", NC_INT, 1, dims);
   ncattput (ncid, frtime_id, "long_name", NC_CHAR, 13,
	     (void *)"forecast time");
   ncattput (ncid, frtime_id, "units", NC_CHAR, 5,
	     (void *)"hours");

   dims[0] = timelen_dim;
   reftime_id = ncvardef (ncid, "reftime", NC_CHAR, 1, dims);
   ncattput (ncid, reftime_id, "long_name", NC_CHAR, 14,
	     (void *)"reference time");
   ncattput (ncid, reftime_id, "units", NC_CHAR, 9,
	     (void *)"text_time");

   scalarv_id = ncvardef (ncid, "scalarv", NC_INT, 0, 0);
   double_val = 1;
   ncattput (ncid, scalarv_id, "scalar_att", NC_DOUBLE, 1,
	     (void *) &double_val);

   /* Global attributes */
   ncattput (ncid, NC_GLOBAL, "history", NC_CHAR, 41,
	     (void *)"created by Unidata LDM from NPS broadcast");
   ncattput (ncid, NC_GLOBAL, "title", NC_CHAR, 48,
	     (void *)"NMC Global Product Set: Pressure at Maximum Wind");

   /* leave define mode */
   ncendef (ncid);
  
   {			/* store lat */
    static long lat_start[] = {0};
    static long lat_edges[] = {4};
    static float lat[] = {-90, -87.5, -85, -82.5};
    ncvarput(ncid, lat_id, lat_start, lat_edges, (void *)lat);
   }

   {			/* store lon */
    static long lon_start[] = {0};
    static long lon_edges[] = {3};
    static float lon[] = {-180, -175, -170};
    ncvarput(ncid, lon_id, lon_start, lon_edges, (void *)lon);
   }

   {			/* store frtime */
    static long frtime_start[] = {0};
    static long frtime_edges[] = {1};
    static long frtime[] = {12};
    ncvarput(ncid, frtime_id, frtime_start, frtime_edges,
	     (void *)frtime);
   }

   {			/* store frtime */
    static long frtime_start[] = {1};
    static long frtime_edges[] = {1};
    static long frtime[] = {18};
    ncvarput(ncid, frtime_id, frtime_start, frtime_edges,
	     (void *)frtime);
   }

   {			/* store reftime */
    static long reftime_start[] = {0};
    static long reftime_edges[] = {20};
    static char reftime[] = {"1992 03 04 12:00"};
    ncvarput(ncid, reftime_id, reftime_start, reftime_edges,
	     (void *)reftime);
   }

   {			/* store P */
    static long P_start[] = {0, 0, 0};
    static long P_edges[] = {2, 4, 3};
    static float P[2][4][3] = {
	{{950, 951, 952}, {953, 954, 955}, {956, 957, 958}, {959, 960, 961}},
	{{962, 963, 964}, {965, 966, 967}, {968, 969, 970}, {971, 972, 973}}
      };
    ncvarput(ncid, P_id, P_start, P_edges, (void *)&P[0][0][0]);
   }

   {			/* store scalarv */
    static long scalarv = {-2147483647};
    ncvarput1(ncid, scalarv_id, (long *)0, (void *)&scalarv);
   }
   ncclose (ncid);
   return 0;
}
