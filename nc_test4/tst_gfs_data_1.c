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
#define NDIM5 5
#define QTR_DATA (DIMSIZE * DIMSIZE / 4)
#define NUM_PROC 4
#define NUM_SLABS 10
#define NUM_SHUFFLE_SETTINGS 2
#ifdef HAVE_H5Z_SZIP
#define NUM_COMPRESSION_FILTERS 2
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
    /* size_t start[NDIMS], count[NDIMS]; */

    /* Dimensions */
    char dim_name[NDIM5][NC_MAX_NAME + 1] = {"grid_xt", "grid_yt", "pfull", "phalf", "time"};
    int dim_len[NDIM5] = {3072, 1536, 127, 128, 1};
    int dimid[NDIM5];

    int f;
    /* int i; */
    /* int res; */
    /* int *slab_data; /\* one slab *\/ */

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    /* Allocate data. */
    /* if (!(slab_data = malloc(sizeof(int) * DIMSIZE * DIMSIZE / mpi_size))) ERR; */

    /* Create phony data. We're going to write a 24x24 array of ints,
       in 4 sets of 144. */
    /* for (i = 0; i < DIMSIZE * DIMSIZE / mpi_size; i++) */
    /*    slab_data[i] = mpi_rank; */

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
		/* if (nc_def_var(ncid, var_name[0], var_type(0), dimids=(/dimid(0)/), varid=varid(0))) ERR; */
/*   call check(nf90_var_par_access(ncid, varid(1), NF90_INDEPENDENT)) */

/*   ! Define variable lon. */
/*   call check(nf90_def_var(ncid, trim(var_name(2)), var_type(2), dimids=(/dimid(1),  dimid(2)/), varid=varid(2))) */
/* !  call check(nf90_var_par_access(ncid, varid(2), NF90_INDEPENDENT)) */

/*   ! Define variable grid_yt. */
/*   call check(nf90_def_var(ncid, trim(var_name(3)), var_type(3), dimids=(/dimid(2)/), varid=varid(3))) */
/*   call check(nf90_var_par_access(ncid, varid(3), NF90_INDEPENDENT)) */

/*   ! Define variable lat. */
/*   call check(nf90_def_var(ncid, trim(var_name(4)), var_type(4), dimids=(/dimid(1), dimid(2)/), varid=varid(4))) */
/*   call check(nf90_var_par_access(ncid, varid(4), NF90_INDEPENDENT)) */

/*   ! Define dimension pfull. */
/*   call check(nf90_def_dim(ncid, trim(dim_name(3)), dim_len(3), dimid(3))) */

/*   ! Define variable pfull and write data. */
/*   call check(nf90_def_var(ncid, trim(var_name(5)), var_type(5), dimids=(/dimid(3)/), varid=varid(5))) */
/*   call check(nf90_var_par_access(ncid, varid(5), NF90_INDEPENDENT)) */
/*   call check(nf90_enddef(ncid)) */
/*   call check(nf90_put_var(ncid, varid(5), start=(/pfull_start/), count=(/pfull_loc_size/), values=value_pfull_loc)) */
/*   call check(nf90_redef(ncid)) */

/*   ! Define dimension phalf. */
/*   call check(nf90_def_dim(ncid, trim(dim_name(4)), dim_len(4), dimid(4))) */

/*   ! Define variable phalf and write data. */
/*   call check(nf90_def_var(ncid, trim(var_name(6)), var_type(6), dimids=(/dimid(4)/), varid=varid(6))) */
/*   call check(nf90_var_par_access(ncid, varid(6), NF90_INDEPENDENT)) */
/*   call check(nf90_enddef(ncid)) */
/*   call check(nf90_put_var(ncid, varid(6), start=(/phalf_start/), count=(/phalf_loc_size/), values=value_phalf_loc)) */
/*   call check(nf90_redef(ncid)) */

/*   ! Define dimension time. */
/*   call check(nf90_def_dim(ncid, trim(dim_name(5)), dim_len(5), dimid(5))) */

/*   ! Define variable time and write data. */
/*   call check(nf90_def_var(ncid, trim(var_name(7)), var_type(7), dimids=(/dimid(5)/), varid=varid(7))) */
/*   call check(nf90_var_par_access(ncid, varid(7), NF90_INDEPENDENT)) */
/*   call check(nf90_enddef(ncid)) */
/*   ! In NOAA code, do all processors write the single time value? */
/*   if (my_rank .eq. 0) then */
/*      call check(nf90_put_var(ncid, varid(7), values=value_time)) */
/*   endif */
/*   call check(nf90_redef(ncid)) */

