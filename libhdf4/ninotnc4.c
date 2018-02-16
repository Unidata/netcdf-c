/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file
 * @internal This file contains functions that return NC_ENOTNC4, for
 * dispatch layers that only implement the classic model.
 *
 * @author Ed Hartnett
 */

#include "nc4internal.h"
#include "nc.h"
#include "hdf4dispatch.h"
#include "ncdispatch.h"

/**
 * @internal Not allowed for classic model.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param id Ignored.
 * @param nparams Ignored.
 * @param parms Ignored.
 *
 * @return ::NC_ENOTNC4 Not allowed for classic model.
 * @author Ed Hartnett
 */
int
NONC4_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams,
                     const unsigned int* parms)
{
   return NC_ENOTNC4;
}


