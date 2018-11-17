/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include <stdlib.h>
#include "config.h"
#include "netcdf.h"
#include "ncoffsets.h"
#include "ncreclaim.h"

/**
Provide a template implementation of
various netcdf3/4 recursive walkers.
The set of walkers currently includes:
1. reclaim - given type information and a pointer
   to a vector of instances, reclaim the whole data
   tree hanging off this vector.
2. clone - given type information and a pointer
   to a vector of instances, deep clone the whole data
   tree hanging off this vector.
Note the the tricky part occurs when we have variable length
data: strings or vlens.
*/

/* It is helpful to have a structure that contains memory and an offset */
typedef struct Position {char* memory; ptrdiff_t offset;} Position;

#ifndef emalloc
/* In case we need to override the memory allocator functions later */
#define emalloc(n) malloc(n)
#define ecalloc(n) calloc(1,n)
#define erealloc(p,n) realloc(p,n)
#define efree(p) free(p)
#endif

/*Forward*/
static int NC_reclaim_datar(NC_reclaim_ops* state, uintptr_t typeid, Position*);
static int NC_reclaim_usertype(NC_reclaim_ops* state, NC_reclaim_type* tsym, Position*);
static int NC_reclaim_vlen(NC_reclaim_ops* state, NC_reclaim_type* tsym, Position*);
static int NC_reclaim_enum(NC_reclaim_ops* state, NC_reclaim_type* tsym, Position*);
static int NC_reclaim_opaque(NC_reclaim_ops* state, NC_reclaim_type* tsym, Position*);
static int NC_reclaim_compound(NC_reclaim_ops* state, NC_reclaim_type* tsym, Position*);

static ptrdiff_t read_align(ptrdiff_t offset, size_t alignment);

/**
Reclaim a chunk of memory. Do not reclaim top level
because we do not know how it was allocated.

@param tsym - the root type node of the data to be reclaimed
@param memory - top-level memory whose content is to be reclaimed
@param count - number of instances of tsym in memory

*/

int
NC_reclaim_template(void* state, uintptr_t typeid, void* memory, size_t count)
{
    int stat = NC_NOERR;
    NC_reclaim_ops* ops = (NC_reclaim_ops*)state;
    size_t i;
    Position position;
    
    if(NULL == ((void*)typeid)
       || (memory == NULL && count > 0))
        {stat = NC_EINVAL; goto done;}
    if(memory == NULL || count == 0)
        goto done; /* ok, do nothing */
    position.offset = 0;
    position.memory = memory;
    for(i=0;i<count;i++) {
	if((stat=NC_reclaim_datar(ops,typeid,&position))) /* reclaim one instance */
	    break;
    }
done:
    return stat;
}

/* Recursive type walker: reclaim a single instance */
static int
NC_reclaim_datar(NC_reclaim_ops* ops, uintptr_t typeid, Position* position)
{
    int stat = NC_NOERR;
    NC_reclaim_type tsym;
    if((stat=ops->inq_type(ops,typeid,&tsym)))
	goto done;
    
    switch  (tsym.typeclass) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE:
    case NC_SHORT: case NC_USHORT:
    case NC_INT: case NC_UINT: case NC_FLOAT:
    case NC_INT64: case NC_UINT64: case NC_DOUBLE:
        position->offset += tsym.size;
	break;
    case NC_STRING: {
	char** sp = (char**)(position->memory+position->offset);
        /* Need to reclaim string */
	if(*sp != NULL) efree(*sp);
	position->offset += tsym.size;
	} break;
    default:
    	/* reclaim a user type */
	if((stat = NC_reclaim_usertype(ops,&tsym,position)))
	    goto done;
	break;
    }
done:
    return stat;
}
	
static int
NC_reclaim_usertype(NC_reclaim_ops* ops, NC_reclaim_type* tsym, Position* Position)
{
    int stat = NC_NOERR;

#ifdef USE_NETCDF4
    /* Get info about the xtype */
    switch (tsym->typeclass) {
    case NC_OPAQUE: stat = NC_reclaim_opaque(ops,tsym,Position); break;
    case NC_ENUM: stat = NC_reclaim_enum(ops,tsym,Position); break;
    case NC_VLEN: stat = NC_reclaim_vlen(ops,tsym,Position); break;
    case NC_COMPOUND: stat = NC_reclaim_compound(ops,tsym,Position); break;
#endif
    default:
        stat = NC_EINVAL;
	break;
    }
    return stat;
}

