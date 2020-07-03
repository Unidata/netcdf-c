/*
 * Copyright 2018, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include "netcdf.h"
#include "netcdf_filter.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "ncfilter.h"

#ifdef USE_HDF5
#include "hdf5internal.h"
#endif

/*
Unified filter related code
*/

#ifndef H5Z_FILTER_SZIP
/** ID of HDF SZIP filter. */
#define H5Z_FILTER_SZIP 4
#endif

/*Mnemonic*/
#define USENAME 1

/**************************************************/
/* Support direct user defined filters */

#ifdef ENABLE_CLIENTSIDE_FILTERS

/* Use void* to avoid having to include hdf.h*/
EXTERNL int
nc_filter_client_register(unsigned int id, void* info)
{
    int stat = NC_NOERR;
#ifdef USE_HDF5
    NC_FILTER_OBJ_HDF5 client;
    if(id == 0 ||info == NULL)
	return NC_EINVAL;
    memset(&client,0,sizeof(client));
    client.hdr.format = NC_FILTER_FORMAT_HDF5;
    client.sort = NC_FILTER_SORT_CLIENT;
    client.u.client.id = id;
    client.u.client.info = info;
    /* Note use of a global function, not part of the dispatch table */
    stat = nc4_global_filter_action(NCFILTER_CLIENT_REG, id, &client);
#else
    stat = NC_ENOTBUILT;
#endif
    return stat;
}

EXTERNL int
nc_filter_client_unregister(unsigned int id)
{
int stat = NC_NOERR;
#ifdef USE_HDF5
    stat = nc4_global_filter_action(NCFILTER_CLIENT_UNREG, id, NULL);
#else
    stat = NC_ENOTBUILT;
#endif
    return stat;
}

/* Use void* to avoid having to include hdf.h*/
EXTERNL int
nc_filter_client_inq(unsigned int id, void* infop)
{
int stat = NC_NOERR;
#ifdef USE_HDF5
    H5Z_class2_t* hct = (H5Z_class2_t*)infop;
    NC_FILTER_OBJ_HDF5 client;
    if(id == 0 ||infop == NULL)
	return NC_EINVAL;
    memset(&client,0,sizeof(client));
    client.hdr.format = NC_FILTER_FORMAT_HDF5;
    client.sort = NC_FILTER_SORT_CLIENT;
    client.u.client.id = id;
    client.u.client.info = hct;
    /* Note use of a global function, not part of the dispatch table */
    stat = nc4_global_filter_action(NCFILTER_CLIENT_INQ, id, &client);
    if(stat == NC_NOERR) {
	*hct = *(H5Z_class2_t*)client.u.client.info;
    }
#else
    stat = NC_ENOTBUILT;
#endif
    return stat;
}
#endif /*ENABLE_CLIENTSIDE_FILTERS*/

/**************************************************/
/* Per-variable filters */
/* The original HDF5-based functions are left
   but are now wrappers around the filterx functions.
*/


/**
Find the set of filters (if any) associated with a variable.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

@deprecated{Use nc_inq_filterx_ids}
\param varid Variable ID
\param nfilters return no. of filters
\param ids return the filter ids (caller allocates)

\returns ::NC_NOERR No error.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_EINVAL Invalid arguments
\ingroup variables
\author Dennis Heimbigner
*/
EXTERNL int
nc_inq_var_filterids(int ncid, int varid, size_t* nfiltersp, unsigned int* ids)
{
   int stat = NC_NOERR;
   size_t nfilters = 0;
   char** xidlist = NULL;

   TRACE(nc_inq_var_filterids);

   if((stat = nc_inq_var_filterx_ids(ncid, varid, &nfilters, NULL))) goto done;
   if(nfilters > 0) {
        if((xidlist = malloc(sizeof(char*)*(nfilters+1)))==NULL)
           {stat = NC_ENOMEM; goto done;}
	if((stat = nc_inq_var_filterx_ids(ncid, varid, &nfilters, xidlist))) goto done;
	/* Convert out to HDF5 format */
	if(ids != NULL) {
	    if((stat = NC_cvtX2I_idlist(nfilters,(const char**)xidlist,ids)))
	        goto done;
	}
   }
   if(nfiltersp) *nfiltersp = nfilters;

done:
   /* Cleanup */
   NC_filterx_freestringvec(nfilters,xidlist);
   return stat;
}

