/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/**
 * @file This header file contains the common set of headers
 * in proper order.
 * This header should not be included in
 * code outside libzarr.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#ifndef ZINCLUDES_H
#define ZINCLUDES_H

#include "config.h"

#ifndef __cplusplus
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h> /* size_t, ptrdiff_t */
#include <string.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif
#ifdef _WIN32
#include <malloc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nc4dispatch.h"
#include "ncuri.h"
#include "nclist.h"
#include "ncbytes.h"
#include "ncauth.h"
#include "nclog.h"
#include "ncs3sdk.h"
#include "ncindex.h"
#include "ncjson.h"
#include "ncproplist.h"
#include "ncutil.h"

#include "zmap.h"
#include "zmetadata.h"
#include "zinternal.h"
#include "zfilter.h"
#include "zformat.h"
#include "zdispatch.h"
#include "zprovenance.h"
#include "zodom.h"
#include "zchunking.h"
#include "zcache.h"
#include "zarr.h"
#include "zdebug.h"
#include "zutil.h"

#ifdef __cplusplus
}
#endif

#define ncz_find_default_chunksizes2 nc4_find_default_chunksizes2
#endif /* ZINCLUDES_H */


