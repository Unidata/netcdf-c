/* This is part of the netCDF package. Copyright 2005-2007 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test handling of formats.

   Ed Hartnett, 11/22/17
*/

#include "config.h"
#include <nc_tests.h>
#include "err_macros.h"

#define FILE_NAME_BASE "tst_formats"
#define HDF4_FILE "ref_contiguous.hdf4"

/* Determine how many formats are available, and what they are. */
void
determine_test_formats(int *num_formats, int *format)
{
   int ind = 0;
   int num;

   /* Check inputs. */
   assert(num_formats && format);

   /* We always have classic and 64-bit offset */
   num = 2;
   format[ind++] = NC_FORMAT_CLASSIC;
   format[ind++] = NC_FORMAT_64BIT_OFFSET;

   /* Do we have netCDF-4 and netCDF-4 classic? */
#ifdef USE_NETCDF4
   num += 2;
   format[ind++] = NC_FORMAT_NETCDF4;
   format[ind++] = NC_FORMAT_NETCDF4_CLASSIC;
#endif /* USE_NETCDF4 */

   /* Do we have CDF5? */
#ifdef ENABLE_CDF5
   num++;
   format[ind++] = NC_FORMAT_CDF5;
#endif /* ENABLE_CDF5 */

   *num_formats = num;
}

/* Function to test nc_inq_format(). */
int
check_inq_format(int ncid, int expected_format, int expected_extended_format, int expected_mode)
{
   int format;
   int extended_format;
   int mode;
   
   if (nc_inq_format(ncid + 66000, NULL) != NC_EBADID) ERR;
   if (nc_inq_format(ncid, NULL)) ERR;
   if (nc_inq_format(ncid, &format)) ERR;
   if (format != expected_format) {
      printf("format %d expected_format %d\n", format, expected_format);      
      ERR;
   }
   if (nc_inq_format_extended(ncid + 66000, &extended_format, &mode) != NC_EBADID) ERR;
   {
      int mode;
      if (nc_inq_format_extended(ncid, NULL, &mode)) ERR;
      if (mode != expected_mode) {
         printf("expected_mode %x mode %x\n", expected_mode, mode);
         //ERR;
      }
   }
   {
      int extended_format;
      if (nc_inq_format_extended(ncid, &extended_format, NULL)) ERR;
      if (extended_format != expected_extended_format) ERR;
   }

   if (nc_inq_format_extended(ncid, &extended_format, &mode)) ERR;
   if (mode != expected_mode) ERR;
   if (extended_format != expected_extended_format) ERR;

   /* Nothing to do with inq_format, but let's check the base_pe
    * functions. */
   if (expected_format == NC_FORMAT_CLASSIC || expected_format == NC_FORMAT_64BIT_OFFSET ||
       expected_format == NC_FORMAT_CDF5) {
      if (nc_set_base_pe(ncid, 0)) ERR;
      if (nc_inq_base_pe(ncid, NULL)) ERR;
   } else {
      if (nc_set_base_pe(ncid, 0) != NC_ENOTNC3) ERR;
      if (nc_inq_base_pe(ncid, NULL) != NC_ENOTNC3) ERR;
   }

   return 0;
}

