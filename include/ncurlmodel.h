/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/**
 * Header file for dmode.c
 * @author Dennis Heimbigner
 */

#ifndef NCURLMODEL_H
#define NCURLMODEL_H

/* Track the information from a URL that will help us
   infer how to access the data pointed to by that URL.
*/
typedef struct NCmode {
    int format; /* NC_FORMAT_XXX value */
    int implementation; /* NC_FORMATX_XXX value */
} NCmode;

/*
Return an NC_FORMATX_... value.
Assumes that the path is known to be a url.
*/
EXTERNL int NC_urlmodel(const char* path, int mode, char** newurl, NCmode* model);

/* allow access url parse and params without exposing nc_url.h */
EXTERNL int NCDAP_urlparse(const char* s, void** dapurl);
EXTERNL void NCDAP_urlfree(void* dapurl);
EXTERNL const char* NCDAP_urllookup(void* dapurl, const char* param);

/* Ping a specific server */
EXTERNL int NCDAP2_ping(const char*);
EXTERNL int NCDAP4_ping(const char*);

#endif /*NCURLMODEL_H*/
