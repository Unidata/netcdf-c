/*! \file
Functions for defining and inquiring about variables.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

/** \defgroup variables Variables

Variables hold multi-dimensional arrays of data.

Variables for a netCDF dataset are defined when the dataset is
created, while the netCDF dataset is in define mode. Other variables
may be added later by reentering define mode. A netCDF variable has a
name, a type, and a shape, which are specified when it is defined. A
variable may also have values, which are established later in data
mode.

Ordinarily, the name, type, and shape are fixed when the variable is
first defined. The name may be changed, but the type and shape of a
variable cannot be changed. However, a variable defined in terms of
the unlimited dimension can grow without bound in that dimension.

A netCDF variable in an open netCDF dataset is referred to by a small
integer called a variable ID.

Variable IDs reflect the order in which variables were defined within
a netCDF dataset. Variable IDs are 0, 1, 2,..., in the order in which
the variables were defined. A function is available for getting the
variable ID from the variable name and vice-versa.

Attributes (see Attributes) may be associated with a variable to
specify such properties as units.

Operations supported on variables are:
- Create a variable, given its name, data type, and shape.
- Get a variable ID from its name.
- Get a variable's name, data type, shape, and number of attributes
  from its ID.
- Put a data value into a variable, given variable ID, indices, and value.
- Put an array of values into a variable, given variable ID, corner
  indices, edge lengths, and a block of values.
- Put a subsampled or mapped array-section of values into a variable,
  given variable ID, corner indices, edge lengths, stride vector,
  index mapping vector, and a block of values.
- Get a data value from a variable, given variable ID and indices.
- Get an array of values from a variable, given variable ID, corner
  indices, and edge lengths.
- Get a subsampled or mapped array-section of values from a variable,
  given variable ID, corner indices, edge lengths, stride vector, and
  index mapping vector.
- Rename a variable. 

\section language_types Language Types Corresponding to netCDF
External Data Types

NetCDF supported six atomic data types through version 3.6.0 (char,
byte, short, int, float, and double). Starting with version 4.0, many
new atomic and user defined data types are supported (unsigned int
types, strings, compound types, variable length arrays, enums,
opaque).

The additional data types are only supported in netCDF-4/HDF5
files. To create netCDF-4/HDF5 files, use the HDF5 flag in
nc_create. (see nc_create).

\section classic_types NetCDF-3 Classic and 64-Bit Offset Data Types

NetCDF-3 classic and 64-bit offset files support 6 atomic data types,
and none of the user defined datatype introduced in NetCDF-4.

The following table gives the netCDF-3 external data types and the
corresponding type constants for defining variables in the C
interface:

<table>
<tr><td>Type</td><td>C define</td><td>Bits</td></tr>
<tr><td>byte</td><td>NC_BYTE</td><td>8</td></tr>
<tr><td>char</td><td>NC_CHAR</td><td>8</td></tr>
<tr><td>short</td><td>NC_SHORT</td><td>16</td></tr>
<tr><td>int</td><td>NC_INT</td><td>32</td></tr>
<tr><td>float</td><td>NC_FLOAT</td><td>32</td></tr>
<tr><td>double</td><td>NC_DOUBLE</td><td>64</td></tr>
</table>

The first column gives the netCDF external data type, which is the
same as the CDL data type. The next column gives the corresponding C
pre-processor macro for use in netCDF functions (the pre-processor
macros are defined in the netCDF C header-file netcdf.h). The last
column gives the number of bits used in the external representation of
values of the corresponding type.

\section netcdf_4_atomic NetCDF-4 Atomic Types

NetCDF-4 files support all of the atomic data types from netCDF-3,
plus additional unsigned integer types, 64-bit integer types, and a
string type.

<table>
<tr><td>Type</td><td>C define</td><td>Bits

<tr><td>byte</td><td>NC_BYTE</td><td>8</td></tr>
<tr><td>unsigned byte </td><td>NC_UBYTE^</td><td> 8</td></tr>
<tr><td>char </td><td>NC_CHAR </td><td>8</td></tr>
<tr><td>short </td><td>NC_SHORT </td><td>16</td></tr>
<tr><td>unsigned short </td><td>NC_USHORT^ </td><td>16</td></tr>
<tr><td>int </td><td>NC_INT </td><td>32</td></tr>
<tr><td>unsigned int </td><td>NC_UINT^ </td><td>32</td></tr>
<tr><td>unsigned long long </td><td>NC_UINT64^ </td><td>64</td></tr>
<tr><td>long long </td><td>NC_INT64^ </td><td>64</td></tr>
<tr><td>float </td><td>NC_FLOAT </td><td>32</td></tr>
<tr><td>double </td><td>NC_DOUBLE </td><td>64</td></tr>
<tr><td>char ** </td><td>NC_STRING^ </td><td>string length + 1</td></tr>
</table>

^This type was introduced in netCDF-4, and is not supported in netCDF
classic or 64-bit offset format files, or in netCDF-4 files if they
are created with the NC_CLASSIC_MODEL flags.
 */
/**@{*/

#include "ncdispatch.h"
#include "netcdf_f.h"

#if defined(__cplusplus)
/* C++ consts default to internal linkage and must be initialized */
const size_t coord_zero[NC_MAX_VAR_DIMS] = {0};
const size_t coord_one[NC_MAX_VAR_DIMS] = {1};
#else
static const size_t coord_zero[NC_MAX_VAR_DIMS];
/* initialized int put/get_var1 below */
static size_t coord_one[NC_MAX_VAR_DIMS];
#endif

