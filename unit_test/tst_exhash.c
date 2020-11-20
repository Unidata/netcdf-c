/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the Extendible Hash Implementation of ncexhash
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
#include "nccrc.h"

#define LEAFN 16

#define HMODE 3

#undef VERBOSE

#if NCEXHASHKEYBITS == 64
#define CRC NC_crc64
#else
#define CRC NC_crc32
#endif

static unsigned N[] = {1000, 10000, 100000, 1000000, 0};

#define CHECK(expr) check((expr),__LINE__)
void check(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: (%d)%s\n",line,stat,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}

static ncexhashkey_t
hkeyfor(unsigned key)
{
    ncexhashkey_t hashkey = 0;
    int i;

    switch (HMODE) {
    case 1:
        hashkey = ncexhashkey((char*)&key,sizeof(key));
	break;
    case 2:
        for(i=0;i<NCEXHASHKEYBITS;i++) {
	    hashkey |= (key & 0x1) << (31-i);
	    key = key >> 1;
        }
        break;
    case 3: /* Convert key to a random number using crc */
	hashkey = CRC(0,(void*)&key,sizeof(key));
	break;
    default:
	abort();
    }
    return hashkey;
}

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

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    NCexhashmap* map = NULL;
    unsigned key;
    struct rusage ru;
    clockid_t clk_id = CLOCK_MONOTONIC;
    struct timespec xinserttime[2];
    struct timespec xreadtime[2];
    struct timespec xremtime[2];
    long  inserttime[2], readtime[2], remtime[2]; /* elapsed time in microseconds */
    unsigned* np;
    uintptr_t data;
    ncexhashkey_t hashkey;

    fprintf(stderr,"insert:\n");

#ifdef VERBOSE
    {
    long microcvt, seccvt;
    struct timespec res;
    if(clock_getres(clk_id, &res) < 0)
	abort();
    fprintf(stderr,"xxx: tv_sec=%lld tv_nsec=%ld\n",(long long)res.tv_sec,res.tv_nsec);
    microcvt = res.tv_nsec;
    seccvt = microcvt * 1000000;
    fprintf(stderr,"xxx: seccvt=%lld microcvt=%lld\n",
		(long long)seccvt,(long long)microcvt);
    }
#endif

    for(np=N;*np;np++) {

    getrusage(RUSAGE_SELF, &ru);
    inserttime[0] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
	     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
    clock_gettime(clk_id,&xinserttime[0]);

    map=ncexhashnew(LEAFN);
    if(map == NULL) CHECK(NC_EINVAL);
#ifdef VERBOSE
    fprintf(stderr,"new:\n"); ncexhashprint(map);    
#endif
    for(key=0;key<*np;key++) {
	ncexhashkey_t hashkey = hkeyfor(key);
#ifdef VERBOSE
        fprintf(stderr,"insert[%08llx|%s->%u]:\n",hashkey,ncexbinstr(hashkey,NCEXHASHKEYBITS),key);
#endif
	CHECK(ncexhashput(map,hashkey,(uintptr_t)key));
    }
#ifdef VERBOSE
    fprintf(stderr,"insert.after:");ncexhashprint(map);    
#endif

    clock_gettime(clk_id,&xinserttime[1]);
    getrusage(RUSAGE_SELF, &ru);
    inserttime[1] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
	     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
    fprintf(stderr,"read:\n");

    getrusage(RUSAGE_SELF, &ru);
    readtime[0] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
	     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
    clock_gettime(clk_id,&xreadtime[0]);

    for(key=0;key<*np;key++) {
	uintptr_t data = 0;
	ncexhashkey_t hashkey = hkeyfor(key);
	CHECK(ncexhashget(map,hashkey,&data));
#ifdef VERBOSE
        fprintf(stderr,"read[%08llx|%s->%u]:\n",hashkey,ncexbinstr(hashkey,NCEXHASHKEYBITS),(unsigned)data);
#endif
	if(data != key) fprintf(stderr,"\tMISMATCH\n");
    }

    clock_gettime(clk_id,&xreadtime[1]);
    getrusage(RUSAGE_SELF, &ru);
    readtime[1] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
	     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);

    fprintf(stderr,"statistics:\n"); ncexhashprintstats(map);    
    fprintf(stderr,"times: N=%u\n",*np);

    reporttime(*np, inserttime, "insert");
    reporttime(*np, readtime, "read");

    xreporttime(*np, xinserttime, "insert");
    xreporttime(*np, xreadtime, "read");

    /* Test iterator */
    {
	unsigned char* found = NULL;
	int nmatches = 0;
	fprintf(stderr,"iterating: %u\n",*np);
	if((found = (unsigned char*)calloc(1,*np))==NULL) CHECK(NC_ENOMEM);
        for(;;) {
	    if((stat = ncexhashiterate(map, &hashkey, &data)) == NC_EINVAL) CHECK(stat);
	    if(stat == NC_ERANGE) break;
	    assert(data >= 0 && data < *np);
	    if(found[data] != 0)
	        fprintf(stderr,"iterator duplicate: data=%lu\n",(unsigned long)data);
	    found[data] = 1;
	    nmatches++;
        }
	for(key=0;key<*np;key++) {
	    if(found[key] == 0) fprintf(stderr,"iterator missing: %u\n",key);
	}
	fprintf(stderr,"iterating: |keys|=%u  |matches|=%u\n",*np,nmatches);
	if(found) free(found);
    }
    
    /* Test setdata */
    {
	fprintf(stderr,"setdata: %u\n",*np);

        for(key=0;key<*np;key++) {
	    ncexhashkey_t hashkey = hkeyfor(key);
	    uintptr_t newdata = (uintptr_t)(key+(*np));
	    stat = ncexhashsetdata(map, hashkey, newdata, &data);
	    assert(stat == NC_NOERR);
	    assert(data == key);
        }
        for(;;) {
	    ncexhashkey_t rehash;
	    if((stat = ncexhashiterate(map, &hashkey, &data)) == NC_EINVAL) CHECK(stat);
	    if(stat == NC_ERANGE) break;
	    rehash = hkeyfor(data - (*np));
	    assert(hashkey == rehash);
        }
    }

    {
	int leaflen = 0;
	int depth = 0;
	int nactive = 0;
	int uid = 0;
	int walking = 0;
        CHECK(ncexhashinqmap(map,&leaflen,&depth,&nactive,&uid,&walking));
	fprintf(stderr,"map parameters: leaflen=%d depth=%d nactive=%d uid=%d walking=%d\n",
		leaflen,depth,nactive,uid,walking);
    }

    /* Test removal */
    {
	int nactive = 0;

	fprintf(stderr,"removing: %u\n",*np);

        getrusage(RUSAGE_SELF, &ru);
        remtime[0] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
	     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
        clock_gettime(clk_id,&xremtime[0]);

        for(key=0;key<*np;key++) {
	    ncexhashkey_t hashkey = hkeyfor(key);
	    switch (stat = ncexhashremove(map, hashkey, &data)) {
	    case NC_NOERR:
	        break;
	    case NC_ENOTFOUND:
		fprintf(stderr,"remove: missing: key=%u hashkey=%llu\n",key,hashkey);
	        break;
	    default: CHECK(stat);
	    }
        }
        CHECK(ncexhashinqmap(map,NULL,NULL,&nactive,NULL,NULL));
	fprintf(stderr,"removal: final nactive=%d\n",nactive);

        clock_gettime(clk_id,&xremtime[1]);
 	getrusage(RUSAGE_SELF, &ru);
        remtime[1] = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);
        reporttime(*np, remtime, "removal");
        xreporttime(*np, xremtime, "removal");
    }
    
    ncexhashmapfree(map);

    }


    return 0;
}
