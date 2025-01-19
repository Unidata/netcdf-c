#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>

#include "h5misc.h"

#include "netcdf_aux.h"

/* WARNING:
Starting with HDF5 version 1.10.x, the plugin code MUST be
careful when using the standard *malloc()*, *realloc()*, and
*free()* function.

In the event that the code is allocating, reallocating, for
free'ing memory that either came from or will be exported to the
calling HDF5 library, then one MUST use the corresponding HDF5
functions *H5allocate_memory()*, *H5resize_memory()*,
*H5free_memory()* [5] to avoid memory failures.

Additionally, if your filter code leaks memory, then the HDF5 library
will generate an error.

*/

#undef DEBUG

/* The C standard apparently defines all floating point constants as double;
   we rely on that in this code.
*/
#define DBLVAL 12345678.12345678

/* Test values */
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

static size_t H5Z_filter_test(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf);

static int paramcheck(size_t nparams, const unsigned int* params, struct All* extracted);
static void mismatch(const char* which);

const H5Z_class2_t H5Z_TEST[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_TEST), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "test",                          /* Filter name for debugging    */
    NULL,			     /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_test,     /* The actual filter function   */
}};

/* External Discovery Functions */
DECLSPEC
H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

DECLSPEC
const void*
H5PLget_plugin_info(void)
{
    return H5Z_TEST;
}

/*
This filter does some verification
that the parameters passed to the filter
are correct. Specifically, that endian-ness
is correct. As a filter, it is the identify
function, passing input to output unchanged.
It also prints out the size of each chunk.

Test cases format:
1.The first param is the test index i.e. which test to execute.
2. The remaining parameters are those for the test chosen in #1

*/

static size_t
H5Z_filter_test(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    void* newbuf;
    unsigned int testcase = 0;
    size_t size = 1024 * sizeof(float) * 2;
    struct All values;

    if(cd_nelmts == 0)
	goto fail;

    testcase = cd_values[0];

    switch (testcase) {
    case TC_PARAMS:
	if(!paramcheck(cd_nelmts,cd_values,&values))
	    goto fail;
	break;
    case TC_ODDSIZE:
        /* Print out the chunk size */
        fprintf(stderr,">>> nbytes = %lld chunk size = %lld\n",(long long)nbytes,(long long)*buf_size);
        fflush(stderr);
	break;
    default: break;
    }

    if (flags & H5Z_FLAG_REVERSE) { /* Decompress */

        if(testcase == TC_EXPANDED) {
#ifdef DEBUG
	    int i;
	    float* b = (float*)*buf;
fprintf(stderr,">>> TC_EXPANDED: decompress: nbytes=%u buf_size=%u xdata[0..8]=|",(unsigned)nbytes,(unsigned)*buf_size);
	    for(i=0;i<8;i++) {
		fprintf(stderr," %u",(int)(b[1024+i]));
	    }
	    fprintf(stderr,"|\n");
#endif
            /* Replace buffer */
            newbuf = H5allocate_memory(*buf_size,0);
            if(newbuf == NULL) abort();
            memcpy(newbuf,*buf,*buf_size);
	
	} else {
            /* Replace buffer */
            newbuf = H5allocate_memory(*buf_size,0);
            if(newbuf == NULL) abort();
            memcpy(newbuf,*buf,*buf_size);
	}
	
        /* reclaim old buffer */
        H5free_memory(*buf);
        *buf = newbuf;

    } else { /* (flags & H5Z_FLAG_REVERSE) Compress */
        if(testcase == TC_EXPANDED) {
	    int i;
	    float* b;
#ifdef DEBUG
fprintf(stderr,">>> TC_EXPANDED: compress: nbytes=%u buf_size=%u size=%u\n",(unsigned)nbytes,(unsigned)*buf_size,(unsigned)size);
#endif
	    /* Replace buffer with one that is bigger than the input size */
            newbuf = H5allocate_memory(size,0);
            if(newbuf == NULL) abort();
	    b = (float*)newbuf;
	    for(i=0;i<1024*2;i++) {
		b[i] = (float)(17+i);
	    }
            memcpy(newbuf,*buf,*buf_size);
	    *buf_size = size;
        } else  {
            /* Replace buffer */
            newbuf = H5allocate_memory(*buf_size,0);
            if(newbuf == NULL) abort();
            memcpy(newbuf,*buf,*buf_size);
	}

	/* reclaim old buffer */
        H5free_memory(*buf);
        *buf = newbuf;
    }

    return *buf_size;

fail:
    return 0;
}

