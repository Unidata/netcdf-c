/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ocinternal.h"
#include "ocdebug.h"

#define LBRACKET '['
#define RBRACKET ']'

const char*
ocparamlookup(OCstate* state, const char* key)
{
    const char* value = NULL;
    if(state == NULL || key == NULL || state->uri == NULL) return NULL;
    value = ncurilookup(state->uri,key);
    return value;
}

#if 0
int
ocparamset(OCstate* state, const char* params)
{
    int i;
    i = ncurisetparams(state->uri,params);
    return i?OC_NOERR:OC_EBADURL;
}
#endif

