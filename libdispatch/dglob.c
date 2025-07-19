/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See COPYRIGHT for license information.
*/

#include "config.h"
#include <stddef.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <regex.h> /* temporary to avoid conflicts */

#include "nclist.h"
#include "ncrc.h"

/**
This file provides support for
a limited form of glob pattern.
Its purpose to allow the url part
of a .ncrc file to be a pattern.
It specifically is applied to the URL hostname
using a '.' separator and to the URL path using
a '/' separator.

A pattern is of this syntax:
pattern:
	  empty
	| element
	| element SEP pattern
	;
element:
	  NAME
	| '*'
	| '**'
	;

NAME: <any sequence of non-separator characters, including UTF8 characters>
SEP: <one of '.' | '/' -- only one of the two separators is allowed within a pattern>

A '*' pattern matches any name.
A '**' pattern matches any sequence of names separated by SEP.

As a context sensitive constraint, it is illegal for a pattern to contain '**.**'.

*/

typede enum STEP { ST_UNDEF=0, ST_NAME=1, ST_STAR=2, ST_STARSTAR=3};

/* Define a single pattern element */
typedef struct NCSTEP {
    enum STEP sort;
    /* Avoid a union since only names have associated values */
    char* name; /* => sort == NCST_NAME */
} NCSTEP;

typedef struct NCSEGMENT {
    char* text;
    NCGLOBMATCH pos;
} NCSEGMENT;

