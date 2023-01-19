/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include <stdarg.h>
#include "d4includes.h"

/*
The primary purpose of this code is to recursively traverse
the incoming data to get the endianness correct.
*/

/* Forward */

static int walkAtomicVar(NCD4meta*, NCD4node*, NCD4node*, NCD4offset* offset);
static int walkOpaqueVar(NCD4meta*,NCD4node*, NCD4node*, NCD4offset* offset);
static int walkStructArray(NCD4meta*,NCD4node*, NCD4node*, NCD4offset* offset);
static int walkStruct(NCD4meta*, NCD4node*, NCD4node*, NCD4offset* offset);
static int walkSeqArray(NCD4meta*, NCD4node*, NCD4node*, NCD4offset* offset);
static int walkSeq(NCD4meta*,NCD4node*, NCD4node*, NCD4offset* offset);

/**************************************************/

/*
Assumes that compiler->swap is true; does necessary
byte swapping.
*/
int
NCD4_swapdata(NCD4meta* compiler, NClist* topvars)
{
    int ret = NC_NOERR;
    int i;
    NCD4offset* offset = NULL;
    
    offset = BUILDOFFSET(compiler->serial.dap,compiler->serial.dapsize);
    for(i=0;i<nclistlength(topvars);i++) {
	NCD4node* var = (NCD4node*)nclistget(topvars,i);
	OFFSET2BLOB(var->data.dap4data,offset);
	switch (var->subsort) {
	default:
	    if((ret=walkAtomicVar(compiler,var,var,offset))) goto done;
	    break;
	case NC_OPAQUE:
	    /* The only thing we need to do is swap the counts */
	    if((ret=walkOpaqueVar(compiler,var,var,offset))) goto done;
	    break;
	case NC_STRUCT:
	    if((ret=walkStructArray(compiler,var,var,offset))) goto done;
	    break;
	case NC_SEQ:
	    if((ret=walkSeqArray(compiler,var,var,offset))) goto done;
	    break;
	}
	var->data.dap4data.size = DELTA(offset,var->data.dap4data.memory);
	/* skip checksum, if there is one */
        if(compiler->controller->data.inferredchecksumming)
	    INCR(offset,CHECKSUMSIZE);
    }
done:
    if(offset) free(offset);
    return THROW(ret);
}

static int
walkAtomicVar(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, NCD4offset* offset)
{
    int ret = NC_NOERR;
    d4size_t i;
    nc_type subsort;
    d4size_t dimproduct;
    NCD4node* basetype;

    basetype = (var->sort == NCD4_TYPE ? var : var->basetype);
    subsort = basetype->subsort;
    dimproduct = (var->sort == NCD4_TYPE ? 1 : NCD4_dimproduct(var));

    if(subsort == NC_ENUM)
	subsort = var->basetype->basetype->subsort;
    /* Only need to swap multi-byte integers and floats */
    if(subsort != NC_STRING) {
        int typesize = NCD4_typesize(subsort);
	d4size_t totalsize = typesize*dimproduct;
	if(typesize == 1) {
	    INCR(offset,totalsize);
	} else { /*(typesize > 1)*/
	    for(i=0;i<dimproduct;i++) {
	        char* sp = offset->offset;
	        if(compiler->swap) {
	            switch (typesize) {
	            case 2: swapinline16(sp); break;
	            case 4: swapinline32(sp); break;
	            case 8: swapinline64(sp); break;
	            default: break;
	            }
		}
	        INCR(offset,typesize);
	    }
	}
    } else if(subsort == NC_STRING) { /* remaining case; just convert the counts */
	COUNTERTYPE count;
	for(i=0;i<dimproduct;i++) {
	    /* Get string count */
	    if(compiler->swap)
		swapinline64(offset);
	    count = GETCOUNTER(offset);
	    SKIPCOUNTER(offset);
	    /* skip count bytes */
	    INCR(offset,count);
	}
    }
    return THROW(ret);
}

