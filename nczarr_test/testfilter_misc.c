/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef USE_HDF5
#include <hdf5.h>
#endif

#include "netcdf.h"
#include "netcdf_aux.h"
#include "netcdf_filter.h"

#include "h5misc.h" /* from plugins dir */

#undef TESTODDSIZE

#undef DEBUG

/* The C standard apparently defines all floating point constants as double;
   we rely on that in this code.
*/
#define DBLVAL 12345678.12345678

#define TEST_ID 32768

#define MAXERRS 8

#define MAXPARAMS 32

static unsigned int baselineparams[NPARAMS];

static const char* testfile = NULL;

#define MAXDIMS 8

#define DFALT_TESTFILE "tmp_misc.nc"

#ifdef TESTODDSIZE
#define NDIMS 1
static size_t dimsize[NDIMS] = {4};
static size_t chunksize[NDIMS] = {3};
#else
#define NDIMS 4
static size_t dimsize[NDIMS] = {4,4,4,4};
static size_t chunksize[NDIMS] = {4,4,4,4};
#endif

static size_t ndims = NDIMS;

static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */
static size_t chunkproduct = 1; /* x-product over actual chunks */

static size_t pattern[MAXDIMS];

static int nerrs = 0;

static int ncid, varid;
static int dimids[MAXDIMS];
static size_t odom[MAXDIMS];
static float* array = NULL;
static float* expected = NULL;

static unsigned int filterid = 0;
static size_t nparams = 0;
static unsigned int params[MAXPARAMS];

/* Test values: must match plugins/H5Zmisc.c */
struct All spec = {
(char)-17,		/* signed byte */
(unsigned char)23,	/* unsigned byte */
(signed short)-25,			/* signed short */
(unsigned short)27U,			/* unsigned short */
77,			/* signed int */
93U,			/* unsigned int */
789.0f,			/* float */
-9223372036854775807LL,	/* signed int64 */
18446744073709551615ULL,/* unsigned int64 */
(double)12345678.12345678/* double */
};

/* Forward */
static int test_test1(void);
static void init(int argc, char** argv);
static void reset(void);
static void odom_reset(void);
static int odom_more(void);
static int odom_next(void);
static int odom_offset(void);
static float expectedvalue(void);
static void verifyparams(void);

#define ERRR do { \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n", \
        __FILE__, __LINE__);                                \
nerrs++;\
} while (0)

static int
check(int err,int line)
{
    if(err != NC_NOERR) {
        fprintf(stderr,"fail (%d): %s\n",line,nc_strerror(err));
    }
    return NC_NOERR;
}

static void
report(const char* msg, int lineno)
{
    fprintf(stderr,"fail: line=%d %s\n",lineno,msg);
    exit(1);
}

#define CHECK(x) check(x,__LINE__)
#define REPORT(x) report(x,__LINE__)

static int
verifychunks(void)
{
    size_t i;
    int store = -1;
    size_t localchunks[MAXDIMS];
    memset(localchunks,0,sizeof(localchunks));
    CHECK(nc_inq_var_chunking(ncid, varid, &store, localchunks));
    if(store != NC_CHUNKED) {
        fprintf(stderr,"bad chunk store\n");
        return 0;
    }
    for(i=0;i<ndims;i++) {
        if(chunksize[i] != localchunks[i]) {
            fprintf(stderr,"bad chunk size: %zu\n",i);
            return 0;
        }
    }
    return 1;
}