/* Define the matcher */
struct NCMATCHER {
{
    NClist* pattern;	 /* Compiled pattern */
    NClist* matches;	 /* NClist<NCMATCH*> List of matched substrings of the subject */
    int sep;		 /* The separator character for this matcher */
};

/* Free up a sequence of steps */
static void
freesteps(NClist* steps)
{
    size_t i;
    for(i=0;i<nclistlength(steps);i++) {
	NCSTEP* step = (NCSTEP*)nclistget(steps,i);
	nullfree(step->name);
    }
    nclistfree(steps);
}

/* Free up an NCMATCHER */
void
NC_rcfreematcher(NCMATCHER* matcher)
{
    size_t i;
    if(matcher == NULL) return;
    freesteps(matcher->pattern);
    freematches(matcher->matches);
    free(matcher);
}

void
freesegment(NCSEGMENT* seg)
{
    nullfree(seg->text);
    nullfree(seg);
}

/**
Parse a string representation of a glob pattern into a sequence of NCSTEP objects.
@param patstr parse this glob pattern and store in the pattern argument.
@param sep the separator character to be used.
@param pattern compiled version of patstr.
@return NC_NOERR or NC_EXXX.

Note that 
*/
static int
rcglobcompile(const char* patstr0, int sep, NCMATCHER* matcher)
{
    int stat = NC_NOERR;
    NCSTEP* step = NULL;
    char* patstr = NULL;
    char* p = NULL;

    if((patstr = strdup(patstr0))==NULL) {stat = NC_ENOMEM; goto done;}
    for(p=patstr;;) {
	char* endp;
	if((step=(NCSTEP*)calloc(1,sizeof(NCSTEP)))==NULL) {stat = NC_ENOMEM; goto done;}
	endp = strchr(p,matcher->sep);
	if(endp == NULL) {endp = p + strlen(p)}
	*endp = '\0'; /* overwrite separator or nul terminator */
	if(strcmp(p,"*") step->sort = ST_STAR;
	else if(strcmp(p,"**") step->sort = ST_STARSTAR;
	else {step->sort = ST_NAME; step->name = strdup(p);}
	nclistpush(matcher->pattern,step);
    }

    /* Check for pattern errors */
    /* cannot have **.** */
    if(nclistlength(matcher->pattern) > 1) {
	for(i=0;i<nclistlength(matcher->pattern)-1;i++) {
	    NCSTEP* step = (NCSTEP*)nclistget(matcher->pattern,i);
	    NCSTEP* next = (NCSTEP*)nclistget(matcher->pattern,i+1);
	    if(step->sort == ST_STARSTAR && next->sort == ST_STARSTAR) {stat = NC_EINVAL; goto done;}
	}
    }
done:
    return stat;
}

/**
Create a matcher for a given pattern.
@param patstr parse this glob pattern and store in the pattern argument.
@param sep the separator character to be used.
@param matcherp the created matcher
@return NC_NOERR or NC_EXXX.
*/
 int
NC_rcnewmatcher(const char* patstr, int sep, NCMATCHER** matcherp)
{
    int stat = NC_NOERR;
    NCMATCHER* matcher = NULL;

    if(strlen(patstr) == NULL) return NC_EINVAL;
    if(sep != '.' && sep != '/') return NC_EINVAL;
    if(pattern == NULL) return  NC_EINVAL;

    if((matcher=(NCMATCHER*)calloc(1,sizeof(NCMATCHER)))==NULL) {stat = NC_ENOMEM; goto done;}
    if((matcher->pattern = nclistnew())==NULL) {stat = NC_ENOMEM; goto done;}
    if((matcher->matches = nclistnew())==NULL) {stat = NC_ENOMEM; goto done;}
    matcher->sep = sep;

    /* Parse the pattern */
    if((stat = rcglobcompile(patstr, matcher))) goto done;

done:
    return stat;
}

#if 0
/* Parse the subject */
static int
parsesubject(const char* subject0, int sep, NClist* segments)
{
    int stat = NC_NOERR;
    NCGLOBMATCH* segment
    char* pos;
    char* subject;

    if((subject=strdup(subject0))==NULL) {stat = NC_ENOMEM; goto done;}
    for(;;) {
	char* endpos;
	endpos = strchr(pos,sep);
	if(endpos == NULL) endpos = pos + strlen(pos);
	*endpos = '\0';
	nclistpush(segments,strdup(pos));
	pos = endpos+1;
    }
done:
    return stat;
}
#endif

/* Recursive helper for matching ** pattern.
Typical match with backtrack.
@param matcher
@param subject
@param startpos
@param endposp
@return 0 if !matches, 1 if matches, NC_EXXX if an error
*/
static int
rcstarstarR(NCMATCHER* matcher, const char* subject, size_t startpos, size_t* endposp)
{
    int stat = NC_NOERR;
    size_t starpos, endpos;
    
    *endposp = 0;

    switch (step->sort) {
    case ST_NAME:
	if(strncmp(subject[startpos],step->name,strlen(step->name))==0) {
	    *endpos = startpos + strlen(step->name);
	    return 1;
	} else
	    return 0;
	break;
    case ST_STAR:
	*endpos = startpos + strlen(step->name);
	return 1;
    case ST_STARSTAR: /* Must recurse until fails */
	stat = rcstarstarR(matcher,startpos,&endpos);
	if(stat == 0) {
	return 1;
    default: return NC_EINTERNAL;
    }
done:
    nclistfreeall(segments);
    return stat;
}

/**
See if a string matches a specified limited glob pattern.
@param s string to be matched to the glob pattern
@param pattern to be matched
*/
int
NC_rcglobmatch(NCMATCHER* matcher, const char* subject0)
{
    int stat = NC_NOERR;
    int passfail;
    char* subject = NULL;
    size_t i;
    NCGLOBMATCH* match = NULL;
    size_t startpos,endpos;
    
    if(matcher == NULL) {stat = NC_EINVAL; goto done;}
    if(subject0) == NULL) {stat = NC_EINVAL; goto done;}
    if((subject = strdup(subject0))==NULL) {stat = NC_ENOMEM; goto done;

    for(startpos=0,i=0;i<nclistlength(matcher->pattern);i++) {
	NCSTEP* step = (NCSTEP*)nclistget(matcher->pattern,i);
	switch (step->sort) {
	case ST_NAME:
	    if(strncmp(subject[startpos],step->name,strlen(step->name))==0) {
	        *endpos = startpos + strlen(step->name);
	    } else {
	        passfail = 0;
		goto stop;
	    }
	    break;
	case ST_STAR:
	    *endpos = startpos + strlen(step->name);
	    break;
        case ST_STARSTAR: /* Must recurse until fails */
	    if((stat = rcstarstarmatch(matcher, step, const char* subject, startpos, &endpos))) goto done;
	    break;
    }
done:
    return stat;
}

NClist*
NC_rcglobmatches(NCMATCHER* matcher)
{
    NClist* matches = NULL;
    if(matcher == NULL) goto done;
    return matcher->matches;
}

