/* This program reads a file currently being modified by another
 * process. See test_hdf5_swmr_writer.c for details
*/

#include "test_swmr_common.h"
#include "netcdf.h"
#include <stdbool.h>
#include <stddef.h>


int main(int argc, char** argv)
{
    int ncid, varid;
    int values[1];
    size_t start[NUM_DIMS] = {0}, count[NUM_DIMS] = {1};

    /* Open the file for SWMR reading, verify data, and close. */
    if ((status = nc_open(FILE_NAME, NC_HDF5_SWMR, &ncid))) ERR_MSG;
    if ((status = nc_inq_varid(ncid, "depth", &varid))) ERR_MSG;

    int time_dim;
    if ((status = nc_inq_dimid(ncid, TIME_DIM_NAME, &time_dim))) ERR_MSG;

    size_t time_dim_len_start;
    if ((status = nc_inq_dimlen(ncid, time_dim, &time_dim_len_start))) ERR_MSG;

    // Read initial slice
    start[0] = time_dim_len_start - 1;
    if ((status = nc_get_vara_int(ncid, varid, start, count, values))) ERR_MSG;
    if (values[0] != (int)time_dim_len_start - 1) ERR;

    // Wait a little bit to be sure the writer process has written a few more slices
    sleep(1);

    size_t time_dim_len_end;
    if ((status = nc_inq_dimlen(ncid, time_dim, &time_dim_len_end))) ERR_MSG;

    if (time_dim_len_end <= time_dim_len_start) ERR;

    start[0] = time_dim_len_end - 1;
    if ((status = nc_get_vara_int(ncid, varid, start, count, values))) ERR_MSG;
    if (values[0] != (int)time_dim_len_end - 1) ERR;

    if ((status = nc_close(ncid))) ERR_MSG;
}