#define INITCOORD1 if(coord_one[0] != 1) {int i; for(i=0;i<NC_MAX_VAR_DIMS;i++) coord_one[i] = 1;}

#define MINVARSSPACE 1024;

static int
getshape(int ncid, int varid, int ndims, size_t* shape)
{
   int dimids[NC_MAX_VAR_DIMS];
   int i;
   int status;

   if ((status = nc_inq_vardimid(ncid, varid, dimids)))
      return status;
   for(i = 0; i < ndims; i++) 
      if ((status = nc_inq_dimlen(ncid, dimids[i], &shape[i])))
	 break;

   return status;
}

/**
Define a new variable.

The function nc_def_var adds a new variable to an open netCDF dataset
in define mode. It returns (as an argument) a variable ID, given the
netCDF ID, the variable name, the variable type, the number of
dimensions, and a list of the dimension IDs.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param name Variable name. For nc_inq_attname, this is a pointer to
the location for the returned attribute name.

\param xtype 

\param ndims Number of dimensions for the variable. For example, 2
specifies a matrix, 1 specifies a vector, and 0 means the variable is
a scalar with no dimensions. Must not be negative or greater than the
predefined constant NC_MAX_VAR_DIMS.

\param dimidsp Vector of ndims dimension IDs corresponding to the
variable dimensions. For classic model netCDF files, if the ID of the
unlimited dimension is included, it must be first. This argument is
ignored if ndims is 0. For expanded model netCDF4/HDF5 files, there
may be any number of unlimited dimensions, and they may be used in any
element of the dimids array.

\param varidp Pointer to location for the returned variable ID.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTINDEFINE Not in define mode. This is returned for
netCDF classic or 64-bit offset files, or for netCDF-4 files, when
they were been created with NC_STRICT_NC3 flag. (see nc_create).
\returns ::NC_ESTRICTNC3 Trying to create a var some place other
than the root group in a netCDF file with NC_STRICT_NC3 turned on.
\returns ::NC_EMAXVARS Max number of variables exceeded in a
classic or 64-bit offset file, or an netCDF-4 file with NC_STRICT_NC3
on.
\returns ::NC_EBADTYPE Bad type.
\returns ::NC_EINVAL Number of dimensions to large.
\returns ::NC_ENAMEINUSE Name already in use.
\returns ::NC_EPERM Attempt to create object in read-only file. 

\section Example

Here is an example using nc_def_var to create a variable named rh of
type double with three dimensions, time, lat, and lon in a new netCDF
dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;       
     int  ncid;          
     int  lat_dim, lon_dim, time_dim;  
     int  rh_id;                       
     int  rh_dimids[3];                
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...

     status = nc_def_dim(ncid, "lat", 5L, &lat_dim);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "lon", 10L, &lon_dim);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "time", NC_UNLIMITED, &time_dim);
     if (status != NC_NOERR) handle_error(status);
        ...

     rh_dimids[0] = time_dim;
     rh_dimids[1] = lat_dim;
     rh_dimids[2] = lon_dim;
     status = nc_def_var (ncid, "rh", NC_DOUBLE, 3, rh_dimids, &rh_id);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_def_var(int ncid, const char *name, nc_type xtype, 
	   int ndims,  const int *dimidsp, int *varidp)
{
   NC* ncp;
   int stat;

   if ((stat = NC_check_id(ncid, &ncp)))
      return stat;
   return ncp->dispatch->def_var(ncid, name, xtype, ndims,
				 dimidsp, varidp);
}

/** Rename a variable.

The function nc_rename_var() changes the name of a netCDF variable in
an open netCDF file. You cannot rename a variable to have the name of
any existing variable.

For classic format, 64-bit offset format, and netCDF-4/HDF5 with
classic mode, if the new name is longer than the old name, the netCDF
dataset must be in define mode. 

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param name New name of the variable.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_EBADNAME Bad name.
\returns ::NC_EMAXNAME Name is too long.
\returns ::NC_ENAMEINUSE Name in use.
\returns ::NC_ENOMEM Out of memory.

\section Example

Here is an example using nc_rename_var to rename the variable rh to
rel_hum in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status; 
     int  ncid;   
     int  rh_id;  
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid); 
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
     status = nc_rename_var (ncid, rh_id, "rel_hum");
     if (status != NC_NOERR) handle_error(status);
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode

*/
int
nc_rename_var(int ncid, int varid, const char *name)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->rename_var(ncid, varid, name);
}



static int
NC_put_vara(int ncid, int varid, const size_t *start, 
	    const size_t *edges, const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   if(edges == NULL) {
      size_t shape[NC_MAX_VAR_DIMS];
      int ndims;
      stat = nc_inq_varndims(ncid, varid, &ndims); 
      if(stat != NC_NOERR) return stat;
      stat = getshape(ncid, varid, ndims, shape);
      if(stat != NC_NOERR) return stat;
      return ncp->dispatch->put_vara(ncid, varid, start, shape, value, memtype);
   } else
      return ncp->dispatch->put_vara(ncid, varid, start, edges, value, memtype);
}

int
NC_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   if(edges == NULL) {
      size_t shape[NC_MAX_VAR_DIMS];
      int ndims;
      stat = nc_inq_varndims(ncid, varid, &ndims); 
      if(stat != NC_NOERR) return stat;
      stat = getshape(ncid,varid,ndims,shape);
      if(stat != NC_NOERR) return stat;
      return ncp->dispatch->get_vara(ncid,varid,start,shape,value,memtype);
   } else
      return ncp->dispatch->get_vara(ncid,varid,start,edges,value,memtype);
}

