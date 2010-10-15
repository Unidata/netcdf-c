 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CACHE3_H
#define CACHE3_H 1

extern int iscached(struct NCDRNO*, struct CDFnode*, struct NCcachenode**);
extern NCerror prefetchdata3(NCDRNO*);
extern NCerror buildcachenode3(NCDRNO*, NCconstraint*, NClist*, NCcachenode**, int);
extern NCcachenode* createnccachenode(void);
extern void clearnccache(NCDRNO*,NCcache*);
extern void freenccachenode(NCDRNO*, NCcachenode*);

#endif /*CACHE3_H*/
