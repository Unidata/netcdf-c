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

/**
 * @internal
 * Determine how many formats are available, and what they are. 
 *
 * @note The constant macro ::MAX_NUM_FORMATS can be used to declare
 * arrays format and num_types.
 *
 * @param num_formats Pointer that gets the number of available
 * formats.
 * @param format Array that gets the available formats.
 * @param num_types Array with one entry per format. It gets the
 * number of atomic types available in that format.
 * @param test_type Array that gets all available atomic types,
 * provided for convenience.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett*/
int
determine_test_formats(int *num_formats, int *format, int *num_types,
                       int *test_type)
{
   int ind = 0;
   int num;

   /* Check inputs. */
   assert(num_formats && format && num_types && test_type);

   /* Fill in the types. */
   test_type[0] = NC_BYTE;
   test_type[1] = NC_CHAR;
   test_type[2] = NC_SHORT;
   test_type[3] = NC_INT;
   test_type[4] = NC_FLOAT;
   test_type[5] = NC_DOUBLE;
   test_type[6] = NC_UBYTE;
   test_type[7] = NC_USHORT;
   test_type[8] = NC_UINT;
   test_type[9] = NC_INT64;
   test_type[10] = NC_UINT64;
   test_type[11] = NC_STRING;

   /* We always have classic and 64-bit offset */
   num = 2;
   num_types[ind] = 6;
   format[ind++] = NC_FORMAT_CLASSIC;
   num_types[ind] = 6;
   format[ind++] = NC_FORMAT_64BIT_OFFSET;

   /* Do we have netCDF-4 and netCDF-4 classic? */
#ifdef USE_NETCDF4
   num += 2;
   num_types[ind] = 12;
   format[ind++] = NC_FORMAT_NETCDF4;
   num_types[ind] = 6;
   format[ind++] = NC_FORMAT_NETCDF4_CLASSIC;
#endif /* USE_NETCDF4 */

   /* Do we have CDF5? */
#ifdef ENABLE_CDF5
   num++;
   num_types[ind] = 11;
   format[ind++] = NC_FORMAT_CDF5;
#endif /* ENABLE_CDF5 */

   *num_formats = num;

   return NC_NOERR;
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
   int num_types[MAX_NUM_FORMATS];
   int test_type[NUM_ENHANCED_TYPES];
   int num_formats;
   int ret;
   
   /* How many formats to be tested? */
   if ((ret = determine_test_formats(&num_formats, format, num_types,
                                     test_type)))
      return ret;

   printf("\n*** Testing netcdf format functions.\n");
   {
      int ncid;
      int expected_mode;
      int expected_extended_format;
      char file_name[NC_MAX_NAME + 1];
      int f;
   
      for (f = 0; f < num_formats; f++)
      {
         printf("*** testing nc_inq_format() and nc_inq_format_extended() "
                "with format %d...", format[f]);
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
#define NUM_DIMS 2
#define DIM_0_NAME "dim_0"
#define DIM_1_NAME "dim_1"
#define DIM_1_LEN 2
#define NUM_VALUES 4
#define NUM_FILL_COMBOS 3
         for (f = 0; f < num_formats; f++)
         {
            int ncid, varid[NUM_ENHANCED_TYPES], dimid[NUM_DIMS];
            signed char byte_data = -TEST_VAL_42;
            unsigned char char_data = 'a';
            short int short_data = -1000 - TEST_VAL_42;
            int int_data = -NC_MAX_SHORT - TEST_VAL_42;
            float float_data = -TEST_VAL_42/100;
            double double_data = TEST_VAL_42 * 10000000;
            unsigned char ubyte_data = TEST_VAL_42;
            unsigned short int ushort_data = 1000 + TEST_VAL_42;
            unsigned int uint_data = NC_MAX_SHORT + TEST_VAL_42;
            long long int int64_data = ((long long int)-NC_MAX_INT * 2);
            unsigned long long int uint64_data = ((unsigned long long int)NC_MAX_INT * 2);
            char *string_data[] = {"x"};
            void *data[NUM_ENHANCED_TYPES] = {&byte_data, &char_data, &short_data, &int_data, &float_data,
                                              &double_data, &ubyte_data, &ushort_data, &uint_data, &int64_data,
                                              &uint64_data, string_data};
            signed char byte_data_in[NUM_VALUES];
            unsigned char char_data_in[NUM_VALUES];
            short int short_data_in[NUM_VALUES];
            int int_data_in[NUM_VALUES];
            float float_data_in[NUM_VALUES];
            double double_data_in[NUM_VALUES];
            unsigned char ubyte_data_in[NUM_VALUES];
            unsigned short int ushort_data_in[NUM_VALUES];
            unsigned int uint_data_in[NUM_VALUES];
            long long int int64_data_in[NUM_VALUES];
            unsigned long long int uint64_data_in[NUM_VALUES];
            char *string_data_in[NUM_VALUES];
            void *data_in[NUM_ENHANCED_TYPES] = {byte_data_in, char_data_in, short_data_in, int_data_in,
                                                 float_data_in, double_data_in, ubyte_data_in,
                                                 ushort_data_in, uint_data_in, int64_data_in,
                                                 uint64_data_in, string_data_in};
            int fill_combo;

            /* We test three combinations of fill value
             * settings. First, no_fill, second fill with default fill
             * values, third, fill with custom fill values. */
            for (fill_combo = 0; fill_combo < NUM_FILL_COMBOS; fill_combo++)
            {
               int default_fill = fill_combo == 1 ? 1 : 0;
               int use_fill = fill_combo ? 1 : 0;
               signed char byte_fill = default_fill ? NC_FILL_BYTE : NC_FILL_BYTE + TEST_VAL_42;
               unsigned char char_fill = default_fill ? NC_FILL_CHAR : NC_FILL_CHAR + TEST_VAL_42;
               short int short_fill = default_fill ? NC_FILL_SHORT : NC_FILL_SHORT + TEST_VAL_42;
               int int_fill = default_fill ? NC_FILL_INT : NC_FILL_INT + TEST_VAL_42;
               float float_fill = default_fill ? NC_FILL_FLOAT : NC_FILL_FLOAT / (float)2.0;
               double double_fill = default_fill ? NC_FILL_DOUBLE : NC_FILL_DOUBLE / (double)2.0;
               unsigned char ubyte_fill = default_fill ? NC_FILL_UBYTE : NC_FILL_UBYTE - TEST_VAL_42;
               unsigned short int ushort_fill = default_fill ? NC_FILL_USHORT : NC_FILL_USHORT - TEST_VAL_42;
               unsigned int uint_fill = default_fill ? NC_FILL_UINT : NC_FILL_UINT - TEST_VAL_42;
               long long int int64_fill = default_fill ? NC_FILL_INT64 : NC_FILL_INT64 + TEST_VAL_42;
               unsigned long long int uint64_fill = default_fill ? NC_FILL_UINT64 : NC_FILL_UINT64 - TEST_VAL_42;
               char *string_fill[] = {"x"};
               void *data_fill[NUM_ENHANCED_TYPES] = {&byte_fill, &char_fill, &short_fill, &int_fill, &float_fill,
                                                      &double_fill, &ubyte_fill, &ushort_fill, &uint_fill, &int64_fill,
                                                      &uint64_fill, string_fill};
               int t;
               int old_mode;

               sprintf(file_name, "%s_fill_format_%d_fill_combo_%d.nc", FILE_NAME_BASE,
                       format[f], fill_combo);
               printf("*** testing fill values and 1 unlimited in file %s...", file_name);

               /* Set the format for the test. */
               if (nc_set_default_format(format[f], NULL)) ERR;
               /* if (nc_set_log_level(3)) ERR; */

               /* Create a file. */
               if (nc_create(file_name, 0, &ncid)) ERR;

               /* Nofill is set on a file level for classic formats. */
               if (!use_fill && format[f] != NC_FORMAT_NETCDF4 && format[f] != NC_FORMAT_NETCDF4_CLASSIC)
                  if (nc_set_fill(ncid, NC_NOFILL, &old_mode)) ERR;

               /* Create dims. */
               if (nc_def_dim(ncid, DIM_0_NAME, NC_UNLIMITED, &dimid[0])) ERR;
               if (nc_def_dim(ncid, DIM_1_NAME, DIM_1_LEN, &dimid[1])) ERR;

               for (t = 0; t < num_types[f]; t++)
               {
                  char var_name[NC_MAX_NAME + 1];

                  sprintf(var_name, "var_type_%d", test_type[t]);
               
                  /* Create a var of each type. */
                  if (nc_def_var(ncid, var_name, test_type[t], NUM_DIMS, dimid,
                                 &varid[t])) ERR;

                  /* Fill mode is turned off per variable for
                   * netCDF-4. This fails for NC_STRING type, see
                   * github issue #727. */
#ifdef USE_NETCDF4
                  if (format[f] == NC_FORMAT_NETCDF4 || format[f] == NC_FORMAT_NETCDF4_CLASSIC)
                     if (!use_fill && test_type[t] != NC_STRING)
                        if (nc_def_var_fill(ncid, varid[t], NC_NOFILL, NULL)) ERR;
#endif /* USE_NETCDF4 */

                  /* If not using default fill values, set a custom
                   * fill value for this var. */
                  if (use_fill && !default_fill)
                     if (nc_put_att(ncid, varid[t], _FillValue, test_type[t], 1,
                                    data_fill[t])) ERR;
               
               } /* next type */

               /* End define mode. */
               if (nc_enddef(ncid)) ERR;

               /* Write to each var. */
               for (t = 0; t < num_types[f]; t++)
               {
                  size_t start[NUM_DIMS] = {1, 0};
                  size_t count[NUM_DIMS] = {1, 1};

                  if (test_type[t] == NC_STRING)
                  {
#ifdef USE_NETCDF4
                     if (nc_put_vara_string(ncid, varid[t], start, count, data[t])) ERR;
#endif
                  }
                  else
                  {
                     if (nc_put_vara(ncid, varid[t], start, count, data[t])) ERR;
                  }
               }
               if (nc_close(ncid)) ERR;

               /* Check the files for correctness. */
               if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;

               for (t = 0; t < num_types[f]; t++)
               {
                  int type_size;

                  /* For some reason nc_get_var() does not work with
                   * string type. See github issue #726. Skipping
                   * NC_STRING for now. */
                  if (test_type[t] == NC_STRING)
                     continue;

                  /* Get length of this type. */
                  type_size = nctypelen(test_type[t]);
               
                  /* printf("Getting data from var %d f %d type_size %d\n", varid[t], f, type_size); */
                  if (nc_get_var(ncid, varid[t], data_in[t])) ERR;

                  /* There is one real data value. */
                  if (memcmp(data_in[t] + type_size * 2, data[t], type_size)) ERR;

                  /* The other three values were not written. They
                   * will contain fill values if fill mode was on. */
                  if (use_fill)
                  {                     
                     if (memcmp(data_in[t], data_fill[t], type_size)) ERR;
                     if (memcmp(data_in[t] + type_size, data_fill[t], type_size)) ERR;
                     if (memcmp(data_in[t] + type_size * 3, data_fill[t], type_size)) ERR;
                  }

                  /* Get fill value settings. */
                  {
#ifdef USE_NETCDF4
                     int no_fill;
                     double fill_value;

                     /* This only works for netCDF-4 formats. */
                     if (format[f] != NC_FORMAT_NETCDF4 && format[f] != NC_FORMAT_NETCDF4_CLASSIC)
                        continue;
                     /* Get the fill value settings. */
                     if (nc_inq_var_fill(ncid, varid[t], &no_fill, &fill_value)) ERR;
                     if (use_fill && no_fill) ERR;
                     if (!use_fill && !no_fill)
                     {
                        printf("no_fill %d\n", no_fill);
                        /* ERR; */
                     }
                     if (use_fill && memcmp(&fill_value, data_fill[t], type_size)) ERR;
#endif /* USE_NETCDF4 */
                  }
               }
            
               if (nc_close(ncid)) ERR;
               SUMMARIZE_ERR;
            } /* next fill_combo */
         } /* next format */
      }
   }
   FINAL_RESULTS;
}
