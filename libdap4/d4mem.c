/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include <stdlib.h>
#include "netcdf.h"
#include "ncoffsets.h"

/* It is helpful to have a structure that contains memory and an offset */
typedef struct Position {char* memory; ptrdiff_t offset;} Position;

#ifndef emalloc
#define emalloc(n) malloc(n)
#define ecalloc(n) calloc(1,n)
#define erealloc(p,n) realloc(p,n)
#define efree(p) free(p)
#endif

/*Forward*/
static int d4reclaim_datar(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4reclaim_usertype(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4reclaim_vlen(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4reclaim_enum(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4reclaim_opaque(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4reclaim_compound(NC_TYPE_INFO_T* tsym, Position* reclaimer);

static int d4clone_datar(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4clone_usertype(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4clone_vlen(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4clone_enum(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4clone_opaque(NC_TYPE_INFO_T* tsym, Position* reclaimer);
static int d4clone_compound(NC_TYPE_INFO_T* tsym, Position* reclaimer);

static ptrdiff_t read_align(ptrdiff_t offset, unsigned long alignment);
static size_t hdf5typealignment(NC_TYPE_INFO_T* type);

/**
Reclaim a chunk of memory. Do not reclaim top level
because we do not know how it was allocated.

@param tsym - the root type node of the data to be reclaimed
@param memory - top-level memory whose content is to be reclaimed
@param count - number of instances of tsym in memory

*/

int
d4reclaim_data(NCD4node* tsym, void* memory, size_t count)
{
    int stat = NC_NOERR;
    size_t i;
    Position position;
    
    if(tsym == NULL
       || (memory == NULL && count > 0))
        {stat = NC_EINVAL; goto done;}
    if(memory == NULL || count == 0)
        goto done; /* ok, do nothing */
    if(tsym->meta.isfixedsize)
    	goto done; /* Should be nothing hanging off of the memory chunk */
    position.offset = 0;
    position.memory = memory;
    for(i=0;i<count;i++) {
	if((stat=d4reclaim_datar(tsym,&position))) /* reclaim one instance */
	    break;
    }
done:
    return stat;
}

/* Recursive type walker: reclaim a single instance */
static int
d4reclaim_datar(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    
    switch  (tsym->subsort) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE:
    case NC_SHORT: case NC_USHORT:
    case NC_INT: case NC_UINT: case NC_FLOAT:
    case NC_INT64: case NC_UINT64: case NC_DOUBLE:
        position->offset += tsym->meta.memsize;
	break;
    case NC_STRING: {
	char** sp = (char**)(position->memory+position->offset);
        /* Need to reclaim string */
	if(*sp != NULL) efree(*sp);
	position->offset += tsym->meta.memsize;
	} break;
    default:
    	/* reclaim a user type */
	stat = d4reclaim_usertype(tsym,position);
	break;
    }
    return stat;
}
	
static int
d4reclaim_usertype(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;

    /* Get info about the xtype */
    switch (tsym->nc_type_class) {
    case NC_OPAQUE: stat = d4reclaim_opaque(tsym,position); break;
    case NC_ENUM: stat = d4reclaim_enum(tsym,position); break;
    case NC_VLEN: stat = d4reclaim_vlen(tsym,position); break;
    case NC_COMPOUND: stat = d4reclaim_compound(tsym,position); break;
    default:
        stat = NC_EINVAL;
	break;
    }
    return stat;
}

static int
d4reclaim_vlen(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    size_t i;
    NCD4node* basetype;
    nc_vlen_t* vl = (nc_vlen_t*)(position->memory+position->offset);

    if((stat=nc4_find_type(tsym->container->nc4_info,tsym->u.v.base_nc_typeid,&basetype)))
	goto done;    

    /* Free up each entry in the vlen list */
    if(vl->p != NULL) {
	Reclaim vposition;
	vposition.memory = vl->p;
	vposition.offset = 0;
        for(i=0;i<vl->len;i++) {
	    size_t alignment = hdf5typealignment(basetype);
	    vposition.offset = read_align(vposition.offset,alignment);
	    if((stat = d4reclaim_datar(basetype,&vposition))) goto done;
	    vposition.offset += basetype->size;
	}
	position->offset += tsym->size;
	efree(vl->p);
    }
done:
    return stat;
}

static int
d4reclaim_enum(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    NCD4node* basetype;
    if((stat=nc4_find_type(tsym->container->nc4_info,tsym->u.v.base_nc_typeid,&basetype)))
	return stat;
    return d4reclaim_datar(basetype,position);
}

static int
d4reclaim_opaque(NCD4node* tsym, Position* position)
{
    /* basically a fixed size sequence of bytes */
    position->offset += tsym->size;
    return NC_NOERR;
}

static int
d4reclaim_compound(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    int nfields;
    size_t fid, i, arraycount;
    ptrdiff_t saveoffset;
    size_t cmpdalign = hdf5typealignment(tsym);

    position->offset = read_align(position->offset,cmpdalign);
    saveoffset = position->offset;

    /* Get info about each field in turn and reclaim it */
    nfields = nclistlength(tsym->u.c.field);
    for(fid=0;fid<nfields;fid++) {
	NC_FIELD_INFO_T* field = nclistget(tsym->u.c.field,fid);
	NCD4node* fieldtype;
        int ndims = field->ndims;
	size_t fieldalign;
	if((stat=nc4_find_type(tsym->container->nc4_info,field->nc_typeid,&fieldtype)))
	    goto done;    
	fieldalign = hdf5typealignment(fieldtype);
	/* compute the total number of elements in the field array */
	for(i=0;i<ndims;i++) arraycount *= field->dim_size[i];
	position->offset = read_align(position->offset,fieldalign);
	for(i=0;i<arraycount;i++) {
	    if((stat = d4reclaim_datar(fieldtype, position))) goto done;
	}		
    }
    position->offset = saveoffset;
    position->offset += tsym->size;
done:
    return stat;
}

/**************************************************/
int
d4clone_data(NCD4node* tsym, void* src, void** dstp, size_t count)
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
	if((stat=d4clone_datar(tsym,&srcpos,&dstpos))) /* clone one instance */
	    break;
    }
done:
    return stat;
}

/* Recursive type walker: reclaim a single instance */
static int
d4clone_datar(NCD4node* tsym, Position* src, Position* dst)
{
    int stat = NC_NOERR;
    
    switch  (tsym->nc_type_class) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE:
    case NC_SHORT: case NC_USHORT:
    case NC_INT: case NC_UINT: case NC_FLOAT:
    case NC_INT64: case NC_UINT64: case NC_DOUBLE:
        position->offset += tsym->size;
	break;
    case NC_STRING: {
	char** sp = (char**)(position->memory+position->offset);
        /* Need to reclaim string */
	if(*sp != NULL) efree(*sp);
	position->offset += tsym->size;
	} break;
    default:
    	/* reclaim a user type */
	stat = d4clone_usertype(tsym,position);
	break;
    }
    return stat;
}
	
static int
d4clone_usertype(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;

    /* Get info about the xtype */
    switch (tsym->nc_type_class) {
    case NC_OPAQUE: stat = d4clone_opaque(tsym,position); break;
    case NC_ENUM: stat = d4clone_enum(tsym,position); break;
    case NC_VLEN: stat = d4clone_vlen(tsym,position); break;
    case NC_COMPOUND: stat = d4clone_compound(tsym,position); break;
    default:
        stat = NC_EINVAL;
	break;
    }
    return stat;
}

static int
d4clone_vlen(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    size_t i;
    NCD4node* basetype;
    nc_vlen_t* vl = (nc_vlen_t*)(position->memory+position->offset);

    if((stat=nc4_find_type(tsym->container->nc4_info,tsym->u.v.base_nc_typeid,&basetype)))
	goto done;    

    /* Free up each entry in the vlen list */
    if(vl->p != NULL) {
	Reclaim vposition;
	vposition.memory = vl->p;
	vposition.offset = 0;
        for(i=0;i<vl->len;i++) {
	    size_t alignment = hdf5typealignment(basetype);
	    vposition.offset = read_align(vposition.offset,alignment);
	    if((stat = d4clone_datar(basetype,&vposition))) goto done;
	    vposition.offset += basetype->size;
	}
	position->offset += tsym->size;
	efree(vl->p);
    }
done:
    return stat;
}

static int
d4clone_enum(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    NCD4node* basetype;
    if((stat=nc4_find_type(tsym->container->nc4_info,tsym->u.v.base_nc_typeid,&basetype)))
	return stat;
    return d4clone_datar(basetype,position);
}

static int
d4clone_opaque(NCD4node* tsym, Position* position)
{
    /* basically a fixed size sequence of bytes */
    position->offset += tsym->size;
    return NC_NOERR;
}

static int
d4clone_compound(NCD4node* tsym, Position* position)
{
    int stat = NC_NOERR;
    int nfields;
    size_t fid, i, arraycount;
    ptrdiff_t saveoffset;
    size_t cmpdalign = hdf5typealignment(tsym);

    position->offset = read_align(position->offset,cmpdalign);
    saveoffset = position->offset;

    /* Get info about each field in turn and reclaim it */
    nfields = nclistlength(tsym->u.c.field);
    for(fid=0;fid<nfields;fid++) {
	NC_FIELD_INFO_T* field = nclistget(tsym->u.c.field,fid);
	NCD4node* fieldtype;
        int ndims = field->ndims;
	size_t fieldalign;
	if((stat=nc4_find_type(tsym->container->nc4_info,field->nc_typeid,&fieldtype)))
	    goto done;    
	fieldalign = hdf5typealignment(fieldtype);
	/* compute the total number of elements in the field array */
	for(i=0;i<ndims;i++) arraycount *= field->dim_size[i];
	position->offset = read_align(position->offset,fieldalign);
	for(i=0;i<arraycount;i++) {
	    if((stat = d4clone_datar(fieldtype, position))) goto done;
	}		
    }
    position->offset = saveoffset;
    position->offset += tsym->size;
done:
    return stat;
}


/**************************************************/
/* Alignment code */

static ptrdiff_t
read_align(ptrdiff_t offset, size_t alignment)
{
    size_t delta = (offset % alignment);
    if(delta == 0) return offset;
    return offset + (alignment - delta);
}

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
hdf5typealignment(NCD4node* typ)
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
   	   NCD4node* fieldtype = NULL;
	   if(nc4_find_type(typ->container->nc4_info,field0->nc_typeid,&fieldtype))
	   	return 0;
	   return hdf5typealignment(fieldtype); /* may recurse repeatedly */
	} break;
        default: break;
	}
    }
    return 0; /* fail */
}
