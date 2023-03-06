/* Copyright 2018, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions. */

/**
 * @file @internal The functions which control NCZ
 * caching. These caching controls allow the user to change the cache
 * sizes of ZARR before opening files.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#include "zincludes.h"
#include "zcache.h"

#undef DEBUG

#undef FILLONREAD

#undef FLUSH

/* Forward */
static int get_chunk(NCZChunkCache* cache, const char* key, NCZCacheEntry* entry);
static int put_chunk(NCZChunkCache* cache, const char* key, const NCZCacheEntry*);
static int create_chunk(NCZChunkCache* cache, const char* key, NCZCacheEntry* entry);
static int buildchunkkey(size_t R, const size64_t* chunkindices, char** keyp);
static int makeroom(NCZChunkCache* cache);

/**************************************************/
/* Dispatch table per-var cache functions */

/**
 * @internal Set chunk cache size for a variable. This is the internal
 * function called by nc_set_var_chunk_cache().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param size Size in bytes to set cache.
 * @param nelems # of entries in cache
 * @param preemption Controls cache swapping.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict
 * nc3 netcdf-4 file.
 * @returns ::NC_EINVAL Invalid input.
 * @returns ::NC_EHDFERR HDF5 error.
 * @author Ed Hartnett
 */
int
NCZ_set_var_chunk_cache(int ncid, int varid, size_t cachesize, size_t nelems, float preemption)
{
    NC_GRP_INFO_T *grp;
    NC_FILE_INFO_T *h5;
    NC_VAR_INFO_T *var;
    NCZ_VAR_INFO_T *zvar;
    int retval;

    /* Check input for validity. */
    if (preemption < 0 || preemption > 1)
        return NC_EINVAL;

    /* Find info for this file and group, and set pointer to each. */
    if ((retval = nc4_find_nc_grp_h5(ncid, NULL, &grp, &h5)))
        return retval;
    assert(grp && h5);

    /* Find the var. */
    if (!(var = (NC_VAR_INFO_T *)ncindexith(grp->vars, varid)))
        return NC_ENOTVAR;
    assert(var && var->hdr.id == varid);

    zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    assert(zvar != NULL && zvar->cache != NULL);

    /* Set the values. */
    var->chunk_cache_size = cachesize;
    var->chunk_cache_nelems = nelems;
    var->chunk_cache_preemption = preemption;

#ifdef LOOK
    /* Reopen the dataset to bring new settings into effect. */
    if ((retval = nc4_reopen_dataset(grp, var)))
        return retval;
#endif
    return NC_NOERR;
}

/**
 * @internal Adjust the chunk cache of a var for better
 * performance.
 *
 * @note For contiguous and compact storage vars, or when parallel I/O
 * is in use, this function will do nothing and return ::NC_NOERR;
 *
 * @param grp Pointer to group info struct.
 * @param var Pointer to var info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NCZ_adjust_var_cache(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var)
{
    /* Reset the cache parameters since var chunking may have changed */
    

    return NC_NOERR;
}

/**************************************************/

/**
 * Create a chunk cache object
 *
 * @param var containing var
 * @param entrysize Size in bytes of an entry
 * @param cachep return cache pointer
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Bad preemption.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_create_chunk_cache(NC_VAR_INFO_T* var, size64_t chunksize, NCZChunkCache** cachep)
{
    int stat = NC_NOERR;
    NCZChunkCache* cache = NULL;
    void* fill = NULL;
    size_t nelems, cachesize;
    NCZ_VAR_INFO_T* zvar = NULL;
	
    if(chunksize == 0) return NC_EINVAL;

    zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    
    if((cache = calloc(1,sizeof(NCZChunkCache))) == NULL)
	{stat = NC_ENOMEM; goto done;}
    cache->var = var;
    cache->ndims = var->ndims + zvar->scalar;
    cache->chunksize = chunksize;
    assert(cache->fillchunk == NULL);
    cache->fillchunk = NULL;

    /* Figure out the actual cache size */
    cachesize = var->chunk_cache_size;
    nelems = (cachesize / chunksize);
    if(nelems == 0) nelems = 1;
    /* Make consistent */
    cachesize = nelems * chunksize;
    cache->maxentries = nelems;
