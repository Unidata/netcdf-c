/* Copyright 2026 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   Test nc_set_meta_block_size and nc_get_meta_block_size.
   Pattern follows nc_test4/tst_alignment.c.

   @author Ward Fisher
*/

#include <nc_tests.h>
#include "err_macros.h"
#include <hdf5.h>
#include <string.h>

#define FILE_NAME  "tst_meta_block_size.nc"
#define FILE_NAME2 "tst_meta_block_size2.nc"
#define META_BLOCK_SIZE ((size_t)1048576)  /* 1 MiB */
#define DIM_LEN 64

int
main(int argc, char **argv)
{
    int ncid, varid, dimid;
    size_t retrieved;
    float data_out[DIM_LEN], data_in[DIM_LEN];
    hid_t fapl_id;
    hsize_t hdf5_retrieved;
    int i;

    printf("\n*** Testing nc_set_meta_block_size and nc_get_meta_block_size.\n");

    /*
     * Test 1: nc_set_meta_block_size / nc_get_meta_block_size  
     */
    printf("*** Test 1: round-trip nc_set_meta_block_size / nc_get_meta_block_size...");
    {
        if (nc_set_meta_block_size(META_BLOCK_SIZE)) ERR;
        if (nc_get_meta_block_size(&retrieved)) ERR;
        if (retrieved != META_BLOCK_SIZE) ERR;
    }
    SUMMARIZE_ERR;

    /*
     * Test 2: create and write a netCDF-4 file while the override is set. 
     */
    printf("*** Test 2: create and write a netCDF-4 file with meta_block_size set...");
    {
        size_t chunks[1];

        if (nc_set_meta_block_size(META_BLOCK_SIZE)) ERR;
        if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
        if (nc_def_dim(ncid, "d0", DIM_LEN, &dimid)) ERR;
        if (nc_def_var(ncid, "v0", NC_FLOAT, 1, &dimid, &varid)) ERR;
        chunks[0] = DIM_LEN;
        if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
        if (nc_enddef(ncid)) ERR;
        for (i = 0; i < DIM_LEN; i++) data_out[i] = (float)i;
        if (nc_put_var_float(ncid, varid, data_out)) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    /*
     * Test 3: data written with the override active is readable. 
     */
    printf("*** Test 3: read back data from file written with meta_block_size set...");
    {
        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, "v0", &varid)) ERR;
        if (nc_get_var_float(ncid, varid, data_in)) ERR;
        for (i = 0; i < DIM_LEN; i++)
            if (data_in[i] != data_out[i]) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    /*
     * Test 4: HDF5 FAPL API round-trip.                                   
     * Verifies H5Pset_meta_block_size / H5Pget_meta_block_size behave as  
     * expected.                  
     */
    printf("*** Test 4: HDF5 FAPL meta_block_size round-trip...");
    {
        if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
        if (H5Pset_meta_block_size(fapl_id, (hsize_t)META_BLOCK_SIZE) < 0) ERR;
        if (H5Pget_meta_block_size(fapl_id, &hdf5_retrieved) < 0) ERR;
        if (hdf5_retrieved != (hsize_t)META_BLOCK_SIZE) ERR;
        if (H5Pclose(fapl_id) < 0) ERR;
    }
    SUMMARIZE_ERR;

    /* 
     * Test 5: setting size to 0 disables the override.             
     * A file created after the reset should succeed normally.
     */
    printf("*** Test 5: nc_set_meta_block_size(0) disables the override...");
    {
        if (nc_set_meta_block_size(0)) ERR;
        if (nc_get_meta_block_size(&retrieved)) ERR;
        if (retrieved != 0) ERR;
        if (nc_create(FILE_NAME2, NC_NETCDF4, &ncid)) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    FINAL_RESULTS;
}