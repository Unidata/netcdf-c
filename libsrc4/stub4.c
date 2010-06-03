/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/* $Id: stub4.c,v 1.6 2010/05/24 19:48:17 dmh Exp $ */
/* $Header: /upc/share/CVS/netcdf-3/libsrc4/stub4.c,v 1.6 2010/05/24 19:48:17 dmh Exp $ */

#include "config.h"
#include "netcdf.h"

extern int NC3_initialize(void);
extern int NC4_initialize(void);

int
NC_initialize(void)
{
    NC3_initialize();
    NC4_initialize();
    return NC_NOERR;
}