#ifdef USE_NETCDF4
static int
NC_reclaim_vlen(NC_reclaim_ops* ops, NC_reclaim_type* tsym, Position* position)
{
    int stat = NC_NOERR;
    size_t i;
    NC_reclaim_type basetype;
    Position vposition;
    nc_vlen_t* vl = (nc_vlen_t*)(position->memory+position->offset);

    if((stat=ops->inq_type(ops,tsym->basetype,&basetype)))
	goto done;

    /* Free up each entry in the vlen list */
    if(vl->p != NULL) {
	vposition.memory = vl->p;
	vposition.offset = 0;
        for(i=0;i<vl->len;i++) {
	    size_t alignment = ops->typealignment(ops,tsym->basetype);
	    vposition.offset = read_align(vposition.offset,alignment);
	    if((stat = NC_reclaim_datar(ops,tsym->basetype,&vposition))) goto done;
	    vposition.offset += basetype.size;
	}
	position->offset += tsym->size;
	efree(vl->p);
    }
done:
    return stat;
}

static int
NC_reclaim_enum(NC_reclaim_ops* ops, NC_reclaim_type* tsym, Position* position)
{
    return NC_reclaim_datar(ops,tsym->basetype,position);
}

static int
NC_reclaim_opaque(NC_reclaim_ops* ops, NC_reclaim_type* tsym, Position* position)
{
    /* basically a fixed size sequence of bytes */
    position->offset += tsym->size;
    return NC_NOERR;
}

static int
NC_reclaim_compound(NC_reclaim_ops* ops, NC_reclaim_type* tsym, Position* position)
{
    int stat = NC_NOERR;
    size_t fid, i, arraycount;
    ptrdiff_t saveoffset;
    size_t cmpdalign = ops->typealignment(ops,tsym->typeid);
    uintptr_t fieldtype;
    size_t fieldalign;
    int ndims;
    int dimsizes[NC_MAX_VAR_DIMS];

    position->offset = read_align(position->offset,cmpdalign);
    saveoffset = position->offset;

    /* Get info about each field in turn and reclaim it */
    for(fid=0;fid<tsym->nfields;fid++) {
	if((stat=ops->inq_compound_field(ops,tsym->typeid,fid,&fieldalign,&fieldtype,&ndims,dimsizes)))
	    goto done;
	fieldalign = ops->typealignment(ops,fieldtype);
	/* compute the total number of elements in the field array */
	for(i=0;i<ndims;i++) arraycount *= dimsizes[i];
	position->offset = read_align(position->offset,fieldalign);
	for(i=0;i<arraycount;i++) {
	    if((stat = NC_reclaim_datar(ops, fieldtype, position))) goto done;
	}		
    }
    position->offset = saveoffset;
    position->offset += tsym->size;
done:
    return stat;
}
#endif /*USE_NETCDF4*/

/**************************************************/
#if 0
int
NC_clone_data(void* tsym, void* src, void** dstp, size_t count)
{
    int stat = NC_NOERR;
    size_t i;
    Position srcpos;
    Position dstpos = {NULL,0};
    
    if(tsym == NULL)
        {stat = NC_EINVAL; goto done;}
    if(src == NULL && count > 0)
        {stat = NC_EINVAL; goto done;}
    if(src == NULL || count == 0)
        goto done; /* ok, do nothing */
    srcpos.offset = 0;
    srcpos.memory = src;
    for(i=0;i<count;i++) {
	if((stat=NC_clone_datar(tsym,&srcpos,&dstpos))) /* clone one instance */
	    break;
    }
done:
    return stat;
}

/* Recursive type walker: reclaim a single instance */
static int
NC_clone_datar(void* tsym, Position* src, Position* dst)
{
    int stat = NC_NOERR;
    
    switch  (tsym->nc_type_class) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE:
    case NC_SHORT: case NC_USHORT:
    case NC_INT: case NC_UINT: case NC_FLOAT:
    case NC_INT64: case NC_UINT64: case NC_DOUBLE:
        Position->offset += tsym->size;
	break;
    case NC_STRING: {
	char** sp = (char**)(Position->memory+Position->offset);
        /* Need to reclaim string */
	if(*sp != NULL) efree(*sp);
	Position->offset += tsym->size;
	} break;
    default:
    	/* reclaim a user type */
	stat = NC_clone_usertype(tsym,Position);
	break;
    }
    return stat;
}
	
static int
NC_clone_usertype(void* tsym, Position* Position)
{
    int stat = NC_NOERR;

    /* Get info about the xtype */
    switch (tsym->nc_type_class) {
    case NC_OPAQUE: stat = NC_clone_opaque(tsym,Position); break;
    case NC_ENUM: stat = NC_clone_enum(tsym,Position); break;
    case NC_VLEN: stat = NC_clone_vlen(tsym,Position); break;
    case NC_COMPOUND: stat = NC_clone_compound(tsym,Position); break;
    default:
        stat = NC_EINVAL;
	break;
    }
    return stat;
}

