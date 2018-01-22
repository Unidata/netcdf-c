/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   Test the netCDF-4 attribute code.

   Ed Hartnett
*/

#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "nc4internal.h"

/* The data file we will create. */
#define FILE_NAME "tst_atts.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netCDF-4 attributes.\n");
   nc_set_log_level(3);
   printf("*** testing attribute renaming for a global attribute...");
   {
#define OLD_NAME "Constantinople"
#define OLD_NAME_2 "Constantinopolis"
#define NEW_NAME "Istanbul"
#define CONTENTS "Lots of people!"
#define VAR_NAME "Earth"

      int ncid, attid;
      char *data_in;
      char too_long_name[NC_MAX_NAME + 2];
      
      /* Set up a name that is too long for netCDF. */
      memset(too_long_name, 'a', NC_MAX_NAME + 1);
      too_long_name[NC_MAX_NAME + 1] = 0;

      if (!(data_in = malloc(strlen(CONTENTS) + 1))) ERR;

      /* Create a file with an att. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, 0, NULL, NULL)) ERR;

      /* These will not work. */
      if (nc_put_att_text(ncid + TEST_VAL_42, NC_GLOBAL, OLD_NAME, strlen(CONTENTS),
                          CONTENTS) != NC_EBADID) ERR;
      if (nc_put_att_text(ncid, TEST_VAL_42, OLD_NAME, strlen(CONTENTS),
                          CONTENTS) != NC_ENOTVAR) ERR;
      if (nc_put_att_text(ncid, NC_GLOBAL, NULL, strlen(CONTENTS),
                          CONTENTS) != NC_EBADNAME) ERR;
      if (nc_put_att_text(ncid, NC_GLOBAL, BAD_NAME, strlen(CONTENTS),
                          CONTENTS) != NC_EBADNAME) ERR;
      if (nc_put_att_text(ncid, NC_GLOBAL, too_long_name, strlen(CONTENTS),
                          CONTENTS) != NC_EBADNAME) ERR;
      {
         /* Check that the NC_GLOBAL reserved words are rejected. */
         const char** reserved = NC_RESERVED_ATT_LIST;
         for ( ; *reserved; reserved++)
         {
            if (nc_put_att_text(ncid, NC_GLOBAL, *reserved, strlen(CONTENTS),
                                CONTENTS) != NC_ENAMEINUSE) ERR;
         }
      }
      {
         /* Check that the variable reserved words are rejected. */
         const char** reserved = NC_RESERVED_VARATT_LIST;
         for ( ; *reserved; reserved++)
         {
            if (nc_put_att_text(ncid, 0, *reserved, strlen(CONTENTS),
                                CONTENTS) != NC_ENAMEINUSE) ERR;
         }
      }
      
      /* Write the attribute at last. */
      if (nc_put_att_text(ncid, NC_GLOBAL, OLD_NAME, strlen(CONTENTS),
                          CONTENTS)) ERR;
      
      /* Write another with different name. */
      if (nc_put_att_text(ncid, NC_GLOBAL, OLD_NAME_2, strlen(CONTENTS),
                          CONTENTS)) ERR;

      /* These will not work. */
      if (nc_rename_att(ncid + TEST_VAL_42, NC_GLOBAL, OLD_NAME, NEW_NAME) != NC_EBADID) ERR;
      if (nc_rename_att(ncid, TEST_VAL_42, OLD_NAME, NEW_NAME) != NC_ENOTVAR) ERR;
      if (nc_rename_att(ncid, NC_GLOBAL, OLD_NAME, NULL) != NC_EINVAL) ERR;
      if (nc_rename_att(ncid, NC_GLOBAL, OLD_NAME, BAD_NAME) != NC_EBADNAME) ERR;
      if (nc_rename_att(ncid, NC_GLOBAL, OLD_NAME, too_long_name) != NC_EMAXNAME) ERR;
      if (nc_rename_att(ncid, NC_GLOBAL, OLD_NAME, OLD_NAME_2) != NC_ENAMEINUSE) ERR;
      
      /* Rename the att. */
      if (nc_rename_att(ncid, NC_GLOBAL, OLD_NAME, NEW_NAME)) ERR;

      /* These will not work. */
      if (nc_inq_attid(ncid + TEST_VAL_42, NC_GLOBAL, NEW_NAME, &attid) != NC_EBADID) ERR;
      if (nc_inq_attid(ncid, TEST_VAL_42, NEW_NAME, &attid) != NC_ENOTVAR) ERR;
      if (nc_inq_attid(ncid, NC_GLOBAL, NULL, &attid) != NC_EBADNAME) ERR;

      /* Check the file. */
      if (nc_inq_attid(ncid, NC_GLOBAL, NEW_NAME, &attid)) ERR;
      if (attid != 0) ERR;

      /* This also works. */
      if (nc_inq_attid(ncid, NC_GLOBAL, NEW_NAME, NULL)) ERR;

      /* These will not work. */
      if (nc_get_att_text(ncid + TEST_VAL_42, NC_GLOBAL, NEW_NAME, data_in) != NC_EBADID) ERR;
      if (nc_get_att_text(ncid, TEST_VAL_42, NEW_NAME, data_in) != NC_ENOTVAR) ERR;
      if (nc_get_att_text(ncid, NC_GLOBAL, NULL, data_in) != NC_EBADNAME) ERR;

      /* Get the attribute at last. */
      if (nc_get_att_text(ncid, NC_GLOBAL, NEW_NAME, data_in)) ERR;

      if (nc_get_att_text(ncid, NC_GLOBAL, NEW_NAME, data_in)) ERR;
      if (strncmp(CONTENTS, data_in, strlen(CONTENTS))) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_attid(ncid, NC_GLOBAL, NEW_NAME, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, NC_GLOBAL, NEW_NAME, data_in)) ERR;
      if (strncmp(CONTENTS, data_in, strlen(CONTENTS))) ERR;
      if (nc_close(ncid)) ERR;

      free(data_in);
   }
   SUMMARIZE_ERR;
   printf("*** testing attribute renaming for a variable attribute...");
   {
#define VAR_NAME "var_name"
#define OLD_NAME1 "Constantinople"
#define NEW_NAME1 "Istanbul____________"
#define CONTENTS1 "Lots of people!"

      int ncid, attid, varid;
      char *data_in;

      if (!(data_in = malloc(strlen(CONTENTS1) + 1))) ERR;

      /* Create a file with an att. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, 0, NULL, &varid)) ERR;
      if (nc_put_att_text(ncid, varid, OLD_NAME1, strlen(CONTENTS1),
                          CONTENTS1)) ERR;

      /* Rename the att. */
      if (nc_rename_att(ncid, varid, OLD_NAME1, NEW_NAME1)) ERR;

      /* Check the file. */
      if (nc_inq_attid(ncid, varid, NEW_NAME1, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, varid, NEW_NAME1, data_in)) ERR;
      if (strncmp(CONTENTS1, data_in, strlen(CONTENTS1))) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_attid(ncid, varid, NEW_NAME1, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, varid, NEW_NAME1, data_in)) ERR;
      if (strncmp(CONTENTS1, data_in, strlen(CONTENTS1))) ERR;
      if (nc_close(ncid)) ERR;

      free(data_in);
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