static int
create(void)
{
    size_t i;

    /* Create a file with one big variable whose dimensions may or may not be a multiple of chunksize (to see what happens) */
    CHECK(nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    for(i=0;i<ndims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%zu",i);
        CHECK(nc_def_dim(ncid, dimname, dimsize[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, ndims, dimids, &varid));
    return NC_NOERR;
}

static int
setchunking(void)
{
    int store;

    store = NC_CHUNKED;
    CHECK(nc_def_var_chunking(ncid,varid,store,chunksize));
    if(!verifychunks())
        return NC_EINVAL;
    return NC_NOERR;
}

static void
setvarfilter(void)
{
    CHECK(nc_def_var_filter(ncid,varid,TEST_ID,NPARAMS,baselineparams));
    verifyparams();
}

static void
verifyparams(void)
{
    size_t i;
    CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
    if(filterid != TEST_ID) REPORT("id mismatch");
    if(nparams != NPARAMS) REPORT("nparams mismatch");
    for(i=0;i<nparams;i++) {
        if(params[i] != baselineparams[i])
            REPORT("param mismatch");
    }
}

static int
openfile(void)
{
    unsigned int* params = NULL;

    /* Open the file and check it. */
    CHECK(nc_open(testfile, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    /* Check the compression algorithm */
    CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,NULL));
    if(nparams > 0) {
        params = (unsigned int*)malloc(sizeof(unsigned int)*nparams);
        if(params == NULL)
            return NC_ENOMEM;
        CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
    }
    if(filterid != TEST_ID) {
        fprintf(stderr,"open: test id mismatch: %d\n",filterid);
        free(params);
        return NC_EFILTER;
    }
    if(nparams != NPARAMS) {
        size_t i;
        fprintf(stderr,"nparams  mismatch\n");
        for(nerrs=0,i=0;i<nparams;i++) {
            if(params[i] != baselineparams[i]) {
                fprintf(stderr,"open: testparam mismatch: %ld\n",(unsigned long)i);
                nerrs++;
            }
        }
    }
    free(params);
    if(nerrs > 0) return NC_EFILTER;

    /* Verify chunking */
    if(!verifychunks())
        return 0;
    fflush(stderr);
    return 1;
}

static void
fill(void)
{

#ifdef NOODOM
   {
        int i;
	if(actualproduct <= 1) abort();
        for(i=0;i<actualproduct;i++)
            expected[i] = (float)i;
   }
#else
   {
	odom_reset();
	while(odom_more()) {
            int offset = odom_offset();
            float expect = expectedvalue();
            expected[offset] = expect;
            odom_next();
        }
   }
#endif
}


static int
compare(void)
{
    int errs = 0;
    fprintf(stderr,"data comparison: |array|=%ld\n",(unsigned long)actualproduct);
#ifdef NOODOM
    {
        int i;
        for(i=0;i<actualproduct;i++) {
            if(expected[i] != array[i]) {
                fprintf(stderr,"data mismatch: array[%d]=%f expected[%d]=%f\n",
                            i,array[i],i,expected[i]);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
        }
   }
#else
   {
       odom_reset();
       while(odom_more()) {
            int offset = odom_offset();
            float expect = expectedvalue();
            if(array[offset] != expect) {
                fprintf(stderr,"data mismatch: array[%d]=%f expected=%f\n",
                            offset,array[offset],expect);
                errs++;
                if(errs >= MAXERRS)
                    break;
            }
            odom_next();
       }
   }
#endif

   if(errs == 0)
        fprintf(stderr,"no data errors\n");
   return (errs == 0);
}

static void
showparameters(void)
{
    size_t i;
    fprintf(stderr,"test: nparams=%ld: params=",(unsigned long)nparams);
    for(i=0;i<nparams;i++) {
        fprintf(stderr," %u",params[i]);
    }
    fprintf(stderr,"\n");
    for(i=0;i<ndims;i++) {
	if(i==0)
            fprintf(stderr,"dimsizes=%ld",(unsigned long)dimsize[i]);
	else
            fprintf(stderr,",%ld",(unsigned long)dimsize[i]);
    }
    fprintf(stderr,"\n");
    for(i=0;i<ndims;i++) {
	if(i==0)
            fprintf(stderr,"chunksizes=%ld",(unsigned long)chunksize[i]);
	else
            fprintf(stderr,",%ld",(unsigned long)chunksize[i]);
    }
    fprintf(stderr,"\n");
    fflush(stderr);
}

static void
insert(int index, void* src, size_t size)
{
    union {
        unsigned char src8[8];
        unsigned param[2];
    } u;
    unsigned char b = 0;
    unsigned short s = 0;
    unsigned int i = 0;
    memset(&u,0,sizeof(u));
    switch (size) {
    case 1:
	b = *((unsigned char*)src);
        u.param[0] = (unsigned)b;
	baselineparams[index+0] = u.param[0];
	break;
    case 2:
	s = *((unsigned short*)src);
        u.param[0] = (unsigned)s;
	baselineparams[index+0] = u.param[0];
	break;
    case 4:
	i = *((unsigned int*)src);
        u.param[0] = (unsigned)i;
	baselineparams[index+0] = u.param[0];
	break;
    case 8:
	memcpy(u.src8,src,size);
	ncaux_h5filterspec_fix8(u.src8,0);
	baselineparams[index+0] = u.param[0];
	baselineparams[index+1] = u.param[1];
	break;
    default: fprintf(stderr,"insert: unexpected size: %u\n",(unsigned)size); abort();
    }
}

static void
buildbaselineparams(unsigned int testcasenumber)
{
    memset(baselineparams,0,sizeof(baselineparams));
    baselineparams[0] = testcasenumber;
    switch (testcasenumber) {
    case TC_PARAMS:
    case TC_EXPANDED:
	insert(1,&spec.tbyte,sizeof(spec.tbyte));
	insert(2,&spec.tubyte,sizeof(spec.tubyte));
	insert(3,&spec.tshort,sizeof(spec.tshort));
	insert(4,&spec.tushort,sizeof(spec.tushort));
	insert(5,&spec.tint,sizeof(spec.tint));
	insert(6,&spec.tuint,sizeof(spec.tuint));
	insert(7,&spec.tfloat32,sizeof(spec.tfloat32));
	insert(8,&spec.tint64,sizeof(spec.tint64)); /*size=8*/
	insert(10,&spec.tuint64,sizeof(spec.tuint64)); /*size=8*/
	insert(12,&spec.tfloat64,sizeof(spec.tfloat64)); /*size=8*/
	break;
    case TC_ODDSIZE:
	break;
    default:
	fprintf(stderr,"Unknown testcase number: %d\n",testcasenumber);
	abort();
    }
}

/* Test simple parameter passing of all the known numeric types */
static int
test_test1(void)
{
    int ok = 1;

    reset();

    buildbaselineparams(TC_PARAMS);

    fprintf(stderr,"test1: compression.\n");
    create();
    setchunking();
    setvarfilter();
    showparameters();    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(nc_put_var(ncid,varid,expected));
    CHECK(nc_close(ncid));

    fprintf(stderr,"test1: decompression.\n");
    reset();
    openfile();
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_test2(void)
{
    int ok = 1;

    reset();

    buildbaselineparams(TC_ODDSIZE);

    fprintf(stderr,"test2: dimsize %% chunksize != 0: compress.\n");
    create();
    setchunking();
    setvarfilter();
    showparameters();
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    CHECK(nc_put_var(ncid,varid,expected));
    CHECK(nc_close(ncid));

    fprintf(stderr,"test2: dimsize %% chunksize != 0: decompress.\n");
    reset();
    openfile();
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

static int
test_test3(void)
{
    int ok = 1;
    int stat = NC_NOERR;

    reset();

    buildbaselineparams(TC_EXPANDED);

    fprintf(stderr,"test3: buffer expansion != 0: compress.\n");
    create();
    setchunking();
    setvarfilter();
    showparameters();
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();
    /* write array */
    stat = nc_put_var(ncid,varid,expected);

    fprintf(stderr,"test3: error code = %d\n",stat);

    CHECK(nc_close(ncid));

    fprintf(stderr,"test3: expansion != 0: decompress.\n");
    reset();
    openfile();
    CHECK(nc_get_var_float(ncid, varid, array));
    ok = compare();
    CHECK(nc_close(ncid));
    return ok;
}

/**************************************************/
/* Utilities */

static void
reset()
{
    memset(array,0,sizeof(float)*actualproduct);
}

static void
odom_reset(void)
{
    memset(odom,0,sizeof(odom));
}

static int
odom_more(void)
{
    return (odom[0] < dimsize[0]);
}

static int
odom_next(void)
{
    ssize_t i; /* do not make unsigned */
    for(i=(ssize_t)(ndims-1);i>=0;i--) {
        odom[i] += 1;
        if(odom[i] < dimsize[i]) break;
        if(i == 0) return 0; /* leave the 0th entry if it overflows*/
        odom[i] = 0; /* reset this position*/
    }
    return 1;
}

static int
odom_offset(void)
{
    size_t i;
    size_t offset = 0;
    for(i=0;i<ndims;i++) {
        offset *= dimsize[i];
        offset += odom[i];
    }
    return offset;
}

static float
expectedvalue(void)
{
    size_t i;
    float offset = 0;

    for(i=0;i<ndims;i++) {
        offset *= (float)dimsize[i];
        offset += (float)odom[i];
    }
    return offset;
}

static void
init(int argc, char** argv)
{
    size_t i;

    /* get the testfile path */
    if(argc > 1)
        testfile = argv[1];
    else
        testfile = DFALT_TESTFILE;

    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<NDIMS;i++) {
        if(pattern[i] == 1)
	    chunksize[i] = 1;
        totalproduct *= dimsize[i];
        if(i < ndims) {
            actualproduct *= dimsize[i];
            chunkproduct *= chunksize[i];
        }
    }
    /* Allocate max size */
    array = (float*)calloc(1,sizeof(float)*actualproduct);
    expected = (float*)calloc(1,sizeof(float)*actualproduct);
}

/**************************************************/
int
main(int argc, char **argv)
{
#ifdef DEBUG
    H5Eprint1(stderr);
    nc_set_log_level(1);
#endif
    init(argc,argv);
    if(!test_test1()) ERRR;
    if(!test_test2()) ERRR;
    if(!test_test3()) ERRR;
    exit(nerrs > 0?1:0);
}
