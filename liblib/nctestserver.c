#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netcdf.h"
#include "ncdispatch.h"
#include "nclist.h"

#ifdef ENABLE_DAP2
extern int NCDAP2_ping(const char*);
#endif

#ifdef ENABLE_DAP4
extern int NCDAP4_ping(const char*);
#endif

#define MAXSERVERURL 4096

static char**
parseServers(const char* remotetestservers)
{
    char** servers = NULL;
    NClist* list = nclistnew();
    char* p;
    char* svc;
    p = strdup(remotetestservers);
    for(;;) {
	svc = p;
	p = strchr(svc,',');
	if(p != NULL) *p = '\0';
	nclistpush(list,strdup(svc));
	if(p == NULL) break;
	p++;
    }
    servers = (char**)nclistextract(list);
    nclistfreeall(list);
    return servers;
}

/**
Given a partial suffix path and a specified
protocol, test if a request to any of the test
servers + path returns some kind of result.
This indicates that the server is up and running.
Return the complete url for the server plus the path.
*/

char*
nc_findtestserver(const char* path, int isdap4, const char* serverlist)
{
    char** svc;
    char url[MAXSERVERURL];

    if((svc = parseServers(serverlist))==NULL) {
	fprintf(stderr,"cannot parse test server list: %s\n",serverlist);
	return NULL;
    }
    for(;*svc;svc++) {
	if(*svc == NULL || strlen(*svc) == 0)
	    return NULL;
        if(path == NULL) path = "";
        if(strlen(path) > 0 && path[0] == '/')
	    path++;
        snprintf(url,MAXSERVERURL,"http://%s/%s",*svc,path);
#ifdef ENABLE_DAP2
	if(!isdap4 && NCDAP2_ping(url) == NC_NOERR)
	    return strdup(url);
#endif
#ifdef ENABLE_DAP4
	if(isdap4 && NCDAP4_ping(url) == NC_NOERR)
	    return strdup(url);
#endif
    }
    return NULL;
}

