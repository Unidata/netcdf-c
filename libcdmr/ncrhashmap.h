/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#ifndef NCRHASHMAP_H
#define NCRHASHMAP_H 1

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
#define externC extern "C"
#else
#define externC extern
#endif

#include "ncrlist.h"

/* Define the type of the elements in the hashmap*/
typedef unsigned long ncrhashid;

externC int ncrhashnull(ncrelem);

typedef struct NCRhashmap {
  int alloc;
  int size; /* # of pairs still in table*/
  NCRlist** table;
} NCRhashmap;

externC NCRhashmap* ncrhashnew(void);
externC NCRhashmap* ncrhashnew0(int);
externC int ncrhashfree(NCRhashmap*);

/* Insert a (ncrncrhashid,ncrelem) pair into the table*/
/* Fail if already there*/
externC int ncrhashinsert(NCRhashmap*, ncrhashid ncrhash, ncrelem value);

/* Insert a (ncrhashid,ncrelem) pair into the table*/
/* Overwrite if already there*/
externC int ncrhashreplace(NCRhashmap*, ncrhashid ncrhash, ncrelem value);

/* lookup a ncrhashid and return found/notfound*/
externC int ncrhashlookup(NCRhashmap*, ncrhashid ncrhash, ncrelem* valuep);

/* lookup a ncrhashid and return 0 or the value*/
externC ncrelem ncrhashget(NCRhashmap*, ncrhashid ncrhash);

/* remove a ncrhashid*/
externC int ncrhashremove(NCRhashmap*, ncrhashid ncrhash);

/* Return the ith pair; order is completely arbitrary*/
/* Can be expensive*/
externC int ncrhashith(NCRhashmap*, int i, ncrhashid*, ncrelem*);

externC int ncrhashkeys(NCRhashmap* hm, ncrhashid** keylist);

/* return the # of pairs in table*/
#define ncrhashsize(hm) ((hm)?(hm)->size:0)

#endif /*NCRHASHMAP_H*/

