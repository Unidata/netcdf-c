/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/

#include <stdlib.h>
#include "netcdf.h"

int
main(void)
{
   int ncid, typeid, varid;
   float missing_value = -999.0;
   nc_vlen_t missing_val;

   if(nc_create("tst_vlen_data.nc", NC_CLOBBER | NC_NETCDF4, &ncid)) abort();

   /* Create a vlen type. */
   if (nc_def_vlen(ncid, "row_of_floats", NC_FLOAT, &typeid)) abort();;

   /* Declare a scalar variable of the vlen type */
   if (nc_def_var(ncid, "ragged_array", typeid, 0, NULL, &varid)) abort();;

   /* Create and write a variable attribute of the vlen type */
   missing_val.p = &missing_value;
   missing_val.len = 1;
   if (nc_put_att(ncid, varid, "_FillValue", typeid, 1, (void *) &missing_val)) abort();;

   if (nc_close(ncid)) abort();;
   exit(0);
}
