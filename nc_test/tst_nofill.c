/*
  Copyright 2007, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This is part of netCDF.
   
  This program tests for a bug discovered with nofill mode that failed
  only on file systems with large block size.

  $Id: tst_misc.c,v 1.6 2010/05/05 22:15:36 dmh Exp $
*/

#include <config.h>
#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILE_NAME1 "tst_fill1.nc"
#define FILE_NAME2 "tst_fill2.nc"

void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
	(void) fprintf(stderr, "line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
	exit(1);
    }
}

int
create_file(char *file_name, int fill_mode) 
{
   int i;
   int  stat;			/* return status */
   int  ncid;			/* netCDF id */

   /* dimension ids */
   int lon_dim;
   int lat_dim;
   int lvl_dim;
   int time_dim;
   int char_size_dim;

   /* dimension lengths */
   size_t lon_len = 240;
   size_t lat_len = 121;
   size_t lvl_len = 31;
   size_t time_len = NC_UNLIMITED;
   size_t char_size_len = 4;

   /* variable ids */
   int time_id;
   int seg_type_id;
   int lat_id;
   int lon_id;
   int lvl_id;
   int base_date_id;
   int base_time_id;
   int valid_date_id;
   int valid_time_id;
   int forc_hrs_id;
   int wrtn_date_id;
   int wrtn_time_id;
   int sfc_pres_id;
   int temp_scrn_id;
   int qsair_scrn_id;
   int topog_id;
   int mslp_id;
   int sfc_temp_id;
   int zonal_wnd_id;

   /* rank (number of dimensions) for each variable */
#  define RANK_time 1
#  define RANK_seg_type 2
#  define RANK_lat 1
#  define RANK_lon 1
#  define RANK_lvl 1
#  define RANK_base_date 1
#  define RANK_base_time 1
#  define RANK_valid_date 1
#  define RANK_valid_time 1
#  define RANK_forc_hrs 1
#  define RANK_wrtn_date 1
#  define RANK_wrtn_time 1
#  define RANK_sfc_pres 3
#  define RANK_temp_scrn 3
#  define RANK_qsair_scrn 3
#  define RANK_topog 3
#  define RANK_mslp 3
#  define RANK_sfc_temp 3
#  define RANK_zonal_wnd 4

   /* variable shapes */
   int time_dims[RANK_time];
   int seg_type_dims[RANK_seg_type];
   int lat_dims[RANK_lat];
   int lon_dims[RANK_lon];
   int lvl_dims[RANK_lvl];
   int base_date_dims[RANK_base_date];
   int base_time_dims[RANK_base_time];
   int valid_date_dims[RANK_valid_date];
   int valid_time_dims[RANK_valid_time];
   int forc_hrs_dims[RANK_forc_hrs];
   int wrtn_date_dims[RANK_wrtn_date];
   int wrtn_time_dims[RANK_wrtn_time];
   int sfc_pres_dims[RANK_sfc_pres];
   int temp_scrn_dims[RANK_temp_scrn];
   int qsair_scrn_dims[RANK_qsair_scrn];
   int topog_dims[RANK_topog];
   int mslp_dims[RANK_mslp];
   int sfc_temp_dims[RANK_sfc_temp];
   int zonal_wnd_dims[RANK_zonal_wnd];

   /* attribute vectors */
   float lat_valid_min[1];
   float lat_valid_max[1];
   float lon_valid_min[1];
   float lon_valid_max[1];
   int sfc_pres_stash_code[1];
   float sfc_pres_missing_value[1];
   int temp_scrn_stash_code[1];
   float temp_scrn_missing_value[1];
   int qsair_scrn_stash_code[1];
   float qsair_scrn_missing_value[1];
   int topog_stash_code[1];
   float topog_missing_value[1];
   int mslp_stash_code[1];
   float mslp_missing_value[1];
   int sfc_temp_stash_code[1];
   float sfc_temp_missing_value[1];
   int zonal_wnd_stash_code[1];
   float zonal_wnd_missing_value[1];

   int old_fill_mode;

   /* enter define mode */
   stat = nc_create(file_name, NC_CLOBBER, &ncid);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_set_fill(ncid, fill_mode, &old_fill_mode);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "lon", lon_len, &lon_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "lat", lat_len, &lat_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "lvl", lvl_len, &lvl_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "time", time_len, &time_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "char_size", char_size_len, &char_size_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   time_dims[0] = time_dim;
   stat = nc_def_var(ncid, "time", NC_DOUBLE, RANK_time, time_dims, &time_id);
   check_err(stat,__LINE__,__FILE__);

   seg_type_dims[0] = time_dim;
   seg_type_dims[1] = char_size_dim;
   stat = nc_def_var(ncid, "seg_type", NC_CHAR, RANK_seg_type, seg_type_dims, &seg_type_id);
   check_err(stat,__LINE__,__FILE__);

   lat_dims[0] = lat_dim;
   stat = nc_def_var(ncid, "lat", NC_FLOAT, RANK_lat, lat_dims, &lat_id);
   check_err(stat,__LINE__,__FILE__);

   lon_dims[0] = lon_dim;
   stat = nc_def_var(ncid, "lon", NC_FLOAT, RANK_lon, lon_dims, &lon_id);
   check_err(stat,__LINE__,__FILE__);

   lvl_dims[0] = lvl_dim;
   stat = nc_def_var(ncid, "lvl", NC_FLOAT, RANK_lvl, lvl_dims, &lvl_id);
   check_err(stat,__LINE__,__FILE__);

   base_date_dims[0] = time_dim;
   stat = nc_def_var(ncid, "base_date", NC_INT, RANK_base_date, base_date_dims, &base_date_id);
   check_err(stat,__LINE__,__FILE__);

   base_time_dims[0] = time_dim;
   stat = nc_def_var(ncid, "base_time", NC_INT, RANK_base_time, base_time_dims, &base_time_id);
   check_err(stat,__LINE__,__FILE__);

   valid_date_dims[0] = time_dim;
   stat = nc_def_var(ncid, "valid_date", NC_INT, RANK_valid_date, valid_date_dims, &valid_date_id);
   check_err(stat,__LINE__,__FILE__);

   valid_time_dims[0] = time_dim;
   stat = nc_def_var(ncid, "valid_time", NC_INT, RANK_valid_time, valid_time_dims, &valid_time_id);
   check_err(stat,__LINE__,__FILE__);

   forc_hrs_dims[0] = time_dim;
   stat = nc_def_var(ncid, "forc_hrs", NC_FLOAT, RANK_forc_hrs, forc_hrs_dims, &forc_hrs_id);
   check_err(stat,__LINE__,__FILE__);

   wrtn_date_dims[0] = time_dim;
   stat = nc_def_var(ncid, "wrtn_date", NC_INT, RANK_wrtn_date, wrtn_date_dims, &wrtn_date_id);
   check_err(stat,__LINE__,__FILE__);

   wrtn_time_dims[0] = time_dim;
   stat = nc_def_var(ncid, "wrtn_time", NC_INT, RANK_wrtn_time, wrtn_time_dims, &wrtn_time_id);
   check_err(stat,__LINE__,__FILE__);

   sfc_pres_dims[0] = time_dim;
   sfc_pres_dims[1] = lat_dim;
   sfc_pres_dims[2] = lon_dim;
   stat = nc_def_var(ncid, "sfc_pres", NC_FLOAT, RANK_sfc_pres, sfc_pres_dims, &sfc_pres_id);
   check_err(stat,__LINE__,__FILE__);

   temp_scrn_dims[0] = time_dim;
   temp_scrn_dims[1] = lat_dim;
   temp_scrn_dims[2] = lon_dim;
   stat = nc_def_var(ncid, "temp_scrn", NC_FLOAT, RANK_temp_scrn, temp_scrn_dims, &temp_scrn_id);
   check_err(stat,__LINE__,__FILE__);

   qsair_scrn_dims[0] = time_dim;
   qsair_scrn_dims[1] = lat_dim;
   qsair_scrn_dims[2] = lon_dim;
   stat = nc_def_var(ncid, "qsair_scrn", NC_FLOAT, RANK_qsair_scrn, qsair_scrn_dims, &qsair_scrn_id);
   check_err(stat,__LINE__,__FILE__);

   topog_dims[0] = time_dim;
   topog_dims[1] = lat_dim;
   topog_dims[2] = lon_dim;
   stat = nc_def_var(ncid, "topog", NC_FLOAT, RANK_topog, topog_dims, &topog_id);
   check_err(stat,__LINE__,__FILE__);

   mslp_dims[0] = time_dim;
   mslp_dims[1] = lat_dim;
   mslp_dims[2] = lon_dim;
   stat = nc_def_var(ncid, "mslp", NC_FLOAT, RANK_mslp, mslp_dims, &mslp_id);
   check_err(stat,__LINE__,__FILE__);

   sfc_temp_dims[0] = time_dim;
   sfc_temp_dims[1] = lat_dim;
   sfc_temp_dims[2] = lon_dim;
   stat = nc_def_var(ncid, "sfc_temp", NC_FLOAT, RANK_sfc_temp, sfc_temp_dims, &sfc_temp_id);
   check_err(stat,__LINE__,__FILE__);

   zonal_wnd_dims[0] = time_dim;
   zonal_wnd_dims[1] = lvl_dim;
   zonal_wnd_dims[2] = lat_dim;
   zonal_wnd_dims[3] = lon_dim;
   stat = nc_def_var(ncid, "zonal_wnd", NC_FLOAT, RANK_zonal_wnd, zonal_wnd_dims, &zonal_wnd_id);
   check_err(stat,__LINE__,__FILE__);

   /* assign attributes */
   stat = nc_put_att_text(ncid, time_id, "units", 40, "days since 2010-07-07 12:0:0.0 0.00     ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, seg_type_id, "long_name", 27, "segment of bmrc header type");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lat_id, "long_name", 9, "latitudes");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lat_id, "type", 8, "uniform ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lat_id, "units", 9, "degrees_N");
   check_err(stat,__LINE__,__FILE__);
   lat_valid_min[0] = -90;
   stat = nc_put_att_float(ncid, lat_id, "valid_min", NC_FLOAT, 1, lat_valid_min);
   check_err(stat,__LINE__,__FILE__);
   lat_valid_max[0] = 90;
   stat = nc_put_att_float(ncid, lat_id, "valid_max", NC_FLOAT, 1, lat_valid_max);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lon_id, "long_name", 10, "longitudes");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lon_id, "type", 7, "uniform");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lon_id, "units", 9, "degrees_E");
   check_err(stat,__LINE__,__FILE__);
   lon_valid_min[0] = -180;
   stat = nc_put_att_float(ncid, lon_id, "valid_min", NC_FLOAT, 1, lon_valid_min);
   check_err(stat,__LINE__,__FILE__);
   lon_valid_max[0] = 360;
   stat = nc_put_att_float(ncid, lon_id, "valid_max", NC_FLOAT, 1, lon_valid_max);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lvl_id, "long_name", 15, "vertical levels");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lvl_id, "type", 8, "pressure");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lvl_id, "units", 3, "hPa");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, lvl_id, "positive", 4, "down");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, base_date_id, "long_name", 36, "base date (YYYYMMDD) of archive file");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, base_date_id, "units", 8, "yyyymmdd");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, base_time_id, "long_name", 32, "base time (HHMM) of archive file");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, base_time_id, "units", 8, "hhmm UTC");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, valid_date_id, "long_name", 37, "valid date (YYYYMMDD) of this segment");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, valid_date_id, "units", 8, "yyyymmdd");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, valid_time_id, "long_name", 33, "valid time (HHMM) of this segment");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, valid_time_id, "units", 8, "hhmm UTC");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, forc_hrs_id, "long_name", 30, "forecast hours of this segment");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, forc_hrs_id, "units", 5, "hours");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, wrtn_date_id, "long_name", 45, "date (YYYYMMDD) that this segment was written");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, wrtn_time_id, "long_name", 41, "time (HHMM) that this segment was written");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_pres_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_pres_id, "level_type", 8, "single  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_pres_id, "units", 2, "Pa");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_pres_id, "long_name", 16, "surface pressure");
   check_err(stat,__LINE__,__FILE__);
   sfc_pres_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, sfc_pres_id, "stash_code", NC_INT, 1, sfc_pres_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_pres_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   sfc_pres_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, sfc_pres_id, "missing_value", NC_FLOAT, 1, sfc_pres_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, temp_scrn_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, temp_scrn_id, "level_type", 8, "single  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, temp_scrn_id, "units", 1, "K");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, temp_scrn_id, "long_name", 24, "screen-level temperature");
   check_err(stat,__LINE__,__FILE__);
   temp_scrn_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, temp_scrn_id, "stash_code", NC_INT, 1, temp_scrn_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, temp_scrn_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   temp_scrn_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, temp_scrn_id, "missing_value", NC_FLOAT, 1, temp_scrn_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, qsair_scrn_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, qsair_scrn_id, "level_type", 8, "single  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, qsair_scrn_id, "units", 5, "kg/kg");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, qsair_scrn_id, "long_name", 10, "qsair_scrn");
   check_err(stat,__LINE__,__FILE__);
   qsair_scrn_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, qsair_scrn_id, "stash_code", NC_INT, 1, qsair_scrn_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, qsair_scrn_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   qsair_scrn_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, qsair_scrn_id, "missing_value", NC_FLOAT, 1, qsair_scrn_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, topog_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, topog_id, "level_type", 8, "single  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, topog_id, "units", 1, "m");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, topog_id, "long_name", 10, "topography");
   check_err(stat,__LINE__,__FILE__);
   topog_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, topog_id, "stash_code", NC_INT, 1, topog_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, topog_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   topog_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, topog_id, "missing_value", NC_FLOAT, 1, topog_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, mslp_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, mslp_id, "level_type", 8, "single  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, mslp_id, "units", 3, "hPa");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, mslp_id, "long_name", 23, "mean sea level pressure");
   check_err(stat,__LINE__,__FILE__);
   mslp_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, mslp_id, "stash_code", NC_INT, 1, mslp_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, mslp_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   mslp_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, mslp_id, "missing_value", NC_FLOAT, 1, mslp_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_temp_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_temp_id, "level_type", 8, "single  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_temp_id, "units", 1, "K");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_temp_id, "long_name", 19, "surface temperature");
   check_err(stat,__LINE__,__FILE__);
   sfc_temp_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, sfc_temp_id, "stash_code", NC_INT, 1, sfc_temp_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sfc_temp_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   sfc_temp_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, sfc_temp_id, "missing_value", NC_FLOAT, 1, sfc_temp_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, zonal_wnd_id, "grid_type", 7, "spatial");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, zonal_wnd_id, "level_type", 8, "multi   ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, zonal_wnd_id, "units", 3, "m/s");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, zonal_wnd_id, "long_name", 10, "zonal wind");
   check_err(stat,__LINE__,__FILE__);
   zonal_wnd_stash_code[0] = 0;
   stat = nc_put_att_int(ncid, zonal_wnd_id, "stash_code", NC_INT, 1, zonal_wnd_stash_code);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, zonal_wnd_id, "accum_type", 13, "instantaneous");
   check_err(stat,__LINE__,__FILE__);
   zonal_wnd_missing_value[0] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, zonal_wnd_id, "missing_value", NC_FLOAT, 1, zonal_wnd_missing_value);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "convention", 6, "COARDS");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "source", 32, "Australian Bureau of Meteorology");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "modl_vrsn", 8, "ACCESS-G");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "expt_id", 4, "0001");
   check_err(stat,__LINE__,__FILE__);

   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

    static size_t time_start[RANK_time];
    static size_t time_count[RANK_time];
    static double time[] = {0.};
    time_len = 1;			/* number of records of time data */
    time_start[0] = 0;
    time_count[0] = time_len;

    static size_t seg_type_start[RANK_seg_type];
    static size_t seg_type_count[RANK_seg_type];
    static char seg_type[] = {"an  "};
    time_len = 1;			/* number of records of seg_type data */
    seg_type_start[0] = 0;
    seg_type_start[1] = 0;
    seg_type_count[0] = time_len;
    seg_type_count[1] = char_size_len;


    static float lat[] = {90, 88.5, 87, 85.5, 84, 82.5, 81, 79.5, 78, 76.5, 75, 73.5, 72, 70.5, 69, 67.5, 66, 64.5, 63, 61.5, 60, 58.5, 57, 55.5, 54, 52.5, 51, 49.5, 48, 46.5, 45, 43.5, 42, 40.5, 39, 37.5, 36, 34.5, 33, 31.5, 30, 28.5, 27, 25.5, 24, 22.5, 21, 19.5, 18, 16.5, 15, 13.5, 12, 10.5, 9, 7.5, 6, 4.5, 3, 1.5, 0, -1.5, -3, -4.5, -6, -7.5, -9, -10.5, -12, -13.5, -15, -16.5, -18, -19.5, -21, -22.5, -24, -25.5, -27, -28.5, -30, -31.5, -33, -34.5, -36, -37.5, -39, -40.5, -42, -43.5, -45, -46.5, -48, -49.5, -51, -52.5, -54, -55.5, -57, -58.5, -60, -61.5, -63, -64.5, -66, -67.5, -69, -70.5, -72, -73.5, -75, -76.5, -78, -79.5, -81, -82.5, -84, -85.5, -87, -88.5, -90};

    static float lon[] = {0, 1.5, 3, 4.5, 6, 7.5, 9, 10.5, 12, 13.5, 15, 16.5, 18, 19.5, 21, 22.5, 24, 25.5, 27, 28.5, 30, 31.5, 33, 34.5, 36, 37.5, 39, 40.5, 42, 43.5, 45, 46.5, 48, 49.5, 51, 52.5, 54, 55.5, 57, 58.5, 60, 61.5, 63, 64.5, 66, 67.5, 69, 70.5, 72, 73.5, 75, 76.5, 78, 79.5, 81, 82.5, 84, 85.5, 87, 88.5, 90, 91.5, 93, 94.5, 96, 97.5, 99, 100.5, 102, 103.5, 105, 106.5, 108, 109.5, 111, 112.5, 114, 115.5, 117, 118.5, 120, 121.5, 123, 124.5, 126, 127.5, 129, 130.5, 132, 133.5, 135, 136.5, 138, 139.5, 141, 142.5, 144, 145.5, 147, 148.5, 150, 151.5, 153, 154.5, 156, 157.5, 159, 160.5, 162, 163.5, 165, 166.5, 168, 169.5, 171, 172.5, 174, 175.5, 177, 178.5, 180, 181.5, 183, 184.5, 186, 187.5, 189, 190.5, 192, 193.5, 195, 196.5, 198, 199.5, 201, 202.5, 204, 205.5, 207, 208.5, 210, 211.5, 213, 214.5, 216, 217.5, 219, 220.5, 222, 223.5, 225, 226.5, 228, 229.5, 231, 232.5, 234, 235.5, 237, 238.5, 240, 241.5, 243, 244.5, 246, 247.5, 249, 250.5, 252, 253.5, 255, 256.5, 258, 259.5, 261, 262.5, 264, 265.5, 267, 268.5, 270, 271.5, 273, 274.5, 276, 277.5, 279, 280.5, 282, 283.5, 285, 286.5, 288, 289.5, 291, 292.5, 294, 295.5, 297, 298.5, 300, 301.5, 303, 304.5, 306, 307.5, 309, 310.5, 312, 313.5, 315, 316.5, 318, 319.5, 321, 322.5, 324, 325.5, 327, 328.5, 330, 331.5, 333, 334.5, 336, 337.5, 339, 
340.5, 342, 343.5, 345, 346.5, 348, 349.5, 351, 352.5, 354, 355.5, 357, 358.5};

    static float lvl[] = {1000, 995, 990, 985, 975, 950, 925, 900, 875, 850, 800, 750, 700, 600, 500, 450, 400, 350, 300, 275, 250, 225, 200, 175, 150, 100, 70, 50, 30, 20, 10};

    static size_t base_date_start[RANK_base_date];
    static size_t base_date_count[RANK_base_date];
    static int base_date[] = {20100707};
    time_len = 1;			/* number of records of base_date data */
    base_date_start[0] = 0;
    base_date_count[0] = time_len;

    static size_t base_time_start[RANK_base_time];
    static size_t base_time_count[RANK_base_time];
    static int base_time[] = {1200};
    time_len = 1;			/* number of records of base_time data */
    base_time_start[0] = 0;
    base_time_count[0] = time_len;

    static size_t valid_date_start[RANK_valid_date];
    static size_t valid_date_count[RANK_valid_date];
    static int valid_date[] = {20100707};
    time_len = 1;			/* number of records of valid_date data */
    valid_date_start[0] = 0;
    valid_date_count[0] = time_len;

    static size_t valid_time_start[RANK_valid_time];
    static size_t valid_time_count[RANK_valid_time];
    static int valid_time[] = {1200};
    time_len = 1;			/* number of records of valid_time data */
    valid_time_start[0] = 0;
    valid_time_count[0] = time_len;

    static size_t forc_hrs_start[RANK_forc_hrs];
    static size_t forc_hrs_count[RANK_forc_hrs];
    static float forc_hrs[] = {0};
    time_len = 1;			/* number of records of forc_hrs data */
    forc_hrs_start[0] = 0;
    forc_hrs_count[0] = time_len;

    static size_t wrtn_date_start[RANK_wrtn_date];
    static size_t wrtn_date_count[RANK_wrtn_date];
    static int wrtn_date[] = {110216};
    time_len = 1;			/* number of records of wrtn_date data */
    wrtn_date_start[0] = 0;
    wrtn_date_count[0] = time_len;

    static size_t wrtn_time_start[RANK_wrtn_time];
    static size_t wrtn_time_count[RANK_wrtn_time];
    static int wrtn_time[] = {424};
    time_len = 1;			/* number of records of wrtn_time data */
    wrtn_time_start[0] = 0;
    wrtn_time_count[0] = time_len;

    static size_t sfc_pres_start[RANK_sfc_pres];
    static size_t sfc_pres_count[RANK_sfc_pres];
    static float sfc_pres[240*121]={0};

    time_len = 1;			/* number of records of sfc_pres data */
    sfc_pres_start[0] = 0;
    sfc_pres_start[1] = 0;
    sfc_pres_start[2] = 0;
    sfc_pres_count[0] = time_len;
    sfc_pres_count[1] = lat_len;
    sfc_pres_count[2] = lon_len;

    static size_t temp_scrn_start[RANK_temp_scrn];
    static size_t temp_scrn_count[RANK_temp_scrn];
    static float temp_scrn[240*121]={0};


    time_len = 1;			/* number of records of temp_scrn data */
    temp_scrn_start[0] = 0;
    temp_scrn_start[1] = 0;
    temp_scrn_start[2] = 0;
    temp_scrn_count[0] = time_len;
    temp_scrn_count[1] = lat_len;
    temp_scrn_count[2] = lon_len;

    static size_t qsair_scrn_start[RANK_qsair_scrn];
    static size_t qsair_scrn_count[RANK_qsair_scrn];
    static float qsair_scrn[240*121] = {0};

    time_len = 1;			/* number of records of qsair_scrn data */
    qsair_scrn_start[0] = 0;
    qsair_scrn_start[1] = 0;
    qsair_scrn_start[2] = 0;
    qsair_scrn_count[0] = time_len;
    qsair_scrn_count[1] = lat_len;
    qsair_scrn_count[2] = lon_len;

    static size_t topog_start[RANK_topog];
    static size_t topog_count[RANK_topog];
    static float topog[240*121] = {0};
    time_len = 1;			/* number of records of topog data */
    topog_start[0] = 0;
    topog_start[1] = 0;
    topog_start[2] = 0;
    topog_count[0] = time_len;
    topog_count[1] = lat_len;
    topog_count[2] = lon_len;

    static size_t mslp_start[RANK_mslp];
    static size_t mslp_count[RANK_mslp];
    static float mslp[240*121] = {0};
    time_len = 1;			/* number of records of mslp data */
    mslp_start[0] = 0;
    mslp_start[1] = 0;
    mslp_start[2] = 0;
    mslp_count[0] = time_len;
    mslp_count[1] = lat_len;
    mslp_count[2] = lon_len;

    static size_t sfc_temp_start[RANK_sfc_temp];
    static size_t sfc_temp_count[RANK_sfc_temp];
    static float sfc_temp[240*121] = {0};

    time_len = 1;			/* number of records of sfc_temp data */
    sfc_temp_start[0] = 0;
    sfc_temp_start[1] = 0;
    sfc_temp_start[2] = 0;
    sfc_temp_count[0] = time_len;
    sfc_temp_count[1] = lat_len;
    sfc_temp_count[2] = lon_len;

    static size_t zonal_wnd_start[RANK_zonal_wnd];
    static size_t zonal_wnd_count[RANK_zonal_wnd];
    static float zonal_wnd[240*121*31] = {0};



    stat = nc_put_vara_double(ncid, time_id, time_start, time_count, time);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_text(ncid, seg_type_id, seg_type_start, seg_type_count, seg_type);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_put_var_float(ncid, lat_id, lat);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_var_float(ncid, lon_id, lon);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_var_float(ncid, lvl_id, lvl);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_put_vara_int(ncid, base_date_id, base_date_start, base_date_count, base_date);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_put_vara_int(ncid, base_time_id, base_time_start, base_time_count, base_time);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_put_vara_int(ncid, valid_date_id, valid_date_start, valid_date_count, valid_date);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_int(ncid, valid_time_id, valid_time_start, valid_time_count, valid_time);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, forc_hrs_id, forc_hrs_start, forc_hrs_count, forc_hrs);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_int(ncid, wrtn_date_id, wrtn_date_start, wrtn_date_count, wrtn_date);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_int(ncid, wrtn_time_id, wrtn_time_start, wrtn_time_count, wrtn_time);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, sfc_pres_id, sfc_pres_start, sfc_pres_count, sfc_pres);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, temp_scrn_id, temp_scrn_start, temp_scrn_count, temp_scrn);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, qsair_scrn_id, qsair_scrn_start, qsair_scrn_count, qsair_scrn);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, topog_id, topog_start, topog_count, topog);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, mslp_id, mslp_start, mslp_count, mslp);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_vara_float(ncid, sfc_temp_id, sfc_temp_start, sfc_temp_count, sfc_temp);
    check_err(stat,__LINE__,__FILE__);

    time_len = 1;			/* number of records of zonal_wnd data */
    
    for(i=30; i>=0; i--)
    {
        zonal_wnd_start[0] = 0;
        zonal_wnd_start[1] = i;
        zonal_wnd_start[2] = 0;
        zonal_wnd_start[3] = 0;
        zonal_wnd_count[0] = time_len;
        zonal_wnd_count[1] = 1;
        zonal_wnd_count[2] = lat_len;
        zonal_wnd_count[3] = lon_len;
        stat = nc_put_vara_float(ncid, zonal_wnd_id, zonal_wnd_start, zonal_wnd_count, zonal_wnd);
        check_err(stat,__LINE__,__FILE__);
    }

   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);
   return 0;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing nofill mode.\n");
   {
       printf("*** Create file in fill mode, writing all values...");
       if (create_file(FILE_NAME1, NC_FILL)) ERR;
       SUMMARIZE_ERR;
   }
   {
       printf("*** Create file with same data in nofill mode, writing all values...");
       if (create_file(FILE_NAME2, NC_NOFILL)) ERR;
       SUMMARIZE_ERR;
   }
   {
       int ncid1, ncid2;
       int nvars1, nvars2;
       int varid;
       int badvars;

       printf("*** Compare values in fill mode and nofill mode files...");
       /* compare numerical data in two files, which should be identical */
       if (nc_open(FILE_NAME1, NC_NOWRITE, &ncid1)) ERR;
       if (nc_open(FILE_NAME2, NC_NOWRITE, &ncid2)) ERR;
       if (nc_inq_nvars(ncid1, &nvars1)) ERR;
       if (nc_inq_nvars(ncid2, &nvars2)) ERR;
       if (nvars1 != nvars2) ERR;
       badvars = 0;
       for(varid = 0; varid < nvars1; varid++) {
	   nc_type vtype;
	   if (nc_inq_vartype(ncid1, varid, &vtype)) ERR;
	   if (vtype == NC_INT || vtype == NC_BYTE || vtype == NC_SHORT 
	      || vtype == NC_FLOAT || vtype == NC_DOUBLE) { /* don't compare chars, for now */
	       size_t nvals, nn;
	       double *data1, *data2;
	       int ndims, *dimids, dim;
	       /* How many values in this variable to compare? */
	       if (nc_inq_varndims(ncid1, varid, &ndims)) ERR;
	       dimids = malloc((ndims + 1) * sizeof(int));
	       if (!dimids) ERR;
	       if (nc_inq_vardimid (ncid1, varid, dimids)) ERR;
	       nvals = 1;
	       for(dim = 0; dim < ndims; dim++) {
		   size_t len;
		   if (nc_inq_dimlen(ncid1, dimids[dim], &len)) ERR;
		   nvals *= len;
	       }
	       /* Allocate space to hold values in both files */
	       data1 = malloc((nvals + 1) * sizeof(double));
	       if (!data1) ERR;
	       data2 = malloc((nvals + 1) * sizeof(double));
	       if (!data2) ERR;
	       /* Read in values */
	       if (nc_get_var_double(ncid1, varid, data1)) ERR;
	       if (nc_get_var_double(ncid2, varid, data2)) ERR;
	       /* Compare values */
	       for(nn = 0; nn < nvals; nn++) {
		   char varname1[NC_MAX_NAME];		   
		   char varname2[NC_MAX_NAME];
		   if (data1[nn] != data2[nn]) {
		       badvars++;
		       if (nc_inq_varname(ncid1, varid, varname1)) ERR;
		       if (nc_inq_varname(ncid1, varid, varname2)) ERR;
		       fprintf(stderr, 
	       "\tFrom   fill file, %s[%d] = %.15g\tFrom nofill file, %s[%d] = %.15g\n", 
			       varname1, nn, data1[nn], varname2, nn, data2[nn]);
		       break;
		   };
	       }
	       free(data1);
	       free(data2);
	       free(dimids);
	   }
       }
       if(badvars > 0) ERR;
       if (nc_close(ncid1)) ERR;
       if (nc_close(ncid2)) ERR;
       SUMMARIZE_ERR;
   }
   FINAL_RESULTS;
}
