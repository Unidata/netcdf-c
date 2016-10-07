/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include <stdarg.h>
#include "d4includes.h"
#include "ezxml.h"

/*
The primary purpose of this code is to recursivly traverse
the incoming data to get the endianness correct.
*/

/* Forward */

static int swapAtomicVar(NCD4meta*,NCD4node*, void** offsetp);
static int swapOpaqueVar(NCD4meta*,NCD4node*, void** offsetp);
static int swapStructArray(NCD4meta*,NCD4node*, void** offsetp);
static int swapStruct(NCD4meta*,NCD4node*, void** offsetp);
static int swapSeqArray(NCD4meta*,NCD4node*, void** offsetp);
static int swapSeq(NCD4meta*,NCD4node*, void** offsetp);

/**************************************************/

int
NCD4_swap(NCD4meta* compiler, NClist* topvars)
{
    int ret = NC_NOERR;
    int i;
    void* offset;

    offset = compiler->serial.dap;
    for(i=0;i<nclistlength(topvars);i++) {
	NCD4node* var = (NCD4node*)nclistget(topvars,i);
	var->data.vardata.memory = offset;

	switch (var->subsort) {
	default:
	    if((ret=swapAtomicVar(compiler,var,&offset))) goto done;
	    break;
	case NC_OPAQUE:
	    /* The only thing we need to do is if((ret=swap the counts */
	    if((ret=swapOpaqueVar(compiler,var,&offset))) goto done;
	    break;
	case NC_STRUCT:
	    if((ret=swapStructArray(compiler,var,&offset))) goto done;
	    break;
	case NC_SEQ:
	    if((ret=swapSeqArray(compiler,var,&offset))) goto done;
	    break;
	}
	var->data.vardata.size = (d4size_t)(offset - var->data.vardata.memory);
	/* skip checksum */
        if(compiler->checksummode == NCD4_DAP || compiler->checksummode == NCD4_ALL)
	    offset += CHECKSUMSIZE;
    }
done:
    return ret;
}

static int
swapAtomicVar(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    int typesize;
    d4size_t i;
    nc_type tid = var->basetype->meta.id;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    typesize = NCD4_typesize(tid);
    /* Only need to swap multi-byte integers and floats */
    if(tid != NC_STRING && typesize > 1) {
	d4size_t totalsize = typesize*dimproduct;
	for(i=0;i<totalsize;i++) {
	    char* sp = (char*)offset;
	    switch (typesize) {
	    case 2: swapinline16(sp); break;
	    case 4: swapinline32(sp); break;
	    case 8: swapinline64(sp); break;
	    default: break;
	    }
	    i += typesize;
	    offset += typesize;
	}
    } else if(tid == NC_STRING) { /* remaining case; just convert the counts */
	unsigned long long count;
	for(i=0;i<dimproduct;i++) {
	    /* Get string count */
	    swapinline64(offset);
	    count = GETCOUNTER(offset);
	    SKIPCOUNTER(offset);
	    /* skip count bytes */
	    offset += count;
	}
    }
    *offsetp = offset;
    return ret;
}

static int
swapOpaqueVar(NCD4meta* compiler,  NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    unsigned long long count;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    for(i=0;i<dimproduct;i++) {
	/* Get and swap opaque count */
	swapinline64(offset);
	count = GETCOUNTER(offset);
	SKIPCOUNTER(offset);
	offset += count;
    }
    *offsetp = offset;
    return ret;
}

static int
swapStructArray(NCD4meta* compiler, NCD4node* var,  void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    for(i=0;i<dimproduct;i++) {
	/* Swap, possibly recursively, the singe struct pointed to by offset*/
	if((ret=swapStruct(compiler,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return ret;
}

static int
swapStruct(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    int i;
    void* offset;

    offset = *offsetp;
    for(i=0;i<nclistlength(var->vars);i++) {
	NCD4node* field = (NCD4node*)nclistget(var->vars,i);
	switch (field->subsort) {
	default:
	    if((ret=swapAtomicVar(compiler,var,&offset))) goto done;
	    break;
	case NC_STRUCT:
	    if((ret=swapStructArray(compiler,var,&offset))) goto done;
	    break;
	case NC_SEQ:
	    if((ret=swapSeqArray(compiler,var,&offset))) goto done;
	    break;
	}
    }
    *offsetp = offset;
done:
    return ret;
}

static int
swapSeqArray(NCD4meta* compiler, NCD4node* var,	 void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    for(i=0;i<dimproduct;i++) {
	/* Swap, possibly recursively, the single seq pointed to by offset*/
	if((ret=swapSeq(compiler,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return ret;
}

static int
swapSeq(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    int i;
    void* offset;
    d4size_t recordcount;

    offset = *offsetp;

    /* Get and swap the record count */
    swapinline64(offset);
    recordcount = GETCOUNTER(offset);
    SKIPCOUNTER(offset);

    for(i=0;i<recordcount;i++) {
	/* We can treat each record like a structure instance */
	if((ret=swapStruct(compiler,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return ret;
}