#ifdef FLUSH
    cache->maxentries = 1;
#endif

#ifdef DEBUG
    fprintf(stderr,"%s.cache: nelems=%ld size=%ld\n",
        var->hdr.name,(unsigned long)cache->maxentries,(unsigned long)(cache->maxentries*cache->chunksize));
#endif
    if((cache->entries = nclistnew()) == NULL)
	{stat = NC_ENOMEM; goto done;}
    nclistsetalloc(cache->entries,cache->maxentries);
    if(cachep) {*cachep = cache; cache = NULL;}
done:
    nullfree(fill);
    nullfree(cache);
    return THROW(stat);
}

void
NCZ_free_chunk_cache(NCZChunkCache* cache)
{
    if(cache == NULL) return;
    /* Iterate over the entries */
    while(nclistlength(cache->entries) > 0) {
        NCZCacheEntry* entry = nclistremove(cache->entries,0);
	nullfree(entry->data); nullfree(entry->key); nullfree(entry);
    }
#ifdef DEBUG
fprintf(stderr,"|cache.free|=%ld\n",nclistlength(cache->entries));
#endif
    nclistfree(cache->entries);
    cache->entries = NULL;
    nullfree(cache->fillchunk);
    nullfree(cache);
}

size64_t
NCZ_cache_entrysize(NCZChunkCache* cache)
{
    assert(cache);
    return cache->chunksize;
}

/* Return number of active entries in cache */
size64_t
NCZ_cache_size(NCZChunkCache* cache)
{
    assert(cache);
    return nclistlength(cache->entries);
}

