/*! \file

Copyright 2019
University Corporation for Atmospheric Research/Unidata.

Author: Even Rouault

See \ref copyright file for more info.

*/
#include <nc_tests.h>
#include "err_macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_bug1442.nc"

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
    double five_zeros[5] = {0, 0, 0, 0, 0};
    double one_two[2] = {1, 2};
    double* four_double = (double*)calloc(4, sizeof(double));

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

    /* Write 5 elements to set the size of the unlimited dim to 5 */
    start[0] = 0;
    count[0] = 5;
    status = nc_put_vara_double(cdfid, other_var, start, count, five_zeros);
    if( status ) ERR;

    /* Write 2 elements in my_var */
    start[0] = 0;
    count[0] = 2;
    status = nc_put_vara_double(cdfid, varid, start, count, one_two);
    if( status ) ERR;

    /* Read 4 elements starting with index=1 */
    start[0] = 1;
    count[0] = 4;
    status = nc_get_vara_double(cdfid, varid, start, count, four_double);
    if( status ) ERR;

    if( four_double[0] != 2 ) ERR;
    if( four_double[1] != NC_FILL_DOUBLE ) ERR;
    if( four_double[2] != NC_FILL_DOUBLE ) ERR;
    if( four_double[3] != NC_FILL_DOUBLE ) ERR;

    status = nc_close(cdfid);
    if( status ) ERR;

    free(four_double);

    SUMMARIZE_ERR;
    FINAL_RESULTS;

    return 0;
}
