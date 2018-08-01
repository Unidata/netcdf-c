/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/


/* Forward */
static NC_OBJ* locate(NClist* list, const char* key, size_t* posp);
static int insert(NClist* list, NC_OBJ* obj, size_t* posp);

/* Locate object by name in an NCindex */
NC_OBJ*
ncindexlookup(NCindex* ncindex, const char* name)
{
    NC_OBJ* obj = NULL;
    if(ncindex == NULL || name == NULL)
	return NULL;
    obj = locate(ncindex->namemap,name, NULL);
    INVARIANTID(ncindex,obj);
    return obj;
}

/* Add object to the end of the vector, also insert into the name map */
/* Return 1 if ok, 0 otherwise.*/
int
ncindexadd(NCindex* ncindex, NC_OBJ* obj)
{
   if(ncindex == NULL || obj == NULL) return 0;
   obj->reserved = nclistlength(ncindex->list); /*maintain invariant*/
   if(!nclistpush(ncindex->list,obj))
	return 0;
   if(!insert(ncindex->namemap,obj,NULL))
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
   if(ncindex == NULL) return 0;
   if(!nclistset(ncindex->list,i,obj)) return 0;
   obj->reserved = i; /* maintain invariant */
   if(!insert(ncindex->namemap,obj,NULL)) return 0;
   INVARIANTID(ncindex,obj);
   return 1;
}
#endif

/**
 * Remove ith object from the index and the name map.
 * Return 1 if ok, 0 otherwise.*/
int
ncindexidel(NCindex* index, size_t i)
{
   NC_OBJ* obj;
   if(index == NULL) return 0;
#ifdef NCINDEXINVARIANT
   obj = nclistget(index->list,i);
   if(obj == NULL) return 0;
   INVARIANTID(index,obj);
#endif
   obj = nclistremove(index->list,i);
   if(obj == NULL) return 0;

   /* Locate by name */
   if(!locate((NClist*)index->namemap,obj->name, &i)) return 0;
   if(!nclistremove((NClist*)index->namemap,i)) return 0;
   return 1;
}

