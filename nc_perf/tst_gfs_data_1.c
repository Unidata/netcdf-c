/*
  Copyright 2020, UCAR/Unidata See COPYRIGHT file for copying and
  redistribution conditions.

  This program tests netcdf-4 parallel I/O using the same access
  pattern as is used by NOAA's GFS when writing and reading model
  data. See https://github.com/Unidata/netcdf-fortran/issues/264

  Ed Hartnett, 6/28/20
*/

#include <nc_tests.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include "err_macros.h"
#include <mpi.h>

#define FILE_NAME "tst_gfs_data_1.nc"
#define NUM_META_VARS 7
#define NDIM2 2
#define NDIM4 4
#define NDIM5 5
#define NUM_PROC 4
#define NUM_SHUFFLE_SETTINGS 2
#ifdef HAVE_H5Z_SZIP
#define NUM_COMPRESSION_FILTERS 2
#else
#define NUM_COMPRESSION_FILTERS 1
#endif
#define NUM_DEFLATE_LEVELS 3
#define THOUSAND 1000
#define NUM_DATA_VARS 10

int
write_metadata(int ncid, int *data_varid, int s, int f, int deflate, int *dim_len, size_t phalf_loc_size, size_t phalf_start,
	       float *value_phalf_loc, size_t *data_start, size_t *data_count, float *value_pfull_loc,
	       size_t grid_xt_start, size_t grid_xt_loc_size, double *value_grid_xt_loc, size_t grid_yt_start,
	       size_t grid_yt_loc_size, double *value_grid_yt_loc, size_t *lat_start, size_t *lat_count,
	       double *value_lat_loc, size_t *lon_start, size_t *lon_count, double *value_lon_loc, int my_rank)
{
    char dim_name[NDIM5][NC_MAX_NAME + 1] = {"grid_xt", "grid_yt", "pfull",
					     "phalf", "time"};
    int dimid[NDIM5];
    int dimid_data[NDIM4];
    char var_name[NUM_META_VARS][NC_MAX_NAME + 1] = {"grid_xt", "lon", "grid_yt",
						     "lat", "pfull", "phalf", "time"};
    int var_type[NUM_META_VARS] = {NC_DOUBLE, NC_DOUBLE, NC_DOUBLE, NC_DOUBLE,
				   NC_FLOAT, NC_FLOAT, NC_DOUBLE};
    int varid[NUM_META_VARS];
    double value_time = 2.0;
    int dv;
    int res;
    		    
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
    if (nc_put_att_text(ncid, varid[1], "long_name", strlen("T-cell longitude"), "T-cell longitude")) ERR;
    if (nc_put_att_text(ncid, varid[1], "units", strlen("degrees_E"), "degrees_E")) ERR;

    if (nc_put_att_text(ncid, varid[0], "cartesian_axis", strlen("X"), "X")) ERR;

    /* Define variable grid_yt. */
    if (nc_def_var(ncid, var_name[2], var_type[2], 1, &dimid[1], &varid[2])) ERR;
    if (nc_var_par_access(ncid, varid[2], NC_INDEPENDENT)) ERR;

    /* Define variable lat. */
    if (nc_def_var(ncid, var_name[3], var_type[3], 2, dimid, &varid[3])) ERR;
    if (nc_var_par_access(ncid, varid[3], NC_INDEPENDENT)) ERR;
    if (nc_put_att_text(ncid, varid[3], "long_name", strlen("T-cell latitude"), "T-cell latitude")) ERR;
    if (nc_put_att_text(ncid, varid[3], "units", strlen("degrees_N"), "degrees_N")) ERR;

    if (nc_put_att_text(ncid, varid[2], "cartesian_axis", strlen("Y"), "Y")) ERR;
	
    /* Define dimension pfull. */
    if (nc_def_dim(ncid, dim_name[2], dim_len[2], &dimid[2])) ERR;

    /* Define variable pfull and write data. */
    if (nc_def_var(ncid, var_name[4], var_type[4], 1, &dimid[2], &varid[4])) ERR;
    if (nc_var_par_access(ncid, varid[4], NC_INDEPENDENT)) ERR;
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_float(ncid, varid[4], &data_start[1], &data_count[1], value_pfull_loc)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Define dimension phalf. This dim is only used by the phalf coord var. */
    if (nc_def_dim(ncid, dim_name[3], dim_len[3], &dimid[3])) ERR;

    /* Define coord variable phalf and write data. */
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
    if (my_rank == 0)
	if (nc_put_var_double(ncid, varid[6], &value_time)) ERR;;
    if (nc_redef(ncid)) ERR;

    /* Write variable grid_xt data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[0], &grid_xt_start, &grid_xt_loc_size, value_grid_xt_loc)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Write lon data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[1], lon_start, lon_count, value_lon_loc)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Write grid_yt data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[2], &grid_yt_start, &grid_yt_loc_size, value_grid_yt_loc)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Write lat data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[3], lat_start, lat_count, value_lat_loc)) ERR;

    /* Specify dimensions for our data vars. */
    dimid_data[0] = dimid[4];
    dimid_data[1] = dimid[2];
    dimid_data[2] = dimid[1];
    dimid_data[3] = dimid[0];

    /* Define data variables. */
    for (dv = 0; dv < NUM_DATA_VARS; dv++)
    {
	char data_var_name[NC_MAX_NAME + 1];

	sprintf(data_var_name, "var_%d", dv);
	if (nc_redef(ncid)) ERR;
	if (nc_def_var(ncid, data_var_name, NC_FLOAT, NDIM4, dimid_data, &data_varid[dv])) ERR;

	/* Setting any filter only will work for HDF5-1.10.3 and later */
	/* versions. */
	if (!f)
	    res = nc_def_var_deflate(ncid, data_varid[dv], s, 1, deflate);
	else
	{
	    res = nc_def_var_deflate(ncid, data_varid[dv], s, 0, 0);
	    if (!res)
		res = nc_def_var_szip(ncid, data_varid[dv], 32, 32);
	}
#ifdef HDF5_SUPPORTS_PAR_FILTERS
	if (res) ERR;
#else
	if (res != NC_EINVAL) ERR;
#endif
		    
	if (nc_var_par_access(ncid, data_varid[dv], NC_COLLECTIVE)) ERR;
	if (nc_enddef(ncid)) ERR;
    }

    if (nc_redef(ncid)) ERR;
    if (nc_put_att_text(ncid, varid[0], "long_name", strlen("T-cell longitude"), "T-cell longitude")) ERR;
    if (nc_put_att_text(ncid, varid[0], "units", strlen("degrees_E"), "degrees_E")) ERR;
    
    if (nc_put_att_text(ncid, varid[2], "long_name", strlen("T-cell latiitude"), "T-cell latiitude")) ERR;
    if (nc_put_att_text(ncid, varid[2], "units", strlen("degrees_N"), "degrees_N")) ERR;
    if (nc_enddef(ncid)) ERR;

    if (nc_redef(ncid)) ERR;
    
    for (dv = 0; dv < NUM_DATA_VARS; dv++)
    {
	float compress_err = 42.22;
	int nbits = 5;
	if (nc_put_att_float(ncid, data_varid[dv], "max_abs_compression_error", NC_FLOAT, 1, &compress_err)) ERR;
	if (nc_put_att_int(ncid, data_varid[dv], "nbits", NC_INT, 1, &nbits)) ERR;
    }
    
    if (nc_enddef(ncid)) ERR;
    return 0;
}

