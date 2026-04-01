/* Regression test for GitHub issue #2212:
   HDF error on reading back NC_VLEN variable with fill value and chunking.

   HDF5 does not support H5Pset_fill_value for VLEN types on chunked datasets.
   The fix is to skip H5Pset_fill_value for NC_VLEN in nc4_def_var_hdf5.
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "tst_vlen_fill_chunk.nc"

int
main(int argc, char **argv)
{
    int ncid, typeid, dimid, varid;
    nc_vlen_t fill;
    nc_vlen_t write_data[2];
    nc_vlen_t read_data[4];
    int fval[] = {-1};
    int b0[] = {10, 20};
    int b1[] = {30};
    size_t start, count;
    size_t chunksizes[1] = {2};

    printf("\n*** Testing VLEN with fill value and chunking (issue #2212).\n");
    printf("*** testing VLEN(int) + chunking + explicit fill value...");

    fill.len = 1;
    fill.p = fval;

    write_data[0].len = 2; write_data[0].p = b0;
    write_data[1].len = 1; write_data[1].p = b1;

    /* Create a VLEN variable with chunking and an explicit fill value. */
    if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_vlen(ncid, "vl", NC_INT, &typeid)) ERR;
    if (nc_def_dim(ncid, "x", NC_UNLIMITED, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;
    if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksizes)) ERR;
    if (nc_put_att(ncid, varid, "_FillValue", typeid, 1, &fill)) ERR;

    /* Write 2 elements starting at offset 1; offset 0 is fill. */
    start = 1; count = 2;
    if (nc_put_vara(ncid, varid, &start, &count, write_data)) ERR;
    if (nc_close(ncid)) ERR;

    /* Read back all 3 elements. */
    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    memset(read_data, 0, sizeof(read_data));
    start = 0; count = 3;
    if (nc_get_vara(ncid, varid, &start, &count, read_data)) ERR;

    /* Element 1 and 2 must match what was written. */
    if (read_data[1].len != 2) ERR;
    if (((int*)read_data[1].p)[0] != 10 || ((int*)read_data[1].p)[1] != 20) ERR;
    if (read_data[2].len != 1) ERR;
    if (((int*)read_data[2].p)[0] != 30) ERR;

    if (nc_reclaim_data(ncid, typeid, read_data, 3)) ERR;
    if (nc_close(ncid)) ERR;

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
