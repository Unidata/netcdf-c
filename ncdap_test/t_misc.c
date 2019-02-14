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
#include "nctestserver.h"

#define FURL "%s"
static char url[4096];

#ifdef DEBUG
static void
CHECK(int e, const char* msg)
{
    if(e == NC_NOERR) return;
    if(msg == NULL) msg = "Error";
    printf("%s: %s\n", msg, nc_strerror(e));
    exit(1);
}
#endif

static void
XFAIL(int e, const char* msg)
{
    if(e == NC_NOERR) return;
    if(msg == NULL) msg = "XFAIL";
    printf("%s: %s\n", msg, nc_strerror(e));
}

int
main()
{
    int ncid,retval;
    char* serverlist = NULL;
    char* svcurl = NULL;
    const char* servlet = "dts";

#ifdef REMOTETESTSERVERS
    serverlist = strdup(REMOTETESTSERVERS);
#endif

    if(serverlist == NULL || strlen(serverlist) == 0) {
	fprintf(stderr,"Cannot determine a server list");
	exit(1);
    }
    svcurl = nc_findtestserver(servlet,0,serverlist);
    if(svcurl == NULL) {
	fprintf(stderr,"not found: %s\n",servlet);
	exit(1);
    }

    snprintf(url,sizeof(url),FURL,svcurl);

    printf("Testing: Misc. Tests url=|%s|\n",url);
    retval = nc_open(url, 0, &ncid);
    XFAIL(retval,"*** XFail : No trailing slash in url");
    retval = nc_close(ncid);
    /* cleanup */
    free(serverlist);
    free(svcurl);
    return 0;
}
