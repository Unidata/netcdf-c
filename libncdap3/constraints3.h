 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CONSTRAINTS3_H
#define CONSTRAINTS3_H 1

/* forward */
struct Getvara;

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

extern NCerror parsedapconstraints(NCDRNO*, char*, NCconstraint*);
extern NCerror mapconstraints3(NCDRNO*);

extern char* simplepathstring(NClist* segments, char* separator);
extern void makesegmentstring3(NClist* segments, NCbytes* buf, char* separator);

extern void restrictprojection3(NCDRNO* drno, NClist*, NClist*);
extern NCerror mergeprojections3(NCDRNO*, NClist*, NClist*);
extern int iswholeprojection(struct NCprojection*);

extern NCsegment* createncsegment(void);
extern NCslice* createncslice(void);
extern void freegetvara(struct Getvara* vara);

extern NCerror slicemerge3(NCslice* dst, NCslice* src);

extern int iswholeslice(struct NCslice*, struct CDFnode* dim);
extern int iswholesegment(struct NCsegment*);

extern int ncceparse(char*, int, NClist**, NClist**, char**);

extern char* buildprojectionstring3(NClist* projections);
extern char* buildselectionstring3(NClist* selections);
extern char* buildconstraintstring3(NCconstraint* constraints);


/* CE struct utilities */
extern NCprojection* createncprojection(void);
extern NCselection* createncselection(void);
extern NCvalue* createncvalue(void);
extern NCfcn* createncfcn(void);
extern NCvar* createncvar(void);
extern NCconstant* createncconstant(void);
extern NCconstraint* createncconstraint(void);

/* Watch out, input==struct and output==struct */
extern NCslice clonencslice(NCslice slice);
extern NClist* clonencsegments(NClist* segments);
extern NCvar* clonencvar(NCvar* var);
extern NCfcn* clonencfcn(NCfcn*);
extern NCconstant* clonencconstant(NCconstant*);
extern NCvalue* clonencvalue(NCvalue*);
extern NClist* clonencsegments(NClist*);
extern NCprojection* clonencprojection(NCprojection*);
extern NClist* clonencprojections(NClist*);
extern NCselection* clonencselection(NCselection*);
extern NClist* clonencselections(NClist*);
extern NCconstraint* clonencconstraint(NCconstraint*);

extern void freencslice(NCslice*);
extern void freencsegment(NCsegment*);
extern void freencsegments(NClist*);
extern void freencvar(NCvar*);
extern void freencfcn(NCfcn*);
extern void freencconstant(NCconstant*);
extern void freencvalue(NCvalue*);
extern void freencprojection(NCprojection*);
extern void freencprojections(NClist*);
extern void freencselection(NCselection*);
extern void freencselections(NClist*);
extern void freencconstraint(NCconstraint*);

extern void tostringncslices(NCslice* slice, int rank, NCbytes* buf);
extern void tostringncsegments(NClist* segments, NCbytes* buf);
extern void tostringncvar(NCvar* var, NCbytes* buf);
extern void tostringncfcn(NCfcn* fcn, NCbytes* buf);
extern void tostringncconstant(NCconstant* value, NCbytes* buf);
extern void tostringncvalue(NCvalue* value, NCbytes* buf);
extern void tostringncprojection(NCprojection* p, NCbytes* buf);
extern void tostringncprojections(NClist* projections, NCbytes* buf);
extern void tostringncselection(NCselection* sel, NCbytes* buf);
extern void tostringncselections(NClist* selections, NCbytes* buf);
extern void tostringncconstraint(NCconstraint* con, NCbytes* buf);

extern NClist* ceallnodes(NCany* node, NCsort which);

#endif /*CONSTRAINTS3_H*/
