/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncconstraints/ncconstraints.h,v 1.40 2010/05/30 19:45:52 dmh Exp $
  *********************************************************************/

#ifndef NCCONSTRAINTS_H
#define NCCONSTRAINTS_H 1

#ifndef NC_MAX_VAR_DIMS
#define NC_MAX_VAR_DIMS 1024
#endif

typedef enum NCCops {
NO_NIL=0,NO_EQ=1,NO_NEQ=2,NO_GE=3,NO_GT=4,NO_LT=5,NO_LE=6,NO_RE=7
} NCCops;

/* Must match NCCops */
#define OPSTRINGS {"?","=","!=",">=",">","<=","<","=~"}

typedef enum NCCsort {
NS_NIL=0,
NS_STR=8,NS_INT=9,NS_FLOAT=10,
NS_VAR=11,NS_FCN=12,NS_CONST=13,
NS_SELECT=14, NS_PROJECT=15,
NS_SEGMENT=16, NS_SLICE=17,
NS_CONSTRAINT=18,
NS_VALUE=19
} NCCsort;

/* Provide a universal cast type containing (mostly) common fields */
typedef struct NCCnode {
    NCCsort sort;    
    void* notes; /* user defined annotations */
    char* name; 
    char* fullname;
} NCCnode;

/*
Store the relevant parameters for accessing
data for a particular variable
Break up the startp, countp, stridep into slices
to facilitate the odometer walk
*/

typedef enum NCCform {
NCF_DAP=0,
NCF_CDMREMOTE=1
} NCCform;

typedef struct NCCslice {
    NCCnode node;
    NCCform form;
    size_t first;
    size_t count;
    size_t length; /* count*stride */
    size_t stride;
    size_t stop; /* == first + count*/
    size_t declsize;  /* from defining dimension, if any.*/
} NCCslice;

typedef struct NCCsegment {
    NCCnode node;
    int slicesdefined; /* do we know yet if this has defined slices */
    unsigned int slicerank; /* Note: this is the rank as shown in the
                               projection; may be less than node->array.rank */
    NCCslice slices[NC_MAX_VAR_DIMS];        
} NCCsegment;

typedef struct NCCfcn {
    NCCnode node;
    NClist* args;
} NCCfcn;

typedef struct NCCvar {
    NCCnode node;
    NClist* segments;
} NCCvar;

typedef struct NCCconstant {
    NCCnode node;
    NCCsort discrim;
    char* text;
    long long intvalue;
    double floatvalue;
} NCCconstant;

typedef struct NCCvalue {
    NCCnode node;
    NCCsort discrim;
    /* Do not bother with a union */
    NCCconstant* constant;
    NCCvar* var;
    NCCfcn* fcn;
} NCCvalue;

typedef struct NCCselection {
    NCCnode node;
    NCCsort operator;
    NCCvalue* lhs;
    NClist* rhs;
} NCCselection;

typedef struct NCCprojection {
    NCCnode node;
    NCCsort discrim;
    /* Do not bother with a union */
    NCCvar* var;
    NCCfcn* fcn;
} NCCprojection;

typedef struct NCCconstraint {
    NCCnode node;
    NClist* projections;
    NClist* selections;
} NCCconstraint;


extern int ncparseconstraints(char* constraints, NCCconstraint* ncconstraint);
extern int nccslicemerge(NCCslice* dst, NCCslice* src);
extern int nccmergeprojections(NClist* dst, NClist* src);

extern char* buildprojectionstring(NClist* projections);
extern char* buildselectionstring(NClist* selections);
extern char* buildconstraintstring(NCCconstraint* constraints);

extern NCCnode* nccclone(NCCnode* node);
extern NClist* ncc_clonelist(NClist* list);

extern void nccfree(NCCnode* node);
extern void ncc_freelist(NClist* list);

extern char* ncctostring(NCCnode* node);
extern char* ncc_listtostring(NClist* list);
extern void ncctobuffer(NCCnode* node, NCbytes* buf);
extern void ncc_listtobuffer(NClist* list, NCbytes* buf);

extern NClist* ceallnodes(NCCnode* node, NCCsort which);

extern NCCnode* ncccreate(NCCsort sort);


#endif /*NCCONSTRAINTS_H*/
