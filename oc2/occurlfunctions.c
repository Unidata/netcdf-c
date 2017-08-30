/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ncrc.h"
#include "ocinternal.h"
#include "ocdebug.h"
#include "occurlfunctions.h"

#define OC_MAX_REDIRECTS 20L

/* Mnemonic */
#define OPTARG void*

/* Condition on libcurl version */
/* Set up an alias as needed */
#ifndef HAVE_CURLOPT_KEYPASSWD
#define CURLOPT_KEYPASSWD CURLOPT_SSLKEYPASSWD
#endif

#define NETRCFILETAG "HTTP.NETRC"

#define CHECK(state,flag,value) {if(check(state,flag,(void*)value) != OC_NOERR) {goto done;}}

static OCerror
check(OCstate* state, int flag, void* value)
{
    OCerror stat = ocset_curlopt(state,flag,value);
#ifdef OCDEBUG
    long l = (long)value;
    const char* name = occurlflagbyflag(flag)->name;
    if(l <= 1000) {
	OCDBG2("%s=%ld",name,l);
    } else {
	char show[65];
	char* s = (char*)value;
	strncpy(show,s,64);
	show[64] = '\0';
	OCDBG2("%s=%s",name,show);
    }
#endif
    return stat;
}

/*
Set a specific curl flag; primary wrapper for curl_easy_setopt
*/
OCerror
ocset_curlopt(OCstate* state, int flag, void* value)
{
    OCerror stat = OC_NOERR;
    CURLcode cstat = CURLE_OK;
    cstat = OCCURLERR(state,curl_easy_setopt(state->curl,flag,value));
    if(cstat != CURLE_OK)
	stat = OC_ECURL;
    return stat;
}

/*
Update a specific flag from state
*/
OCerror
ocset_curlflag(OCstate* state, int flag)
{
    OCerror stat = OC_NOERR;

    switch (flag) {

    case CURLOPT_USERPWD: /* Does both user and pwd */
        if(state->creds.user != NULL && state->creds.pwd != NULL) {
	    CHECK(state, CURLOPT_USERNAME, state->creds.user);
	    CHECK(state, CURLOPT_PASSWORD, state->creds.pwd);
            CHECK(state, CURLOPT_HTTPAUTH, (OPTARG)CURLAUTH_ANY);
	}
	break;

    case CURLOPT_COOKIEJAR: case CURLOPT_COOKIEFILE:
        if(state->curlflags.cookiejar) {
	    /* Assume we will read and write cookies to same place */
	    CHECK(state, CURLOPT_COOKIEJAR, state->curlflags.cookiejar);
	    CHECK(state, CURLOPT_COOKIEFILE, state->curlflags.cookiejar);
        }
	break;

    case CURLOPT_NETRC: case CURLOPT_NETRC_FILE:
	if(state->curlflags.netrc) {
	    CHECK(state, CURLOPT_NETRC, (OPTARG)CURL_NETRC_REQUIRED);
	    CHECK(state, CURLOPT_NETRC_FILE, state->curlflags.netrc);
        }
	break;

    case CURLOPT_VERBOSE:
	if(state->curlflags.verbose)
	    CHECK(state, CURLOPT_VERBOSE, (OPTARG)1L);
	break;

    case CURLOPT_TIMEOUT:
	if(state->curlflags.timeout)
	    CHECK(state, CURLOPT_TIMEOUT, (OPTARG)((long)state->curlflags.timeout));
	break;

    case CURLOPT_USERAGENT:
        if(state->curlflags.useragent)
	    CHECK(state, CURLOPT_USERAGENT, state->curlflags.useragent);
	break;

    case CURLOPT_FOLLOWLOCATION:
        CHECK(state, CURLOPT_FOLLOWLOCATION, (OPTARG)1L);
	break;

    case CURLOPT_MAXREDIRS:
	CHECK(state, CURLOPT_MAXREDIRS, (OPTARG)OC_MAX_REDIRECTS);
	break;

    case CURLOPT_ERRORBUFFER:
	CHECK(state, CURLOPT_ERRORBUFFER, state->error.curlerrorbuf);
	break;

    case CURLOPT_ENCODING:
#ifdef CURLOPT_ENCODING
	if(state->curlflags.compress) {
	    CHECK(state, CURLOPT_ENCODING,"deflate, gzip");
        }
#endif
	break;

    case CURLOPT_PROXY:
	if(state->proxy.host != NULL) {
	    CHECK(state, CURLOPT_PROXY, state->proxy.host);
	    CHECK(state, CURLOPT_PROXYPORT, (OPTARG)(long)state->proxy.port);
	    if(state->proxy.user != NULL && state->proxy.pwd != NULL) {
                CHECK(state, CURLOPT_PROXYUSERNAME, state->proxy.user);
                CHECK(state, CURLOPT_PROXYPASSWORD, state->proxy.pwd);
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
        struct OCSSL* ssl = &state->ssl;
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

    default: {
	struct OCCURLFLAG* f = occurlflagbyflag(flag);
	if(f != NULL)
	    nclog(NCLOGWARN,"Attempt to update unexpected curl flag: %s",
				f->name);
	} break;
    }
done:
    return stat;
}


/* Set various general curl flags per fetch  */
OCerror
ocset_flags_perfetch(OCstate* state)
{
    OCerror stat = OC_NOERR;
    /* currently none */
    return stat;
}

/* Set various general curl flags per link */

OCerror
ocset_flags_perlink(OCstate* state)
{
    OCerror stat = OC_NOERR;

    /* Following are always set */
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_ENCODING);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_NETRC);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_VERBOSE);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_TIMEOUT);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_USERAGENT);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_COOKIEJAR);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_USERPWD);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_PROXY);
    if(stat == OC_NOERR) stat = ocset_curlflag(state,CURLOPT_USE_SSL);
    if(stat == OC_NOERR) stat = ocset_curlflag(state, CURLOPT_FOLLOWLOCATION);
    if(stat == OC_NOERR) stat = ocset_curlflag(state, CURLOPT_MAXREDIRS);
    if(stat == OC_NOERR) stat = ocset_curlflag(state, CURLOPT_ERRORBUFFER);
    return stat;
}

void
oc_curl_debug(OCstate* state)
{
    state->curlflags.verbose = 1;
    ocset_curlflag(state,CURLOPT_VERBOSE);
    ocset_curlflag(state,CURLOPT_ERRORBUFFER);
}

/* Misc. */

int
ocrc_netrc_required(OCstate* state)
{
    char* netrcfile = ocrc_lookup(NETRCFILETAG,state->uri->uri);
    return (netrcfile != NULL || state->curlflags.netrc != NULL ? 0 : 1);
}

void
oc_curl_printerror(OCstate* state)
{
    fprintf(stderr,"curl error details: %s\n",state->curlerror);
}

/* Determine if this version of curl supports
       "file://..." &/or "https://..." urls.
*/
void
oc_curl_protocols(NCRCglobalstate* state)
{
    const char* const* proto; /*weird*/
    curl_version_info_data* curldata;
    curldata = curl_version_info(CURLVERSION_NOW);
    for(proto=curldata->protocols;*proto;proto++) {
        if(strcmp("file",*proto)==0) {state->curl.proto_file=1;}
        if(strcmp("http",*proto)==0) {state->curl.proto_https=1;}
    }
    if(ocdebug > 0) {
        nclog(NCLOGNOTE,"Curl file:// support = %d",state->curl.proto_file);
        nclog(NCLOGNOTE,"Curl https:// support = %d",state->curl.proto_https);
    }
}
