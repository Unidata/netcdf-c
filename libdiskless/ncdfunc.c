/* \file \internal

Some functions for diskless API.

Copyright 2011, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.  
*/

#include "nc4internal.h"
#include "ncddispatch.h"

/* This function only does anything for netcdf-3 files. */
int
NCD_set_base_pe(int ncid, int pe)
{
   NC_FILE_INFO_T *nc;
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;
   if (nc->nc4_info)
      return NC_ENOTNC3;
   return NC3_set_base_pe(nc->int_ncid,  pe);
}

/* This function only does anything for netcdf-3 files. */
int
NCD_inq_base_pe(int ncid, int *pe)
{
   NC_FILE_INFO_T *nc;
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;
   if (nc->nc4_info)
      return NC_ENOTNC3;
   return NC3_inq_base_pe(nc->int_ncid, pe);
}

/* Get the format (i.e. classic, 64-bit-offset, or netcdf-4) of an
 * open file. */
int
NCD_inq_format(int ncid, int *formatp)
{
   NC_FILE_INFO_T *nc;

   LOG((2, "nc_inq_format: ncid 0x%x", ncid));

   if (!formatp)
      return NC_NOERR;

   /* Find the file metadata. */
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* If this isn't a netcdf-4 file, pass this call on to the netcdf-3
    * library. */
   if (!nc->nc4_info)
      return NC3_inq_format(nc->int_ncid, formatp);
   
   /* Otherwise, this is a netcdf-4 file. Check if classic NC3 rules
    * are in effect for this file. */
   if (nc->nc4_info->cmode & NC_CLASSIC_MODEL)
      *formatp = NC_FORMAT_NETCDF4_CLASSIC;
   else
      *formatp = NC_FORMAT_NETCDF4;

   return NC_NOERR;
}


