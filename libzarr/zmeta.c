/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
 * @file
 * @internal The ZARR functions to interact with libsrc4
 *
 * @author Dennis Heimbigner
 */

#include "zincludes.h"
#include <stdarg.h>
#include "nczinternal.h"
#include "ncoffsets.h"

/**
 * Provide an API to build a netcdf4 node tree (via libhdf4)
 */

/***************************************************/
/* Forwards */

/***************************************************/
/* API */

/**
@internal Create the topmost dataset object
@param NC - [in] the containing NC object
@param filep - [out] created file object
@return NC_NOERR
*/
int
NCZ_make_dataset(NC* nc, NC_FILE_INFO_T** filep)
{
    int stat = NC_NOERR;
    NC_FILE_INFO_T* file = NULL;
    if((stat=nc4_nc4f_list_add(nc, nc->path, nc->mode))) goto done;
    if(filep) *filep = (NC_FILE_INFO_T*)nc->dispatchdata;
done:
    return stat;
}

/**
@internal Create the root group object
@param dataset - [in] the root dataset object
@param rootp - [out] created root group
@return NC_NOERR
*/
int
NCZ_make_root_group(NC_FILE_INFO_T* dataset, NC_GRP_INFO_T** rootp)
{
    int stat = NC_NOERR;
    NC_GRP_INFO_T* root = NULL;
    if((stat=nc4_grp_list_add(dataset, NULL, NULL, rootp))) goto done;

done:
    return stat;
}

/**
@internal Create a group
@param parent - [in] the containing group
@param u8name - [in] the UTF8 name
@param grpp - [out] created group
@return NC_NOERR
*/
int
NCZ_make_group(NC_GRP_INFO_T* parent, char* u8name, NC_GRP_INFO_T* grpp)
{
    int stat = NC_NOERR;
    NC_GRP_INFO_T* grp = NULL;
    if((stat=nc4_grp_list_add(dataset, parent, u8name, grpp))) goto done;
done:
    return stat;
}

/**
@internal Create a dimension
@param parent - [in] the containing group
@param u8name - [in] the UTF8 name (NULL => anonymous)
@param len - [in] the dimension length
@param dimp - [out] created dimension
@return NC_NOERR
*/
int
NCZ_make_dim(NC_GRP_INFO_T* parent, char* u8name, lsize_t len, NC_DIM_INFO_T** dimp)
{
    int stat = NC_NOERR;
    NC_DIM_INFO_T* dim = NULL;
    if((stat=nc4_grp_list_add(parent, u8name, (size_t)len, 0, dimp))) goto done;
done:
    return stat;
}

/**
@internal Create an Enumeration type 
@param parent - [in] the containing group
@param u8name - [in] the UTF8 name
@param basetype - [in] the basetype
@param enump - [out] created enumeration
@return NC_NOERR
*/
int
NCZ_make_enum(NC_GRP_INFO_T* parent, char* u8name, nc_type basetype, NC_TYPE_INFO_T** enump)
{
    int stat = NC_NOERR;
    NC_TYPE_INFO_T* en = NULL;
    if((stat=nc4_type_list_add(parent, basetype->size, u8name, &en))) goto done;
    /* Mark the type as an enumeration */
    en->nc_type_class = NC_ENUM;
    en->u.e.base_nc_typeid = basetype;
    en->u.e.enum_member = nclistnew();
    if(enump) *enump = en;
done:
    return stat;
}

/**
@internal Create an enumeration constant
@param parent - [in] the containing enumeration
@param u8name - [in] the UTF8 name
@param value - [in] the constant value
@return NC_NOERR
*/
int
NCZ_make_enumconst(NC_TYPE_INFO_T* en, char* u8name, void* value)
{
    int stat = NC_NOERR;
    if((stat=nc4_enum_member_add(en, basetype->size, u8name, value))) goto done;
done:
    return stat;
}

