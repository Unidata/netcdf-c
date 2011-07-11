/*! \file
Functions for inquiring about variables.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

#include "ncdispatch.h"

/** \name Learning about Variables

Functions to learn about the variables in a file. */
/*! \{ */ /* All these functions are part of this named group... */

/** 
\ingroup variables
Find the ID of a variable, from the name.

The function nc_inq_varid returns the ID of a netCDF variable, given
its name.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param name Name of the variable.

\param varidp Pointer to location for returned variable ID.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.

\section Example

Here is an example using nc_inq_varid to find out the ID of a variable
named rh in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status, ncid, rh_id;
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_inq_varid(int ncid, const char *name, int *varidp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_varid(ncid, name, varidp);
}

/** 
\ingroup variables
Learn about a variable.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param name Returned variable name. The caller must allocate space for
the returned name. The maximum length is \ref NC_MAX_NAME. Ignored if
NULL.

\param xtypep Pointer where typeid will be stored. Ignored if NULL.

\param ndimsp Pointer where number of dimensions will be
stored. Ignored if NULL.

\param dimidsp Pointer where array of dimension IDs will be
stored. Ignored if NULL.

\param nattsp Pointer where number of attributes will be
stored. Ignored if NULL.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.
\returns \ref NC_ENOTVAR Invalid variable ID.

\section Example

Here is an example using nc_inq_var() to find out about a variable named
rh in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status                     
     int  ncid;                      
     int  rh_id;                     
     nc_type rh_type;                
     int rh_ndims;                   
     int  rh_dimids[NC_MAX_VAR_DIMS];
     int rh_natts 
        ...
     status = nc_open ("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_var (ncid, rh_id, 0, &rh_type, &rh_ndims, rh_dimids,
                          &rh_natts);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_inq_var(int ncid, int varid, char *name, nc_type *xtypep,  
	   int *ndimsp, int *dimidsp, int *nattsp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(ncid, varid, name, xtypep, ndimsp, 
				     dimidsp, nattsp, NULL, NULL, NULL, 
				     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

/** 
\ingroup variables
Learn the name of a variable.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param name Returned variable name. The caller must allocate space for
the returned name. The maximum length is \ref NC_MAX_NAME. Ignored if
NULL.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.
\returns \ref NC_ENOTVAR Invalid variable ID.
 */
int 
nc_inq_varname(int ncid, int varid, char *name)
{
   return nc_inq_var(ncid, varid, name, NULL, NULL,
		     NULL, NULL);
}

/** Learn the type of a variable.
\ingroup variables

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param typep Pointer where typeid will be stored. Ignored if NULL.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.
\returns \ref NC_ENOTVAR Invalid variable ID.
 */
int 
nc_inq_vartype(int ncid, int varid, nc_type *typep)
{
   return nc_inq_var(ncid, varid, NULL, typep, NULL,
		     NULL, NULL);
}

/** Learn how many dimensions are associated with a variable.
\ingroup variables

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param ndimsp Pointer where number of dimensions will be
stored. Ignored if NULL.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.
\returns \ref NC_ENOTVAR Invalid variable ID.
 */
int 
nc_inq_varndims(int ncid, int varid, int *ndimsp)
{
   return nc_inq_var(ncid, varid, NULL, NULL, ndimsp, NULL, NULL);
}

/** Learn the dimension IDs associated with a variable.
\ingroup variables

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param dimidsp Pointer where array of dimension IDs will be
stored. Ignored if NULL.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.
\returns \ref NC_ENOTVAR Invalid variable ID.
 */
int 
nc_inq_vardimid(int ncid, int varid, int *dimidsp)
{
   return nc_inq_var(ncid, varid, NULL, NULL, NULL, 
		     dimidsp, NULL);
}

/** Learn how many attributes are associated with a variable.
\ingroup variables

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param nattsp Pointer where number of attributes will be
stored. Ignored if NULL.

\returns \ref NC_NOERR No error.
\returns \ref NC_EBADID Bad ncid.
\returns \ref NC_ENOTVAR Invalid variable ID.
 */
int 
nc_inq_varnatts(int ncid, int varid, int *nattsp)
{
   if (varid == NC_GLOBAL)
      return nc_inq_natts(ncid,nattsp);
   /*else*/
   return nc_inq_var(ncid, varid, NULL, NULL, NULL, NULL, 
		     nattsp);
}

/*! \} */  /* End of named group ...*/
