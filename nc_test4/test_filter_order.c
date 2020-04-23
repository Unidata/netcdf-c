/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include <hdf5.h>
/* Older versions of the hdf library may define H5PL_type_t here */
#include <H5PLextern.h>

#include "netcdf.h"
#include "netcdf_filter.h"

#undef TESTODDSIZE

#undef DEBUG

#define FILTER_ID 32768

#define MAXERRS 8

#define MAXPARAMS 32

#define MAXDIMS 8

#define TESTFILE "testfilter_reg.nc"

#define NPARAMS 1
#define PARAMVAL 17

#define NDIMS 4
static size_t dimsize[NDIMS] = {4,4,4,4};
static size_t chunksize[NDIMS] = {4,4,4,4};

static size_t ndims = NDIMS;

static size_t totalproduct = 1; /* x-product over max dims */
static size_t actualproduct = 1; /* x-product over actualdims */
static size_t chunkproduct = 1; /* x-product over actual chunks */

static int nerrs = 0;

static int ncid, varid;
static int dimids[MAXDIMS];
static size_t odom[MAXDIMS];
static float* array = NULL;
static float* expected = NULL;

static size_t nparams = 0;
static unsigned int params[MAXPARAMS];
static unsigned int baseline[2][NPARAMS] = {{0},{1}};

static struct Base {
    unsigned int id;
    H5Z_class2_t* info;
} baseinfo;

static const H5Z_class2_t H5Z_REG[2];

/* Forward */
static int filter_test1(void);
static void init(int argc, char** argv);
static void reset(void);
static void odom_reset(void);
static int odom_more(void);
static int odom_next(void);
static int odom_offset(void);
static float expectedvalue(void);
static void verifyparams(int);

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
    int i;
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
            fprintf(stderr,"bad chunk size: %d\n",i);
            return 0;
        }
    }
    return 1;
}

static int
create(void)
{
    int i;

    /* Create a file with one big variable */
    CHECK(nc_create(TESTFILE, NC_NETCDF4|NC_CLOBBER, &ncid));
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));
    for(i=0;i<ndims;i++) {
        char dimname[1024];
        snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, dimsize[i], &dimids[i]));
    }
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, ndims, dimids, &varid));
    return NC_NOERR;
}