static int
NC_get_var(int ncid, int varid, void *value, nc_type memtype)
{
   int ndims;
   size_t shape[NC_MAX_VAR_DIMS];
   int stat = nc_inq_varndims(ncid,varid, &ndims);
   if(stat) return stat;
   stat = getshape(ncid,varid, ndims, shape);
   if(stat) return stat;
   return NC_get_vara(ncid, varid, coord_zero, shape, value, memtype);
}

static int
NC_put_var(int ncid, int varid, const void *value, nc_type memtype)
{
   int ndims;
   size_t shape[NC_MAX_VAR_DIMS];
   int stat = nc_inq_varndims(ncid,varid, &ndims);
   if(stat) return stat;
   stat = getshape(ncid,varid, ndims, shape);
   if(stat) return stat;
   return NC_put_vara(ncid, varid, coord_zero, shape, value, memtype);
}

static int
NC_get_var1(int ncid, int varid, const size_t *coord, void* value, 
	    nc_type memtype)
{
   INITCOORD1;
   return NC_get_vara(ncid, varid, coord, coord_one, value, memtype);
}

static int
NC_put_var1(int ncid, int varid, const size_t *coord, const void* value, 
	    nc_type memtype)
{
   INITCOORD1;
   return NC_put_vara(ncid, varid, coord, coord_one, value, memtype);
}

static int
is_recvar(int ncid, int varid, size_t* nrecs)
{
   int status;
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

/* Most dispatch tables will use the default procedures */
int
NCDEFAULT_get_vars(int ncid, int varid, const size_t * start,
	    const size_t * edges, const ptrdiff_t * stride,
	    void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->get_varm(ncid,varid,start,edges,stride,NULL,value,memtype);
}

int
NCDEFAULT_put_vars(int ncid, int varid, const size_t * start,
	    const size_t * edges, const ptrdiff_t * stride,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->put_varm(ncid,varid,start,edges,stride,NULL,value,memtype);
}

int
NCDEFAULT_get_varm(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride,
	    const ptrdiff_t *imapp, void *value0, nc_type memtype)
{
   int status;
   nc_type vartype;
   int varndims,maxidim;
   NC* ncp;
   size_t memtypelen;
   ptrdiff_t cvtmap[NC_MAX_VAR_DIMS];
   char* value = (char*)value0;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

/*
  if(NC_indef(ncp)) return NC_EINDEFINE;
*/

   status = nc_inq_vartype(ncid, varid, &vartype); 
   if(status != NC_NOERR) return status;
   /* Check that this is an atomic type */
   if(vartype >= NC_MAX_ATOMIC_TYPE)
	return NC_EMAPTYPE;

   status = nc_inq_varndims(ncid, varid, &varndims); 
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) {
      if(imapp != NULL && varndims != 0) {
	 /*
	  * convert map units from bytes to units of sizeof(type)
	  */
	 size_t ii;
	 const ptrdiff_t szof = (ptrdiff_t) nctypelen(vartype);
	 for(ii = 0; ii < varndims; ii++) {
	    if(imapp[ii] % szof != 0) {
	       /*free(cvtmap);*/
	       return NC_EINVAL;
	    }
	    cvtmap[ii] = imapp[ii] / szof;
	 }
	 imapp = cvtmap;
      }
      memtype = vartype;
   }

   if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
   else if(memtype != NC_CHAR && vartype == NC_CHAR)  
      return NC_ECHAR;

   memtypelen = nctypelen(memtype);

   maxidim = (int) varndims - 1;

   if (maxidim < 0)
   {
      /*
       * The variable is a scalar; consequently,
       * there s only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_get_vara(ncid, varid, start, edge1, value, memtype);
   }

   /*
    * else
    * The variable is an array.
    */
   {
      int idim;
      size_t *mystart = NULL;
      size_t *myedges;
      size_t *iocount;    /* count vector */
      size_t *stop;   /* stop indexes */
      size_t *length; /* edge lengths in bytes */
      ptrdiff_t *mystride;
      ptrdiff_t *mymap;
      size_t varshape[NC_MAX_VAR_DIMS];
      int isrecvar;
      size_t numrecs;

      /* Compute some dimension related values */
      isrecvar = is_recvar(ncid,varid,&numrecs);
      getshape(ncid,varid,varndims,varshape);	

      /*
       * Verify stride argument; also see if stride is all ones
       */
      if(stride != NULL) {
	 int stride1 = 1;
	 for (idim = 0; idim <= maxidim; ++idim)
	 {
            if (stride[idim] == 0
		/* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
            {
	       return NC_ESTRIDE;
            }
	    if(stride[idim] != 1) stride1 = 0;
	 }
         /* If stride1 is true, and there is no imap 
            then call get_vara directly.
         */
         if(stride1 && imapp == NULL) {
	     return NC_get_vara(ncid, varid, start, edges, value, memtype);
	 }
      }

      /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
      /* Allocate space for mystart,mystride,mymap etc.all at once */
      mystart = (size_t *)calloc(varndims * 7, sizeof(ptrdiff_t));
      if(mystart == NULL) return NC_ENOMEM;
      myedges = mystart + varndims;
      iocount = myedges + varndims;
      stop = iocount + varndims;
      length = stop + varndims;
      mystride = (ptrdiff_t *)(length + varndims);
      mymap = mystride + varndims;

      /*
       * Initialize I/O parameters.
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 mystart[idim] = start != NULL
	    ? start[idim]
	    : 0;

	 if (edges != NULL && edges[idim] == 0)
	 {
	    status = NC_NOERR;    /* read/write no data */
	    goto done;
	 }

#ifdef COMPLEX
	 myedges[idim] = edges != NULL
	    ? edges[idim]
	    : idim == 0 && isrecvar
	    ? numrecs - mystart[idim]
	    : varshape[idim] - mystart[idim];
#else
	 if(edges != NULL)
	    myedges[idim] = edges[idim];
	 else if (idim == 0 && isrecvar)
	    myedges[idim] = numrecs - mystart[idim];
	 else
	    myedges[idim] = varshape[idim] - mystart[idim];
#endif

	 mystride[idim] = stride != NULL
	    ? stride[idim]
	    : 1;

	 /* Remember: imapp is byte oriented, not index oriented */
#ifdef COMPLEX
	 mymap[idim] = (imapp != NULL
			? imapp[idim]
			: (idim == maxidim ? 1
			   : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1]));
#else
	 if(imapp != NULL)
	    mymap[idim] = imapp[idim];
	 else if (idim == maxidim)
	    mymap[idim] = 1;
	 else
	    mymap[idim] = 
	       mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];
#endif
	 iocount[idim] = 1;
	 length[idim] = mymap[idim] * myedges[idim];
	 stop[idim] = mystart[idim] + myedges[idim] * mystride[idim];
      }

      /*
       * Check start, edges
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 size_t dimlen = 
	    idim == 0 && isrecvar
	    ? numrecs
	    : varshape[idim];
	 if (mystart[idim] >= dimlen)
	 {
	    status = NC_EINVALCOORDS;
	    goto done;
	 }

	 if (mystart[idim] + myedges[idim] > dimlen)
	 {
	    status = NC_EEDGE;
	    goto done;
	 }

      }


      /* Lower body */
      /*
       * As an optimization, adjust I/O parameters when the fastest 
       * dimension has unity stride both externally and internally.
       * In this case, the user could have called a simpler routine
       * (i.e. ncvar$1()
       */
      if (mystride[maxidim] == 1
	  && mymap[maxidim] == 1)
      {
	 iocount[maxidim] = myedges[maxidim];
	 mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
	 mymap[maxidim] = (ptrdiff_t) length[maxidim];
      }

      /* 
       * Perform I/O.  Exit when done.
       */
      for (;;)
      {
	 /* TODO: */
	 int lstatus = NC_get_vara(ncid, varid, mystart, iocount,
				   value, memtype);
	 if (lstatus != NC_NOERR) {
	    if(status == NC_NOERR || lstatus != NC_ERANGE)
	       status = lstatus;
	 }
	 /*
	  * The following code permutes through the variable s
	  * external start-index space and it s internal address
	  * space.  At the UPC, this algorithm is commonly
	  * called "odometer code".
	  */
	 idim = maxidim;
        carry:
	 value += (mymap[idim] * memtypelen);
	 mystart[idim] += mystride[idim];
	 if (mystart[idim] == stop[idim])
	 {
	    mystart[idim] = start[idim];
	    value -= (length[idim] * memtypelen);
	    if (--idim < 0)
	       break; /* normal return */
	    goto carry;
	 }
      } /* I/O loop */
     done:
      free(mystart);
   } /* variable is array */
   return status;
}