static int
walkOpaqueVar(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, NCD4offset* offset)
{
    int ret = NC_NOERR;
    d4size_t i;
    unsigned long long count;
    d4size_t dimproduct = NCD4_dimproduct(var);

    dimproduct = (var->sort == NCD4_TYPE ? 1 : NCD4_dimproduct(var));

    for(i=0;i<dimproduct;i++) {
	/* Get and swap opaque count */
	if(compiler->swap)
	    swapinline64(offset);
	count = GETCOUNTER(offset);
	SKIPCOUNTER(offset);
	INCR(offset,count);
    }
    return THROW(ret);
}

static int
walkStructArray(NCD4meta* compiler, NCD4node* topvar, NCD4node* var,  NCD4offset* offset)
{
    int ret = NC_NOERR;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);
    NCD4node* basetype = var->basetype;

    for(i=0;i<dimproduct;i++) {
	/* Swap, possibly recursively, the single struct pointed to by offset*/
	if((ret=walkStruct(compiler,topvar,basetype,offset))) goto done;
    }

done:
    return THROW(ret);
}

static int
walkStruct(NCD4meta* compiler, NCD4node* topvar, NCD4node* structtype, NCD4offset* offset)
{
    int ret = NC_NOERR;
    int i;

    for(i=0;i<nclistlength(structtype->vars);i++) {
	NCD4node* field = (NCD4node*)nclistget(structtype->vars,i);
	NCD4node* fieldbase = field->basetype;
        switch (fieldbase->subsort) {
        default:
	    if((ret=walkAtomicVar(compiler,topvar,field,offset))) goto done;
  	    break;
	case NC_OPAQUE:
	    /* The only thing we need to do is swap the counts */
	    if((ret=walkOpaqueVar(compiler,topvar,field,offset))) goto done;
	    break;
        case NC_STRUCT:
	    if((ret=walkStructArray(compiler,topvar,field,offset))) goto done;
  	    break;
        case NC_SEQ:
	    if((ret=walkSeqArray(compiler,topvar,field,offset))) goto done;
	    break;
        }
    }

done:
    return THROW(ret);
}

static int
walkSeqArray(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, NCD4offset* offset)
{
    int ret = NC_NOERR;
    d4size_t i;
    d4size_t dimproduct;
    NCD4node* seqtype;

    assert(var->sort == NCD4_VAR);
    dimproduct = NCD4_dimproduct(var);
    seqtype = var->basetype;

    for(i=0;i<dimproduct;i++) {
	/* Swap, possibly recursively, the single seq pointed to by offset*/
	if((ret=walkSeq(compiler,topvar,seqtype,offset))) goto done;
    }

done:
    return THROW(ret);
}

/*
Remember that the base type of var is a vlen.
*/
static int
walkSeq(NCD4meta* compiler, NCD4node* topvar, NCD4node* vlentype, NCD4offset* offset)
{
    int ret = NC_NOERR;
    int i;
    d4size_t recordcount;
    NCD4node* basetype;

    /* process the record count */
    if(compiler->swap)
        swapinline64(offset);
    recordcount = GETCOUNTER(offset);
    SKIPCOUNTER(offset);

    basetype = vlentype->basetype; /* This may be of any type potentially */
    assert(basetype->sort == NCD4_TYPE);

    for(i=0;i<recordcount;i++) {
        switch(basetype->subsort) {
	default: /* atomic basetype */
	    if((ret=walkAtomicVar(compiler,topvar,basetype,offset))) goto done;
	    break;
	case NC_OPAQUE:
	    if((ret=walkOpaqueVar(compiler,topvar,basetype,offset))) goto done;
	    break;
	case NC_STRUCT:
	    /* We can treat each record like a structure instance */
	    if((ret=walkStruct(compiler,topvar,basetype,offset))) goto done;
	    break;
	case NC_SEQ:
	    if((ret=walkSeq(compiler,topvar,basetype,offset))) goto done;
	    break;
	}
    }

done:
    return THROW(ret);
}
