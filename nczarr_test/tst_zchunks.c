/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables.
   Ed Hartnett, Dennis Heimbigner
   modified version of nc_test4/tst_chunks.c
*/

#include "ut_includes.h"
#include "test_nczarr_utils.h"

#define DEBUGNOFILL
#undef PRINT_DEFAULT_CHUNKSIZE_TABLE

#undef FILTERS

#define FILE_NAME "tst_chunks"
#define NDIMS1 1
#define D_SMALL "small_dim"
#define D_SMALL_LEN 16
#define D_MEDIUM "medium_dim"
#define D_MEDIUM_LEN 65546
#define D_LARGE "large_dim"
#define D_LARGE_LEN 1048586
#define V_SMALL "small_var"
#define V_MEDIUM "medium_var"
#define V_LARGE "large_var"

int
main(int argc, char **argv)
{
   processoptions(argc,argv,FILE_NAME);

   printf("\n*** Testing netcdf-4 variable chunking.\n");
   printf("**** testing that fixed vars with forced chunking end up being chunked, with good sizes...");
   {

      int ncid;
      int nvars, ndims, ngatts, unlimdimid;
      int storage;
      int ndims_in, natts_in, dimids_in;
      int small_dimid, medium_dimid, large_dimid;
      int small_varid, medium_varid, large_varid;
      char var_name_in[NC_MAX_NAME + 1];
      size_t chunksize_in[NDIMS1];
      nc_type xtype_in;

      /* Create a netcdf-4 file with three dimensions. */
      if (nc_create(itoptions.path, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D_SMALL, D_SMALL_LEN, &small_dimid)) ERR;
      if (nc_def_dim(ncid, D_MEDIUM, D_MEDIUM_LEN, &medium_dimid)) ERR;
      if (nc_def_dim(ncid, D_LARGE, D_LARGE_LEN, &large_dimid)) ERR;

      /* Add three vars, with forced chunking. */
      if (nc_def_var(ncid, V_SMALL, NC_INT64, NDIMS1, &small_dimid, &small_varid)) ERR;
      if (nc_def_var(ncid, V_MEDIUM, NC_INT64, NDIMS1, &medium_dimid, &medium_varid)) ERR;
      if (nc_def_var(ncid, V_LARGE, NC_INT64, NDIMS1, &large_dimid, &large_varid)) ERR;
#ifdef FILTERS
      if (nc_def_var_deflate(ncid, small_varid, 0, 1, 4)) ERR;
      if (nc_def_var_deflate(ncid, medium_varid, 1, 0, 0)) ERR;
      if (nc_def_var_fletcher32(ncid, large_varid, 1)) ERR;
#else
      if(nc_def_var_chunking(ncid,small_varid,NC_CHUNKED,NULL)) ERR;
      if(nc_def_var_chunking(ncid,medium_varid,NC_CHUNKED,NULL)) ERR;
      if(nc_def_var_chunking(ncid,large_varid,NC_CHUNKED,NULL)) ERR;
#endif
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(itoptions.path, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 3 || ndims != 3 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, &dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, V_SMALL) || xtype_in != NC_INT64 || ndims_in != 1 ||
	  natts_in != 0) ERR;

      /* Make sure chunking sizes are what we expect. */
      if (nc_inq_var_chunking(ncid, small_varid, &storage, chunksize_in)) ERR;
      if (storage || chunksize_in[0] != D_SMALL_LEN) ERR;
      if (nc_inq_var_chunking(ncid, medium_varid, &storage, chunksize_in)) ERR;
      if (storage || chunksize_in[0] * sizeof(long long) > DEFAULT_CHUNK_SIZE) ERR;
      if (nc_inq_var_chunking(ncid, large_varid, &storage, chunksize_in)) ERR;
      if (storage || chunksize_in[0] * sizeof(long long) > DEFAULT_CHUNK_SIZE) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("**** testing default chunksizes...");
   {
      int nvars, ndims, ngatts, unlimdimid;
      int storage;
#define NUM_DIM 4
#define NUM_TYPE 2
      int ncid;
      int dim_len[NUM_DIM] = {1, 100, 1000, 2000};
//      int dim_len[NUM_DIM] = {1, 50, 100, 200};
      size_t chunksize_in[NUM_DIM];
      int type_id[NUM_TYPE] = {NC_BYTE, NC_INT};
      int dimid[NUM_DIM], varid[NUM_TYPE];
      char dim_name[NC_MAX_NAME + 1], var_name[NC_MAX_NAME + 1];
      int d, t;

      /* Create a netcdf-4 file with NUM_DIM dimensions. */
      if (nc_create(itoptions.path, NC_NETCDF4, &ncid)) ERR;
#ifdef DEBUGNOFILL
	 if(nc_set_fill(ncid,NC_NOFILL,&d)) ERR;
#endif

      for (d = 0; d < NUM_DIM; d++)
      {
	 sprintf(dim_name, "dim_%d", dim_len[d]);
#ifdef PRINT_DEFAULT_CHUNKSIZE_TABLE
	 printf("creating dim[%d] %s = %d\n", d,  dim_name, dim_len[d]);
#endif
	 if (nc_def_dim(ncid, dim_name, dim_len[d], &dimid[d])) ERR;
      }

      for (t = 0; t < NUM_TYPE; t++)
      {
	 sprintf(var_name, "var_%d", type_id[t]);
	 if (nc_def_var(ncid, var_name, type_id[t], NUM_DIM, dimid, &varid[t])) ERR;
	 if (nc_inq_var_chunking(ncid, varid[t], &storage, chunksize_in)) ERR;
#ifdef PRINT_DEFAULT_CHUNKSIZE_TABLE
	 printf("chunksizes for %d x %d x %d x %d var %s: %d x %d x %d x %d (=%d)\n",
		dim_len[0], dim_len[1], dim_len[2], dim_len[3],
		var_name,
		(int)chunksize_in[0], (int)chunksize_in[1], (int)chunksize_in[2],
		(int)chunksize_in[3],
		(int)(chunksize_in[0] * chunksize_in[1] * chunksize_in[2] * chunksize_in[3]));
#endif
      }

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(itoptions.path, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_TYPE || ndims != NUM_DIM || ngatts != 0 || unlimdimid == 0) ERR;


      for (t = 0; t < NUM_TYPE; t++)
      {
	 sprintf(var_name, "var_%d", type_id[t]);
	 if (nc_inq_var_chunking(ncid, varid[t], &storage, chunksize_in)) ERR;
	 if (storage) ERR;
#ifdef PRINT_DEFAULT_CHUNKSIZE_TABLE
	 printf("chunksizes for %d x %d x %d x %d var: %d x %d x %d x %d (=%d)\n",
		dim_len[0], dim_len[1], dim_len[2], dim_len[3],
		(int)chunksize_in[0], (int)chunksize_in[1], (int)chunksize_in[2],
		(int)chunksize_in[3],
		(int)(chunksize_in[0] * chunksize_in[1] * chunksize_in[2] * chunksize_in[3]));
#endif
      }

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("**** testing that chunking works on classic mode files...");
   {
#define D_SMALL_LEN2 66
      int ncid;
      int nvars, ndims, ngatts, unlimdimid;
      int storage;
      int ndims_in, natts_in, dimids_in;
      int small_dimid, medium_dimid, large_dimid;
      int small_varid, medium_varid, large_varid;
      char var_name_in[NC_MAX_NAME + 1];
      size_t chunks[1], chunksize_in;
      nc_type xtype_in;

      /* Create a netcdf-4 file with three dimensions. */
      if (nc_create(itoptions.path, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D_SMALL, D_SMALL_LEN2, &small_dimid)) ERR;
      if (nc_def_dim(ncid, D_MEDIUM, D_MEDIUM_LEN, &medium_dimid)) ERR;
      if (nc_def_dim(ncid, D_LARGE, D_LARGE_LEN, &large_dimid)) ERR;

      /* Add three vars. */
      if (nc_def_var(ncid, V_SMALL, NC_INT64, NDIMS1, &small_dimid, &small_varid)) ERR;
      if (nc_def_var_chunking(ncid, small_varid, 1, NULL)) ERR;

      if (nc_def_var(ncid, V_MEDIUM, NC_INT64, NDIMS1, &medium_dimid, &medium_varid)) ERR;
      chunks[0] = D_MEDIUM_LEN / 100;
      if (nc_def_var_chunking(ncid, medium_varid, 0, chunks)) ERR;
#ifdef FILTERS
      if (nc_def_var_deflate(ncid, medium_varid, 1, 0, 0)) ERR;
#endif
      if (nc_def_var(ncid, V_LARGE, NC_INT64, NDIMS1, &large_dimid, &large_varid)) ERR;
      chunks[0] = D_LARGE_LEN / 1000;
      if (nc_def_var_chunking(ncid, large_varid, 0, chunks)) ERR;
#ifdef FILTERS
      if (nc_def_var_fletcher32(ncid, large_varid, 1)) ERR;
#endif
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(itoptions.path, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 3 || ndims != 3 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, &dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, V_SMALL) || xtype_in != NC_INT64 || ndims_in != 1 ||
	  natts_in != 0) ERR;

      /* Make sure chunking settings are what we expect. */
      if (nc_inq_var_chunking(ncid, small_varid, &storage, &chunksize_in)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (nc_inq_var_chunking(ncid, medium_varid, &storage, &chunksize_in)) ERR;
      if (storage || chunksize_in != D_MEDIUM_LEN / 100) ERR;
      if (nc_inq_var_chunking(ncid, large_varid, &storage, &chunksize_in)) ERR;
      if (storage || chunksize_in != D_LARGE_LEN / 1000) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing many chunking variables...");
   {
#define NDIMS_3 3
#define NUM_PLANS 30
#define D_SNEAKINESS "sneakiness"
#define D_SNEAKINESS_LEN 5
#define D_CLEVERNESS "clevernesss"
#define D_CLEVERNESS_LEN 3
#define D_EFFECTIVENESS "effectiveness"
#define D_EFFECTIVENESS_LEN 2

      int ncid, dimids[NDIMS_3], varid[NUM_PLANS];
      size_t chunksize[NDIMS_3] = {D_SNEAKINESS_LEN, D_CLEVERNESS_LEN,
				   D_EFFECTIVENESS_LEN};
      char plan_name[NC_MAX_NAME + 1];
      int storage;
      size_t chunksize_in[NDIMS_3];
      int i, j;

      /* Create a netcdf-4 file with three dimensions. */
      if (nc_create(itoptions.path, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D_SNEAKINESS, D_SNEAKINESS_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, D_CLEVERNESS, D_CLEVERNESS_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, D_EFFECTIVENESS, D_EFFECTIVENESS_LEN, &dimids[2])) ERR;

      /* Oh that tricky Cardinal Richelieu, he had many plans! */
      for (i = 0; i < NUM_PLANS; i++)
      {
	 sprintf(plan_name, "Richelieu_sneaky_plan_%d", i);
	 if (nc_def_var(ncid, plan_name, i % (NC_STRING - 1) + 1, NDIMS_3,
			dimids, &varid[i])) ERR;
	 if (nc_def_var_chunking(ncid, varid[i], 0, chunksize)) ERR;
      }

      /* Check the chunking. */
      for (i = 0; i < NUM_PLANS; i++)
      {
	 if (nc_inq_var_chunking(ncid, varid[i], &storage, chunksize_in)) ERR;
	 {
	    for (j = 0; j < NDIMS_3; j++)
	       if (chunksize_in[j] != chunksize[j]) ERR;
	 }
      }
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(itoptions.path, NC_WRITE, &ncid)) ERR;
      /* Check the chunking. */
      for (i = 0; i < NUM_PLANS; i++)
      {
	 if (nc_inq_var_chunking(ncid, varid[i], &storage, chunksize_in)) ERR;
	 {
	    for (j = 0; j < NDIMS_3; j++)
	       if (chunksize_in[j] != chunksize[j]) ERR;
	 }
      }
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing that too large chunksizes fail...");
   {
#define D_SMALL_LEN2 66
      int stat = NC_NOERR;
      int ncid;
      int small_dimid;
      int small_varid;
      size_t chunks[1];

      /* Create a netcdf-4 file with three dimensions. */
      if (nc_create(itoptions.path, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D_SMALL, D_SMALL_LEN2, &small_dimid)) ERR;

      /* Add one var. */
      if (nc_def_var(ncid, V_SMALL, NC_INT64, NDIMS1, &small_dimid, &small_varid)) ERR;

      /* Attempt to set too large chunksizes */
      chunks[0] = D_SMALL_LEN2 + 1;
      stat = nc_def_var_chunking(ncid, small_varid, NC_CHUNKED, chunks);
      if(stat != NC_EBADCHUNK) {
	printf("Return code is '%s', expected NC_BADCHUNK",nc_strerror(stat));
	ERR;
      }
      /* try again with proper chunksize */
      chunks[0] = D_SMALL_LEN2;
      stat = nc_def_var_chunking(ncid, small_varid, NC_CHUNKED, chunks);
      if(stat != NC_NOERR) {
	printf("Return code is '%s', expected NC_NOERR",nc_strerror(stat));
	ERR;
      }
      if (nc_abort(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing cache size smaller than chunk size...");
   {
#define NDIM2 2
#define DIM_X_LEN 10000
#define DIM_Y_LEN 10000
#define DIM_NAME_X_CACHE_CHUNK "Height"
#define DIM_NAME_Y_CACHE_CHUNK "Width"
#define VAR_NAME_CACHE_CHUNK "House_Size"
#define VAR_NAME_CACHE_CHUNK_2 "Boat_Size"
#define VAR_NAME_CACHE_CHUNK_3 "Deck_Size"

      int ncid;
      int dimid[NDIM2];
      int varid, varid2, varid3;
      size_t chunks[NDIM2] = {100, 100};
      size_t chunks_big[NDIM2] = {DIM_X_LEN, DIM_Y_LEN};
      size_t chunks_in[NDIM2];
      int storage;
      size_t cache_size = 16;
      size_t cache_nelems = 1;
      float cache_preemption = 0.5;
      size_t cache_size_in;
      size_t cache_nelems_in;
      float cache_preemption_in;

      /* Create a netcdf-4 file with two dimensions. */
      if (nc_create(itoptions.path, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME_X_CACHE_CHUNK, DIM_X_LEN, &dimid[0])) ERR;
      if (nc_def_dim(ncid, DIM_NAME_Y_CACHE_CHUNK, DIM_Y_LEN, &dimid[1])) ERR;

      /* Add vars. */
      if (nc_def_var(ncid, VAR_NAME_CACHE_CHUNK, NC_INT64, NDIM2, dimid, &varid)) ERR;
      if (nc_def_var(ncid, VAR_NAME_CACHE_CHUNK_2, NC_INT64, NDIM2, dimid, &varid2)) ERR;
      if (nc_def_var(ncid, VAR_NAME_CACHE_CHUNK_3, NC_INT64, NDIM2, dimid, &varid3)) ERR;

      /* Set the var cache to something arbitrary but small */
      if (nc_set_var_chunk_cache(ncid, varid, cache_size, cache_nelems,
                                 cache_preemption)) ERR;

      /* Set the chunking. */
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
      if (nc_inq_var_chunking(ncid, varid, &storage, chunks_in)) ERR;
      if (storage || chunks_in[0] != chunks[0] || chunks_in[1] != chunks[1]) ERR;
      if (nc_def_var_chunking(ncid, varid2, NC_CHUNKED, chunks)) ERR;
      if (nc_inq_var_chunking(ncid, varid2, &storage, chunks_in)) ERR;
      if (storage || chunks_in[0] != chunks[0] || chunks_in[1] != chunks[1]) ERR;
      if (nc_def_var_chunking(ncid, varid3, NC_CHUNKED, chunks_big)) ERR;
      if (nc_inq_var_chunking(ncid, varid3, &storage, chunks_in)) ERR;
      if (storage || chunks_in[0] != chunks_big[0] || chunks_in[1] != chunks_big[1]) ERR;

      /* Get the var cache values. */
      if (nc_get_var_chunk_cache(ncid, varid, &cache_size_in, &cache_nelems_in,
                                 &cache_preemption_in)) ERR;
      if (cache_size_in != cache_size || cache_nelems_in != cache_nelems ||
          cache_preemption_in != cache_preemption) ERR;
      if (nc_get_var_chunk_cache(ncid, varid2, &cache_size_in, &cache_nelems_in,
                                 &cache_preemption_in)) ERR;
      if (cache_size_in != CHUNK_CACHE_SIZE_NCZARR) ERR;

#if 0
      /* Inapplicable to zarr */
      /* The cache_size has been increased due to larger chunksizes
       * for varid3. */
      if (nc_get_var_chunk_cache(ncid, varid3, &cache_size_in, &cache_nelems_in,
                                 &cache_preemption_in)) ERR;
      if (cache_nelems_in != CHUNK_CACHE_NELEMS ||
          cache_preemption_in != CHUNK_CACHE_PREEMPTION) ERR;
      /* printf("cache_size_in %ld\n", cache_size_in); */
#endif

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Reopen the file. */
      if (nc_open(itoptions.path, NC_NOWRITE, &ncid)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
   }
   clearoptions();
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