static int
NC_clone_vlen(void* tsym, Position* Position)
{
    int stat = NC_NOERR;
    size_t i;
    void* basetype;
    nc_vlen_t* vl = (nc_vlen_t*)(Position->memory+Position->offset);

    if((stat=nc4_find_type(tsym->container->nc4_info,tsym->u.v.base_nc_typeid,&basetype)))
	goto done;    

    /* Free up each entry in the vlen list */
    if(vl->p != NULL) {
	Reclaim vPosition;
	vPosition.memory = vl->p;
	vPosition.offset = 0;
        for(i=0;i<vl->len;i++) {
	    size_t alignment = hdf5typealignment(basetype);
	    vPosition.offset = read_align(vPosition.offset,alignment);
	    if((stat = NC_clone_datar(basetype,&vPosition))) goto done;
	    vPosition.offset += basetype->size;
	}
	Position->offset += tsym->size;
	efree(vl->p);
    }
done:
    return stat;
}

static int
NC_clone_enum(void* tsym, Position* Position)
{
    int stat = NC_NOERR;
    void* basetype;
    if((stat=nc4_find_type(tsym->container->nc4_info,tsym->u.v.base_nc_typeid,&basetype)))
	return stat;
    return NC_clone_datar(basetype,Position);
}

static int
NC_clone_opaque(void* tsym, Position* Position)
{
    /* basically a fixed size sequence of bytes */
    Position->offset += tsym->size;
    return NC_NOERR;
}

static int
NC_clone_compound(void* tsym, Position* Position)
{
    int stat = NC_NOERR;
    int nfields;
    size_t fid, i, arraycount;
    ptrdiff_t saveoffset;
    size_t cmpdalign = hdf5typealignment(tsym);

    Position->offset = read_align(Position->offset,cmpdalign);
    saveoffset = Position->offset;

    /* Get info about each field in turn and reclaim it */
    nfields = nclistlength(tsym->u.c.field);
    for(fid=0;fid<nfields;fid++) {
	NC_FIELD_INFO_T* field = nclistget(tsym->u.c.field,fid);
	void* fieldtype;
        int ndims = field->ndims;
	size_t fieldalign;
	if((stat=nc4_find_type(tsym->container->nc4_info,field->nc_typeid,&fieldtype)))
	    goto done;    
	fieldalign = hdf5typealignment(fieldtype);
	/* compute the total number of elements in the field array */
	for(i=0;i<ndims;i++) arraycount *= field->dim_size[i];
	Position->offset = read_align(Position->offset,fieldalign);
	for(i=0;i<arraycount;i++) {
	    if((stat = NC_clone_datar(fieldtype, Position))) goto done;
	}		
    }
    Position->offset = saveoffset;
    Position->offset += tsym->size;
done:
    return stat;
}
#endif

/**************************************************/
/* Alignment code */

static ptrdiff_t
read_align(ptrdiff_t offset, size_t alignment)
{
    size_t delta = (offset % alignment);
    if(delta == 0) return offset;
    return offset + (alignment - delta);
}

#if 0
/*
The heart of this is the following macro,
which computes the offset of a field x
when preceded by a char field.
The assumptions appear to be as follows:
1. the offset produced in this situation indicates
   the alignment for x relative in such a way that it
   depends only on the types that precede it in the struct.
2. the compiler does not reorder fields.
3. arrays are tightly packed.
4. nested structs are alignd according to their first member
   (this actually follows from C language requirement that
    a struct can legally be cast to an instance of its first member).
Given the alignments for the various common primitive types,
it is assumed that one can use them anywhere to construct
the layout of a struct of such types.
It seems to work for HDF5 for a wide variety of machines.
*/

static size_t
typealignment(void* typ)
{
    if(!nc_alignments_computed) {
	nc_compute_alignments();
	nc_alignments_computed = 1;
    }
    if(typ->hdr.id <= NC_MAX_ATOMIC_TYPE)
        return nctypealignment(typ->hdr.id);
    else {/* Presumably a user type */
	switch(typ->nc_type_class) {
        case NC_VLEN: return nctypealignment(typ->hdr.id);
        case NC_OPAQUE: return nctypealignment(typ->hdr.id);
        case NC_COMPOUND: {/* get alignment of the first field of the compound */
	   NC_FIELD_INFO_T* field0 = nclistget(typ->u.c.field,0);
   	   void* fieldtype = NULL;
	   if(nc4_find_type(typ->container->nc4_info,field0->nc_typeid,&fieldtype))
	   	return 0;
	   return hdf5typealignment(fieldtype); /* may recurse repeatedly */
	} break;
        default: break;
	}
    }
    return 0; /* fail */
}
#endif
