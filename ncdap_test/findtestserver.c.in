/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nctestserver.h"

/* Support stringification of -D macros */
#define XSTRINGIFY(s) #s
#define STRINGIFY(s) XSTRINGIFY(s)


/**
usage: findtestserver dap2|dap4 suffix [serverlist]

Given a partial suffix path, try to find a
server for which a request to server + suffix
returns some kind of result using the
specified protocol.  This indicates that the
server is up and running.  Return the complete
url for the server plus the path.
If serverlist is present, then is should be a comma
separated list of servers (host+port) to try.
It defaults to REMOTETESTSERVERS.
*/

static void
usage()
{
    fprintf(stderr,"usage: findtestserver dap2|dap4 suffix [serverlist]\n");
    exit(1);
}


int
main(int argc, char** argv)
{
    char* url = NULL;
    const char* servlet = NULL;
    const char* proto = NULL;
    char* serverlist = NULL;
    int isdap4 = 0; /* 1 => dap4 */

    argc--; argv++;
    if(argc < 2)
	usage();
    proto = strdup(argv[0]);
    servlet = strdup(argv[1]);
    if(argc >= 3)
	serverlist = strdup(argv[2]);

#ifdef ENABLE_DAP
    if(strcasecmp(proto,"dap2")==0)
	isdap4 = 0;
    else
#endif
#ifdef ENABLE_DAP4
    if(strcasecmp(proto,"dap4")==0)
	isdap4 = 1;
    else
#endif
	usage();

    if(serverlist == NULL) {
#ifdef REMOTETESTSERVERS
	serverlist = strdup(REMOTETESTSERVERS);
#endif
    }
    if(serverlist == NULL || strlen(serverlist) == 0)
	fprintf(stderr,"Cannot determine a server list");

    url = nc_findtestserver(servlet,isdap4,serverlist);
    if(url == NULL) {
       url = "";
	fprintf(stderr,"not found: %s\n",servlet);
    }
    printf("%s",url);
    fflush(stdout);
    /* clean up */
    free(serverlist);
    free(url);
    exit(0);
}