int
decomp_4D(int my_rank, int mpi_size, int *dim_len, size_t *start, size_t *count)
{
    start[0] = 0;
    count[0] = 1;
    
    count[1] = dim_len[2]/mpi_size;
    start[1] = my_rank * count[1];
    /* Add any extra to the end. */
    if (my_rank == mpi_size - 1)
	count[1] = count[1] + dim_len[2] % mpi_size;

    if (my_rank == 0 || my_rank == 1)
    {
	start[2] = 0;
	start[3] = 0;
    }
    else
    {
	start[2] = 768;
	start[3] = 768;
    }
    count[2] = 768;
    count[3] = 1536;

    printf("%d: start %ld %ld %ld %ld count %ld %ld %ld %ld\n", my_rank, start[0],
	   start[1], start[2], start[3], count[0], count[1], count[2], count[3]);  
    
    return 0;
}

int
main(int argc, char **argv)
{
    /* MPI stuff. */
    int mpi_size, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* For timing. */
    double meta_start_time, meta_stop_time;
    double data_start_time, data_stop_time;
    
    int ncid;
    size_t lat_start[NDIM2], lat_count[NDIM2];
    size_t lon_start[NDIM2], lon_count[NDIM2];
    size_t data_start[NDIM4], data_count[NDIM4];

    int dim_len[NDIM5] = {3072, 1536, 127, 128, 1};

    /* Variables. */
    int data_varid[NUM_DATA_VARS];
    float *value_pfull_loc;
    size_t phalf_loc_size, phalf_start;
    float *value_phalf_loc;
    size_t grid_xt_loc_size, grid_xt_start;
    double *value_grid_xt_loc;
    size_t grid_yt_loc_size, grid_yt_start;
    double *value_grid_yt_loc;
    double *value_lon_loc;
    double *value_lat_loc;
    float *value_data;
    int deflate_level[NUM_DEFLATE_LEVELS] = {1, 4, 9};

    int f;
    int i, j, k, dv, dl;

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Determine data decomposition. */
    if (decomp_4D(my_rank, mpi_size, dim_len, data_start, data_count)) ERR;

    /* Size of local (i.e. for this pe) grid_xt data. */
    grid_xt_loc_size = dim_len[0]/mpi_size;
    grid_xt_start = my_rank * grid_xt_loc_size;
    if (my_rank == mpi_size - 1)
	grid_xt_loc_size = grid_xt_loc_size + dim_len[0] % mpi_size;

    /* Size of local (i.e. for this pe) grid_yt data. */
    grid_yt_loc_size = dim_len[1]/mpi_size;
    grid_yt_start = my_rank * grid_yt_loc_size;
    if (my_rank == mpi_size - 1)
	grid_yt_loc_size = grid_yt_loc_size + dim_len[1] % mpi_size;

    /* Size of local (i.e. for this pe) phalf data. */
    phalf_loc_size = dim_len[3]/mpi_size;
    phalf_start = my_rank * phalf_loc_size;
    if (my_rank == mpi_size - 1)
	phalf_loc_size = phalf_loc_size + dim_len[3] % mpi_size;

    /* Size of local arrays (i.e. for this pe) lon and lat data. This is */
    /* specific to 4 pes. */
    lon_count[0] = 1536;
    lat_count[0] = 1536;
    if (my_rank == 0 || my_rank == 2)
    {
	lon_start[0] = 0;
	lat_start[0] = 0;
    }
    else
    {
	lon_start[0] = 1536;
	lat_start[0] = 1536;
    }
    lon_count[1] = 768;
    lat_count[1] = 768;
    if (my_rank == 0 || my_rank == 1)
    {
	lon_start[1] = 0;
	lat_start[1] = 0;
    }
    else
    {
	lon_start[1] = 768;
	lat_start[1] = 768;
    }

    /* Allocate space on this pe to hold the coordinate var data for this pe. */
    if (!(value_pfull_loc = malloc(data_count[1] * sizeof(float)))) ERR;
    if (!(value_phalf_loc = malloc(phalf_loc_size * sizeof(float)))) ERR;
    if (!(value_grid_xt_loc = malloc(grid_xt_loc_size * sizeof(double)))) ERR;
    if (!(value_grid_yt_loc = malloc(grid_yt_loc_size * sizeof(double)))) ERR;
    if (!(value_lon_loc = malloc(lon_count[0] * lon_count[1] * sizeof(double)))) ERR;
    if (!(value_lat_loc = malloc(lat_count[0] * lat_count[1] * sizeof(double)))) ERR;

    /* Allocate space to hold the data. */
    if (!(value_data = malloc(data_count[3] * data_count[2] * data_count[1] * sizeof(float)))) ERR;

    /* Some fake data for this pe to write. */
    for (i = 0; i < data_count[1]; i++)
	value_pfull_loc[i] = my_rank * 100 + i;
    for (i = 0; i < phalf_loc_size; i++)
	value_phalf_loc[i] = my_rank * 100 + i;
    for (i = 0; i < grid_xt_loc_size; i++)
	value_grid_xt_loc[i] = my_rank * 100 + i;
    for (i = 0; i < grid_yt_loc_size; i++)
	value_grid_yt_loc[i] = my_rank * 100 + i;
    for (j = 0; j < lon_count[1]; j++)
    {
	for(i = 0; i < lon_count[0]; i++)
	{
	    value_lon_loc[j * lon_count[0] + i] = my_rank * 100 + i + j;
	    value_lat_loc[j * lon_count[0] + i] = my_rank * 100 + i + j;
	    for (k = 0; k < data_count[1]; k++)
		value_data[j * lon_count[0] + i] = my_rank * 100 + i + j + k;
	}
    }

    if (my_rank == 0)
    {
	printf("Benchmarking creation of UFS file.\n");
	printf("comp, level, shuffle, meta, data\n");
    }
    {
        int s;
        for (f = 0; f < NUM_COMPRESSION_FILTERS; f++)
        {
            for (s = 0; s < NUM_SHUFFLE_SETTINGS; s++)
            {
		for (dl = 0; dl < NUM_DEFLATE_LEVELS; dl++)
		{
		    /* nc_set_log_level(3); */
		    /* Create a parallel netcdf-4 file. */
		    meta_start_time = MPI_Wtime();		
		    if (nc_create_par(FILE_NAME, NC_NETCDF4, comm, info, &ncid)) ERR;

		    if (write_metadata(ncid, data_varid, s, f, deflate_level[dl], dim_len, phalf_loc_size, phalf_start, value_phalf_loc, data_start, data_count, value_pfull_loc,
				       grid_xt_start, grid_xt_loc_size, value_grid_xt_loc, grid_yt_start, grid_yt_loc_size, value_grid_yt_loc,
				       lat_start, lat_count, value_lat_loc, lon_start, lon_count, value_lon_loc, my_rank)) ERR;
		
		
		    MPI_Barrier(MPI_COMM_WORLD);
		    meta_stop_time = MPI_Wtime();
		    data_start_time = MPI_Wtime();

		    /* Write one record each of the data variables. */
		    for (dv = 0; dv < NUM_DATA_VARS; dv++)
		    {
			if (nc_put_vara_float(ncid, data_varid[dv], data_start, data_count, value_data)) ERR;
			if (nc_redef(ncid)) ERR;
		    }

		    /* Close the file. */
		    if (nc_close(ncid)) ERR;
		    MPI_Barrier(MPI_COMM_WORLD);
		    data_stop_time = MPI_Wtime();
		    if (my_rank == 0)
			printf("%s, %d, %d, %g, %g\n", (f ? "szip" : "zlib"), deflate_level[dl], s, meta_stop_time - meta_start_time,
			       data_stop_time - data_start_time);
		} /* next deflate level */
            } /* next shuffle filter test */
        } /* next compression filter (zlib and szip) */

	/* Free resources. */
	free(value_grid_xt_loc);
	free(value_grid_yt_loc);
	free(value_pfull_loc);
	free(value_phalf_loc);
	free(value_lon_loc);
	free(value_lat_loc);
	free(value_data);
    }

    if (!my_rank)
        SUMMARIZE_ERR;
    
    /* Shut down MPI. */
    MPI_Finalize();

    if (!my_rank)
    	FINAL_RESULTS;

    return 0;
}