int
NCDEFAULT_put_varm(
   int ncid,
   int varid,
   const size_t * start,
   const size_t * edges,
   const ptrdiff_t * stride,
   const ptrdiff_t * imapp,
   const void *value0,
   nc_type memtype)
{
   int status;
   nc_type vartype;
   int varndims,maxidim;
   NC* ncp;
   size_t memtypelen;
   ptrdiff_t cvtmap[NC_MAX_VAR_DIMS];
   const char* value = (char*)value0;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

/*
  if(NC_indef(ncp)) return NC_EINDEFINE;
  if(NC_readonly (ncp)) return NC_EPERM;
*/

   /* mid body */
   status = nc_inq_vartype(ncid, varid, &vartype); 
   if(status != NC_NOERR) return status;
   /* Check that this is an atomic type */
   if(vartype >= NC_MAX_ATOMIC_TYPE)
	return NC_EMAPTYPE;

   status = nc_inq_varndims(ncid, varid, &varndims); 
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) {
      if(imapp != NULL && varndims != 0) {
	 /*
	  * convert map units from bytes to units of sizeof(type)
	  */
	 size_t ii;
	 const ptrdiff_t szof = (ptrdiff_t) nctypelen(vartype);
	 for(ii = 0; ii < varndims; ii++) {
	    if(imapp[ii] % szof != 0) {
	       /*free(cvtmap);*/
	       return NC_EINVAL;
	    }
	    cvtmap[ii] = imapp[ii] / szof;
	 }
	 imapp = cvtmap;
      }
      memtype = vartype;
   }

   if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
   else if(memtype != NC_CHAR && vartype == NC_CHAR)  
      return NC_ECHAR;

   memtypelen = nctypelen(memtype);

   maxidim = (int) varndims - 1;

   if (maxidim < 0)
   {
      /*
       * The variable is a scalar; consequently,
       * there s only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_put_vara(ncid, varid, start, edge1, value, memtype);
   }

   /*
    * else
    * The variable is an array.
    */
   {
      int idim;
      size_t *mystart = NULL;
      size_t *myedges;
      size_t *iocount;    /* count vector */
      size_t *stop;   /* stop indexes */
      size_t *length; /* edge lengths in bytes */
      ptrdiff_t *mystride;
      ptrdiff_t *mymap;
      size_t varshape[NC_MAX_VAR_DIMS];
      int isrecvar;
      size_t numrecs;
      int stride1; /* is stride all ones? */

      /*
       * Verify stride argument.
       */
      stride1 = 1; // assume ok;
      if(stride != NULL) {
	 for (idim = 0; idim <= maxidim; ++idim) {
            if ((stride[idim] == 0)
		/* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
	    {
	       return NC_ESTRIDE;
            }
	    if(stride[idim] != 1) stride1 = 0;
	 }
      }

      /* If stride1 is true, and there is no imap, then call get_vara
         directly
      */
      if(stride1 && imapp == NULL) {
	 return NC_put_vara(ncid, varid, start, edges, value, memtype);
      }

      /* Compute some dimension related values */
      isrecvar = is_recvar(ncid,varid,&numrecs);
      getshape(ncid,varid,varndims,varshape);	

      /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
      mystart = (size_t *)calloc(varndims * 7, sizeof(ptrdiff_t));
      if(mystart == NULL) return NC_ENOMEM;
      myedges = mystart + varndims;
      iocount = myedges + varndims;
      stop = iocount + varndims;
      length = stop + varndims;
      mystride = (ptrdiff_t *)(length + varndims);
      mymap = mystride + varndims;

      /*
       * Initialize I/O parameters.
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 mystart[idim] = start != NULL
	    ? start[idim]
	    : 0;

	 if (edges != NULL && edges[idim] == 0)
	 {
	    status = NC_NOERR;    /* read/write no data */
	    goto done;
	 }

	 myedges[idim] = edges != NULL
	    ? edges[idim]
	    : idim == 0 && isrecvar
	    ? numrecs - mystart[idim]
	    : varshape[idim] - mystart[idim];
	 mystride[idim] = stride != NULL
	    ? stride[idim]
	    : 1;
	 mymap[idim] = imapp != NULL
	    ? imapp[idim]
	    : idim == maxidim
	    ? 1
	    : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];

	 iocount[idim] = 1;
	 length[idim] = mymap[idim] * myedges[idim];
	 stop[idim] = mystart[idim] + myedges[idim] * mystride[idim];
      }

      /*
       * Check start, edges
       */
      for (idim = isrecvar; idim < maxidim; ++idim)
      {
	 if (mystart[idim] > varshape[idim])
	 {
	    status = NC_EINVALCOORDS;
	    goto done;
	 }
	 if (mystart[idim] + myedges[idim] > varshape[idim])
	 {
	    status = NC_EEDGE;
	    goto done;
	 }
      }

      /* Lower body */
      /*
       * As an optimization, adjust I/O parameters when the fastest 
       * dimension has unity stride both externally and internally.
       * In this case, the user could have called a simpler routine
       * (i.e. ncvar$1()
       */
      if (mystride[maxidim] == 1
	  && mymap[maxidim] == 1)
      {
	 iocount[maxidim] = myedges[maxidim];
	 mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
	 mymap[maxidim] = (ptrdiff_t) length[maxidim];
      }

      /*
       * Perform I/O.  Exit when done.
       */
      for (;;)
      {
	 /* TODO: */
	 int lstatus = NC_put_vara(ncid, varid, mystart, iocount,
				   value, memtype);
	 if (lstatus != NC_NOERR) {
	    if(status == NC_NOERR || lstatus != NC_ERANGE)
	       status = lstatus;
	 }	    

	 /*
	  * The following code permutes through the variable s
	  * external start-index space and it s internal address
	  * space.  At the UPC, this algorithm is commonly
	  * called "odometer code".
	  */
	 idim = maxidim;
        carry:
	 value += (mymap[idim] * memtypelen);
	 mystart[idim] += mystride[idim];
	 if (mystart[idim] == stop[idim])
	 {
	    mystart[idim] = start[idim];
	    value -= (length[idim] * memtypelen);
	    if (--idim < 0)
	       break; /* normal return */
	    goto carry;
	 }
      } /* I/O loop */
     done:
      free(mystart);
   } /* variable is array */
   return status;
}

