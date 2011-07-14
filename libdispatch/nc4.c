/*! \file
Functions for netCDF-4 features.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See \ref COPYRIGHT file for more info. */

#include "ncdispatch.h"

/** \defgroup user_types User-Defined Types

User defined types allow for more complex data structures.

NetCDF-4 has added support for four different user defined data
types. User defined type may only be used in files created with the
::NC_NETCDF4 and without ::NC_CLASSIC_MODEL.
- compound type: like a C struct, a compound type is a collection of
types, including other user defined types, in one package.  
- variable length array type: used to store ragged arrays.
- opaque type: This type has only a size per element, and no other
  type information.
- enum type: Like an enumeration in C, this type lets you assign text
  values to integer values, and store the integer values.

Users may construct user defined type with the various nc_def_*
functions described in this section. They may learn about user defined
types by using the nc_inq_ functions defined in this section.

Once types are constructed, define variables of the new type with
nc_def_var (see nc_def_var). Write to them with nc_put_var1,
nc_put_var, nc_put_vara, or nc_put_vars. Read data of user-defined
type with nc_get_var1, nc_get_var, nc_get_vara, or nc_get_vars (see
\ref variables).

Create attributes of the new type with nc_put_att (see nc_put_att_
type). Read attributes of the new type with nc_get_att (see
\ref attributes).
*/
/** \{ */ 

/** Free memory in a VLEN object. 

When you read VLEN type the library will actually allocate the storage
space for the data. This storage space must be freed, so pass the
pointer back to this function, when you're done with the data, and it
will free the vlen memory.

The function nc_free_vlens() is more useful than this function,
because it can free an array of VLEN objects.

\param vl pointer to the vlen object.

\returns ::NC_NOERR No error.
*/
int
nc_free_vlen(nc_vlen_t *vl)
{
   free(vl->p);
   return NC_NOERR;
}

/** Free an array of vlens given the number of elements and an array. 

When you read VLEN type the library will actually allocate the storage
space for the data. This storage space must be freed, so pass the
pointer back to this function, when you're done with the data, and it
will free the vlen memory.

\param len number of elements in the array.
\param vlens pointer to the vlen object.

\returns ::NC_NOERR No error.
*/ 
int
nc_free_vlens(size_t len, nc_vlen_t vlens[])
{
   int ret;
   size_t i;

   for(i = 0; i < len; i++) 
      if ((ret = nc_free_vlen(&vlens[i])))
	 return ret;

   return NC_NOERR;
}

int
nc_inq_type_equal(int ncid1, nc_type typeid1, int ncid2, 
		  nc_type typeid2, int *equal)
{
    NC* ncp1;
    int stat = NC_check_id(ncid1,&ncp1);
    if(stat != NC_NOERR) return stat;
    return ncp1->dispatch->inq_type_equal(ncid1,typeid1,ncid2,typeid2,equal);
}

int
nc_inq_typeid(int ncid, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_typeid(ncid,name,typeidp);
}

int
nc_def_vlen(int ncid, const char *name, nc_type base_typeid, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_vlen(ncid,name,base_typeid,xtypep);
}

int
nc_inq_vlen(int ncid, nc_type xtype, char *name, size_t *datum_sizep, nc_type *base_nc_typep)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,datum_sizep,base_nc_typep,NULL,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_VLEN) stat = NC_EBADTYPE;
    return stat;
}

int
nc_put_vlen_element(int ncid, int typeid1, void *vlen_element, size_t len, const void *data)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_vlen_element(ncid,typeid1,vlen_element,len,data);
}

int
nc_get_vlen_element(int ncid, int typeid1, const void *vlen_element, size_t *len, void *data)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_vlen_element(ncid,typeid1,vlen_element,len,data);
}

int
nc_inq_user_type(int ncid, nc_type xtype, char *name, size_t *size, nc_type *base_nc_typep, size_t *nfieldsp, int *classp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_user_type(ncid,xtype,name,size,base_nc_typep,nfieldsp,classp);
}

