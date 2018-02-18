/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file @internal This file handles the variable functions for the
 * HDF4 dispatch layer.
 *
 * @author Ed Hartnett
 */

#include <nc4internal.h>
#include "hdf4dispatch.h"
#include "nc4dispatch.h"
#include <mfhdf.h>

/**
 * @internal Get data from an HDF4 SD dataset.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param startp Array of start indicies.
 * @param countp Array of counts.
 * @param mem_nc_type The type of these data after it is read into memory.
 * @param is_long Ignored for HDF4.
 * @param data pointer that gets the data.
 * @returns ::NC_NOERR for success
 * @author Ed Hartnett
 */
static int
nc4_get_hdf4_vara(NC *nc, int ncid, int varid, const size_t *startp,
                  const size_t *countp, nc_type mem_nc_type, int is_long, void *data)
{
   NC_GRP_INFO_T *grp;
   NC_VAR_HDF4_INFO_T *hdf4_var;   
   NC_VAR_INFO_T *var;
   int32 start32[NC_MAX_VAR_DIMS], edge32[NC_MAX_VAR_DIMS];
   int retval, d;

   /* Check inputs. */
   assert(nc);

   /* Find our metadata for this file, group, and var. */
   if ((retval = nc4_find_g_var_nc(nc, ncid, varid, &grp, &var)))
      return retval;
   assert(grp && var && var->name && var->format_var_info);

   /* Get the HDF4 specific var metadata. */
   hdf4_var = (NC_VAR_HDF4_INFO_T *)var->format_var_info;

   /* Convert starts/edges to the int32 type HDF4 wants. */
   for (d = 0; d < var->ndims; d++)
   {
      start32[d] = startp[d];
      edge32[d] = countp[d];
   }

   /* Read the data with HDF4. */
   if (SDreaddata(hdf4_var->sdsid, start32, NULL, edge32, data))
      return NC_EHDFERR;

   return NC_NOERR;
}

/**
 * Read an array of values. This is called by nc_get_vara() for
 * netCDF-4 files, as well as all the other nc_get_vara_*
 * functions. HDF4 files are handled as a special case.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param startp Array of start indicies.
 * @param countp Array of counts.
 * @param ip pointer that gets the data.
 * @param memtype The type of these data after it is read into memory.

 * @returns ::NC_NOERR for success
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
HDF4_get_vara(int ncid, int varid, const size_t *startp,
              const size_t *countp, void *ip, int memtype)
{
   NC *nc;
   NC_HDF5_FILE_INFO_T* h5;

   LOG((2, "%s: ncid 0x%x varid %d memtype %d", __func__, ncid, varid,
        memtype));

   if (!(nc = nc4_find_nc_file(ncid, &h5)))
      return NC_EBADID;

   /* Handle HDF4 cases. */
   return nc4_get_hdf4_vara(nc, ncid, varid, startp, countp, memtype,
                            0, (void *)ip);
}
