 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CONSTRAINTS3_H
#define CONSTRAINTS3_H 1

extern int iscached(struct NCDRNO*, struct CDFnode*, struct NCcachenode**);

extern NCerror buildvaraprojection3(NCDRNO*, Getvara*,
		     const size_t*, const size_t*, const ptrdiff_t*,
		     NCprojection**);

extern NCerror buildconstraints3(NCDRNO* drno);

extern char* makeprojectionstring3(NClist* projections);
extern char* makeselectionstring3(NClist* selections);
extern char* makeconstraintstring3(NClist* projections, char* selectionstring);
extern char* simplepathstring(NClist* segments, char* separator);
extern void makesegmentstring3(NClist* segments, NCbytes* buf, char* separator);

extern NCprojection* createncprojection(void);
extern NCselection* createncselection(void);
extern NCvalue* createncvalue(void);
extern NCfcn* createncfcn(void);
extern NCvar* createncvar(void);
extern NCconst* createncconst(void);

extern NClist* cloneprojections(NClist* projectionlist);
extern NCprojection* cloneprojection1(NCprojection* p);
extern NClist* cloneselections(NClist* selectionlist);
extern NCselection* cloneselection1(NCselection* s);
extern NCconstraint cloneconstraint(NCconstraint* con);
extern void freencprojections(NClist*);
extern void freencprojection1(NCprojection* p);

extern void freencselections(NClist*);
extern void freencselection1(NCselection* s);
extern void clearncconstraint(NCconstraint* con);
extern void restrictprojection3(NCDRNO* drno, NClist*, NClist*);

extern NCerror mergeprojections3(NCDRNO*, NClist*, NClist*);
extern int iswholeprojection(struct NCprojection*);

extern NCsegment* createncsegment(void);
extern NCslice* createncslice(void);
extern void freencsegment(NCsegment* seg);
extern void freegetvara(struct Getvara* vara);

extern NCerror slicemerge3(NCslice* dst, NCslice* src);

extern NCerror prefetchdata3(NCDRNO*);
extern NCerror buildcachenode3(NCDRNO*, NCconstraint*, NClist*, NCcachenode**, int);
extern NCcachenode* createnccachenode(void);
extern void clearnccache(NCDRNO*,NCcache*);
extern void freenccachenode(NCDRNO*, NCcachenode*);


extern int iswholeslice(struct NCslice*, struct CDFnode* dim);
extern int iswholesegment(struct NCsegment*);

extern int ncceparse(char*, int, NClist**, NClist**, char**);

#endif /*CONSTRAINTS3_H*/
