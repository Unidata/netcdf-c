/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include "d4curlfunctions.h"

#define MAX_REDIRECTS 20L

/* Mnemonic */
#define OPTARG void*

/* Condition on libcurl version */
/* Set up an alias as needed */
#ifndef HAVE_CURLOPT_KEYPASSWD
#define CURLOPT_KEYPASSWD CURLOPT_SSLKEYPASSWD
#endif

#define NETRCFILETAG "HTTP.NETRC"

#define CHECK(state,flag,value) {if(check(state,flag,(void*)value) != NC_NOERR) {goto done;}}

/* forward */
static int set_curlflag(NCD4INFO* state, int flag);
static int set_curlopt(NCD4INFO* state, int flag, void* value);
static int set_curl_options(NCD4INFO* state);
static void* cvt(char* value, enum CURLFLAGTYPE type);

static int
check(NCD4INFO* state, int flag, void* value)
{
    int ret = set_curlopt(state,flag,value);
    return THROW(ret);
}

/*
Set a specific curl flag; primary wrapper for curl_easy_setopt
*/
static int
set_curlopt(NCD4INFO* state, int flag, void* value)
{
    int ret = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    cstat = curl_easy_setopt(state->curl->curl,flag,value);
    if(cstat != CURLE_OK)
	ret = NC_ECURL;
    return THROW(ret);
}

/*
Update a specific flag from state
*/
static int
set_curlflag(NCD4INFO* state, int flag)
{
    int ret = NC_NOERR;
    switch (flag) {
    case CURLOPT_USERPWD:
        if(state->curl->creds.userpwd != NULL) {
	    CHECK(state, CURLOPT_USERPWD, state->curl->creds.userpwd);
            CHECK(state, CURLOPT_HTTPAUTH, (OPTARG)CURLAUTH_ANY);
	}
	break;
    case CURLOPT_COOKIEJAR: case CURLOPT_COOKIEFILE:
        if(state->curl->curlflags.cookiejar) {
	    /* Assume we will read and write cookies to same place */
	    CHECK(state, CURLOPT_COOKIEJAR, state->curl->curlflags.cookiejar);
	    CHECK(state, CURLOPT_COOKIEFILE, state->curl->curlflags.cookiejar);
        }
	break;
    case CURLOPT_NETRC: case CURLOPT_NETRC_FILE:
	if(state->curl->curlflags.netrc) {
	    CHECK(state, CURLOPT_NETRC, (OPTARG)CURL_NETRC_REQUIRED);
	    CHECK(state, CURLOPT_NETRC_FILE, state->curl->curlflags.netrc);
        }
	break;
    case CURLOPT_VERBOSE:
	if(state->curl->curlflags.verbose)
	    CHECK(state, CURLOPT_VERBOSE, (OPTARG)1L);
	break;
    case CURLOPT_TIMEOUT:
	if(state->curl->curlflags.timeout)
	    CHECK(state, CURLOPT_TIMEOUT, (OPTARG)((long)state->curl->curlflags.timeout));
	break;
    case CURLOPT_USERAGENT:
        if(state->curl->curlflags.useragent)
	    CHECK(state, CURLOPT_USERAGENT, state->curl->curlflags.useragent);
	break;
    case CURLOPT_FOLLOWLOCATION:
        CHECK(state, CURLOPT_FOLLOWLOCATION, (OPTARG)1L);
	break;
    case CURLOPT_MAXREDIRS:
	CHECK(state, CURLOPT_MAXREDIRS, (OPTARG)MAX_REDIRECTS);
	break;
    case CURLOPT_ERRORBUFFER:
	CHECK(state, CURLOPT_ERRORBUFFER, state->curl->errdata.errorbuf);
	break;
    case CURLOPT_ENCODING:
#ifdef CURLOPT_ENCODING
	if(state->curl->curlflags.compress) {
	    CHECK(state, CURLOPT_ENCODING,"deflate, gzip");
        }
#endif
	break;
    case CURLOPT_PROXY:
	if(state->curl->proxy.host != NULL) {
	    CHECK(state, CURLOPT_PROXY, state->curl->proxy.host);
	    CHECK(state, CURLOPT_PROXYPORT, (OPTARG)(long)state->curl->proxy.port);
	    if(state->curl->proxy.userpwd) {
                CHECK(state, CURLOPT_PROXYUSERPWD, state->curl->proxy.userpwd);
#ifdef CURLOPT_PROXYAUTH
	        CHECK(state, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
#endif
	    }
	}
	break;
    case CURLOPT_USE_SSL:
    case CURLOPT_SSLCERT: case CURLOPT_SSLKEY:
    case CURLOPT_SSL_VERIFYPEER: case CURLOPT_SSL_VERIFYHOST:
    {
        struct ssl* ssl = &state->curl->ssl;
        CHECK(state, CURLOPT_SSL_VERIFYPEER, (OPTARG)(ssl->verifypeer?1L:0L));
        CHECK(state, CURLOPT_SSL_VERIFYHOST, (OPTARG)(ssl->verifyhost?1L:0L));
        if(ssl->certificate)
            CHECK(state, CURLOPT_SSLCERT, ssl->certificate);
        if(ssl->key)
            CHECK(state, CURLOPT_SSLKEY, ssl->key);
        if(ssl->keypasswd)
            /* libcurl prior to 7.16.4 used 'CURLOPT_SSLKEYPASSWD' */
            CHECK(state, CURLOPT_KEYPASSWD, ssl->keypasswd);
        if(ssl->cainfo)
            CHECK(state, CURLOPT_CAINFO, ssl->cainfo);
        if(ssl->capath)
            CHECK(state, CURLOPT_CAPATH, ssl->capath);
    }
    break;

    default:
        nclog(NCLOGWARN,"Attempt to update unexpected curl flag: %d",flag);
	break;
    }
done:
    return THROW(ret);
}

/* Set various general curl flags per fetch  */
int
NCD4_set_flags_perfetch(NCD4INFO* state)
{
    int ret = NC_NOERR;
    /* currently none */
    return THROW(ret);
}

/* Set various general curl flags per link */

int
NCD4_set_flags_perlink(NCD4INFO* state)
{
    int ret = NC_NOERR;
    /* Following are always set */
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_ENCODING);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_NETRC);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_VERBOSE);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_TIMEOUT);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_USERAGENT);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_COOKIEJAR);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_USERPWD);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_PROXY);
    if(ret == NC_NOERR) ret = set_curlflag(state,CURLOPT_USE_SSL);
    if(ret == NC_NOERR) ret = set_curlflag(state, CURLOPT_FOLLOWLOCATION);
    if(ret == NC_NOERR) ret = set_curlflag(state, CURLOPT_MAXREDIRS);
    if(ret == NC_NOERR) ret = set_curlflag(state, CURLOPT_ERRORBUFFER);

    /* Set the CURL. options */
    if(ret == NC_NOERR) ret = set_curl_options(state);
    return THROW(ret);
}

