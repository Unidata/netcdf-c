/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef _NCD4DISPATCH_H
#define _NCD4DISPATCH_H

#include <stddef.h> /* size_t, ptrdiff_t */
#include "netcdf.h"
#include "ncdispatch.h"

#if defined(__cplusplus)
extern "C" {
#endif

EXTERNL int
NCD4_open(const char *path, int mode,
         int basepe, size_t *chunksizehintp,
         int use_parallel, void* mpidata,
         struct NC_Dispatch* dispatch, NC* ncp);

EXTERNL int
NCD4_close(int ncid);

EXTERNL int
NCD4_abort(int ncid);

extern int NCD4_initialize(void);

#if defined(__cplusplus)
}
#endif

#endif /*_NCD4DISPATCH_H*/
