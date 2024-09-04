/* This is part of the netCDF package. Copyright 2024 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This files tests dataset creation and writing with zstd
   compression. This test is only run if zstd compression is enabled
   in the build.

   Ed Hartnett
*/

#include "h5_err_macros.h"
#include <hdf5.h>

#define FILE_NAME "tst_h_zstd.h5"
#define GRP_NAME "Bebbanburg"
#define VAR_BOOL_NAME "Uhtred"
#define VAR_INT_NAME "Alfred"
#define DIM1_LEN 3
#define DIM1_LEN_20 20
#define H5Z_FILTER_ZSTD 32015

int
main()
{
    hid_t fileid, grpid, spaceid, datasetid;
    int bool_out[DIM1_LEN] = {0, 1, 0};
    hsize_t dims[1];

    printf("\n*** Checking HDF5 variable functions with zstandard compression.\n");
    printf("*** Checking HDF5 zstd filter setting and getting...");
#define MAX_NAME 100
#define NUM_CD_ELEM 10
/* HDF5 defines this... */
#define ZSTD_NAME "zstd"
    {
        H5Z_filter_t filter;
        int num_filters;
        hid_t propid;
        unsigned int flags, cd_values[NUM_CD_ELEM], filter_config;
        size_t cd_nelems = NUM_CD_ELEM;
        size_t namelen = MAX_NAME;
        char name[MAX_NAME + 1];
        htri_t avail = -1;
        unsigned int id = H5Z_FILTER_ZSTD;
        unsigned int ulevel = 1;
        herr_t code;

        /* Check that zstandard filter is available. */
        if((avail = H5Zfilter_avail(id)) < 0) ERR;
        if (!avail)
        {
            printf("zstandard filter not available, but expected to be.\n");
            ERR;
        }
        
        /* Open file and create group. */
        if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
                                H5P_DEFAULT)) < 0) ERR;
        if ((grpid = H5Gcreate1(fileid, GRP_NAME, 0)) < 0) ERR;

        /* Write an array of bools, with zstandard compression. */
        dims[0] = DIM1_LEN;
        if ((propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
        if (H5Pset_layout(propid, H5D_CHUNKED)) ERR;
        if (H5Pset_chunk(propid, 1, dims)) ERR;
        if ((code = H5Pset_filter(propid, id, H5Z_FLAG_OPTIONAL, 1, &ulevel)))
            ERR;
        if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
        if ((datasetid = H5Dcreate1(grpid, VAR_BOOL_NAME, H5T_NATIVE_HBOOL,
                                   spaceid, propid)) < 0) ERR;
        if (H5Dwrite(datasetid, H5T_NATIVE_HBOOL, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                     bool_out) < 0) ERR;
        if (H5Dclose(datasetid) < 0 ||
            H5Pclose(propid) < 0 ||
            H5Sclose(spaceid) < 0 ||
            H5Gclose(grpid) < 0 ||
            H5Fclose(fileid) < 0)
            ERR;

        /* Now reopen the file and check. */
        if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
        if ((grpid = H5Gopen1(fileid, GRP_NAME)) < 0) ERR;
        if ((datasetid = H5Dopen1(grpid, VAR_BOOL_NAME)) < 0) ERR;
        if ((propid = H5Dget_create_plist(datasetid)) < 0) ERR;

        /* The possible values of filter (which is just an int) can be
         * found in H5Zpublic.h. */
        if ((num_filters = H5Pget_nfilters(propid)) < 0) ERR;
        if (num_filters != 1) ERR;
        if ((filter = H5Pget_filter2(propid, 0, &flags, &cd_nelems, cd_values,
                                     namelen, name, &filter_config)) < 0) ERR;
        if (filter != H5Z_FILTER_ZSTD || cd_nelems != 1 ||
            cd_values[0] != ulevel) ERR;
        if (strcmp(name, ZSTD_NAME)) ERR;

        if (H5Dclose(datasetid) < 0 ||
            H5Pclose(propid) < 0 ||
            H5Gclose(grpid) < 0 ||
            H5Fclose(fileid) < 0)
            ERR;
    }

    SUMMARIZE_ERR;
    printf("*** Checking HDF5 zstd filter with int data...");
#define MAX_NAME 100
#define NUM_CD_ELEM 10
/* HDF5 defines this... */
#define ZSTD_NAME "zstd"
    {
        H5Z_filter_t filter;
        int num_filters;
        hid_t propid;
        unsigned int flags, cd_values[NUM_CD_ELEM], filter_config;
        size_t cd_nelems = NUM_CD_ELEM;
        size_t namelen = MAX_NAME;
        char name[MAX_NAME + 1];
        unsigned int id = H5Z_FILTER_ZSTD;
        unsigned int ulevel = 1;
	int int_out[DIM1_LEN_20];
	int int_in[DIM1_LEN_20];
        herr_t code;
	int i;

	for (i = 0; i < DIM1_LEN_20; i++)
	    int_out[i] = i * 10;
	
        /* Open file and create group. */
        if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
                                H5P_DEFAULT)) < 0) ERR;
        if ((grpid = H5Gcreate1(fileid, GRP_NAME, 0)) < 0) ERR;

        /* Write an array of ints, with zstandard compression. */
        dims[0] = DIM1_LEN_20;
        if ((propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
        if (H5Pset_layout(propid, H5D_CHUNKED)) ERR;
        if (H5Pset_chunk(propid, 1, dims)) ERR;
        if ((code = H5Pset_filter(propid, id, H5Z_FLAG_OPTIONAL, 1, &ulevel)))
            ERR;
        if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
        if ((datasetid = H5Dcreate1(grpid, VAR_INT_NAME, H5T_NATIVE_INT,
                                   spaceid, propid)) < 0) ERR;
        if (H5Dwrite(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                     int_out) < 0) ERR;
        if (H5Dclose(datasetid) < 0 ||
            H5Pclose(propid) < 0 ||
            H5Sclose(spaceid) < 0 ||
            H5Gclose(grpid) < 0 ||
            H5Fclose(fileid) < 0)
            ERR;

        /* Now reopen the file and check. */
        if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
        if ((grpid = H5Gopen1(fileid, GRP_NAME)) < 0) ERR;
        if ((datasetid = H5Dopen1(grpid, VAR_INT_NAME)) < 0) ERR;
        if ((propid = H5Dget_create_plist(datasetid)) < 0) ERR;

        /* The possible values of filter (which is just an int) can be
         * found in H5Zpublic.h. */
        if ((num_filters = H5Pget_nfilters(propid)) < 0) ERR;
        if (num_filters != 1) ERR;
        if ((filter = H5Pget_filter2(propid, 0, &flags, &cd_nelems, cd_values,
                                     namelen, name, &filter_config)) < 0) ERR;
        if (filter != H5Z_FILTER_ZSTD || cd_nelems != 1 ||
            cd_values[0] != ulevel) ERR;
        if (strcmp(name, ZSTD_NAME)) ERR;

        if (H5Dread(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                     int_in) < 0) ERR;
	for (i = 0; i < DIM1_LEN_20; i++)
	    if (int_in[i] != int_out[i]) ERR;
        if (H5Dclose(datasetid) < 0 ||
            H5Pclose(propid) < 0 ||
            H5Gclose(grpid) < 0 ||
            H5Fclose(fileid) < 0)
            ERR;
    }

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
