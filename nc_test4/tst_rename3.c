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

#define TEST_NAME "tst_rename3"

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
   fprintf(stderr,"Test more renaming.\n");
   fprintf(stderr,"*** test renaming affect on varids...");
   {
      int ncid, varid1, varid2;
      int varid_in;
      char file_name[NC_MAX_NAME + 1];

      /* Create file with two scalar vars. */
      sprintf(file_name, "%s_coord_to_non_coord.nc", TEST_NAME);
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
      /* if (varid_in != varid1) ERR; */
      if (nc_inq_varid(ncid, D1_NAME, &varid_in) != NC_ENOTVAR) ERR;
      if (nc_inq_varid(ncid, D2_NAME, &varid_in)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   fprintf(stderr,"*** test renaming coord var to non-coord var...");
   {
      int ncid, dimid1, dimid2, varid1, varid2;
      int dimid_in, varid_in;
      char file_name[NC_MAX_NAME + 1];

      /* Create file with two dims and associated coordinate vars. */
      sprintf(file_name, "%s_coord_to_non_coord.nc", TEST_NAME);
      if (nc_create(file_name, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, D1_NAME, DIM1_LEN, &dimid1)) ERR;
      if (nc_def_dim(ncid, D2_NAME, DIM1_LEN, &dimid2)) ERR;
      if (nc_def_var(ncid, D1_NAME, NC_INT, NDIM1, &dimid1, &varid1)) ERR;
      if (nc_def_var(ncid, D2_NAME, NC_INT, NDIM1, &dimid2, &varid2)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and rename a var. */
      nc_set_log_level(4);
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_rename_var(ncid, varid1, TMP_NAME)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen file and check, */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, D1_NAME, &dimid_in)) ERR;
      printf("dimid_in %d\n", dimid_in);
      if (dimid_in != dimid1) ERR;
      if (nc_inq_dimid(ncid, D2_NAME, &dimid_in)) ERR;
      if (dimid_in != dimid2) ERR;
      if (nc_inq_dimid(ncid, TMP_NAME, &dimid_in) != NC_EBADDIM) ERR;
      if (nc_inq_varid(ncid, TMP_NAME, &varid_in)) ERR;
      /* if (varid_in != varid1) ERR; */
      if (nc_inq_varid(ncid, D1_NAME, &varid_in) != NC_ENOTVAR) ERR;
      if (nc_close(ncid)) ERR;

      /* This should work but does not (yet). */
      /* if (nc_open(file_name, NC_WRITE, &ncid)) ERR; */
      /* if (nc_rename_var(ncid, varid2, D1_NAME)) ERR; */
      /* if (nc_close(ncid)) ERR; */

      /* /\* Reopen file and check, *\/ */
      /* if (nc_open(file_name, NC_WRITE, &ncid)) ERR; */
      /* if (nc_inq_dimid(ncid, D1_NAME, &dimid_in)) ERR; */
      /* if (dimid_in != dimid1) ERR; */
      /* if (nc_inq_dimid(ncid, D2_NAME, &dimid_in)) ERR; */
      /* if (dimid_in != dimid2) ERR; */
      /* if (nc_inq_dimid(ncid, TMP_NAME, &dimid_in) != NC_EBADDIM) ERR; */
      /* if (nc_inq_varid(ncid, TMP_NAME, &varid_in)) ERR; */
      /* if (varid_in != varid1) ERR; */
      /* if (nc_inq_varid(ncid, D1_NAME, &varid_in)) ERR; */
      /* if (varid_in != varid2) ERR; */
      /* if (nc_close(ncid)) ERR; */
   }
   SUMMARIZE_ERR;
   fprintf(stderr,"*** test exchanging names of two coord vars, making them non-coord vars with names same as dims...");
   {
      int ncid, dimid1, dimid2, varid1, varid2;
      /* int dimid_in; */
      /* int varid_in; */
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
      nc_sync(ncid);
      /* This should work but doesn't yet. */
      /* if (nc_rename_var(ncid, varid2, D1_NAME)) ERR; */
      /* nc_sync(ncid); */
      /* if (nc_rename_var(ncid, varid1, D2_NAME)) ERR; */
      if (nc_close(ncid)) ERR;

      /* Reopen file and check, this should work but doesn't yet. */
      if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
      /* if (nc_inq_dimid(ncid, D1_NAME, &dimid_in)) ERR; */
      /* if (dimid_in != dimid1) ERR; */
      /* if (nc_inq_dimid(ncid, D2_NAME, &dimid_in)) ERR; */
      /* if (dimid_in != dimid2) ERR; */
      /* if (nc_inq_dimid(ncid, TMP_NAME, &dimid_in) != NC_EBADDIM) ERR; */
      /* if (nc_inq_varid(ncid, TMP_NAME, &varid_in)) ERR; */
      /* if (varid_in != varid1) ERR; */
      /* if (nc_inq_varid(ncid, D1_NAME, &varid_in)) ERR; */
      /* if (varid_in != varid2) ERR; */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
