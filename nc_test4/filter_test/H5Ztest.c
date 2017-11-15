#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <hdf5.h>
/* Older versions of the hdf library may define H5PL_type_t here */
#include <H5PLextern.h>

#include "h5test.h"

#undef DEBUG 

static int paramcheck(size_t nparams, const unsigned int* params);
static void byteswap8(unsigned char* mem);
static void mismatch(size_t i, const char* which);

const H5Z_class2_t H5Z_TEST[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
    (H5Z_filter_t)(H5Z_FILTER_TEST), /* Filter id number */
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
    "test",                          /* Filter name for debugging    */
    NULL,                            /* The "can apply" callback     */
    NULL,                            /* The "set local" callback     */
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

/*
This filter does some verification
that the parameters passed to the filter
are correct. Specifically, that endian-ness
is correct. As a filter, it is the identify
function, passing input to output unchanged.

Test cases format:
1.The first param is the test index i.e. which test to execute.
2. The remaining parameters are those for the test chosen in #1

*/

size_t
H5Z_filter_test(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    int ret;
    void* newbuf;
    unsigned int testcase = 0;

    if(cd_nelmts == 0)
	goto fail;

    testcase = cd_values[0];

    if(testcase == TC_ENDIAN) {
	if(!paramcheck(cd_nelmts,cd_values))
	    goto fail;
    }

    if (flags & H5Z_FLAG_REVERSE) {

        /* Replace buffer */
        newbuf = malloc(*buf_size);
        if(newbuf == NULL) abort();
        memcpy(newbuf,*buf,*buf_size);
        *buf = newbuf;

    } else {

        /* Replace buffer */
        newbuf = malloc(*buf_size);
        if(newbuf == NULL) abort();
        memcpy(newbuf,*buf,*buf_size);
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
    /* Test endianness of this machine */
    const unsigned char b[4] = {0x0,0x0,0x0,0x1}; /* value 1 in big-endian*/
    int endianness = (1 == *(unsigned int*)b); /* 1=>big 0=>little*/

    if(nparams != 14) {
	fprintf(stderr,"Too few parameters: need=16 sent=%ld\n",nparams);
	return 0;
    }       

    for(i=0;i<nparams;i++) {
        switch (i) {
        case 0: break; /* this is the testcase # */
        case 1: if(((signed char)-17) != (signed int)(params[i]))
	    {mismatch(i,"signed byte"); return 0; };
	    break;
        case 2: if(((unsigned char)23) != (unsigned int)(params[i]))
	    {mismatch(i,"unsigned byte"); return 0; };
	    break;
        case 3: if(((signed short)-25) != (signed int)(params[i]))
	    {mismatch(i,"signed short"); return 0; };
	    break;
        case 4: if(((unsigned short)27) != (unsigned int)(params[i]))
	    {mismatch(i,"unsigned short"); return 0; };
	    break;
        case 5: if(77 != (signed int)(params[i]))
	    {mismatch(i,"signed int"); return 0; };
	    break;
        case 6: if(93u != (unsigned int)(params[i]))
	    {mismatch(i,"unsigned int"); return 0; };
	    break;
        case 7: if(789.0f != *(float*)(&params[i]))
	    {mismatch(i,"float"); return 0; };
	    break;
        case 8: {/*double*/
            double x = *(double*)&params[i];
            i++; /* takes two parameters */
            if(endianness == 1)
		byteswap8((unsigned char*)&x);
            if(12345678.12345678d != x) {
                mismatch(i,"double");
                return 0;
            }
            }; break;
        case 10: {/*signed long long*/
            signed long long x = *(signed long long*)&params[i];
            i++; /* takes two parameters */
            if(endianness == 1)
		byteswap8((unsigned char*)&x);
            if(-9223372036854775807L != x) {
                mismatch(i,"signed long long");
                return 0;
            }
            }; break;
        case 12: {/*unsigned long long*/
            unsigned long long x = *(unsigned long long*)&params[i];
            i++; /* takes two parameters */
            if(endianness == 1)
		byteswap8((unsigned char*)&x);
            if(18446744073709551615UL != x) {
                mismatch(i,"unsigned long long");
                return 0;
            }
            }; break;
        default:
            mismatch(i,"unexpected parameter");
            return 0;
            break;
        }
    }

#ifdef DEBUG
    {
	size_t i;
	fprintf(stderr,"endianness=%d nparams=%d params=\n",endianness,nparams);
	for(i=0;i<nparams;i++) {
	    fprintf(stderr,"[%d] %ud %d %f\n", (unsigned int)i, params[i],(signed int)params[i],*(float*)&params[i]);
	}
	fflush(stderr);
    }
#endif
}

static void
byteswap8(unsigned char* mem)
{
    unsigned char c;
    c = mem[0];
    mem[0] = mem[7];
    mem[7] = c;
    c = mem[1];
    mem[1] = mem[6];
    mem[6] = c;
    c = mem[2];
    mem[2] = mem[5];
    mem[5] = c;
    c = mem[3];
    mem[3] = mem[4];
    mem[4] = c;
}

static void
mismatch(size_t i, const char* which)
{
    fprintf(stderr,"mismatch: [%ld] %s\n",i,which);
    fflush(stderr);
}
