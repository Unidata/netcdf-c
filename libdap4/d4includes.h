/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef D4INCLUDES_H
#define D4INCLUDES_H 1

#include "config.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <curl/curl.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_GETRLIMIT
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/time.h>
#  endif
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/resource.h>
#  endif
#endif

#include "nc.h"
#include "netcdf.h"
#include "ncbytes.h"
#include "nclist.h"
#include "ncuri.h"
#include "nclog.h"
#include "ncdap.h"

#include "d4bytes.h"
#include "d4util.h"

#include "ncd4types.h"
#include "ncd4.h"

#include "d4debug.h"
#include "d4chunk.h"

#endif /*D4INCLUDES_H*/

