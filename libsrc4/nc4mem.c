/*********************************************************************
*    Copyright 2018, UCAR/Unidata
*    See netcdf/COPYRIGHT file for copying and redistribution conditions.
* ********************************************************************/

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

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <hdf5.h>
#include <H5DSpublic.h> /* must be after nc4internal.h */
#include <H5Fpublic.h>
#include <H5LTpublic.h>

#include "netcdf.h"
#include "nc4internal.h"

#undef DEBUG

#ifndef HDrealloc
    #define HDrealloc(M,Z)    realloc(M,Z)
#endif /* HDrealloc */

extern int NC4_open_image(NC_HDF5_FILE_INFO_T* h5);

#if 0
/* Forward */
/* not working */
static int NC4_set_realloc_callback(NC_HDF5_FILE_INFO_T*, hid_t);
#endif

int
NC4_open_image_file(NC_HDF5_FILE_INFO_T* h5)
{
    int stat = NC_NOERR;
    hid_t hdfid;

    /* check arguments */
    if(h5->mem.memio.memory == NULL || h5->mem.memio.size == 0)
	{stat = NC_EINVAL; goto done;}

    /* Figure out the flags */
    h5->mem.flags = 0;
    if(h5->mem.locked) {
	h5->mem.flags |= (H5LT_FILE_IMAGE_DONT_COPY | H5LT_FILE_IMAGE_DONT_RELEASE);
    }
    if(!h5->no_write)
	h5->mem.flags |= H5LT_FILE_IMAGE_OPEN_RW;

    /* Create the file but using our version of H5LTopen_file_image */
    hdfid = NC4_open_image(h5);
#if 0
    hdfid = H5LTopen_file_image(h5->mem.memio.memory,h5->mem.memio.size,h5->mem.flags);
#endif
    if(hdfid < 0)
	{stat = NC_EHDFERR; goto done;}

#if 0
    /* Override the realloc callback if necessary */
    if(h5->mem.flags & H5LT_FILE_IMAGE_DONT_COPY) {
	if((stat = NC4_set_realloc_callback(h5,hdfid)) != NC_NOERR)
	    goto done;
    }
#endif

    /* Return file identifier */ 
    h5->hdfid = hdfid;

done:
    return stat;
}

