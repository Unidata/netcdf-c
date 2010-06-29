/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/* $Id: stub3.c,v 1.4 2010/05/24 19:48:16 dmh Exp $ */
/* $Header: /upc/share/CVS/netcdf-3/libsrc/stub3.c,v 1.4 2010/05/24 19:48:16 dmh Exp $ */

#include "config.h"
#include "netcdf.h"

/* Need to re-direct certain functions that are
   duplicated in netcdf4
*/

extern int NC3_initialize(void);

int
NC_initialize(void)
{
    NC3_initialize();    
    return NC_NOERR;
}
