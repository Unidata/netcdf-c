 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CONSTRAINTS3_H
#define CONSTRAINTS3_H 1

extern NCerror buildconstraints3(struct NCDAPCOMMON* drno);
extern int iscached(struct NCDAPCOMMON*, struct CDFnode*, struct NCcachenode**);

extern char* makeprojectionstring3(NClist* projections);
extern char* makeselectionstring3(NClist* selections);
extern char* makeconstraintstring3(NClist* projections, char* selectionstring);
extern char* simplepathstring(NClist* segments, char* separator);
extern void makesegmentstring3(NClist* segments, NCbytes* buf, char* separator);

extern NCsegment* createncsegment(void);
extern NCprojection* createncprojection(void);
extern NCselection* createncselection(void);
extern NCslice* createncslice(void);
extern NCvalue* createncvalue(void);

extern NClist* cloneprojections(NClist* projectionlist);
extern NCprojection* cloneprojection1(NCprojection* p);
extern NClist* cloneselections(NClist* selectionlist);
extern NCselection* cloneselection1(NCselection* s);
extern NCconstraint cloneconstraint(NCconstraint* con);
extern void freencprojections(NClist*);
extern void freencprojection1(NCprojection* p);
extern void freencsegment(NCsegment* seg);
extern void freencselections(NClist*);
extern void freencselection1(NCselection* s);
extern void clearncconstraint(NCconstraint* con);
extern void freegetvara(struct Getvara* vara);

extern NCerror slicemerge3(NCslice* dst, NCslice* src);

extern NCerror prefetchdata3(struct NCDAPCOMMON*);
extern NCerror buildcachenode34(struct NCDAPCOMMON*, NCconstraint*, NClist*, NCcachenode**, int);
extern NCcachenode* createnccachenode(void);
extern void clearnccache(struct NCDAPCOMMON*,NCcache*);
extern void freenccachenode(struct NCDAPCOMMON*, NCcachenode*);

extern NCerror mergeprojections3(struct NCDAPCOMMON*, NClist*, NClist*);

extern int iswholeslice(struct NCslice*, struct CDFnode* dim);
extern int iswholesegment(struct NCsegment*);
extern int iswholeprojection(struct NCprojection*);

extern int ncceparse(char*, int, NClist**, NClist**, char**);

#endif /*CONSTRAINTS3_H*/


