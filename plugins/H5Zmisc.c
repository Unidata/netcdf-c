#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <hdf5.h>
/* Older versions of the hdf library may define H5PL_type_t here */
#include <H5PLextern.h>
#include "h5misc.h"

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

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

static int paramcheck(size_t nparams, const unsigned int* params);
static void mismatch(size_t i, const char* which);

extern void NC_filterfix8(void* mem, int decode);

const H5Z_class2_t H5Z_TEST[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_TEST), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "test",                          /* Filter name for debugging    */
    (H5Z_can_apply_func_t)H5Z_test_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_test,     /* The actual filter function   */
}};

/* External Discovery Functions */
H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

const void*
H5PLget_plugin_info(void)
{
    return H5Z_TEST;
}

/* Make this explicit */
/*
 * The "can_apply" callback returns positive a valid combination, zero for an
 * invalid combination and negative for an error.
 */
htri_t
H5Z_test_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* Assume it can always apply */
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

size_t
H5Z_filter_test(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    void* newbuf;
    unsigned int testcase = 0;

    if(cd_nelmts == 0)
	goto fail;

    testcase = cd_values[0];

    switch (testcase) {
    case TC_ENDIAN:
	if(!paramcheck(cd_nelmts,cd_values))
	    goto fail;
	break;
    case TC_ODDSIZE:
        /* Print out the chunk size */
        fprintf(stderr,"nbytes = %lld chunk size = %lld\n",(long long)nbytes,(long long)*buf_size);
        fflush(stderr);
	break;
    default: break;
    }

    if (flags & H5Z_FLAG_REVERSE) {

        /* Replace buffer */
#ifdef HAVE_H5ALLOCATE_MEMORY
        newbuf = H5allocate_memory(*buf_size,0);
#else
        newbuf = malloc(*buf_size);
#endif
        if(newbuf == NULL) abort();
        memcpy(newbuf,*buf,*buf_size);
        /* reclaim old buffer */
#ifdef HAVE_H5FREE_MEMORY
        H5free_memory(*buf);
#else
        free(*buf);
#endif
        *buf = newbuf;

    } else {

        /* Replace buffer */
#ifdef HAVE_H5ALLOCATE_MEMORY
      newbuf = H5allocate_memory(*buf_size,0);
#else
      newbuf = malloc(*buf_size);
#endif
      if(newbuf == NULL) abort();
        memcpy(newbuf,*buf,*buf_size);
	/* reclaim old buffer */
#ifdef HAVE_H5FREE_MEMORY
        H5free_memory(*buf);
#else
        free(*buf);
#endif
        *buf = newbuf;

    }

    return *buf_size;

fail:
    return 0;
}

static int
paramcheck(size_t nparams, const unsigned int* params)
{
    size_t i;
    unsigned char mem[8];

    if(nparams != 14) {
	fprintf(stderr,"Too few parameters: need=14 sent=%ld\n",(unsigned long)nparams);
	goto fail;
    }

    for(i=0;i<nparams;i++) {
	unsigned int ival;
	unsigned long long lval;
	float fval;
	double dval;
        switch (i) {
        case 0: break; /* this is the testcase # */
        case 1:
	    ival = (-17) & 0xff;
	    if(ival != (signed int)(params[i]))
	    {mismatch(i,"signed byte"); goto fail; };
	    break;
        case 2:
	    ival = 23;
	    if(ival != (unsigned int)(params[i]))
	    {mismatch(i,"unsigned byte"); goto fail; };
	    break;
        case 3:
	    ival = (-25) & 0xffff;
	    if(ival != (signed int)(params[i]))
	    {mismatch(i,"signed short"); goto fail; };
	    break;
        case 4:
	    ival = 27;
	    if(ival != (unsigned int)(params[i]))
	    {mismatch(i,"unsigned short"); goto fail; };
	    break;
        case 5:
	    ival = 77;
	    if(ival != (signed int)(params[i]))
	    {mismatch(i,"signed int"); goto fail; };
	    break;
        case 6:
	    ival = 93u;
	    if(ival != (unsigned int)(params[i]))
	    {mismatch(i,"unsigned int"); goto fail; };
	    break;
        case 7:
	    fval = 789.0f;
	    if(fval != *(float*)(&params[i]))
	    {mismatch(i,"float"); goto fail; };
	    break;
        case 8: {/*double*/
            double x;
	    memcpy(mem,&params[i],sizeof(mem));
	    NC_filterfix8(mem,1); /* Fix up endianness */
            x = *(double*)mem;
	    dval = DBLVAL;
            i++; /* takes two parameters */
	    if(dval != x) {
                mismatch(i,"double");
                goto fail;
            }
            }; break;
        case 10: {/*signed long long*/
            signed long long x;
	    memcpy(mem,&params[i],sizeof(mem));
	    NC_filterfix8(mem,1); /* Fix up endianness */
            x = *(signed long long*)mem;
	    NC_filterfix8(&x,1); /* Fix up endianness */
	    lval = -9223372036854775807L;
            i++; /* takes two parameters */
            if(lval != x) {
                mismatch(i,"signed long long");
                goto fail;
            }
            }; break;
        case 12: {/*unsigned long long*/
            unsigned long long x;
	    memcpy(mem,&params[i],sizeof(mem));
	    NC_filterfix8(mem,1); /* Fix up endianness */
            x = *(unsigned long long*)mem;
	    lval = 18446744073709551615UL;
            i++; /* takes two parameters */
            if(lval != x) {
                mismatch(i,"unsigned long long");
                goto fail;
            }
            }; break;

        default:
            mismatch(i,"unexpected parameter");
            goto fail;
            break;
        }
    }

#ifdef DEBUG
    {
	size_t i;
	fprintf(stderr,"bigendian=%d nparams=%d params=\n",bigendian,nparams);
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
mismatch(size_t i, const char* which)
{
    fprintf(stderr,"mismatch: [%ld] %s\n",(unsigned long)i,which);
    fflush(stderr);
}
