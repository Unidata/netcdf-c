/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#ifndef NCPROPLIST_H
#define NCPROPLIST_H

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

#endif /*!NCPROPLIST_H!*/ /* Leave the ! as a tag for sed */
