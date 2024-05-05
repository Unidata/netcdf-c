/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#ifndef NETCDF_PROPLIST_H
#define NETCDF_PROPLIST_H

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdint.h>
#include "ncexternl.h"

/*
WARNING:
If you modify this file,
then you need to got to
the include/ directory
and do the command:
    make makepluginjson
*/

/* Inside libnetcdf and for plugins, export the json symbols */
#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif
#endif

/* Override for plugins */
#ifndef OPTEXPORT
#ifdef NETCDF_PROPLIST_H
#define OPTEXPORT static
#else
#define OPTEXPORT DLLEXPORT
#endif
#endif

/**************************************************/
/*
This is used to store a property list mapping a small number of
fixed-sized key strings to an arbitrary uintptr_t value.  The
uintptr_t type is used to ensure that the value can be a pointer or a
small string upto sizeof(uintptr_t) - 1 (for trailing nul).  The big
problem is reclaiming the value if it a pointer.  The fact that the
number of keys is small makes it feasible to use linear search.
This is currently only used for plugins, but may be extended to other uses.
*/

/*! Proplist-related structs.
  NOTES:
  1. 'value' is the an arbitrary uintptr_t integer or void* pointer.

  WARNINGS:
  1. It is critical that |uintptr_t| == |void*|
*/

#define NCPROPSMAXKEY 31 /* characters assert (NCPROPSMAXKEY+1)/8 == 0*/

/* Returns 0 => error; 1 => success */
typedef int (*NCPreclaimfcn)(uintptr_t userdata, const char* key, void* value, uintptr_t size);

/* The property list proper is a sequence of these objects */
typedef struct NCProperty {
    char key[NCPROPSMAXKEY+1]; /* copy of the key string; +1 for trailing nul */
    uintptr_t flags;
#       define NCPF_SIMPLE  (1<<0) /* non-reclaimable */
#       define NCPF_BYTES   (1<<1) /* reclaimable bytes */
#       define NCPF_COMPLEX (1<<2) /* extended case */
    uintptr_t value;
    uintptr_t size;        /* size = |value| as ptr to memory, if string, then include trailing nul */
    uintptr_t userdata;    /* extra data for following functions */
    NCPreclaimfcn reclaim;
} NCProperty;

/*
The property list object.
*/
typedef struct NCproplist {
  size_t alloc; /* allocated space to hold properties */
  size_t count; /* # of defined properties */
  NCProperty* properties;
} NCproplist;

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
extern "C" {
#endif

#ifndef NETCDF_PROPLIST_H

/* Suppress create/modify function for plugin use */

/* Create, free, etc. */
OPTEXPORT NCproplist* ncplistnew(void);
OPTEXPORT int ncplistfree(NCproplist*);

/* Insert properties */
OPTEXPORT int ncplistadd(NCproplist* plist,const char* key, uintptr_t value); /* use when reclaim not needed */
OPTEXPORT int ncplistaddstring(NCproplist* plist, const char* key, const char* str); /* use when value is simple string (char*) */
OPTEXPORT int ncplistaddbytes(NCproplist* plist, const char* key, void* value, uintptr_t size); /* use when value is simple ptr and reclaim is simple free function */
OPTEXPORT int ncplistaddx(NCproplist* plist, const char* key, void* value, uintptr_t size, uintptr_t userdata, NCPreclaimfcn); /* fully extended case */

#endif /*NETCDF_PROPLIST_H*/

/* 
Lookup key and return value.
Return 1 if found, 0 otherwise; returns the data in datap if !null
*/
OPTEXPORT int ncplistget(const NCproplist*, const char* key, uintptr_t* datap, uintptr_t* sizep);

/* Iteration support */

/* Return the number of properties in the property list */
#define ncplistlen(plist) (((NCproplist)(plist))->count)

/* get the ith key+value */
OPTEXPORT int ncplistith(const NCproplist*, size_t i, char* const * keyp, uintptr_t const * valuep, uintptr_t* sizep);

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
}
#endif

/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include "ncdispatch.h"
#include "nccrc.h"

#undef DEBUG
#define ASSERTIONS

#ifdef ASSERTIONS
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

/**************************************************/
/* Hide everything for plugins */
#ifdef NETCDF_PROPLIST_H
#define OPTSTATIC static
#else /*!NETCDF_PROPLIST_H*/
#define OPTSTATIC
#endif /*NETCDF_PROPLIST_H*/

/**************************************************/

#define MINPROPS 2
#define EXPANDFACTOR 1

