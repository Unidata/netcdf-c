/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef NCFILTER_H
#define NCFILTER_H 1

/* API for libdispatch/dfilter.c */

/* Must match values in <H5Zpublic.h> */
#ifndef H5Z_FILTER_SZIP
#define H5Z_FILTER_SZIP 4
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* Provide consistent filter spec parser */
EXTERNL int NC_parsefilterspec(const char* spec, unsigned int* idp, size_t* nparamsp, unsigned int** paramsp);

EXTERNL void NC_byteswap8(unsigned char* mem);

#if defined(__cplusplus)
}
#endif

#endif /* NCFILTER_H */
