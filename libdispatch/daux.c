/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "config.h"
#include "netcdf.h"
#include "netcdf_aux.h"
#include "ncoffsets.h"
#include "ncreclaim.h"
#include "nclog.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

struct NCAUX_FIELD {
    char* name;
    nc_type fieldtype;
    size_t ndims;
    int dimsizes[NC_MAX_VAR_DIMS];    
    size_t size;
    size_t offset;
    size_t alignment;
};

struct NCAUX_CMPD {
    int ncid;
    int mode;
    char* name;
    size_t nfields;
    struct NCAUX_FIELD* fields;
    size_t size;
    size_t offset; /* cumulative as fields are added */
    size_t alignment;
};

typedef struct NCAUX_RECLAIM_STATE {
    NC_reclaim_ops ops;
    int ncid;
} NCAUX_RECLAIM_STATE;

static int ncaux_initialized = 0;

#ifdef USE_NETCDF4
static int computefieldinfo(struct NCAUX_CMPD* cmpd);
#endif /* USE_NETCDF4 */

/**************************************************/

static int
ncaux_inq_type(void* state0, uintptr_t typeid, NC_reclaim_type* typeinfo)
{
    int ret = NC_NOERR;
    NCAUX_RECLAIM_STATE* state = (NCAUX_RECLAIM_STATE*)state0;
    nc_type xtype = (nc_type)typeid;

    typeinfo->typeid = typeid;
    if((ret=nc_inq_type(state->ncid,xtype,NULL,&typeinfo->size))) goto done;
    if(xtype <= NC_MAX_ATOMIC_TYPE) {
    } else {
	size_t baseid;
	if((ret=nc_inq_user_type(state->ncid,xtype,NULL,&typeinfo->size,&baseid,&typeinfo->nfields,&typeinfo->typeclass)))
	    goto done;
	typeinfo->basetype = (uintptr_t)baseid;
    }
done:
    return ret;
}

static int
ncaux_inq_compound_field(void* state0, uintptr_t cmpd, int fieldid, size_t* fieldalign, uintptr_t* fieldtypep, int* ndims, int* dimsizes)
{
    int ret = NC_NOERR;
    NCAUX_RECLAIM_STATE* state = (NCAUX_RECLAIM_STATE*)state0;
    nc_type xtype = (nc_type)cmpd;
    nc_type fieldtype;

    if((ret=nc_inq_compound_field(state->ncid,xtype, fieldid, NULL, fieldalign,
				     &fieldtype, ndims, dimsizes))) goto done;
    if(fieldtypep) *fieldtypep = (uintptr_t)fieldtype;
done:
    return ret;
}

static int
ncaux_typealignment(void* state0, uintptr_t typeid)
{
    int ret = NC_NOERR;
    NCAUX_RECLAIM_STATE* state = (NCAUX_RECLAIM_STATE*)state0;
    nc_type xtype = (nc_type)typeid;
    int klass = NC_NAT;

    if(!NC_alignments_computed) {
	NC_compute_alignments();
	NC_alignments_computed = 1;
    }
    if(xtype <= NC_MAX_ATOMIC_TYPE)
        {ret = NC_class_alignment(xtype); goto done;} /* type == class */
#ifdef USE_NETCDF4
    else {/* Presumably a user type */
        if((ret = nc_inq_user_type(state->ncid,xtype,NULL,NULL,NULL,NULL,&klass))) goto done;
	switch(klass) {
        case NC_VLEN: return NC_class_alignment(klass);
        case NC_OPAQUE: return NC_class_alignment(klass);
        case NC_COMPOUND: {/* get alignment of the first field of the compound */
	   int fieldtype = NC_NAT;
	   if((ret=nc_inq_compound_fieldtype(state->ncid,xtype,0,&fieldtype))) goto done;
           if((ret = nc_inq_user_type(state->ncid,fieldtype,NULL,NULL,NULL,NULL,&klass))) goto done;
	   return ncaux_typealignment(state0, fieldtype); /* may recurse repeatedly */
	} break;
        default: break;
	}
    }
#endif /*USE_NETCDF4 */

done:
    return ret;
}

/**************************************************/

/**
Reclaim the output tree of data from a call
to e.g. nc_get_vara or the input to e.g. nc_put_vara.
This recursively walks the top-level instances to
reclaim any nested data such as vlen or strings or such.

Assumes it is passed a pointer to count instances of xtype.
Reclaims any nested data.
WARNING: does not reclaim the top-level memory because
we do not know how it was allocated.

Should work for any netcdf format.

@param ncid file ncid
@param xtype type id
@param memory to reclaim
@param count number of instances of the type in memory
@return error code
*/

