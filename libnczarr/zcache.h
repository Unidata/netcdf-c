/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef ZCACHE_H
#define ZCACHE_H

typedef struct NCZCacheEntry {
    int modified;
    size64_t indices[NC_MAX_VAR_DIMS];
    char* key; /* as string */
    void* data;
} NCZCacheEntry;

typedef struct NCZChunkCache {
    const NC_VAR_INFO_T* var; /* backlink */
    size64_t chunksize;
    void* fillchunk; /* enough fillvalues to fill a chunk */
    size_t maxentries; /* Max number of entries allowed */
    NClist* entries; /* in LRU order */
} NCZChunkCache;

/**************************************************/

extern int NCZ_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption);
extern int NCZ_adjust_var_cache(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);

extern int NCZ_create_chunk_cache(NC_VAR_INFO_T* var, size64_t, NCZChunkCache** cachep);
extern void NCZ_free_chunk_cache(NCZChunkCache* cache);
extern int NCZ_read_cache_chunk(NCZChunkCache* cache, const size64_t* indices, void** datap);
extern int NCZ_flush_chunk_cache(NCZChunkCache* cache);
extern size64_t NCZ_cache_entrysize(NCZChunkCache* cache);
extern NCZCacheEntry* NCZ_cache_entry(NCZChunkCache* cache, const size64_t* indices);
extern size64_t NCZ_cache_size(NCZChunkCache* cache);
extern int NCZ_buildchunkpath(NCZChunkCache* cache, const size64_t* chunkindices, char** keyp);

#endif /*ZCACHE_H*/
