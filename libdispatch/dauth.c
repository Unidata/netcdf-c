/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See COPYRIGHT for license information.
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
    char* uri_hostport = NULL;

#if 0
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
#endif

    if(uri != NULL)
      uri_hostport = NC_combinehostport(uri);
    else
      return NC_EDAP; /* Generic EDAP error. */
    setdefaults(auth);

    /* Note, we still must do this function even if
       ncrc_globalstate.rc.ignore is set in order
       to getinfo e.g. host+port  from url
    */

    setauthfield(auth,"HTTP.DEFLATE",
		      NC_rclookup("HTTP.DEFLATE",uri_hostport));
    setauthfield(auth,"HTTP.VERBOSE",
			NC_rclookup("HTTP.VERBOSE",uri_hostport));
    setauthfield(auth,"HTTP.TIMEOUT",
			NC_rclookup("HTTP.TIMEOUT",uri_hostport));
    setauthfield(auth,"HTTP.USERAGENT",
			NC_rclookup("HTTP.USERAGENT",uri_hostport));
    setauthfield(auth,"HTTP.COOKIEFILE",
			NC_rclookup("HTTP.COOKIEFILE",uri_hostport));
    setauthfield(auth,"HTTP.COOKIE_FILE",
			NC_rclookup("HTTP.COOKIE_FILE",uri_hostport));
    setauthfield(auth,"HTTP.COOKIEJAR",
			NC_rclookup("HTTP.COOKIEJAR",uri_hostport));
    setauthfield(auth,"HTTP.COOKIE_JAR",
			NC_rclookup("HTTP.COOKIE_JAR",uri_hostport));
    setauthfield(auth,"HTTP.PROXY.SERVER",
			NC_rclookup("HTTP.PROXY.SERVER",uri_hostport));
    setauthfield(auth,"HTTP.PROXY_SERVER",
			NC_rclookup("HTTP.PROXY_SERVER",uri_hostport));
    setauthfield(auth,"HTTP.SSL.VALIDATE",
			NC_rclookup("HTTP.SSL.VALIDATE",uri_hostport));
    setauthfield(auth,"HTTP.SSL.CERTIFICATE",
			NC_rclookup("HTTP.SSL.CERTIFICATE",uri_hostport));
    setauthfield(auth,"HTTP.SSL.KEY",
			NC_rclookup("HTTP.SSL.KEY",uri_hostport));
    setauthfield(auth,"HTTP.SSL.KEYPASSWORD",
			NC_rclookup("HTTP.SSL.KEYPASSWORD",uri_hostport));
    setauthfield(auth,"HTTP.SSL.CAINFO",
			NC_rclookup("HTTP.SSL.CAINFO",uri_hostport));
    setauthfield(auth,"HTTP.SSL.CAPATH",
			NC_rclookup("HTTP.SSL.CAPATH",uri_hostport));
    setauthfield(auth,"HTTP.SSL.VERIFYPEER",
			NC_rclookup("HTTP.SSL.VERIFYPEER",uri_hostport));
    setauthfield(auth,"HTTP.NETRC",
			NC_rclookup("HTTP.NETRC",uri_hostport));

    { /* Handle various cases for user + password */
      /* First, see if the user+pwd was in the original url */
      char* user = NULL;
      char* pwd = NULL;
      if(uri->user != NULL && uri->password != NULL) {
	    user = uri->user;
	    pwd = uri->password;
      } else {
   	    user = NC_rclookup("HTTP.CREDENTIALS.USER",uri_hostport);
	    pwd = NC_rclookup("HTTP.CREDENTIALS.PASSWORD",uri_hostport);
      }
      if(user != NULL && pwd != NULL) {
        user = strdup(user); /* so we can consistently reclaim */
        pwd = strdup(pwd);
      } else {
	    /* Could not get user and pwd, so try USERPASSWORD */
	    const char* userpwd = NC_rclookup("HTTP.CREDENTIALS.USERPASSWORD",uri_hostport);
	    if(userpwd != NULL) {
          ret = NC_parsecredentials(userpwd,&user,&pwd);
          if(ret) {nullfree(uri_hostport); return ret;}
	    }
      }
      setauthfield(auth,"HTTP.CREDENTIALS.USERNAME",user);
      setauthfield(auth,"HTTP.CREDENTIALS.PASSWORD",pwd);
      nullfree(user);
      nullfree(pwd);
      nullfree(uri_hostport);
>>>>>>> master
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

