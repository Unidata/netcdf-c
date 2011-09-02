/** \file \internal
Internal netcdf-4 functions.

This file contains functions internal to the netcdf4 library. None of
the functions in this file are exposed in the exetnal API. These
functions all relate to the manipulation of netcdf-4's in-memory
buffer of metadata information, i.e. the linked list of NC_FILE_INFO_T
structs.

Copyright 2003-2011, University Corporation for Atmospheric
Research. See the COPYRIGHT file for copying and redistribution
conditions.
*/

#include "config.h"
#include "nc4internal.h"
#include "nc.h" /* from libsrc */
#include "ncdispatch.h" /* from libdispatch */
#include <utf8proc.h>

/** Show the in-memory metadata for a netcdf file. */
int
NCD_show_metadata(int ncid)
{
   int retval = NC_NOERR;
#ifdef LOGGING
   NC_FILE_INFO_T *nc;
   int old_log_level = nc_log_level;
   
   /* Find file metadata. */
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Log level must be 2 to see metadata. */
   nc_log_level = 2;
   retval = log_metadata_nc(nc);
   nc_log_level = old_log_level;
#endif /*LOGGING*/
   return retval;
}

int
NCD_new_nc(NC** ncpp)
{
    NC_FILE_INFO_T** ncp;
    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(NC_FILE_INFO_T)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}

