/*
  Copyright 2008, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netcdf.h"

//#define BASELINE 1

#define USEFLOAT 1

/* Write using var1_T instead of var_T */
#undef VAR1

#define BZIP2_ID 307
#define BZIP2_LEVEL 9

#define MAXERRS 8

#define MAXPARAMS 32

/* Following 3 must be consistent */
#ifdef USEFLOAT
#define T float
#else
#define T int
#endif

#define NC_PUT_VAR1 nc_put_var1_float
#define NC_PUT_VAR nc_put_var_float

typedef enum XZIP { NOZIP=0, BZIP2=1} XZIP;

/* Created Meta-data 
netcdf zip {
dimensions:
	dim1 = .. ;
	dim2 = ... ;
	dim3 = ... ;
	...
	dimn = ... ;
variables:
	int var(dim1, dim2, dim3,...dimn) ;
}
*/

#define MAXDIMS 8

#define DEFAULTACTUALDIMS 4
#define DEFAULTDIMSIZE 4
#define DEFAULTCHUNKSIZE 4

static size_t dimsize = DEFAULTDIMSIZE;
static size_t chunksize = DEFAULTCHUNKSIZE;
static size_t actualdims = DEFAULTACTUALDIMS;
static size_t pattern[MAXDIMS];

static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */
static size_t chunkproduct = 1; /* x-product over actual chunks */

static size_t dims[MAXDIMS];
static size_t chunks[MAXDIMS];

static int nerrs = 0;

static int ncid, varid;
static int dimids[MAXDIMS];
static size_t odom[MAXDIMS];
static T* array = NULL;
static T* expected = NULL;
static unsigned int filterid = 0;
static size_t nparams = 0;
static unsigned int* params = NULL;

/* Forward */
static int test_bzip2(void);
static void init(int argc, char** argv);
static void reset(void);
static void odom_reset(void);
static int odom_more(void);
static int odom_next(void);
static int odom_offset(void);
static T expectedvalue(void);

#define ERRR do { \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n", \
	__FILE__, __LINE__);				    \
nerrs++;\
} while (0)

static int
check(int err,int line)
{
    if(err != NC_NOERR) {
	fprintf(stderr,"fail (%d): %s\n",line,nc_strerror(err));
	fflush(stderr);
	exit(1);
    }
    return NC_NOERR;
}

#define CHECK(x) check(x,__LINE__)

static char*
filenamefor(XZIP encoder)
{
    static char testfile[2048];
    snprintf(testfile,sizeof(testfile),"%s.nc",
		(encoder == NOZIP?"nozip":"bzip2"));
    return testfile;
}

static int
verifychunks(void)
{
    int i;
    int store = -1;
    size_t chunksizes[MAXDIMS];
    memset(chunksizes,0,sizeof(chunksizes));
    CHECK(nc_inq_var_chunking(ncid, varid, &store, chunksizes));
    if(store != NC_CHUNKED) {
	fprintf(stderr,"bad chunk store\n");
	return 0;
    }
    for(i=0;i<actualdims;i++) {
        if(chunksizes[i] != chunks[i]) {
	    fprintf(stderr,"bad chunk size: %d\n",i);
	    return 0;
	}
    }
    return 1;
}

static int
create(XZIP encoder)
{
    int i;
    char* testfile = filenamefor(encoder);

    /* Create a file with one big variable. */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    for(i=0;i<actualdims;i++) {
	char dimname[1024];
	snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, dims[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, actualdims, dimids, &varid));
    return NC_NOERR;
}

static void
setvarfilter(XZIP encoder)
{
    unsigned int level = BZIP2_LEVEL;
    unsigned int id=0;
    size_t nparams = 0;
    if(encoder == BZIP2) {
        CHECK(nc_def_var_filter(ncid,varid,BZIP2_ID,1,&level));
	level = 0;
        CHECK(nc_inq_var_filter(ncid,varid,&id,&nparams,&level));
	if(id != BZIP2_ID || nparams != 1 || level != BZIP2_LEVEL)
	    printf("setvarfilter: def/inq mismatch\n");
    }
}

static int
open(XZIP encoder)
{
    char* testfile = filenamefor(encoder);

    /* Open the file and check it. */
    CHECK(nc_open(testfile, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    if(0 || encoder != NOZIP) {
        /* Check the compression algorithm */
	filterid = 0;
	nparams = 0;
	params = NULL;
        CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,NULL));
	if(nparams > 0) {
	    params = (unsigned int*)malloc(sizeof(unsigned int)*nparams);
	    if(params == NULL)
		return NC_ENOMEM;
            CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
	}
        if(filterid != BZIP2_ID) {
	    printf("Bzip2 id mismatch: %d\n",filterid);
	    return NC_EFILTER;
        }
	if(nparams != 1 && params != NULL && params[0] != BZIP2_LEVEL) {
	    printf("Compression parameter mismatch\n");
	    return NC_EFILTER; 
	}
    }

    /* Verify chunking */
    if(!verifychunks())
	return 0;
    fflush(stderr);
    return 1;
}

