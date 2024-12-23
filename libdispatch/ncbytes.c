/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ncbytes.h"
#include "nclog.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 1024
#define ALLOCINCR 1024

#define NCBYTESDEBUG 1

#ifdef NCBYTESDEBUG
#include <assert.h>
#endif

static int
ncbytesfail(void)
{
    fflush(stdout);
    fprintf(stderr,"NCbytes failure\n");
    fflush(stderr);
#ifdef NCBYTESDEBUG
    assert(!("NCbytes failure"));
#endif
    return FALSE;
}

NCbytes*
ncbytesnew(void)
{
  NCbytes* bb = (NCbytes*)malloc(sizeof(NCbytes));
  if(bb == NULL) return (ncbytesfail(),NULL);
  bb->alloc=0;
  bb->length=0;
  bb->content=NULL;
  bb->nonextendible = 0;
  return bb;
}

int
ncbytessetalloc(NCbytes* bb, unsigned long sz)
{
  char* newcontent;
  if(bb == NULL) return ncbytesfail();
  if(bb->nonextendible) return ncbytesfail();
//  if(sz == 0) {sz = (bb->alloc?2*bb->alloc:DEFAULTALLOC);} /* Default the size */
  if(sz == 0) return TRUE;
  /* We always guarantee that bb->content is not NULL */
  if(bb->alloc >= sz) {
    assert(bb->content != NULL);
    return TRUE;
  }
  newcontent=(char*)calloc(sz,sizeof(char));
  if(newcontent == NULL) ncbytesfail();
  if(bb->length > 0 && bb->content != NULL) {
    memcpy((void*)newcontent,(void*)bb->content,sizeof(char)*bb->length);
  }
  if(bb->content != NULL) {free(bb->content); bb->content = NULL;}
  bb->content = newcontent; newcontent = NULL;
  bb->alloc = sz;
  return TRUE;
}

EXTERNL void
ncbytesfree(NCbytes* bb)
{
  if(bb == NULL) return;
  if(!bb->nonextendible && bb->content != NULL) {
      free(bb->content); bb->content = NULL;
  }
  free(bb);
}

int
ncbytessetlength(NCbytes* bb, unsigned long sz)
{
  if(bb == NULL) return ncbytesfail();
  if(bb->length < sz) {
      if(sz > bb->alloc) {if(!ncbytessetalloc(bb,sz)) return ncbytesfail();}
  }
  bb->length = sz;
  return TRUE;
}

int
ncbytesfill(NCbytes* bb, char fill)
{
  unsigned long i;
  if(bb == NULL) return ncbytesfail();
  for(i=0;i<bb->length;i++) bb->content[i] = fill;
  return TRUE;
}

int
ncbytesget(NCbytes* bb, unsigned long index)
{
  if(bb == NULL) return -1;
  if(index >= bb->length) return -1;
  return bb->content[index];
}

int
ncbytesset(NCbytes* bb, unsigned long index, char elem)
{
  if(bb == NULL) return ncbytesfail();
  if(index >= bb->alloc) return ncbytesfail();
  bb->content[index] = elem;
  return TRUE;
}

int
ncbytesappend(NCbytes* bb, char elem)
{
  char s[2];
  if(bb == NULL) return ncbytesfail();
  s[0] = elem;
  s[1] = '\0';
  ncbytesappendn(bb,s,1);
  return TRUE;
}

/* This assumes s is a null terminated string; but nul not part of the length*/
int
ncbytescat(NCbytes* bb, const char* s)
{
  if(bb == NULL) return ncbytesfail();
  if(s == NULL) return 1;
  ncbytesappendn(bb,(void*)s,strlen(s));
  ncbytesnull(bb); /* include trailing null*/
  return 1;
}

int
ncbytesappendn(NCbytes* bb, const void* elem, unsigned long n)
{
  if(bb == NULL || elem == NULL) return ncbytesfail();
  ncbytessetalloc(bb,bb->length+n);
  if(n == 0) return TRUE;
  memcpy((void*)&bb->content[bb->length],(void*)elem,n);
  bb->length += n;
  return TRUE;
}

int
ncbytesprepend(NCbytes* bb, char elem)
{
  int i; /* do not make unsigned */
  if(bb == NULL) return ncbytesfail();
  if(bb->length >= bb->alloc) if(!ncbytessetalloc(bb,0)) return ncbytesfail();
  /* could we trust memcpy? instead */
  for(i=(int)bb->alloc;i>=1;i--) {bb->content[i]=bb->content[i-1];}
  bb->content[0] = elem;
  bb->length++;
  return TRUE;
}

char*
ncbytesdup(NCbytes* bb)
{
    char* result = (char*)malloc(bb->length+1);
    memcpy((void*)result,(const void*)bb->content,bb->length);
    result[bb->length] = '\0'; /* just in case it is a string*/
    return result;
}

char*
ncbytesextract(NCbytes* bb)
{
    char* result = bb->content;
    bb->alloc = 0;
    bb->length = 0;
    bb->content = NULL;
    return result;
}

int
ncbytessetcontents(NCbytes* bb, void* contents, unsigned long alloc)
{
    if(bb == NULL) return ncbytesfail();
    ncbytesclear(bb);
    if(!bb->nonextendible && bb->content != NULL) {free(bb->content); bb->content = NULL;}
    bb->content = (char*)contents;
    bb->length = alloc;
    bb->alloc = alloc;
    bb->nonextendible = 1;
    return 1;
}

/* Null terminate the byte string without extending its length */
int
ncbytesnull(NCbytes* bb)
{
    ncbytessetalloc(bb,bb->length+1); /* Force allocation of space and make room for nul term  */
    bb->content[bb->length] = '\0'; /* Leave length unchanged */
    return 1;
}

/* Remove char at position i */
int
ncbytesremove(NCbytes* bb, unsigned long pos)
{
    if(bb == NULL) return ncbytesfail();
    if(bb->length <= pos) return ncbytesfail();
    if(pos < (bb->length - 1)) {
	size_t copylen = (bb->length - pos) - 1;
        memmove(bb->content+pos,bb->content+pos+1,copylen);
    }
    bb->length--;
    return TRUE;
}

/* Insert n bytes into the buffer at position pos*/
int
ncbytesinsert(NCbytes* bb, size_t pos, size_t n, const char* s)
{
    if(bb == NULL) return ncbytesfail();
    if(pos > bb->length) ncbytesfail();
    if((bb->length + n) >= bb->alloc) if(!ncbytessetalloc(bb,bb->length+n+1)) return ncbytesfail();
    if(bb->length > 0)
        memmove(bb->content+pos+n,bb->content+pos,(bb->length - pos));
    memcpy(bb->content+pos,s,n);    
    bb->length += n;    
    bb->content[bb->length] = '\0';
    return TRUE;
}
