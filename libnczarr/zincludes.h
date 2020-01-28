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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h> /* size_t, ptrdiff_t */

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nc4dispatch.h"
#include "ncuri.h"
#include "nclist.h"
#include "ncbytes.h"
#include "ncauth.h"
#include "nclog.h"
#include "ncrc.h"

#include "zinternal.h"
#include "zdispatch.h"
#include "zprovenance.h"
#include "zmap.h"
#include "zchunking.h"
#include "zjson.h"
#include "zarr.h"
#include "zdebug.h"

#endif /* ZINCLUDES_H */


