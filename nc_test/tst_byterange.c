/*********************************************************************
 *   Copyright 1996-2018, UCAR/Unidata
 *   See COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include "unistd.h"
#endif
#ifdef _WIN32
#include <io.h>
#endif

#include "netcdf.h"

/*
https://github.com/Unidata/netcdf-c/issues/1251
*/

struct TESTURLS {
    int format; /* instance of NC_FORMATX_XXX */
    const char* url;
} testurls[] = {
{NC_FORMAT_CLASSIC,"http://149.165.169.123:8080/thredds/fileServer/testdata/2004050300_eta_211.nc#bytes"},
#ifdef USE_NETCDF4
{NC_FORMAT_NETCDF4,"http://noaa-goes16.s3.amazonaws.com/ABI-L1b-RadC/2017/059/03/OR_ABI-L1b-RadC-M3C13_G16_s20170590337505_e20170590340289_c20170590340316.nc#mode=bytes"},
#endif
{0,NULL}
};

static int lineno = 0;

static int
fail(int ret)
{
    if(ret != NC_NOERR) {
        fprintf(stderr,"*** Fail: line: %d: (%d) %s\n", lineno, ret, nc_strerror(ret));
        fflush(stderr);
    }
    return ret;
}

static int
dotest(struct TESTURLS* test)
{
    int ret = NC_NOERR;
    int ncid;
    int format = -1;

    /* First, try to open the url */
    if((ret = nc_open(test->url,0,&ncid))) return fail(ret);

    /* Verify format */
    if((ret = nc_inq_format(ncid,&format))) return fail(ret);
    if(format != test->format) {
	printf("%s: format mismatch: expected %d received %d\n",__FILE__,test->format,format);
	return fail(NC_EINVAL);
    }

    if((ret = nc_close(ncid))) return fail(ret);

    return ret;
}

int
main()
{
    int ret = NC_NOERR;
    struct TESTURLS* test = NULL;

    for(test=testurls;test->format;test++) {
	if((ret=dotest(test))) goto done;
    }

done:
    return ret;
}
