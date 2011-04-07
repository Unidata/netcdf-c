/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncconstraints/ncconstraints.h,v 1.40 2010/05/30 19:45:52 dmh Exp $
  *********************************************************************/
#ifndef NCCONSTRAINTS_H
#define NCCONSTRAINTS_H 1

typedef enum NCsort {
NS_NIL=0,
NS_EQ=1,NS_NEQ=2,NS_GE=3,NS_GT=4,NS_LT=5,NS_LE=6,NS_RE=7,
NS_STR=8,NS_INT=9,NS_FLOAT=10,
NS_VAR=11,NS_FCN=12,NS_CONST=13,
NS_SELECT=14, NS_PROJECT=15,
NS_SEGMENT=16, NS_SLICE=17,
NS_CONSTRAINT=18,
NS_VALUE=19
} NCsort;

/* Must match NCsort */
#define OPSTRINGS \
{"?","=","!=",">=",">","<=","<","=~","?","?","?","?","?","?","?","?","?","?","?"}


/* Provide a universal cast type */
typedef struct NCany {
    NCsort sort;    
} NCany;

/*
Store the relevant parameters for accessing
data for a particular variable
Break up the startp, countp, stridep into slices
to facilitate the odometer walk
*/

typedef struct NCslice {
    NCsort sort;    
    size_t first;
    size_t count;
    size_t length; /* count*stride */
    size_t stride;
    size_t stop; /* == first + count*/
    size_t declsize;  /* from defining dimension, if any.*/
} NCslice;


typedef struct NCsegment {
    NCsort sort;
    char* name;
    struct CDFnode* node;
    int slicesdefined; /* do we know yet if this has defined slices */
    unsigned int slicerank; /* Note: this is the rank as shown in the
                               projection; may be less than node->array.rank */
    NCslice slices[NC_MAX_VAR_DIMS];        
} NCsegment;

typedef struct NCfcn {
    NCsort sort;
    char* name;
    NClist* args;
} NCfcn;

typedef struct NCvar {
    NCsort sort;
    NClist* segments;
    struct CDFnode* node;
    /* Following duplicate info inferrable from the segments */
    struct CDFnode* leaf;
} NCvar;

typedef struct NCconstant {
    NCsort sort;
    NCsort discrim;
    char* text;
    long long intvalue;
    double floatvalue;
} NCconstant;

typedef struct NCvalue {
    NCsort sort;
    NCsort discrim;
    NCconstant* constant;
    NCvar* var;
    NCfcn* fcn;
} NCvalue;

typedef struct NCselection {
    NCsort sort;
    NCsort operator;
    NCvalue* lhs;
    NClist* rhs;
} NCselection;

typedef struct NCprojection {
    NCsort sort;
    NCsort discrim;
    NCvar* var;
    NCfcn* fcn;
} NCprojection;

typedef struct NCconstraint {
    NCsort sort;
    NClist* projections;
    NClist* selections;
} NCconstraint;

#endif /*NCCONSTRAINTS_H*/
