/* This program uses the HDF5 SWMR mode to write to a file while a
 * second process simultaneously reads from it. As there are some
 * strict ordering requirements, this is orchestrated by an
 * accompanying shell script.
 *
 * Here, we create the file, create our dimensions and variables, then
 * close and reopen it to write data. We write half the data, pause,
 * then write the other half. This gives us the opportunity to see the
 * data get *updated* in the reader process.
*/

#include "test_swmr_common.h"
#include "netcdf.h"
#include <stddef.h>


int main(int argc, char** argv)
{
    int ncid, varid, dimids[NUM_DIMS];
    size_t i;
    int values[1];
    size_t start[NUM_DIMS] = {0}, count[NUM_DIMS] = {1};

    /* Create a file in SWMR mode for writing, create structure and close. */
    if (nc_create(FILE_NAME, NC_NETCDF4|NC_HDF5_SWMR, &ncid)) ERR;
    if (nc_def_dim(ncid, TIME_DIM_NAME, NC_UNLIMITED, &dimids[0])) ERR;
    if (nc_def_var(ncid, DEPTH_VAR_NAME, NC_INT, NUM_DIMS, dimids, &varid)) ERR;
    if (nc_close(ncid)) ERR;

    /* Open the file for SWMR writing, append data, and close. */
    if (nc_open(FILE_NAME, NC_WRITE|NC_HDF5_SWMR, &ncid)) ERR;
    if (nc_inq_varid(ncid, "depth", &varid)) ERR;

    /* Write a bunch of initial values */
    for (i = 0; i < TOTAL_LENGTH / 2; i++) {
        start[0] = i;
        values[0] = (int)i;
        if (nc_put_vara_int(ncid, varid, start, count, values)) ERR;
    }

    /* Pause here briefly so reader process has a chance to start */
    sleep(1);

    /* At this point, the reader process should've opened and read the
     * first half of the data so we can write the second half */
    for (i = TOTAL_LENGTH / 2; i < TOTAL_LENGTH; i++) {
        start[0] = i;
        values[0] = (int)i;
        if (nc_put_vara_int(ncid, varid, start, count, values)) ERR;
    }
    
    if (nc_close(ncid)) ERR;
}