int
NC4_create_image_file(NC_HDF5_FILE_INFO_T* h5)
{
    int stat = NC_NOERR;
    NC* nc = h5->controller;
    hid_t		fcpl, fapl, file_id;	/* HDF5 identifiers */
    size_t              alloc_incr;     /* Buffer allocation increment */
    size_t              min_incr = 65536; /* Minimum buffer increment */
#if 0
    double              buf_prcnt = 0.1f;  /* Percentage of buffer size to set
                                             as increment */
#endif

#if 0
see if we can get by without creating an initial memory for the core driver
    /* Assume that the default buffer size and memory were set by caller */
    if(h5->mem.memio.memory == NULL || h5->memio.size == 0)
	{stat = NC_EINVAL; goto done;}

    /* set allocation increment to a percentage of the supplied buffer mem, or
     * a pre-defined minimum increment value, whichever is larger
     */
    if ((buf_prcnt * h5->memio.size) > min_incr)
        alloc_incr = (size_t)(buf_prcnt * h5->memio.size);
    else
        alloc_incr = min_incr;

#else
    alloc_incr = min_incr;
#endif

    /* Create FAPL to establish core driver */
    if ((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0) 
        {stat = NC_EHDFERR; goto done;}

    /* Make fcpl be default for now */
    fcpl = H5P_DEFAULT;

    /* Configure FAPL to use the core file driver with no backing store */
    if (H5Pset_fapl_core(fapl, alloc_incr, 0) < 0) 
        {stat = NC_EHDFERR; goto done;}

#if 0
    /* Assign file image in user buffer to FAPL */
    if(H5Pset_file_image(fapl, h5->memio.memory, h5->memio.size) < 0) 
        {stat = NC_EHDFERR; goto done;}
#endif

    /* Create the file */
    if((file_id = H5Fcreate(nc->path, H5F_ACC_TRUNC, fcpl, fapl)) < 0) 
        {stat = NC_EHDFERR; goto done;}

    /* Return file identifier */ 
    h5->hdfid = file_id;
    h5->mem.fapl = fapl;

done:
    if(stat != NC_NOERR) {
        /* Close FAPL */
        H5Pclose(fapl);
    }
    return stat;
}

int
NC4_extract_file_image(NC_HDF5_FILE_INFO_T* h5)
{
    int stat = NC_NOERR;
    hid_t fapl;
    herr_t herr;
    NC_memio mem;

    assert(h5 && h5->hdfid && !h5->no_write);

    /* Get the file access property list */
#if 0
    fapl = H5Fget_access_plist(h5->hdfid);
#else
    fapl = h5->mem.fapl;
#endif
    if(fapl < 0)
	{stat = NC_EHDFERR; goto done;}

    memset(&mem,0,sizeof(mem));
    herr = H5Pget_file_image(fapl, &mem.memory, &mem.size);
    if(herr < 0)
	{stat = NC_EHDFERR; goto done;}
    h5->mem.memio = mem;

    /* Close FAPL */
    if (H5Pclose(fapl) < 0) 
	{stat = NC_EHDFERR; goto done;}        
    h5->mem.fapl = 0;

#if 0
    /* Pass 1: get the memory chunk size */
    size = H5Fget_file_image(hdfid, NULL, 0);
    if(size < 0)
        {stat = NC_EHDFERR; goto done;}
    /* Create the space to hold image */
    h5->mem.memio.size = (size_t)size;
    h5->mem.memio.memory = malloc(h5->mem.memio.size);
    if(h5->mem.memio.memory == NULL)
        {stat = NC_ENOMEM; goto done;}
    /* Pass 2: get the memory chunk itself */
    size = H5Fget_file_image(hdfid, h5->mem.memio.memory, h5->mem.memio.size);
    if(size < 0)
        {stat = NC_EHDFERR; goto done;}
#endif

done:
    return stat;
}

/*-------------------------------------------------------------------------
* Function: image_realloc 
*
* Purpose: Reallocates the shared application image buffer and updates data
*          structures that manage buffer "copying". 
*           
* Return: Address of reallocated buffer, if successful. Otherwise, it returns
*         NULL.  
*
* Programmer: Christian Chilan 
*
* Date: October 3, 2011
*
*-------------------------------------------------------------------------
* Modified from code in H5LT.c.
*/

/**
Type H5LT_file_image_ud_t is taken from H5LT.c.
*/

#if 0
/* Data structure to pass application data to callbacks. */ 
typedef struct {
    void *app_image_ptr;	/* Pointer to application buffer */ 
    size_t app_image_size;	/* Size of application buffer */
    void *fapl_image_ptr;	/* Pointer to FAPL buffer */
    size_t fapl_image_size;	/* Size of FAPL buffer */
    int fapl_ref_count;		/* Reference counter for FAPL buffer */
    void *vfd_image_ptr;	/* Pointer to VFD buffer */
    size_t vfd_image_size;	/* Size of VFD buffer */
    int vfd_ref_count;		/* Reference counter for VFD buffer */
    unsigned flags;		/* Flags indicate how the file image will */
                                /* be open */
    int ref_count;		/* Reference counter on udata struct */
} H5LT_file_image_ud_t;

static void *
local_image_realloc(void *ptr, size_t size, H5FD_file_image_op_t file_image_op, void *_udata)
{
    H5LT_file_image_ud_t *udata = (H5LT_file_image_ud_t *)_udata;
    void * return_value = NULL;

    /* callback is only used if the application buffer is not actually copied */
    if (!(udata->flags & H5LT_FILE_IMAGE_DONT_COPY)) 
        goto out;

    /* realloc() is not allowed if the image is open in read-only mode */
    if (!(udata->flags & H5LT_FILE_IMAGE_OPEN_RW)) 
        goto out; 

    if (file_image_op == H5FD_FILE_IMAGE_OP_FILE_RESIZE) {
        if (udata->vfd_image_ptr != ptr) 
            goto out; 

        if (udata->vfd_ref_count != 1) 
            goto out;

	/* Modified: 
           If H5LT_FILE_IMAGE_DONT_RELEASE flag is set: two cases
           1. If the realloc new size is <= existing size,
	      then pretend we did a realloc and return success.
           2. Otherwise, realloc() is not allowed because reallocation
              may change the address of the buffer. The
              new address cannot be communicated to the application
              to release it.
        */
        if (udata->flags & H5LT_FILE_IMAGE_DONT_RELEASE) {
	    if(size > udata->vfd_image_size)
                goto out; 
        } else {
	    if (NULL == (udata->vfd_image_ptr = HDrealloc(ptr, size)))
                goto out; 
            udata->vfd_image_size = size;
        }
        return_value = udata->vfd_image_ptr;
    } /* end if */
    else
        goto out;

    return(return_value);

out:
    return NULL;
} /* end local_image_realloc() */

static int
NC4_set_realloc_callback(NC_HDF5_FILE_INFO_T* h5, hid_t hdfid)
{
    int stat = NC_NOERR;
    hid_t fapl;
    herr_t hstat;
    H5FD_file_image_callbacks_t callbacks;
   
    /* Get the file access property list */
    fapl = H5Fget_access_plist(hdfid);
    if(fapl < 0)
	{stat = NC_EHDFERR; goto done;}

    /* Now get the callbacks */
    memset(&callbacks,0,sizeof(callbacks));
/* For some reason, this is not returning the callbacks */
    hstat = H5Pget_file_image_callbacks(fapl, &callbacks);
    if(hstat < 0)
	{stat = NC_EHDFERR; goto done;}

    /* Replace the realloc callback */
    callbacks.image_realloc = local_image_realloc;

    /* Now reset the callbacks */
    hstat = H5Pset_file_image_callbacks(fapl, &callbacks);
    if(hstat < 0)
	{stat = NC_EHDFERR; goto done;}

done:
    return stat;
}
#endif
