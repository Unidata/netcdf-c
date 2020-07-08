/*
  Copyright 2020, UCAR/Unidata See COPYRIGHT file for copying and
  redistribution conditions.

  This program tests and benchmarks netcdf-4 parallel I/O using the
  same access pattern as is used by NOAA's GFS when writing and
  reading model data. See:
  https://github.com/Unidata/netcdf-fortran/issues/264.

  Also see the file gfs_sample.cdl to see what is being produced by
  this program.

  Ed Hartnett, 6/28/20
*/

#include <nc_tests.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include "err_macros.h"
#include <mpi.h>


#define FILE_NAME "tst_gfs_data_1.nc"
#define NUM_META_VARS 7
#define NUM_META_TRIES 2
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
#define NUM_UNLIM_TRIES 1
#define THOUSAND 1000
#define NUM_DATA_VARS 10
#define ERR_AWFUL 1

#define GRID_XT_LEN 3072
#define GRID_YT_LEN 1536
#define PFULL_LEN 127
#define PHALF_LEN 128
#define TIME_LEN 1

char dim_name[NDIM5][NC_MAX_NAME + 1] = {"grid_xt", "grid_yt", "pfull",
					 "phalf", "time"};
char var_name[NUM_META_VARS][NC_MAX_NAME + 1] = {"grid_xt", "lon", "grid_yt",
						 "lat", "pfull", "phalf", "time"};
int var_type[NUM_META_VARS] = {NC_DOUBLE, NC_DOUBLE, NC_DOUBLE, NC_DOUBLE,
			       NC_FLOAT, NC_FLOAT, NC_DOUBLE};
int dim_len[NDIM5] = {GRID_XT_LEN, GRID_YT_LEN, PFULL_LEN, PHALF_LEN,
		      TIME_LEN};

/* Get the size of a file in bytes. */
int
get_file_size(char *filename, size_t *file_size)
{
    FILE *fp;
    assert(filename && file_size);

    fp = fopen(filename, "r");
    if (fp)
    {
        fseek(fp, 0 , SEEK_END);
        *file_size = ftell(fp);
        fclose(fp);
    }
    return 0;
}

