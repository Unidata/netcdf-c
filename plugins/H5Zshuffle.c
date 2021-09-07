/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "netcdf_filter_build.h"

#ifndef H5Z_FILTER_SHUFFLE
#define H5Z_FILTER_SHUFFLE      2
#endif

#ifndef H5Z_SHUFFLE_TOTAL_NPARMS
#define H5Z_SHUFFLE_TOTAL_NPARMS   1
#endif
#ifndef H5Z_SHUFFLE_USER_NPARMS
#define H5Z_SHUFFLE_USER_NPARMS   0
#endif

/* Local function prototypes */
#ifdef USE_HDF5
static herr_t H5Z__set_local_shuffle(hid_t dcpl_id, hid_t type_id, hid_t space_id);
#else
#define H5Z__set_local_shuffle NULL
#endif

static size_t H5Z__filter_shuffle(unsigned flags, size_t cd_nelmts,
    const unsigned cd_values[], size_t nbytes, size_t *buf_size, void **buf);

/* This message derives from H5Z */
const H5Z_class2_t H5Z_SHUFFLE[1] = {{
    H5Z_CLASS_T_VERS,           /* H5Z_class_t version */
    H5Z_FILTER_SHUFFLE,		/* Filter id number		*/
    1,                          /* encoder_present flag (set to true) */
    1,                          /* decoder_present flag (set to true) */
    "shuffle",			/* Filter name for debugging	*/
    NULL,                       /* The "can apply" callback     */
    H5Z__set_local_shuffle,     /* The "set local" callback     */
    H5Z__filter_shuffle,	/* The actual filter function	*/
}};

/* External Discovery Functions */
DLLEXPORT
H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

DLLEXPORT
const void*
H5PLget_plugin_info(void)
{
    return H5Z_SHUFFLE;
}

/* Local macros */
#define H5Z_SHUFFLE_PARM_SIZE      0       /* "Local" parameter for shuffling size */

#ifdef USE_HDF5
/*-------------------------------------------------------------------------
 * Function:	H5Z__set_local_shuffle
 *
 * Purpose:	Set the "local" dataset parameter for data shuffling to be
 *              the size of the datatype.
 *
 * Return:	Success: Non-negative
 *		Failure: Negative
 *
 * Programmer:	Quincey Koziol
 *              Monday, April  7, 2003
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5Z__set_local_shuffle(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    unsigned flags = 0;                     /* Filter flags */
    unsigned cd_values[H5Z_SHUFFLE_TOTAL_NPARMS];  /* Filter parameters */
    herr_t ret_value = 1;         /* Return value */

    /* Set "local" parameter for this dataset */
    if((cd_values[0] = (unsigned)H5Tget_size(type_id)) == 0)
	HGOTO_ERROR(H5E_PLINE, H5E_BADTYPE, FAIL, "bad datatype size")

    /* Modify the filter's parameters for this dataset */
    if(H5Pmodify_filter(dcpl_id, H5Z_FILTER_SHUFFLE, flags, (size_t)H5Z_SHUFFLE_TOTAL_NPARMS, cd_values) < 0)
	HGOTO_ERROR(H5E_PLINE, H5E_CANTSET, FAIL, "can't set local shuffle parameters")

done:
    return ret_value;
} /* end H5Z__set_local_shuffle() */
#endif


/*-------------------------------------------------------------------------
 * Function:	H5Z__filter_shuffle
 *
 * Purpose:	Implement an I/O filter which "de-interlaces" a block of data
 *              by putting all the bytes in a byte-position for each element
 *              together in the block.  For example, for 4-byte elements stored
 *              as: 012301230123, shuffling will store them as: 000111222333
 *              Usually, the bytes in each byte position are more related to
 *              each other and putting them together will increase compression.
 *
 * Return:	Success: Size of buffer filtered
 *		Failure: 0
 *
 * Programmer:	Kent Yang
 *              Wednesday, November 13, 2002
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5Z__filter_shuffle(unsigned flags, size_t cd_nelmts, const unsigned cd_values[],
                   size_t nbytes, size_t *buf_size, void **buf)
{
    void *dest = NULL;          /* Buffer to deposit [un]shuffled bytes into */
    unsigned char *_src=NULL;   /* Alias for source buffer */
    unsigned char *_dest=NULL;  /* Alias for destination buffer */
    unsigned bytesoftype;       /* Number of bytes per element */
    size_t numofelements;       /* Number of elements in buffer */
    size_t i;                   /* Local index variables */