static void
extract1(void* field, size_t size, const unsigned int* params)
{
    union {
	unsigned long long ll;
        unsigned char char8[8];
        unsigned param[2];
    } u;
    unsigned char b = 0;
    unsigned short s = 0;
    unsigned int i = 0;
    unsigned char* bp = 0;
    unsigned short* sp = NULL;
    unsigned int* ip = NULL;
    unsigned long long* llp = NULL;
    memset(&u,0,sizeof(u));
    switch (size) {
    case 1:
	b = (unsigned char)(params[0]);
	bp = (unsigned char*)field;
	*bp = b;
	break;
    case 2:
	s = (unsigned short)(params[0]);
	sp = (unsigned short*)field;
	*sp = s;
	break;
    case 4:
	i = (unsigned)(params[0]);
	ip = (unsigned*)field;
	*ip = i;
	break;
    case 8:
	u.param[0] = params[0];
	u.param[1] = params[1];
	ncaux_h5filterspec_fix8(u.char8,0);
	llp = (unsigned long long*)field;
	*llp = u.ll;
	break;
    default: fprintf(stderr,">>> insert: unexpected size: %u\n",(unsigned)size); abort();
    }
}

static void
extractparams(size_t nparams, const unsigned int* params, struct All* all)
{
    size_t offset = 0;
    NC_UNUSED(nparams);
    extract1(&all->tbyte,sizeof(all->tbyte),&params[offset]); offset += 1;
    extract1(&all->tubyte,sizeof(all->tubyte),&params[offset]); offset += 1;
    extract1(&all->tshort,sizeof(all->tshort),&params[offset]); offset += 1;
    extract1(&all->tushort,sizeof(all->tushort),&params[offset]); offset += 1;
    extract1(&all->tint,sizeof(all->tint),&params[offset]); offset += 1;
    extract1(&all->tuint,sizeof(all->tuint),&params[offset]); offset += 1;
    extract1(&all->tfloat32,sizeof(all->tfloat32),&params[offset]); offset += 1;
    extract1(&all->tint64,sizeof(all->tint64),&params[offset]); offset += 2*1;
    extract1(&all->tuint64,sizeof(all->tuint64),&params[offset]); offset += 2*1;
    extract1(&all->tfloat64,sizeof(all->tfloat64),&params[offset]); offset += 2*1;
}

/* Verify values of the parameters */
static int
paramcheck(size_t nparams, const unsigned int* params, struct All* extracted)
{
    struct All all;

    memset(&all,0,sizeof(all));

    if(nparams != NPARAMS) {
	fprintf(stderr,">>> (0) Incorrect number of parameters: expected=%ld sent=%ld\n",(unsigned long)NPARAMS,(unsigned long)nparams);
	goto fail;
    }

    /* Convert numeric params to instance of struct All */
    extractparams(nparams-1, &params[1], &all);
    if(extracted) *extracted = all;

    /* verify */
    if(all.tbyte != spec.tbyte) mismatch("tbyte");
    if(all.tubyte != spec.tubyte) mismatch("tubyte");
    if(all.tshort != spec.tshort) mismatch("tshort");
    if(all.tushort != spec.tushort) mismatch("tushort");
    if(all.tint != spec.tint) mismatch("tint");
    if(all.tuint != spec.tuint) mismatch("tuint");
    if(all.tfloat32 != spec.tfloat32) mismatch("tfloat32");
    if(all.tint64 != spec.tint64) mismatch("tint64");
    if(all.tuint64 != spec.tuint64) mismatch("tuint64");
    if(all.tfloat64 != spec.tfloat64) mismatch("tfloat64");

#ifdef DEBUG
    {
	size_t i;
	fprintf(stderr,">>> nparams=%lu params=\n",nparams);
	for(i=0;i<nparams;i++) {
	    fprintf(stderr,"[%d] %ud %d %f\n", (unsigned int)i, params[i],(signed int)params[i],*(float*)&params[i]);
	}
	fflush(stderr);
    }
#endif
    return 1;
fail:
    return 0;
}

static void
mismatch(const char* which)
{
    fprintf(stderr,">>> mismatch: %s\n",which);
    fflush(stderr);
}