/* Check all the metadata, including coordinate variable data. */
int
check_meta(int ncid, int *data_varid, int s, int f, int deflate, int u,
	   size_t phalf_size, size_t phalf_start, float *phalf, size_t *data_start,
	   size_t *data_count, size_t pfull_start, size_t pfull_size, float *pfull,
	   size_t grid_xt_start, size_t grid_xt_size, double *grid_xt, size_t grid_yt_start,
           size_t grid_yt_size, double *grid_yt, size_t *latlon_start,
	   size_t *latlon_count, double *lat, double *lon, int my_rank)
{
    int ndims, nvars, natts, unlimdimid;
    char name_in[NC_MAX_NAME + 1];
    int xtype_in;
    int ndims_in;
    int dimids_in[NDIM4];
    size_t len_in;
    double *grid_xt_in, *grid_yt_in;
    double *lat_in, *lon_in;
    float *phalf_in, *pfull_in;
    int d, v, i;

    /* Check number of dims, vars, atts. */
    if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
    if (ndims != NDIM5 || nvars != NUM_META_VARS + NUM_DATA_VARS ||
	natts != 0) ERR;
    if (unlimdimid != (u ? 4 : -1)) ERR;

    /* Check the dimensions. */
    for (d = 0; d < NDIM5; d++)
    {
	if (nc_inq_dim(ncid, d, name_in, &len_in)) ERR;
	if (strcmp(name_in, dim_name[d]) || len_in != dim_len[d]) ERR;
    }

    /* Check metadata vars. */
    for (v = 0; v < NUM_META_VARS; v++)
    {
	if (nc_inq_var(ncid, v, name_in, &xtype_in, &ndims_in, dimids_in,
		       &natts)) ERR;
	if (strcmp(name_in, var_name[v]) || xtype_in != var_type[v]) ERR;
    }

    /* Check the values for grid_xt. */
    if (!(grid_xt_in = malloc(grid_xt_size * sizeof(double)))) ERR;
    if (nc_get_vara_double(ncid, 0, &grid_xt_start, &grid_xt_size, grid_xt_in)) ERR;
    for (i = 0; i < grid_xt_size; i++)
	if (grid_xt_in[i] != grid_xt[i]) ERR;
    free(grid_xt_in);
    
    /* Check the values for lon. */
    if (!(lon_in = malloc(latlon_count[0] * latlon_count[1] * sizeof(double)))) ERR;
    if (nc_get_vara_double(ncid, 1, latlon_start, latlon_count, lon_in)) ERR;
    for (i = 0; i < latlon_count[0] * latlon_count[1]; i++)
    	if (lon_in[i] != lon[i]) ERR;
    free(lon_in);

    /* Check the values for grid_yt. */
    if (!(grid_yt_in = malloc(grid_yt_size * sizeof(double)))) ERR;
    if (nc_get_vara_double(ncid, 2, &grid_yt_start, &grid_yt_size, grid_yt_in)) ERR;
    for (i = 0; i < grid_yt_size; i++)
	if (grid_yt_in[i] != grid_yt[i]) ERR;
    free(grid_yt_in);

    /* Check the values for lat. */
    if (!(lat_in = malloc(latlon_count[0] * latlon_count[1] * sizeof(double)))) ERR;
    if (nc_get_vara_double(ncid, 1, latlon_start, latlon_count, lat_in)) ERR;
    for (i = 0; i < latlon_count[0] * latlon_count[1]; i++)
    	if (lat_in[i] != lat[i]) ERR;
    free(lat_in);

    /* Check the values for pfull. */
    if (!(pfull_in = malloc(pfull_size * sizeof(float)))) ERR;
    if (nc_get_vara_float(ncid, 4, &pfull_start, &pfull_size, pfull_in)) ERR;
    for (i = 0; i < pfull_size; i++)
	if (pfull_in[i] != pfull[i]) ERR;
    free(pfull_in);

    /* Check the values for phalf. */
    if (!(phalf_in = malloc(phalf_size * sizeof(float)))) ERR;
    if (nc_get_vara_float(ncid, 5, &phalf_start, &phalf_size, phalf_in)) ERR;
    for (i = 0; i < phalf_size; i++)
	if (phalf_in[i] != phalf[i]) ERR;
    free(phalf_in);
    
    return 0;
}

/* Write all the metadata, including coordinate variable data. */
int
write_meta(int ncid, int *data_varid, int s, int f, int deflate, int u,
	   size_t phalf_size, size_t phalf_start, float *phalf, size_t *data_start,
	   size_t *data_count, size_t pfull_start, size_t pfull_size, float *pfull,
	   size_t grid_xt_start, size_t grid_xt_size, double *grid_xt, size_t grid_yt_start,
	   size_t grid_yt_size, double *grid_yt, size_t *latlon_start,
	   size_t *latlon_count, double *lat, double *lon, int my_rank)
{
    int dimid[NDIM5];
    int dimid_data[NDIM4];
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
    if (nc_put_vara_float(ncid, varid[4], &pfull_start, &pfull_size, pfull)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Define dimension phalf. This dim is only used by the phalf coord var. */
    if (nc_def_dim(ncid, dim_name[3], dim_len[3], &dimid[3])) ERR;

    /* Define coord variable phalf and write data. */
    if (nc_def_var(ncid, var_name[5], var_type[5], 1, &dimid[3], &varid[5])) ERR;
    if (nc_var_par_access(ncid, varid[5], NC_INDEPENDENT)) ERR;
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_float(ncid, varid[5], &phalf_start, &phalf_size, phalf)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Define dimension time, sometimes the unlimited dimension,
     * sometimes a fixed dim of 1. */
    if (nc_def_dim(ncid, dim_name[4], (u ? NC_UNLIMITED : 1), &dimid[4])) ERR;

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
    if (nc_put_vara_double(ncid, varid[0], &grid_xt_start, &grid_xt_size, grid_xt)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Write lon data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[1], latlon_start, latlon_count, lon)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Write grid_yt data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[2], &grid_yt_start, &grid_yt_size, grid_yt)) ERR;
    if (nc_redef(ncid)) ERR;

    /* Write lat data. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_vara_double(ncid, varid[3], latlon_start, latlon_count, lat)) ERR;

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

/* Based on the MPI rank and number of tasks, calculate the
 * decomposition of the 2D lat/lon coordinate variables. */
