
/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCAUXINTERNAL_H
#define NCAUXINTERNAL_H

#include "ncaux.h"

struct NCAUX_FIELD {
    char* name;
    nc_type fieldtype;
    size_t ndims;
    int dimsizes[NC_MAX_VAR_DIMS];    
    size_t size;
    size_t offset;
    size_t alignment;
};

struct NCAUX_CMPD {
    int ncid;
    int mode;
    char* name;
    size_t nfields;
    struct NCAUX_FIELD* fields;
    size_t size;
    size_t offset; /* cumulative as fields are added */
    size_t alignment;
};

typedef struct Typealignment {
    char* typename;
    size_t alignment;
} Typealignment;

/* Capture in struct and in a vector*/
typedef struct Typealignset {
    Typealignment charalign;	/* char*/
    Typealignment ucharalign;	/* unsigned char*/
    Typealignment shortalign;	/* short*/
    Typealignment ushortalign;	/* unsigned short*/
    Typealignment intalign;		/* int*/
    Typealignment uintalign;	/* unsigned int*/
    Typealignment longalign;	/* long*/
    Typealignment ulongalign;	/* unsigned long*/
    Typealignment longlongalign;	/* long long*/
    Typealignment ulonglongalign;	/* unsigned long long*/
    Typealignment floatalign;	/* float*/
    Typealignment doublealign;	/* double*/
    Typealignment ptralign;		/* void**/
    Typealignment ncvlenalign;	/* nc_vlen_t*/
} Typealignset;

#define NCTYPES        15

#endif /*NCAUXINTERNAL_H*/

