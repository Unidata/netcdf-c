/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"
#include "ncdispatch.h"
#include "nc.h"
#include "nccr.h"
#include "nccrdispatch.h"

int 
NCCR_get_vara(int ncid, int varid,
	      const size_t* startp,
	      const size_t* countp,
	      void* data,
	      nc_type externaltype0)
{
    return NC_NOERR;
}