/**
Directly set any options starting with 'CURL.'
*/
static int
set_curl_options(NCD4INFO* state)
{
    int ret = NC_NOERR;
    NClist* store = NULL;
    int i;
    char hostport[NC_MAX_PATH];

    NCD4_hostport(state->uri,hostport,sizeof(hostport));

    store = NCD4_globalstate->rc.rc;

    for(i=0;i<nclistlength(store);i++) {
        struct CURLFLAG* flag;
	NCD4triple* triple = (NCD4triple*)nclistget(store,i);
        size_t hostlen = strlen(triple->host);
        const char* flagname;
        if(strncmp("CURL.",triple->key,5) != 0) continue; /* not a curl flag */
        /* do hostport prefix comparison */
        if(hostport[0] != '\0') {
          int t = strncmp(hostport,triple->host,hostlen);
          if(t !=  0) continue;
        }
        flagname = triple->key+5; /* 5 == strlen("CURL."); */
        flag = NCD4_curlflagbyname(flagname);
        if(flag == NULL) {ret = NC_ECURL; goto done;}
        ret = set_curlopt(state,flag->flag,cvt(triple->value,flag->type));
    }
 done:
    return THROW(ret);
}

static void*
cvt(char* value, enum CURLFLAGTYPE type)
{
    switch (type) {
    case CF_LONG: {
	/* Try to convert to long value */
	const char* p = value;
	char* q = NULL;
	long longvalue = strtol(p,&q,10);
	if(*q != '\0')
	    return NULL;
	return (void*)longvalue;
	}
    case CF_STRING:
	return (void*)value;
    case CF_UNKNOWN: case CF_OTHER:
	return (void*)value;
    }
    return NULL;
}

void
NCD4_curl_debug(NCD4INFO* state)
{
    state->curl->curlflags.verbose = 1;
    set_curlflag(state,CURLOPT_VERBOSE);
    set_curlflag(state,CURLOPT_ERRORBUFFER);
}

/* Misc. */

