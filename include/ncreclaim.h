/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef NCRECLAIM_H
#define NCRECLAIM_H

/* Need definition of uintptr_t */
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/**
Provide a template implementation of
various netcdf3/4 recursive walkers.
The set of walkers currently includes:
1. reclaim - given type information and a pointer
   to a vector of instances, reclaim the whole data
   tree hanging off this vector.
2. clone - given type information and a pointer
   to a vector of instances, deep clone the whole data
   tree hanging off this vector.
Note the the tricky part occurs when we have variable length
data: strings or vlens.
*/

/*
Define the template structure that contains info
for any specific use of this template.

The idea is that the user defines something like this:
struct User_Memstate {
    NC_reclaim_ops ops;
    ... user state fields
};

Then by recasting the passed in NC_reclaim_ops
structure as User_memstate, the users operations
can get access to user-specific data.
We do this rather than using a void* pointer
in the NC_reclaim_Ops structure
*/

/* This is the information we need about a type */
typedef struct NC_reclaim_type {
    uintptr_t typeid;
    int typeclass;
    size_t size; /* mem size */
    uintptr_t basetype;
    size_t nfields;
} NC_reclaim_type;

typedef struct NC_reclaim_ops {
/* Define the dispatch table prototypes */
int (*inq_type)(void* state, uintptr_t typeid, NC_reclaim_type* typeinfo);
int (*inq_compound_field)(void* state, uintptr_t cmpd, int fieldid, size_t* fieldalign, uintptr_t* fieldtype, int* ndims, int* dimsizes);
int (*typealignment)(void* state, uintptr_t typeid);
} NC_reclaim_ops;

/* Externally visible API */

EXTERNL int NC_reclaim_template(void* state, uintptr_t tsym, void* memory, size_t count);
EXTERNL int NC_clone_template(void* state, uintptr_t tsym, void* memory, size_t count);

#endif /*NCRECLAIM_H*/
