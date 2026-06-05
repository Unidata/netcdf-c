/* This is part of the netCDF package.
   Copyright 2026 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. This test is for various HDF5 settings which may improve performance.

   Ed Hartnett, 6/4/2026
*/

#include "err_macros.h"
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_file_perf.h5"
#define GRP_NAME "Dectectives"
#define STR_LEN 255

/* Prototype from tst_utils.c. */
int nc4_timeval_subtract(struct timeval *result, struct timeval *x,
                         struct timeval *y);
int
main()
{
    struct timeval start_time, end_time, diff_time;
    double sec;

    printf("\n*** Checking HDF5 file functions.\n");
    printf("*** Checking HDF5 metadata fragmentation and read times...\n");
#define OPAQUE_SIZE 20
#define OPAQUE_NAME "type"
#define ATT_NAME "att_name"
#define DIM_LEN 3
    {
        hid_t fileid, access_plist, typeid, spaceid, attid, fapl_id, grpid;
        hsize_t dims[1]; /* netcdf attributes always 1-D. */
        unsigned char data[DIM_LEN][OPAQUE_SIZE];
        hsize_t num_obj, i;
        int obj_class;
        char obj_name[STR_LEN + 1];
        H5T_class_t class;
        int j, k;
        hid_t tmp1;

        H5open();

        /* Initialize some data. */
        for (j = 0; j < DIM_LEN; j++)
            for (k = 0; k < OPAQUE_SIZE; k++)
                data[j][k] = 42;

        /* Set the access list so that closes will fail if something is
         * still open in the file. */
        tmp1 = H5P_FILE_ACCESS;
        if ((access_plist = H5Pcreate(tmp1)) < 0) ERR;
        if (H5Pset_fclose_degree(access_plist, H5F_CLOSE_SEMI)) ERR;

        /* Start timing. */
        if (gettimeofday(&start_time, NULL))
            ERR;
        
        /* Create file. */
        if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
                                access_plist)) < 0) ERR;
        /* Add an opaque type. */
        if ((typeid = H5Tcreate(H5T_OPAQUE, OPAQUE_SIZE)) < 0) ERR;
        if (H5Tcommit1(fileid, OPAQUE_NAME, typeid) < 0) ERR;

        /* Add attribute of this type. */
        dims[0] = 3;
        if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
        if ((attid = H5Acreate1(fileid, ATT_NAME, typeid, spaceid,
                               H5P_DEFAULT)) < 0) ERR;
        if (H5Awrite(attid, typeid, data) < 0) ERR;

        if (H5Aclose(attid) < 0) ERR;
        if (H5Sclose(spaceid) < 0) ERR;
        if (H5Tclose(typeid) < 0) ERR;
        if (H5Fclose(fileid) < 0) ERR;
        if (H5Pclose(access_plist) < 0) ERR;

        if (H5Eset_auto1(NULL, NULL) < 0) ERR;

        /* Stop timer and report elapsed time. */
        if (gettimeofday(&end_time, NULL)) ERR;
        if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
        sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
        printf("write time:\t%.3g sec\n", sec);

        /* Start timing. */
        if (gettimeofday(&start_time, NULL))
            ERR;
        
        /* Reopen the file. */
        if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
        /*if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI)) ERR;*/
        if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
        if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
        if ((grpid = H5Gopen1(fileid, "/")) < 0) ERR;

        if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
        for (i = 0; i < num_obj; i++)
        {
            if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
            if (H5Gget_objname_by_idx(grpid, i, obj_name,
                                      STR_LEN) < 0) ERR;
            if (obj_class != H5G_TYPE) ERR;
            if ((typeid = H5Topen1(grpid, obj_name)) < 0) ERR;
            if ((class = H5Tget_class(typeid)) < 0) ERR;
            if (class != H5T_OPAQUE) ERR;
            if (!(H5Tget_size(typeid))) ERR;
        }

        /* Close everything. */
        if (H5Pclose(fapl_id)) ERR;
        if (H5Gclose(grpid) < 0) ERR;
        /*if (H5Tclose(typeid) < 0) ERR;*/
        if (H5Fclose(fileid) < 0) ERR;

        /* Stop timer and report elapsed time. */
        if (gettimeofday(&end_time, NULL)) ERR;
        if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
        sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
        printf("read time:\t%.3g sec\n", sec);

    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