#define hasspace(plist,nelems) ((plist)->alloc >= ((plist)->count + (nelems)))

#define emptyprop {"                               ",0,0,0,NULL}

/**************************************************/

#ifndef NETCDF_PROPLIST_H

/* The non-read-only functions are elided in the unified header */

/* Forward */
static int extendplist(NCproplist* plist, size_t nprops);

/**
 * Create new property list
 * @return pointer to the created property list.
 */
OPTSTATIC NCproplist*
ncplistnew(void)
{
   NCproplist* plist = NULL;
   plist = calloc(1,sizeof(NCproplist));
   /* Assume property list will hold at lease MINPROPS properties */
   plist->alloc = MINPROPS;
   plist->count = 0;
   plist->properties = (NCProperty*)calloc(MINPROPS,sizeof(NCProperty));
   return plist;
}

/**
 * Reclaim memory used by a property list
 * @param plist to reclaim
 * @return NC_NOERR if succeed, NC_EXXX otherwise.
 */
OPTSTATIC int
ncplistfree(NCproplist* plist)
{
    int stat = NC_NOERR;
    size_t i;
    if(plist == NULL) goto done;
    if(plist->properties != NULL) {
        for(i=0;i<plist->count;i++) {
            NCProperty* prop = &plist->properties[i];
	    void* ptr = (void*)prop->value; /* convert to ptr */
	    assert(prop->flags & (NCPF_SIMPLE|NCPF_BYTES|NCPF_COMPLEX));
	    if(prop->flags & NCPF_SIMPLE) continue; /* no reclaim needed */
    	    if(prop->flags & NCPF_BYTES) {
		if(ptr != NULL) free(ptr);
	    } else { /* (prop->flags & NCPF_COMPLEX) */
		int ok;
		assert(prop->reclaim != NULL);
		ok = prop->reclaim(prop->userdata, prop->key, ptr, prop->size);
		if(!ok && stat == NC_NOERR) stat = NC_EINVAL;
	    }	
	}
	free(plist->properties);
    }
    free(plist);
done:
    return stat;
}

/**
 * Add a non-reclaimable entry to the property list
 * @param plist into which the value is be inserted.
 * @param key
 * @param value
 * @return NC_NOERR if succeed, NC_EXXX otherwise.
 */
OPTSTATIC int
ncplistadd(NCproplist* plist, const char* key, uintptr_t value)
{
    int stat = NC_NOERR;
    NCProperty* prop = NULL;
    size_t keylen;
    if(plist == NULL) goto done;
    if(!hasspace(plist,1)) {if((stat = extendplist(plist,(plist->count+1)*EXPANDFACTOR))) goto done;} /* extra space */
    prop = &plist->properties[plist->count];
    keylen = strlen(key);
    if(keylen > NCPROPSMAXKEY) keylen = NCPROPSMAXKEY; /* truncate */
    memcpy(prop->key,key,keylen);
    prop->key[keylen] = '\0';
    prop->value = value;
    prop->flags = NCPF_SIMPLE;
    plist->count++;
done:
    return stat;
}

/**
 * Add a reclaimable entry to the property list, where the value
 * can be reclaimed using a simple free();
 * @param plist into which the value is be inserted.
 * @param key
 * @param value ptr to memory chunk
 * @param size |*value|
 * @return NC_NOERR if succeed, NC_EXXX otherwise.
 */
OPTSTATIC int
ncplistaddbytes(NCproplist* plist, const char* key, void* value, uintptr_t size)
{
    int stat = NC_NOERR;
    NCProperty* prop = NULL;
    size_t keylen;
    if(plist == NULL) goto done;
    if(!hasspace(plist,1)) {if((stat = extendplist(plist,(plist->count+1)*EXPANDFACTOR))) goto done;} /* extra space */
    prop = &plist->properties[plist->count];
    keylen = strlen(key);
    if(keylen > NCPROPSMAXKEY) keylen = NCPROPSMAXKEY; /* truncate */
    memcpy(prop->key,key,keylen);
    prop->key[keylen] = '\0';
    prop->value = (uintptr_t)value;
    prop->flags = NCPF_BYTES;
    plist->count++;
done:
    return stat;
}

/**
 * Add a reclaimable entry to the property list, where the value
 * can be reclaimed using a simple free();
 * @param plist into which the value is be inserted.
 * @param key
 * @param value ptr to memory chunk
 * @param size |*value|
 * @return NC_NOERR if succeed, NC_EXXX otherwise.
 */
