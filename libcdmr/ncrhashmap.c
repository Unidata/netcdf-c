/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ncrhashmap.h"

static ncrelem ncrDATANULL = (ncrelem)0;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 31

NCRhashmap* ncrhashnew(void) {return ncrhashnew0(DEFAULTALLOC);}

NCRhashmap* ncrhashnew0(int alloc)
{
  NCRhashmap* hm;
  hm = (NCRhashmap*)malloc(sizeof(NCRhashmap));
  if(!hm) return NULL;
  hm->alloc = alloc;
  hm->table = (NClist**)malloc(hm->alloc*sizeof(NClist*));
  if(!hm->table) {free(hm); return NULL;}
  memset((void*)hm->table,0,hm->alloc*sizeof(NClist*));
  return hm;
}

int
ncrhashfree(NCRhashmap* hm)
{
  if(hm) {
    int i;
    for(i=0;i<hm->alloc;i++) {
	if(hm->table[i] != NULL) ncrlistfree(hm->table[i]);
    }
    free(hm->table);
    free(hm);
  }
  return TRUE;
}

/* Insert a <ncrhashid,ncrelem> pair into the table*/
/* Fail if already there*/
int
ncrhashinsert(NCRhashmap* hm, ncrhashid hash, ncrelem value)
{
    int i,offset,len;
    NCRlist* seq;
    ncrelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) {seq = ncrlistnew(); hm->table[offset] = seq;}
    len = ncrlistlength(seq);
    list = ncrlistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) return FALSE;
    }    
    ncrlistpush(seq,(ncrelem)hash);
    ncrlistpush(seq,value);
    hm->size++;
    return TRUE;
}

/* Insert a <ncrhashid,ncrelem> pair into the table*/
/* Overwrite if already there*/
int
ncrhashreplace(NCRhashmap* hm, ncrhashid hash, ncrelem value)
{
    int i,offset,len;
    NCRlist* seq;
    ncrelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) {seq = ncrlistnew(); hm->table[offset] = seq;}
    len = ncrlistlength(seq);
    list = ncrlistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) {list[1] = value; return TRUE;}
    }    
    ncrlistpush(seq,(ncrelem)hash);
    ncrlistpush(seq,value);
    hm->size++;
    return TRUE;
}

/* remove a ncrhashid*/
/* return TRUE if found, false otherwise*/
int
ncrhashremove(NCRhashmap* hm, ncrhashid hash)
{
    int i,offset,len;
    NCRlist* seq;
    ncrelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) return TRUE;
    len = ncrlistlength(seq);
    list = ncrlistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) {
	    ncrlistremove(seq,i+1);
	    ncrlistremove(seq,i);
	    hm->size--;
	    if(ncrlistlength(seq) == 0) {ncrlistfree(seq); hm->table[offset] = NULL;}
	    return TRUE;
	}
    }    
    return FALSE;
}

/* lookup a ncrhashid; return DATANULL if not found*/
/* (use hashlookup if the possible values include 0)*/
ncrelem
ncrhashget(NCRhashmap* hm, ncrhashid hash)
{
    ncrelem value;
    if(!ncrhashlookup(hm,hash,&value)) return ncrDATANULL;
    return value;
}

int
ncrhashlookup(NCRhashmap* hm, ncrhashid hash, ncrelem* valuep)
{
    int i,offset,len;
    NCRlist* seq;
    ncrelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) return TRUE;
    len = ncrlistlength(seq);
    list = ncrlistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) {if(valuep) {*valuep = list[1]; return TRUE;}}
    }
    return FALSE;
}

/* Return the ith pair; order is completely arbitrary*/
/* Can be expensive*/
int
ncrhashith(NCRhashmap* hm, int index, ncrhashid* hashp, ncrelem* elemp)
{
    int i;
    if(hm == NULL) return FALSE;
    for(i=0;i<hm->alloc;i++) {
	NCRlist* seq = hm->table[i];
	int len = ncrlistlength(seq) / 2;
	if(len == 0) continue;
	if((index - len) < 0) {
	    if(hashp) *hashp = (ncrhashid)ncrlistget(seq,index*2);
	    if(elemp) *elemp = ncrlistget(seq,(index*2)+1);
	    return TRUE;
	}
	index -= len;
    }
    return FALSE;
}

/* Return all the keys; order is completely arbitrary*/
/* Can be expensive*/
int
ncrhashkeys(NCRhashmap* hm, ncrhashid** keylist)
{
    int i,j,index;
    ncrhashid* keys;
    if(hm == NULL) return FALSE;
    if(hm->size == 0) {
	keys = NULL;
    } else {
        keys = (ncrhashid*)malloc(sizeof(ncrhashid)*hm->size);
        for(index=0,i=0;i<hm->alloc;i++) {
 	    NCRlist* seq = hm->table[i];
	    for(j=0;j<ncrlistlength(seq);j+=2) {	
	        keys[index++] = (ncrhashid)ncrlistget(seq,j);
	    }
	}
    }
    if(keylist) {*keylist = keys;}
    return TRUE;
}

