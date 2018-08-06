/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

/** \file \internal
Internal netcdf-4 functions.

This file contains functions for manipulating ncindex objects.

Warning: This code depends critically on the assumption that
|void*| == |uintptr_t|

*/

/* Define this for debug so that table sizes are small */
#undef SMALLTABLE

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <assert.h>

#include "nc4internal.h"
#include "ncindex.h"

#ifdef SMALLTABLE
/* Keep the table sizes small initially */
#define DFALTTABLESIZE 7
#else
#define DFALTTABLESIZE 37
#endif

/* Common forwards */
static const char* sortname(NC_SORT sort);
void printindexmap(NCindex* lm);

/* Functions independent of the name map implementation */

/* Get ith object in the vector */
NC_OBJ*
ncindexith(NCindex* index, size_t i)
{
   if(index == NULL) return NULL;
   assert(index->list != NULL);
   return nclistget(index->list,i);
}

/* See if x is contained in the index */
/* Return 1 if found, 0 otherwise */
int
ncindexcontains(NCindex* index, NC_OBJ* nco)
{
    int i;
    NClist* list;
    if(index == NULL || nco == NULL) return -1;
    list = index->list;
    for(i=0;i<nclistlength(list);i++) {
	NC_OBJ* o = (NC_OBJ*)list->content[i];
	if(nco == o) return 1;
    }
    return 0;
}

/*Return a duplicate of the index's vector */
/* Return list if ok, NULL otherwise.*/
NC_OBJ**
ncindexdup(NCindex* index)
{
    if(index == NULL || nclistlength(index->list) == 0)
	return NULL;
    return (NC_OBJ**)nclistdup(index->list);
}

/* Count the non-null entries in an NCindex */
int
ncindexcount(NCindex* index)
{
   int count = 0;
   int i;
   for(i=0;i<ncindexsize(index);i++) {
	if(ncindexith(index,i) != NULL) count++;
   }
   return count;
}

/* Pick name map implementation */
#ifdef NCNOHASH
#include "ncindexnohash.hc"
#endif
#ifdef NCSORTLIST
#include "ncindexsorted.hc"
#endif
#ifdef NCHASHED
#include "ncindexhash.hc"
#endif

static const char*
sortname(NC_SORT sort)
{
    switch(sort) {
    case NCNAT: return "NCNAT";
    case NCVAR: return "NCVAR";
    case NCDIM: return "NCDIM";
    case NCATT: return "NCATT";
    case NCTYP: return "NCTYP";
    case NCGRP: return "NCGRP";
    default: break;
    }
    return "unknown";
}

void
printindexlist(NClist* lm)
{
    int i;
    if(lm == NULL) {
	fprintf(stderr,"<empty>\n");
	return;
    }
    for(i=0;i<nclistlength(lm);i++) {
	NC_OBJ* o = (NC_OBJ*)nclistget(lm,i);
	if(o == NULL)
            fprintf(stderr,"[%ld] <null>\n",(unsigned long)i);
	else
            fprintf(stderr,"[%ld] sort=%s name=|%s| id=%lu reserved=%lu\n",
		(unsigned long)i,sortname(o->sort),o->name,
		(unsigned long)o->id, (unsigned long)o->reserved);
    }
}

void
printindex(NCindex* lm)
{
    if(lm == NULL) {
	fprintf(stderr,"<empty>\n");
	return;
    }
    printindexlist(lm->list);
    printindexmap(lm);
}

/**************************************************/
/* Hide semantics of NC_OBJ->tag */
void
ncindexsetdata(NC_OBJ* hdr)
{
#if 0
   hdr->tag = NC_hashmapkey(hdr->name,strlen(hdr->name));    
#endif
}

/*
Wrap Attribute renumbering
Start renumbering with index->list[from].
*/
int
ncindexrenumberid(NCindex* index, size_t from)
{
   int i;
   for(i=from;i<ncindexsize(index);i++) {
      NC_OBJ* obj = ncindexith(index,i);
      if(obj == NULL) continue;
      obj->id--;
   }
   return 1;
}
