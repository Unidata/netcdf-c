 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CONSTRAINTS3_H
#define CONSTRAINTS3_H 1

/* Define the constraint annotation structure */
typedef struct NCDnotes {
    CDFnode* node; /*sort==NS_VAR|NS_SEGMENT*/
    CDFnode* leaf; /*sort==NS_VAR*/
} NCDnotes;

extern NCerror parsedapconstraints(NCDAPCOMMON*, char*, NCCconstraint*);
extern NCerror mapconstraints3(NCDAPCOMMON*);

extern char* simplepathstring(NClist* segments, char* separator);
extern void makesegmentstring3(NClist* segments, NCbytes* buf, char* separator);

extern NCerror mergeprojections3(NClist*, NClist*);
extern int iswholeprojection(struct NCCprojection*);

extern void freegetvara(struct Getvara* vara);

extern NCerror slicemerge3(NCCslice* dst, NCCslice* src);

extern int iswholeslice(struct NCCslice*, struct CDFnode* dim);
extern int iswholesegment(struct NCCsegment*);

extern int iswholeconstraint(NCCconstraint* con);

extern char* buildprojectionstring3(NClist* projections);
extern char* buildselectionstring3(NClist* selections);
extern char* buildconstraintstring3(NCCconstraint* constraints);

extern void makewholesegment3(struct NCCsegment*,struct CDFnode*);
extern void makewholeslice3(struct NCCslice* slice, struct CDFnode* dim);

#define GETNOTES(n) ((NCDnotes*)((NCCnode*)(n))->notes)

#endif /*CONSTRAINTS3_H*/
