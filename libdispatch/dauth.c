/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include "netcdf.h"
#include "ncbytes.h"
#include "ncuri.h"
#include "ncauth.h"
#include "nclog.h"
#include "ncwinpath.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

#undef MEMCHECK
#define MEMCHECK(x) if((x)==NULL) {goto nomem;} else {}

const NCRCFIELDS NC_dfaltfields = {
-1, /*int   HTTP_VERBOSE;*/
-1, /*int   HTTP_DEFLATE;*/
NULL, /*char* HTTP_COOKIEJAR;*/
NULL, /*char* HTTP_CREDENTIALS_USERNAME;*/
NULL, /*char* HTTP_CREDENTIALS_PASSWORD;*/
NULL, /*char* HTTP_SSL_CERTIFICATE;*/
NULL, /*char* HTTP_SSL_KEY;*/
NULL, /*char* HTTP_SSL_KEYPASSWORD;*/
NULL, /*char* HTTP_SSL_CAPATH;*/
-1, /*int   HTTP_SSL_VALIDATE;*/
1800, /*long  HTTP_TIMEOUT;*/
NULL, /*char* HTTP_PROXY_SERVER;*/
-1, /*long  HTTP_READ_BUFFERSIZE;*/
NULL, /*char* HTTP_NETRC;*/
NULL, /*char* HTTP_USERAGENT;*/
{0,0,0,0}
};

/* Forward */
static int setauthfield(NCauth* auth, const char* flag, NCRCFIELDS*);

/**************************************************/
/* External Entry Points */

static int
parseproxy(NCauth* auth, const char* surl)
{
    int ret = NC_NOERR;
    NCURI* uri = NULL;
    if(surl == NULL || strlen(surl) == 0)
	return (NC_NOERR); /* nothing there*/
    if(ncuriparse(surl,&uri) != NCU_OK)
	return (NC_EURL);
    auth->proxy.user = uri->user;
    auth->proxy.pwd = uri->password;
    auth->proxy.host = strdup(uri->host);
    if(uri->port != NULL)
        auth->proxy.port = atoi(uri->port);
    else
        auth->proxy.port = 80;
    return (ret);
}

int
NC_authsetup(NCauth* auth, NCRCFIELDS* rcfields)
{
    int ret = NC_NOERR;
    if(rcfields != NULL) {
    setauthfield(auth,"HTTP.DEFLATE",rcfields);
    setauthfield(auth,"HTTP.VERBOSE",rcfields);
    setauthfield(auth,"HTTP.TIMEOUT",rcfields);
    setauthfield(auth,"HTTP.USERAGENT",rcfields);
    setauthfield(auth,"HTTP.VERBOSE",rcfields);
    setauthfield(auth,"HTTP.COOKIEJAR",rcfields);
    setauthfield(auth,"HTTP.PROXY",rcfields);
    setauthfield(auth,"HTTP.SSL.VALIDATE",rcfields);
    setauthfield(auth,"HTTP.SSL.CERTIFICATE",rcfields);
    setauthfield(auth,"HTTP.SSL.KEY",rcfields);
    setauthfield(auth,"HTTP.SSL.KEYPASSWORD",rcfields);
    setauthfield(auth,"HTTP.SSL.CAPATH",rcfields);
    setauthfield(auth,"HTTP.SSL.VERIFYPEER",rcfields);
    setauthfield(auth,"HTTP.NETRC",rcfields);
    setauthfield(auth,"HTTP.CREDENTIALS.USER",rcfields);
    setauthfield(auth,"HTTP.CREDENTIALS.PASSWORD",rcfields);
    }
    return (ret);
}

void
NC_authclear(NCauth* auth)
{
    if(auth->curlflags.cookiejarcreated) {
#ifdef _MSC_VER
        DeleteFile(auth->curlflags.cookiejar);
#else
        remove(auth->curlflags.cookiejar);
#endif
    }
    nullfree(auth->curlflags.useragent);
    nullfree(auth->curlflags.cookiejar);
    nullfree(auth->curlflags.netrc);
    nullfree(auth->ssl.certificate);
    nullfree(auth->ssl.key);
    nullfree(auth->ssl.keypasswd);
    nullfree(auth->ssl.capath);
    nullfree(auth->proxy.host);
    nullfree(auth->proxy.user);
    nullfree(auth->proxy.pwd);
    nullfree(auth->creds.user);
    nullfree(auth->creds.pwd);
}

/**************************************************/

