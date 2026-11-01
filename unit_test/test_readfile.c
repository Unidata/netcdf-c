/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test NC_readfileF.

In particular, verify that a request for N bytes consumes only N bytes
from the stream. This matters because nc_open() uses NC_readfileF to
read the few bytes of the magic number, and reading more than that
(e.g., the whole file) is expensive on network file systems.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncbytes.h"
#include "ncutil.h"

#define TESTFILE "tmp_readfile.dat"

/* Larger than the internal read block size so that multiple blocks are needed */
#define BIGSIZE (9 * 1024 * 1024 + 17)
#define SMALLSIZE 100

static int failures = 0;

#define CHECK(cond,msg) do { \
    if(!(cond)) {fprintf(stderr,"FAIL (line %d): %s\n",__LINE__,(msg)); failures++;} \
} while(0)

static unsigned char
pattern(size_t i)
{
    return (unsigned char)((i * 31 + 7) & 0xFF);
}

static int
writefile(size_t size)
{
    size_t i;
    FILE* f = fopen(TESTFILE,"wb");
    if(f == NULL) {perror("fopen"); return 0;}
    for(i=0;i<size;i++) fputc(pattern(i),f);
    fclose(f);
    return 1;
}

static int
matches(const NCbytes* buf, size_t count)
{
    size_t i;
    const unsigned char* contents = (const unsigned char*)ncbytescontents((NCbytes*)buf);
    if(ncbyteslength((NCbytes*)buf) != count) return 0;
    for(i=0;i<count;i++) if(contents[i] != pattern(i)) return 0;
    return 1;
}

/* Read amount bytes; expect status expectstat, count bytes in result,
   and the stream positioned at expectpos. */
static void
readtest(const char* name, long long amount, int expectstat, size_t count, long expectpos)
{
    int stat;
    FILE* f = fopen(TESTFILE,"rb");
    NCbytes* buf = ncbytesnew();
    if(f == NULL) {perror("fopen"); failures++; goto done;}
    stat = NC_readfileF(f,buf,amount);
    fprintf(stderr,"%s: amount=%lld stat=%d length=%lu pos=%ld\n",
	    name,amount,stat,(unsigned long)ncbyteslength(buf),ftell(f));
    CHECK(stat == expectstat, name);
    if(expectstat == NC_NOERR) {
        CHECK(matches(buf,count), name);
        CHECK(ftell(f) == expectpos, name);
    }
done:
    if(f) fclose(f);
    ncbytesfree(buf);
}

int
main(void)
{
    /* Small file */
    if(!writefile(SMALLSIZE)) return 1;
    /* Must not read past the requested amount */
    readtest("magic",8,NC_NOERR,8,8);
    readtest("one-byte",1,NC_NOERR,1,1);
    readtest("zero",0,NC_NOERR,0,0);
    readtest("exact",SMALLSIZE,NC_NOERR,SMALLSIZE,SMALLSIZE);
    /* Whole file */
    readtest("all-small",-1,NC_NOERR,SMALLSIZE,SMALLSIZE);
    /* File shorter than requested */
    readtest("truncated",SMALLSIZE+1,NC_ETRUNC,0,0);

    /* Large file spanning several internal blocks */
    if(!writefile(BIGSIZE)) return 1;
    readtest("magic-big",8,NC_NOERR,8,8);
    readtest("half-block",1000000,NC_NOERR,1000000,1000000);
    readtest("multi-block",5 * 1024 * 1024 + 3,NC_NOERR,5 * 1024 * 1024 + 3,5 * 1024 * 1024 + 3);
    readtest("all-big",-1,NC_NOERR,BIGSIZE,BIGSIZE);

    remove(TESTFILE);
    if(failures) {
        fprintf(stderr,"*** FAIL: %d failure(s)\n",failures);
        return 1;
    }
    fprintf(stderr,"*** PASS\n");
    return 0;
}