/* Called by externally visible nc_get_vars_xxx routines */
static int
NC_get_vars(int ncid, int varid, const size_t *start, 
	    const size_t *edges, const ptrdiff_t *stride, void *value,
	    nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->get_vars(ncid,varid,start,edges,stride,value,memtype);
}

static int
NC_put_vars(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->put_vars(ncid,varid,start,edges,stride,value,memtype);
}

/* Called by externally visible nc_get_vars_xxx routines */
static int
NC_get_varm(int ncid, int varid, const size_t *start, 
	    const size_t *edges, const ptrdiff_t *stride, const ptrdiff_t* map,
	    void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->get_varm(ncid,varid,start,edges,stride,map,value,memtype);
}

static int
NC_put_varm(int ncid, int varid, const size_t *start, 
	    const size_t *edges, const ptrdiff_t *stride, const ptrdiff_t* map,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->put_varm(ncid,varid,start,edges,stride,map,value,memtype);
}

/* Ok to use NC pointers because
   all IOSP's will use that structure,
   but not ok to use e.g. NC_Var pointers
   because they may be different structure
   entirely.
*/

/*
 *  This is how much space is required by the user, as in
 *
 *   vals = malloc(nel * nctypelen(var.type));
 *   ncvarget(cdfid, varid, cor, edg, vals);
 */
int
nctypelen(nc_type type) 
{
   switch(type){
      case NC_CHAR :
	 return((int)sizeof(char));
      case NC_BYTE :
	 return((int)sizeof(signed char));
      case NC_SHORT :
	 return(int)(sizeof(short));
      case NC_INT :
	 return((int)sizeof(int));
      case NC_FLOAT :
	 return((int)sizeof(float));
      case NC_DOUBLE : 
	 return((int)sizeof(double));

	 /* These can occur in netcdf-3 code */ 
      case NC_UBYTE :
	 return((int)sizeof(unsigned char));
      case NC_USHORT :
	 return((int)(sizeof(unsigned short)));
      case NC_UINT :
	 return((int)sizeof(unsigned int));
      case NC_INT64 :
	 return((int)sizeof(signed long long));
      case NC_UINT64 :
	 return((int)sizeof(unsigned long long));
#ifdef USE_NETCDF4
      case NC_STRING :
	 return((int)sizeof(char*));
#endif /*USE_NETCDF4*/

      default:
	 return -1;
   }
}

