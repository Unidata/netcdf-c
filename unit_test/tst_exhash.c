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
#include <assert.h>
#include "netcdf.h"
#include "ncexhash.h"
#include "nccrc.h"

#include "timer_utils.h"

#define LEAFN 16

#define HMODE 3

#undef VERBOSE

#if NCEXHASHKEYBITS == 64
#define CRC NC_crc64
#else
#define CRC NC_crc32
#endif

/* Approximate average times; if we get out of this range, then
   something is drastically wrong */
static const struct TimeRange insertrange = {0, 50000};
static const struct TimeRange readrange = {0, 5000};
static const struct TimeRange remrange = {0,5000};

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
        hashkey = ncexhashkey((unsigned char*)&key,sizeof(key));
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

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    NCexhashmap* map = NULL;
    unsigned key;
    unsigned* np;
    uintptr_t data;
    ncexhashkey_t hashkey;
    Nanotime inserttime[2];
    Nanotime readtime[2];
    Nanotime remtime[2];

    NCT_inittimer();

    fprintf(stderr,"insert:\n");

    for(np=N;*np;np++) {

    NCT_marktime(&inserttime[0]);
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

    NCT_marktime(&inserttime[1]);

    fprintf(stderr,"read:\n");

    NCT_marktime(&readtime[0]);

    for(key=0;key<*np;key++) {
	uintptr_t data = 0;
	ncexhashkey_t hashkey = hkeyfor(key);
	CHECK(ncexhashget(map,hashkey,&data));
#ifdef VERBOSE
        fprintf(stderr,"read[%08llx|%s->%u]:\n",hashkey,ncexbinstr(hashkey,NCEXHASHKEYBITS),(unsigned)data);
#endif
	if(data != key) fprintf(stderr,"\tMISMATCH\n");
    }

    NCT_marktime(&readtime[1]);

    fprintf(stderr,"statistics:\n"); ncexhashprintstats(map);    
    fprintf(stderr,"times: N=%u\n",*np);

    if(!NCT_reporttime(*np, inserttime, insertrange, "insert")) goto fail;
    if(!NCT_reporttime(*np, readtime, readrange, "read")) goto fail;

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
	    if(found[key] == 0) fprintf(stderr,"iterator missing key: %u\n",key);
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

	NCT_marktime(&remtime[0]);

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

	NCT_marktime(&remtime[1]);
        if(!NCT_reporttime(*np, remtime, remrange, "removal")) goto fail;
    }
    
    ncexhashmapfree(map);

    }


    return 0;
fail:
    return 1;
}
