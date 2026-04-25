/* This is part of the netCDF package.  Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program exercises HDF5 variable length array code.

   For many years I felt bad that the word "phony" was misspelt in the
   code below, and in the libhdf4 code. But I've recently learned that
   "phoney" is the UK spelling, just as "misspelt" is the UK spelling
   of "misspelled." So I didn't misspell the word, I was just
   programming in the incorrect location.

   Edward Hartnett, 4/23/26
*/

#include "h5_err_macros.h"
#include <hdf5.h>
#include <stddef.h>

#define FILE_NAME "tst_h_vl_fill.h5"
#define DIM1_LEN 3
#define NUM_ELEM 2
#define FILL_VAL -42
#define FILL_NUM_ELEM 1
#define LARGE_FILL_NUM_ELEM (64 * 1024 * 1024 / sizeof(int) + 1)
#define VAR_NAME "var_name"

int
main()
{
    printf("\n*** Checking HDF5 VLEN types and their fill values.\n");
    /* Test 1: VLEN dataset with no fill value set (HDF5 default).
       When only the first element of a DIM1_LEN-element dataset is
       written, HDF5 leaves the remaining elements as the default VLEN
       fill: an empty sequence with len=0 and p=NULL. */
    printf("*** Checking simple HDF5 variable length type with default fill value...");
    {
	hid_t fileid, spaceid, typeid, datasetid, wspaceid;
	hsize_t dims[1] = {DIM1_LEN}, wdims[1] = {1};
	hvl_t data_out[1], data_in[DIM1_LEN];
	int *phoney;
	size_t j;
      
	/* Create one phoney vlen. */
	if (!(phoney = malloc(sizeof(int) * NUM_ELEM))) ERR;
	for (j=0; j<NUM_ELEM; j++)
	    phoney[j] = -99;
	data_out[0].p = phoney;
	data_out[0].len = NUM_ELEM;
      
	/* Create file. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
				H5P_DEFAULT)) < 0) ERR;

	/* Create VLEN type. */
	if ((typeid =  H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;

	/* Create space for dataset. */
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;

	/* Create dataset. */
	if ((datasetid = H5Dcreate1(fileid, VAR_NAME, typeid,
				    spaceid, H5P_DEFAULT)) < 0) ERR;

	/* Create space for write. */
	if ((wspaceid = H5Screate_simple(1, wdims, NULL)) < 0) ERR;

	/* Write data. */
	if (H5Dwrite(datasetid, typeid, wspaceid, wspaceid, H5P_DEFAULT,
		     data_out) < 0) ERR;
      
	if (H5Dclose(datasetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Sclose(wspaceid) < 0) ERR;
	if (H5Tclose(typeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;

	/* Free the memory allocated above. */
	free(phoney);

	/* Reopen the file and read the vlen data. */
	if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
	if ((typeid = H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
	if ((datasetid = H5Dopen1(fileid, VAR_NAME)) < 0) ERR;

	/* Read all elements. */
	if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_in) < 0) ERR;

	/* First element must match what we wrote: NUM_ELEM ints all equal to -99. */
	if (data_in[0].len != NUM_ELEM) ERR;
	for (j = 0; j < NUM_ELEM; j++)
	    if (((int *)data_in[0].p)[j] != -99) ERR;

	/* Remaining elements were not written; they must be the default empty VLEN. */
	for (j = 1; j < DIM1_LEN; j++)
	    if (data_in[j].len != 0 || data_in[j].p != NULL) ERR;

	/* HDF5 allocated memory to store the data. Free that memory. */
#if H5_VERSION_GE(1,12,0)
	if (H5Treclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;
#else
	if (H5Dvlen_reclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;
#endif

	/* Close everything. */
	if (H5Dclose(datasetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Tclose(typeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;
    }

    SUMMARIZE_ERR;
    /* Test 2: VLEN dataset with a user-defined fill value.
       The dataset creation property list sets the fill value to a
       VLEN of FILL_NUM_ELEM integers each equal to FILL_VAL. When
       only the first element is written, HDF5 populates the remaining
       elements with the defined fill value on read, so each unwritten
       element will have len=FILL_NUM_ELEM and its integers equal to
       FILL_VAL. */
    printf("*** Checking simple HDF5 variable length type with a defined fill value...");
    {
	hid_t fileid, spaceid, typeid, datasetid, wspaceid, dcplid;
	hsize_t dims[1] = {DIM1_LEN}, wdims[1] = {1};
	hvl_t data_out[1], data_in[DIM1_LEN], fill_vlen;
	int fill_data[FILL_NUM_ELEM];
	int *phoney;
	size_t j;

	/* Set up the fill value: a VLEN of FILL_NUM_ELEM ints equal to FILL_VAL. */
	for (j = 0; j < FILL_NUM_ELEM; j++)
	    fill_data[j] = FILL_VAL;
	fill_vlen.p = fill_data;
	fill_vlen.len = FILL_NUM_ELEM;

	/* Create one phoney vlen. */
	if (!(phoney = malloc(sizeof(int) * NUM_ELEM))) ERR;
	for (j=0; j<NUM_ELEM; j++)
	    phoney[j] = -99;
	data_out[0].p = phoney;
	data_out[0].len = NUM_ELEM;
      
	/* Create file. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
				H5P_DEFAULT)) < 0) ERR;

	/* Create VLEN type. */
	if ((typeid = H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;

	/* Create space for dataset. */
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;

	/* Create dataset creation property list and set fill value. */
	if ((dcplid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
	if (H5Pset_fill_value(dcplid, typeid, &fill_vlen) < 0) ERR;

	/* Create dataset with fill value. */
	if ((datasetid = H5Dcreate1(fileid, VAR_NAME, typeid,
				    spaceid, dcplid)) < 0) ERR;

	/* Create space for write. */
	if ((wspaceid = H5Screate_simple(1, wdims, NULL)) < 0) ERR;

	/* Write only the first element. */
	if (H5Dwrite(datasetid, typeid, wspaceid, wspaceid, H5P_DEFAULT,
		     data_out) < 0) ERR;

	if (H5Dclose(datasetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Sclose(wspaceid) < 0) ERR;
	if (H5Pclose(dcplid) < 0) ERR;
	if (H5Tclose(typeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;

	/* Free the memory allocated above. */
	free(phoney);

	/* Reopen the file and read the vlen data. */
	if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
	if ((typeid = H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
	if ((datasetid = H5Dopen1(fileid, VAR_NAME)) < 0) ERR;

	/* Read all elements. */
	if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_in) < 0) ERR;

	/* First element must match what we wrote: NUM_ELEM ints all equal to -99. */
	if (data_in[0].len != NUM_ELEM) ERR;
	for (j = 0; j < NUM_ELEM; j++)
	    if (((int *)data_in[0].p)[j] != -99) ERR;

	/* Remaining elements were not written; they must contain the fill value. */
	for (j = 1; j < DIM1_LEN; j++)
	{
	    if (data_in[j].len != FILL_NUM_ELEM) ERR;
	    if (((int *)data_in[j].p)[0] != FILL_VAL) ERR;
	}

	/* HDF5 allocated memory to store the data. Free that memory. */
#if H5_VERSION_GE(1,12,0)
	if (H5Treclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;
#else
	if (H5Dvlen_reclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;
#endif

	/* Close everything. */
	if (H5Dclose(datasetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Tclose(typeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;
    }

    SUMMARIZE_ERR;
    /* Test 3: VLEN dataset with a fill value whose data exceeds 64 MB.
       HDF5 stores fill values in the object header of the dataset.
       Despite the large size, HDF5 accepts and stores the fill value
       successfully. When only the first element is written, the
       remaining elements are populated with the large fill VLEN on
       read: each will have len=LARGE_FILL_NUM_ELEM and every integer
       equal to FILL_VAL. */
    printf("*** Checking simple HDF5 variable length type with a large (>64MB) fill value...");
    {
	hid_t fileid, spaceid, typeid, datasetid, wspaceid, dcplid;
	hsize_t dims[1] = {DIM1_LEN}, wdims[1] = {1};
	hvl_t data_out[1], data_in[DIM1_LEN], fill_vlen;
	int *fill_data;
	int *phoney;
	size_t j;

	/* Allocate and initialise the large fill value buffer. */
	if (!(fill_data = malloc(sizeof(int) * LARGE_FILL_NUM_ELEM))) ERR;
	for (j = 0; j < LARGE_FILL_NUM_ELEM; j++)
	    fill_data[j] = FILL_VAL;
	fill_vlen.p = fill_data;
	fill_vlen.len = LARGE_FILL_NUM_ELEM;

	/* Create one phoney vlen for the write. */
	if (!(phoney = malloc(sizeof(int) * NUM_ELEM))) ERR;
	for (j = 0; j < NUM_ELEM; j++)
	    phoney[j] = -99;
	data_out[0].p = phoney;
	data_out[0].len = NUM_ELEM;

	/* Create file. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
				H5P_DEFAULT)) < 0) ERR;

	/* Create VLEN type. */
	if ((typeid = H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;

	/* Create space for dataset. */
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;

	/* Create dataset creation property list and set the large fill value. */
	if ((dcplid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
	if (H5Pset_fill_value(dcplid, typeid, &fill_vlen) < 0) ERR;

	/* Create dataset with large fill value. */
	if ((datasetid = H5Dcreate1(fileid, VAR_NAME, typeid,
				    spaceid, dcplid)) < 0) ERR;

	/* Create space for write and write only the first element. */
	if ((wspaceid = H5Screate_simple(1, wdims, NULL)) < 0) ERR;
	if (H5Dwrite(datasetid, typeid, wspaceid, wspaceid,
		     H5P_DEFAULT, data_out) < 0) ERR;

	if (H5Dclose(datasetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Sclose(wspaceid) < 0) ERR;
	if (H5Pclose(dcplid) < 0) ERR;
	if (H5Tclose(typeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;

	free(phoney);

	/* Reopen the file and read the vlen data. */
	if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
	if ((typeid = H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
	if ((datasetid = H5Dopen1(fileid, VAR_NAME)) < 0) ERR;

	/* Read all elements. */
	if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_in) < 0) ERR;

	/* First element must match what we wrote: NUM_ELEM ints all equal to -99. */
	if (data_in[0].len != NUM_ELEM) ERR;
	for (j = 0; j < NUM_ELEM; j++)
	    if (((int *)data_in[0].p)[j] != -99) ERR;

	/* Remaining elements must contain the large fill value. */
	for (j = 1; j < DIM1_LEN; j++)
	{
	    if (data_in[j].len != LARGE_FILL_NUM_ELEM) ERR;
	    for (size_t k = 0; k < LARGE_FILL_NUM_ELEM; k++)
		if (((int *)data_in[j].p)[k] != FILL_VAL) ERR;
	}

	/* HDF5 allocated memory to store the data. Free that memory. */
#if H5_VERSION_GE(1,12,0)
	if (H5Treclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;
#else
	if (H5Dvlen_reclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;
#endif

	/* Close everything. */
	if (H5Dclose(datasetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Tclose(typeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;

	free(fill_data);
    }

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
