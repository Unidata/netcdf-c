/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include <curl/curl.h>
#include "ocinternal.h"
#include "occurlfunctions.h"
#include "ocdebug.h"

static int nflags = 0;
static long maxflag = -1;
static struct OCCURLFLAG* nameindices[26]; /* for radix access */
static struct OCCURLFLAG** flagindices = NULL; /* for radix access */

/* Define supported curlflags */

static struct OCCURLFLAG oc_curlflags[] = {
#ifdef HAVE_CURLOPT_BUFFERSIZE
{"CURLOPT_BUFFERSIZE",CURLOPT_BUFFERSIZE,98,CF_LONG},
#endif
{"CURLOPT_ACCEPT_ENCODING",CURLOPT_ENCODING,10102,CF_STRING},
{"CURLOPT_CAINFO",CURLOPT_CAINFO,10065,CF_STRING},
{"CURLOPT_CAPATH",CURLOPT_CAPATH,10097,CF_STRING},
{"CURLOPT_COOKIEFILE",CURLOPT_COOKIEFILE,10031,CF_STRING},
{"CURLOPT_COOKIEJAR",CURLOPT_COOKIEJAR,10082,CF_STRING},
{"CURLOPT_ERRORBUFFER",CURLOPT_ERRORBUFFER,10010,CF_STRING},
{"CURLOPT_FOLLOWLOCATION",CURLOPT_FOLLOWLOCATION,52,CF_LONG},
{"CURLOPT_HTTPAUTH",CURLOPT_HTTPAUTH,1007,CF_LONG},
#ifdef HAVE_CURLOPT_KEYPASSWD
{"CURLOPT_KEYPASSWD",CURLOPT_KEYPASSWD,10026,CF_STRING},
{"CURLOPT_SSLKEYPASSWD",CURLOPT_SSLKEYPASSWD,CURLOPT_KEYPASSWD,CF_STRING},
#endif
{"CURLOPT_MAXREDIRS",CURLOPT_MAXREDIRS,68,CF_LONG},
{"CURLOPT_NETRC",CURLOPT_NETRC,51,CF_LONG},
{"CURLOPT_NETRC_FILE",CURLOPT_NETRC_FILE,10118,CF_STRING},
{"CURLOPT_PROXY",CURLOPT_PROXY,10004,CF_STRING},
{"CURLOPT_PROXYAUTH",CURLOPT_PROXYAUTH,111,CF_STRING},
{"CURLOPT_PROXYPORT",CURLOPT_PROXYPORT,59,CF_LONG},
{"CURLOPT_PROXYUSERPWD",CURLOPT_PROXYUSERPWD,10006,CF_STRING},
{"CURLOPT_SSLCERT",CURLOPT_SSLCERT,10025,CF_STRING},
{"CURLOPT_SSLKEY",CURLOPT_SSLKEY,10087,CF_STRING},
{"CURLOPT_SSL_VERIFYHOST",CURLOPT_SSL_VERIFYHOST,81,CF_LONG},
{"CURLOPT_SSL_VERIFYPEER",CURLOPT_SSL_VERIFYPEER,64,CF_LONG},
#ifdef HAVE_CURLOPT_KEEPALIVE
{"CURLOPT_TCP_KEEPALIVE",CURLOPT_TCP_KEEPALIVE,213,CF_LONG},
#endif
{"CURLOPT_TIMEOUT",CURLOPT_TIMEOUT,13,CF_LONG},
{"CURLOPT_CONNECTTIMEOUT",CURLOPT_CONNECTTIMEOUT,78,CF_LONG},
{"CURLOPT_USERAGENT",CURLOPT_USERAGENT,10018,CF_STRING},
{"CURLOPT_USERPWD",CURLOPT_USERPWD,10005,CF_STRING},
{"CURLOPT_VERBOSE",CURLOPT_VERBOSE,41,CF_LONG},
{"CURLOPT_USE_SSL",CURLOPT_USE_SSL,119,CF_LONG},
{NULL,0}
};

static int touppercase(int c)
{
    if(c >= 'a' && c <= 'z')
	c = ((c-'a') + 'A');
    return c;
}

static void
initialize(void)
{
  struct OCCURLFLAG* p;
  size_t len = 0;

  if(nflags == 0) {  /* initialize */
	maxflag = -1;
    for(p=oc_curlflags;p->name;p++) {
      int c;
      nflags++; /* count number of flags */
      if(p->flag > maxflag) maxflag = p->flag;
      /* construct alphabetic radix nameindices */
      c = p->name[0];
      OCASSERT(c >= 'A' && c <= 'Z');
      if(nameindices[c] == NULL)
		nameindices[c] = p;
	}


    len = maxflag;
    if(maxflag == -1) len += 2; else len += 1;
    flagindices = (struct OCCURLFLAG**)calloc(1,len*sizeof(struct OCCURLFLAG*));
    for(p=oc_curlflags;p->name;p++)
      flagindices[p->flag] = p;
  }
}

struct OCCURLFLAG*
occurlflags(void)
{
    if(nflags == 0) initialize();
    return oc_curlflags;
}

struct OCCURLFLAG*
occurlflagbyname(const char* name)
{
    struct OCCURLFLAG* f;
    int c = name[0];
    char flagname[4096];
    const char* p;
    char* q;

    if(nflags == 0) initialize();
    /* Force upper case */
    for(p=name,q=flagname;*p;p++) {
        int cc = touppercase(*p);
        if(cc < 'A' || cc > 'Z') return NULL;
        *q++ = cc;
    }
    *q = '\0';

    if(nameindices[c] == NULL)
	return NULL; /* no possible match */
    for(f=nameindices[c];f->name;f++) {
	int cmp = strcmp(name,f->name);
	if(cmp > 0) break; /* We assume sorted */
	if(cmp == 0) return f;
    }
    return NULL;
}

struct OCCURLFLAG*
occurlflagbyflag(int flag)
{
    if(nflags == 0) initialize();
    if(flag >= 0 || flag <= maxflag)
	return flagindices[flag];
    return NULL;
}
