/*
  This is part of ncgen, a utility which is part of netCDF. Copyright
  2010, UCAR/Unidata. See COPYRIGHT file for copying and
  redistribution conditions.
 */

/* $Id: includes.h,v 1.7 2010/05/25 18:57:00 dmh Exp $ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/includes.h,v 1.7 2010/05/25 18:57:00 dmh Exp $ */

#ifndef NCGEN_INCLUDES_H
#define NCGEN_INCLUDES_H

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>	/* for isprint() */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef __SunOS
#include <strings.h>
#endif

#ifdef __hpux
#include <locale.h>
#endif

#include "list.h"
#include "bytebuffer.h"
#include "nctime.h"

/* Local Configuration flags*/
#define ENABLE_BINARY
#define ENABLE_C
#define ENABLE_F77
#define ENABLE_JAVA

#include "netcdf.h"
#include "netcdf_filter.h"
#include "nc4internal.h"
#include "ncoffsets.h"
#include "netcdf_aux.h"
#include "data.h"
#include "ncgen.h"
#include "generate.h"
#include "genlib.h"
#include "util.h"
#include "debug.h"
#include "nc.h"
#include "netcdf_aux.h"

extern int specialconstants;

#undef ITERBUG
#undef CHARBUG

#undef nullfree
#define nullfree(x) {if((x)) free(x);}

#endif /* NCGEN_INCLUDES_H */