int
nc_def_enum(int ncid, nc_type base_typeid, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_enum(ncid,base_typeid,name,typeidp);
}

int
nc_insert_enum(int ncid, nc_type xtype, const char *name, const void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->insert_enum(ncid,xtype,name,value);
}

int
nc_inq_enum(int ncid, nc_type xtype, char *name, nc_type *base_nc_typep, size_t *base_sizep, size_t *num_membersp)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,base_sizep,base_nc_typep,num_membersp,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_ENUM) stat = NC_EBADTYPE;
    return stat;
}

int
nc_inq_enum_member(int ncid, nc_type xtype, int idx, char *name, void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_enum_member(ncid,xtype,idx,name,value);
}

int
nc_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_enum_ident(ncid,xtype,value,identifier);
}

int
nc_def_opaque(int ncid, size_t size, const char *name, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_opaque(ncid,size,name,xtypep);
}

int
nc_inq_opaque(int ncid, nc_type xtype, char *name, size_t *sizep)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,sizep,NULL,NULL,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_OPAQUE) stat = NC_EBADTYPE;
    return stat;
}
/** \} */ 

/** \defgroup groups Groups

NetCDF-4 added support for hierarchical groups within netCDF datasets.

Groups are identified with a ncid, which identifies both the open
file, and the group within that file. When a file is opened with
nc_open or nc_create, the ncid for the root group of that file is
provided. Using that as a starting point, users can add new groups, or
list and navigate existing groups.

All netCDF calls take a ncid which determines where the call will take
its action. For example, the nc_def_var function takes a ncid as its
first parameter. It will create a variable in whichever group its ncid
refers to. Use the root ncid provided by nc_create or nc_open to
create a variable in the root group. Or use nc_def_grp to create a
group and use its ncid to define a variable in the new group.

Variable are only visible in the group in which they are defined. The
same applies to attributes. “Global” attributes are associated with
the group whose ncid is used.

Dimensions are visible in their groups, and all child groups.

Group operations are only permitted on netCDF-4 files - that is, files
created with the HDF5 flag in nc_create(). Groups are not compatible
with the netCDF classic data model, so files created with the
::NC_CLASSIC_MODEL file cannot contain groups (except the root group).

 */
/** \{ */ 
int
nc_inq_ncid(int ncid, const char *name, int *grp_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_ncid(ncid,name,grp_ncid);
}

int
nc_inq_grps(int ncid, int *numgrps, int *ncids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grps(ncid,numgrps,ncids);
}

int
nc_inq_grpname(int ncid, char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grpname(ncid,name);
}

int
nc_inq_grpname_full(int ncid, size_t *lenp, char *full_name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grpname_full(ncid,lenp,full_name);
}

int
nc_inq_grpname_len(int ncid, size_t *lenp)
{
    int stat = nc_inq_grpname_full(ncid,lenp,NULL);    
    return stat;
}

int
nc_inq_grp_parent(int ncid, int *parent_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grp_parent(ncid,parent_ncid);
}

/* This has same semantics as nc_inq_ncid */ 
int
nc_inq_grp_ncid(int ncid, const char *grp_name, int *grp_ncid)
{
    return nc_inq_ncid(ncid,grp_name,grp_ncid);    
}

int
nc_inq_grp_full_ncid(int ncid, const char *full_name, int *grp_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grp_full_ncid(ncid,full_name,grp_ncid);
}

int 
nc_inq_varids(int ncid, int *nvars, int *varids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_varids(ncid,nvars,varids);
}

int 
nc_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_dimids(ncid,ndims,dimids,include_parents);
}

int 
nc_inq_typeids(int ncid, int *ntypes, int *typeids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_typeids(ncid,ntypes,typeids);
}

int
nc_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
    NC* ncp;
    int stat = NC_check_id(parent_ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_grp(parent_ncid,name,new_ncid);
}



int 
nc_show_metadata(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->show_metadata(ncid);
}

/** \} */ 
