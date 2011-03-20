/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <curl/curl.h>

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc.h"
#include "nc4internal.h"

#include "nccr.h"
#include "crdebug.h"
#include "ast.h"
#include "curlwrap.h"

#include "ncstreamx.h"

enum Dimcase {DC_UNKNOWN, DC_FIXED, DC_UNLIMITED, DC_VLEN, DC_PRIVATE};

static int uid = 0;

/*
Fetch the metadata and define in the temporary netcdf-4 file
*/
NCerror
crbuildnc(NCCR* nccr, Header* hdr)
{
    NCerror ncstat = NC_NOERR;
    nc_id ncid = nccr->info.ext_ncid; /*root id*/

    ncstat = crpredefinedtypes(nccr,ncid);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = crfillgroup(nccr, hdr->root, ncid);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

#ifdef IGNORE
    ncstat = crbuilddims(cdmr,hdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = crbuildtypes(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = crbuildvars(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = crbuildglobalattrs(cdmr,getncid(cdmr),dds);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
#endif

done:
    return THROW(ncstat);
}

/* Define needed predefine types in root */
static NCerror
crpredefinedtypes(NCCR* nccr, nc_id ncid)
{
    NCerror ncstat = NC_NOERR;
    nc_type tid;

    /* NC_UINT8(*) bytes_t */
    ncstat = nc_def_vlen(ncid,"bytes_t",NC_UINT8,&tid);
    if(ncstat != NC_NOERR) goto done;

done:
    return ncstat;
}


/* Actual group is created by caller */
static NCerror
crfillgroup(NCCR* nccr, Group* grp, nc_id grpid)
{
    NCerror ncstat = NC_NOERR;
    size_t i,j,k;
    
    /* Create the dimensions */
    for(i=0;i<grp->dims.count;i++) {
	Dimension* dim = grp->dims.values[i];
	if(dim->name.defined) {
	    int dimid;
	    size_t length = (dim->length.defined?dim->length.value:1);
	    if(dim->isUnlimited.defined && dim->isUnlimited.value != 0)
		length = NC_UNLIMITED;
	    ncstat = nc_def_dim(grpd,dim->name.value, length, &dimid);
	    if(ncstat != NC_NOERR) goto done;
	}
    }

    /* Create the enum types */
    for(i=0;i<grp->enumTypes.count;i++) {
	EnumTypeDef* en = grp->enumTypes.values[i];
	int enid;
	if(en->map.count == 0) continue;
	ncstat = nc_def_enum(grpid,typeid,en->name,&enid);
	if(ncstat != NC_NOERR) goto done;
        for(j=0;j<en->map.count;i++) {
	    EnumType* econst = en->map.values[i];	
	    ncstat = nc_insert_enum(grpid,NC_UINT,econst->value,&econst->code);
   	    if(ncstat != NC_NOERR) goto done;	
	}
    }

    /* Create the structs (compound) types */
    /* Note: structs here are also variables */
    for(i=0;i<grp->structs.count;i++) {
	int strucid;
	Structure* struc = grp->structs.values[i];
	ncstat = nc_def_compound(grpid,size,struc->name,&strucid)1
	if(ncstat != NC_NOERR) goto done;	
	for(j=0;j<struc->var.count;j++) {
	    Variable* v = struc->var.values[i];
	    if(v->shape.count == 0) {
	        ncstat = nc_insert_compound(grpid,strucid,v->name,
					    offset, nctypefor(v->dataType));
	    } else {
		int dimsize[NC_MAX_VAR_DIMS];	        		
		for(k=0;k<v->shape.count;k++) {
		    Dimension* dim = v->shape.values[k];
		    dimsize[k] = (dim->length.defined?) dim->length.value:1);
		}
	        ncstat = nc_insert_array_compound(grpid,strucid,v->name,
					    offset, nctypefor(v->dataType),
					    v->shape.count, dimsizes);
	    }
	    if(ncstat != NC_NOERR) goto done;	
	}

    /* Create the group global attributes */
    for(i=0;i<grp->atts.count;i++) {
	Attribute* att = grp->atts.values[i];

	if(att->data.defined) {
	    ncstat = nc_put_att(grpid,NC_GLOBAL,att->name,
				cvtstreamtonc(v->dataType),
				att->len,
				att->data.value);
	} else {
	    switch (att->type) {
	    case STRING: {
	        ncstat = nc_put_att(grpid,NC_GLOBAL,att->name,
				cvtstreamtonc(v->dataType),
				att->sdata.count,
				att->sdata.values);
	    case OPAQUE:
	    default: abort();
	    }
	}
        if(ncstat != NC_NOERR) goto done;
    }


    /* Create the group non-struct variables */
    struct {size_t count; Variable** values;} vars;
    for(i=0;i<grp->vars.count;i++) {
        Variable* v = grp->vars.values[i];
        ncstat = nc_def_var(grpid,v->name,cvtstreamtonc(v->dataType),
			    ndims,
			    dimids,
			    varidp);

        /* Create the group struct variables */
    struct {size_t count; Structure** values;} structs;







    }

}


static int
crbuildstructure(int grpid, Structure* struc)
{
    int ncstatus = NC_NOERR;
    int strucid;
    int i,j;

    ncstat = nc_def_compound(grpid,size,struc->name,&strucid)1
    if(ncstat != NC_NOERR) goto done;       
    for(i=0;i<struc->var.count;i++) {
        Variable* v = struc->var.values[i];
        if(v->shape.count == 0) {
            ncstat = nc_insert_compound(grpid,strucid,v->name,
                                        offset, nctypefor(v->dataType));
        } else {
            int dimsize[NC_MAX_VAR_DIMS];                           
            for(k=0;k<v->shape.count;k++) {
                Dimension* dim = v->shape.values[k];
                dimsize[k] = (dim->length.defined?) dim->length.value:1);
            }
            ncstat = nc_insert_array_compound(grpid,strucid,v->name,
                                        offset, nctypefor(v->dataType),
                                        v->shape.count, dimsizes);
        }
        if(ncstat != NC_NOERR) goto done;   
    }

done:
    return ncstatus;
}

/***************************************************/

/* Map ncstream primitive datatypes to netcdf primitive datatypes */
static nc_type
cvtstreamtonc(Datatype datatype)
{
    switch (datatype) {
    case CHAR: return NC_CHAR;
    case BYTE: return NC_BYTE;
    case SHORT: return NC_SHORT;
    case INT: return NC_INT;
    case LONG: return NC_INT;
    case FLOAT: return NC_FLOAT;
    case DOUBLE: return NC_DOUBLE;
    case STRING: return NC_STRING;
    case UCHAR: return NC_UCHAR;
    case UBYTE: return NC_UBYTE;
    case USHORT: return NC_USHORT;
    case UINT: return NC_UINT;
    case INT64: return NC_INT64;
    case UINT64: return NC_UINT64;
    }
    return NC_NAT;
}


/* Classify a dimension */
static enum Dimcase
classifydim(Dimension* dim)
{
    int len=0, unlim=0, vlen=0, priv=0;
    if(dim->length.isdefined) len=(dim->length.value?1:0);
    if(dim->isUnlimited.isdefined) unlim=(dim->isUnlimited.value?1:0);
    if(dim->isVlen.isdefined) vlen=(dim->isVlen.value?1:0);
    if(dim->isPrivate.isdefined) priv=(dim->isPrivate.value?1:0);

    if(len+unlim+vlen+priv > 1) goto fail;
    if(len) return DC_FIXED;
    if(unlim) return DC_UNLIMITED
    if(vlen) return DC_VLEN;
    if(priv) return DC_PRIVATE;

fail:
    return DC_UNKNOWN;
}


static int
dimsize(Dimension* dim)
{
    if(dim->isUnlimited.defined && dim->isUnlimited.value)
	return NC_UNLIMITED;
    if(dim->isVlen.defined && dim->isVlen.value)
	return -1;
    if(dim->length.defined)
	return dim->length.value;
    return -1;
}

static int
dimsizes(int ndims, Dimension** dims, int sizes[NC_MAX_VAR_DIMS])
{
    int i;
    if(ndims > NC_MAX_VAR_DIMS) return NC_EINVAL;
    for(i=0;i<ndims;i++) {
        sizes[i] = dimsize(dims[i]);
    }
    return NC_NOERR;
}


/* Validate that the set of dimensions can be translated */
static int
validate_dimensions(size_t ndims, int nounlim, Dimension** dims)
{
    int i,j;

    if(ndims == 0) return NC_NOERR;

    /* Validate the dimensions to check for
       non-translatable situations
	- UNLIMITED following a star
	- Any occurrence of Private
	- no occurrences of unlimited (if nounlim is set)
    */

    /* Look for untranslatable dimensions */
    for(i=0;i<ndims;i++) {
        Dimension dim = dims[i];
	enum Dimcase dc = classify(dim);
	switch (dc) {
	case DC_FIXED: break;
	case DC_VLEN: break;
	case DC_UNLIMITED: if(nounlim) {goto untranslatable; else break;}
	default: goto untranslatable;
	}
    }

    /* Look for unlimited after vlen */
    for(i=0;i<ndims;i++) {
        Dimension dim0 = dims[i];
	enum Dimcase dc = classify(dim);
	if(dc != DC_VLEN) continue;
        for(j=i+1;j<ndims;j++) {
            Dimension dim1 = dims[j];
	    dc = classify(dim1);	
	    if(dc == DC_UNLIMITED) goto untranslatable;
	}	   
    }    
    return NC_NOERR;

untranslatable:
    return NC_ETRANSLATION;
}

/*
In order to define a structure, we have to define
a potentially large number of types to support it;
vlens for example
*/
static int
definetypes(NCCRtype* struc)
{
    int i,j,imin,imax;
    int status = NC_NOERR;
    size_t ndims;
    Dimension** dims;
    int dimset[NC_MAX_VAR_DIMS];
    enum Dimcase dc;

    ndims = struc->shape.ndims;
    dims = struc->shape.values;

    status = verify_dimensions(ndims, dims);
    if(status != NC_NOERR) goto done;

    /* Working backward, find the last unprocessed vlen (star) dimension */
    imin = ndims;
    imax = ndims;
    for(i=imin-1;i>=0;i--) {
	Dimension* dim = dims[i];
	dc = classifydim(dim);
	if(dc == DC_VLEN) break;
    }
    if(dc == VLEN) {
	/* We can use the whole dimension set as is */
	if(ndims > NC_MAX_VAR_DIMS) /*provided there are not too many */
	    {status = NC_ETRANSLATION; goto done;}
    } else {
	

    }
    



done:
    return status;
}

/**
Given a variable of the form
T v(d1,d2,d3,*,d4,*,d5)
We need to create the following compound and vlen types.
compound v_1 {T v(d5);
v_1(*) v_2;
compound v_2 {v_1 v(d4);}
v_2(*) v_3;

and then finally
v3 v(d1,d2,d3)

Handle special cases:
1. when last dim is a vlen

*/

static int
buildfield(int ncid,
	   void* cmpd,
	   char* name,
	   nc_type basetype,
	   int ndims,
	   Dimension** dims)
{
    int status = NC_NOERR;
    /* Do some special cases */
    if(ndims == 0) {
	status = ncaux_add_field(cmpd,name,basetype,0,NULL);
    } else {
        /* Need to create the needed subsidiary vlen types */ 
	int vlenid,pos;
	pos = locateleftvldn(ndims,dims,0);
	status = buildvlenchain(ncid,name,basetype,ndims,dims,pos+1,&vlenid);
        if(status != NC_NOERR) goto done;
	if(pos == 0) {
            status = ncaux_add_field(cmpd,name,vlenid,0,NULL);
	} else { /* pos > 0 */
	    int dimsizes[NC_MAX_VAR_DIMS];
	    for(i=0;i<pos;i++)
		dimsizes[i] = dimsize(dims[i]);
            status = ncaux_add_field(cmpd,name,vlenid,pos,dimsizes);
	}
        if(status != NC_NOERR) goto done;
    }

done:
    return status;
}


static int
buildvlenchain(int ncid,
		char* name,
		nc_type basetype,
		int ndims,
		Dimension** dims,
		int index,
		nc_type* vidp)
{
    int i, pos;
    int status = NC_NOERR;
    nc_type* vlenid = basetypeid;
    char suid[4];
    char typename[NC_MAX_NAME+3+1];
    void* tag;
    int nidims;
    int idims[NC_MAX_VAR_DIMS];
    nc_type newid;
   
    /* Locate the leftmost * dimension starting at index*/
    pos = locateleftvlen(ndims,dims,index);
    if(pos < 0) { 
	/* We have hit the terminal set of dimensions; create a compound type */
	strcpy(typename,name);
	snprintf(suid,sizeof(suid),"%3d",index);
	strcat(typename,suid);
	status = ncaux_begin_compound(ncid,typename,NCAUX_ALIGN_C,&tag);
	if(status != NC_NOERR) goto done;
	/* Create a single field */
	nidims = (ndims - index)
	for(i=index;i<ndims;i++)
	    idims[i] = dimsize(dims[i]);
	status = ncaux_add_field(tag,name,basetype,nidims,idims);
	if(status != NC_NOERR) goto done;
	status = ncaux_end_compound(tag,vidp);
	if(status != NC_NOERR) goto done;		
    } else if(pos == (ndims - 1) {
	/* Create a terminal vlen
	strcpy(typename,name);
	snprintf(suid,sizeof(suid),"%3d",index);
	strcat(typename,suid);
	status = nc_def_vlen(ncid,typename,basetype,vidp);
	if(status != NC_NOERR) goto done;
    } else { /* recurse */
        status = buildvlenchain(ncid,name,basetype,ndims,dims,pos+1,&newid);
	if(status != NC_NOERR) goto done;
	/* Create the compound if needed */
	if(pos > (index + 1)) {
	    /* intermediate dimensions, so create compound */
	    strcpy(typename,name);
	    snprintf(suid,sizeof(suid),"%3d",index+1);
	    strcat(typename,suid);
	    status = ncaux_begin_compound(ncid,typename,NCAUX_ALIGN_C,&tag);
	    if(status != NC_NOERR) goto done;
	    /* Create a single field */
	    nidims = (ndims - index)
	    for(i=index;i<ndims;i++)
	        idims[i] = dimsize(dims[i]);
	    status = ncaux_add_field(tag,name,basetype,nidims,idims);
	    if(status != NC_NOERR) goto done;
	    status = ncaux_end_compound(tag,&newid);
	    if(status != NC_NOERR) goto done;		
	} else
	    newid = basetype;
	/* create the vlen */
	strcpy(typename,name);
	snprintf(suid,sizeof(suid),"%3d",index);
	strcat(typename,suid);
	status = nc_def_vlen(ncid,typename,newid,vidp);
	if(status != NC_NOERR) goto done;
    }

done:
    return status;
}

static int
locateleftvlen(int ndims, Dimension** dims, int index)
{
    int i;
    /* Locate the leftmost * dimension starting at index*/
    for(i=0;i<ndims;i++) {
	Dimension* dim = dims[i];
        enum Dimcase dc = classifydim(dim);
        if(dc == DC_VLEN) return i;
    }
    return -1; /* no vlen located */
}

/* Given a structure,
   recursively convert into a set
   of netcdf-4 types as represented
   by NCCRType
*/

typedef struct NCCRType {
    Structure* struc;
    nc_type subclass; /* VLEN, etc */
    char* name;
    int ncid; /* of the parent group */
    int typeid; /* from nc_def_XXX */
    nclist* fieldtypes; /* the struc fields only */
    NCCRDimset dimset;
} NCCRType;

/* utility free function */
static void
cleartypes(nclist* crtypes)
{
    int i,j;
    for(i=0;i<nclistlength(crtypes);i++) {
	NCCRType* t = (NCCRType*)nclistpop(crtypes);
	if(t == NULL) continue;
	if(t->name) free(t->name);
	if(fieldtypes != NULL) cleartypes(fieldtypes);
	free(t);
    }
}

static int
buildstructure(int ncid, Structure* struc)
{
    int dmin, dmax;
    int status = NC_NOERR;
    nclist* definedtypes = nclistnew();

    if(definedtypes == NULL || dimsubsets == NULL) return NC_ENOMEM;

    status = validate_dimensions(struc->shape.count,struc->shape.values);
    if(status != NC_NOERR) goto done;

    status = buildbasetype(ncid,struc,definedtypes);
    if(status != NC_NOERR) goto done;

done:
    return status;
}


static int
buildbasetype(int ncid, Structure struc, nclist* definedtypes, int* basetypeidp)
{
    int i;
    int status = NC_NOERR;
    NCCRType* crtype = NULL;
    nclist* dimsubsets = nclistnew();
    void* tag;
    nc_type cmpdid;

    crtype = (NCCRType*)calloc(1,sizeof(NCCRType));
    if(crtype == NULL) return NC_ENOMEM;

    crtype->struc = struc;
    crtype->ncid = ncid;
    crtype->subclass = NC_COMPOUND;
    crtype->name = strdup(struc->name);
    crtype->nfields = struc->structs.count;
    if(struc->structs.count > 0) {
	crtype->fieldtypes = (NCCRTYPE**)malloc(crtype->nfields*sizeof(NCCRType*));
    }

    /* Build the field types for the struct fields */
    for(i=0;i<struc->structs.count;i++) {
	NCCRType* t;
	status = buildbasetype(ncid,struc->structs.values[i],definedtypes,&t);
	if(status != NC_NOERR) goto done;
	crtyp->fields[crtype->nfields] = t;
	crtype->nfields++;
    }

    /* Define this compound type */
    status = ncaux_begin_compound(ncid,crtype->name,NCAUX_ALIGN_C,&tag);
    if(status != NC_NOERR) goto done;

    /* define the non-struct fields */
    for(i=0;i<struc->vars.count;i++) {
	Variable* v = struc->vars.values[i];
	int ndims = v->shape.count;
	Dimension** dims = v->shape.values;
	/* Validate the dimensions: unlimited disallowed */
	status = validate_dimensions(ndims,dims);
	if(status != NC_NOERR) goto done;
	/* Generate a field */
	status =  buildfield(ncid,tag,v->name,cvtstreamtonc(v->dataType),ndims,dims);
        if(status != NC_NOERR) goto done;
    }

    /* define the struct fields */
    for(i=0;i<struc->structs.count;i++) {
	Structure* s = struc->structs.values[i];
	int ndims = s->shape.count;
	Dimension** dims = s->shape.values;
        int vlenid;
	status = validate_dimensions(ndims,dims);
	if(status != NC_NOERR) goto done;
	status =  buildfield(ncid,tag,s->name,crtype->fields[i].typeid,ndims,dims);
        if(status != NC_NOERR) goto done;
    }

    /* Finish defining the compound */
    status = ncaux_end_compound(tag,&cmpdid);

done:
    if(status != NC_NOERR) {
	if(crtype) free(crtype);
	nclistfree(dimsubsets);
    }
    return status;
}