EXTERNL int
ncaux_reclaim_data(int ncid, int xtype, void* memory, size_t count)
{
    int stat = NC_NOERR;
    NCAUX_RECLAIM_STATE state;

    state.ops.inq_type = ncaux_inq_type;
    state.ops.inq_compound_field = ncaux_inq_compound_field;
    state.ops.typealignment = ncaux_typealignment;
    state.ncid = ncid;
    
    if(ncid < 0 || xtype < 0
       || (memory == NULL && count > 0)
       || xtype == NC_NAT)
        {stat = NC_EINVAL; goto done;}
    if(memory == NULL || count == 0)
        goto done; /* ok, do nothing */
    stat = NC_reclaim_template(&state,(uintptr_t)xtype,memory,count);
done:
    return stat;
}

/**************************************************/

/*
This code is a variant of the H5detect.c code from HDF5.
Author: D. Heimbigner 10/7/2008
*/

EXTERNL int
ncaux_begin_compound(int ncid, const char *name, int alignmode, void** tagp)
{
#ifdef USE_NETCDF4
    int status = NC_NOERR;
    struct NCAUX_CMPD* cmpd = NULL;

    if(tagp) *tagp = NULL;
	
    cmpd = (struct NCAUX_CMPD*)calloc(1,sizeof(struct NCAUX_CMPD));
    if(cmpd == NULL) {status = NC_ENOMEM; goto fail;}
    cmpd->ncid = ncid;
    cmpd->mode = alignmode;
    cmpd->nfields = 0;
    cmpd->name = strdup(name);    
    if(cmpd->name == NULL) {status = NC_ENOMEM; goto fail;}

    if(tagp) {
      *tagp = (void*)cmpd;
    } else { /* Error, free cmpd to avoid memory leak. */
      free(cmpd);
    }
    return status;

fail:
    ncaux_abort_compound((void*)cmpd);
    return status;
#else
    return NC_ENOTBUILT;
#endif
}

EXTERNL int
ncaux_abort_compound(void* tag)
{
#ifdef USE_NETCDF4
    int i;
    struct NCAUX_CMPD* cmpd = (struct NCAUX_CMPD*)tag;
    if(cmpd == NULL) goto done;
    if(cmpd->name) free(cmpd->name);
    for(i=0;i<cmpd->nfields;i++) {
	struct NCAUX_FIELD* field = &cmpd->fields[i];
	if(field->name) free(field->name);
    }
    if(cmpd->fields) free(cmpd->fields);
    free(cmpd);

done:
    return NC_NOERR;
#else
    return NC_ENOTBUILT;
#endif
}

EXTERNL int
ncaux_add_field(void* tag,  const char *name, nc_type field_type,
			   int ndims, const int* dimsizes)
{
#ifdef USE_NETCDF4
    int i;
    int status = NC_NOERR;
    struct NCAUX_CMPD* cmpd = (struct NCAUX_CMPD*)tag;
    struct NCAUX_FIELD* newfields = NULL;
    struct NCAUX_FIELD* field = NULL;

    if(cmpd == NULL) goto done;
    if(ndims < 0) {status = NC_EINVAL; goto done;}
    for(i=0;i<ndims;i++) {
	if(dimsizes[i] <= 0) {status = NC_EINVAL; goto done;}
    }
    if(cmpd->fields == NULL) {
        newfields = (struct NCAUX_FIELD*)calloc(1,sizeof(struct NCAUX_FIELD));
    } else {
        newfields = (struct NCAUX_FIELD*)realloc(cmpd->fields,cmpd->nfields+1*sizeof(struct NCAUX_FIELD));
    }
    if(cmpd->fields == NULL) {status = NC_ENOMEM; goto done;}
    cmpd->fields = newfields;
    field = &cmpd->fields[cmpd->nfields+1];
    field->name = strdup(name);
    field->fieldtype = field_type;
    if(field->name == NULL) {status = NC_ENOMEM; goto done;}    
    field->ndims = (size_t)ndims;
    memcpy(field->dimsizes,dimsizes,sizeof(int)*field->ndims);
    cmpd->nfields++;

done:
    if(newfields)
      free(newfields);
    return status;
#else
    return NC_ENOTBUILT;
#endif
}

EXTERNL int
ncaux_end_compound(void* tag, nc_type* idp)
{
#ifdef USE_NETCDF4
    int i;
    int status = NC_NOERR;
    struct NCAUX_CMPD* cmpd = (struct NCAUX_CMPD*)tag;

    if(cmpd == NULL) {status = NC_EINVAL; goto done;}

    /* Compute field and compound info */
    status = computefieldinfo(cmpd);
    if(status != NC_NOERR) goto done;

    status = nc_def_compound(cmpd->ncid, cmpd->size, cmpd->name, idp);
    if(status != NC_NOERR) goto done;

    for(i=0;i<cmpd->nfields;i++) {
	struct NCAUX_FIELD* field = &cmpd->fields[i];
	if(field->ndims > 0) {
            status = nc_insert_compound(cmpd->ncid, *idp, field->name,
					field->offset, field->fieldtype);
	} else {
            status = nc_insert_array_compound(cmpd->ncid, *idp, field->name,
					field->offset, field->fieldtype,
					(int)field->ndims,field->dimsizes);
	}
        if(status != NC_NOERR) goto done;
    }

done:
    return status;
#else
    return NC_ENOTBUILT;
#endif
}

