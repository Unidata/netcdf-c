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

static unsigned int filterid = 0;
static size_t nparams = 0;
static unsigned int params[MAXPARAMS];
static unsigned int baseline[NPARAMS] = {PARAMVAL};

static NC_FILTER_INFO baseinfo;

static const H5Z_class2_t H5Z_REG[1];

/* Forward */
static int filter_test1(void);
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
verifyfilterinfo(NC_FILTER_INFO* info, NC_FILTER_INFO* base)
{
    int stat = NC_NOERR;
    if(info->version != base->version)
	{stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: version mismatch\n");}
    if(info->format != base->format)
	{stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: format mismatch\n");}
    if(info->id != base->id)
	{stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: id mismatch\n");}
    if(info->format == NC_FILTER_FORMAT_HDF5) {
#ifdef USE_HDF5
	H5Z_class2_t* h5info = (H5Z_class2_t*)info->info;
	H5Z_class2_t* h5base = (H5Z_class2_t*)base->info;
        if(h5info->version != h5base->version)
	    {stat = NC_EINVAL; fprintf(stderr,"verifyfilterinfo: H5Z_class_t: version mismatch\n");}
        if(h5info->id != h5base->id)
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
    } else
	{stat = NC_EINVAL; fprintf(stderr,"Unknown format\n");}

    return stat;
}

static void
registerfilter(void)
{
    NC_FILTER_INFO inqinfo;

    baseinfo.version = NC_FILTER_INFO_VERSION;
    baseinfo.format = NC_FILTER_FORMAT_HDF5;
    baseinfo.id = FILTER_ID;
    baseinfo.info = (void*)&H5Z_REG[0];
    CHECK(nc_filter_register(&baseinfo));
    /* Verify by inquiry */
    memset(&inqinfo,0,sizeof(NC_FILTER_INFO));
    CHECK((nc_filter_inq(NC_FILTER_FORMAT_HDF5, FILTER_ID, &inqinfo)));
    CHECK((verifyfilterinfo(&inqinfo,&baseinfo)));
}

static void
unregisterfilter(void)
{
    int stat = NC_NOERR;
    NC_FILTER_INFO inqinfo;

    /* Verify that the filter info is still good */
    memset(&inqinfo,0,sizeof(NC_FILTER_INFO));
    CHECK((stat = nc_filter_inq(NC_FILTER_FORMAT_HDF5, FILTER_ID, &inqinfo)));
    CHECK((verifyfilterinfo(&inqinfo,&baseinfo)));    
    /* Unregister */
    CHECK((stat = nc_filter_unregister(NC_FILTER_FORMAT_HDF5, FILTER_ID)));
    /* Inq again to verify unregistered */
    stat = nc_filter_inq(NC_FILTER_FORMAT_HDF5, FILTER_ID, NULL);
    if(stat != NC_EFILTER) {
	fprintf(stderr,"unregister: failed\n");
	CHECK(NC_EFILTER);
    }
}

static void
setvarfilter(void)
{
    /* NOOP the params */
    CHECK(nc_def_var_filter(ncid,varid,FILTER_ID,NPARAMS,baseline));
    verifyparams();
}

static void
verifyparams(void)
{
    int i;
    CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
    if(filterid != FILTER_ID) REPORT("id mismatch");
    if(nparams != NPARAMS) REPORT("nparams mismatch");
    for(i=0;i<nparams;i++) {
        if(params[i] != baseline[i])
            REPORT("param mismatch");
    }
}

static int
openfile(void)
{
    unsigned int* params = NULL;

    /* Open the file and check it. */
    CHECK(nc_open(TESTFILE, NC_NOWRITE, &ncid));
    CHECK(nc_inq_varid(ncid, "var", &varid));

    /* Check the compression algorithm */
    CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,NULL));
    if(nparams > 0) {
        params = (unsigned int*)malloc(sizeof(unsigned int)*nparams);
        if(params == NULL)
            return NC_ENOMEM;
        CHECK(nc_inq_var_filter(ncid,varid,&filterid,&nparams,params));
    }
    if(filterid != FILTER_ID) {
        fprintf(stderr,"open: test id mismatch: %d\n",filterid);
        return NC_EFILTER;
    }
    if(nparams != NPARAMS) {
	size_t i;
	unsigned int inqparams[MAXPARAMS];
        fprintf(stderr,"nparams  mismatch\n");
        for(nerrs=0,i=0;i<nparams;i++) {
            if(inqparams[i] != baseline[i]) {
                fprintf(stderr,"open: testparam mismatch: %ld\n",(unsigned long)i);
		nerrs++;
	    }
	}
    }
    if(nerrs > 0) return NC_EFILTER; 

    if(params) free(params);

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
    fprintf(stderr,"data comparison: |array|=%ld\n",(unsigned long)actualproduct);
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
        fprintf(stderr,"no data errors\n");
   return (errs == 0);
}

static void
showparameters(void)
{
    int i;
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

static int
filter_test1(void)
{
    int ok = 1;

    reset();

    fprintf(stderr,"test1: compression.\n");
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

    fprintf(stderr,"test1: decompression.\n");
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
    registerfilter();
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
    /* Unregister filter */
    unregisterfilter();
    exit(nerrs > 0?1:0);
}

/**************************************************/
/* In-line filter code */

#define H5Z_FILTER_REG FILTER_ID

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

    fprintf(stderr,"nbytes=%ld\n",(long)nbytes);

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

static const H5Z_class2_t H5Z_REG[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_REG), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "registered",                    /* Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_reg_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_reg,     /* The actual filter function   */
}};
