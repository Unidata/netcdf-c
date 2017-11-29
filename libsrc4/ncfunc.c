/*
  Copyright 2003, University Corporation for Atmospheric Research. See
  netcdf-4/docs/COPYRIGHT file for copying and redistribution
  conditions.

  This file is part of netcdf-4, a netCDF-like interface for HDF5, or a
  HDF5 backend for netCDF, depending on your point of view.

  This file handles the (useless) *_base_pe() functions, and the
  inq_format functions.

  @author Ed Hartnett, Dennis Heimbigner

*/

#include "nc4internal.h"
#include "nc4dispatch.h"

/**
 * This function only does anything for netcdf-3 files.
 *
 * \param ncid File ID (ignored).
 * \param pe Processor element (ignored).
 *
 * \returns ::NC_ENOTNC3 Not a netCDF classic format file.
 * \internal
 * \author Ed Hartnett
 */
int
NC4_set_base_pe(int ncid, int pe)
{
   return NC_ENOTNC3;
}

/**
 * This function only does anything for netcdf-3 files.
 *
 * \param ncid File ID (ignored).
 * \param pe Pointer to processor element. Ignored if NULL. Gets a 0
 * if present.
 *
 * \returns ::NC_ENOTNC3 Not a netCDF classic format file.
 * \internal
 * \author Ed Hartnett
 */
int
NC4_inq_base_pe(int ncid, int *pe)
{
   return NC_ENOTNC3;
}

/**
 * Get the format (i.e. NC_FORMAT_NETCDF4 pr
 * NC_FORMAT_NETCDF4_CLASSIC) of an open netCDF-4 file.
 *
 * \param ncid File ID (ignored).
 * \param formatp Pointer that gets the constant indicating format.

 * \returns ::NC_NOERR No error.
 * \returns ::NC_EBADID Bad ncid.
 * \internal
 * \author Ed Hartnett
 */
int
NC4_inq_format(int ncid, int *formatp)
{
   NC *nc;
   NC_HDF5_FILE_INFO_T* nc4_info;

   LOG((2, "nc_inq_format: ncid 0x%x", ncid));

   if (!formatp)
      return NC_NOERR;

   /* Find the file metadata. */
   if (!(nc = nc4_find_nc_file(ncid,&nc4_info)))
      return NC_EBADID;

   /* Otherwise, this is a netcdf-4 file. Check if classic NC3 rules
    * are in effect for this file. */
   if (nc4_info->cmode & NC_CLASSIC_MODEL)
      *formatp = NC_FORMAT_NETCDF4_CLASSIC;
   else
      *formatp = NC_FORMAT_NETCDF4;

   return NC_NOERR;
}

/**
 * Return the extended format (i.e. the dispatch model), plus the mode
 * associated with an open file.
 *
 * \param ncid File ID (ignored).
 * \param formatp a pointer that gets the extended format. Note that
 * this is not the same as the format provided by nc_inq_format(). The
 * extended foramt indicates the dispatch layer model. NetCDF-4 files
 * will always get NC_FORMATX_NC4 for netCDF files, NC_FORMATX_HDF4
 * for HDF4 files.
 * \param modep a pointer that gets the open/create mode associated with
 * this file. Ignored if NULL.

 * \returns ::NC_NOERR No error.
 * \returns ::NC_EBADID Bad ncid.
 * \internal
 * \author Dennis Heimbigner
 */
int
NC4_inq_format_extended(int ncid, int *formatp, int *modep)
{
   NC *nc;
   NC_HDF5_FILE_INFO_T* h5;

   LOG((2, "nc_inq_format_extended: ncid 0x%x", ncid));

   /* Find the file metadata. */
   if (!(nc = nc4_find_nc_file(ncid,&h5)))
      return NC_EBADID;

   if(modep) *modep = (nc->mode|NC_NETCDF4);

   if(formatp) {
#ifdef USE_HDF4
      /* Distinguish HDF5 from HDF4 */
      *formatp = (h5->hdf4 ? NC_FORMATX_NC_HDF4 : NC_FORMATX_NC_HDF5);
#else /* USE_HDF4 */
      *formatp = NC_FORMATX_NC_HDF5;
#endif /* USE_HDF4 */
   }
   return NC_NOERR;
}
