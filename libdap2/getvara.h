/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/getvara.h,v 1.11 2010/05/27 21:34:08 dmh Exp $
 *********************************************************************/
#ifndef GETVARA_H
#define GETVARA_H

/*
Define the overall maximum cache size
and the per-retrieval maximum size
*/

/* Use slightly misspelled names to avoid conflicts */
#define KILBYTE 0x400
#define MEGBYTE 0x100000
#define GIGBYTE 0x40000000

/* The cache limit is in terms of bytes */
#define DFALTCACHELIMIT (100*MEGBYTE)
/* The fetch limit is in terms of bytes */
#define DFALTFETCHLIMIT (100*KILBYTE)

/* WARNING: The small limit is in terms of the # of vector elements */
#define DFALTSMALLLIMIT (1*KILBYTE)

/* Max number of cache nodes */
#define DFALTCACHECOUNT (100)

#ifdef IGNORE
typedef int nc_tactic;
#define tactic_null	0
#define tactic_all	1
#define tactic_partial	2
#define tactic_grid	4
#define tactic_var	8
#endif

typedef struct Getvara {
    void* memory; /* where result is put*/
    struct NCcachenode* cache;
    struct DCEprojection* varaprojection;
    /* associated variable*/
    OCtype dsttype;
    CDFnode* target;
#ifdef IGNORE
    CDFnode* target0;
#endif
    int wholevariable;
} Getvara;

#ifdef IGNORE
extern int varamergeprojections3(NClist*,DCEprojection*,NClist*,NClist**);
#endif

#endif /*GETVARA_H*/
