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
#define TEST_NAME "tst_h_zstd"
#define GRP_NAME "Henry_V"
#define VAR_BOOL_NAME "Southhamptons_Battle_Record"
#define GRP2_NAME "Some_3D_Met_Data"
#define DIM1_LEN 3
#define MAX_DIMS 255
#define NDIM1 1
#define H5Z_FILTER_ZSTD 32015

int
main()
{
    hid_t fileid, grpid, spaceid, datasetid;
    int bool_out[DIM1_LEN] = {0, 1, 0};
    hsize_t dims[1];

    printf("\n*** Checking HDF5 variable functions with zstandard compression.\n");
    printf("*** Checking HDF5 deflate filter setting and getting...");
#define DEFLATE_LEVEL 9
#define MAX_NAME 100
#define NUM_CD_ELEM 10
/* HDF5 defines this... */
#define DEFLATE_NAME "deflate"
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
        
        if (H5Pset_deflate(propid, DEFLATE_LEVEL)) ERR;
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
        printf("num_filters %d\n");
        /* if (num_filters != 1) ERR; */
        /* if ((filter = H5Pget_filter2(propid, 0, &flags, &cd_nelems, cd_values, */
        /*                              namelen, name, &filter_config)) < 0) ERR; */
        /* if (filter != H5Z_FILTER_ZSTD || cd_nelems != 1 || */
        /*     cd_values[0] != ulevel) ERR; */
        /* if (strcmp(name, ZSTD_NAME)) ERR; */

        if (H5Dclose(datasetid) < 0 ||
            H5Pclose(propid) < 0 ||
            H5Gclose(grpid) < 0 ||
            H5Fclose(fileid) < 0)
            ERR;
    }

    SUMMARIZE_ERR;
    /* printf("*** Checking HDF5 deflate, fletcher32, shuffle filter setting and getting..."); */
    /* { */
    /*     H5Z_filter_t filter; */
    /*     int num_filters; */
    /*     hid_t propid; */
    /*     unsigned int flags, cd_values[NUM_CD_ELEM], filter_config; */
    /*     size_t cd_nelems = NUM_CD_ELEM; */
    /*     size_t namelen = MAX_NAME; */
    /*     char name[MAX_NAME + 1]; */
    /*     int found_shuffle = 0, found_fletcher32 = 0, found_deflate = 0; */

    /*     /\* Open file and create group. *\/ */
    /*     if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, */
    /*                             H5P_DEFAULT)) < 0) ERR; */
    /*     if ((grpid = H5Gcreate1(fileid, GRP_NAME, 0)) < 0) ERR; */

    /*     /\* Write an array of bools, with compression, fletcher32 */
    /*      * checksum, shuffle filters. Like a hoogie with "the works." *\/ */
    /*     dims[0] = DIM1_LEN; */
    /*     if ((propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR; */
    /*     if (H5Pset_layout(propid, H5D_CHUNKED)) ERR; */
    /*     if (H5Pset_chunk(propid, 1, dims)) ERR; */
    /*     if (H5Pset_shuffle(propid)) ERR; */
    /*     if (H5Pset_deflate(propid, DEFLATE_LEVEL)) ERR; */
    /*     if (H5Pset_fletcher32(propid)) ERR; */
    /*     if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR; */
    /*     if ((datasetid = H5Dcreate1(grpid, VAR_BOOL_NAME, H5T_NATIVE_HBOOL, */
    /*                                spaceid, propid)) < 0) ERR; */
    /*     if (H5Dwrite(datasetid, H5T_NATIVE_HBOOL, H5S_ALL, H5S_ALL, H5P_DEFAULT, */
    /*                  bool_out) < 0) ERR; */
    /*     if (H5Dclose(datasetid) < 0 || */
    /*         H5Pclose(propid) < 0 || */
    /*         H5Sclose(spaceid) < 0 || */
    /*         H5Gclose(grpid) < 0 || */
    /*         H5Fclose(fileid) < 0) */
    /*         ERR; */

    /*     /\* Now reopen the file and check. *\/ */
    /*     if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR; */
    /*     if ((grpid = H5Gopen1(fileid, GRP_NAME)) < 0) ERR; */
    /*     if ((datasetid = H5Dopen1(grpid, VAR_BOOL_NAME)) < 0) ERR; */
    /*     if ((propid = H5Dget_create_plist(datasetid)) < 0) ERR; */

    /*     /\* The possible values of filter (which is just an int) can be */
    /*      * found in H5Zpublic.h. *\/ */
    /*     if ((num_filters = H5Pget_nfilters(propid)) < 0) ERR; */
    /*     if (num_filters != 3) ERR; */
    /*     for (unsigned int f = 0; f < num_filters; f++) */
    /*     { */
    /*         if ((filter = H5Pget_filter2(propid, f, &flags, &cd_nelems, cd_values, */
    /*                                      namelen, name, &filter_config)) < 0) ERR; */
    /*         switch (filter) */
    /*         { */
    /*         case H5Z_FILTER_SHUFFLE: */
    /*             found_shuffle++; */
    /*             break; */
    /*         case H5Z_FILTER_FLETCHER32: */
    /*             found_fletcher32++; */
    /*             break; */
    /*         case H5Z_FILTER_DEFLATE: */
    /*             found_deflate++; */
    /*             if (cd_nelems != 1 || cd_values[0] != DEFLATE_LEVEL || */
    /*                 strcmp(name, DEFLATE_NAME)) ERR; */
    /*             break; */
    /*         default: */
    /*             break; */
    /*         } */
    /*     } */
    /*     if (!found_fletcher32 || !found_deflate || !found_shuffle) ERR; */

    /*     if (H5Dclose(datasetid) < 0 || */
    /*         H5Pclose(propid) < 0 || */
    /*         H5Gclose(grpid) < 0 || */
    /*         H5Fclose(fileid) < 0) */
    /*         ERR; */
    /* } */

    /* SUMMARIZE_ERR; */