static int
setauthfield(NCauth* auth, const char* flag, NCRCFIELDS* rcfields)
{
    int ret = NC_NOERR;
    if(strcmp(flag,"HTTP.DEFLATE")==0) {
        if(rcfields->HTTP_DEFLATE >= 0)
	    auth->curlflags.compress = rcfields->HTTP_DEFLATE;
    } else if(strcmp(flag,"HTTP.VERBOSE")==0) {
        if(rcfields->HTTP_VERBOSE >= 0)
	    auth->curlflags.verbose = rcfields->HTTP_VERBOSE;
    } else if(strcmp(flag,"HTTP.TIMEOUT")==0) {
        if(rcfields->HTTP_TIMEOUT >= 0)
	    auth->curlflags.timeout = rcfields->HTTP_TIMEOUT;
    } else if(strcmp(flag,"HTTP.USERAGENT")==0) {
        if(rcfields->HTTP_USERAGENT != NULL) {
  	    nullfree(auth->curlflags.useragent);
	    auth->curlflags.useragent = strdup(rcfields->HTTP_USERAGENT);
            MEMCHECK(auth->curlflags.useragent);
	}
    }
    if(strcmp(flag,"HTTP.COOKIEJAR")==0) {
        if(rcfields->HTTP_COOKIEJAR != NULL) {
  	    nullfree(auth->curlflags.cookiejar);
	    auth->curlflags.cookiejar = strdup(rcfields->HTTP_COOKIEJAR);
            MEMCHECK(auth->curlflags.cookiejar);
	}
    }
    if(strcmp(flag,"HTTP.PROXY.SERVER")==0) {
	if(rcfields->HTTP_PROXY_SERVER != NULL) {
            ret = parseproxy(auth, rcfields->HTTP_PROXY_SERVER);
            if(ret != NC_NOERR) goto done;
	}
    }
    if(strcmp(flag,"HTTP.SSL.VALIDATE")==0) {
        if(rcfields->HTTP_SSL_VALIDATE >= 0) {
	    auth->ssl.verifypeer = rcfields->HTTP_SSL_VALIDATE;
	    auth->ssl.verifyhost = rcfields->HTTP_SSL_VALIDATE;
	}
    }
    if(strcmp(flag,"HTTP.SSL.CERTIFICATE")==0) {
        if(rcfields->HTTP_SSL_CERTIFICATE != NULL) {
	    nullfree(auth->ssl.certificate);
            auth->ssl.certificate = strdup(rcfields->HTTP_SSL_CERTIFICATE);
            MEMCHECK(auth->ssl.certificate);
	}
    }
    if(strcmp(flag,"HTTP.SSL.KEY")==0) {
        if(rcfields->HTTP_SSL_KEY != NULL) {
	    nullfree(auth->ssl.key);
            auth->ssl.key = strdup(rcfields->HTTP_SSL_KEY);
            MEMCHECK(auth->ssl.key);
	}
    }
    if(strcmp(flag,"HTTP.SSL.KEYPASSWORD")==0) {
        if(rcfields->HTTP_SSL_KEYPASSWORD != NULL) {
	    nullfree(auth->ssl.keypasswd);
            auth->ssl.keypasswd = strdup(rcfields->HTTP_SSL_KEYPASSWORD);
            MEMCHECK(auth->ssl.keypasswd);
	}
    }
    if(strcmp(flag,"HTTP.SSL.CAPATH")==0) {
        if(rcfields->HTTP_SSL_CAPATH != NULL) {
	    nullfree(auth->ssl.capath);
            auth->ssl.capath = strdup(rcfields->HTTP_SSL_CAPATH);
            MEMCHECK(auth->ssl.capath);
	}
    }
    if(strcmp(flag,"HTTP.NETRC")==0) {
	if(rcfields->HTTP_NETRC != NULL) {
            nullfree(auth->curlflags.netrc);
            auth->curlflags.netrc = strdup(rcfields->HTTP_NETRC);
            MEMCHECK(auth->curlflags.netrc);
	}
    }
    if(strcmp(flag,"HTTP.CREDENTIALS.USERNAME")==0) {
	if(rcfields->HTTP_CREDENTIALS_USERNAME != NULL) {
            nullfree(auth->creds.user);
            auth->creds.user = strdup(rcfields->HTTP_CREDENTIALS_USERNAME);
            MEMCHECK(auth->creds.user);
	}
    }
    if(strcmp(flag,"HTTP.CREDENTIALS.PASSWORD")==0) {
	if(rcfields->HTTP_CREDENTIALS_PASSWORD != NULL) {
            nullfree(auth->creds.pwd);
            auth->creds.pwd = strdup(rcfields->HTTP_CREDENTIALS_PASSWORD);
            MEMCHECK(auth->creds.pwd);
	}
    }

done:
    return (ret);

nomem:
    return (NC_ENOMEM);
}