/* utility functions */
/* Redunant over nctypelen above */
int
NC_atomictypelen(nc_type xtype)
{
   int sz = 0;
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

char*
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

int
nc_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            const void *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   nc_type xtype;
   if(stat != NC_NOERR) return stat;
   stat = nc_inq_vartype(ncid, varid, &xtype);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,start,edges,value,xtype);
}

/**
Read an array of values from a variable. 

The array to be read is specified by giving a corner and a vector of
edge lengths to \ref specify_hyperslab. 

The data values are read into consecutive locations with the last
dimension varying fastest. The netCDF dataset must be in data mode
(for netCDF-4/HDF5 files, the switch to data mode will happen
automatically, unless the classic model is used).

The nc_get_vara() function will read a variable of any type,
including user defined type. For this function, the type of the data
in memory must match the type of the variable - no data conversion is
done.

Other nc_get_vara_ functions will convert data to the desired output
type as needed.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param ip Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.

\section Example

Here is an example using nc_get_vara_double() to read all the values of
the variable named rh from an existing netCDF dataset named
foo.nc. For simplicity in this example, we assume that we know that rh
is dimensioned with time, lat, and lon, and that there are three time
values, five lat values, and ten lon values.

\code
     #include <netcdf.h>
        ...
     #define TIMES 3
     #define LATS 5
     #define LONS 10
     int  status;                      
     int ncid;                         
     int rh_id;                        
     static size_t start[] = {0, 0, 0};
     static size_t count[] = {TIMES, LATS, LONS};
     double rh_vals[TIMES*LATS*LONS]; 
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_get_vara_double(ncid, rh_id, start, count, rh_vals);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
/**@{*/
int
nc_get_vara(int ncid, int varid, const size_t *startp, 
	    const size_t *countp, void *ip)
{
   NC* ncp;
   nc_type xtype;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   stat = nc_inq_vartype(ncid, varid, &xtype);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid, varid, startp, countp, ip, xtype);
}

int
nc_get_vara_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, char *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid, varid, startp, countp, 
		      (void *)ip, NC_CHAR);
}

int
nc_get_vara_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, signed char *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid, varid, startp, countp, 
		      (void *)ip, NC_BYTE);
}

int
nc_get_vara_uchar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, unsigned char *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid, varid, startp, countp, 
		      (void *)ip, T_uchar);
}

int
nc_get_vara_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, short *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid, varid, startp, countp, 
		      (void *)ip, NC_SHORT);
}

int
nc_get_vara_int(int ncid, int varid,
		const size_t *startp, const size_t *countp, int *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,NC_INT);
}

int
nc_get_vara_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp, long *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_long);
}

int
nc_get_vara_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp, float *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_float);
}


int
nc_get_vara_double(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, double *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_double);
}

int
nc_get_vara_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp, unsigned char *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_ubyte);
}

int
nc_get_vara_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp, unsigned short *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_ushort);
}

int
nc_get_vara_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp, unsigned int *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_uint);
}

int
nc_get_vara_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp, long long *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,T_longlong);
}

int
nc_get_vara_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp, unsigned long long *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_vara_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp, char* *ip)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid,varid,startp,countp,(void*)ip,NC_STRING);
}

#endif /*USE_NETCDF4*/
/**@}*/
int
nc_get_var(int ncid, int varid, void *value)
{
   return NC_get_var(ncid, varid, value, NC_NAT);
}

int
nc_put_var(int ncid, int varid, const void *value)
{
   return NC_put_var(ncid, varid, value, NC_NAT);
}

int
nc_get_var1(int ncid, int varid, const size_t *coord, void *value)
{
   return NC_get_var1(ncid, varid, coord, value, NC_NAT);
}

int
nc_put_var1(int ncid, int varid, const size_t *coord, const void *value)
{
   return NC_put_var1(ncid, varid, coord, value, NC_NAT);
}

int
nc_get_varm(int ncid, int varid, const size_t * startp,
	    const size_t * countp, const ptrdiff_t * stride,
	    const ptrdiff_t * imapp, void *value)
{
   NC* ncp;
   int stat;

   if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
   return ncp->dispatch->get_varm(ncid, varid, startp, countp, stride, imapp,
		      value, NC_NAT);
}

int
nc_put_varm (int ncid, int varid, const size_t * startp,
	     const size_t * countp, const ptrdiff_t * stride,
	     const ptrdiff_t * imapp, const void *value)
{
   NC* ncp;
   int stat;

   if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
   return ncp->dispatch->put_varm(ncid, varid, startp, countp, stride, imapp,
		      value, NC_NAT);
}

int
nc_get_vars (int ncid, int varid, const size_t * startp,
	     const size_t * countp, const ptrdiff_t * stride,
	     void *value)
{
   NC* ncp;
   int stat;

   if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
   return ncp->dispatch->get_vars(ncid, varid, startp, countp, stride,
		      value, NC_NAT);
}

int
nc_put_vars (int ncid, int varid, const size_t * startp,
	     const size_t * countp, const ptrdiff_t * stride,
	     const void *value)
{
   NC* ncp;
   int stat;

   if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
   return ncp->dispatch->put_vars(ncid, varid, startp, countp, stride,
		      value, NC_NAT);
}

