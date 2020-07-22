/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See COPYRIGHT for license information.
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

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "config.h"
#include "netcdf.h"
#include "netcdf_aux.h"
#include "ncoffsets.h"
#include "nclog.h"
#include "netcdf_filter.h"
#include "ncfilter.h"

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


/* It is helpful to have a structure that contains memory and an offset */
typedef struct Position{char* memory; ptrdiff_t offset;} Position;

/* Forward */
static int reclaim_datar(int ncid, int xtype, size_t typesize, Position*);

static int ncaux_initialized = 0;

#ifdef USE_NETCDF4
static int reclaim_usertype(int ncid, int xtype, Position* offset);
static int reclaim_compound(int ncid, int xtype, size_t size, size_t nfields, Position* offset);
static int reclaim_vlen(int ncid, int xtype, int basetype, Position* offset);
static int reclaim_enum(int ncid, int xtype, int basetype, size_t, Position* offset);
static int reclaim_opaque(int ncid, int xtype, size_t size, Position* offset);
static int computefieldinfo(struct NCAUX_CMPD* cmpd);
#endif /* USE_NETCDF4 */

static void ncaux_freestringvec(int n, char** vec);
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
    size_t typesize = 0;
    size_t i;
    Position offset;

    if(ncid < 0 || xtype < 0
       || (memory == NULL && count > 0)
       || xtype == NC_NAT)
        {stat = NC_EINVAL; goto done;}
    if(memory == NULL || count == 0)
        goto done; /* ok, do nothing */
    if((stat=nc_inq_type(ncid,xtype,NULL,&typesize))) goto done;
    offset.memory = (char*)memory; /* use char* so we can do pointer arithmetic */
    offset.offset = 0;
    for(i=0;i<count;i++) {
	if((stat=reclaim_datar(ncid,xtype,typesize,&offset))) /* reclaim one instance */
	    break;
    }

done:
    return stat;
}

/* Recursive type walker: reclaim a single instance */
static int
reclaim_datar(int ncid, int xtype, size_t typesize, Position* offset)
{
    int stat = NC_NOERR;

    switch  (xtype) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE:
    case NC_SHORT: case NC_USHORT:
    case NC_INT: case NC_UINT: case NC_FLOAT:
    case NC_INT64: case NC_UINT64: case NC_DOUBLE:
        offset->offset += typesize;
	break;

#ifdef USE_NETCDF4
    case NC_STRING: {
        char** sp = (char**)(offset->memory + offset->offset);
        /* Need to reclaim string */
	if(*sp != NULL) free(*sp);
	offset->offset += typesize;
	} break;
    default:
    	/* reclaim a user type */
	stat = reclaim_usertype(ncid,xtype,offset);
#else
    default:
	stat = NC_ENOTNC4;
#endif
	break;
    }
    return stat;
}

#ifdef USE_NETCDF4

static ptrdiff_t
read_align(ptrdiff_t offset, size_t alignment)
{
  size_t loc_align = (alignment == 0 ? 1 : alignment);
  size_t delta = (offset % loc_align);
  if(delta == 0) return offset;
  return offset + (alignment - delta);
}

static int
reclaim_usertype(int ncid, int xtype, Position* offset)
{
    int stat = NC_NOERR;
    size_t size;
    nc_type basetype;
    size_t nfields;
    int klass;

    /* Get info about the xtype */
    stat = nc_inq_user_type(ncid, xtype, NULL, &size, &basetype, &nfields, &klass);
    switch (klass) {
    case NC_OPAQUE: stat = reclaim_opaque(ncid,xtype,size,offset); break;
    case NC_ENUM: stat = reclaim_enum(ncid,xtype,basetype,size,offset); break;
    case NC_COMPOUND: stat = reclaim_compound(ncid,xtype,size,nfields,offset); break;
    case NC_VLEN: stat = reclaim_vlen(ncid,xtype,basetype,offset); break;
    default:
        stat = NC_EINVAL;
	break;
    }
    return stat;
}