/*
Rebuild the index by using the list to rebuild the namemap
using the current, possibly changed id and name;
also recompute the tag.
Return 1 if ok, 0 otherwise.
*/
static int
ncindexrebuild(NCindex* index)
{ 
#if 0 /* unused */
    size_t i;
    size_t size = nclistlength(index->list);

    for(i=0;i<size;i++) {
	NC_OBJ* obj = nclistget(index->namemap,i);
	/* Guarantee that its tag is same as i */
	obj->tag = i;

    /* Now, reinsert all the objects except NULLs */
    for(i=0;i<size;i++) {
	NC_OBJ* tmp = contents[i];
	if(tmp == NULL) continue; /* ignore */
	if(!ncindexadd(index,tmp))
	    return 0;
    }
#endif
    return 1;    
}

/*
Do the necessary fixups to rename an object.
Assume that the NC_OBJ->name is the new name
and old_name is the previous name.
Assume that NC_OBJ->id is the proper new id
Fixup NC_OBJ.reserved.
*/
/* Return 1 if ok, 0 otherwise */
int
ncindexrename(NCindex* index, NC_OBJ* obj, const char* oldname)
{
    size_t i;
    size_t lpos = 0; /* actual pos in list */
    size_t mpos = 0; /* actual pos in namemap */
    int lfound = 0;
    int mfound = 0;
    /* In case the obj->reserved is incorrect, do a linear search of list*/
    for(i=0;i<nclistlength(index->list);i++) {
	NC_OBJ* o2 = nclistget(index->list,i);
	if(obj == o2) {
	    lpos = i;
	    lfound = 1;
            /* Fixup reserved field */
            obj->reserved = lpos;
	    break;
	}
    }
    /* search name map */
    for(i=0;i<nclistlength(index->namemap);i++) {
	NC_OBJ* o2 = nclistget(index->namemap,i);
	if(obj == o2) {
	    mpos = i;
	    mfound = 1;
	    break;
	}
    }
    /* Remove from name map*/
    if(mfound)
	nclistremove(index->namemap,mpos);
    /* Reinsert */
    if(!insert(index->namemap,obj,NULL)) return 0;
    return 1;
}

/* Free a list map */
int
ncindexfree(NCindex* index)
{
    if(index == NULL) return 1;
    nclistfree(index->list);
    nclistfree((NClist*)index->namemap);
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
    if(index == NULL) goto fail;
    index->list = nclistnew();
    if(index->list == NULL) goto fail;
    nclistsetalloc(index->list,size);
    index->namemap = nclistnew();
    if(index->namemap == NULL) goto fail;
    nclistsetalloc((NClist*)index->namemap,size);
    return index;
fail:
    if(index != NULL) ncindexfree(index);
    return NULL;    
}

int
ncindexverify(NCindex* lm, int dump)
{
    size_t i;
    NClist* l = lm->list;
    NClist* lnm = (NClist*)lm->namemap;
    int nerrs = 0;

    if(lm == NULL) {
	fprintf(stderr,"index: <empty>\n");
	return 1;
    }  
    if(dump) {
	fprintf(stderr,"-------------------------\n");
        if(nclistlength(lnm) == 0) {
	    fprintf(stderr,"namemap: <empty>\n");
	    goto next1;
	}
	for(i=0;i < nclistlength(lnm); i++) {
	    NC_OBJ* obj = nclistget(lnm,i);
	    fprintf(stderr,"namemap: %ld: key=%s\n",(unsigned long)i,obj->name);
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
    /* Need to verify that every entry in namemap is also in vector and vice-versa */

    /* Verify |list| == |namemap| */
    if(nclistlength(l) != nclistlength(lnm)) {
        fprintf(stderr,"length mismatch: |list|=%lu |namemap|=%lu\n",
		(unsigned long)nclistlength(l),(unsigned long)nclistlength(lnm));
	goto done;		
    }

    /* Verify that each list entry exists in namemap */
    for(i=0;i < nclistlength(l); i++) {
	NC_OBJ* ol = nclistget(l,i); /* object from vector */
	NC_OBJ* om;
	size_t pos;

	/* See if ol object is in namemap and is matching object*/
	om = locate(lnm,ol->name,&pos);
	if(om == NULL)
	    fprintf(stderr,"missing name map object: %d: %s\n",(int)i,ol->name);
	if(om == ol)
	    fprintf(stderr,"object mismatch: %d: list: %p namemap: %p\n",(int)i,ol,om);

    }
    /* Verify that each namemap entry exists in list */
    for(i=0;i < nclistlength(l); i++) {
	NC_OBJ* ol = nclistget(l,i); /* object from vector */
	NC_OBJ* om;
	size_t pos;

	/* See if ol object is in namemap and is matching object*/
	om = locate(lnm,ol->name,&pos);
	if(om == NULL)
	    fprintf(stderr,"missing name map object: %d: %s\n",(int)i,ol->name);
	if(om == ol)
	    fprintf(stderr,"object mismatch: %d: list: %p namemap: %p\n",(int)i,ol,om);

    }
done:
    fflush(stderr);
    return (nerrs > 0 ? 0: 1);
}

void
printindexmap(NCindex* lm)
{
    int i;
    NClist* map;
    if(lm == NULL || nclistlength(lm->list) == 0) {
	fprintf(stderr,"namemap: <empty>\n");
	return;
    }
    fprintf(stderr,"-------------------------\n");
    map = (NClist*)lm->namemap;
    for(i=0;i < nclistlength(map); i++) {
        NC_OBJ* obj = nclistget(map,i);
        fprintf(stderr,"namemap: %ld: key=%s\n",(unsigned long)i,obj->name);
        fflush(stderr);
    }
}

/* Binary search a set of NC_OBJECT's against a given key;
   return matching object and (optionally) the index of the
   matching object or largest position below it.
   Return NULL if not found.
*/
static NC_OBJ*
locate(NClist* list, const char* key, size_t* posp)
{
    NC_OBJ** objlist;
    int n = nclistlength(list);
    int L = 0;
    int R = n;
    int m, cmp;
    NC_OBJ* p;

    if(n == 0) {if(posp) *posp = -1; return NULL;} /* empty list */
    objlist = (NC_OBJ**)nclistcontents(list);
    /* find position of leftmost element less or equal to key */
    while(L < R) {
        m = (L + R) / 2;
	p = objlist[m];
	cmp = strcmp(p->name,key);
	if(cmp < 0)
	    L = (m + 1);
        else
            R = m;
    }
    if(posp) *posp = (size_t)L; /* where the key is or would go */
    return (cmp == 0 ? p : NULL);
}

/* 
Insert object into proper position in sorted list by name.
Return NC_NOERR is inserted
Return NC_ENOMEM if list expansion failed
Return NC_ENAMEINUSE if name matched object already in list
Optionally return insertion position in posp.
*/
static int
insert(NClist* list, NC_OBJ* obj, size_t* posp)
{
    size_t pos;
    NC_OBJ* p;
    if(nclistlength(list) == 0)
	pos = 0; /* insert here */
    else {    
        p = locate(list,obj->name, &pos);
        if(p != NULL)
	    return NC_ENAMEINUSE; /* already there */
	/* at this point, pos should be insertion point */
    }
    if(!nclistinsert(list,pos,p))
	return NC_ENOMEM;
    if(posp) *posp = pos;
    return NC_NOERR;
}
