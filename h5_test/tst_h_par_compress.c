/* This is part of the netCDF package.  Copyright 2020 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This files tests parallel I/O using compression filters. This
   functionality is only available in HDF5-1.10.3 and later versions.

   Ed Hartnett
*/
#include <nc_tests.h>
#include "err_macros.h"
#include <hdf5.h>

#define FILE_NAME "tst_h_par_compress.h5"
#define VAR_NAME "Athelstan"
#define NDIMS 1
#define DIM2_LEN 16000000
#define SC1 100000 /* slice count. */
#define H5Z_FILTER_DEFLATE 1
#define H5Z_FILTER_ZSTD 32015
#define H5Z_FILTER_SZIP 4
#define MAX_NUM_FILTERS 3

int
main(int argc, char **argv)
{
    int cf;
    int p, my_rank;
    int num_compress_filters;
    char filter_name[MAX_NUM_FILTERS][NC_MAX_NAME + 1];
    int id[MAX_NUM_FILTERS];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /* We always have zlib. */
    id[0] = H5Z_FILTER_DEFLATE;
    strcpy(filter_name[0], "zlib");
    num_compress_filters = 1;

    /* We might have zstd. */
#ifdef HAVE_ZSTD
    id[num_compress_filters] = H5Z_FILTER_ZSTD;
    strcpy(filter_name[num_compress_filters], "zstd");
    num_compress_filters++;
#endif

    /* We might have szip. */
#ifdef HAVE_H5Z_SZIP    
    id[num_compress_filters] = H5Z_FILTER_SZIP;
    strcpy(filter_name[num_compress_filters], "szip");
    num_compress_filters++;
#endif

    /* For builds with HDF5 prior to 1.10.3, just return success. */
#ifdef HDF5_SUPPORTS_PAR_FILTERS
    for (cf = 0; cf < num_compress_filters; cf++)
    {
	if (!my_rank)
	    printf("*** Testing parallel I/O with %s compression...", filter_name[cf]);
	{
	    hid_t fapl_id, fileid, whole_spaceid, dsid, slice_spaceid, whole_spaceid1, xferid;
	    hid_t plistid;
	    hsize_t start[NDIMS], count[NDIMS], ones[NDIMS];
	    hsize_t dims[1], chunksize = SC1;
	    int data[SC1], data_in[SC1];
	    int num_steps;
	    int deflate_level = 4;
	    unsigned int ulevel = 1;	    
	    int i, s;

	    /* We will write the same slice of random data over and over to
	     * fill the file. */
	    for (i = 0; i < SC1; i++)
		data[i] = rand();

	    /* Create file. */
	    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
	    if (H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL) < 0) ERR;
	    if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
				    fapl_id)) < 0) ERR;

	    /* Create a space to deal with one slice in memory. */
	    dims[0] = SC1;
	    if ((slice_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

	    /* Create a space to write all slices. */
	    dims[0] = DIM2_LEN;
	    if ((whole_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

	    /* Create property list for dataset. */
	    if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
	
	    /* Turn off object tracking times in HDF5 (as is done in nc4hdf.c). */
	    if (H5Pset_obj_track_times(plistid, 0) < 0) ERR;
	
	    /* Required to truly turn HDF5 fill values off */
	    if (H5Pset_fill_time(plistid, H5D_FILL_TIME_NEVER) < 0) ERR;
	
	    /* Set compression, either deflate or szip. */
	    if (id[cf] == H5Z_FILTER_DEFLATE)
	    {
		if (H5Pset_deflate(plistid, deflate_level) < 0) ERR;
	    }
	    else if (id[cf] == H5Z_FILTER_ZSTD)
	    {
		herr_t code;		
		if ((code = H5Pset_filter(plistid, H5Z_FILTER_ZSTD, H5Z_FLAG_OPTIONAL, 1, &ulevel)))
		    ERR;
	    }
	    else if (id[cf] == H5Z_FILTER_SZIP)
            {
		int options_mask = 32;
		int bits_per_pixel = 32;
		if (H5Pset_szip(plistid, options_mask, bits_per_pixel)) ERR;
            }

	    /* Set chunking. */
	    if (H5Pset_chunk(plistid, NDIMS, &chunksize) < 0) ERR;

	    /* Turn on creation order tracking. */
	    if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
					   H5P_CRT_ORDER_INDEXED) < 0) ERR;
	
	    /* Create dataset. */
	    if ((dsid = H5Dcreate2(fileid, VAR_NAME, H5T_NATIVE_INT,
				   whole_spaceid, H5P_DEFAULT, plistid, H5P_DEFAULT)) < 0) ERR;

	    /* Use collective write operations. */
	    if ((xferid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;
	    if (H5Pset_dxpl_mpio(xferid, H5FD_MPIO_COLLECTIVE) < 0) ERR;

	    /* Write the data in num_step steps. */
	    num_steps = (DIM2_LEN/SC1) / p;
	    for (s = 0; s < num_steps; s++)
	    {
		/* Select hyperslab for write of one slice. */
		start[0] = s * SC1 * p + my_rank * SC1;
		count[0] = SC1;
		ones[0] = 1;
		if (H5Sselect_hyperslab(whole_spaceid, H5S_SELECT_SET,
					start, NULL, ones, count) < 0) ERR;

		if (H5Dwrite(dsid, H5T_NATIVE_INT, slice_spaceid, whole_spaceid,
			     xferid, data) < 0) ERR;

	    }

	    /* Close. These collective operations will allow every process
	     * to catch up. */
	    if (H5Dclose(dsid) < 0 ||
		H5Sclose(whole_spaceid) < 0 ||
		H5Sclose(slice_spaceid) < 0 ||
		H5Pclose(fapl_id) < 0 ||
		H5Pclose(plistid) < 0 ||
		H5Fclose(fileid) < 0)
		ERR;

	    /* Open the file. */
	    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
	    if (H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL) < 0) ERR;


	    if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
	    if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;

	    /* Create a space to deal with one slice in memory. */
	    dims[0] = SC1;
	    if ((slice_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

	    /* Open the dataset. */
	    if ((dsid = H5Dopen1(fileid, VAR_NAME)) < 0) ERR;
	    if ((whole_spaceid1 = H5Dget_space(dsid)) < 0) ERR;

	    /* Read the data, a slice at a time. */
	    for (s = 0; s < num_steps; s++)
	    {
		/* Select hyperslab for read of one slice. */
		start[0] = s * SC1 * p + my_rank * SC1;
		count[0] = SC1;
		ones[0] = 1;
		if (H5Sselect_hyperslab(whole_spaceid1, H5S_SELECT_SET,
					start, NULL, ones, count) < 0)
		{
		    ERR;
		    return 2;
		}

		if (H5Dread(dsid, H5T_NATIVE_INT, slice_spaceid, whole_spaceid1,
			    H5P_DEFAULT, data_in) < 0)
		{
		    ERR;
		    return 2;
		}

		/* Check the slice of data. */
		for (i = 0; i < SC1; i++)
		    if (data[i] != data_in[i])
		    {
			ERR;
			return 2;
		    }
	    }

	    /* Close down. */
	    if (H5Dclose(dsid) < 0 ||
		H5Sclose(slice_spaceid) < 0 ||
		H5Sclose(whole_spaceid1) < 0 ||
		H5Pclose(fapl_id) < 0 ||
		H5Fclose(fileid) < 0)
		ERR;
	}
	if (!my_rank)
	    SUMMARIZE_ERR;

    } /* next cf */
#else 
    {
	if (!my_rank)
	    printf("*** HDF5 1.10.3 or greater required for this test.\n");
    }

#endif /* HDF5_SUPPORTS_PAR_FILTERS */
    
    MPI_Finalize();

    if (!my_rank)
        FINAL_RESULTS;
    return 0;
}