static int
reclaim_vlen(int ncid, int xtype, int basetype, Position* offset)
{
    int stat = NC_NOERR;
    size_t i, basesize;
    nc_vlen_t* vl = (nc_vlen_t*)(offset->memory+offset->offset);

    /* Get size of the basetype */
    if((stat=nc_inq_type(ncid,basetype,NULL,&basesize))) goto done;
    /* Free up each entry in the vlen list */
    if(vl->p != NULL) {
	Position voffset;
	unsigned int alignment = ncaux_type_alignment(basetype,ncid);
	voffset.memory = vl->p;
	voffset.offset = 0;
        for(i=0;i<vl->len;i++) {
	    voffset.offset = read_align(voffset.offset,alignment);
	    if((stat = reclaim_datar(ncid,basetype,basesize,&voffset))) goto done;
	}
	offset->offset += sizeof(nc_vlen_t);
	free(vl->p);
    }

done:
    return stat;
}

static int
reclaim_enum(int ncid, int xtype, int basetype, size_t basesize, Position* offset)
{
    /* basically same as an instance of the enum's integer basetype */
    return reclaim_datar(ncid,basetype,basesize,offset);
}

static int
reclaim_opaque(int ncid, int xtype, size_t opsize, Position* offset)
{
    /* basically a fixed size sequence of bytes */
    offset->offset += opsize;
    return NC_NOERR;
}

static int
reclaim_compound(int ncid, int xtype, size_t cmpdsize, size_t nfields, Position* offset)
{
    int stat = NC_NOERR;
    size_t fid, fieldoffset, i, fieldsize, arraycount;
    int dimsizes[NC_MAX_VAR_DIMS];
    int ndims;
    nc_type fieldtype;
    ptrdiff_t saveoffset;

    saveoffset = offset->offset;

    /* Get info about each field in turn and reclaim it */
    for(fid=0;fid<nfields;fid++) {
	unsigned int fieldalignment;
	/* Get all relevant info about the field */
        if((stat = nc_inq_compound_field(ncid,xtype,fid,NULL,&fieldoffset, &fieldtype, &ndims, dimsizes))) goto done;
	fieldalignment = ncaux_type_alignment(fieldtype,ncid);
        if((stat = nc_inq_type(ncid,fieldtype,NULL,&fieldsize))) goto done;
	if(ndims == 0) {ndims=1; dimsizes[0]=1;} /* fake the scalar case */
	/* Align to this field */
	offset->offset = read_align(offset->offset,fieldalignment);
	/* compute the total number of elements in the field array */
	arraycount = 1;
	for(i=0;i<ndims;i++) arraycount *= dimsizes[i];
	for(i=0;i<arraycount;i++) {
	    if((stat = reclaim_datar(ncid, fieldtype, fieldsize, offset))) goto done;
	}
    }
    /* Return to beginning of the compound and move |compound| */
    offset->offset = saveoffset;
    offset->offset += cmpdsize;

done:
    return stat;
}