static int
setchunking(void)
{
    int i;
    int store;

    store = NC_CHUNKED;
    CHECK(nc_def_var_chunking(ncid,varid,store,chunks));
    if(!verifychunks())
	return NC_EINVAL;
    return NC_NOERR;
}

static void
fill(void)
{
   odom_reset();
   if(1) {
	int i;
if(actualproduct <= 1) abort();
	for(i=0;i<actualproduct;i++)
	    expected[i] = (T)i;
   } else {
       while(odom_more()) {
	    int offset = odom_offset();
	    T expect = expectedvalue();
	    expected[offset] = expect;
	    odom_next();
	}
   }
}

static int
write(void)
{
   int stat = NC_NOERR;
#ifdef VAR1
   odom_reset();
   while(odom_more()) {
	size_t offset = odom_offset();
	CHECK(NC_PUT_VAR1(ncid,varid,odom,&expected[offset]));
	odom_next();
   }
#else
   stat = NC_PUT_VAR(ncid,varid,expected);
#endif
   return stat;
}


static int
compare(void)
{
    int errs = 0;
    printf("data comparison: |array|=%d\n",actualproduct);
    if(1)
    {
	int i;
	for(i=0;i<actualproduct;i++) {
	    if(expected[i] != array[i]) {
                printf("mismatch: array[%d]=%f expected[%d]=%f\n",
                            i,array[i],i,expected[i]);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
	}
   } else
   {
       odom_reset();
       while(odom_more()) {
            int offset = odom_offset();
            float expect = expectedvalue();
            if(array[offset] != expect) {
                printf("mismatch: array[%d]=%f expected=%f\n",
                            offset,array[offset],expect);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
            odom_next();
       }
   }

   if(errs == 0)
        printf("no data errors\n");
   return (errs == 0);
}

static void
showparameters(XZIP encoding)
{
    int i;
    printf("bzip2:");
    for(i=0;i<nparams;i++) {
	printf(" %u",params[i]);
    }
    printf("\n");
    for(i=0;i<actualdims;i++)
	printf("%s%d",(i==0?" chunks=":","),chunks[i]);
    printf("\n");
}

static int
test_bzip2(void)
{
    int ok = 1;
    unsigned int param = BZIP2_LEVEL;

    printf("\n*** Testing API: bzip2 compression.\n");
    reset();

    create(BZIP2);
    setchunking();
    setvarfilter(BZIP2); showparameters(BZIP2);
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    printf("\n*** Testing API: bzip2 decompression.\n");
    reset();
    open(BZIP2);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

#ifdef BASELINE
static int
test_nozip(void)
{
    int ok = 1;

    printf("\n*** Testing nozip compression.\n");
    reset();

    create(NOZIP);
    setchunking();

    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(write());
    CHECK(nc_close(ncid));

    printf("\n*** Testing nozip decompression.\n");
    reset();
    open(NOZIP);
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}
#endif

/**************************************************/
/* Utilities */

static void
reset()
{
    memset(array,0,sizeof(T)*actualproduct);
}

static void
odom_reset(void)
{
    memset(odom,0,sizeof(odom));
}

static int
odom_more(void)
{
    return (odom[0] < dims[0]);
}

static int
odom_next(void)
{
    int i; /* do not make unsigned */
    for(i=actualdims-1;i>=0;i--) {
        odom[i] += 1;
        if(odom[i] < dims[i]) break;
	if(i == 0) return 0; /* leave the 0th entry if it overflows*/
	odom[i] = 0; /* reset this position*/
    }
    return 1;
}

static int
odom_offset(void)
{
    int i;
    int offset = 0;
    for(i=0;i<actualdims;i++) {
	offset *= dims[i];
	offset += odom[i];
    } 
    return offset;
}

static T
expectedvalue(void)
{
    int i;
    T offset = 0;

    for(i=0;i<actualdims;i++) {
	offset *= dims[i];
	offset += odom[i];
    } 
    return offset;
}

#if 0
#ifndef USEFLOAT
static size_t
getint(const char* arg)
{
    char* p;
    long l = strtol(arg,&p,10);
    if(*p == '\0')
	return (size_t)l;
    fprintf(stderr,"expected integer: found %s\n", arg);
    exit(1);
}
#else
static double
getdouble(const char* arg)
{
    char* p;
    double d = strtod(arg,&p);
    if(*p == '\0')
	return d;
    fprintf(stderr,"expected double: found %s\n", arg);
    exit(1);
}
#endif
#endif

static void
init(int argc, char** argv)
{
    int i;
    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<MAXDIMS;i++) {
	dims[i] = dimsize;
	chunks[i] = (pattern[i] == 1 ? 1 : chunksize);
	totalproduct *= dims[i];
	if(i < actualdims) {
	    actualproduct *= dims[i];
	    chunkproduct *= chunks[i];
	}
    }
    /* Allocate max size */
    array = (T*)calloc(1,sizeof(T)*actualproduct);
    expected = (T*)calloc(1,sizeof(T)*actualproduct);
}

/**************************************************/
int
main(int argc, char **argv)
{
    init(argc,argv);
#ifdef BASELINE
    if(!test_nozip()) ERRR;
#endif
    if(!test_bzip2()) ERRR;
    exit(nerrs > 0?1:0);
}

