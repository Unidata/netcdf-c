/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

#include <stdlib.h>
#include <string.h>
#include "ncxml.h"
#include "ezxml.h"

#ifndef nulldup
#define nulldup(s) ((s)?strdup(s):NULL)
#endif

static int ncxml_initialized = 0;

void
ncxml_initialize(void)
{
    ncxml_initialized = 1;
}

void
ncxml_finalize(void)
{
    ncxml_initialized = 0;
}

ncxml_doc_t
ncxml_parse(char* contents, size_t len)
{
    return (ncxml_t)ezxml_parse_str(contents,len);
}

void
ncxml_free(ncxml_doc_t doc0)
{
    ezxml_t doc = (ezxml_t)doc0;
    ezxml_free(doc);
}

ncxml_t
ncxml_root(ncxml_doc_t doc0)
{
    ezxml_t doc = (ezxml_t)doc0;
    return (ncxml_t)doc;
}

const char*
ncxml_name(ncxml_t xml0)
{
    ezxml_t xml = (ezxml_t)xml0;    
    return (xml?xml->name:NULL);
}

char*
ncxml_attr(ncxml_t xml0, const char* key)
{
    ezxml_t xml = (ezxml_t)xml0;
    return nulldup(ezxml_attr(xml,key));
}

ncxml_t
ncxml_child(ncxml_t xml0, const char* name)
{
    ezxml_t xml = (ezxml_t)xml0;
    return (ncxml_t)ezxml_child(xml,name);
}

ncxml_t
ncxml_next(ncxml_t xml0, const char* name)
{
    ezxml_t xml = (ezxml_t)xml0;
    (void)name; /* unused */
    return (ncxml_t)ezxml_next(xml);
}

char*
ncxml_text(ncxml_t xml0)
{
    ezxml_t xml = (ezxml_t)xml0;
    return (xml?strdup(xml->txt):strdup(""));
}

/* Nameless versions of child and next */
ncxml_t
ncxml_child_first(ncxml_t xml0)
{
    ezxml_t xml = (ezxml_t)xml0;
    return (xml?xml->child:NULL);
}

ncxml_t
ncxml_child_next(ncxml_t xml0)
{
    ezxml_t xml = (ezxml_t)xml0;
    return (xml?xml->ordered:NULL);
}

int
ncxml_attr_pairs(ncxml_t xml0, char*** pairsp)
{
    char** pairs = NULL;
    ezxml_t xml = (ezxml_t)xml0;
    if(xml) {
        /* First count */
        int i,count = 0;
        const char** p = (const char**)xml->attr;
	for(count=0;*p;p+=2)
	    count++; /* pair count */
        pairs = (char**)malloc(sizeof(char*)*((2*count)+1));
	if(pairs == NULL) return 0;
        p = (const char**)xml->attr;
	for(i=0;*p;p+=2,i+=2) {
	    pairs[i] = strdup(p[0]);
    	    pairs[i+1] = strdup(p[1]);
	}
	pairs[2*count] = NULL;
	if(pairsp) *pairsp = pairs;
	return 1;
    }
    return 0;
}
