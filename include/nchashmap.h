/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#ifndef NCHASHMAP_H
#define NCHASHMAP_H

/*

Data is presumed to be an index into some other table Assume it
can be compared using simple == The key is some hash of some
null terminated string.

One problem here is that we need to do a final equality check on
the name string to avoid an accidental hash collision. It would
be nice if we had a large enough hashkey that was known to have
an extremely low probability of collisions so we could compare
the hashkeys to determine exact match. A quick internet search
indicates that this is rather more tricky than just using
e.g. crc64 or such.  Needs some thought.
*/

/*! Hashmap-related structs.
  NOTES:
  1. 'data' is the an arbitrary uintptr_t integer or void* pointer.
  2. hashkey is a crc32 hash of key
    
  WARNINGS:
  1. It is critical that |uintptr_t| == |void*|
  2. We do a bad hack here because we know that the incoming key
     might be the object id. In this case, we can
     store it directly instead of a pointer to it.
  3. If the incoming is a string, then we we do not copy it;
     this means we do not free it. This has consequences when renaming
     objects. 
*/

typedef struct NC_hentry {
    int flags;
    uintptr_t data;
    unsigned int hashkey; /* Hash id */
    size_t keysize;
    uintptr_t key; /* |key| <= |uintptr_t| =>actual content  else ptr to content */
} NC_hentry;

/*
The hashmap object must give us the hash table (table),
the |table| size, and the # of defined entries in the table
*/
typedef struct NC_hashmap {
  size_t alloc; /* allocated # of entries */
  size_t active; /* # of active entries */
  NC_hentry* table;
} NC_hashmap;

/* defined in nchashmap.c */

/*
There are two "kinds" of functions:
1. those that take the key+size -- they compute the hashkey internally.
2. those that take the hashkey directly
The API here is not complete in that some #1 functions do not have
a corresponding #2 function, and vice-versa.
The NC_hashmapkey function can be used to effectively convert a #2 to a #1.
If both functions cases are present, then the #2 case will be suffixed with 0.
*/

/** Creates a new hashmap near the given size. */
extern NC_hashmap* NC_hashmapnew(size_t startsize);

/** Inserts a new element into the hashmap; takes key+size */
/* key points to size bytes to convert to hash key */
extern int NC_hashmapadd(NC_hashmap*, uintptr_t data, void* key, size_t keysize);

/** Removes the storage for the element of the key; takes key+size.
    Return 1 if found, 0 otherwise; returns the data in datap if !null
*/
extern int NC_hashmapremove(NC_hashmap*, void* key, size_t keysize, uintptr_t* datap);

/** Returns the data for the key; takes key+size.
    Return 1 if found, 0 otherwise; returns the data in datap if !null
*/
extern int NC_hashmapget(NC_hashmap*, void* key, size_t keysize, uintptr_t* datap);

/** Change the data for the specified key; takes hashkey.
    Return 1 if found, 0 otherwise
*/
extern int NC_hashmapsetdata(NC_hashmap*, void* key, size_t keylen, uintptr_t newdata);

/** Returns the number of saved elements. */
extern size_t NC_hashmapcount(NC_hashmap*);

/** Reclaims the hashmap structure. */
extern int NC_hashmapfree(NC_hashmap*);

/* Hacks to access internal state: Use with care */

/* Convert an entry from ACTIVE to DELETED;
   Return 0 if not found.
*/
extern int NC_hashmapdeactivate(NC_hashmap*, uintptr_t data);

/* Return the hash key for specified key; takes key+size*/
extern unsigned int NC_hashmapkey(void* key, size_t size);

#if 0
/** Removes the storage for the element of the key; takes hashkey.
    Return 1 if found, 0 otherwise; returns the data in datap if !null
*/
extern int NC_hashmapremove0(NC_hashmap*, unsigned int key, uintptr_t* datap);

/** Returns the data for the key; takes hashkey
    Return 1 if found, 0 otherwise; returns the data in datap if !null
*/
extern int NC_hashmapget0(NC_hashmap*, unsigned int hashkey, uintptr_t* datap);
#endif

#endif /*NCHASHMAP_H*/

