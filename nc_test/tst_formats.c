/* This is part of the netCDF package. Copyright 2005-2018 University
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
#define NDIM1 1
#define DIM_LEN 10
#define VAR_NAME "Copernicus_var"
#define DIM_NAME "Copernicus_dim"
#define NUM_FILL_WRITE_TESTS 2
#define NUM_FILL_WRITE_METHOD_TESTS 2

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
#ifdef USE_HDF5
   num += 2;
   format[ind++] = NC_FORMAT_NETCDF4;
   format[ind++] = NC_FORMAT_NETCDF4_CLASSIC;
#endif /* USE_HDF5 */

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
         /*ERR;*/
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
   if (nc_set_base_pe(ncid, 0)) ERR;
   if (nc_inq_base_pe(ncid, NULL)) ERR;

   return 0;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf format functions.\n");
   {
      int ncid;
      int f, d, a;
      int format[MAX_NUM_FORMATS];
      int num_formats;
      int ret;

      /* How many formats to be tested? */
      determine_test_formats(&num_formats, format);

      for (f = 0; f < num_formats; f++)
      {
         printf("*** testing nc_inq_format() and nc_inq_format_extended() with format %d...", format[f]);
         {
            char file_name[NC_MAX_NAME + 1];
            int expected_mode;
            int expected_extended_format;

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
         }
         SUMMARIZE_ERR;
         /* Test without and with actual data write. */
         for (d = 0; d < NUM_FILL_WRITE_TESTS; d++)
         {
            /* Test setting _FillValue directly or calling nc_def_var_fill(). */
            for (a = 0; a < NUM_FILL_WRITE_METHOD_TESTS; a++)
            {
               printf("*** testing late fill handling with format %d writing %d "
                      "using def_var_fill %d...", format[f], d, a);
               char file_name[NC_MAX_NAME + 1];
               int dimid, varid;
               size_t index = {DIM_LEN - 1};
               int data = TEST_VAL_42;
               int data_in;
               int fill_value = TEST_VAL_42 * 2;

               /* Try to set fill mode after data have been written. */
               sprintf(file_name, "%s_%d_%d_%d_elatefill.nc", FILE_NAME_BASE, format[f], d, a);
               if (nc_set_default_format(format[f], NULL)) ERR;
               if (nc_create(file_name, 0, &ncid)) ERR;
               if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
               if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIM1, &dimid, &varid)) ERR;
               if (nc_enddef(ncid)) ERR;
               /* For netCDF-4, we don't actually have to write data to
                * prevent future setting of the fill value. Once the user
                * leaves calls enddef after defining a var, fill values
                * can no longer be set. */
               if (d)
                  if (nc_put_var1_int(ncid, varid, &index, &data)) ERR;
               if (nc_redef(ncid)) ERR;
               if (a)
               {
                  ret = nc_def_var_fill(ncid, varid, NC_FILL, &fill_value);
               }
               else
               {
                  ret = nc_put_att_int(ncid, varid, "_FillValue", NC_INT, 1,
                                       &fill_value);
               }

               /* Setting the fill value after data are written is
                * allowed in classic formats, but not netcdf-4. */
               if (format[f] == NC_FORMAT_CLASSIC || format[f] == NC_FORMAT_64BIT_OFFSET ||
                   format[f] == NC_FORMAT_CDF5)
               {
                  if (ret) ERR;
               }
               else
               {
                  if (ret != (a ? NC_ELATEDEF: NC_ELATEFILL)) ERR;
               }
               if (nc_enddef(ncid)) ERR;
               if (nc_close(ncid)) ERR;

               /* Open the file and check data. */
               if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;
               if (nc_get_var1_int(ncid, varid, &index, &data_in)) ERR;
               if (data_in != (d ? data : NC_FILL_INT)) ERR;
               if (nc_close(ncid)) ERR;
               SUMMARIZE_ERR;
            } /* next fill value method test */
         } /* next fill val write test */

