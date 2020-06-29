/*
   Copyright 2020, UCAR/Unidata See COPYRIGHT file for copying and
   redistribution conditions.

   This program tests netcdf-4 parallel I/O using the same access
   pattern as is used by NOAA's GFS when writing and reading model
   data.

   Ed Hartnett, 6/28/20
*/

#include <nc_tests.h>
#include "err_macros.h"
#include <mpi.h>

#define FILE_NAME "tst_gfs_data_1.nc"
#define NDIM4 4
#define NDIM5 5
#define QTR_DATA (DIMSIZE * DIMSIZE / 4)
#define NUM_PROC 4
#define NUM_SLABS 10
#define NUM_SHUFFLE_SETTINGS 1
/* #define NUM_SHUFFLE_SETTINGS 2 */
#ifdef HAVE_H5Z_SZIP
#define NUM_COMPRESSION_FILTERS 1
/* #define NUM_COMPRESSION_FILTERS 2 */
#else
#define NUM_COMPRESSION_FILTERS 1
#endif

int
main(int argc, char **argv)
{
    /* MPI stuff. */
    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    int ncid;
    size_t start[NDIM4], count[NDIM4];

    /* Dimensions. */
    char dim_name[NDIM5][NC_MAX_NAME + 1] = {"grid_xt", "grid_yt", "pfull", "phalf", "time"};
    int dim_len[NDIM5] = {3072, 1536, 127, 128, 1};
    int dimid[NDIM5];
    int dimid_data[NDIM4];

    /* Variables. */
#define NUM_VARS 8
    char var_name[NUM_VARS][NC_MAX_NAME + 1] = {"grid_xt", "lon", "grid_yt", "lat", "pfull", "phalf", "time", "clwmr"};
    int varid[NUM_VARS];
    int var_type[NUM_VARS] = {NC_DOUBLE, NC_DOUBLE, NC_DOUBLE, NC_DOUBLE, NC_FLOAT, NC_FLOAT, NC_DOUBLE, NC_FLOAT};
    /* integer :: var_ndims(NUM_VARS) = (/ 1, 2, 1, 2, 1, 1, 1, 4 /) */
    /* integer :: ideflate = 4 */
    double value_time = 2.0;
    /* float value_time_in; */
    /* real, allocatable :: value_clwmr(:,:,:,:) */
    size_t pfull_loc_size, pfull_start;
    float *value_pfull_loc, *value_pfull_loc_in;
    size_t phalf_loc_size, phalf_start;
    float *value_phalf_loc, *value_phalf_loc_in;
    size_t grid_xt_loc_size, grid_xt_start;
    double *value_grid_xt_loc, *value_grid_xt_loc_in;
    size_t grid_yt_loc_size, grid_yt_start;
    double *value_grid_yt_loc, *value_grid_yt_loc_in;
    size_t lon_xt_loc_size, lon_xt_start, lon_yt_loc_size, lon_yt_start;
    double *value_lon_loc, *value_lon_loc_in;
    size_t lat_xt_loc_size, lat_xt_start, lat_yt_loc_size, lat_yt_start;
    double *value_lat_loc, *value_lat_loc_in;
    float *value_clwmr_loc, *value_clwmr_loc_in;

    int f;
    int i, j, k;
    int res;
    /* int *slab_data; /\* one slab *\/ */

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    /* Size of local (i.e. for this pe) grid_xt data. */
    grid_xt_loc_size = dim_len[0]/mpi_size;
    grid_xt_start = mpi_rank * grid_xt_loc_size;
    if (mpi_rank == mpi_size - 1)
	grid_xt_loc_size = grid_xt_loc_size + dim_len[0] % mpi_size;
    /* !print *, mpi_rank, 'grid_xt', dim_len(3), grid_xt_start, grid_xt_loc_size */

    /* Size of local (i.e. for this pe) grid_yt data. */
    grid_yt_loc_size = dim_len[1]/mpi_size;
    grid_yt_start = mpi_rank * grid_yt_loc_size;
    if (mpi_rank == mpi_size - 1) 
	grid_yt_loc_size = grid_yt_loc_size + dim_len[1] % mpi_size;
    /* !print *, mpi_rank, 'grid_yt', dim_len(3), grid_yt_start, grid_yt_loc_size */

    /* Size of local (i.e. for this pe) pfull data. */
    pfull_loc_size = dim_len[2]/mpi_size;
    pfull_start = mpi_rank * pfull_loc_size;
    if (mpi_rank == mpi_size - 1) 
	pfull_loc_size = pfull_loc_size + dim_len[2] % mpi_size;
    /* !print *, mpi_rank, 'pfull', dim_len(3), pfull_start, pfull_loc_size */
  
    /* Size of local (i.e. for this pe) phalf data. */
    phalf_loc_size = dim_len[3]/mpi_size;
    phalf_start = mpi_rank * phalf_loc_size;
    if (mpi_rank == mpi_size - 1)
	phalf_loc_size = phalf_loc_size + dim_len[3] % mpi_size;
    /* !print *, mpi_rank, 'phalf', dim_len(4), phalf_start, phalf_loc_size */

    /* Size of local arrays (i.e. for this pe) lon and lat data. This is */
    /* specific to 4 pes. */
    lon_xt_loc_size = 1536;
    lat_xt_loc_size = 1536;
    if (mpi_rank == 0 || mpi_rank == 2)
    {
	lon_xt_start = 0;
	lat_xt_start = 0;
    }
    else
    {
	lon_xt_start = 1536;
	lat_xt_start = 1536;
    }
    lon_yt_loc_size = 768;
    lat_yt_loc_size = 768;
    if (mpi_rank == 0 || mpi_rank == 1)
    {
	lon_yt_start = 0;
	lat_yt_start = 0;
    }
    else
    {
	lon_yt_start = 768;
	lat_yt_start = 768;
    }
    /* !  print *, mpi_rank, 'lon_xt_start', lon_xt_start, 'lon_yt_start', lon_yt_start */
    /* !  print *, mpi_rank, 'lon_xt_loc_size', lon_xt_loc_size, 'lon_yt_loc_size', lon_yt_loc_size */

    /* ! Allocate space on this pe to hold the data for this pe. */
    if (!(value_pfull_loc = malloc(pfull_loc_size * sizeof(float)))) ERR;
    if (!(value_pfull_loc_in = malloc(pfull_loc_size * sizeof(float)))) ERR;
    if (!(value_phalf_loc = malloc(phalf_loc_size * sizeof(float)))) ERR;
    if (!(value_phalf_loc_in = malloc(phalf_loc_size * sizeof(float)))) ERR;
    if (!(value_grid_xt_loc = malloc(grid_xt_loc_size * sizeof(double)))) ERR;
    if (!(value_grid_xt_loc_in = malloc(grid_xt_loc_size * sizeof(double)))) ERR;
    if (!(value_grid_yt_loc = malloc(grid_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_grid_yt_loc_in = malloc(grid_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_lon_loc = malloc(lon_xt_loc_size * lon_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_lon_loc_in = malloc(lon_xt_loc_size * lon_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_lat_loc = malloc(lat_xt_loc_size * lat_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_lat_loc_in = malloc(lat_xt_loc_size * lat_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_clwmr_loc = malloc(lat_xt_loc_size * lat_yt_loc_size * pfull_loc_size * sizeof(float)))) ERR;
    if (!(value_clwmr_loc_in = malloc(lat_xt_loc_size * lat_yt_loc_size * pfull_loc_size * sizeof(float)))) ERR;
  
    /* Some fake data for this pe to write. */
    for (i = 0; i < pfull_loc_size; i++)
	value_pfull_loc[i] = mpi_rank * 100 + i;
    for (i = 0; i < phalf_loc_size; i++)
	value_phalf_loc[i] = mpi_rank * 100 + i;
    for (j = 0; j < lon_yt_loc_size; j++)
    {
	for(i = 0; i < lon_xt_loc_size; i++)
	{
	    value_lon_loc[j * lon_xt_loc_size + i] = mpi_rank * 100 + i + j;
	    value_lat_loc[j * lon_xt_loc_size + i] = mpi_rank * 100 + i + j;
	    for (k = 0; k < pfull_loc_size; k++)
		value_clwmr_loc[j * lon_xt_loc_size + i] = mpi_rank * 100 + i + j + k;
	}
    }
	
    if (!mpi_rank)
       printf("\n*** Testing parallel writes with compression filters.\n");
    {
        int s;
        for (f = 0; f < NUM_COMPRESSION_FILTERS; f++)
        {
            for (s = 0; s < NUM_SHUFFLE_SETTINGS; s++)
            {
                if (!mpi_rank)
                {
                    printf("*** testing simple write with %s shuffle %d...",
                           (f ? "szip" : "zlib"), s);
                }

                /* nc_set_log_level(3); */
                /* Create a parallel netcdf-4 file. */
                if (nc_create_par(FILE_NAME, NC_NETCDF4, comm, info, &ncid)) ERR;
		
		/* Turn off fill mode. */
		if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;

		/* Define dimension grid_xt. */
		if (nc_def_dim(ncid, dim_name[0], dim_len[0], &dimid[0])) ERR;

		/* Define dimension grid_yt. */
		if (nc_def_dim(ncid, dim_name[1], dim_len[1], &dimid[1])) ERR;

		/* Define variable grid_xt. */
		if (nc_def_var(ncid, var_name[0], var_type[0], 1, &dimid[0], &varid[0])) ERR;
		if (nc_var_par_access(ncid, varid[0], NC_INDEPENDENT)) ERR;

		/* Define variable lon. */
		if (nc_def_var(ncid, var_name[1], var_type[1], 2, dimid, &varid[1])) ERR;
		if (nc_var_par_access(ncid, varid[1], NC_INDEPENDENT));

		/* Define variable grid_yt. */
		if (nc_def_var(ncid, var_name[2], var_type[2], 1, &dimid[1], &varid[2])) ERR;
		if (nc_var_par_access(ncid, varid[2], NC_INDEPENDENT)) ERR;

		/* Define variable lat. */
		if (nc_def_var(ncid, var_name[3], var_type[3], 2, dimid, &varid[3])) ERR;
		if (nc_var_par_access(ncid, varid[3], NC_INDEPENDENT)) ERR;

		/* Define dimension pfull. */
		if (nc_def_dim(ncid, dim_name[2], dim_len[2], &dimid[2])) ERR;

		/* Define variable pfull and write data. */
		if (nc_def_var(ncid, var_name[4], var_type[4], 1, &dimid[2], &varid[4])) ERR;
		if (nc_var_par_access(ncid, varid[4], NC_INDEPENDENT)) ERR;
		if (nc_enddef(ncid)) ERR;
		if (nc_put_vara_float(ncid, varid[4], &pfull_start, &pfull_loc_size, value_pfull_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		/* Define dimension phalf. */
		if (nc_def_dim(ncid, dim_name[3], dim_len[3], &dimid[3])) ERR;

		/* Define variable phalf and write data. */
		if (nc_def_var(ncid, var_name[5], var_type[5], 1, &dimid[3], &varid[5])) ERR;
		if (nc_var_par_access(ncid, varid[5], NC_INDEPENDENT)) ERR;
		if (nc_enddef(ncid)) ERR;
		if (nc_put_vara_float(ncid, varid[5], &phalf_start, &phalf_loc_size, value_phalf_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		/* Define dimension time. */
		if (nc_def_dim(ncid, dim_name[4], dim_len[4], &dimid[4])) ERR;

		/* Define variable time and write data. */
		if (nc_def_var(ncid, var_name[6], var_type[6], 1, &dimid[4], &varid[6])) ERR;
		if (nc_var_par_access(ncid, varid[6], NC_INDEPENDENT)) ERR;
		if (nc_enddef(ncid)) ERR;
		
		/* In NOAA code, do all processors write the single time value? */
		if (mpi_rank == 0)
		    if (nc_put_var_double(ncid, varid[6], &value_time));
		if (nc_redef(ncid));

		/* Write variable grid_xt data. */
		if (nc_enddef(ncid)) ERR;
		if (nc_put_vara_double(ncid, varid[0], &grid_xt_start, &grid_xt_loc_size, value_grid_xt_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		/* Write lon data. */
		if (nc_enddef(ncid)) ERR;
		start[0] = lon_xt_start;
		start[1] = lon_yt_start;
		count[0] = lon_xt_loc_size;
		count[1] = lon_yt_loc_size;
		if (nc_put_vara_double(ncid, varid[1], start, count, value_lon_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		/* Write grid_yt data. */
		if (nc_enddef(ncid)) ERR;
		if (nc_put_vara_double(ncid, varid[2], &grid_yt_start, &grid_yt_loc_size, value_grid_yt_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		/* Write lat data. */
		if (nc_enddef(ncid)) ERR;
		start[0] = lat_xt_start;
		start[1] = lat_yt_start;
		count[0] = lat_xt_loc_size;
		count[1] = lat_yt_loc_size;
		if (nc_put_vara_double(ncid, varid[3], start, count, value_lat_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		
		/* Define variable clwmr and write data (?) */
		dimid_data[0] = dimid[4];
		dimid_data[1] = dimid[2];
		dimid_data[2] = dimid[1];
		dimid_data[3] = dimid[0];
		if (nc_def_var(ncid, var_name[7], var_type[7], NDIM4, dimid_data, &varid[7])) ERR;

                /* Setting any filter only will work for HDF5-1.10.3 and later */
                /* versions. */
                if (!f)
                    res = nc_def_var_deflate(ncid, varid[7], s, 1, 4);
                else
                {
                    res = nc_def_var_deflate(ncid, varid[7], s, 0, 0);
                    if (!res)
                        res = nc_def_var_szip(ncid, varid[7], 32, 32);
                }
#ifdef HDF5_SUPPORTS_PAR_FILTERS
                if (res) ERR;
#else
                if (res != NC_EINVAL) ERR;
#endif

		if (nc_var_par_access(ncid, varid[7], NC_COLLECTIVE)) ERR;
		if (nc_enddef(ncid)) ERR;
		start[0] = 0;
		start[1] = pfull_start;
		start[2] = lat_yt_start;
		start[3] = lat_xt_start;
		count[0] = 1;
		count[1] = pfull_loc_size;
		count[2] = lat_yt_loc_size;
		count[3] = lat_xt_loc_size;
		if (nc_put_vara_float(ncid, varid[7], start, count, value_clwmr_loc)) ERR;
		if (nc_redef(ncid)) ERR;

		/* Close the file. */
		if (nc_close(ncid)) ERR;

                /* /\* Check file. *\/ */
                /* { */
                /*     int shuffle_in, deflate_in, deflate_level_in; */
                /*     int options_mask_in, pixels_per_block_in; */
                /*     int *slab_data_in; */

                /*     /\* Allocate data. *\/ */
                /*     if (!(slab_data_in = malloc(sizeof(int) * DIMSIZE * DIMSIZE / mpi_size))) ERR; */

                /*     /\* Reopen the file for parallel access. *\/ */
                /*     if (nc_open_par(FILE_NAME, NC_NOWRITE, comm, info, &ncid)) ERR; */

                /*     /\* Check state of compression. *\/ */
                /*     if (!f) */
                /*     { */
                /*         if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, &deflate_level_in)) ERR; */
                /*         if ((s && !shuffle_in) || (!s && shuffle_in)) ERR; */
                /*         if (!deflate_in || deflate_level_in != 1) ERR; */
                /*     } */
                /*     else */
                /*     { */
                /*         if (nc_inq_var_deflate(ncid, 0, &shuffle_in, NULL, NULL)) ERR; */
                /*         if ((s && !shuffle_in) || (!s && shuffle_in)) ERR; */
                /*         if (nc_inq_var_szip(ncid, 0, &options_mask_in, &pixels_per_block_in)) ERR; */
                /*     } */

                /*     /\* Use parallel I/O to read the data. *\/ */
                /*     for (start[2] = 0; start[2] < NUM_SLABS; start[2]++) */
                /*     { */
                /*         if (nc_get_vara_int(ncid, 0, start, count, slab_data_in)) ERR; */
                /*         for (i = 0; i < DIMSIZE * DIMSIZE / mpi_size; i++) */
                /*             if (slab_data_in[i] != mpi_rank) ERR; */
                /*     } */

                /*     /\* Close the netcdf file. *\/ */
                /*     if (nc_close(ncid)) ERR; */

                /*     free(slab_data_in); */
                /* } */

                if (!mpi_rank)
                    SUMMARIZE_ERR;
            } /* next shuffle filter test */
        } /* next compression filter (zlib and szip) */
        /* free(slab_data); */

	/* Free resources. */
	free(value_grid_xt_loc);
	free(value_grid_xt_loc_in);
	free(value_grid_yt_loc);
	free(value_grid_yt_loc_in);
	free(value_pfull_loc);
	free(value_pfull_loc_in);
	free(value_phalf_loc);
	free(value_phalf_loc_in);
	free(value_lon_loc);
	free(value_lon_loc_in);
	free(value_lat_loc);
	free(value_lat_loc_in);
	free(value_clwmr_loc);
	free(value_clwmr_loc_in);
    }

    /* Shut down MPI. */
    MPI_Finalize();

    if (!mpi_rank)
       FINAL_RESULTS;

    return 0;
}