int
NCZ_read_cache_chunk(NCZChunkCache* cache, const size64_t* indices, void** datap)
{
    int stat = NC_NOERR;
    char* key = NULL;
    int rank = cache->ndims;
    NC_FILE_INFO_T* file = cache->var->container->nc4_info;
    NCZCacheEntry* entry = NULL;
    int i;
	
    /* Create the key for this cache */
    if((stat = NCZ_buildchunkpath(cache,indices,&key))) goto done;

    /* See if already in cache try MRU */
    for(i=nclistlength(cache->entries)-1;i>=0;i--) {
	entry = (NCZCacheEntry*)nclistget(cache->entries,i);
	if(strcmp(key,entry->key)==0) {
            if(datap) *datap = entry->data;
	    /* Move to keep MRU at end */
	    nclistremove(cache->entries,i);	    
	    break;
        } else entry = NULL;
    }
    if(entry == NULL) { /*!found*/
	/* Make room in the cache */
	if((stat=makeroom(cache))) goto done;
	/* Create a new entry */
	if((entry = calloc(1,sizeof(NCZCacheEntry)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	memcpy(entry->indices,indices,rank*sizeof(size64_t));
	/* Create the local copy space */
	if((entry->data = calloc(1,cache->chunksize)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	entry->key= key; key = NULL;
	/* Try to read the object in toto */
	stat=get_chunk(cache,entry->key,entry);
	switch (stat) {
	case NC_NOERR: break;
	case NC_EEMPTY:
	case NC_ENOTFOUND: /*signals the chunk needs to be created */
	    /* If the file is read-only, then fake the chunk */
	    entry->modified = (!file->no_write);
	    if(!file->no_write) {
                if((stat = create_chunk(cache,entry->key,entry))) goto done;
	    }
#ifdef FILLONREAD
	    /* apply fill value */
	    memcpy(entry->data,cache->fillchunk,cache->chunksize);
#else
	    memset(entry->data,0,cache->chunksize);
#endif
	    break;
	default: goto done;
	}
    }
    nclistpush(cache->entries,entry);
#ifdef DEBUG
fprintf(stderr,"|cache.read.lru|=%ld\n",nclistlength(cache->entries));
#endif
    if(datap) *datap = entry->data;
    entry = NULL;

done:
    if(entry) {nullfree(entry->data); nullfree(entry->key);}
    nullfree(entry);
    nullfree(key);
    return THROW(stat);
}

int
NCZ_write_cache_chunk(NCZChunkCache* cache, const size64_t* indices, void** datap)
{
    int stat = NC_NOERR;
    char* key = NULL;
    int i,rank = cache->ndims;
    NCZCacheEntry* entry = NULL;
    
    /* Create the key for this cache */
    if((stat = NCZ_buildchunkpath(cache,indices,&key))) goto done;

    /* See if already in cache try MRU */
    for(i=nclistlength(cache->entries)-1;i>=0;i--) {
	entry = (NCZCacheEntry*)nclistget(cache->entries,i);
	if(strcmp(key,entry->key)==0) {
            if(datap) *datap = entry->data;
	    /* Move to keep MRU at end */
	    nclistremove(cache->entries,i);	    
	    break;
        } else entry = NULL;
    }
    if(entry == NULL) { /*!found*/
	if((stat=makeroom(cache))) goto done;
	/* Create a new entry */
	if((entry = calloc(1,sizeof(NCZCacheEntry)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	memcpy(entry->indices,indices,rank*sizeof(size64_t));
	/* Create the local copy space */
	if((entry->data = calloc(1,cache->chunksize)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	entry->key= key; key = NULL;
    }
    entry->modified = 1;
    nclistpush(cache->entries,entry); /* MRU order */
#ifdef DEBUG
fprintf(stderr,"|cache.write|=%ld\n",nclistlength(cache->entries));
#endif
    entry = NULL;

done:
    if(entry) {nullfree(entry->data); nullfree(entry->key);}
    nullfree(entry);
    nullfree(key);
    return THROW(stat);
}

static int
makeroom(NCZChunkCache* cache)
{
    int stat = NC_NOERR;
    /* Flush from LRU end if we are at capacity */
    while(nclistlength(cache->entries) >= cache->maxentries) {
	NCZCacheEntry* e = nclistremove(cache->entries,0);
	assert(e != NULL);
	if(e->modified) /* flush to file */
	    stat=put_chunk(cache,e->key,e);
	/* reclaim */
        nullfree(e->data); nullfree(e->key); nullfree(e);
    }
#ifdef DEBUG
fprintf(stderr,"|cache.makeroom|=%ld\n",nclistlength(cache->entries));
#endif
    return stat;
}

int
NCZ_flush_chunk_cache(NCZChunkCache* cache)
{
    int stat = NC_NOERR;
    size_t i;

    if(NCZ_cache_size(cache) == 0) goto done;
    
    /* Iterate over the entries in hashmap */
    for(i=0;i<nclistlength(cache->entries);i++) {
        NCZCacheEntry* entry = nclistget(cache->entries,i);
        if(entry->modified) {
	    /* Write out this chunk in toto*/
  	    if((stat=put_chunk(cache,entry->key,entry)))
	        goto done;
	}
        entry->modified = 0;
    }

done:
    return THROW(stat);
}

#if 0
int
NCZ_chunk_cache_modified(NCZChunkCache* cache, const size64_t* indices)
{
    int stat = NC_NOERR;
    char* key = NULL;
    NCZCacheEntry* entry = NULL;
    int rank = cache->ndims;

    /* Create the key for this cache */
    if((stat=buildchunkkey(rank, indices, &key))) goto done;

    /* See if already in cache */
    if(NC_hashmapget(cache->entries, key, strlen(key), (uintptr_t*)entry)) { /* found */
	entry->modified = 1;
    }

done:
    nullfree(key);
    return THROW(stat);
}
#endif

/**************************************************/
/*
From Zarr V2 Specification:
"The compressed sequence of bytes for each chunk is stored under
a key formed from the index of the chunk within the grid of
chunks representing the array.  To form a string key for a
chunk, the indices are converted to strings and concatenated
with the dimension_separator character ('/' or '.') separating each index. For
example, given an array with shape (10000, 10000) and chunk
shape (1000, 1000) there will be 100 chunks laid out in a 10 by
10 grid. The chunk with indices (0, 0) provides data for rows
0-1000 and columns 0-1000 and is stored under the key "0.0"; the
chunk with indices (2, 4) provides data for rows 2000-3000 and
columns 4000-5000 and is stored under the key "2.4"; etc."
*/

/**
 * @param R Rank
 * @param chunkindices The chunk indices
 * @param keyp Return the chunk key string
 */
static int
buildchunkkey(size_t R, const size64_t* chunkindices, char** keyp)
{
    int stat = NC_NOERR;
    int r;
    NCbytes* key = ncbytesnew();

    if(keyp) *keyp = NULL;
    
    for(r=0;r<R;r++) {
	char sindex[64];
        if(r > 0) ncbytescat(key,".");
	/* Print as decimal with no leading zeros */
	snprintf(sindex,sizeof(sindex),"%lu",(unsigned long)chunkindices[r]);	
	ncbytescat(key,sindex);
    }
    ncbytesnull(key);
    if(keyp) *keyp = ncbytesextract(key);

    ncbytesfree(key);
    return THROW(stat);
}

/**
 * @internal Push data to chunk of a file.
 * If chunk does not exist, create it
 *
 * @param file Pointer to file info struct.
 * @param proj Chunk projection
 * @param datalen size of data
 * @param data Buffer containing the chunk data to write
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
put_chunk(NCZChunkCache* cache, const char* key, const NCZCacheEntry* entry)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;
    NCZMAP* map = NULL;

    LOG((3, "%s: var: %p", __func__, cache->var));

    zfile = ((cache->var->container)->nc4_info)->format_file_info;
    map = zfile->map;

    stat = nczmap_write(map,key,0,cache->chunksize,entry->data);
    switch(stat) {
    case NC_NOERR: break;
    case NC_EEMPTY:
	/* Create the chunk */
	switch (stat = nczmap_defineobj(map,key)) {
	case NC_NOERR: case NC_EFOUND: break;
	default: goto done;
	}
	/* write again */
	if((stat = nczmap_write(map,key,0,cache->chunksize,entry->data)))
	    goto done;
	break;
    default: goto done;
    }
done:
    return THROW(stat);
}

/**
 * @internal Push data from memory to file.
 *
 * @param cache Pointer to parent cache
 * @param key chunk key
 * @param entry cache entry to read into
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
get_chunk(NCZChunkCache* cache, const char* key, NCZCacheEntry* entry)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    NC_FILE_INFO_T* file = NULL;
    NCZ_FILE_INFO_T* zfile = NULL;

    LOG((3, "%s: file: %p", __func__, file));

    file = (cache->var->container)->nc4_info;
    zfile = file->format_file_info;
    map = zfile->map;
    assert(map && entry->data);

    stat = nczmap_read(map,key,0,cache->chunksize,(char*)entry->data);

    return THROW(stat);
}

static int
create_chunk(NCZChunkCache* cache, const char* key, NCZCacheEntry* entry)
{
    int stat = NC_NOERR;
    NC_FILE_INFO_T* file = NULL;
    NCZ_FILE_INFO_T* zfile = NULL;
    NCZMAP* map = NULL;

    file = (cache->var->container)->nc4_info;
    zfile = file->format_file_info;
    map = zfile->map;

    /* Create the chunk */
    if((stat = nczmap_defineobj(map,key))) goto done;
    entry->modified = 1; /* mark as modified */
    /* let higher function decide on fill */

done:
    return THROW(stat);
}

int
NCZ_buildchunkpath(NCZChunkCache* cache, const size64_t* chunkindices, char** keyp)
{
    int stat = NC_NOERR;
    char* chunkname = NULL;
    char* varkey = NULL;
    char* key = NULL;

    /* Get the chunk object name */
    if((stat = buildchunkkey(cache->ndims, chunkindices, &chunkname))) goto done;
    /* Get the var object key */
    if((stat = NCZ_varkey(cache->var,&varkey))) goto done;
    /* Prefix the path to the containing variable object */
    if((stat=nczm_concat(varkey,chunkname,&key))) goto done;
    if(keyp) {*keyp = key; key = NULL;}

done:
    nullfree(chunkname);
    nullfree(varkey);
    nullfree(key);
    return THROW(stat);
}