/**************************************************/

/**
 @param ncclass - type class for which alignment is requested; excludes ENUM|COMPOUND
*/
EXTERNL size_t
ncaux_class_alignment(int ncclass)
{
    if(ncclass <= NC_MAX_ATOMIC_TYPE || ncclass == NC_VLEN || ncclass == NC_OPAQUE)
        return NC_class_alignment(ncclass);
    nclog(NCLOGERR,"ncaux_class_alignment: class %d; alignment cannot be determermined",ncclass);
    return 0;
}

/**
 @param ncid - only needed for a compound type
 @param xtype - type for which alignment is requested
*/
EXTERNL size_t
ncaux_type_alignment(int xtype, int ncid)
{
    if(!NC_alignments_computed) {
	NC_compute_alignments();
	ncaux_initialized = 1;
    }
    if(xtype <= NC_MAX_ATOMIC_TYPE)
        return NC_class_alignment(xtype); /* type == class */
#ifdef USE_NETCDF4
    else {/* Presumably a user type */
	int klass = NC_NAT;
        int stat = nc_inq_user_type(ncid,xtype,NULL,NULL,NULL,NULL,&klass);
	if(stat) goto done;
	switch(klass) {
        case NC_VLEN: return NC_class_alignment(klass);
        case NC_OPAQUE: return NC_class_alignment(klass);
        case NC_COMPOUND: {/* get alignment of the first field of the compound */
	   int fieldtype = NC_NAT;
	   if((stat=nc_inq_compound_fieldtype(ncid,xtype,0,&fieldtype))) goto done;
	   return ncaux_type_alignment(fieldtype,ncid); /* may recurse repeatedly */
	} break;
        default: break;
	}
    }

done:
#endif /*USE_NETCDF4 */
    return 0; /* fail */
}

#ifdef USE_NETCDF4
/* Find first primitive field of a possibly nested sequence of compounds */
static nc_type
findfirstfield(int ncid, nc_type xtype)
{
    int status = NC_NOERR;
    nc_type fieldtype = xtype;
    if(xtype <= NC_MAX_ATOMIC_TYPE) goto done;
        
    status = nc_inq_compound_fieldtype(ncid, xtype, 0, &fieldtype);
    if(status != NC_NOERR) goto done;
    fieldtype = findfirstfield(ncid,fieldtype);

done:
    return (status == NC_NOERR?fieldtype:NC_NAT);
}

static size_t
getpadding(size_t offset, size_t alignment)
{
    size_t rem = (alignment==0?0:(offset % alignment));
    size_t pad = (rem==0?0:(alignment - rem));
    return pad;
}

static size_t
dimproduct(size_t ndims, int* dimsizes)
{
    int i;
    size_t product = 1;
    for(i=0;i<ndims;i++) product *= (size_t)dimsizes[i];
    return product;
}

static int
computefieldinfo(struct NCAUX_CMPD* cmpd)
{
    int i;
    int status = NC_NOERR;
    size_t offset = 0;
    size_t totaldimsize;

    /* Assign the sizes for the fields */
    for(i=0;i<cmpd->nfields;i++) {
	struct NCAUX_FIELD* field = &cmpd->fields[i];	
	status = nc_inq_type(cmpd->ncid,field->fieldtype,NULL,&field->size);
        if(status != NC_NOERR) goto done;
	totaldimsize = dimproduct(field->ndims,field->dimsizes);
	field->size *= totaldimsize;
    }

    for(offset=0,i=0;i<cmpd->nfields;i++) {
        struct NCAUX_FIELD* field = &cmpd->fields[i];
	int alignment = 0;
	nc_type firsttype = findfirstfield(cmpd->ncid,field->fieldtype);

        /* only support 'c' alignment for now*/
	switch (field->fieldtype) {
	case NC_OPAQUE:
	    field->alignment = 1;
	    break;
	case NC_ENUM:
            field->alignment = ncaux_type_alignment(firsttype,cmpd->ncid);
	    break;	
	case NC_VLEN: /*fall thru*/
	case NC_COMPOUND:
            field->alignment = ncaux_type_alignment(firsttype,cmpd->ncid);
	    break;
	default:
            field->alignment = ncaux_type_alignment(field->fieldtype,cmpd->ncid);
	    break;

	}
        offset += getpadding(offset,alignment);
        field->offset = offset;
        offset += field->size;
    }
    cmpd->size = offset;
    cmpd->alignment = cmpd->fields[0].alignment;

done:
    return status;
}

#endif /*USE_NETCDF4*/


