/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/

/*
 * Test more renames of vars and dims.
 *
 * Ed Hartnett
 */

#include "nc_tests.h"
#include "err_macros.h"

#define TEST_NAME "tst_rename2"
#define LAT "lat"
#define LON "lon"
#define LEV "lev"
#define DIM_X "x"
#define DIM_Y "y"
#define DIM_Z "z"
#define VAR_NAME_START "lon"
#define VAR_NAME_END "lo"
#define DIM_NAME_START "lon"
#define DIM_NAME_END "lo"

#define DIM1_LEN 4
#define NDIM1 1
#define NDIM3 3
#define NUM_ENDDEF_SETTINGS 2
#define D1_NAME "d1"
#define D2_NAME "d2"
#define TMP_NAME "t1"

int
main(int argc, char **argv)
{
#define NUM_FORMATS 2
   int formats[NUM_FORMATS] = {NC_FORMAT_NETCDF4, NC_FORMAT_NETCDF4_CLASSIC};
   int format;

   fprintf(stderr,"*** Testing more renames\n");

   for (format = 0; format < NUM_FORMATS; format++)
   {
      fprintf(stderr,"*** test renaming 3 dimensions with format %d...",
              formats[format]);
      {
         char filename[NC_MAX_NAME + 1];
         int ncid, dimid[NDIM3];
         int dimid_in;
         int enddef_setting;

         if (nc_set_default_format(formats[format], NULL)) ERR;

         for (enddef_setting = 0; enddef_setting < NUM_ENDDEF_SETTINGS;
              enddef_setting++)
         {
            sprintf(filename, "%s_%d_%d.nc", TEST_NAME, formats[format],
                    enddef_setting);

            /* Create file with three dims. */
            if (nc_create(filename, 0, &ncid)) ERR;
            if (nc_def_dim(ncid, LAT, DIM1_LEN, &dimid[0])) ERR;
            if (nc_def_dim(ncid, LON, DIM1_LEN, &dimid[1])) ERR;
            if (nc_def_dim(ncid, LEV, DIM1_LEN, &dimid[2])) ERR;

            if (enddef_setting)
            {
               if (nc_enddef(ncid)) ERR;
               if (nc_redef(ncid)) ERR;
            }

            /* Rename the dimensions. */
            if (nc_rename_dim(ncid, 0, DIM_X)) ERR;
            if (nc_rename_dim(ncid, 1, DIM_Y)) ERR;
            if (nc_rename_dim(ncid, 2, DIM_Z)) ERR;

            /* Close the file. */
            if (nc_close(ncid)) ERR;

            /* Reopen the file and check. */
            if (nc_open(filename, NC_NOWRITE, &ncid)) ERR;
            if (nc_inq_dimid(ncid, DIM_X, &dimid_in)) ERR;
            if (dimid_in != 0) ERR;
            if (nc_inq_dimid(ncid, DIM_Y, &dimid_in)) ERR;
            if (dimid_in != 1) ERR;
            if (nc_inq_dimid(ncid, DIM_Z, &dimid_in)) ERR;
            if (dimid_in != 2) ERR;
            if (nc_close(ncid)) ERR;
         } /* next enddef setting */
      }
      SUMMARIZE_ERR;

      fprintf(stderr,"*** test renaming 3 dims with coord data format %d...",
              formats[format]);
      {
         char filename[NC_MAX_NAME + 1];
         int ncid, dimid[NDIM3], varid[NDIM3];
         int dimid_in;
         int lat_data[DIM1_LEN] = {0, 1, 2, 3};
         int lon_data[DIM1_LEN] = {0, 10, 20, 30};
         int lev_data[DIM1_LEN] = {0, 100, 200, 300};

         if (nc_set_default_format(formats[format], NULL)) ERR;

         sprintf(filename, "%s_data_%d.nc", TEST_NAME, formats[format]);

         /* Create file with three dims. */
         if (nc_create(filename, 0, &ncid)) ERR;
         if (nc_def_dim(ncid, LAT, DIM1_LEN, &dimid[0])) ERR;
         if (nc_def_dim(ncid, LON, DIM1_LEN, &dimid[1])) ERR;
         if (nc_def_dim(ncid, LEV, DIM1_LEN, &dimid[2])) ERR;

         /* Define coordinate data vars. */
         if (nc_def_var(ncid, LAT, NC_INT, NDIM1, &dimid[0], &varid[0])) ERR;
         if (nc_def_var(ncid, LON, NC_INT, NDIM1, &dimid[1], &varid[1])) ERR;
         if (nc_def_var(ncid, LEV, NC_INT, NDIM1, &dimid[2], &varid[2])) ERR;

         if (nc_enddef(ncid)) ERR;

         if (nc_put_var(ncid, 0, lat_data)) ERR;
         if (nc_put_var(ncid, 1, lon_data)) ERR;
         if (nc_put_var(ncid, 2, lev_data)) ERR;

         if (nc_close(ncid)) ERR;
         if (nc_open(filename, NC_WRITE, &ncid)) ERR;
         if (nc_redef(ncid)) ERR;

         /* Rename the dimensions. */
         if (nc_rename_dim(ncid, 0, DIM_X)) ERR;
         if (nc_rename_dim(ncid, 1, DIM_Y)) ERR;
         if (nc_rename_dim(ncid, 2, DIM_Z)) ERR;

         /* Close the file. */
         if (nc_close(ncid)) ERR;

         /* Reopen the file and check. */
         if (nc_open(filename, NC_NOWRITE, &ncid)) ERR;
         if (nc_inq_dimid(ncid, DIM_X, &dimid_in)) ERR;
         if (dimid_in != 0) ERR;
         if (nc_inq_dimid(ncid, DIM_Y, &dimid_in)) ERR;
         if (dimid_in != 1) ERR;
         if (nc_inq_dimid(ncid, DIM_Z, &dimid_in)) ERR;
         if (dimid_in != 2) ERR;
         if (nc_close(ncid)) ERR;
      }
      SUMMARIZE_ERR;

   } /* next format */

#define FILE_NAME1 "tst_dims_foo1.nc"
#define DIM_NAME "lat_T42"
#define VAR_NAME DIM_NAME
#define DIM_NAME2 "lat"
#define VAR_NAME2 DIM_NAME2
#define RANK_lat_T42 1
   fprintf(stderr,"*** test renaming with sync...");
   {
      int ncid, dimid, varid;
      char file_name[NC_MAX_NAME + 1];
      char name[NC_MAX_NAME + 1];

      /* Create file with dim and associated coordinate var. */
      sprintf(file_name, "%s_sync.nc", TEST_NAME);
      if (nc_create(file_name, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME_END, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, DIM_NAME_END, NC_INT, NDIM1, &dimid, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      if (nc_create(file_name, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME_START, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, DIM_NAME_END, NC_INT, NDIM1, &dimid, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_rename_dim(ncid, dimid, DIM_NAME_END)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen file and check, */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, DIM_NAME_END, &dimid)) ERR;
      if (nc_inq_varid(ncid, DIM_NAME_END, &varid)) ERR;
      if (nc_inq_dimname(ncid, dimid, name)) ERR;
      if (strcmp(name, DIM_NAME_END)) ERR;
      if (nc_inq_varname(ncid, varid, name)) ERR;
      if (strcmp(name, DIM_NAME_END)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   fprintf(stderr,"*** test renaming with sync...");
   {
      int ncid, dimid, varid;
      char file_name[NC_MAX_NAME + 1];
      char name[NC_MAX_NAME + 1];

      /* Create file with dim and associated coordinate var. */
      sprintf(file_name, "%s_sync.nc", TEST_NAME);
      if (nc_create(file_name, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME_START, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME_START, NC_INT, NDIM1, &dimid, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* nc_set_log_level(4); */
      /* Open the file and rename the var. */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, DIM_NAME_START, &dimid)) ERR;
      if (nc_inq_varid(ncid, VAR_NAME_START, &varid)) ERR;
      if (nc_rename_var(ncid, varid, VAR_NAME_END)) ERR;

      /* Sync to disk. Now the file has one dim and one var. The dim
       * is a dimscale only dataset, and the var is a dataset with a
       * dimscale attached pointing to the dim. */
      /* if (nc_sync(ncid)) ERR; */
      if (nc_close(ncid)) ERR;
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      /* Now rename the dim to the same name as the var. After this
       * there will be one dataset, called DIM_NAME_END, which will be
       * a dimscale. */
      if (nc_rename_dim(ncid, dimid, DIM_NAME_END)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen file and check, */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, DIM_NAME_END, &dimid)) ERR;
      if (nc_inq_varid(ncid, VAR_NAME_END, &varid)) ERR;
      if (nc_inq_dimname(ncid, dimid, name)) ERR;
      if (strcmp(name, DIM_NAME_END)) ERR;
      if (nc_inq_varname(ncid, varid, name)) ERR;
      if (strcmp(name, VAR_NAME_END)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   fprintf(stderr,"*** test renaming non-coord var to same name as dim...");
   {
      int ncid, dimid1, dimid2, varid1, varid2;
      int dimid_in, varid_in;
      char file_name[NC_MAX_NAME + 1];

      /* Create file with dim and associated coordinate var. */
      sprintf(file_name, "%s_non_coord_to_dim.nc", TEST_NAME);
      if (nc_create(file_name, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, D1_NAME, DIM1_LEN, &dimid1)) ERR;
      if (nc_def_dim(ncid, D2_NAME, DIM1_LEN, &dimid2)) ERR;
      if (nc_def_var(ncid, D1_NAME, NC_INT, NDIM1, &dimid1, &varid1)) ERR;
      if (nc_def_var(ncid, D2_NAME, NC_INT, NDIM1, &dimid2, &varid2)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and rename the vars. */
      nc_set_log_level(4);
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_rename_var(ncid, varid1, TMP_NAME)) ERR;
      if (nc_rename_var(ncid, varid2, D1_NAME)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen file and check, */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, D1_NAME, &dimid_in)) ERR;
      if (dimid_in != dimid1) ERR;
      if (nc_inq_dimid(ncid, D2_NAME, &dimid_in)) ERR;
      if (dimid_in != dimid2) ERR;
      if (nc_inq_dimid(ncid, TMP_NAME, &dimid_in) != NC_EBADDIM) ERR;
      if (nc_inq_varid(ncid, TMP_NAME, &varid_in)) ERR;
      if (varid_in != varid1) ERR;
      if (nc_inq_varid(ncid, D1_NAME, &varid_in)) ERR;
      if (varid_in != varid2) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   fprintf(stderr,"*** test renaming affect on varids...");
   {
      int ncid, varid1, varid2;
      int varid_in;
      char file_name[NC_MAX_NAME + 1];

      /* Create file with two scalar vars. */
      sprintf(file_name, "%s_rename_affect_varid_order.nc", TEST_NAME);
      if (nc_create(file_name, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_var(ncid, D1_NAME, NC_INT, 0, NULL, &varid1)) ERR;
      if (nc_def_var(ncid, D2_NAME, NC_INT, 0, NULL, &varid2)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and rename a var. */
      nc_set_log_level(4);
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_rename_var(ncid, varid1, TMP_NAME)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen file and check, */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_inq_varid(ncid, TMP_NAME, &varid_in)) ERR;
      if (varid_in != varid1) ERR;
      if (nc_inq_varid(ncid, D1_NAME, &varid_in) != NC_ENOTVAR) ERR;
      if (nc_inq_varid(ncid, D2_NAME, &varid_in)) ERR;
      if (varid_in != varid2) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
