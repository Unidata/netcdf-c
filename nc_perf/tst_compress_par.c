/*
  Copyright 2020, UCAR/Unidata See COPYRIGHT file for copying and
  redistribution conditions.

  This program tests and benchmarks netcdf-4 parallel I/O doing
  compression.

  This program tries to use the same access pattern as is used by
  NOAA's GFS when writing and reading model data. See:
  https://github.com/Unidata/netcdf-fortran/issues/264.

  Also see the file gfs_sample.cdl to see what is being produced by
  this program.

  Ed Hartnett, 6/28/20
*/

#include <config.h>
#include <nc_tests.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include "err_macros.h"
#include <mpi.h>
#include <H5public.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <netcdf.h>
#include <netcdf_par.h>
#include <netcdf_meta.h>

#define TEST_NAME "tst_compress_par"
#define NUM_META_VARS 7
#define NUM_META_TRIES 2
#define NDIM2 2
#define NDIM4 4
#define NDIM5 5
#define NUM_PROC 4
#define NUM_SHUFFLE_SETTINGS 1
/* #define NUM_DEFLATE_LEVELS 3 */
#define NUM_DEFLATE_LEVELS 3
#define NUM_UNLIM_TRIES 1
#define NUM_NSD_SETTINGS 2
#define THOUSAND 1000
#define NUM_DATA_VARS 3
#define ERR_AWFUL 1

/* #define USE_SMALL 1 */

#ifdef USE_SMALL
#define GRID_XT_LEN 8
#define GRID_YT_LEN 4
#define PFULL_LEN 4
#define PHALF_LEN 5
#else
#define GRID_XT_LEN 3072
#define GRID_YT_LEN 1536
#define PFULL_LEN 127
#define PHALF_LEN 128
#endif /* USE_SMALL */
#define TIME_LEN 1

#define MAX_COMPRESSION_FILTERS 4
char compression_filter_name[MAX_COMPRESSION_FILTERS][NC_MAX_NAME + 1];
int deflate_level[MAX_COMPRESSION_FILTERS][NUM_DEFLATE_LEVELS];
int nsd[NUM_NSD_SETTINGS] = {0, 4};

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
write_meta(int ncid, int *data_varid, int s, int f, int nsd, int deflate, int u,
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

        if (nsd)
            if (nc_def_var_quantize(ncid, data_varid[dv], NC_QUANTIZE_BITGROOM, nsd)) ERR;

        /* Setting any filter only will work for HDF5-1.10.3 and later */
        /* versions. Do nothing for "none". */
        if (!strcmp(compression_filter_name[f], "zlib"))
            if (nc_def_var_deflate(ncid, data_varid[dv], s, 1, deflate)) ERR;
        
#if NC_HAS_SZIP_WRITE
        if (!strcmp(compression_filter_name[f], "szip"))
            if (nc_def_var_szip(ncid, data_varid[dv], 32, 32)) ERR;
#endif /* NC_HAS_SZIP_WRITE */
        
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
    else 
        return ERR_AWFUL;

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

    /* printf("%d: latlon_start %ld %ld latlon_count %ld %ld\n", my_rank, latlon_start[0], */
    /*        latlon_start[1], latlon_count[0], latlon_count[1]); */

    return 0;
}

/* Based on the MPI rank and number of tasks, calculate the
 * decomposition of the 4D data. */
