/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nclist.h"

#if defined(_WIN32) && !defined(__MINGW32__)
#define strcasecmp _stricmp
#endif

int nclistisnull(void* e) {return e == NULL;}

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 16
#define ALLOCINCR 16

NClist* nclistnew(void)
{
  NClist* l;
/*
  if(!ncinitialized) {
    memset((void*)&ncDATANULL,0,sizeof(void*));
    ncinitialized = 1;
  }
*/
  l = (NClist*)malloc(sizeof(NClist));
  if(l) {
    l->alloc=0;
    l->length=0;
    l->content=NULL;
  }
  return l;
}

int
nclistfree(NClist* l)
{
  if(l) {
    l->alloc = 0;
    if(l->content != NULL) {free(l->content); l->content = NULL;}
    free(l);
  }
  return TRUE;
}

/*
Free a list and its contents
*/
int
nclistfreeall(NClist* l)
{
    nclistclearall(l);
    return nclistfree(l);
}

/*
Free the contents of a list
*/
int
nclistclearall(NClist* l)
{
  size_t i,len;
  if(l == NULL) return TRUE;
  len = l->length;
  for(i=0;i<len;i++) {
      void* value = l->content[i];
      if(value != NULL) free(value);
  }
  nclistsetlength(l,0);
  return TRUE;
}

int
nclistsetalloc(NClist* l, size_t sz)
{
  void** newcontent = NULL;
  if(l == NULL) return FALSE;
  if(sz <= 0) {sz = (l->length?2*l->length:DEFAULTALLOC);}
  if(l->alloc >= sz) {return TRUE;}
  newcontent=(void**)calloc(sz,sizeof(void*));
  if(newcontent != NULL && l->alloc > 0 && l->length > 0 && l->content != NULL) {
    memcpy((void*)newcontent,(void*)l->content,sizeof(void*)*l->length);
  }
  if(l->content != NULL) free(l->content);
  l->content=newcontent;
  l->alloc=sz;
  return TRUE;
}

int
nclistsetlength(NClist* l, size_t newlen)
{
  if(l == NULL) return FALSE;
  if(newlen > l->alloc && !nclistsetalloc(l,newlen)) return FALSE;
  if(newlen > l->length) {
      /* clear any extension */
      memset(&l->content[l->length],0,(newlen - l->length)*sizeof(void*));
  }
  l->length = newlen;
  return TRUE;
}

void*
nclistget(const NClist* l, size_t index)
{
  if(l == NULL || l->length == 0) return NULL;
  if(index >= l->length) return NULL;
  return l->content[index];
}

/* Insert at position i of l; will overwrite previous value;
   guarantees alloc and length
*/
int
nclistset(NClist* l, size_t index, void* elem)
{
  if(l == NULL) return FALSE;
  if(!nclistsetalloc(l,index+1)) return FALSE;
  if(index >= l->length) {
      if(!nclistsetlength(l,index+1)) return FALSE;
  }
  l->content[index] = elem;
  return TRUE;
}

/* Insert at position i of l; will push up elements i..|seq|. */
int
nclistinsert(NClist* l, size_t index, void* elem)
{
  long i; /* do not make unsigned */
  if(l == NULL) return FALSE;
  if(index > l->length) return FALSE;
  nclistsetalloc(l,0);
  for(i=(long)l->length;i>index;i--) l->content[i] = l->content[i-1];
  l->content[index] = elem;
  l->length++;
  return TRUE;
}

int
nclistpush(NClist* l, const void* elem)
{
  if(l == NULL) return FALSE;
  if(l->length >= l->alloc) nclistsetalloc(l,0);
  l->content[l->length] = (void*)elem;
  l->length++;
  return TRUE;
}

void*
nclistpop(NClist* l)
{
  if(l == NULL || l->length == 0) return NULL;
  l->length--;
  return l->content[l->length];
}

void*
nclisttop(NClist* l)
{
  if(l == NULL || l->length == 0) return NULL;
  return l->content[l->length - 1];
}

void*
nclistremove(NClist* l, size_t i)
{
  size_t len;
  void* elem;
  if(l == NULL || (len=l->length) == 0) return NULL;
  if(i >= len) return NULL;
  elem = l->content[i];
  for(i+=1;i<len;i++) l->content[i-1] = l->content[i];
  l->length--;
  return elem;
}

/* Match on == */
int
nclistcontains(NClist* l, void* elem)
{
    size_t i;
    for(i=0;i<nclistlength(l);i++) {
	if(elem == nclistget(l,i)) return 1;
    }
    return 0;
}

/* Match on str(case)cmp */
int
nclistmatch(NClist* l, const char* elem, int casesensitive)
{
    size_t i;
    for(i=0;i<nclistlength(l);i++) {
	const char* candidate = (const char*)nclistget(l,i);
	int match;
	if(casesensitive)
	    match = strcmp(elem,candidate);
	else
	    match = strcasecmp(elem,candidate);
	if(match == 0) return 1;
    }
    return 0;
}

/* Remove element by value; only removes first encountered */
int
nclistelemremove(NClist* l, void* elem)
{
  size_t len;
  size_t i;
  int found = 0;
  if(l == NULL || (len=l->length) == 0) return 0;
  for(i=0;i<nclistlength(l);i++) {
    void* candidate = l->content[i];
    if(elem == candidate) {
      for(i+=1;i<len;i++) l->content[i-1] = l->content[i];
      l->length--;
      found = 1;
      break;
    }
  }
  return found;
}

/* Extends nclist to include a unique operator
   which remove duplicate values; NULL values removed
   return value is always 1.
*/

int
nclistunique(NClist* l)
{
    size_t i,j,k,len;
    void** content;
    if(l == NULL || l->length == 0) return 1;
    len = l->length;
    content = l->content;
    for(i=0;i<len;i++) {
        for(j=i+1;j<len;j++) {
	    if(content[i] == content[j]) {
		/* compress out jth element */
                for(k=j+1;k<len;k++) content[k-1] = content[k];
		len--;
	    }
	}
    }
    l->length = len;
    return 1;
}

/* Duplicate a list and if deep is true, assume the contents
   are char** and duplicate those also */
NClist*
nclistclone(const NClist* l, int deep)
{
    NClist* clone = NULL;
    if(l == NULL) goto done;
    clone = nclistnew();
    nclistsetalloc(clone,l->length+1); /* make room for final null */
    if(!deep) {
        nclistsetlength(clone,l->length);
        memcpy((void*)clone->content,(void*)l->content,sizeof(void*)*l->length);
    } else { /*deep*/
	int i;
	for(i=0;i<nclistlength(l);i++) {
	    char* dups = strdup(nclistget(l,i));
	    if(dups == NULL) {nclistfreeall(clone); clone = NULL; goto done;}
	    nclistpush(clone,dups);	    
	}
    }
    clone->content[l->length] = (void*)0;
done:
    return clone;
}


void*
nclistextract(NClist* l)
{
    void* result = l->content;
    l->alloc = 0;
    l->length = 0;
    l->content = NULL;
    return result;
}

/* Extends nclist to include a NULL that is not included
   in list length.
   return value is always 1.
*/
int
nclistnull(NClist* l)
{
    if(l == NULL || l->length == 0) return 1;
    nclistpush(l,NULL);
    nclistsetlength(l,l->length-1);
    return 1;
}