/*     printf("*** Checking using szip and zlib on same var..."); */
/* #define BOTH_VAR_NAME "szip_var" */
/* #define BOTH_DIM1_LEN 50 */
/* #define NUM_FILE 5 */
/* #define MAX_STR 80 */
/*     { */
/*         int data[BOTH_DIM1_LEN]; */
/*         hid_t plistid; */
/*         hsize_t chunksize[NDIM1] = {BOTH_DIM1_LEN}; */
/*         int options_mask = 32, pixels_per_block = 4; */
/*         int deflate_level = 3; */
/*         hsize_t my_dims[NDIM1]; */
/*         int i, f; */

/*         /\* Create data. *\/ */
/*         for (i = 0; i < BOTH_DIM1_LEN; i++) */
/*             data[i] = i; */

/*         /\* Run test 4 times. *\/ */
/*         for (f = 0; f < NUM_FILE; f++) */
/*         { */
/*             char file_name[MAX_STR * 2 + 1]; */
/*             char desc[NUM_FILE][MAX_STR + 1] = {"uncompressed", "zlib", */
/*                                                 "szip", "zlib_and_szip", */
/*                                                 "szip_and_zlib"}; */

/*             /\* Open file and create group. *\/ */
/*             snprintf(file_name, sizeof(file_name), "%s_%s.h5", TEST_NAME, desc[f]); */
/*             if ((fileid = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, */
/*                                     H5P_DEFAULT)) < 0) ERR; */
/*             if ((grpid = H5Gcreate1(fileid, GRP_NAME, 0)) < 0) ERR; */

/*             /\* Create dataset creation property list. *\/ */
/*             if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR; */

/*             /\* Turn on chunking. *\/ */
/*             if (H5Pset_chunk(plistid, NDIM1, chunksize) < 0) ERR; */

/*             /\* Turn off object tracking times in HDF5 (as netcdf-4 does). *\/ */
/*             if (H5Pset_obj_track_times(plistid, 0) < 0) ERR; */

/*             /\* Turn on compression for some files. *\/ */
/*             switch (f) */
/*             { */
/*             case 1: */
/*                 if (H5Pset_deflate(plistid, deflate_level) < 0) */
/*                     break; */
/*             case 2: */
/*                 if (H5Pset_szip(plistid, options_mask, pixels_per_block) < 0) ERR; */
/*                 break; */
/*             case 3: */
/*                 if (H5Pset_deflate(plistid, deflate_level) < 0) */
/*                 if (H5Pset_szip(plistid, options_mask, pixels_per_block) < 0) ERR; */
/*                 break; */
/*             case 4: */
/*                 if (H5Pset_szip(plistid, options_mask, pixels_per_block) < 0) ERR; */
/*                 if (H5Pset_deflate(plistid, deflate_level) < 0) */
/*                 break; */
/*             } */


/*             /\* Create a space. *\/ */
/*             my_dims[0] = BOTH_DIM1_LEN; */
/*             if ((spaceid = H5Screate_simple(1, my_dims, my_dims)) < 0) ERR; */

/*             /\* Create a dataset. *\/ */
/*             if ((datasetid = H5Dcreate2(grpid, BOTH_VAR_NAME, H5T_NATIVE_INT, */
/*                                         spaceid, H5P_DEFAULT, plistid, */
/*                                         H5P_DEFAULT)) < 0) ERR; */

/*             /\* Write data. *\/ */
/*             if (H5Dwrite(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, */
/*                          data) < 0) ERR; */

/*             /\* Release resources. *\/ */
/*             if (H5Dclose(datasetid) < 0 || */
/*                 H5Sclose(spaceid) < 0 || */
/*                 H5Pclose(plistid) < 0 || */
/*                 H5Gclose(grpid) < 0 || */
/*                 H5Fclose(fileid) < 0) */
/*                 ERR; */

/*             { */
/*                 /\* Now reopen the file and check. *\/ */
/*                 int data_in[BOTH_DIM1_LEN]; */
/*                 hid_t native_did; */
/*                 int i; */

/*                 if ((fileid = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR; */
/*                 if ((grpid = H5Gopen1(fileid, GRP_NAME)) < 0) ERR; */

/*                 if ((native_did = H5Dopen1(grpid, BOTH_VAR_NAME)) < 0) ERR; */

/*                 if (H5Dread(native_did, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, */
/*                             data_in) < 0) ERR; */
/*                 for (i = 0; i < BOTH_DIM1_LEN; i++) */
/*                     if (data[i] != data_in[i]) ERR; */

/*                 if (H5Dclose(native_did) < 0 || */
/*                     H5Gclose(grpid) < 0 || */
/*                     H5Fclose(fileid) < 0) */
/*                     ERR; */
/*             } */
/*         } /\* next file *\/ */
/*     } */
/*     SUMMARIZE_ERR; */
/* #endif /\* HAVE_H5Z_SZIP *\/ */
    FINAL_RESULTS;
}
