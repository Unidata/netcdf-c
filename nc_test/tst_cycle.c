/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test multiple cycles of nc_initialize...nc_finalize
*/

#include "config.h"
#include <nc_tests.h>
#include "err_macros.h"
#include <netcdf.h>

#define ERR2 { \
    err++; \
    fprintf(stderr, "Sorry! Unexpected result, %s, line: %d (%s)\n", \
    __FILE__, __LINE__, nc_strerror(stat));                          \
    exit(1); \
}

int
main(int argc, char **argv)
{
    int stat = NC_NOERR;

    printf("\n*** Testing initialize/finalize cycle.\n");

    /* Directly invoke nc_initialize...nc_finalize multiple times.*/
    /* Ideally do this test with some kind of memory checker to look
    for memory errors
    */

    if((stat = nc_initialize())) ERR;
    if((stat = nc_finalize())) ERR;
    if((stat = nc_initialize())) ERR;
    if((stat = nc_finalize())) ERR;
    if((stat = nc_initialize())) ERR;
    if((stat = nc_finalize())) ERR;

    SUMMARIZE_ERR;
    
    FINAL_RESULTS;
}
    
