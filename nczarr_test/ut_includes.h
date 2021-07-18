/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef UT_INCLUDES_H
#define UT_INCLUDES_H

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "zmap.h"

#include "netcdf.h"
#include "nclist.h"
#include "ncbytes.h"
#include "ncuri.h"
#include "ncpathmgr.h"

#include "zincludes.h"
#include "ut_test.h"
#include "ut_util.h"

extern struct ZUTEST zutester;

#endif /*UT_INCLUDES_H*/
