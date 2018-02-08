/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file
 * @internal This file handles the HDF4 attribute functions.
 *
 * @author Ed Hartnett
 */

#include "nc4internal.h"
#include "nc.h"
#include "hdf4dispatch.h"
#include "ncdispatch.h"

int nc4typelen(nc_type type);

/**
 * @internal Not allowed for HDF4.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param name Ignored.
 * @param newname Ignored.
 *
 * @return ::NC_EPERM Not allowed for HDF4.
 * @author Ed Hartnett
 */
int
HDF4_rename_att(int ncid, int varid, const char *name, const char *newname)
{
   return NC_EPERM;
}

/**
 * @internal Not allowed for HDF4.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param name Ignored.
 *
 * @return ::NC_EPERM Not allowed with HDF4.
 * @author Ed Hartnett
 */
int
HDF4_del_att(int ncid, int varid, const char *name)
{
   return NC_EPERM;
}

/**
 * @internal Not allowed for HDF4.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param name Ignored.
 * @param file_type Ignored.
 * @param len Ignored.
 * @param data Ignored.
 * @param memtype Ignored.
 *
 * @return ::NC_EPERM Not allowed with HDF4.
 * @author Ed Hartnett
 */
int
HDF4_put_att(int ncid, int varid, const char *name, nc_type file_type,
             size_t len, const void *data, nc_type mem_type)
{
   return NC_EPERM;
}
