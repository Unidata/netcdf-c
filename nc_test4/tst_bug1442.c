/*! \file

Copyright 2019
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/
#include <nc_tests.h>
#include "err_macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_bug1442nc"

int
main(int argc, char **argv)
{
    /* Test bugfix for https://github.com/Unidata/netcdf-c/pull/1442 */
    /* that is using nc_get_vara_xxx() with a offset[] != 0 on a variable */
    /* indexed by a unlimited dimension, and the size taken by that variable */
    /* does not reach the actual size of that dimension */
    int status;
    int cdfid = -1;
    int unlimited_dim;
    int varid;
    int other_var;
    size_t start[1];
    size_t count[1];
    double three_zeros[3] = {0, 0, 0};
    double* two_double = (double*)calloc(2, sizeof(double));

    status = nc_create(FILENAME, NC_NETCDF4, &cdfid);
    if( status ) ERR;

    status = nc_def_dim(cdfid, "unlimited_dim", NC_UNLIMITED, &unlimited_dim);
    if( status ) ERR;

    status = nc_def_var(cdfid, "my_var", NC_DOUBLE, 1, &unlimited_dim, &varid);
    if( status ) ERR;

    status = nc_def_var(cdfid, "other_var", NC_DOUBLE, 1, &unlimited_dim, &other_var);
    if( status ) ERR;

    status = nc_enddef(cdfid);
    if( status ) ERR;

    /* Write 3 elements to set the size of the unlimited dim to 3 */
    start[0] = 0;
    count[0] = 3;
    status = nc_put_vara_double(cdfid, other_var, start, count, three_zeros);
    if( status ) ERR;

    /* Read 2 elements starting with index=1 */
    start[0] = 1;
    count[0] = 2;
    status = nc_get_vara_double(cdfid, varid, start, count, two_double);
    if( status ) ERR;

    if( two_double[0] != NC_FILL_DOUBLE ) ERR;
    if( two_double[1] != NC_FILL_DOUBLE ) ERR;

    status = nc_close(cdfid);
    if( status ) ERR;

    free(two_double);

    SUMMARIZE_ERR;
    FINAL_RESULTS;

    return 0;
}