OPTSTATIC int
ncplistaddstring(NCproplist* plist, const char* key, const char* str)
{
    uintptr_t size = 0;
    if(str) size = (uintptr_t)strlen(str);
    return ncplistaddbytes(plist,key,(void*)str,size);
}

/**
 * Most general case for adding a property.
 * @param plist into which the value is be inserted.
 * @param key
 * @param value
 * @param size
 * @param userdata extra environment data for the reclaim function.
 * @param fcn the reclaim function
 * @return NC_NOERR if succeed, NC_EXXX otherwise.
 */
OPTSTATIC int
ncplistaddx(NCproplist* plist, const char* key, void* value, uintptr_t size, uintptr_t userdata, NCPreclaimfcn fcn)
{
    int stat = NC_NOERR;
    NCProperty* prop = NULL;
    size_t keylen;
    if(plist == NULL) goto done;
    if(!hasspace(plist,1)) {if((stat = extendplist(plist,(plist->count+1)*EXPANDFACTOR))) goto done;} /* extra space */
    prop = &plist->properties[plist->count];
    keylen = strlen(key);
    if(keylen > NCPROPSMAXKEY) keylen = NCPROPSMAXKEY; /* truncate */
    memcpy(prop->key,key,keylen);
    prop->key[keylen] = '\0';
    prop->value = (uintptr_t)value;
    prop->size = size;
    prop->reclaim = fcn;
    prop->userdata = userdata;
    prop->flags = NCPF_COMPLEX;
    plist->count++;
done:
    return stat;
}

/* Increase size of a plist to be at lease nprops properties */
static int
extendplist(NCproplist* plist, size_t nprops)
{
    int stat = NC_NOERR;
    size_t newsize = plist->count + nprops;
    NCProperty* newlist = NULL;
    if((plist->alloc >= newsize) || (nprops == 0))
	goto done; /* Already enough space */
    newlist = realloc(plist->properties,newsize*sizeof(NCProperty));
    if(newlist == NULL) {stat = NC_ENOMEM; goto done;}
    plist->properties = newlist; newlist = NULL;    
    plist->alloc = newsize;
done:
    return stat;
}

#endif /*NETCDF_PROPLIST_H*/

/**
 * Lookup key and return value and size
 * @param plist to search
 * @param key for which to search
 * @param valuep returned value
 * @param sizep returned size
 * @return NC_NOERR if key found, NC_ENOOBJECT if key not found; NC_EXXX otherwise
 */
OPTSTATIC int
ncplistget(const NCproplist* plist, const char* key, uintptr_t* valuep, uintptr_t* sizep)
{
    int stat = NC_ENOOBJECT; /* assume not found til proven otherwise */
    size_t i;
    NCProperty* props;
    uintptr_t value = 0;
    uintptr_t size = 0;
    if(plist == NULL || key == NULL) goto done;
    for(i=0,props=plist->properties;i<plist->count;i++,props++) {
	if(strcmp(props->key,key)==0) {
	    value = props->value;
	    size = props->size;	    
	    stat = NC_NOERR; /* found */
	    break;
	}
    }
    if(valuep) *valuep = value;
    if(sizep) *sizep = size;
done:
    return stat;
}

/* Iteration support */

/**
 * Get the ith key+value.a
 * @param plist to search
 * @param i which property to get.
 * @param keyp return i'th key
 * @param valuep return i'th value
 * @param valuep return i'th size
 * @return NC_NOERR if success, NC_EINVAL otherwise
 */
OPTSTATIC int
ncplistith(const NCproplist* plist, size_t i, char* const * keyp, uintptr_t const * valuep, uintptr_t* sizep)
{
    int stat = NC_NOERR;
    NCProperty* prop = NULL;    
    if(plist == NULL) goto done;
    if(i >= plist->count) {stat = NC_EINVAL; goto done;}
    prop = &plist->properties[i];
    if(keyp) *((char**)keyp) = (char*)prop->key;
    if(valuep) *((uintptr_t*)valuep) = (uintptr_t)prop->value;
    if(sizep) *sizep = prop->size;
done:
    return stat;
}

/**************************************************/
/* Support Functions */

/* Hack to avoid static unused warning */
static void
ncp_supresswarnings(void)
{
    void* ignore;
    ignore = (void*)ncp_supresswarnings;
    ignore = (void*)ncplistget;
    ignore = (void*)ncplistith;
    ignore = ignore;
}
#endif /*NETCDF_PROPLIST_H*/
