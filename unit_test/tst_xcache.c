/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the NCxcache data structure
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "netcdf.h"
#include "ncexhash.h"
#include "ncxcache.h"

#include "timer_utils.h"

#ifdef _WIN32
#define srandom srand
#define random (long)rand
#endif

#define DEBUG 0

/* Approximate average times; if we get out of this range, then
   something is drastically wrong */
static const struct TimeRange insertrange = {0,50000};
static const struct TimeRange readrange = {0,5000};

static Nanotime inserttime[2];
static Nanotime readtime[2];

#define CHECK(expr) check((expr),__LINE__)
void check(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: (%d)%s\n",line,stat,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}

/*
Test set 1:
- insert string with obj
- lookup by various methods
- remove
- verify remove
*/


#define MAXSTRLEN 25
#define DEFAULTSEED 1

typedef struct NC_OBJ {
    int sort;
    char* name;
    size_t id;
} NC_OBJ;

typedef struct NCXSTR {
    void* next;
    void* prev;
    void* ptr;
    char* string;
} NCXSTR;

//static int N[] = {10, 100, 1000, 0};
static int N[] = {4,0};

/* Generate random ascii strings */
static NCXSTR* strings = NULL;

static void
generatestrings(int n, unsigned seed)
{
    int i,k;
    long rnd;
    int len;
    char* s = NULL;

    srandom(seed);
    strings = (NCXSTR*)calloc(sizeof(NCXSTR),(n+1));
    if(strings == NULL) abort();
    for(i=0;i<n;i++) {
        /* Generate one random string */
	if((s = (char*)malloc(1+MAXSTRLEN))==NULL) abort();
	rnd = random();
	len = rnd % MAXSTRLEN;
	/* generate the characters */
	for(k=0;k<len;k++) {
	    do {rnd = random() % 127;} while(rnd <= ' ');
	    assert(rnd > ' ' && rnd < 127);
	    s[k] = (char)rnd;
	}
	s[len] = '\0';
	strings[i].string = s;
#if DEBUG >= 3
	fprintf(stderr,"strings[%d] = |%s|\n",i,strings[i].string);
#endif
    }
}

static void
freestrings(void)
{
    if(strings) {
        NCXSTR* p = strings;
        for(;p->string;p++) {nullfree(p->string);}
        nullfree(strings);
        strings = NULL;
    }
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    NCxcache* cache = NULL;
    ncexhashkey_t hkey;
    int* np = NULL;
    void* content = NULL;

    NCT_inittimer();

    for(np=N;*np;np++) {    
	int ns,i;

	ns = *np;
        generatestrings(ns,DEFAULTSEED);

	fprintf(stderr,"insert:\n");

        if((stat =  ncxcachenew(2,&cache))) goto done;

	NCT_marktime(&inserttime[0]);

	for(i=0;i<ns;i++) {
	    hkey = ncexhashkey((unsigned char*)strings[i].string,strlen(strings[i].string));
	    if((stat=ncxcacheinsert(cache,hkey,&strings[i]))) goto done;
	}
	assert(ncxcachecount(cache) == ns);

	NCT_marktime(&inserttime[1]);

#if DEBUG > 0
	ncxcacheprint(cache);
#endif

        NCT_reporttime(ns, inserttime, insertrange, "insert");

	/* Try to touch and extract all the entries */

	fprintf(stderr,"read:\n");

	NCT_marktime(&readtime[0]);

	for(i=0;i<ns;i++) {
	    void* top = NULL;
	    hkey = ncexhashkey((unsigned char*)strings[i].string,strlen(strings[i].string));
	    if((stat=ncxcachelookup(cache,hkey,&content))) goto done;
    	    if((stat=ncxcachetouch(cache,hkey))) goto done;
	    top = ncxcachefirst(cache);
	    if(top != content) {stat = NC_EINTERNAL; goto done;}
	}

	NCT_marktime(&readtime[1]);

#if DEBUG > 0
	ncxcacheprint(cache);
#endif

	NCT_reporttime(ns, readtime, readrange, "read");

	for(i=0;i<ns;i++) {
	    void* top = NULL;
	    hkey = ncexhashkey((unsigned char*)strings[i].string,strlen(strings[i].string));
	    if((stat=ncxcachetouch(cache,hkey))) goto done;
	    top = ncxcachefirst(cache);
	    if(top != &strings[i])
	        fprintf(stderr,"touch failure: top=%p strings[%d]=%p\n",top,i,&strings[i]);
	}
	fprintf(stderr,"touch: passed\n");

	freestrings();
	ncxcachefree(cache);
    }
done:
    if(stat) fprintf(stderr,"***fail: %s\n",nc_strerror(stat));
    return 0;
}
