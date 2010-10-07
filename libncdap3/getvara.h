/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/getvara.h,v 1.11 2010/05/27 21:34:08 dmh Exp $
 *********************************************************************/
#ifndef GETVARA_H
#define GETVARA_H

/*
Define the overall maximum cache size
and the per-retrieval maximum size
*/

/* Use slightly misspelled names to avoid conflicts */
#define KILBYTE 0x400
#define MEGBYTE 0x100000
#define GIGBYTE 0x40000000

/* The cache limit is in terms of bytes */
#define DFALTCACHELIMIT (100*MEGBYTE)
/* The fetch limit is in terms of bytes */
#define DFALTFETCHLIMIT (100*KILBYTE)

/* WARNING: The small limit is in terms of the # of vector elements */
#define DFALTSMALLLIMIT (1*KILBYTE)

/* Max number of cache nodes */
#define DFALTCACHECOUNT (100)

/*
Store the relevant parameters for accessing
data for a particular variable
Break up the startp, countp, stridep into slices
to facilitate the odometer walk
*/

typedef struct NCslice {
    size_t first;
    size_t count;
    size_t length; /* count*stride */
    size_t stride;
    size_t stop; /* == first + count*/
    size_t declsize;  /* from defining dimension, if any.*/
} NCslice;

/* Define a tracker for memory to support*/
/* the concatenation*/

struct NCMEMORY {
    void* memory;
    char* next; /* where to store the next chunk of data*/
}; 

typedef struct NCsegment {
    char* name;
    struct CDFnode* node;
    int slicesdefined; /* do we know yet if this has defined slices */
    unsigned int slicerank; /* Note: this is the rank as shown in the
                               projection; may be less than node->array.rank */
    NCslice slices[NC_MAX_VAR_DIMS];        
} NCsegment;


/* Hold a projection segment */

typedef enum SelectionTag {
ST_NIL=0,
ST_EQ=1,ST_NEQ=2,ST_GE=3,ST_GT=4,ST_LT=5,ST_LE=6,ST_RE=7,
ST_STR=8,ST_INT=9,ST_FLOAT=10,ST_VAR=11,ST_FCN,ST_CONST
} SelectionTag;



typedef struct NCfcn {
    SelectionTag kind;
    char* name;
    NClist* args;
} NCfcn;

typedef struct NCvar {
    SelectionTag kind;
    NClist* segments;
    struct CDFnode* node;
} NCvar;

typedef struct NCprojection {
    SelectionTag kind;
    NCvar* var;
    NCfcn* fcn;
    /* Following duplicate info inferrable from the segments */
    struct CDFnode* leaf;
} NCprojection;

typedef struct NCconst {
    SelectionTag kind;
    char* text;
    long long intvalue;
    double floatvalue;
} NCconst;

typedef struct NCvalue {
    SelectionTag kind;
    NCconst* constant;
    NCvar* var;
    NCfcn* fcn;
} NCvalue;

typedef struct NCselection {
    SelectionTag operator;
    NClist* segments;
    NClist* values;
    struct CDFnode* leaf;
    char* fcn;
} NCselection;


typedef int nc_tactic;
#define tactic_null	0
#define tactic_all	1
#define tactic_partial	2
#define tactic_grid	4
#define tactic_var	8

typedef struct Getvara {
    int projected; /* Were projections applied when retrieving data */
    void* memory; /* where result is put*/
    NCcachenode* cache;
    NCprojection* varaprojection;
    /* associated variable*/
    OCtype dsttype;
    CDFnode* target;
    CDFnode* target0;
} Getvara;

#endif /*GETVARA_H*/
