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
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <assert.h>

#include "netcdf.h"
#include "ncexhash.h"
#include "ncxcache.h"

#define DEBUG 0


static struct rusage ru;
static clockid_t clk_id = CLOCK_MONOTONIC;
static struct timespec xinserttime[2];
static long inserttime[2];
static struct timespec xreadtime[2];
static long readtime[2];
#if 0
static struct timespec xremtime[2];
static long readtime[2];
static long remtime[2];
#endif

#define CHECK(expr) check((expr),__LINE__)
void check(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: (%d)%s\n",line,stat,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}

#if 0
static void
reporttime(unsigned nelems, long* times, const char* tag)
{
    NC_UNUSED(nelems);
    NC_UNUSED(times);
    NC_UNUSED(tag);
#if 0
    double delta;
    double deltasec;
    delta = (double)(times[1] - times[0]);
    deltasec = delta / 1000000.0;
    fprintf(stderr,"\t%s:\t%5.1lf sec",tag,deltasec);
    fprintf(stderr," avg=%5.1lf usec\n",delta/nelems);
#endif
}
#endif

static void
xreporttime(unsigned nelems, struct timespec* times, const char* tag)
{
    double delta;
    double deltasec;
    long long nsec[2];
    
    nsec[0] = times[0].tv_nsec+(1000000000 * times[0].tv_sec);
    nsec[1] = times[1].tv_nsec+(1000000000 * times[1].tv_sec);

    delta = (double)(nsec[1] - nsec[0]);
    deltasec = delta / 1000000000.0;
    fprintf(stderr,"\t%s:\t%8.6lf sec",tag,deltasec);
    fprintf(stderr," avg=%5.2lf nsec\n",delta/nelems);
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
	    do {rnd = random() % 127;} while(rnd < ' ');
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

    for(np=N;*np;np++) {    
	int ns,i;

	ns = *np;
        generatestrings(ns,DEFAULTSEED);

	fprintf(stderr,"insert:\n");

        if((stat =  ncxcachenew(2,&cache))) goto done;

        getrusage(RUSAGE_SELF, &ru);
        inserttime[0] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
        clock_gettime(clk_id,&xinserttime[0]);

	for(i=0;i<ns;i++) {
	    hkey = ncexhashkey(strings[i].string,strlen(strings[i].string));
	    if((stat=ncxcacheinsert(cache,hkey,&strings[i]))) goto done;
	}
	assert(ncxcachecount(cache) == ns);

        clock_gettime(clk_id,&xinserttime[1]);
        getrusage(RUSAGE_SELF, &ru);
        inserttime[1] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);

#if DEBUG > 0
	ncxcacheprint(cache);
#endif

        xreporttime(ns, xinserttime, "insert");

	/* Try to touch and extract all the entries */

	fprintf(stderr,"read:\n");

        getrusage(RUSAGE_SELF, &ru);
        readtime[0] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
        clock_gettime(clk_id,&xreadtime[0]);

	for(i=0;i<ns;i++) {
	    void* top = NULL;
	    hkey = ncexhashkey(strings[i].string,strlen(strings[i].string));
	    if((stat=ncxcachelookup(cache,hkey,&content))) goto done;
    	    if((stat=ncxcachetouch(cache,hkey))) goto done;
	    top = ncxcachefirst(cache);
	    if(top != content) {stat = NC_EINTERNAL; goto done;}
	}

        clock_gettime(clk_id,&xreadtime[1]);
        getrusage(RUSAGE_SELF, &ru);
        readtime[1] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);

#if DEBUG > 0
	ncxcacheprint(cache);
#endif

        xreporttime(ns, xreadtime, "read");

	for(i=0;i<ns;i++) {
	    void* top = NULL;
	    hkey = ncexhashkey(strings[i].string,strlen(strings[i].string));
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