int
decomp_latlon(int my_rank, int mpi_size, int *dim_len, size_t *latlon_start,
	      size_t *latlon_count, double **lat, double **lon)
{
    int i, j;

    assert(dim_len && latlon_start && latlon_count && lat && lon && !*lat &&
	   !*lon);

    /* Size of local arrays (i.e. for this pe) lon and lat data. */
    if (mpi_size == 1)
    {
	latlon_start[0] = 0;
	latlon_start[1] = 0;
	latlon_count[0] = dim_len[0];
	latlon_count[1] = dim_len[1];
    }
    else if (mpi_size == 4)
    {
	latlon_count[0] = dim_len[0]/2;
	if (my_rank == 0 || my_rank == 2)
	{
	    latlon_start[0] = 0;
	}
	else
	{
	    latlon_start[0] = dim_len[0]/2;
	}
	latlon_count[1] = dim_len[1]/2;
	if (my_rank == 0 || my_rank == 1)
	{
	    latlon_start[1] = 0;
	}
	else
	{
	    latlon_start[1] = dim_len[1]/2;
	}
    }
    else if (mpi_size == 36)
    {
    }

    /* Allocate storage. */
    if (!(*lon = malloc(latlon_count[0] * latlon_count[1] * sizeof(double)))) ERR;
    if (!(*lat = malloc(latlon_count[0] * latlon_count[1] * sizeof(double)))) ERR;

    /* Now calculate some latlon values to write. */
    for (i = 0; i < latlon_count[0]; i++)
    {
	for (j = 0; j < latlon_count[1]; j++)
        {
            (*lon)[j * latlon_count[0] + i] = my_rank * 100 + i + j;
            (*lat)[j * latlon_count[0] + i] = my_rank * 100 + i + j;
        }
    }

    printf("%d: latlon_start %ld %ld latlon_count %ld %ld\n", my_rank, latlon_start[0],
           latlon_start[1], latlon_count[0], latlon_count[1]);

    return 0;
}

/* Based on the MPI rank and number of tasks, calculate the
 * decomposition of the 4D data. */
int
decomp_4D(int my_rank, int mpi_size, int *dim_len, size_t *start, size_t *count)
{
    start[0] = 0;
    count[0] = 1;

    count[1] = dim_len[2];
    start[1] = 0;
    
    if (mpi_size == 1)
    {
	start[2] = 0;
	start[3] = 0;
	count[2] = dim_len[2];
	count[3] = dim_len[3];
    }
    else if (mpi_size == 4)
    {
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
    }
    else if (mpi_size == 36)
    {
        start[2] = my_rank * 256;
        start[3] = my_rank * 512;
        count[2] = 256;
        count[3] = 512;
    }
    else
        return ERR_AWFUL;

    printf("%d: start %ld %ld %ld %ld count %ld %ld %ld %ld\n", my_rank, start[0],
           start[1], start[2], start[3], count[0], count[1], count[2], count[3]);

    return 0;
}

/* Decompose the grid_xt and grid_yt coordinate vars, and also come up
 * with some data. */
int
decomp_grid(int my_rank, int mpi_size, int *dim_len, size_t *grid_xt_start, size_t *grid_xt_size,
	    size_t *grid_yt_start, size_t *grid_yt_size, double **grid_xt, double **grid_yt)
{
    int i;

   /* Size of local (i.e. for this pe) grid_xt data. */
    *grid_xt_size = dim_len[0]/mpi_size;
    *grid_xt_start = my_rank * *grid_xt_size;
    if (my_rank == mpi_size - 1)
        *grid_xt_size = *grid_xt_size + dim_len[0] % mpi_size;

    /* Size of local (i.e. for this pe) grid_yt data. */
    *grid_yt_size = dim_len[1]/mpi_size;
    *grid_yt_start = my_rank * *grid_yt_size;
    if (my_rank == mpi_size - 1)
        *grid_yt_size = *grid_yt_size + dim_len[1] % mpi_size;

    /* Allocate storage for the grid_xy and grid_yt coordinate
     * variable data. */
    if (!(*grid_xt = malloc(*grid_xt_size * sizeof(double)))) ERR;
    if (!(*grid_yt = malloc(*grid_yt_size * sizeof(double)))) ERR;

    /* Fill the grid_xt and grid_yt coordinate data arrays. */
    for (i = 0; i < *grid_xt_size; i++)
        (*grid_xt)[i] = my_rank * 100 + i;
    for (i = 0; i < *grid_yt_size; i++)
        (*grid_yt)[i] = my_rank * 100 + i;

    return 0;
}