/**
Find the the param info about filter (if any)
associated with a variable and with specified id.

This is a wrapper for nc_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

@deprecated{Use nc_inq_filterx_info}
\param varid Variable ID
\param id The filter id of interest
\param formatp (Out) Storage for the filter format
\param nparamsp (Out) Storage which will get the number of parameters to the filter
\param params (Out) Storage which will get associated parameters.
Note: the caller must allocate and free.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_ENOFILTER No filter defined.
\ingroup variables
\author Dennis Heimbigner
*/
EXTERNL int
nc_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparamsp, unsigned int* params)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    size_t nparams = 0;
    char** xparamslist = NULL;
    char* xid = NULL;

    if(stat != NC_NOERR) return stat;
    TRACE(nc_inq_var_filter_info);

    if((stat = NC_cvtI2X_id(id,&xid,!USENAME))) goto done;
    if((stat = nc_inq_var_filterx_info(ncid,varid,xid,&nparams,NULL)))
	goto done;
    if((xparamslist = malloc(sizeof(char*)*(nparams+1))) == NULL) goto done;
    if((stat = nc_inq_var_filterx_info(ncid,varid,xid,&nparams,xparamslist)))
	goto done;
    if(nparamsp) *nparamsp = nparams;
    if(params) {
	if((stat = NC_cvtX2I_params(nparams,(const char**)xparamslist,params))) goto done;
    }
done:
    nullfree(xid);
    NC_filterx_freestringvec(nparams,xparamslist);
    return stat;
}

/**
Find the first filter (if any) associated with a variable.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param idp Storage which will get the filter id; a return value of zero means no filter

\param nparamsp Storage which will get the number of parameters to the
filter

\param params Storage which will get associated parameters.
Note: the caller must allocate and free.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_ENOFILTER No filter defined.
\ingroup variables
\author Dennis Heimbigner
*/
EXTERNL int
nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparamsp, unsigned int* params)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   size_t nfilters,nparams;
   unsigned int* filterids = NULL;
   
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_filter);

   if(idp == NULL)
   	{stat = NC_ENOFILTER; goto done;}
   if((stat = nc_inq_var_filterids(ncid,varid, &nfilters, NULL))) goto done;
   if(nfilters == 0)
   	{stat = NC_ENOFILTER; goto done;}
   if((filterids = malloc(sizeof(unsigned int)*nfilters))==NULL)
	{stat = NC_ENOMEM; goto done;}
   if((stat = nc_inq_var_filterids(ncid,varid, &nfilters, filterids))) goto done;
   if((stat = nc_inq_var_filter_info(ncid,varid,filterids[0],&nparams,params))) goto done;
   if(idp) *idp = filterids[0];
   if(nparamsp) *nparamsp = nparams;

done:
   nullfree(filterids);   
   return stat;
}

/**
   Define a new variable hdf5 filter.

   Only variables with chunked storage can use filters.

   @deprecated{Use nc_def_var_filterx}
   @param ncid File and group ID.
   @param varid Variable ID.
   @param id Filter ID.
   @param nparams Number of filter parameters.
   @param parms Filter parameters.

   @return ::NC_NOERR No error.
   @return ::NC_EINVAL Variable must be chunked.
   @return ::NC_EBADID Bad ID.
   @author Dennis Heimbigner
*/

EXTERNL int
nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* params)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    char* xid = NULL;
    char** xparams = NULL;
   
    if(stat != NC_NOERR) return stat;
    TRACE(nc_def_var_filter);

    if((stat = NC_cvtI2X_id(id,&xid,!USENAME))) goto done;
    if(nparams > 0) {
        if((xparams=malloc(nparams*sizeof(char*))) == NULL)
            {stat = NC_ENOMEM; goto done;}
        if((stat = NC_cvtI2X_params(nparams,params,xparams))) goto done;
    }
    if((stat = nc_def_var_filterx(ncid,varid,xid,nparams,(const char**)xparams))) goto done;
    
done:
    nullfree(xid);
    NC_filterx_freestringvec(nparams,xparams);
    return stat;
}

/**
   Remove all filters with specified id from a variable

   @deprecated{Use nc_var_filterx_remove}
   @param ncid File and group ID.
   @param varid Variable ID.
   @param id filter to remove

   @return ::NC_NOERR No error.
   @return ::NC_EBADID Bad ID.
   @author Dennis Heimbigner
*/
EXTERNL int
nc_var_filter_remove(int ncid, int varid, unsigned int id)
{
    int stat = NC_NOERR;
    char* xid = NULL;
    
    TRACE(nc_var_filter_hdf5_remove);
    if((stat = NC_cvtI2X_id(id,&xid,!USENAME))) goto done;
    if((stat = nc_var_filterx_remove(ncid,varid,xid))) goto done;

done:
    nullfree(xid);
    return stat;
}
