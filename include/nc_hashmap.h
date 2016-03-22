#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED

/** Hashmap structure (forward declaration) */
struct s_hashmap;
typedef struct s_hashmap NC_hashmap;

/** Creates a new hashmap near the given size. */
extern NC_hashmap* NC_hashmapCreate(int startsize);

/** Inserts a new element into the hashmap. */
extern void NC_hashmapInsert(NC_hashmap*, int data, unsigned long key);

/** Removes the storage for the element of the key and returns the element. */
extern int NC_hashmapRemove(NC_hashmap*, unsigned long key);

/** Returns the element for the key. */
extern int NC_hashmapGet(NC_hashmap*, unsigned long key);

/** Returns the number of saved elements. */
extern long NC_hashmapCount(NC_hashmap*);

/** Removes the hashmap structure. */
extern void NC_hashmapDelete(NC_hashmap*);

#endif
