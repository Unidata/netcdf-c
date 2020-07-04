/*
   Copyright 2019, UCAR/Unidata See COPYRIGHT file for copying and
   redistribution conditions.

   This program tests netcdf-4 parallel I/O using zlib, shuffle, and
   fletcher32 fliters while writing with parallel I/O. This works for
   HDF5-1.10.3 and later. In this case HDF5_SUPPORTS_PAR_FILTERS will
   be defined during configure.

   If szip was built into HDF5, this test will also test parallel szip
   writes. If szip is present, then HAVE_H5Z_SZIP will be defined in
   config.h.

   Ed Hartnett, 12/19/2019
*/

#include <nc_tests.h>
#include "err_macros.h"
#include <mpi.h>

#define FILE_NAME "tst_parallel_compress.nc"
#define NDIMS 3
#define DIMSIZE 24
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

    /* Netcdf-4 stuff. */
    int ncid, v1id, dimids[NDIMS];
    size_t start[NDIMS], count[NDIMS];

    int f, i, s, res;
    int *slab_data; /* one slab */

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    /* Allocate data. */
    if (!(slab_data = malloc(sizeof(int) * DIMSIZE * DIMSIZE / mpi_size))) ERR;

    /* Create phony data. We're going to write a 24x24 array of ints,
       in 4 sets of 144. */
    for (i = 0; i < DIMSIZE * DIMSIZE / mpi_size; i++)
       slab_data[i] = mpi_rank;

    if (!mpi_rank)
       printf("\n*** Testing parallel writes with compression filters.\n");
    {
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

                /* Create three dimensions. */
                if (nc_def_dim(ncid, "d1", DIMSIZE, dimids)) ERR;
                if (nc_def_dim(ncid, "d2", DIMSIZE, &dimids[1])) ERR;
                if (nc_def_dim(ncid, "d3", NUM_SLABS, &dimids[2])) ERR;

                /* Create one var. Turn on deflation. */
                if ((res = nc_def_var(ncid, "v1", NC_INT, NDIMS, dimids, &v1id))) ERR;

                /* Setting any filter only will work for HDF5-1.10.3 and later
                 * versions. */
                if (!f)
                    res = nc_def_var_deflate(ncid, 0, s, 1, 1);
                else
                {
                    res = nc_def_var_deflate(ncid, 0, s, 0, 0);
                    if (!res)
                        res = nc_def_var_szip(ncid, 0, 32, 32);
                }
#ifdef HDF5_SUPPORTS_PAR_FILTERS
                if (res) ERR;
#else
                if (res != NC_EINVAL) ERR;
#endif

                /* Setting fletcher32 only will work for HDF5-1.10.3 and later
                 * versions. */
                res = nc_def_var_fletcher32(ncid, 0, 1);
#ifdef HDF5_SUPPORTS_PAR_FILTERS
                if (res) ERR;
#else
                if (res != NC_EINVAL) ERR;
#endif

                /* Write metadata to file. */
                if (nc_enddef(ncid)) ERR;

                /* Set up slab for this process. */
                start[0] = mpi_rank * DIMSIZE/mpi_size;
                start[1] = 0;
                count[0] = DIMSIZE/mpi_size;
                count[1] = DIMSIZE;
                count[2] = 1;
                /*printf("mpi_rank=%d start[0]=%d start[1]=%d count[0]=%d count[1]=%d\n",
                  mpi_rank, start[0], start[1], count[0], count[1]);*/

                /* Should not be allowed to change access to independent,
                 * because filters are in use. */
                if (nc_var_par_access(ncid, v1id, NC_INDEPENDENT) != NC_EINVAL) ERR;

                /* Write slabs of data. */
                for (start[2] = 0; start[2] < NUM_SLABS; start[2]++)
                    if (nc_put_vara_int(ncid, v1id, start, count, slab_data)) ERR;

                /* Close the netcdf file. */
                if (nc_close(ncid)) ERR;

                /* Check file. */
                {
                    int shuffle_in, deflate_in, deflate_level_in;
                    int options_mask_in, pixels_per_block_in;
                    int *slab_data_in;

                    /* Allocate data. */
                    if (!(slab_data_in = malloc(sizeof(int) * DIMSIZE * DIMSIZE / mpi_size))) ERR;

                    /* Reopen the file for parallel access. */
                    if (nc_open_par(FILE_NAME, NC_NOWRITE, comm, info, &ncid)) ERR;

                    /* Check state of compression. */
                    if (!f)
                    {
                        if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, &deflate_level_in)) ERR;
                        if ((s && !shuffle_in) || (!s && shuffle_in)) ERR;
                        if (!deflate_in || deflate_level_in != 1) ERR;
                    }
                    else
                    {
                        if (nc_inq_var_deflate(ncid, 0, &shuffle_in, NULL, NULL)) ERR;
                        if ((s && !shuffle_in) || (!s && shuffle_in)) ERR;
                        if (nc_inq_var_szip(ncid, 0, &options_mask_in, &pixels_per_block_in)) ERR;
                    }

                    /* Use parallel I/O to read the data. */
                    for (start[2] = 0; start[2] < NUM_SLABS; start[2]++)
                    {
                        if (nc_get_vara_int(ncid, 0, start, count, slab_data_in)) ERR;
                        for (i = 0; i < DIMSIZE * DIMSIZE / mpi_size; i++)
                            if (slab_data_in[i] != mpi_rank) ERR;
                    }

                    /* Close the netcdf file. */
                    if (nc_close(ncid)) ERR;

                    free(slab_data_in);
                }

                if (!mpi_rank)
                    SUMMARIZE_ERR;
            } /* next shuffle filter test */
        } /* next compression filter (zlib and szip) */
	    
	/* Now run tests with unlimited dim. */
        /* for (f = 0; f < NUM_COMPRESSION_FILTERS; f++) */
        for (f = 1; f < NUM_COMPRESSION_FILTERS; f++)
        {
            for (s = 0; s < NUM_SHUFFLE_SETTINGS; s++)
            {
                if (!mpi_rank)
                {
                    printf("*** testing write along unlim dim with %s shuffle %d...",
                           (f ? "szip" : "zlib"), s);
                }

                /* nc_set_log_level(3); */
                /* Create a parallel netcdf-4 file. */
                if (nc_create_par(FILE_NAME, NC_NETCDF4, comm, info, &ncid)) ERR;

                /* Create three dimensions. */
                if (nc_def_dim(ncid, "d1", DIMSIZE, &dimids[1])) ERR;
                if (nc_def_dim(ncid, "d2", DIMSIZE, &dimids[2])) ERR;
                if (nc_def_dim(ncid, "d3", NC_UNLIMITED, &dimids[0])) ERR;

                /* Create one var. Turn on deflation. */
                if ((res = nc_def_var(ncid, "v1", NC_INT, NDIMS, dimids, &v1id))) ERR;

                /* Setting any filter only will work for HDF5-1.10.3 and later
                 * versions. */
                if (!f)
                    res = nc_def_var_deflate(ncid, 0, s, 1, 1);
                else
                {
                    res = nc_def_var_deflate(ncid, 0, s, 0, 0);
                    if (!res)
                        res = nc_def_var_szip(ncid, 0, 32, 32);
                }
#ifdef HDF5_SUPPORTS_PAR_FILTERS
                if (res) ERR;
#else
                if (res != NC_EINVAL) ERR;
#endif

                /* Setting fletcher32 only will work for HDF5-1.10.3 and later
                 * versions. */
                res = nc_def_var_fletcher32(ncid, 0, 1);
#ifdef HDF5_SUPPORTS_PAR_FILTERS
                if (res) ERR;
#else
                if (res != NC_EINVAL) ERR;
#endif

                /* Write metadata to file. */
                if (nc_enddef(ncid)) ERR;

                /* Set up slab for this process. */
                start[1] = mpi_rank * DIMSIZE/mpi_size;
                start[2] = 0;
                count[1] = DIMSIZE/mpi_size;
                count[2] = DIMSIZE;
                count[0] = 1;
                /*printf("mpi_rank=%d start[0]=%d start[1]=%d count[0]=%d count[1]=%d\n",
                  mpi_rank, start[0], start[1], count[0], count[1]);*/

                /* Should not be allowed to change access to independent,
                 * because filters are in use. */
                if (nc_var_par_access(ncid, v1id, NC_INDEPENDENT) != NC_EINVAL) ERR;

                /* Write slabs of data. */
                for (start[0] = 0; start[0] < NUM_SLABS; start[0]++)
                    if (nc_put_vara_int(ncid, v1id, start, count, slab_data)) ERR;

                /* Close the netcdf file. */
                if (nc_close(ncid)) ERR;

                /* Check file. */
                {
                    int shuffle_in, deflate_in, deflate_level_in;
                    int options_mask_in, pixels_per_block_in;
                    int *slab_data_in;

                    /* Allocate data. */
                    if (!(slab_data_in = malloc(sizeof(int) * DIMSIZE * DIMSIZE / mpi_size))) ERR;

                    /* Reopen the file for parallel access. */
                    if (nc_open_par(FILE_NAME, NC_NOWRITE, comm, info, &ncid)) ERR;

                    /* Check state of compression. */
                    if (!f)
                    {
                        if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, &deflate_level_in)) ERR;
                        if ((s && !shuffle_in) || (!s && shuffle_in)) ERR;
                        if (!deflate_in || deflate_level_in != 1) ERR;
                    }
                    else
                    {
                        if (nc_inq_var_deflate(ncid, 0, &shuffle_in, NULL, NULL)) ERR;
                        if ((s && !shuffle_in) || (!s && shuffle_in)) ERR;
                        if (nc_inq_var_szip(ncid, 0, &options_mask_in, &pixels_per_block_in)) ERR;
                    }

                    /* Use parallel I/O to read the data. */
                    for (start[0] = 0; start[0] < NUM_SLABS; start[0]++)
                    {
                        if (nc_get_vara_int(ncid, 0, start, count, slab_data_in)) ERR;
                        for (i = 0; i < DIMSIZE * DIMSIZE / mpi_size; i++)
                            if (slab_data_in[i] != mpi_rank) ERR;
                    }

                    /* Close the netcdf file. */
                    if (nc_close(ncid)) ERR;

                    free(slab_data_in);
                }

                if (!mpi_rank)
                    SUMMARIZE_ERR;
            } /* next shuffle filter test */
        } /* next compression filter (zlib and szip) */
        free(slab_data);
    }

    /* Shut down MPI. */
    MPI_Finalize();

    if (!mpi_rank)
       FINAL_RESULTS;

    return 0;
}