#endif /*USE_NETCDF4*/


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
size_t
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
size_t
ncaux_type_alignment(int xtype, int ncid)
{
    if(!ncaux_initialized) {
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


/**************************************************/
/* Forward */

#define NUMCHAR "0123456789"
#define LPAREN '('
#define RPAREN ')'
#define LBRACK '['
#define RBRACK ']'

static int gettype(const int q0, const int q1, int* unsignedp);

const struct LegalFormat {
    const char* tag;
    int format;
} legalformats[] = {
    {"hdf5", NC_FILTER_FORMAT_HDF5},
    {NULL, 0},
};


void
ncaux_filterfix8(unsigned char* mem, int decode)
{
#ifdef WORDS_BIGENDIAN
    if(decode) { /* Apply inverse of the encode case */
	byteswap4(mem); /* step 1: byte-swap each piece */
	byteswap4(mem+4);
	byteswap8(mem); /* step 2: convert to little endian format */
    } else { /* encode */
	byteswap8(mem); /* step 1: convert to little endian format */
	byteswap4(mem); /* step 2: byte-swap each piece */
	byteswap4(mem+4);
    }
#else /* Little endian */
    /* No action is necessary */
#endif	    
}

/* Look at q0 and q1) to determine type */
static int
gettype(const int q0, const int q1, int* isunsignedp)
{
    int type = 0;
    int isunsigned = 0;
    char typechar;
    
    isunsigned = (q0 == 'u' || q0 == 'U');
    if(q1 == '\0')
	typechar = q0; /* we were given only a single char */
    else if(isunsigned)
	typechar = q1; /* we have something like Ux as the tag */
    else
	typechar = q1; /* look at last char for tag */
    switch (typechar) {
    case 'f': case 'F': case '.': type = 'f'; break; /* float */
    case 'd': case 'D': type = 'd'; break; /* double */
    case 'b': case 'B': type = 'b'; break; /* byte */
    case 's': case 'S': type = 's'; break; /* short */
    case 'l': case 'L': type = 'l'; break; /* long long */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': type = 'i'; break;
    case 'u': case 'U': type = 'i'; isunsigned = 1; break; /* unsigned int */
    case '\0': type = 'i'; break;
    default: break;
    }
    if(isunsignedp) *isunsignedp = isunsigned;
    return type;
}

#ifdef WORDS_BIGENDIAN
/* Byte swap an 8-byte integer in place */
static void
byteswap8(unsigned char* mem)
{
    unsigned char c;
    c = mem[0];
    mem[0] = mem[7];
    mem[7] = c;
    c = mem[1];
    mem[1] = mem[6];
    mem[6] = c;
    c = mem[2];
    mem[2] = mem[5];
    mem[5] = c;
    c = mem[3];
    mem[3] = mem[4];
    mem[4] = c;
}

/* Byte swap an 8-byte integer in place */
static void
byteswap4(unsigned char* mem)
{
    unsigned char c;
    c = mem[0];
    mem[0] = mem[3];
    mem[3] = c;
    c = mem[1];
    mem[1] = mem[2];
    mem[2] = c;
}
#endif

/*
Parse a filter spec string into a NC_FILTER_SPEC*
@param txt - a string containing the spec as a sequence of
              constants separated by commas.
@param specp - store the parsed filter here -- caller frees
@return NC_NOERR if parse succeeded
@return NC_EINVAL otherwise
*/

EXTERNL int
ncaux_filterspec_parse(const char* txt, NC_Filterspec** specp)
{
    int i,stat = NC_NOERR;
    char* p;
    char* sdata0 = NULL; /* what to free */
    char* sdata = NULL; /* sdata0 with leading prefix skipped */
    size_t nparams; /* no. of comma delimited params */
    char* filterid = NULL;
    char** params = NULL;
    size_t len;
    NC_Filterspec* pfs = NULL;
    
    if(txt == NULL)
        {stat = NC_EINVAL; goto done;}
    len = strlen(txt);
    if(len == 0)
        {stat = NC_EINVAL; goto done;}

    if((sdata0 = (char*)calloc(1,len+1+1))==NULL)
	{stat = NC_ENOMEM; goto done;}	
    memcpy(sdata0,txt,len);
    sdata = sdata0;

    /* Count number of parameters + id and delimit */
    p=sdata;
    for(nparams=0;;nparams++) {
        char* q = strchr(p,',');
	if(q == NULL) break;
	*q++ = '\0';
	p = q;
    }
    nparams++; /* for final piece */

    if(nparams == 0)
	{stat = NC_EINVAL; goto done;} /* no id and no parameters */

    p = sdata;

    /* Extract the filter id */
    if((filterid=strdup(p))==NULL)
        {stat = NC_ENOMEM; goto done;}
    nparams--;

    /* skip past the filter id */
    p = p + strlen(p) + 1;

    /* Allocate the max needed space */
    if((params = (char**)calloc(sizeof(char*),(nparams)))==NULL)
	{stat = NC_ENOMEM; goto done;}

    /* walk and capture */
    for(i=0;i<nparams;i++) { /* step thru param strings */
	len = strlen(p);
	/* skip leading white space */
	while(strchr(" 	",*p) != NULL) {p++; len--;}
	if((params[i] = strdup(p))==NULL) goto done;
        p = p + strlen(p) + 1; /* move to next param */
    }
    /* Now return results */
    if(*specp != NULL) abort();
    {
        pfs = calloc(1,sizeof(NC_Filterspec));
        if(pfs == NULL) {stat = NC_ENOMEM; goto done;}
	pfs->version = NCAUX_FILTERSPEC_VERSION;
        pfs->filterid = filterid; filterid = NULL;
        pfs->nparams = nparams;;
        pfs->params = params; params = NULL;
	*specp = pfs; pfs = NULL;
    }

done:
    ncaux_filterspec_free(pfs);
    nullfree(sdata);
    nullfree(filterid);
    ncaux_freestringvec(nparams,params);
    return stat;
}

/*
Parse a string containing multiple '|' separated filter specs.

@param spec0 - a string containing the list of filter specs.
@param nspecsp - # of parsed specs
@param specsp - pointer to hold vector of parsed specs. Caller frees
@return NC_NOERR if parse succeeded
@return NC_EINVAL if bad parameters or parse failed
*/

EXTERNL int
ncaux_filterspec_parselist(const char* txt0, char** formatp, size_t* nspecsp, NC_Filterspec*** vectorp)
{
    int stat = NC_NOERR;
    char* format = NULL;
    size_t len = 0;
    size_t nspecs = 0;
    NC_Filterspec** vector = NULL;
    char* spec0 = NULL; /* with prefix */
    char* spec = NULL; /* without prefix */
    char* p = NULL;
    char* q = NULL;

    if(txt0  == NULL) return NC_EINVAL;
    /* Duplicate txt0 so we can modify it */
    len = strlen(txt0);
    if((spec = calloc(1,len+1+1)) == NULL) {stat = NC_ENOMEM; goto done;}
    memcpy(spec,txt0,len); /* Note double ending nul */
    spec0 = spec; /* Save for later free */

    /* See if there is a prefix '[format]' tag */
    if(spec[0] == LBRACK) {
	p = spec + 1;
	q = strchr(p,RBRACK);
	if(q == NULL) {stat = NC_EINVAL; goto done;}
	*q++ = '\0'; /* delimit tag */
	if((format = strdup(p))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	spec = q; /* skip tag wrt later processing */
    }

    /* pass 1: count number of specs */
    p = spec;
    nspecs = 0;
    while(*p) {
	q = strchr(p,'|');
	if(q == NULL) q = p + strlen(p); /* fake it */
	nspecs++;
	p = q + 1;
    }
    if(nspecs >  0) {
	int count = 0;
	if((vector = (NC_Filterspec**)calloc(sizeof(NC_Filterspec*),nspecs)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	/* pass 2: parse */
	p = spec;
	for(count=0;count<nspecs;count++) {
	    NC_Filterspec* aspec = NULL;
	    q = strchr(p,'|');
	    if(q == NULL) q = p + strlen(p); /* fake it */
	    *q = '\0';
	    if((stat=ncaux_filterspec_parse(p,&aspec))) goto done;
	    vector[count] = aspec; aspec = NULL;
	    p = q+1; /* ok because of double nul */
	}
    }
    if(formatp) {*formatp = format; format = NULL;}
    if(nspecsp) *nspecsp = nspecs;
    if(vectorp) {*vectorp = vector; vector = NULL;}
done:
    nullfree(spec0);
    nullfree(format);
    if(vector) {
	int i;
        for(i=0;i<nspecs;i++)
            ncaux_filterspec_free(vector[i]);
	nullfree(vector);
    }
    return stat;
}

void
ncaux_filterspec_free(NC_Filterspec* spec)
{
    if(spec) {
	nullfree(spec->filterid);
	if(spec->params != NULL)
	    ncaux_freestringvec(spec->nparams,spec->params);
	free(spec);
    }
}

static void
ncaux_freestringvec(int n, char** vec)
{
    int i;
    if(vec) {
        for(i=0;i<n;i++)
            nullfree(vec[i]);
	nullfree(vec);
    }
}


/*
Convert an NC_Filterspec to equivalent NC_H5_Filterspec.

@param spec - (in) NC_Filterspec instance
@param spech5p - (out) NC_H5_Filterspec poinbter
@return NC_NOERR if parse succeeded
@return NC_EINVAL if bad parameters or parse failed
*/

EXTERNL int
ncaux_filterspec_cvt(const NC_Filterspec* spec, NC_H5_Filterspec** spech5p)
{
    int i,stat = NC_NOERR;
    NC_H5_Filterspec* h5spec = NULL;
    unsigned int* h5params = NULL;
    size_t nparams = 0; /*actual count*/
    
    if((h5spec = calloc(1,sizeof(NC_H5_Filterspec))) == NULL)
        {stat = NC_ENOMEM; goto done;}
    /* Try to name convert the filter id */
    if((stat = NC_cvtX2I_id(spec->filterid,&h5spec->filterid))) goto done;
    if(spec->nparams >  0) {
        if((h5params = (unsigned int*)calloc(sizeof(unsigned int),(spec->nparams)*2)) == NULL) /* Assume all are 8-byte */
            {stat = NC_ENOMEM; goto done;}
	/* Walk and convert */
	for(nparams=0,i=0;i<spec->nparams;i++) {
	    unsigned long long val64u;
	    unsigned int val32u;
	    double vald;
	    float valf;
	    unsigned int *vector;
    	    unsigned char mem[8];
	    int isunsigned = 0;
	    int isnegative = 0;
	    int type = 0;
    	    char* q;
	    char* p = spec->params[i];
   	    size_t len = strlen(p);
	    int sstat;
	    /* skip leading white space */
    	    while(strchr(" 	",*p) != NULL) {p++; len--;}
	    /* Get leading sign character, if any */
	    if(*p == '-') isnegative = 1;
            /* Get trailing type tag characters */
	    switch (len) {
	    case 0: stat = NC_EINVAL; goto done; /* empty parameter */
	    case 1: case 2:
	        q = (p + len) - 1; /* point to last char */
	        type = gettype(*q,'\0',&isunsigned);
	        break;
	    default: /* > 2 => we might have a two letter tag */
	        q = (p + len) - 2;
	        type = gettype(*q,*(q+1),&isunsigned);
	        break;
	    }
	    /* Now parse */
	    switch (type) {
	    case 'b': case 's': case 'i':
 	        /* special case for a positive integer;for back compatibility.*/
	        if(!isnegative)
	            sstat = sscanf(p,"%u",&val32u);
	        else
                    sstat = sscanf(p,"%d",(int*)&val32u);
	        if(sstat != 1) {stat = NC_EINVAL; goto done;}
	        switch(type) {
	        case 'b': val32u = (val32u & 0xFF); break;
	        case 's': val32u = (val32u & 0xFFFF); break;
	        }
	        h5params[nparams++] = val32u;
	        break;
  	    case 'f':
                sstat = sscanf(p,"%lf",&vald);
                if(sstat != 1) {stat = NC_EINVAL; goto done;}
                valf = (float)vald;
                h5params[nparams++] = *(unsigned int*)&valf;
                break;
            /* The following are 8-byte values, so we must swap pieces if this
               is a little endian machine */        
            case 'd':
                sstat = sscanf(p,"%lf",&vald);
                if(sstat != 1) {stat = NC_EINVAL; goto done;};
                memcpy(mem,&vald,sizeof(mem));
                ncaux_filterfix8(mem,0);
                vector = (unsigned int*)mem;
                h5params[nparams++] = vector[0];
                h5params[nparams++] = vector[1];
                break;
            case 'l': /* long long */
                if(isunsigned)
                    sstat = sscanf(p,"%llu",&val64u);
                else
                    sstat = sscanf(p,"%lld",(long long*)&val64u);
                if(sstat != 1) {stat = NC_EINVAL; goto done;};
                memcpy(mem,&val64u,sizeof(mem));
                ncaux_filterfix8(mem,0);
                vector = (unsigned int*)&mem;
                h5params[nparams++] = vector[0];
                h5params[nparams++] = vector[1];
                break;
            default:
                {stat = NC_EINVAL; goto done;};
            }
 	}
	h5spec->params = h5params;
	h5params = NULL;
    }

    if(spech5p) {*spech5p = h5spec; h5spec = NULL;}

done:
    if(h5spec) {
        nullfree(h5spec->params);
	nullfree(h5spec);
    }
    return stat;
}
    
    
#if 0
/*
Parse a filter spec string into a NC_H5_Filterspec*
@param txt - a string containing the spec as a sequence of
              constants separated by commas.
@param specp - store the parsed filter here -- caller frees
@return NC_NOERR if parse succeeded
@return NC_EINVAL otherwise
*/

EXTERNL int
ncaux_filter_parsespec(const char* txt, NC_H5_Filterspec** h5specp)
{
    int stat = NC_NOERR;
    NC_Filterspec* spec = NULL;
    NC_H5_Filterspec* h5spec = NULL;
    size_t len;
    
    if(txt == NULL) 
	{stat = NC_EINVAL; goto done;}
    len = strlen(txt);
    if(len == 0) {stat = NC_EINVAL; goto done;}

    /* Parse as strings */
    if((stat = ncaux_filterspec_parse(txt,&spec))) goto done;
    /* walk and convert */
    if((stat = ncaux_filterspec_cvt(spec,&h5spec))) goto done;
    /* Now return results */
    if(h5specp != NULL) {*h5specp = h5spec; h5spec = NULL;}

done:
    ncaux_filterspec_free(spec);
    if(h5spec) nullfree(h5spec->params);
    nullfree(h5spec);
    return stat;
}

/*
Parse a string containing multiple '|' separated filter specs.

@param spec0 - a string containing the list of filter specs.
@param nspecsp - # of parsed specs
@param specsp - pointer to hold vector of parsed specs. Caller frees
@return NC_NOERR if parse succeeded
@return NC_EINVAL if bad parameters or parse failed
*/

EXTERNL int
ncaux_filter_parselist(const char* txt0, size_t* nspecsp, NC_H5_Filterspec*** vectorp)
{
    int stat = NC_NOERR;
    size_t len = 0;
    size_t nspecs = 0;
    NC_H5_Filterspec** vector = NULL;
    char* spec0 = NULL; /* with prefix */
    char* spec = NULL; /* without prefix */
    char* p = NULL;
    char* q = NULL;

    if(txt0  == NULL) return NC_EINVAL;
    /* Duplicate txt0 so we can modify it */
    len = strlen(txt0);
    if((spec = calloc(1,len+1+1)) == NULL) return NC_ENOMEM;
    memcpy(spec,txt0,len); /* Note double ending nul */
    spec0 = spec; /* Save for later free */

    /* See if there is a prefix '[format]' tag; ignore it */
    if(spec[0] == LBRACK) {
	spec = q; /* skip tag wrt later processing */
    }
    /* pass 1: count number of specs */
    p = spec;
    nspecs = 0;
    while(*p) {
	q = strchr(p,'|');
	if(q == NULL) q = p + strlen(p); /* fake it */
	nspecs++;
	p = q + 1;
    }
    if(nspecs >  0) {
	int count = 0;
	if((vector = (NC_H5_Filterspec**)malloc(sizeof(NC_H5_Filterspec*)*nspecs)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	/* pass 2: parse */
	p = spec;
	for(count=0;count<nspecs;count++) {
	    NC_H5_Filterspec* aspec = NULL;
	    q = strchr(p,'|');
	    if(q == NULL) q = p + strlen(p); /* fake it */
	    *q = '\0';
	    if(ncaux_filter_parsespec(p,&aspec))
	        {stat = NC_EINVAL; goto done;}
	    vector[count] = aspec; aspec = NULL;
	    p = q+1; /* ok because of double nul */
	}
    }
    if(nspecsp) *nspecsp = nspecs;
    if(vectorp) *vectorp = (nspecs == 0 ? NULL : vector);
    vector = NULL;
done:
    nullfree(spec0);
    if(vector != NULL) {
	int k;
	for(k=0;k<nspecs;k++) {
	    NC_H5_Filterspec* nfs = vector[k];
	    if(nfs->params) free(nfs->params);
	    nullfree(nfs);
	}
	free(vector);
    }
    return stat;
}
#endif
