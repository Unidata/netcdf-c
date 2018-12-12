/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include "d4curlfunctions.h"

/* Define supported curl flags */
struct CURLFLAG curlopts[] = { 
#ifdef HAVE_CURLOPT_BUFFERSIZE
{"CURLOPT_BUFFERSIZE",CURLOPT_BUFFERSIZE,98,CF_LONG},
#endif
{"CURLOPT_PROXYUSERPWD",CURLOPT_PROXYUSERPWD,10006,CF_STRING},
{"CURLOPT_SSLCERT",CURLOPT_SSLCERT,10025,CF_STRING},
{"CURLOPT_SSLKEY",CURLOPT_SSLKEY,10087,CF_STRING},
#ifdef HAVE_CURLOPT_KEYPASSWD
{"CURLOPT_SSLKEYPASSWD",CURLOPT_SSLKEYPASSWD,CURLOPT_KEYPASSWD,CF_STRING},
#endif
{"CURLOPT_SSL_VERIFYHOST",CURLOPT_SSL_VERIFYHOST,81,CF_LONG},
{"CURLOPT_SSL_VERIFYPEER",CURLOPT_SSL_VERIFYPEER,64,CF_LONG},
#ifdef HAVE_CURLOPT_KEEPALIVE
{"CURLOPT_TCP_KEEPALIVE",CURLOPT_TCP_KEEPALIVE,213,CF_LONG},
#endif
{"CURLOPT_TIMEOUT",CURLOPT_TIMEOUT,13,CF_LONG},
{"CURLOPT_USERAGENT",CURLOPT_USERAGENT,10018,CF_STRING},
{"CURLOPT_USERPWD",CURLOPT_USERPWD,10005,CF_STRING},
{"CURLOPT_USE_SSL",CURLOPT_USE_SSL,119,CF_LONG},
{"CURLOPT_VERBOSE",CURLOPT_VERBOSE,41,CF_LONG},
{NULL,0}
};

struct CURLFLAG*
NCD4_curlflagbyname(const char* name)
{
    struct CURLFLAG* p;
    for(p=curlopts;p->name;p++) {
	if(strcmp(p->name,name)==0) return p;
    }
    return NULL;
}
