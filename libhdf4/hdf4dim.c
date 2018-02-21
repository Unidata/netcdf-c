/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file
 * @internal This file handles the HDF4 dimension functions.
 *
 * @author Ed Hartnett
 */

#include "nc4internal.h"
#include "nc4dispatch.h"

/**
 * @internal Dims cannot be defined for HDF4 files.
 *
 * @param ncid Ignored.
 * @param name Ignored.
 * @param len Ignored.
 * @param idp Ignored.
 *
 * @return ::NC_EPERM Can't define dims.
 * @author Ed Hartnett
 */
int
HDF4_def_dim(int ncid, const char *name, size_t len, int *idp)
{
   return NC_EPERM;
}

/**
 * @internal Not allowed for HDF4.
 *
 * @param ncid Ignored.
 * @param dimid Ignored.
 * @param name Ignored.
 *
 * @return ::NC_NEPERM Can't write to HDF4 file.
 * @author Ed Hartnett
 */
int
HDF4_rename_dim(int ncid, int dimid, const char *name)
{
   return NC_EPERM;
}
