#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <hdf5.h>
/* Older versions of the hdf library may define H5PL_type_t here */
#include <H5PLextern.h>


#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

#if NOOP_INSTANCE == 1
const static int instance1 = 1;
#endif
#if NOOP_INSTANCE == 0
const static int instance0 = 1;
#endif

/* use a temporary */
#define H5Z_FILTER_NOOP 40000

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

extern void NC_h5filterspec_fix8(void* mem, int decode);

static htri_t H5Z_noop_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static size_t H5Z_filter_noop(unsigned int, size_t, const unsigned int cd_values[], size_t, size_t*, void**);

static H5Z_class2_t H5Z_NOOP[1] = {{
    H5Z_CLASS_T_VERS,                /* H5Z_class_t version */
#if NOOP_INSTANCE == 0
    (H5Z_filter_t)(H5Z_FILTER_NOOP), /* Filter id number */
#else
    (H5Z_filter_t)(H5Z_FILTER_NOOP+1), /* Filter id number */
#endif
    1,                               /* encoder_present flag (set to true) */
    1,                               /* decoder_present flag (set to true) */
#if NOOP_INSTANCE == 0
    "noop",                          /*  Filter name for debugging    */
#else
    "noop1",                         /*  Filter name for debugging    */
#endif
    (H5Z_can_apply_func_t)H5Z_noop_can_apply, /* The "can apply" callback  */
    NULL,			     /* The "set local" callback  */
    (H5Z_func_t)H5Z_filter_noop,     /* The actual filter function   */
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
    return H5Z_NOOP;
}

/* Make this explicit */
/*
 * The "can_apply" callback returns positive a valid combination, zero for an
 * invalid combination and negative for an error.
 */
static htri_t
H5Z_noop_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    return 1; /* Assume it can always apply */
}

static size_t
H5Z_filter_noop(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf)
{
    void* newbuf;
    size_t i;    
    
#if NOOP_INSTANCE == 0
    NC_UNUSED(instance0);
#endif
#if NOOP_INSTANCE == 1
    NC_UNUSED(instance1);
#endif

    printf("cd_nelmts=%lu cd_values=",(unsigned long)cd_nelmts);
    for(i=0;i<cd_nelmts;i++)
	printf(" %u",cd_values[i]);
    printf("\n");

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
}
