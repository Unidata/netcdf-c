/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#include "nchashmap.h"

/* Hack to access internal state of a hashmap. Use with care */
/* Convert an entry from ACTIVE to DELETED;
   Return 0 if not found.
*/
extern int NC_hashmapdeactivate(NC_hashmap*, uintptr_t data);

#define ACTIVE 1

/* Locate object by name in an NCindex */
NC_OBJ*
ncindexlookup(NCindex* ncindex, const char* name)
{
    NC_OBJ* obj = NULL;
    uintptr_t index;
    if(ncindex == NULL || name == NULL)
	return NULL;
    assert(ncindex->namemap != NULL);
    if(!NC_hashmapget(ncindex->namemap,(void*)name,&index))
	    return NULL; /* not present */
    obj = (NC_OBJ*)nclistget(ncindex->list,(size_t)index);
    INVARIANTID(ncindex,obj);
    return obj;
}

/* Add object to the end of the vector, also insert into the name map */
/* Return 1 if ok, 0 otherwise.*/
int
ncindexadd(NCindex* ncindex, NC_OBJ* obj)
{
   if(ncindex == NULL || obj == NULL) return 0;
   obj->reserved = nclistlength(ncindex->list); /* maintain invariant */
   if(!NC_hashmapadd(ncindex->namemap,(uintptr_t)obj->reserved,obj->name))
	return 0;
   if(!nclistpush(ncindex->list,obj))
	return 0;
   INVARIANTID(ncindex,obj);
   return 1;
}

#if 0
/* Insert object at ith position of the vector, also insert into the name map; */
/* Return 1 if ok, 0 otherwise.*/
int
ncindexset(NCindex* ncindex, size_t i, NC_OBJ* obj)
{
   uintptr_t index = (uintptr_t)i;
   if(ncindex == NULL) return 0;
   if(!nclistset(ncindex->list,i,obj)) return 0;
   NC_hashmapadd(ncindex->namemap,index,(void*)obj->name);
   return 1;
}
#endif

/**
 * Remove object from the index
 * Return 1 if found, 0 otherwise.*/
int
ncindexremove(NCindex* index, NC_OBJ* obj)
{
   NC_OBJ* o2;
   if(index == NULL) return 0;
   INVARIANTID(index,obj);
   o2 = nclistremove(index->list,obj->reserved);
   if(o2 == NULL || o2 != obj) return 0;

   /* Remove from the hash map by deactivating its entry */
   if(!NC_hashmapdeactivate(index->namemap,obj))
	return 0; /* not present */
   return 1;
}

/*
Rebuild the list map by rehashing all entries
using their current, possibly changed id and name;
also recompute their hashkey.
*/
/* Return 1 if ok, 0 otherwise.*/
static int
ncindexrebuild(NCindex* index)
{ 
#if 0
    size_t i;
    size_t size = nclistlength(index->list);
    NC_OBJ** contents = (NC_OBJ**)nclistextract(index->list);
    /* Reset the index map and list*/
    nclistfree(index->list);
    index->list = nclistnew();
    nclistsetalloc(index->list,size);
    NC_hashmapfree(index->namemap);
    index->namemap = NC_hashmapnew(size);
    /* Now, reinsert all the objects  except NULLs */
    for(i=0;i<size;i++) {
	NC_OBJ* tmp = contents[i];
	if(tmp == NULL) continue; /* ignore */
	if(!ncindexadd(index,tmp))
	    return 0;
    }
    if(contents != NULL) free(contents);
#endif
    return 0;
}

/*
Remove and re-insert a renamed object.
Assume obj->name is new name, oldname is the previous name.
*/

static int
ncindexreinsert(NCindex* index, NC_OBJ* obj, const char* oldname)
{
    uintptr_t data;
    if(index == NULL || obj == NULL || oldname == NULL)
	return 0;
    INVARIANTID(index,obj);
    /* Remove from namemap using old key */
    if(!NC_hashmapremove(index->namemap,oldname,&data))
	return 0;
#ifdef NCINDEXINVARIANT
    assert((obj == (NC_OBJ*)nclistget(index->list,(size_t)data)));
#endif
    /* Reinsert using new name and old data */
    if(!NC_hashmapadd(index->namemap,obj->reserved,obj->name))
	return 0;
    return 1;
}

/* Free a list map */
int
ncindexfree(NCindex* index)
{
    if(index == NULL) return 1;
    nclistfree(index->list);
    NC_hashmapfree(index->namemap);
    free(index);    
    return 1;
}

