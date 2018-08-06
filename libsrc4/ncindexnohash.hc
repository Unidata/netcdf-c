/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

/* Locate object by name in an NCindex */
NC_OBJ*
ncindexlookup(NCindex* ncindex, const char* name)
{
   size_t i;
   if(ncindex == NULL || name == NULL)
	return NULL;
   for(i=0;i<nclistlength(ncindex->list);i++) {
      NC_OBJ* o = nclistget(ncindex->list,i);
      if(o == NULL) continue;
      if(strcmp(name,o->name)==0)
	return o;
    }
    return NULL;
}

/* Add object to the end of the vector, also insert into the name map */
/* Return 1 if ok, 0 otherwise.*/
int
ncindexadd(NCindex* ncindex, NC_OBJ* obj)
{
   if(ncindex == NULL) return 0;
   obj->reserved = nclistlength(ncindex->list); /* maintain invariant */
   if(!nclistpush(ncindex->list,obj))
	return 0;
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
   return 1;
}
#endif

/**
 * Remove ith object from the index;
 * Return 1 if ok, 0 otherwise.*/
int
ncindexremove(NCindex* index, NC_OBJ* obj)
{
   if(index == NULL) return 0;
   /* Overwrite entry in list with NULL */
   if(!nclistset(index->list,obj->reserved,NULL))
	return 0;
   return 1;
}

#if 0
/*
Rebuild the list map by rehashing all entries
using their current, possibly changed id and name;
also recompute their hashkey.
*/
/* Return 1 if ok, 0 otherwise.*/
static int
ncindexrebuild(NCindex* index)
{ 
    /* Nothing to do */
    return 1;    
}
#endif

/*
Rename an object and ensure that the
list and namemap are properly updated
*/
int
ncindexrename(NCindex* index, NC_OBJ* hdr, const char* newname)
{
   char* oldname = NULL;
   if(!hdr) return 0;
   oldname = hdr->name;
   if(oldname == NULL) return 0;
   if (!(hdr->name = strdup(newname)))
      return 0;
   free(oldname);
   return 1;
}

/* Free a list map */
int
ncindexfree(NCindex* index)
{
    if(index == NULL) return 1;
    nclistfree(index->list);
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
    return index;
}

int
ncindexverify(NCindex* lm, int dump)
{
    size_t i;
    NClist* l = lm->list;
    int nerrs = 0;

    if(lm == NULL) {
	fprintf(stderr,"index: <empty>\n");
	return 1;
    }  
    if(dump) {
	fprintf(stderr,"-------------------------\n");
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
    fflush(stderr);
    return (nerrs > 0 ? 0: 1);
}

void
printindexmap(NCindex* lm)
{
    return;
}
