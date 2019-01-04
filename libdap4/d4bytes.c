/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "d4util.h"
#include "d4bytes.h"

#define DEFAULTMINALLOC 8

D4bytes*
d4bytesnew(void)
{
    D4bytes* d4m = (D4bytes*)malloc(sizeof(D4bytes));
    if(d4m == NULL) return NULL;
    d4m->alloc=0;
    d4m->used=0;
    d4m->memory=NULL;
    return d4m;
}

void
d4bytesfree(D4bytes* d4m)
{
    if(d4m == NULL) return;
    if(d4m->memory != NULL) free(d4m->memory);
    free(d4m);
}

void*
d4bytesalloc(D4bytes* d4m, size_t sz)
{
    void* allocation;
    size_t newalloc;
    if(sz == 0 || d4m == NULL) return NULL;
    newalloc = d4m->alloc;
    if(newalloc == 0) newalloc = DEFAULTMINALLOC;
    for(;;) {
        size_t avail = newalloc - d4m->used;
        if(avail >= sz) { /* rebuild */
	    void* newmem = realloc(d4m->memory,newalloc);
	    d4m->memory = newmem;
	    d4m->alloc = newalloc;
	    break;
        }
	newalloc = (2*newalloc); /* keep doubling */
    }
    allocation = (void*)(((char*)d4m->memory)+d4m->used);
    d4m->used += sz;
    return allocation;
}


void*
d4byteszero(D4bytes* d4m, size_t sz)
{
    void* mem = d4bytesalloc(d4m,sz);
    memset(mem,0,sz);
    return mem;
}

D4bytes*
d4bytesconcat(D4bytes* dst, D4bytes* src)
{
    if(dst == NULL && src != NULL && src->used > 0) {
	void* target = d4bytesalloc(dst,src->used);
	d4memmove(target,src->memory,src->used);
    }
    return dst;
}