/* Create a new index */
NCindex*
ncindexnew(size_t size0)
{
    NCindex* index = NULL;
    size_t size = (size0 == 0 ? DFALTTABLESIZE : size0);
    index = calloc(1,sizeof(NCindex));
    if(index == NULL) return NULL;
    index->list = nclistnew();
    if(index->list == NULL) {ncindexfree(index); return NULL;}
    nclistsetalloc(index->list,size);
    index->namemap = NC_hashmapnew(size);
    if(index->namemap == NULL) {ncindexfree(index); return NULL;}
    return index;
}

int
ncindexverify(NCindex* lm, int dump)
{
    size_t i;
    NClist* l = lm->list;
    int nerrs = 0;
    size_t m;

    if(lm == NULL) {
	fprintf(stderr,"index: <empty>\n");
	return 1;
    }  
    if(dump) {
	fprintf(stderr,"-------------------------\n");
        if(lm->namemap->active == 0) {
	    fprintf(stderr,"hash: <empty>\n");
	    goto next1;
	}
	for(i=0;i < lm->namemap->alloc; i++) {
	    NC_hentry* e = &lm->namemap->table[i];
	    if(e->flags != 1) continue;
	    fprintf(stderr,"hash: %ld: data=%lu key=%s\n",(unsigned long)i,(unsigned long)e->data,e->key);
	    fflush(stderr);
	}
next1:
        if(nclistlength(l) == 0) {
	    fprintf(stderr,"list: <empty>\n");
	    goto next2;
	}
	for(i=0;i < nclistlength(l); i++) {
	    const char** a = (const char**)nclistget(l,i);
	    fprintf(stderr,"list: %ld: name=%s\n",(unsigned long)i,*a);
	    fflush(stderr);
	}
	fprintf(stderr,"-------------------------\n");
	fflush(stderr);
    }

next2:
    /* Need to verify that every entry in map is also in vector and vice-versa */

    /* Verify that map entry points to same-named entry in vector */
    for(m=0;m < lm->namemap->alloc; m++) {
	NC_hentry* e = &lm->namemap->table[m];
        char** object = NULL;
	char* oname = NULL;
	uintptr_t udata = (uintptr_t)e->data;
	if((e->flags & ACTIVE) == 0) continue;
	object = nclistget(l,(size_t)udata);
        if(object == NULL) {
	    fprintf(stderr,"bad data: %d: %lu\n",(int)m,(unsigned long)udata);
	    nerrs++;
	} else {
	    oname = *object;
	    if(strcmp(oname,e->key) != 0)  {
	        fprintf(stderr,"name mismatch: %d: %lu: hash=%s list=%s\n",
			(int)m,(unsigned long)udata,e->key,oname);
	        nerrs++;
	    }
	}
    }
    /* Walk vector and mark corresponding hash entry*/
    if(nclistlength(l) == 0 || lm->namemap->active == 0)
	goto done; /* cannot verify */
    for(i=0;i < nclistlength(l); i++) {
	int match;
	const char** xp = (const char**)nclistget(l,i);
        /* Walk map looking for *xp */
	for(match=0,m=0;m < lm->namemap->active; m++) {
	    NC_hentry* e = &lm->namemap->table[m];
	    if((e->flags & 1) == 0) continue;
	    if(strcmp(e->key,*xp)==0) {
		if((e->flags & 128) == 128) {
		    fprintf(stderr,"%ld: %s already in map at %ld\n",(unsigned long)i,e->key,(unsigned long)m);
		    nerrs++;
		}
		match = 1;
		e->flags += 128;
	    }
	}
	if(!match) {
	    fprintf(stderr,"mismatch: %d: %s in vector, not in map\n",(int)i,*xp);
	    nerrs++;
	}
    }
    /* Verify that every element in map in in vector */
    for(m=0;m < lm->namemap->active; m++) {
	NC_hentry* e = &lm->namemap->table[m];
	if((e->flags & 1) == 0) continue;
	if((e->flags & 128) == 128) continue;
	/* We have a hash entry not in the vector */
	fprintf(stderr,"mismatch: %d: %s->%lu in hash, not in vector\n",(int)m,e->key,(unsigned long)e->data);
	nerrs++;
    }
    /* clear the 'touched' flag */
    for(m=0;m < lm->namemap->active; m++) {
	NC_hentry* e = &lm->namemap->table[m];
	e->flags &= ~128;
    }

done:
    fflush(stderr);
    return (nerrs > 0 ? 0: 1);
}

void
printindexmap(NCindex* lm)
{
    if(lm == NULL) {
	fprintf(stderr,"<empty>\n");
	return;
    }
    printhashmap(lm->namemap);
}