#ifdef NO_DUFFS_DEVICE
    size_t j;                   /* Local index variable */
#endif /* NO_DUFFS_DEVICE */
    size_t leftover;            /* Extra bytes at end of buffer */
    size_t ret_value = 0;       /* Return value */

    /* Check arguments */
    if (cd_nelmts!=H5Z_SHUFFLE_TOTAL_NPARMS || cd_values[H5Z_SHUFFLE_PARM_SIZE]==0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "invalid shuffle parameters")

    /* Get the number of bytes per element from the parameter block */
    bytesoftype=cd_values[H5Z_SHUFFLE_PARM_SIZE];

    /* Compute the number of elements in buffer */
    numofelements=nbytes/bytesoftype;

    /* Don't do anything for 1-byte elements, or "fractional" elements */
    if(bytesoftype > 1 && numofelements > 1) {
        /* Compute the leftover bytes if there are any */
        leftover = nbytes%bytesoftype;

        /* Allocate the destination buffer */
        if (NULL==(dest = H5MM_malloc(nbytes)))
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, 0, "memory allocation failed for shuffle buffer")

        if(flags & H5Z_FLAG_REVERSE) {
            /* Get the pointer to the source buffer */
            _src =(unsigned char *)(*buf);

            /* Input; unshuffle */
            for(i=0; i<bytesoftype; i++) {
                _dest=((unsigned char *)dest)+i;
#define DUFF_GUTS							    \
    *_dest=*_src++;                             \
    _dest+=bytesoftype;
#ifdef NO_DUFFS_DEVICE
                j = numofelements;
                while(j > 0) {
                    DUFF_GUTS;

                    j--;
                } /* end for */
#else /* NO_DUFFS_DEVICE */
            {
                size_t duffs_index; /* Counting index for Duff's device */

                duffs_index = (numofelements + 7) / 8;
                switch (numofelements % 8) {
                    default:
                        HDassert(0 && "This Should never be executed!");
                        break;
                    case 0:
                        do
                          {
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 7:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 6:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 5:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 4:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 3:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 2:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 1:
                            DUFF_GUTS
                      } while (--duffs_index > 0);
                } /* end switch */
            }
#endif /* NO_DUFFS_DEVICE */
#undef DUFF_GUTS
            } /* end for */

            /* Add leftover to the end of data */
            if(leftover>0) {
                /* Adjust back to end of shuffled bytes */
                _dest -= (bytesoftype - 1);     /*lint !e794 _dest is initialized */
                H5MM_memcpy((void*)_dest, (void*)_src, leftover);
            }
        } /* end if */
        else {
            /* Get the pointer to the destination buffer */
            _dest =(unsigned char *)dest;

            /* Output; shuffle */
            for(i=0; i<bytesoftype; i++) {
                _src=((unsigned char *)(*buf))+i;
#define DUFF_GUTS							    \
    *_dest++=*_src;                             \
    _src+=bytesoftype;
#ifdef NO_DUFFS_DEVICE
                j = numofelements;
                while(j > 0) {
                    DUFF_GUTS;

                    j--;
                } /* end for */
#else /* NO_DUFFS_DEVICE */
            {
                size_t duffs_index; /* Counting index for Duff's device */

                duffs_index = (numofelements + 7) / 8;
                switch (numofelements % 8) {
                    default:
                        HDassert(0 && "This Should never be executed!");
                        break;
                    case 0:
                        do
                          {
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 7:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 6:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 5:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 4:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 3:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 2:
                            DUFF_GUTS
                            H5_ATTR_FALLTHROUGH
                    case 1:
                            DUFF_GUTS
                      } while (--duffs_index > 0);
                } /* end switch */
            }
#endif /* NO_DUFFS_DEVICE */
#undef DUFF_GUTS
            } /* end for */

            /* Add leftover to the end of data */
            if(leftover>0) {
                /* Adjust back to end of shuffled bytes */
                _src -= (bytesoftype - 1);      /*lint !e794 _src is initialized */
                H5MM_memcpy((void*)_dest, (void*)_src, leftover);
            }
        } /* end else */

        /* Free the input buffer */
        H5MM_xfree(*buf);

        /* Set the buffer information to return */
        *buf = dest;
        *buf_size=nbytes;
    } /* end else */

    /* Set the return value */
    ret_value = nbytes;

done:
    return ret_value;
}

