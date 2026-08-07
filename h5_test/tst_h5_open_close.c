/* This is part of the netCDF package.
   Copyright 2026 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Pure-HDF5 regression test for the repeated open/close memory growth
   reported in https://github.com/Unidata/netcdf-c/issues/2626.

   This test exercises HDF5 directly using the same H5F_CLOSE_WEAK +
   open/close pattern that netCDF-4 uses. It does not attempt to
   monitor memory itself; run it under address sanitizer (ASAN) to
   verify that no memory is leaked.

   Ed Hartnett, 3/17/26
*/

#include "h5_err_macros.h"
#include <hdf5.h>

#define FILE_NAME "tst_h5_open_close.h5"
#define NITER 500

int
main()
{
    hid_t fid, sid, did, fapl;
    hsize_t dims[1] = {10};
    double data[10] = {0};
    int i;

    printf("\n*** Checking pure-HDF5 open/close (issue #2626).\n");
    printf("*** Checking repeated H5Fopen/H5Fclose...");

    /* Create a small HDF5 file with one dataset. */
    if ((fid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC,
                         H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
    if ((sid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
    if ((did = H5Dcreate2(fid, "dummy", H5T_NATIVE_DOUBLE, sid,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
    if (H5Dwrite(did, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                 H5P_DEFAULT, data) < 0) ERR;
    if (H5Dclose(did) < 0) ERR;
    if (H5Sclose(sid) < 0) ERR;
    if (H5Fclose(fid) < 0) ERR;

    /* Use H5F_CLOSE_WEAK, matching netCDF-4's open behaviour. */
    if ((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
    if (H5Pset_fclose_degree(fapl, H5F_CLOSE_WEAK) < 0) ERR;

    /* Repeatedly open and close the file. ASAN will flag any leaks. */
    for (i = 0; i < NITER; i++)
    {
        if ((fid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl)) < 0) ERR;
        if (H5Fclose(fid) < 0) ERR;
    }

    if (H5Pclose(fapl) < 0) ERR;
    remove(FILE_NAME);

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
