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
#define VAR_NAME "var_name"
#define GROUP_NAME "grp1"

int
main()
{
    printf("\n*** Checking HDF5 VLEN types and their fill values.\n");
    printf("*** Checking simple HDF5 variable length type with default fill value...");
    {
	hid_t fileid, grpid, spaceid, typeid, datasetid, wspaceid;
	hsize_t dims[1] = {DIM1_LEN}, wdims[1] = {1};
	hvl_t data_out[1], data_in[DIM1_LEN];
	int *phoney;
	size_t j;
	size_t size;
      
	/* Create one phoney vlen. */
	if (!(phoney = malloc(sizeof(int) * NUM_ELEM))) ERR;
	for (j=0; j<NUM_ELEM; j++)
	    phoney[j] = -99;
	data_out[0].p = phoney;
	data_out[0].len = NUM_ELEM;
      
	/* Create file and group. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
				H5P_DEFAULT)) < 0) ERR;
	if ((grpid = H5Gcreate1(fileid, GROUP_NAME, 0)) < 0) ERR;

	/* Create VLEN type. */
	if ((typeid =  H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;

	/* Create space for dataset. */
	if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;

	/* Create dataset. */
	if ((datasetid = H5Dcreate1(grpid, VAR_NAME, typeid,
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
	if (H5Gclose(grpid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;

	/* Free the memory allocated above. */
	free(phoney);

	/* /\* Reopen the file and read the vlen data. *\/ */
	/* if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR; */
	/* if ((grpid = H5Gopen1(fileid, GROUP_NAME)) < 0) ERR; */

/*       /\* HDF5 allocated memory to store the data. Free that memory. *\/ */
/* #if H5_VERSION_GE(1,12,0) */
/*       if (H5Treclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR; */
/* #else */
/*       if (H5Dvlen_reclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR; */
/* #endif */

/*       /\* Close everything. *\/ */
/*       if (H5Sclose(spaceid) < 0 ||  */
/* 	  H5Tclose(typeid) < 0 ||  */
/* 	  H5Gclose(grpid) < 0 ||  */
/* 	  H5Fclose(fileid) < 0) ERR; */
    }

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