#define NDIM2 2
#define DIM1_NAME "dim1"
#define DIM2_NAME "dim2"
#define NTYPE 6
#define DATA_LEN 4

         printf("*** testing handling of null strides with format %d... ",
                format[f]);
         {
            char file_name[NC_MAX_NAME + 1];
            char var_name[NC_MAX_NAME + 1];
            int dimid[NDIM2];
            int xtype[NTYPE] = {NC_BYTE, NC_CHAR, NC_SHORT, NC_INT, NC_FLOAT, NC_DOUBLE};
            int type_size[NTYPE] = {1, 1, 2, 4, 4, 8};
            int varid[NTYPE];
            size_t start[NDIM2] = {0, 0};
            size_t count[NDIM2] = {2, 2};
            signed char data_byte[DATA_LEN] = {1, 2, 3, 4};
            unsigned char data_char[DATA_LEN] = {1, 2, 3, 4};
            short data_short[DATA_LEN] = {1, 2, 3, 4};
            int data_int[DATA_LEN] = {1, 2, 3, 4};
            float data_float[DATA_LEN] = {1, 2, 3, 4};
            double data_double[DATA_LEN] = {1, 2, 3, 4};
            void *data_ptr[NTYPE] = {data_byte, data_char, data_short, data_int, data_float, data_double};
            int t;

            /* Create the test file. */
            sprintf(file_name, "%s_%d_null_strides.nc", FILE_NAME_BASE, format[f]);
            if (nc_set_default_format(format[f], NULL)) ERR;
            if (nc_create(file_name, 0, &ncid)) ERR;
            if (nc_def_dim(ncid, DIM1_NAME, DIM_LEN, &dimid[0])) ERR;
            if (nc_def_dim(ncid, DIM2_NAME, DIM_LEN, &dimid[1])) ERR;
            for (t = 0; t < NTYPE; t++)
            {
               sprintf(var_name, "var_%d", xtype[t]);
               if (nc_def_var(ncid, var_name, xtype[t], NDIM2, dimid, &varid[t])) ERR;
            }
            if (nc_enddef(ncid)) ERR;

            /* Write some data. */
            for (t = 0; t < NTYPE; t++)
            {
               if (nc_put_vars(ncid, varid[t], start, count, NULL, data_ptr[t])) ERR;
            }
            if (nc_close(ncid)) ERR;

            /* Open the file and check data. */
            {
               int ndims, nvars, ngatts, unlimdimid;

               if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;
               if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
               if (ndims != 2 || nvars != NTYPE || ngatts != 0 || unlimdimid != -1) ERR;
               for (t = 0; t < NTYPE; t++)
               {
                  nc_type my_type;
                  int var_ndims, natts;
                  int var_dimid[NDIM2];
                  void *data_in;

                  if (nc_inq_var(ncid, varid[t], NULL, &my_type, &var_ndims, var_dimid, &natts)) ERR;
                  if (my_type != xtype[t] || var_ndims != 2 || var_dimid[0] != dimid[0] ||
                      var_dimid[1] != dimid[1] || natts != 0) ERR;
                  if (!(data_in = malloc(DATA_LEN * type_size[t]))) ERR;
                  if (nc_get_vars(ncid, varid[t], start, count, NULL, data_in)) ERR;
                  if (memcmp(data_in, data_ptr[t], DATA_LEN * type_size[t])) ERR;
                  free(data_in);

               }
               if (nc_close(ncid)) ERR;
            }

         }
         SUMMARIZE_ERR;
         printf("*** testing bad name for nc_open/nc_create with format %d... ", format[f]);
         {
             int ncid;
             if (nc_set_default_format(format[f], NULL)) ERR;
             if (nc_create(NULL, 0, &ncid) != NC_EINVAL) ERR;
             if (nc_open(NULL, NC_NOWRITE, &ncid) != NC_EINVAL) ERR;
             if (nc_delete(NULL) != NC_EINVAL) ERR;
         }
         SUMMARIZE_ERR;
      } /* next format */
   }
   FINAL_RESULTS;
}
