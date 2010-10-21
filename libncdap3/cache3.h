/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CACHE3_H
#define CACHE3_H 1

/* Detail information about each cache item */
typedef struct NCcachenode {
    int prefetch; /* is this the prefetch cache entry? */
    size_t xdrsize;
    struct NCconstraint* constraint; /* as used to create this node */
    NClist* vars; /* vars potentially covered by this cache node */
    struct CDFnode* datadds;
    OCobject ocroot;
    OCdata content;
} NCcachenode;

/* All cache info */
typedef struct NCcache {
    size_t cachelimit; /* max total size for all cached entries */
    size_t cachesize; /* current size */
    size_t cachecount; /* max # nodes in cache */
    NCcachenode* prefetch;
    NClist* nodes; /* cache nodes other than prefetch */
} NCcache;


extern int iscached(NCDRNO*, CDFnode*, NCcachenode**);
extern NCerror prefetchdata3(NCDRNO*);
extern NCerror buildcachenode3(NCDRNO*, struct NCconstraint*, NClist*, NCcachenode**, int);
extern NCcachenode* createnccachenode(void);
extern NCcache* createnccache();
extern void freenccache(NCDRNO*,NCcache*);
extern void freenccachenode(NCDRNO*, NCcachenode*);

#endif /*CACHE3_H*/