static int
verifyfilterinfo(struct Base* info, struct Base* base)
{
    int stat = NC_NOERR;
    if(info->id != base->id)
	{stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: id mismatch\n");}
#ifdef USE_HDF5
    H5Z_class2_t* h5info = info->info;
    H5Z_class2_t* h5base = base->info;
    if(info->id != base->id)
        {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: id mismatch\n");}
    if(h5info->encoder_present != h5base->encoder_present)
        {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: encoder_present mismatch\n");}
    if(h5info->decoder_present != h5base->decoder_present)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: decoder_present mismatch\n");}
    if(h5info->decoder_present != h5base->decoder_present)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: decoder_present mismatch\n");}
    if(strcmp(h5info->name,h5base->name) != 0)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: name mismatch\n");}
    if(h5info->can_apply != h5base->can_apply)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: can_apply mismatch\n");}
    if(h5info->set_local != h5base->set_local)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: set_local mismatch\n");}
    if(h5info->filter != h5base->filter)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: filter mismatch\n");}
#else
     stat = NC_ENOTBUILT; fprintf(stderr,"Unknown format\n")}
#endif
    return stat;
}

static void
registerfilters(void)
{
    struct Base inqinfo;

    /* Register filter 0 */
    baseinfo.id = FILTER_ID;
    baseinfo.info = (H5Z_class2_t*)&H5Z_REG[0];
    CHECK(nc_filter_client_register(baseinfo.id,baseinfo.info));
    /* Verify by inquiry */
    memset(&inqinfo,0,sizeof(struct Base));
    inqinfo.id = FILTER_ID;
    if((inqinfo.info = (H5Z_class2_t*)calloc(1,sizeof(H5Z_class2_t))) == NULL)
        CHECK(NC_ENOMEM);
    CHECK((nc_filter_client_inq(inqinfo.id,(void*)inqinfo.info)));
    CHECK((verifyfilterinfo(&inqinfo,&baseinfo)));
    nullfree(inqinfo.info);

    /* Register filter 1 */
    baseinfo.id = FILTER_ID+1;
    baseinfo.info = (H5Z_class2_t*)&H5Z_REG[1];
    CHECK(nc_filter_client_register(baseinfo.id,baseinfo.info));
    /* Verify by inquiry */
    memset(&inqinfo,0,sizeof(struct Base));
    inqinfo.id = FILTER_ID+1;
    if((inqinfo.info = (H5Z_class2_t*)calloc(1,sizeof(H5Z_class2_t))) == NULL)
        CHECK(NC_ENOMEM);
    CHECK((nc_filter_client_inq(inqinfo.id,(void*)inqinfo.info)));
    CHECK((verifyfilterinfo(&inqinfo,&baseinfo)));
    nullfree(inqinfo.info);
}

static void
setvarfilter(int index)
{
    /* NOOP the params */
    CHECK(nc_def_var_filter(ncid,varid,FILTER_ID+index,NPARAMS,baseline[index]));
    verifyparams(index);
}

static void
verifyparams(int index)
{
    int i;
    CHECK(nc_inq_var_filter_info(ncid,varid,FILTER_ID+index,&nparams,params));
    if(nparams != NPARAMS) REPORT("nparams mismatch");
    for(i=0;i<nparams;i++) {
        if(params[i] != baseline[index][i])
            REPORT("param mismatch");
    }
}

static int
openfile(void)
{
    unsigned int filterids[2];
    unsigned int params[1];
    size_t nfilters = 0;
    int k;

    /* Open the file and check it. */
    CHECK(nc_open(TESTFILE, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    /* Check the compression algorithms */
    CHECK(nc_inq_var_filterids(ncid,varid,&nfilters,filterids));
    if(nfilters != 2)
        return NC_EINVAL;
    for(k=0;k<nfilters;k++) {
        CHECK(nc_inq_var_filter_info(ncid,varid,filterids[k],&nparams,params));
        if(nparams != 1) {
            fprintf(stderr,"open: test nparams mismatch: %u\n",(unsigned)nparams);
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
    int store;

    store = NC_CHUNKED;
    CHECK(nc_def_var_chunking(ncid,varid,store,chunksize));
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
            expected[i] = (float)i;
   } else {
       while(odom_more()) {
            int offset = odom_offset();
            float expect = expectedvalue();
            expected[offset] = expect;
            odom_next();
        }
   }
}

static int
compare(void)
{
    int errs = 0;
    printf("data comparison: |array|=%ld\n",(unsigned long)actualproduct);
    if(1)
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
   } else
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

   if(errs == 0)
        printf("no data errors\n");
   return (errs == 0);
}

static void
showparameters(void)
{
    int i;
    printf("test: nparams=%ld: params=",(unsigned long)nparams);
    for(i=0;i<nparams;i++) {
        printf(" %u",params[i]);
    }
    printf("\n");
    for(i=0;i<ndims;i++) {
	if(i==0)
            printf("dimsizes=%ld",(unsigned long)dimsize[i]);
	else
            printf(",%ld",(unsigned long)dimsize[i]);
    }
    printf("\n");
    for(i=0;i<ndims;i++) {
	if(i==0)
            printf("chunksizes=%ld",(unsigned long)chunksize[i]);
	else
            printf(",%ld",(unsigned long)chunksize[i]);
    }
    printf("\n");
    fflush(stderr);
}

static int
filter_test1(void)
{
    int ok = 1;

    reset();

    printf("test1: filter order.\n");
    create();
    setchunking();
    printf("set var filter 0\n");
    setvarfilter(0);
    showparameters();
    printf("set var filter 1\n");
    setvarfilter(1);
    showparameters();
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    fill();

    printf("test1: compression.\n");
    /* write array */
    CHECK(nc_put_var(ncid,varid,expected));
    CHECK(nc_close(ncid));

    printf("test1: decompression.\n");
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
    int i; /* do not make unsigned */
    for(i=ndims-1;i>=0;i--) {
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
    int i;
    int offset = 0;
    for(i=0;i<ndims;i++) {
        offset *= dimsize[i];
        offset += odom[i];
    }
    return offset;
}

static float
expectedvalue(void)
{
    int i;
    float offset = 0;

    for(i=0;i<ndims;i++) {
        offset *= dimsize[i];
        offset += odom[i];
    }
    return offset;
}

static void
init(int argc, char** argv)
{
    int i;
    /* Setup various variables */
    totalproduct = 1;
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<NDIMS;i++) {
	totalproduct *= dimsize[i];
        if(i < ndims) {
            actualproduct *= dimsize[i];
            chunkproduct *= chunksize[i];
        }
    }
    /* Allocate max size */
    array = (float*)calloc(1,sizeof(float)*actualproduct);
    expected = (float*)calloc(1,sizeof(float)*actualproduct);
    /* Register the filter */
    registerfilters();
}

/**************************************************/
int
main(int argc, char **argv)
{
#ifdef DEBUG
    H5Eprint(stderr);
    nc_set_log_level(1);
#endif
    init(argc,argv);
    if(!filter_test1()) ERRR;
    exit(nerrs > 0?1:0);
}

/**************************************************/
/* In-line filter code */

#define H5Z_FILTER_REG0 FILTER_ID
#define H5Z_FILTER_REG1 (FILTER_ID+1)

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

static int paramcheck(size_t nparams, const unsigned int* params);

/* Forward */
static int paramcheck(size_t nparams, const unsigned int* params);

/* Make this explicit */
/*
 * The "can_apply" callback returns positive a valid combination, zero for an
 * invalid combination and negative for an error.
 */
static htri_t
H5Z_reg_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* Assume it can always apply */
}

/*
As a filter, it is the identity function,
passing input to output unchanged.
*/

size_t
H5Z_filter_reg(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    void* newbuf;

    if(cd_nelmts == 0)
	goto fail;

    if(!paramcheck(cd_nelmts,cd_values))
	goto fail;

    printf("Apply filter: %u\n",cd_values[0]);

    if (flags & H5Z_FLAG_REVERSE) {
        /* Replace buffer */
#ifdef HAVE_H5ALLOCATE_MEMORY
        newbuf = H5allocate_memory(*buf_size,0);
#else
        newbuf = malloc(*buf_size);
#endif
        if(newbuf == NULL) abort();
	if(*buf != NULL) {
            memcpy(newbuf,*buf,*buf_size);
            /* reclaim old buffer */
#ifdef HAVE_H5FREE_MEMORY
            H5free_memory(*buf);
#else
            free(*buf);
#endif
	}
        *buf = newbuf;

    } else {

        /* Replace buffer */
#ifdef HAVE_H5ALLOCATE_MEMORY
        newbuf = H5allocate_memory(*buf_size,0);
#else
	newbuf = malloc(*buf_size);
#endif
	if(newbuf == NULL) abort();
	if(*buf != NULL) {
            memcpy(newbuf,*buf,*buf_size);
    	    /* reclaim old buffer */
#ifdef HAVE_H5FREE_MEMORY
            H5free_memory(*buf);
#else
            free(*buf);
#endif
	}
        *buf = newbuf;

    }

    return *buf_size;

fail:
    return 0;
}

static int
paramcheck(size_t nparams, const unsigned int* params)
{
    if(nparams != 1) {
	fprintf(stderr,"Incorrect parameter count: need=1 sent=%ld\n",(unsigned long)nparams);
	goto fail;
    }

    return 1;

fail:
    return 0;
}

static const H5Z_class2_t H5Z_REG[2] = {
{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_REG0), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "registered0",                   /* Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_reg_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_reg,     /* The actual filter function   */
},
{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_REG1), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "registered1",                   /* Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_reg_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_reg,     /* The actual filter function   */
}
};
