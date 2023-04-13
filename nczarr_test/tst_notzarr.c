/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test nczarr filter loading
   Author: Dennis Heimbigner
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "netcdf.h"

#define ERR(r) {fprintf(stderr,"fail: line %d: (%d) %s\n",__LINE__,(r),nc_strerror((r)));}

int
main(int argc, char **argv)
{
    int ret = NC_NOERR;
    int ncid;
    
    if(argc < 2) {
	fprintf(stderr,"Usage: tst_notzarr <url>\n");
	exit(1);
    }    
    ret = nc_open(argv[1],NC_NETCDF4,&ncid);
    printf("%d",ret);
    if(ret == NC_NOERR) nc_close(ncid);
    exit(0);
}