/**
@internal Create a compound type
@param parent - [in] the containing group
@param u8name - [in] the UTF8 name
@param fields - [in] the fields for the type
@param cmpdp - [out] created compound type
@return NC_NOERR
*/
int
NCZ_make_cmpd(NC_GRP_INFO_T* parent, char* u8name, NClist* fields, NC_TYPE_INFO_T** cmpdp)
{
    int stat = NC_NOERR;
    NC_TYPE_INFO_T* cmpd = NULL;
    int i;
    size_t totalsize, offset;


    /* First, compute the total size and offsets of the compound
       assuming no alignment */
    totalsize = 0;
    offset = 0;
    for(i=0;i<nclistlength(fields);i++) {
	FIELD* fld = nclistget(fields,i);
	totalsize += computesize(fld);
	fld->offset = offset;
	offset += typesize(fld->basetype);
    }
    /* Create the compound type */
    if((stat=nc4_type_list_add(parent, totalsize, u8name, &cmpd))) goto done;
    /* Mark the type as compound */
    cmpd->nc_type_class = NC_COMPOUND;
    cmpd->u.c.fields = nclistnew();
    /* insert the field set */
    for(i=0;i<nclistlength(fields);i++) {
        FIELD* fld = nclistget(fields,i);
	int dimsizes[NC_MAX_VAR_DIMS];
	int j;
	for(j=0;j<nclistlength(fld->dims);j++)
	    dimsizes[j] = (int)((uintptr_t)nclistget(fld->dims,j));
        if((stat=nc4_field_list_add(cmpd, fld->name, fld->offset,
				    field->basetype,nclistlength(fld->dims),
				    dimsizes))) goto done;
    }
    if(cmpdp) *cmpdp = cmpd;
done:
    return stat;
}

/**
@internal Create a variable
@param parent - [in] the containing group
@param u8name - [in] the UTF8 name
@param basetype - [in] basetype
@param dims - [in] NClist<NC_DIM_INFO_T*> dimension references (|dims|==0 => scalar)
@param varp - [out] created variable
@return NC_NOERR
*/
int
NCZ_make_var(NC_GRP_INFO_T* parent, char* u8name, nc_type basetype,
		NClist* dims, NC_VAR_INFO_T** varp)
{
    int stat = NC_NOERR;
    NC_VAR_INFO_T* var = NULL;
    int i;

    if((stat=nc4_var_list_add(parent, u8name, nclistlength(dims), &var))) goto done;
    for(i=0;i<nclistlength(dims;i++) {
	NC_DIM_INFO_T* dim = nclistget(dims,i);
	var->dimids[i] = dim->hdr.id;
    }
    var->type_info = findtype(basetype);
    if(varp) *varp = var;
done:
    return stat;
}

/**
@internal Create an attribute
@param parent - [in] the containing group or variable
@param u8name - [in] the UTF8 name
@param basetype - [in] basetype
@param attp - [out] created attribute
@return NC_NOERR
*/
int
NCZ_make_attr(NC_OBJ* parent, char* u8name, nc_type basetype, NC_ATT_INFO_T** attp)
{
    int stat = NC_NOERR;
    NC_ATT_INFO_T* att = NULL;
    NCindex* attrs = NULL;

    if(parent->sort == NC_GROUP) {
	attrs = ((NC_VAR_INFO_T*)object)->att;
    } else {
	attrs = ((NC_GRP_INFO_T*)object)->att;
    }

    if((stat=nc4_att_list_add(attrs,u8name,&att))) goto done;
    att->nc_typeid = basetype;

    if(attrp) *attrp = att;
done:
    return stat;
}

/**
@internal Annotate an object with its zarr specific data
@param object - [in] object to annotate
@param format_data - [in] annotation
@return NC_NOERR
*/
int
NCZ_annotate(NC_OBJ* object, void* format_data)
{
    int stat = NC_NOERR;
    switch (object->sort) {
    case NCGRP: (((NC_GROUP_INFO_T*)object)->format_grp_info = format_data; break;
    case NCVAR: (((NC_VAR_INFO_T*)object)->format_var_info = format_data; break;
    case NCDIM: (((NC_DIM_INFO_T*)object)->format_dim_info = format_data; break;
    case NCATT: (((NC_ATT_INFO_T*)object)->format_att_info = format_data; break;
    case NCTYP: (((NC_TYPE_INFO_T*)object)->format_type_info = format_data; break;
    case NCFLD: (((NC_FIELD_INFO_T*)object)->format_field_info = format_data; break;
    default: return NC_EINTERNAL;
    }
    return NC_NOERR;
done:
    return stat;
}