int
nc_get_var1_text(int ncid, int varid, const size_t *coord, char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid, varid, coord, (void*)value, NC_CHAR);
}

int
nc_get_var1_schar(int ncid, int varid, const size_t *coord, signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid, varid, coord, (void*)value, NC_BYTE);
}

int
nc_get_var1_uchar(int ncid, int varid, const size_t *coord, unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid, varid, coord, (void*)value, NC_UBYTE);
}

int
nc_get_var1_short(int ncid, int varid, const size_t *coord, short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid, varid, coord, (void*)value, NC_SHORT);
}

int
nc_get_var1_int(int ncid, int varid, const size_t *coord, int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid, varid, coord, (void*)value, NC_INT);
}

int
nc_get_var1_long(int ncid, int varid, const size_t *coord, long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, longtype);
}

int
nc_get_var1_float(int ncid, int varid, const size_t *coord, float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_FLOAT);
}

int
nc_get_var1_double(int ncid, int varid, const size_t *coord, double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_DOUBLE);
}

int
nc_get_var1_ubyte(int ncid, int varid, const size_t *coord, unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_UBYTE);
}

int
nc_get_var1_ushort(int ncid, int varid, const size_t *coord, 
		   unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_USHORT);
}

int
nc_get_var1_uint(int ncid, int varid, const size_t *coord, unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_INT);
}

int
nc_get_var1_longlong(int ncid, int varid, const size_t *coord, long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_INT64);
}

int
nc_get_var1_ulonglong(int ncid, int varid, const size_t *coord, unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_var1_string(int ncid, int varid, const size_t *coord, char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_get_var1(ncid,varid,coord,(void*)value, NC_STRING);
}

#endif /*USE_NETCDF4*/

int
nc_put_var1_text(int ncid, int varid, const size_t *coord, const char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_CHAR);
}

int
nc_put_var1_schar(int ncid, int varid, const size_t *coord, const signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_BYTE);
}

int
nc_put_var1_uchar(int ncid, int varid, const size_t *coord, const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_UBYTE);
}

int
nc_put_var1_short(int ncid, int varid, const size_t *coord, const short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_SHORT);
}

int
nc_put_var1_int(int ncid, int varid, const size_t *coord, const int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_INT);
}

int
nc_put_var1_long(int ncid, int varid, const size_t *coord, const long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid, varid, coord, (void*)value, longtype);
}

int
nc_put_var1_float(int ncid, int varid, const size_t *coord, const float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid, varid, coord, (void*)value, NC_FLOAT);
}

int
nc_put_var1_double(int ncid, int varid, const size_t *coord, const double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_DOUBLE);
}

int
nc_put_var1_ubyte(int ncid, int varid, const size_t *coord, const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_UBYTE);
}

int
nc_put_var1_ushort(int ncid, int varid, const size_t *coord, const unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_USHORT);
}

int
nc_put_var1_uint(int ncid, int varid, const size_t *coord, const unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_UINT);
}

int
nc_put_var1_longlong(int ncid, int varid, const size_t *coord, const long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_INT64);
}

int
nc_put_var1_ulonglong(int ncid, int varid, const size_t *coord, const unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid,varid,coord,(void*)value, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_var1_string(int ncid, int varid, const size_t *coord, const char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   INITCOORD1;
   return NC_put_var1(ncid, varid, coord, (void*)value, NC_STRING);
}

#endif /*USE_NETCDF4*/

int
nc_get_var_text(int ncid, int varid, char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid, varid, (void*)value, NC_CHAR);
}

int
nc_get_var_schar(int ncid, int varid, signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_BYTE);
}

int
nc_get_var_uchar(int ncid, int varid, unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_UBYTE);
}

int
nc_get_var_short(int ncid, int varid, short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_SHORT);
}

int
nc_get_var_int(int ncid, int varid, int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_INT);
}

int
nc_get_var_long(int ncid, int varid, long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, longtype);
}

int
nc_get_var_float(int ncid, int varid, float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_FLOAT);
}

int
nc_get_var_double(int ncid, int varid, double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_DOUBLE);
}

int
nc_get_var_ubyte(int ncid, int varid, unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_UBYTE);
}

int
nc_get_var_ushort(int ncid, int varid, unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_USHORT);
}

int
nc_get_var_uint(int ncid, int varid, unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_UINT);
}

int
nc_get_var_longlong(int ncid, int varid, long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value, NC_INT64);
}

int
nc_get_var_ulonglong(int ncid, int varid, unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_var_string(int ncid, int varid, char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_var(ncid,varid,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/

int
nc_put_var_text(int ncid, int varid, const char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,NC_CHAR);
}

int
nc_put_var_schar(int ncid, int varid, const signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,NC_BYTE);
}

int
nc_put_var_uchar(int ncid, int varid, const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_uchar);
}

int
nc_put_var_short(int ncid, int varid, const short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,NC_SHORT);
}

int
nc_put_var_int(int ncid, int varid, const int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,NC_INT);
}

int
nc_put_var_long(int ncid, int varid, const long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_long);
}

int
nc_put_var_float(int ncid, int varid, const float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_float);
}

int
nc_put_var_double(int ncid, int varid, const double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_double);
}

int
nc_put_var_ubyte(int ncid, int varid, const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_ubyte);
}

int
nc_put_var_ushort(int ncid, int varid, const unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_ushort);
}

int
nc_put_var_uint(int ncid, int varid, const unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_uint);
}

int
nc_put_var_longlong(int ncid, int varid, const long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,T_longlong);
}

