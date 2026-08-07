/* This is part of the netCDF package. Copyright 2005-2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test netcdf-4 file code.
   Ed Hartnett
*/

#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#ifdef USE_HDF5
#include <hdf5.h>
#endif

int test_redef(int format);

#define FILE_NAME "tst_files.nc"
#define ATT1_NAME "MoneyOwned"
#define ATT2_NAME "Number_of_Shoes"
#define ATT3_NAME "att3"
#define DIM1_NAME "Character_Growth"
#define DIM1_LEN 10
#define DIM2_NAME "TimeInMonths"
#define DIM2_LEN 15
#define VAR1_NAME "HuckFinn"
#define VAR2_NAME "TomSawyer"
#define VAR3_NAME "Jim"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 file functions.\n");
   {
      char str[NC_MAX_NAME+1];

      /* Actually we never make any promises about the length of the
       * version string, but it is always smaller than NC_MAX_NAME. */
      if (strlen(nc_inq_libvers()) > NC_MAX_NAME) ERR;
      strcpy(str, nc_inq_libvers());
      printf("*** testing version %s...", str);
   }
   SUMMARIZE_ERR;
   printf("*** testing with bad inputs...");
   {
      int ncid;

      /* Create an empty file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* These will all fail due to incorrect mode flag combinations. */
      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_NETCDF4, &ncid) != NC_EINVAL) ERR;
      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_CDF5, &ncid) != NC_EINVAL) ERR;
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CDF5, &ncid) != NC_EINVAL) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple opens and creates...");
   {
      int ncid, ncid2, ncid3, varid, dimids[2];
      int ndims, nvars, natts, unlimdimid;
      int dimids_var[1], var_type;
      size_t dim_len;
      char dim_name[NC_MAX_NAME+1], var_name[NC_MAX_NAME+1];
      unsigned char uchar_out[DIM1_LEN] = {0, 128, 255};

      /* Open and close empty file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate it again. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. Then define a new variable. Since it's
       * netcdf-4, nc_enddef isn't required - it's called
       * automatically. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_redef(ncid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR3_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open three copies of the same file. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_open(FILE_NAME, NC_WRITE, &ncid2)) ERR;
      if (nc_open(FILE_NAME, NC_WRITE, &ncid3)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq(ncid2, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq(ncid3, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;
      if (nc_close(ncid2)) ERR;
      if (nc_close(ncid3)) ERR;

      /* Open and close empty file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a file with one dimension and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 0 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a simple file, and write some data to it. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_BYTE, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_uchar(ncid, varid, uchar_out) != NC_ERANGE) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 1 || strcmp(var_name, VAR1_NAME) || var_type != NC_BYTE ||
          dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate the file. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_BYTE, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_uchar(ncid, varid, uchar_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate it, then make sure NOCLOBBER works. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_NOCLOBBER, &ncid) != NC_EEXIST) ERR;

      /* Recreate it again. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. Then define a new variable. Since it's
       * netcdf-4, nc_enddef isn't required - it's called
       * automatically. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_redef(ncid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR3_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate it again with netcdf-3 rules turned on. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. Check that netcdf-3 rules are in effect. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid) != NC_ENOTINDEFINE) ERR;
      if (nc_close(ncid)) ERR;

      /* Check some other stuff about it. Closing and reopening the
       * file forces HDF5 to tell us if we forgot to free some HDF5
       * resource associated with the file. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 1 || strcmp(var_name, VAR1_NAME) || var_type != NC_INT ||
          dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing more complex opens and creates...");
   {
      int ncid, varid, dimids[2];
      int ndims, nvars, natts, unlimdimid;
      int dimids_var[2], var_type;
      size_t dim_len;
      char dim_name[NC_MAX_NAME+1], var_name[NC_MAX_NAME+1];
      float float_in, float_out = 99.99f;
      int int_in, int_out = -9999;

      /* Create a file, this time with attributes. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid)) ERR;
      if (nc_put_att_float(ncid, NC_GLOBAL, ATT1_NAME, NC_FLOAT, 1, &float_out)) ERR;
      if (nc_put_att_int(ncid, NC_GLOBAL, ATT2_NAME, NC_INT, 1, &int_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen it and check each dim, var, and att. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_inq_dim(ncid, 1, dim_name, &dim_len)) ERR;
      if (dim_len != DIM2_LEN || strcmp(dim_name, DIM2_NAME)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 2 || strcmp(var_name, VAR1_NAME) || var_type != NC_INT ||
          dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_inq_var(ncid, 1, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 2 || strcmp(var_name, VAR2_NAME) || var_type != NC_UINT ||
          dimids_var[1] != dimids[1] || natts != 0) ERR;
      if (nc_get_att_float(ncid, NC_GLOBAL, ATT1_NAME, &float_in)) ERR;
      if (float_in != float_out) ERR;
      if (nc_get_att_int(ncid, NC_GLOBAL, ATT2_NAME, &int_in)) ERR;
      if (int_in != int_out) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing redef for netCDF classic...");
   test_redef(NC_FORMAT_CLASSIC);
   SUMMARIZE_ERR;
   printf("*** testing redef for netCDF 64-bit offset...");
   test_redef(NC_FORMAT_64BIT_OFFSET);
   SUMMARIZE_ERR;

   printf("*** testing redef for netCDF-4 ...");
   test_redef(NC_FORMAT_NETCDF4);
   SUMMARIZE_ERR;
   printf("*** testing redef for netCDF-4, with strict netCDF-3 rules...");
   test_redef(NC_FORMAT_NETCDF4_CLASSIC);
   SUMMARIZE_ERR;

#ifdef NETCDF_ENABLE_CDF5
   printf("*** testing redef for CDF5...");
   test_redef(NC_FORMAT_CDF5);
   SUMMARIZE_ERR;
#endif /* NETCDF_ENABLE_CDF5 */

   printf("*** testing different formats...");
   {
      int ncid;
      int format;

      /* Create a netcdf-3 file. */
      if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_CLASSIC) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a netcdf-3 64-bit offset file. */
      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_64BIT_OFFSET) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a netcdf-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_NETCDF4) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing CLASSIC_MODEL flag with classic formats...");
   {
      int ncid;
      int format;

      /* Create a netcdf-3 file. */
      if (nc_create(FILE_NAME, NC_CLOBBER|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_CLASSIC) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a netcdf-3 64-bit offset file. */
      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_CLOBBER|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_64BIT_OFFSET) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing multiple open files...");
   {
#define VAR_NAME "Captain_Kirk"
#define NDIMS 1
#define NUM_FILES 30
#define TEXT_LEN 15
#define D1_NAME "tl"
      int ncid[NUM_FILES], varid;
      int dimid;
      size_t count[NDIMS], index[NDIMS] = {0};
      const char ttext[TEXT_LEN + 1] = "20051224.150000";
      char ttext_in[TEXT_LEN + 1];
      char file_name[NC_MAX_NAME + 1];
      size_t chunks[NDIMS] = {TEXT_LEN + 1};
      int f;

      /* Create a bunch of files. */
      for (f = 0; f < NUM_FILES; f++)
      {
         snprintf(file_name, sizeof(file_name), "tst_files2_%d.nc", f);
         if (nc_create(file_name, NC_NETCDF4, &ncid[f])) ERR;
         if (nc_def_dim(ncid[f], D1_NAME, TEXT_LEN + 1, &dimid)) ERR;
         if (nc_def_var(ncid[f], VAR_NAME, NC_CHAR, NDIMS, &dimid, &varid)) ERR;
         if (f % 2 == 0)
            if (nc_def_var_chunking(ncid[f], varid, 0, chunks)) ERR;

         /* Write one time to the coordinate variable. */
         count[0] = TEXT_LEN + 1;
         if (nc_put_vara_text(ncid[f], varid, index, count, ttext)) ERR;
      }

      /* Read something from each file. */
      for (f = 0; f < NUM_FILES; f++)
      {
         if (nc_get_vara_text(ncid[f], varid, index, count, (char *)ttext_in)) ERR;
         if (strcmp(ttext_in, ttext)) ERR;
      }

      /* Close all open files. */
      for (f = 0; f < NUM_FILES; f++)
         if (nc_close(ncid[f])) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

#define REDEF_ATT1_NAME "CANTERBURY"
#define REDEF_ATT2_NAME "ELY"
#define REDEF_ATT3_NAME "KING_HENRY_V"
#define REDEF_DIM1_NAME "performance_length"
#define REDEF_DIM1_LEN 101
/* Change illegal name from '?' to '/'; the latter will
   more likely be illegal even when the switch is made to
   escaped characters in identifiers.
*/
#define REDEF_NAME_ILLEGAL "/"
#define REDEF_DIM2_NAME "ZZ_number_of_performers_or_perhaps_actors_or_maybe_I_should_say_players_or_one_could_call_them_artists_but_one_doesnt_like_to_get"

#define REDEF_DIM2_LEN 999
#define REDEF_VAR1_NAME "Royal_Shakespeare_Company_season_of_2004"
#define REDEF_VAR2_NAME "Ms_Beths_Kindergardern_class_of_2003"
#define REDEF_VAR3_NAME "Costumed_Mice_in_my_Garage_in_the_Winter_of_my_Discontent"
#define REDEF_NDIMS 2

#define NEW_CACHE_SIZE 32000000
#define NEW_CACHE_NELEMS 2000
#define NEW_CACHE_PREEMPTION .75
#define NEW_CACHE_SIZE_2 16000000
#define NEW_CACHE_NELEMS_2 1000
#define NEW_CACHE_PREEMPTION_2 .50

/* These prototypes are needed because these functions, used by the
 * Fortran API, are not prototyped in netcdf.h. */
int nc_get_chunk_cache_ints(int *sizep, int *nelemsp, int *preemptionp);
int nc_set_chunk_cache_ints(int size, int nelems, int preemption);

int
test_redef(int format)
{
   int ncid, varid, dimids[REDEF_NDIMS], dimids_in[REDEF_NDIMS];
   int ndims, nvars, natts, unlimdimid;
   int dimids_var[REDEF_NDIMS], var_type;
   int cflags = 0;
   size_t dim_len;
   char dim_name[NC_MAX_NAME+1], var_name[NC_MAX_NAME+1];
   float float_in;
   double double_out = 99E99;
   int int_in;
   unsigned char uchar_in, uchar_out = 255;
   short short_out = -999;
   nc_type xtype_in;
   size_t cache_size_in, cache_nelems_in;
   int cache_size_int_in, cache_nelems_int_in;
   int cache_preemption_int_in;
   float cache_preemption_in;
   int ret;

   /* Get and report default chunk cache values before modification. */
   if (nc_get_chunk_cache(&cache_size_in, &cache_nelems_in,
                          &cache_preemption_in)) ERR;
   printf("    default chunk cache: size=%zu nelems=%zu preemption=%f\n",
          cache_size_in, cache_nelems_in, cache_preemption_in);

   /* Report what netCDF thinks the default should be */
   printf("    netCDF default: size=%zu (expected 67108864 = 64MB)\n",
          cache_size_in);

#ifdef USE_HDF5
   /* Check HDF5 library default chunk cache settings by creating a DAPL. */
   {
       hid_t dapl_id;
       size_t hdf5_cache_size = 0;
       size_t hdf5_nelems = 0;
       double hdf5_preemption = 0;
       herr_t status;

       dapl_id = H5Pcreate(H5P_DATASET_ACCESS);
       if (dapl_id >= 0) {
           status = H5Pget_chunk_cache(dapl_id, &hdf5_nelems,
                                       &hdf5_cache_size, &hdf5_preemption);
           if (status >= 0) {
               printf("    HDF5 library defaults: nelems=%zu size=%zu preemption=%f\n",
                      hdf5_nelems, hdf5_cache_size, hdf5_preemption);
           }
           H5Pclose(dapl_id);
       }
   }

   /* For first netCDF-4 test (before cache is modified), verify file uses 64MB */
   if (format == NC_FORMAT_NETCDF4 && cache_size_in == 67108864) {
       int ncid_test, dimid_test, varid_test;
       hid_t h5_file_id, h5_var_id, h5_dapl;
       size_t h5_cache_size, h5_nelems;
       double h5_preemption;
       herr_t status;

       printf("    Verifying netCDF file uses 64MB chunk cache...\n");

       /* Create netCDF-4 file with chunked variable */
       if (nc_create("tst_cache_verify.nc", NC_NETCDF4 | NC_CLOBBER,
                     &ncid_test)) ERR;
       if (nc_def_dim(ncid_test, "d1", 1000, &dimid_test)) ERR;
       if (nc_def_var(ncid_test, "v1", NC_INT, 1, &dimid_test,
                      &varid_test)) ERR;
       {
           size_t chunksizes[1] = {100};
           if (nc_def_var_chunking(ncid_test, varid_test, NC_CHUNKED, chunksizes)) ERR;
       }
       if (nc_enddef(ncid_test)) ERR;

       /* Check var chunk cache before close */
       {
           size_t var_cache_size, var_cache_nelems;
           float var_cache_preemption;
           if (nc_get_var_chunk_cache(ncid_test, varid_test, &var_cache_size,
                                      &var_cache_nelems, &var_cache_preemption)) ERR;
           printf("    netCDF var cache before close: size=%zu nelems=%zu preemption=%f\n",
                  var_cache_size, var_cache_nelems, var_cache_preemption);
       }

       if (nc_close(ncid_test)) ERR;

       /* Reopen with HDF5 and check actual chunk cache */
       h5_file_id = H5Fopen("tst_cache_verify.nc", H5F_ACC_RDONLY, H5P_DEFAULT);
       if (h5_file_id >= 0) {
           h5_var_id = H5Dopen2(h5_file_id, "/v1", H5P_DEFAULT);
           if (h5_var_id >= 0) {
               h5_dapl = H5Dget_access_plist(h5_var_id);
               if (h5_dapl >= 0) {
                   status = H5Pget_chunk_cache(h5_dapl, &h5_nelems,
                                               &h5_cache_size, &h5_preemption);
                   if (status >= 0) {
                       printf("    HDF5 actual file cache: nelems=%zu size=%zu preemption=%f\n",
                              h5_nelems, h5_cache_size, h5_preemption);
                       if (h5_cache_size == 67108864) {
                           printf("    SUCCESS: file uses 64MB chunk cache\n");
                       } else {
                           printf("    UNEXPECTED: file cache %zu != 67108864 (64MB)\n",
                                  h5_cache_size);
                       }
                   }
                   H5Pclose(h5_dapl);
               }
               H5Dclose(h5_var_id);
           }
           H5Fclose(h5_file_id);
       }
       remove("tst_cache_verify.nc");
   }
#else
   printf("    HDF5: USE_HDF5 not defined\n");
#endif

   if (format == NC_FORMAT_64BIT_OFFSET)
      cflags |= NC_64BIT_OFFSET;
   else if (format == NC_FORMAT_CDF5)
      cflags |= NC_CDF5;
   else if (format == NC_FORMAT_NETCDF4_CLASSIC)
      cflags |= (NC_NETCDF4|NC_CLASSIC_MODEL);
   else if (format == NC_FORMAT_NETCDF4)
      cflags |= NC_NETCDF4;

   /* Change chunk cache. */
   if (nc_set_chunk_cache(NEW_CACHE_SIZE, NEW_CACHE_NELEMS,
                          NEW_CACHE_PREEMPTION)) ERR;

   /* Create a file with two dims, two vars, and two atts. */
   if (nc_create(FILE_NAME, cflags|NC_CLOBBER, &ncid)) ERR;

   /* Retrieve the chunk cache settings, just for fun. */
   if (nc_get_chunk_cache(&cache_size_in, &cache_nelems_in,
                          &cache_preemption_in)) ERR;
   if (cache_size_in != NEW_CACHE_SIZE || cache_nelems_in != NEW_CACHE_NELEMS ||
       cache_preemption_in != NEW_CACHE_PREEMPTION) ERR;
   cache_size_in = 0;
   if (nc_get_chunk_cache(&cache_size_in, NULL, NULL)) ERR;
   if (cache_size_in != NEW_CACHE_SIZE) ERR;
   cache_nelems_in = 0;
   if (nc_get_chunk_cache(NULL, &cache_nelems_in, NULL)) ERR;
   if (cache_nelems_in != NEW_CACHE_NELEMS) ERR;
   cache_preemption_in = 0;
   if (nc_get_chunk_cache(NULL, NULL, &cache_preemption_in)) ERR;
   if (cache_preemption_in != NEW_CACHE_PREEMPTION) ERR;

   /* Retrieve the chunk cache settings as integers, like the fortran API. */
   if (nc_get_chunk_cache_ints(&cache_size_int_in, &cache_nelems_int_in,
                               &cache_preemption_int_in)) ERR;
   if (cache_size_int_in != NEW_CACHE_SIZE || cache_nelems_int_in != NEW_CACHE_NELEMS ||
       cache_preemption_int_in != (int)(NEW_CACHE_PREEMPTION * 100)) ERR;
   if (nc_get_chunk_cache_ints(NULL, NULL, NULL)) ERR;
   cache_size_int_in = 0;
   if (nc_get_chunk_cache_ints(&cache_size_int_in, NULL, NULL)) ERR;
   if (cache_size_int_in != NEW_CACHE_SIZE) ERR;
   cache_nelems_int_in = 0;
   if (nc_get_chunk_cache_ints(NULL, &cache_nelems_int_in, NULL)) ERR;
   if (cache_nelems_int_in != NEW_CACHE_NELEMS) ERR;
   cache_preemption_int_in = 0;
   if (nc_get_chunk_cache_ints(NULL, NULL, &cache_preemption_int_in)) ERR;
   if (cache_preemption_int_in != (int)(NEW_CACHE_PREEMPTION * 100)) ERR;

   /* These won't work. */
   if (nc_set_chunk_cache_ints(-1, NEW_CACHE_NELEMS_2,
                               (int)(NEW_CACHE_PREEMPTION_2 * 100)) != NC_EINVAL) ERR;
   if (nc_set_chunk_cache_ints(NEW_CACHE_SIZE_2, 0,
                               (int)(NEW_CACHE_PREEMPTION_2 * 100)) != NC_EINVAL) ERR;
   if (nc_set_chunk_cache_ints(NEW_CACHE_SIZE_2, NEW_CACHE_NELEMS_2,
                               -1) != NC_EINVAL) ERR;
   if (nc_set_chunk_cache_ints(NEW_CACHE_SIZE_2, NEW_CACHE_NELEMS_2,
                               101) != NC_EINVAL) ERR;


   /* Change chunk cache again. */
   if (nc_set_chunk_cache_ints(NEW_CACHE_SIZE_2, NEW_CACHE_NELEMS_2,
                               (int)(NEW_CACHE_PREEMPTION_2 * 100))) ERR;
   if (nc_get_chunk_cache_ints(&cache_size_int_in, &cache_nelems_int_in,
                               &cache_preemption_int_in)) ERR;
   if (cache_size_int_in != NEW_CACHE_SIZE_2 || cache_nelems_int_in != NEW_CACHE_NELEMS_2 ||
       cache_preemption_int_in != (int)(NEW_CACHE_PREEMPTION_2 * 100)) ERR;


   /* This will fail, except for netcdf-4/hdf5, which permits any
    * name. */
   if (format != NC_FORMAT_NETCDF4)
      if (nc_def_dim(ncid, REDEF_NAME_ILLEGAL, REDEF_DIM2_LEN,
                     &dimids[1]) != NC_EBADNAME) ERR;

   if (nc_def_dim(ncid, REDEF_DIM1_NAME, REDEF_DIM1_LEN, &dimids[0])) ERR;
   if (nc_def_dim(ncid, REDEF_DIM2_NAME, REDEF_DIM2_LEN, &dimids[1])) ERR;
   if (nc_def_var(ncid, REDEF_VAR1_NAME, NC_INT, REDEF_NDIMS, dimids, &varid)) ERR;
   if (nc_def_var(ncid, REDEF_VAR2_NAME, NC_BYTE, REDEF_NDIMS, dimids, &varid)) ERR;
   if (nc_put_att_double(ncid, NC_GLOBAL, REDEF_ATT1_NAME, NC_DOUBLE, 1, &double_out)) ERR;
   if (nc_put_att_short(ncid, NC_GLOBAL, REDEF_ATT2_NAME, NC_SHORT, 1, &short_out)) ERR;

   /* Check it out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;

   /* Try enddef/redef. */
   if (nc_enddef(ncid)) ERR;
   if (nc_redef(ncid)) ERR;

   /* NetCDF/HDF5 files ignore the repeated redef(). */
   if (format != NC_FORMAT_NETCDF4)
   {
       if (nc_redef(ncid) != NC_EINDEFINE) ERR;
   }
   else
   {
       if (nc_redef(ncid)) ERR;
   }
   
   /* Close it up. */
   if (format != NC_FORMAT_NETCDF4)
      if (nc_enddef(ncid)) ERR;
   if (nc_close(ncid)) ERR;

   /* Reopen as read only - make sure it doesn't let us change file. */
   if (nc_open(FILE_NAME, 0, &ncid)) ERR;
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;

   /* This will fail. */
   ret = nc_def_var(ncid, REDEF_VAR3_NAME, NC_UBYTE, REDEF_NDIMS,
                    dimids, &varid);
   if(format == NC_FORMAT_NETCDF4) {
      if(ret != NC_EPERM) {
         ERR;
      }
   } else {
      if(ret != NC_ENOTINDEFINE) {
         ERR;
      }
   }
   /* This will fail. */
   if (!nc_put_att_uchar(ncid, NC_GLOBAL, REDEF_ATT3_NAME, NC_CHAR, 1, &uchar_out)) ERR;
   if (nc_close(ncid)) ERR;

   /* Make sure we can't redef a file opened for NOWRITE. */
   if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
   if (nc_redef(ncid) != NC_EPERM) ERR;

   /* Check it out again. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;

   if (nc_close(ncid)) ERR;

   /* Reopen the file and check it, add a variable and attribute. */
   if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

   /* Check it out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;

   /* Add var. */
   if ((format != NC_FORMAT_NETCDF4) && nc_redef(ncid)) ERR;
   if (nc_def_var(ncid, REDEF_VAR3_NAME, NC_BYTE, REDEF_NDIMS, dimids, &varid)) ERR;

   /* Add att. */
   ret = nc_put_att_uchar(ncid, NC_GLOBAL, REDEF_ATT3_NAME, NC_BYTE, 1, &uchar_out);
   if (format == NC_FORMAT_NETCDF4 || format == NC_FORMAT_64BIT_DATA)
   {
      if (ret != NC_ERANGE) ERR;
   }
   else if (ret) ERR;

   /* Check it out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 3 || natts != 3 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS ||
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 2, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR3_NAME) || var_type != NC_BYTE ||
       natts != 0) ERR;

   if (nc_close(ncid)) ERR;

   /* Reopen it and check each dim, var, and att. */
   if (nc_open(FILE_NAME, 0, &ncid)) ERR;
   if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
   if (dim_len != REDEF_DIM1_LEN || strcmp(dim_name, REDEF_DIM1_NAME)) ERR;
   if (nc_inq_dim(ncid, 1, dim_name, &dim_len)) ERR;
   if (dim_len != REDEF_DIM2_LEN || strcmp(dim_name, REDEF_DIM2_NAME)) ERR;
   if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR1_NAME) || var_type != NC_INT ||
       natts != 0) ERR;
   if (nc_inq_var(ncid, 1, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR2_NAME) || var_type != NC_BYTE ||
       natts != 0) ERR;
   if (nc_inq_var(ncid, 2, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR3_NAME) || var_type != NC_BYTE ||
       natts != 0) ERR;
   if (nc_get_att_float(ncid, NC_GLOBAL, REDEF_ATT1_NAME, &float_in) != NC_ERANGE) ERR;
   if (nc_get_att_int(ncid, NC_GLOBAL, REDEF_ATT2_NAME, &int_in)) ERR;
   if (int_in != short_out) ERR;
   ret = nc_get_att_uchar(ncid, NC_GLOBAL, REDEF_ATT3_NAME, &uchar_in);
   if (format == NC_FORMAT_NETCDF4 || format == NC_FORMAT_64BIT_DATA)
   {
      if (ret != NC_ERANGE) ERR;
   }
   else if (ret) ERR;

   if (uchar_in != uchar_out) ERR;
   if (nc_close(ncid)) ERR;
   return NC_NOERR;
}