/* Determine if this version of curl supports
       "file://..." &/or "https://..." urls.
*/
void
NCD4_curl_protocols(NCD4globalstate* state)
{
    const char* const* proto; /*weird*/
    curl_version_info_data* curldata;
    curldata = curl_version_info(CURLVERSION_NOW);
    for(proto=curldata->protocols;*proto;proto++) {
        if(strcmp("file",*proto)==0) {state->curl.proto_file=1;}
        if(strcmp("http",*proto)==0) {state->curl.proto_https=1;}
    }
#ifdef D4DEBUG	
    nclog(NCLOGNOTE,"Curl file:// support = %d",state->curl.proto_file);
    nclog(NCLOGNOTE,"Curl https:// support = %d",state->curl.proto_https);
#endif
}


/*
"Inverse" of set_curlflag;
Given a flag and value, it updates state.
Update a specific flag from state->curlflags.
*/
int
NCD4_set_curlstate(NCD4INFO* state, int flag, void* value)
{
    int ret = NC_NOERR;
    switch (flag) {
    case CURLOPT_USERPWD:
        if(state->curl->creds.userpwd != NULL) free(state->curl->creds.userpwd);
	state->curl->creds.userpwd = strdup((char*)value);
	break;
    case CURLOPT_COOKIEJAR: case CURLOPT_COOKIEFILE:
        if(state->curl->curlflags.cookiejar != NULL) free(state->curl->curlflags.cookiejar);
	state->curl->curlflags.cookiejar = strdup((char*)value);
	break;
    case CURLOPT_NETRC: case CURLOPT_NETRC_FILE:
        if(state->curl->curlflags.netrc != NULL) free(state->curl->curlflags.netrc);
	state->curl->curlflags.netrc = strdup((char*)value);
	break;
    case CURLOPT_VERBOSE:
	state->curl->curlflags.verbose = (long)value;
	break;
    case CURLOPT_TIMEOUT:
	state->curl->curlflags.timeout = (long)value;
	break;
    case CURLOPT_USERAGENT:
        if(state->curl->curlflags.useragent != NULL) free(state->curl->curlflags.useragent);
        state->curl->curlflags.useragent = strdup((char*)value);
	break;
    case CURLOPT_FOLLOWLOCATION:
	/* no need to store; will always be set */
	break;
    case CURLOPT_MAXREDIRS:
	/* no need to store; will always be set */
	break;
    case CURLOPT_ERRORBUFFER:
	/* no need to store; will always be set */
	break;
    case CURLOPT_ENCODING:
	/* no need to store; will always be set to fixed value */
	break;
    case CURLOPT_PROXY:
	/* We assume that the value is the proxy url */
	if(state->curl->proxy.host != NULL) free(state->curl->proxy.host);
	if(state->curl->proxy.userpwd != NULL) free(state->curl->proxy.userpwd);
	state->curl->proxy.host = NULL;
	state->curl->proxy.userpwd = NULL;
	if(!NCD4_parseproxy(state,(char*)value))
		{ret = NC_EINVAL; goto done;}
	break;
    case CURLOPT_SSLCERT:
	if(state->curl->ssl.certificate != NULL) free(state->curl->ssl.certificate);
	state->curl->ssl.certificate = strdup((char*)value);
	break;
    case CURLOPT_SSLKEY:
	if(state->curl->ssl.key != NULL) free(state->curl->ssl.key);
	state->curl->ssl.key = strdup((char*)value);
	break;
    case CURLOPT_KEYPASSWD:
	if(state->curl->ssl.keypasswd!= NULL) free(state->curl->ssl.keypasswd);
	state->curl->ssl.keypasswd = strdup((char*)value);
	break;
    case CURLOPT_SSL_VERIFYPEER:
      state->curl->ssl.verifypeer = (long)value;
      break;
    case CURLOPT_SSL_VERIFYHOST:
      state->curl->ssl.verifyhost = (long)value;
      break;
    case CURLOPT_CAINFO:
      if(state->curl->ssl.cainfo != NULL) free(state->curl->ssl.cainfo);
      state->curl->ssl.cainfo = strdup((char*)value);
      break;
    case CURLOPT_CAPATH:
	if(state->curl->ssl.capath != NULL) free(state->curl->ssl.capath);
	state->curl->ssl.capath = strdup((char*)value);
	break;

    default: break;
    }
done:
    return THROW(ret);
}

void
NCD4_curl_printerror(NCD4INFO* state)
{
    fprintf(stderr,"curl error details: %s\n",state->curl->errdata.errorbuf);
}

CURLcode
NCD4_reportcurlerror(CURLcode cstat)
{
    if(cstat != CURLE_OK) {
        fprintf(stderr,"CURL Error: %s\n",curl_easy_strerror(cstat));
    }
    fflush(stderr);
    return cstat;
}
