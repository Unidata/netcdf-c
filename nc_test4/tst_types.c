/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 types.

   Ed Hartnett
*/

#include "config.h"
#include "nc_tests.h"
#include "err_macros.h"

#define MAX_VARNAME 20
#define NUM_TYPES 6
#define NUM_DIMS 1
#define SIZE 5
#define STRIDE_SIZE 2
#define FILENAME "tst_types.nc"

#define CLEAN_INPUT_BUFFERS                     \
   for (i=0; i<SIZE; i++) {                     \
      ubyte_data_out[i] = 0;                    \
      ushort_data_out[i] = 0;                   \
      uint_data_out[i] = 0;                     \
      int64_data_out[i] = 0;                    \
      uint64_data_out[i] = 0;                   \
   }

int main(int argc, char *argv[])
{
   /* IDs, names, and parameters for the var[asm1] functions. */
   int ncid, varid[NUM_TYPES], dimid;
   char varname[MAX_VARNAME];
   size_t index1[NUM_DIMS], start[NUM_DIMS];
   size_t count[NUM_DIMS];
   ptrdiff_t imap[NUM_DIMS];
   ptrdiff_t stride[NUM_DIMS];

   /* Phoney data we will write. */
   unsigned char ubyte_data_out[] = {0,1,2,3,4};
   unsigned short ushort_data_out[] = {0,11,22,33,44};
   unsigned int uint_data_out[] = {0,111,222,333,3000000000u};
   long long int int64_data_out[] = {0,-111111111,2222222222,-3333333333,444444444};
   unsigned long long int uint64_data_out[] = {0,111111111,2222222222,33333333,44444444};

   /* We will read back in the phoney data with these. */
   unsigned char ubyte_data_in[SIZE];
   unsigned short ushort_data_in[SIZE];
   unsigned int uint_data_in[SIZE];
   long long int int64_data_in[SIZE];
   unsigned long long int uint64_data_in[SIZE];

   int i;
   int type;

   printf("\n*** Testing netCDF-4 new atomic types...");
   {
      /* Open a netcdf-4 file, and one dimension. */
      if (nc_create(FILENAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, "dim1", SIZE, &dimid)) ERR;

      /* Create vars of the new types. Take advantage of the fact that
       * new types are numbered from NC_UBYTE (7) through NC_STRING
       * (12). */
      for(type = 0; type < NUM_TYPES; type++)
      {
         /* Create a var... */
         sprintf(varname, "var_%d", type);
         if (nc_def_var(ncid, varname, type+NC_UBYTE, 1, &dimid, &varid[type])) ERR;
      }
   }
   SUMMARIZE_ERR;

   /* Test the varm functions. */
   printf("*** testing varm functions...");
   {
      CLEAN_INPUT_BUFFERS;
      start[0] = 0;
      count[0] = 1;
      stride[0] = 1;
      imap[0] = 0;

      if (nc_put_varm_ubyte(ncid, varid[0], start, count,
                            stride, imap, ubyte_data_out)) ERR;
      if (nc_get_varm_ubyte(ncid, varid[0], start, count,
                            stride, imap, ubyte_data_in)) ERR;
      for (i=0; i<STRIDE_SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_varm_ushort(ncid, varid[1], start, count,
                             stride, imap, ushort_data_out)) ERR;
      if (nc_get_varm_ushort(ncid, varid[1], start, count,
                             stride, imap, ushort_data_in)) ERR;
      for (i=0; i<STRIDE_SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_varm_uint(ncid, varid[2], start,
                           count, stride, imap, uint_data_out)) ERR;
      if (nc_get_varm_uint(ncid, varid[2], start, count,
                           stride, imap, uint_data_in)) ERR;
      for (i=0; i<STRIDE_SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_varm_longlong(ncid, varid[3], start, count,
                               stride, imap, int64_data_out)) ERR;
      if (nc_get_varm_longlong(ncid, varid[3], start, count,
                               stride, imap, int64_data_in)) ERR;
      for (i=0; i<STRIDE_SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_varm_ulonglong(ncid, varid[4], start, count,
                                stride, imap, uint64_data_out)) ERR;
      if (nc_get_varm_ulonglong(ncid, varid[4], start, count,
                                stride, imap, uint64_data_in)) ERR;
      for (i=0; i<STRIDE_SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;
   }
   SUMMARIZE_ERR;

   /* Test the vars functions. */
   printf("*** testing vars functions...");
   {
      CLEAN_INPUT_BUFFERS;
      start[0] = 0;
      count[0] = 2;
      stride[0] = STRIDE_SIZE;

      if (nc_put_vars_uchar(ncid, varid[0], start, count,
                            stride, ubyte_data_out)) ERR;
      if (nc_get_vars_uchar(ncid, varid[0], start, count,
                            stride, ubyte_data_in)) ERR;
      if (ubyte_data_in[0] != ubyte_data_out[0]) ERR;
      if (ubyte_data_in[1] != ubyte_data_out[STRIDE_SIZE]) ERR;

      if (nc_put_vars_ushort(ncid, varid[1], start, count,
                             stride, ushort_data_out)) ERR;
      if (nc_get_vars_ushort(ncid, varid[1], start, count,
                             stride, ushort_data_in)) ERR;
      for (i=0; i<2; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vars_uint(ncid, varid[2], start,
                           count, stride, uint_data_out)) ERR;
      if (nc_get_vars_uint(ncid, varid[2], start, count,
                           stride, uint_data_in)) ERR;
      for (i=0; i<2; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vars_longlong(ncid, varid[3], start, count,
                               stride, int64_data_out)) ERR;
      if (nc_get_vars_longlong(ncid, varid[3], start, count,
                               stride, int64_data_in)) ERR;
      for (i=0; i<2; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vars_ulonglong(ncid, varid[4], start, count,
                                stride, uint64_data_out)) ERR;
      if (nc_get_vars_ulonglong(ncid, varid[4], start, count,
                                stride, uint64_data_in)) ERR;
      for (i=0; i<2; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;
   }
   SUMMARIZE_ERR;

   /* Test the vara functions. */
   printf("*** testing vara functions...");
   {
      CLEAN_INPUT_BUFFERS;
      start[0] = 0;
      count[0] = SIZE;

      if (nc_put_vara_uchar(ncid, varid[0], start, count, ubyte_data_out)) ERR;
      if (nc_get_vara_uchar(ncid, varid[0], start, count, ubyte_data_in)) ERR;
      for (i=0; i<SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vara_ushort(ncid, varid[1], start, count, ushort_data_out)) ERR;
      if (nc_get_vara_ushort(ncid, varid[1], start, count, ushort_data_in)) ERR;
      for (i=0; i<SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vara_uint(ncid, varid[2], start, count, uint_data_out)) ERR;
      if (nc_get_vara_uint(ncid, varid[2], start, count, uint_data_in)) ERR;
      for (i=0; i<SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vara_longlong(ncid, varid[3], start, count, int64_data_out)) ERR;
      if (nc_get_vara_longlong(ncid, varid[3], start, count, int64_data_in)) ERR;
      for (i=0; i<SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

      if (nc_put_vara_ulonglong(ncid, varid[4], start, count, uint64_data_out)) ERR;
      if (nc_get_vara_ulonglong(ncid, varid[4], start, count, uint64_data_in)) ERR;
      for (i=0; i<SIZE; i++)
         if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;
   }
   SUMMARIZE_ERR;

   /* Test the var1 functions. */
   printf("*** testing var1 functions...");
   {
      CLEAN_INPUT_BUFFERS;
      index1[0] = 0;

      if (nc_put_var1_uchar(ncid, varid[0], index1, ubyte_data_out)) ERR;
      if (nc_get_var1_uchar(ncid, varid[0], index1, ubyte_data_in)) ERR;
      if (ubyte_data_in[0] != ubyte_data_out[0]) ERR;

      if (nc_put_var1_ushort(ncid, varid[1], index1, ushort_data_out)) ERR;
      if (nc_get_var1_ushort(ncid, varid[1], index1, ushort_data_in)) ERR;
      if (ushort_data_in[0] != ushort_data_out[0]) ERR;

      if (nc_put_var1_uint(ncid, varid[2], index1, uint_data_out)) ERR;
      if (nc_get_var1_uint(ncid, varid[2], index1, uint_data_in)) ERR;
      if (uint_data_in[0] != uint_data_out[0]) ERR;

      if (nc_put_var1_longlong(ncid, varid[3], index1, int64_data_out)) ERR;
      if (nc_get_var1_longlong(ncid, varid[3], index1, int64_data_in)) ERR;
      if (int64_data_in[0] != int64_data_out[0]) ERR;

      if (nc_put_var1_ulonglong(ncid, varid[4], index1, uint64_data_out)) ERR;
      if (nc_get_var1_ulonglong(ncid, varid[4], index1, uint64_data_in)) ERR;
      if (uint64_data_in[0] != uint64_data_out[0]) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