/*   ! Write variable grid_xt data. */
/*   call check(nf90_enddef(ncid)) */
/*   call check(nf90_put_var(ncid, varid(1), start=(/grid_xt_start/), count=(/grid_xt_loc_size/), values=value_grid_xt_loc))   */
/*   call check(nf90_redef(ncid)) */

/*   ! Write lon data. */
/*   call check(nf90_enddef(ncid)) */
/*   call check(nf90_put_var(ncid, varid(2), start=(/lon_xt_start, lon_yt_start/), count=(/lon_xt_loc_size, lon_yt_loc_size/), & */
/*        values=value_lon_loc))   */
/*   call check(nf90_redef(ncid)) */

/*   ! Write grid_yt data. */
/*   call check(nf90_enddef(ncid)) */
/*   call check(nf90_put_var(ncid, varid(3), start=(/grid_yt_start/), count=(/grid_yt_loc_size/), values=value_grid_yt_loc))   */
/*   call check(nf90_redef(ncid)) */

/*   ! Write lat data. */
/*   call check(nf90_enddef(ncid)) */
/*   call check(nf90_put_var(ncid, varid(4), start=(/lat_xt_start, lat_yt_start/), count=(/lat_xt_loc_size, lat_yt_loc_size/), & */
/*        values=value_lat_loc))   */
/*   call check(nf90_redef(ncid)) */

/*   ! Define variable clwmr and write data (?) */
/*   call check(nf90_def_var(ncid, trim(var_name(8)), var_type(8), dimids=(/dimid(1), dimid(2), dimid(3), dimid(5)/), & */
/*        varid=varid(8), shuffle=.true., deflate_level=ideflate)) */
/*   call check(nf90_var_par_access(ncid, varid(8), NF90_COLLECTIVE)) */
/*   call check(nf90_enddef(ncid)) */
/* !  call check(nf90_put_var(ncid, varid(8), values=value_clwmr))   */
/*   call check(nf90_put_var(ncid, varid(8), start=(/lat_xt_start, lat_yt_start, pfull_start, 1/), & */
/*        count=(/lat_xt_loc_size, lat_yt_loc_size, pfull_loc_size, 1/), values=value_clwmr_loc))   */
/*   call check(nf90_redef(ncid)) */

/*   ! Close the file. */
/*   call check(nf90_close(ncid)) */

                /* Setting any filter only will work for HDF5-1.10.3 and later
                 * versions. */
/*                 if (!f) */
/*                     res = nc_def_var_deflate(ncid, 0, s, 1, 1); */
/*                 else */
/*                 { */
/*                     res = nc_def_var_deflate(ncid, 0, s, 0, 0); */
/*                     if (!res) */
/*                         res = nc_def_var_szip(ncid, 0, 32, 32); */
/*                 } */
/* #ifdef HDF5_SUPPORTS_PAR_FILTERS */
/*                 if (res) ERR; */
/* #else */
/*                 if (res != NC_EINVAL) ERR; */
/* #endif */

/*                 /\* Setting fletcher32 only will work for HDF5-1.10.3 and later */
/*                  * versions. *\/ */
/*                 res = nc_def_var_fletcher32(ncid, 0, 1); */
/* #ifdef HDF5_SUPPORTS_PAR_FILTERS */
/*                 if (res) ERR; */
/* #else */
/*                 if (res != NC_EINVAL) ERR; */
/* #endif */

                /* /\* Write metadata to file. *\/ */
                /* if (nc_enddef(ncid)) ERR; */

                /* /\* Set up slab for this process. *\/ */
                /* start[0] = mpi_rank * DIMSIZE/mpi_size; */
                /* start[1] = 0; */
                /* count[0] = DIMSIZE/mpi_size; */
                /* count[1] = DIMSIZE; */
                /* count[2] = 1; */
                /* /\*printf("mpi_rank=%d start[0]=%d start[1]=%d count[0]=%d count[1]=%d\n", */
                /*   mpi_rank, start[0], start[1], count[0], count[1]);*\/ */

                /* /\* Should not be allowed to change access to independent, */
                /*  * because filters are in use. *\/ */
                /* if (nc_var_par_access(ncid, v1id, NC_INDEPENDENT) != NC_EINVAL) ERR; */

                /* /\* Write slabs of data. *\/ */
                /* for (start[2] = 0; start[2] < NUM_SLABS; start[2]++) */
                /*     if (nc_put_vara_int(ncid, v1id, start, count, slab_data)) ERR; */

                /* Close the netcdf file. */
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
    }

    /* Shut down MPI. */
    MPI_Finalize();

    if (!mpi_rank)
       FINAL_RESULTS;

    return 0;
}
