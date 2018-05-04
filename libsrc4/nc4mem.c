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

extern int NC4_image_init(NC_HDF5_FILE_INFO_T* h5);

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
    hdfid = NC4_image_init(h5);
    if(hdfid < 0)
	{stat = NC_EHDFERR; goto done;}
    /* Return file identifier */ 
    h5->hdfid = hdfid;

done:
    return stat;
}

int
NC4_create_image_file(NC_HDF5_FILE_INFO_T* h5, size_t initialsz)
{
    int stat = NC_NOERR;
    int hdfid;

    /* Create the file but using our version of H5LTopen_file_image */
    h5->mem.created = 1;
    h5->mem.initialsize = initialsz;
    hdfid = NC4_image_init(h5);
    if(hdfid < 0)
	{stat = NC_EHDFERR; goto done;}
    /* Return file identifier */ 
    h5->hdfid = hdfid;
done:
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
    fapl = h5->mem.fapl;
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
done:
    return stat;
}
