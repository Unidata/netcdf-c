#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED
#include <stdlib.h>

typedef struct NC_vararray NC_vararray;
typedef struct NC_dimarray NC_dimarray;

/** Hashmap structure (forward declaration) */
struct s_hashmap;
typedef struct s_hashmap NC_hashmap;

/** Creates a new hashmap near the given size. */
extern NC_hashmap* NC_hashmapCreate(unsigned long startsize);

/** Inserts a new element into the hashmap. */
extern void NC_hashmapAddDim(const NC_dimarray*, long data, const char *name);

/** Removes the storage for the element of the key and returns the element. */
extern long NC_hashmapRemoveDim(const NC_dimarray*, const char *name);

/** Returns the element for the key. */
extern long NC_hashmapGetDim(const NC_dimarray*, const char *name);

/** Inserts a new element into the hashmap. */
extern void NC_hashmapAddVar(const NC_vararray*, long data, const char *name);

/** Removes the storage for the element of the key and returns the element. */
extern long NC_hashmapRemoveVar(const NC_vararray*, const char *name);

/** Returns the element for the key. */
extern long NC_hashmapGetVar(const NC_vararray*, const char *name);

/** Returns the number of saved elements. */
extern unsigned long NC_hashmapCount(NC_hashmap*);

/** Removes the hashmap structure. */
extern void NC_hashmapDelete(NC_hashmap*);

#endif