int
decomp_4D(int my_rank, int mpi_size, int *dim_len, size_t *start, size_t *count)
{
    /* Time dimension. */
    start[0] = 0;
    count[0] = 1;

    /* Vertical dimension (pfull). */
    count[1] = dim_len[2];
    start[1] = 0;
    
    if (mpi_size == 1)
    {
	start[2] = 0;
	start[3] = 0;
	count[2] = dim_len[1];
	count[3] = dim_len[0];
    }
    else if (mpi_size == 4)
    {
#ifdef USE_SMALL
        start[2] = (my_rank < 2) ? 0 : 2;
        start[3] = (!my_rank || my_rank == 2) ? 0 : 4;
        count[2] = 2;
        count[3] = 4;
#else
        start[2] = (my_rank < 2) ? 0 : 768;
        start[3] = (!my_rank || my_rank == 2) ? 0 : 1536;        
        count[2] = 768;
        count[3] = 1536;
#endif /* USE_SMALL */
    }
    else
        return ERR_AWFUL;

    /* printf("%d: start %ld %ld %ld %ld count %ld %ld %ld %ld\n", my_rank, start[0], */
    /*        start[1], start[2], start[3], count[0], count[1], count[2], count[3]); */

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

/* Determine what compression filters are present. */
int
find_filters(int *num_compression_filters, char compression_filter_name[][NC_MAX_NAME + 1],
             int deflate_level[][NUM_DEFLATE_LEVELS])
{
    int nfilters = 0;
       
    /* Try with no compression. */
    strcpy(compression_filter_name[nfilters], "none");
    nfilters++;

    /* zlib is always present. */
    strcpy(compression_filter_name[nfilters], "zlib");
    deflate_level[nfilters][0] = 1;
    deflate_level[nfilters][1] = 4;
    deflate_level[nfilters][2] = 9;
    nfilters++;

    /* szip is optionally present. */
#if NC_HAS_SZIP_WRITE
    strcpy(compression_filter_name[nfilters], "szip");
    nfilters++;
#endif /* NC_HAS_SZIP_WRITE */

    *num_compression_filters = nfilters;
    return 0;
}

int
main(int argc, char **argv)
{
    /* Parallel I/O with compression was not supported in HDF5 prior to 1.10.2. */
#if H5_VERSION_GE(1,10,2)    
    /* MPI stuff. */
    int mpi_size, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* For timing. */
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

    /* Compression filter info. */
    int num_compression_filters;

    int f, s, n;
    int i, j, k, dv, dl;
    int ret;

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Determine what compression filters are present. */
    if ((ret = find_filters(&num_compression_filters, compression_filter_name, deflate_level)))
        return ret;       

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

    /* printf("%d: data_count[3] %ld data_count[2] %ld data_count[1] %ld\n", my_rank, */
    /*        data_count[3], data_count[2], data_count[1]); */
    
    /* Allocate space to hold the data. */
    if (!(value_data = malloc(data_count[3] * data_count[2] * data_count[1] *
			      sizeof(float)))) ERR;

    /* Create some data. */
    size_t cnt = 0;
    for (k = 0; k < data_count[1]; k++)
    {
    	for (j = 0; j < data_count[2]; j++)
        {
    	    for(i = 0; i < data_count[3]; i++)
            {
                /* value_data[cnt] = (-1 * i%2) * my_rank * 1000 + cnt / sqrt(my_rank + cnt + 1) - (-1 * i%3 * i); */
                value_data[cnt] = (-1 * i%2) * my_rank * 1000 + cnt / sqrt(my_rank + cnt + 1) - (-1 * i%2 * i);
                /* printf("%d: value_data[%ld] %g\n", my_rank, cnt, value_data[cnt]); */
                cnt++;
            }
        }
    }

    if (my_rank == 0)
    {
        printf("Benchmarking creation of file similar to one produced by the UFS.\n");
        printf("comp, level, nsd, shuffle, data wr rate (MB/s), "
	       "file size (MB)\n");
    }
    for (f = 0; f < num_compression_filters; f++)
    {
        for (s = 0; s < NUM_SHUFFLE_SETTINGS; s++)
        {
            for (n = 0; n < NUM_NSD_SETTINGS; n++)
            {
		for (dl = 0; dl < NUM_DEFLATE_LEVELS; dl++)
		{
		    size_t file_size;
                    char file_name[NC_MAX_NAME * 3 + 1];

		    /* No deflate levels for szip or none. */
                    if (!strcmp(compression_filter_name[f], "szip") && dl) continue;
                    if (!strcmp(compression_filter_name[f], "none") && dl) continue;

                    /* Use the same filename every time, so we don't
                     * create many large files, just one. ;-) */
                    sprintf(file_name, "%s.nc", TEST_NAME);

		    /* nc_set_log_level(3); */
		    /* Create a parallel netcdf-4 file. */
		    if (nc_create_par(file_name, NC_NETCDF4, comm, info,
				      &ncid)) ERR;
		    if (write_meta(ncid, data_varid, s, f, nsd[n], deflate_level[f][dl], 0,
				   phalf_size, phalf_start, phalf,
				   data_start, data_count, pfull_start, pfull_size, pfull, grid_xt_start,
				   grid_xt_size, grid_xt, grid_yt_start,
				   grid_yt_size, grid_yt, latlon_start,
				   latlon_count, lat, lon, my_rank)) ERR;

		    /* Stop the timer for metadata writes. */
		    MPI_Barrier(MPI_COMM_WORLD);
		    data_start_time = MPI_Wtime();

		    /* Write one record each of the data variables. */
		    for (dv = 0; dv < NUM_DATA_VARS; dv++)
		    {
                        /* printf("%d: data_start %ld %ld %ld %ld data_count %ld %ld %ld %ld\n", my_rank, data_start[0], data_start[1], */
                        /*        data_start[2], data_start[3], data_count[0], data_count[1], data_count[2], data_count[3]); */
                        /* MPI_Barrier(MPI_COMM_WORLD); */
                        if (nc_put_vara_float(ncid, data_varid[dv], data_start, data_count,
                                              value_data)) ERR;
			if (nc_redef(ncid)) ERR;
		    }

		    /* Close the file. */
		    if (nc_close(ncid)) ERR;

		    /* Stop the data timer. */
		    MPI_Barrier(MPI_COMM_WORLD);
		    data_stop_time = MPI_Wtime();

		    /* Get the file size. */
		    if (get_file_size(file_name, &file_size)) ERR;

		    /* Check the file metadata for correctness. */
		    if (nc_open_par(file_name, NC_NOWRITE, comm, info, &ncid)) ERR;
		    if (check_meta(ncid, data_varid, s, f, deflate_level[f][dl], 0,
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
			data_size = (NUM_DATA_VARS * dim_len[0] * dim_len[1] * dim_len[2] *
                                     dim_len[4] * sizeof(float))/MILLION;
                        /* printf("data_size %f (data_stop_time - data_start_time) %g\n", data_size, (data_stop_time - data_start_time)); */
			data_rate = data_size / (data_stop_time - data_start_time);
			printf("%s, %d, %d, %d, %g, %g\n", compression_filter_name[f],
			       deflate_level[f][dl], nsd[n], s,
			       data_rate, (float)file_size/MILLION);
		    }
		    MPI_Barrier(MPI_COMM_WORLD);
		} /* next deflate level */
            } /* next nsd */
        } /* next shuffle filter test */
    } /* next compression filter (zlib and szip) */

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
#endif /* HDF5 version > 1.10.2 */
        FINAL_RESULTS;

    return 0;
}
