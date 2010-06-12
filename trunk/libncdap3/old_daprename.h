/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/old_daprename.h,v 1.1 2010/05/29 18:59:58 dmh Exp $
 *********************************************************************/

#ifndef DAPRENAME_H
#define DAPRENAME_H

/*
Define rename criteris
NETCDF4=no  => use netcdf.h
NETCDF4=yes => use netcdf3.h then nctonc3.h + RENAME
where
netcdf3l.h is netcdf3 with nc3_* converted to lnc3_*
nctonc3.h converts nc_* to nc3_

See also libsrc/rename.h
*/

#ifdef USE_NETCDF4
# include "netcdf3l.h"
# define RENAMEDAP(name) l3nc_##name
# define RENAME3(name) nc3_##name
#else
# include "netcdf3l.h"
# define RENAMEDAP(name) l3nc_##name
# define RENAME3(name) nc_##name
#endif

#endif /*DAPRENAME_H*/
