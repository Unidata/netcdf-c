/*
  Copyright 2010 University Corporation for Atmospheric
  Research/Unidata. See COPYRIGHT file for more info.

  This file defines the opendap-related functions.

  "$Id: nc4.c,v 1.1 2010/06/01 15:46:50 ed Exp $" 
*/

#include "ncdispatch.h"

/* allow access dapurlparse and params while minimizing exposing dapurl.h */
int
NCDAP_urlparse(const char* s, void** dapurlp)
{
    DAPURL* dapurl = NULL;
    dapurl = calloc(1,sizeof(DAPURL));
    if(dapurl == 0) return NC_ENOMEM;
    if(!dapurlparse(s,dapurl)) {
	dapurlclear(dapurl);
	free(dapurl);
	return NC_EINVAL;
    }
    if(dapurlp) *dapurlp = dapurl;
    return NC_NOERR;
}

void
NCDAP_urlfree(void* durl)
{
    DAPURL* dapurl = (DAPURL*)durl;
    if(dapurl != NULL) {
	dapurlclear(dapurl);
	free(dapurl);
    }
}

const char*
NCDAP_urllookup(void* durl, const char* param)
{
    DAPURL* dapurl = (DAPURL*)durl;
    if(param == NULL || strlen(param) == 0 || dapurl == NULL) return NULL;
    return dapurllookup(dapurl,param);
}


/* int */
/* NCDAP_urlparse(const char* s, void** dapurlp) */
/* { */
/*     return NC_EINVAL; */
/* } */

/* void */
/* NCDAP_urlfree(void* durl) */
/* { */
/*     return; */
/* } */

/* const char* */
/* NCDAP_urllookup(void* durl, const char* param) */
/* { */
/*     return NULL; */
/* } */