int
nc_put_var_ulonglong(int ncid, int varid, const unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_var_string(int ncid, int varid, const char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/


int
nc_put_vara_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const char *value)
{
   return NC_put_vara(ncid, varid, startp, countp, 
		      (void*)value, NC_CHAR);
}

int
nc_put_vara_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp, const signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,NC_BYTE);
}

int
nc_put_vara_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp, const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_uchar);
}

int
nc_put_vara_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp, const short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,NC_SHORT);
}

int
nc_put_vara_int(int ncid, int varid,
		const size_t *startp, const size_t *countp, const int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,NC_INT);
}

int
nc_put_vara_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp, const long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_long);
}

int
nc_put_vara_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp, const float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_float);
}

int
nc_put_vara_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp, const double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_double);
}

int
nc_put_vara_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp, const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_ubyte);
}

int
nc_put_vara_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp, const unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_ushort);
}

int
nc_put_vara_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp, const unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_uint);
}

int
nc_put_vara_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp, const long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,T_longlong);
}

int
nc_put_vara_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp, const unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_vara_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp, const char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid,varid,startp,countp,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/


int
nc_put_varm_text(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride, const ptrdiff_t * imapp,
		 const char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_CHAR);
}

int
nc_put_varm_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  const signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_BYTE);
}

int
nc_put_varm_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_uchar);
}

int
nc_put_varm_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  const short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_SHORT);
}

int
nc_put_varm_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t * stride, const ptrdiff_t * imapp,
		const int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_INT);
}

int
nc_put_varm_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride, const ptrdiff_t * imapp,
		 const long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_long);
}

int
nc_put_varm_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  const float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_float);
}

int
nc_put_varm_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride, const ptrdiff_t * imapp,
		   const double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_double);
}

int
nc_put_varm_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_ubyte);
}

int
nc_put_varm_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride, const ptrdiff_t * imapp,
		   const unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_ushort);
}

int
nc_put_varm_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride, const ptrdiff_t * imapp,
		 const unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_uint);
}

int
nc_put_varm_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t * stride, const ptrdiff_t * imapp,
		     const long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_longlong);
}

int
nc_put_varm_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t * stride, const ptrdiff_t * imapp,
		      const unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_varm_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride, const ptrdiff_t * imapp,
		   const char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/

int
nc_get_varm_text(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride, const ptrdiff_t * imapp,
		 char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_CHAR);
}

int
nc_get_varm_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_BYTE);
}

int
nc_get_varm_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_uchar);
}

int
nc_get_varm_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_SHORT);
}

int
nc_get_varm_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t * stride, const ptrdiff_t * imapp,
		int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_INT);
}

int
nc_get_varm_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride, const ptrdiff_t * imapp,
		 long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_long);
}

int
nc_get_varm_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_float);
}

int
nc_get_varm_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride, const ptrdiff_t * imapp,
		   double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_double);
}

int
nc_get_varm_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride, const ptrdiff_t * imapp,
		  unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_ubyte);
}

int
nc_get_varm_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride, const ptrdiff_t * imapp,
		   unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_ushort);
}

int
nc_get_varm_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride, const ptrdiff_t * imapp,
		 unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_uint);
}

int
nc_get_varm_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t * stride, const ptrdiff_t * imapp,
		     long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,T_longlong);
}

int
nc_get_varm_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t * stride, const ptrdiff_t * imapp,
		      unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_varm_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride, const ptrdiff_t * imapp,
		   char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_varm(ncid,varid,startp,countp,stride,imapp,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/

int
nc_put_vars_text(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride,
		 const char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,NC_CHAR);
}

int
nc_put_vars_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  const signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,NC_BYTE);
}

int
nc_put_vars_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_uchar);
}

int
nc_put_vars_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  const short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,NC_SHORT);
}

int
nc_put_vars_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t * stride,
		const int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,NC_INT);
}

int
nc_put_vars_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride,
		 const long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_long);
}

int
nc_put_vars_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  const float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_float);
}

int
nc_put_vars_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride,
		   const double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_double);
}

int
nc_put_vars_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  const unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_ubyte);
}

int
nc_put_vars_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride,
		   const unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_ushort);
}

int
nc_put_vars_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride,
		 const unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_uint);
}

int
nc_put_vars_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t * stride,
		     const long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,T_longlong);
}

int
nc_put_vars_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t * stride,
		      const unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_vars_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride,
		   const char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid,varid,startp,countp,stride,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/

int
nc_get_vars_text(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride,
		 char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,NC_CHAR);
}

int
nc_get_vars_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  signed char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,NC_BYTE);
}

int
nc_get_vars_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_uchar);
}

int
nc_get_vars_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,NC_SHORT);
}

int
nc_get_vars_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t * stride,
		int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,NC_INT);
}

int
nc_get_vars_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride,
		 long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_long);
}

int
nc_get_vars_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  float *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_float);
}

int
nc_get_vars_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride,
		   double *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_double);
}

int
nc_get_vars_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t * stride,
		  unsigned char *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_ubyte);
}

int
nc_get_vars_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride,
		   unsigned short *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_ushort);
}

int
nc_get_vars_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t * stride,
		 unsigned int *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_uint);
}

int
nc_get_vars_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t * stride,
		     long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,T_longlong);
}

int
nc_get_vars_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t * stride,
		      unsigned long long *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_vars_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stride,
		   char* *value)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid,varid,startp,countp,stride,(void*)value,NC_STRING);
}

#endif /*USE_NETCDF4*/
/**@}*/

