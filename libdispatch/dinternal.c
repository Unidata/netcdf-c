/*! \internal
Public functions from dispatch table.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/


#include "config.h"
#include <stdlib.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include "netcdf.h"
#include "nc.h"
#include "ncdispatch.h"


/** \internal
\ingroup variables
 */
int
NC_is_recvar(int ncid, int varid, size_t* nrecs)
{
   int status = NC_NOERR;
   int unlimid;
   int ndims;
   int dimset[NC_MAX_VAR_DIMS];

   status = nc_inq_unlimdim(ncid,&unlimid);
   if(status != NC_NOERR) return 0; /* no unlimited defined */
   status = nc_inq_varndims(ncid,varid,&ndims);
   if(status != NC_NOERR) return 0; /* no unlimited defined */
   if(ndims == 0) return 0; /* scalar */
   status = nc_inq_vardimid(ncid,varid,dimset);
   if(status != NC_NOERR) return 0; /* no unlimited defined */
   status = nc_inq_dim(ncid,dimset[0],NULL,nrecs);
   if(status != NC_NOERR) return 0;
   return (dimset[0] == unlimid ? 1: 0);
}

/** \internal
\ingroup variables
Get the number of record dimensions for a variable and an array that
identifies which of a variable's dimensions are record dimensions.
Intended to be used instead of NC_is_recvar, which doesn't work for
netCDF-4 variables which have multiple unlimited dimensions or an
unlimited dimension that is not the first of a variable's dimensions.
Example use:
\code
int nrecdims;
int is_recdim[NC_MAX_VAR_DIMS];
  ...
status = NC_inq_recvar(ncid,varid,&nrecdims,is_recdim);
isrecvar = (nrecdims > 0);
\endcode
 */
int
NC_inq_recvar(int ncid, int varid, int* nrecdimsp, int *is_recdim)
{
   int status = NC_NOERR;
   int unlimid;
   int nvardims;
   int dimset[NC_MAX_VAR_DIMS];
   int dim;
   int nrecdims = 0;

   status = nc_inq_varndims(ncid,varid,&nvardims);
   if(status != NC_NOERR) return status;
   if(nvardims == 0) return NC_NOERR; /* scalars have no dims */
   for(dim = 0; dim < nvardims; dim++)
     is_recdim[dim] = 0;
   status = nc_inq_unlimdim(ncid, &unlimid);
   if(status != NC_NOERR) return status;
   if(unlimid == -1) return status; /* no unlimited dims for any variables */
#ifdef USE_NETCDF4
   {
     int nunlimdims;
     int *unlimids;
     int recdim;
     status = nc_inq_unlimdims(ncid, &nunlimdims, NULL); /* for group or file, not variable */
     if(status != NC_NOERR) return status;
     if(nunlimdims == 0) return status;

     if (!(unlimids = malloc(nunlimdims * sizeof(int))))
       return NC_ENOMEM;
     status = nc_inq_unlimdims(ncid, &nunlimdims, unlimids); /* for group or file, not variable */
     if(status != NC_NOERR) {
       free(unlimids);
       return status;
     }
     status = nc_inq_vardimid(ncid, varid, dimset);
     if(status != NC_NOERR) {
       free(unlimids);
       return status;
     }
     for (dim = 0; dim < nvardims; dim++) { /* netCDF-4 rec dims need not be first dim for a rec var */
       for(recdim = 0; recdim < nunlimdims; recdim++) {
	 if(dimset[dim] == unlimids[recdim]) {
	   is_recdim[dim] = 1;
	   nrecdims++;
	 }
       }
     }
     free(unlimids);
   }
#else
   status = nc_inq_vardimid(ncid, varid, dimset);
   if(status != NC_NOERR) return status;
   if(dimset[0] == unlimid) {
     is_recdim[0] = 1;
     nrecdims++;
   }
#endif /* USE_NETCDF4 */
   if(nrecdimsp) *nrecdimsp = nrecdims;
   return status;
}

/* Ok to use NC pointers because
   all IOSP's will use that structure,
   but not ok to use e.g. NC_Var pointers
   because they may be different structure
   entirely.
*/

