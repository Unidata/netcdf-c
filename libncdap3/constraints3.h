 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CONSTRAINTS3_H
#define CONSTRAINTS3_H 1


extern NCerror parsedapconstraints(NCDAPCOMMON*, char*, NCconstraint**);
extern NCerror mapconstraints3(NCDAPCOMMON*);

extern char* simplepathstring(NClist* segments, char* separator);
extern void makesegmentstring3(NClist* segments, NCbytes* buf, char* separator);

extern NCerror mergeprojections3(NClist*, NClist*);
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