int
main(int argc, char **argv)
{
   int format[MAX_NUM_FORMATS];
   int num_formats;
   
   /* How many formats to be tested? */
   determine_test_formats(&num_formats, format);

   printf("\n*** Testing netcdf format functions.\n");
   {
      int ncid;
      int expected_mode;
      int expected_extended_format;
      char file_name[NC_MAX_NAME + 1];
      int f;
   
      for (f = 0; f < num_formats; f++)
      {
         printf("*** testing nc_inq_format() and nc_inq_format_extended() with format %d...", format[f]);
         sprintf(file_name, "%s_%d.nc", FILE_NAME_BASE, format[f]);

         /* Set up test. */
         switch (format[f]) {
         case NC_FORMAT_CLASSIC:
            expected_extended_format = NC_FORMATX_NC3;
            expected_mode = 0;
            break;
         case NC_FORMAT_64BIT_OFFSET:
            expected_extended_format = NC_FORMATX_NC3;
            expected_mode = NC_64BIT_OFFSET;            
            break;
         case NC_FORMAT_CDF5:
            expected_extended_format = NC_FORMATX_NC3;
            expected_mode = NC_CDF5;            
            break;
         case NC_FORMAT_NETCDF4:
            expected_extended_format = NC_FORMATX_NC4;
            expected_mode = NC_NETCDF4;            
            break;
         case NC_FORMAT_NETCDF4_CLASSIC:
            expected_extended_format = NC_FORMATX_NC4;
            expected_mode = NC_NETCDF4|NC_CLASSIC_MODEL;            
            break;
         }
         if (nc_set_default_format(format[f], NULL)) ERR;

         /* Create a file. */
         if (nc_create(file_name, 0, &ncid)) ERR;
         if (check_inq_format(ncid, format[f], expected_extended_format, expected_mode)) ERR;
         if (nc_close(ncid)) ERR;

         /* Re-open the file and check it again. */
         if (nc_open(file_name, 0, &ncid)) ERR;
         /* Classic flag is not set on mode in nc_open(). Not sure if
          * this is a bug or not. */
         if (format[f] == NC_FORMAT_NETCDF4_CLASSIC)
            expected_mode = NC_NETCDF4;
         if (check_inq_format(ncid, format[f], expected_extended_format, expected_mode)) ERR;
         if (nc_close(ncid)) ERR;
         SUMMARIZE_ERR;
      } /* next format */

      {
#define NUM_TEST_TYPES 1
#define NUM_VARS 1
#define NUM_DIMS 2
#define DIM_0_NAME "dim_0"
#define DIM_1_NAME "dim_1"
#define DIM_1_LEN 2
         for (f = 0; f < num_formats; f++)
         {
            nc_type test_type[NUM_TEST_TYPES] = {NC_INT};
            int ncid, varid[NUM_VARS], dimid[NUM_DIMS];
            int t;
            
            printf("*** testing fill values with format %d...", format[f]);
            sprintf(file_name, "%s_file_values_%d.nc", FILE_NAME_BASE, format[f]);

            /* Set the format for the test. */
            if (nc_set_default_format(format[f], NULL)) ERR;

            /* Create a file. */
            if (nc_create(file_name, 0, &ncid)) ERR;

            /* Create dims. */
            if (nc_def_dim(ncid, DIM_0_NAME, NC_UNLIMITED, &dimid[0])) ERR;
            if (nc_def_dim(ncid, DIM_1_NAME, DIM_1_LEN, &dimid[1])) ERR;

            for (t = 0; t < NUM_TEST_TYPES; t++)
            {
               char var_name[NC_MAX_NAME + 1];
               
               sprintf(var_name, "var_type_%d", test_type[t]);
               
               /* Create a var of each type. */
               if (nc_def_var(ncid, var_name, test_type[t], NUM_DIMS, dimid,
                              &varid[t])) ERR;
               
            } /* next type */

            /* End define mode. */
            if (nc_enddef(ncid)) ERR;

            /* Write to each var. */
            for (t = 0; t < NUM_TEST_TYPES; t++)
            {
               size_t start[NUM_DIMS] = {1, 0};
               size_t count[NUM_DIMS] = {1, 1};
               int data = TEST_VAL_42;
               
               if (nc_put_vara(ncid, varid[t], start, count, &data)) ERR;
            }            
            if (nc_close(ncid)) ERR;

            /* Check the files for correctness. */
            if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;

            for (t = 0; t < NUM_TEST_TYPES; t++)
            {
               int data_in[4];
               if (nc_get_var(ncid, varid[t], data_in)) ERR;
               if (data_in[0] != NC_FILL_INT) ERR;
               if (data_in[1] != NC_FILL_INT) ERR;
               if (data_in[2] != TEST_VAL_42) ERR;
               if (data_in[3] != NC_FILL_INT) ERR;
            }
            
            if (nc_close(ncid)) ERR;
      
            SUMMARIZE_ERR;
         } /* next format */
      }
   }
   FINAL_RESULTS;
}
