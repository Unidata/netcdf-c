/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "t_srcdir.h"

#define VAR "i32"

#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

#undef DEBUG

int
main()
{
    int ncid, varid;
    int retval;
    int i32[100];
    size_t start[1];
    size_t count[1];
    int ok = 1;
    const char* topsrcdir;
    char url[4096];


    topsrcdir = gettopsrcdir();

    strncpy(url,"file://",sizeof(url));
    strlcat(url,topsrcdir,sizeof(url));
    strlcat(url,"/ncdap_test/testdata3/test.02",sizeof(url));

    if ((retval = nc_open(url, 0, &ncid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, VAR, &varid)))
       ERR(retval);

    start[0] = 0;
    count[0] = 26;
    if ((retval = nc_get_vara_int(ncid, varid, start, count, i32)))
    if(retval != NC_EINVALCOORDS) {
	printf("nc_get_vara_int did not return NC_EINVALCOORDS");
	ok = 0;
    }

    nc_close(ncid);

    printf(ok?"*** PASS\n":"*** FAIL\n");
    return 0;
}
