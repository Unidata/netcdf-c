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
#include "netcdf_filter.h"

#define ERR(r) {fprintf(stderr,"fail: line %d: (%d) %s\n",__LINE__,(r),nc_strerror((r)));}

#define FILENAME "file://tmp_nczfilter.nc#mode=nczarr,file"

#define FILTERID 1

int
main(int argc, char **argv)
{
    int ret = NC_NOERR;
    int ncid;
    int dimid;
    int varid;
    size_t chunksizes[1] = {4};
    unsigned params[1] = {9};
    char* furl = NULL;
    int data[4] = {17,18,19,20};
    size_t nfilters;
    unsigned int filterids[8];
    size_t nparams;
    unsigned inqparams[8];
    
    if(argc == 1)
	furl = FILENAME;
    else
	furl = argv[1];

    if ((ret=nc_create(furl, NC_NETCDF4, &ncid))) ERR(ret);
    if ((ret=nc_def_dim(ncid, "d", 4, &dimid))) ERR(ret);
    if ((ret=nc_def_var(ncid, "v", NC_INT, 1, &dimid, &varid))) ERR(ret);
    if((ret=nc_def_var_chunking(ncid,varid,NC_CHUNKED,chunksizes))) ERR(ret);
    if((ret=nc_def_var_filter(ncid,varid,FILTERID,1,params))) ERR(ret);
    if((ret=nc_put_var(ncid,varid,data))) ERR(ret);
    if ((ret=nc_close(ncid))) ERR(ret);

    if ((ret=nc_open(furl, 0, &ncid))) ERR(ret);
    if ((ret=nc_inq_varid(ncid, "v", &varid))) ERR(ret);

    if((ret=nc_inq_var_filter_ids(ncid, varid, &nfilters, filterids))) ERR(ret);
    if(nfilters != 1) ERR(NC_EFILTER);
    if(filterids[0] != FILTERID) ERR(NC_EFILTER);
    printf("nfilters=%u filterids[0]=%u\n",(unsigned)nfilters,(unsigned)filterids[0]);
    if((ret=nc_inq_var_filter_info(ncid, varid, filterids[0], &nparams, inqparams))) ERR(ret);
    if(nparams != 1) ERR(NC_EFILTER);
    if(inqparams[0] != params[0]) ERR(NC_EFILTER);
    printf("nparams=%u params[0]=%u\n",(unsigned)nparams,(unsigned)inqparams[0]);
    
    if ((ret=nc_close(ncid))) ERR(ret);

    nc_finalize();
    return 0;
}