/* Decompose the pfull and phalf coordinate vars. */
int
decomp_p(int my_rank, int mpi_size, size_t *data_count, int *dim_len,
	 size_t *phalf_start, size_t *phalf_size, float **phalf,
	 size_t *pfull_start, size_t *pfull_size, float **pfull)
{
    int i;
    
    /* Size of local (i.e. for this pe) phalf data. */
    *phalf_size = dim_len[3]/mpi_size;
    *phalf_start = my_rank * *phalf_size;
    if (my_rank == mpi_size - 1)
        *phalf_size = *phalf_size + dim_len[3] % mpi_size;

    *pfull_size = dim_len[2]/mpi_size;
    *pfull_start = my_rank * *pfull_size;
    if (my_rank == mpi_size - 1)
        *pfull_size = *pfull_size + dim_len[2] % mpi_size;
    
    /* Allocate space on this pe to hold the coordinate var data for this pe. */
    if (!(*pfull = malloc(data_count[1] * sizeof(float)))) ERR;
    if (!(*phalf = malloc(*phalf_size * sizeof(float)))) ERR;
    
    /* Some fake data for this pe to write. */
    for (i = 0; i < data_count[1]; i++)
        (*pfull)[i] = my_rank * 100 + i;
    for (i = 0; i < *phalf_size; i++)
        (*phalf)[i] = my_rank * 100 + i;

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
    size_t latlon_start[NDIM2], latlon_count[NDIM2];
    size_t data_start[NDIM4], data_count[NDIM4];

    /* Variables. */
    int data_varid[NUM_DATA_VARS];
    size_t pfull_size, pfull_start;
    float *pfull = NULL;
    size_t phalf_size, phalf_start;
    float *phalf = NULL;
    size_t grid_xt_size, grid_xt_start;
    double *grid_xt = NULL;
    size_t grid_yt_size, grid_yt_start;
    double *grid_yt = NULL;
    double *lon = NULL;
    double *lat = NULL;
    float *value_data;
    int deflate_level[NUM_DEFLATE_LEVELS] = {1, 4, 9};

    int f, s, u;
    int i, j, k, dv, dl;

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Determine 4D data decomposition to write data vars. */
    if (decomp_4D(my_rank, mpi_size, dim_len, data_start, data_count)) ERR;

    /* Determine 2D data decomposition to write lat/lon coordinate vars. */
    if (decomp_latlon(my_rank, mpi_size, dim_len, latlon_start, latlon_count,
		      &lat, &lon)) ERR;

    /* Decompose grid_xt and grid_yt coordiate vars. */
    if (decomp_grid(my_rank, mpi_size, dim_len, &grid_xt_start, &grid_xt_size,
		    &grid_yt_start, &grid_yt_size, &grid_xt, &grid_yt)) ERR;

    /* Decompose phalf and pfull. */
    if (decomp_p(my_rank, mpi_size, data_count, dim_len, &phalf_start,
		 &phalf_size, &phalf, &pfull_start, &pfull_size, &pfull)) ERR;
    
    /* Allocate space to hold the data. */
    if (!(value_data = malloc(data_count[3] * data_count[2] * data_count[1] *
			      sizeof(float)))) ERR;

    /* Create some data. */
    for (k = 0; k < data_count[1]; k++)
    	for (j = 0; j < data_count[2]; j++)
    	    for(i = 0; i < data_count[3]; i++)
                value_data[j * data_count[3] + i] = my_rank * 100 + i + j + k;

    if (my_rank == 0)
    {
        printf("Benchmarking creation of UFS file.\n");
        printf("unlim, comp, level, shuffle, meta wr time (s), data wr rate (MB/s), "
	       "file size (MB)\n");
    }
    for (u = 0; u < NUM_UNLIM_TRIES; u++)
    {
	for (f = 0; f < NUM_COMPRESSION_FILTERS; f++)
	{
	    for (s = 0; s < NUM_SHUFFLE_SETTINGS; s++)
	    {
		for (dl = 0; dl < NUM_DEFLATE_LEVELS; dl++)
		{
		    size_t file_size;

		    /* No deflate levels for szip. */
		    if (f && dl) continue;

		    /* nc_set_log_level(3); */
		    /* Create a parallel netcdf-4 file. */
		    meta_start_time = MPI_Wtime();
		    if (nc_create_par(FILE_NAME, NC_NETCDF4, comm, info,
				      &ncid)) ERR;
		    if (write_meta(ncid, data_varid, s, f, deflate_level[dl], u,
				   phalf_size, phalf_start, phalf,
				   data_start, data_count, pfull_start, pfull_size, pfull, grid_xt_start,
				   grid_xt_size, grid_xt, grid_yt_start,
				   grid_yt_size, grid_yt, latlon_start,
				   latlon_count, lat, lon, my_rank)) ERR;

		    /* Stop the timer for metadata writes. */
		    MPI_Barrier(MPI_COMM_WORLD);
		    meta_stop_time = MPI_Wtime();
		    data_start_time = MPI_Wtime();

		    /* Write one record each of the data variables. */
		    for (dv = 0; dv < NUM_DATA_VARS; dv++)
		    {
			if (nc_put_vara_float(ncid, data_varid[dv], data_start,
					      data_count, value_data)) ERR;
			if (nc_redef(ncid)) ERR;
		    }

		    /* Close the file. */
		    if (nc_close(ncid)) ERR;

		    /* Stop the data timer. */
		    MPI_Barrier(MPI_COMM_WORLD);
		    data_stop_time = MPI_Wtime();

		    /* Get the file size. */
		    if (get_file_size(FILE_NAME, &file_size)) ERR;

		    /* Check the file metadata for correctness. */
		    if (nc_open_par(FILE_NAME, NC_NOWRITE, comm, info, &ncid)) ERR;
		    if (check_meta(ncid, data_varid, s, f, deflate_level[dl], u,
				   phalf_size, phalf_start, phalf,
				   data_start, data_count, pfull_start, pfull_size,
				   pfull, grid_xt_start, grid_xt_size, grid_xt,
				   grid_yt_start, grid_yt_size, grid_yt, latlon_start,
				   latlon_count, lat, lon, my_rank)) ERR;
		    if (nc_close(ncid)) ERR;

		    /* Print out results. */
		    if (my_rank == 0)
		    {
			float data_size, data_rate;
			data_size = NUM_DATA_VARS * dim_len[0] * dim_len[1] *
			    dim_len[3] * sizeof(float)/1000000;
			data_rate = data_size / (data_stop_time - data_start_time);
			printf("%d %s, %d, %d, %g, %g, %g\n", u, (f ? "szip" : "zlib"),
			       deflate_level[dl], s, meta_stop_time - meta_start_time,
			       data_rate, (float)file_size/1000000);
		    }
		    MPI_Barrier(MPI_COMM_WORLD);
		} /* next deflate level */
	    } /* next shuffle filter test */
	} /* next compression filter (zlib and szip) */
    } /* next unlim_try */

    /* Free resources. */
    if (grid_xt)
	free(grid_xt);
    if (grid_yt)
	free(grid_yt);
    if (pfull)
	free(pfull);
    if (phalf)
	free(phalf);
    if (lon)
	free(lon);
    if (lat)
	free(lat);
    free(value_data);

    if (!my_rank)
        SUMMARIZE_ERR;

    /* Shut down MPI. */
    MPI_Finalize();

    if (!my_rank)
        FINAL_RESULTS;

    return 0;
}