/** \internal
\ingroup variables
Find the length of a type. This is how much space is required by the user, as in
\code
vals = malloc(nel * nctypelen(var.type));
ncvarget(cdfid, varid, cor, edg, vals);
\endcode
 */
int
NCDISPATCH_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep,
               int *ndimsp, int *dimidsp, int *nattsp,
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp,
               int *no_fill, void *fill_valuep, int *endiannessp,
	       unsigned int* idp, size_t* nparamsp, unsigned int* params
               )
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid, name, xtypep,
      ndimsp, dimidsp, nattsp,
      shufflep, deflatep, deflate_levelp, fletcher32p,
      contiguousp, chunksizesp,
      no_fill, fill_valuep,
      endiannessp,
      idp, nparamsp, params);
}

/** \internal
\ingroup variables
Find the length of a type. Redunant over nctypelen() above. */
size_t
NC_atomictypelen(nc_type xtype)
{
   size_t sz = 0;
   switch(xtype) {
      case NC_NAT: sz = 0; break;
      case NC_BYTE: sz = sizeof(signed char); break;
      case NC_CHAR: sz = sizeof(char); break;
      case NC_SHORT: sz = sizeof(short); break;
      case NC_INT: sz = sizeof(int); break;
      case NC_FLOAT: sz = sizeof(float); break;
      case NC_DOUBLE: sz = sizeof(double); break;
      case NC_INT64: sz = sizeof(signed long long); break;
      case NC_UBYTE: sz = sizeof(unsigned char); break;
      case NC_USHORT: sz = sizeof(unsigned short); break;
      case NC_UINT: sz = sizeof(unsigned int); break;
      case NC_UINT64: sz = sizeof(unsigned long long); break;
#ifdef USE_NETCDF4
      case NC_STRING: sz = sizeof(char*); break;
#endif
      default: break;
   }
   return sz;
}

/** \internal
\ingroup variables
    Get the type name. */
char *
NC_atomictypename(nc_type xtype)
{
   char* nm = NULL;
   switch(xtype) {
      case NC_NAT: nm = "undefined"; break;
      case NC_BYTE: nm = "byte"; break;
      case NC_CHAR: nm = "char"; break;
      case NC_SHORT: nm = "short"; break;
      case NC_INT: nm = "int"; break;
      case NC_FLOAT: nm = "float"; break;
      case NC_DOUBLE: nm = "double"; break;
      case NC_INT64: nm = "int64"; break;
      case NC_UBYTE: nm = "ubyte"; break;
      case NC_USHORT: nm = "ushort"; break;
      case NC_UINT: nm = "uint"; break;
      case NC_UINT64: nm = "uint64"; break;
#ifdef USE_NETCDF4
      case NC_STRING: nm = "string"; break;
#endif
      default: break;
   }
   return nm;
}

/** \internal
\ingroup variables
Get the shape of a variable.
 */
int
NC_getshape(int ncid, int varid, int ndims, size_t* shape)
{
   int dimids[NC_MAX_VAR_DIMS];
   int i;
   int status = NC_NOERR;

   if ((status = nc_inq_vardimid(ncid, varid, dimids)))
      return status;
   for(i = 0; i < ndims; i++)
      if ((status = nc_inq_dimlen(ncid, dimids[i], &shape[i])))
	 break;

   return status;
}


/*Provide an internal function for generating pseudo file descriptors
  for systems that are not file based (e.g. dap, memio).
*/

/* Static counter for pseudo file descriptors (incremented) */
static int pseudofd = 0;

/* Create a pseudo file descriptor that does not
   overlap real file descriptors
*/
int
nc__pseudofd(void)
{
    if(pseudofd == 0)  {
        int maxfd = 32767; /* default */
#ifdef HAVE_GETRLIMIT
        struct rlimit rl;
        if(getrlimit(RLIMIT_NOFILE,&rl) == 0) {
	    if(rl.rlim_max != RLIM_INFINITY)
	        maxfd = (int)rl.rlim_max;
	    if(rl.rlim_cur != RLIM_INFINITY)
	        maxfd = (int)rl.rlim_cur;
	}
	pseudofd = maxfd+1;
#endif
    }
    return pseudofd++;
}
