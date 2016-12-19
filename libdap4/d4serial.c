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

static int walkAtomicVar(NCD4meta*, NCD4node*, NCD4node*, void** offsetp);
static int walkOpaqueVar(NCD4meta*,NCD4node*, NCD4node*, void** offsetp);
static int walkStructArray(NCD4meta*,NCD4node*, NCD4node*, void** offsetp);
static int walkStruct(NCD4meta*, NCD4node*, NCD4node*, void** offsetp);
static int walkSeqArray(NCD4meta*, NCD4node*, NCD4node*, void** offsetp);
static int walkSeq(NCD4meta*,NCD4node*, NCD4node*, void** offsetp);

#ifdef D4DUMPCSUM
static unsigned int debugcrc32(unsigned int crc, const void *buf, size_t size)
{
    int i;
    fprintf(stderr,"crc32: ");
    for(i=0;i<size;i++) {fprintf(stderr,"%02x",((unsigned char*)buf)[i]);}
    fprintf(stderr,"\n");
    return NCD4_crc32(crc,buf,size);
}
#define CRC32 debugcrc32
#else
#define CRC32 NCD4_crc32
#endif

/**************************************************/

int
NCD4_serial(NCD4meta* compiler, NClist* topvars)
{
    int ret = NC_NOERR;
    int i;
    void* offset;

    offset = compiler->serial.dap;
    for(i=0;i<nclistlength(topvars);i++) {
	NCD4node* var = (NCD4node*)nclistget(topvars,i);
	var->data.topvardata.memory = offset;

	var->data.remotechecksum = 0;
	var->data.localchecksum = 0; /* initialize local checksum computation */

	switch (var->subsort) {
	default:
	    if((ret=walkAtomicVar(compiler,var,var,&offset))) goto done;
	    break;
	case NC_OPAQUE:
	    /* The only thing we need to do is if((ret=swap the counts */
	    if((ret=walkOpaqueVar(compiler,var,var,&offset))) goto done;
	    break;
	case NC_STRUCT:
	    if((ret=walkStructArray(compiler,var,var,&offset))) goto done;
	    break;
	case NC_SEQ:
	    if((ret=walkSeqArray(compiler,var,var,&offset))) goto done;
	    break;
	}
	var->data.topvardata.size = (d4size_t)(offset - var->data.topvardata.memory);
	/* skip checksum, if there is one */
        if(!compiler->serial.nochecksum)
	    offset += CHECKSUMSIZE;
    }
done:
    return THROW(ret);
}

static int
walkAtomicVar(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    nc_type subsort = var->basetype->subsort;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    if(subsort == NC_ENUM)
	subsort = var->basetype->basetype->subsort;
    /* Only need to swap multi-byte integers and floats; but checksum all of them */
    if(subsort != NC_STRING) {
        int typesize = NCD4_typesize(subsort);
	d4size_t totalsize = typesize*dimproduct;
	if(compiler->checksumming) {
	    topvar->data.localchecksum = CRC32(topvar->data.localchecksum, offset, totalsize);
	}
	if(typesize == 1) {
	    offset += totalsize;
	} else { /*(typesize > 1)*/
	    for(i=0;i<dimproduct;i++) {
	        char* sp = (char*)offset;
	        if(compiler->swap) {
	            switch (typesize) {
	            case 2: swapinline16(sp); break;
	            case 4: swapinline32(sp); break;
	            case 8: swapinline64(sp); break;
	            default: break;
	            }
		}
	        offset += typesize;
	    }
	}
    } else if(subsort == NC_STRING) { /* remaining case; just convert the counts */
	COUNTERTYPE count;
	for(i=0;i<dimproduct;i++) {
	    if(compiler->checksumming)
	        topvar->data.localchecksum = CRC32(topvar->data.localchecksum, offset, COUNTERSIZE);
	    /* Get string count */
	    if(compiler->swap)
		swapinline64(offset);
	    count = GETCOUNTER(offset);
	    SKIPCOUNTER(offset);
	    if(compiler->checksumming)
	        topvar->data.localchecksum = CRC32(topvar->data.localchecksum, offset, count);
	    /* skip count bytes */
	    offset += count;
	}
    }
    *offsetp = offset;
    return THROW(ret);
}

static int
walkOpaqueVar(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    unsigned long long count;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    for(i=0;i<dimproduct;i++) {
	if(compiler->checksumming)
	    topvar->data.localchecksum = CRC32(topvar->data.localchecksum, offset, COUNTERSIZE);
	/* Get and swap opaque count */
	if(compiler->swap)
	    swapinline64(offset);
	count = GETCOUNTER(offset);
	SKIPCOUNTER(offset);
        if(compiler->checksumming)
	    topvar->data.localchecksum = CRC32(topvar->data.localchecksum, offset, count);
	offset += count;
    }
    *offsetp = offset;
    return THROW(ret);
}

static int
walkStructArray(NCD4meta* compiler, NCD4node* topvar, NCD4node* var,  void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    for(i=0;i<dimproduct;i++) {
	/* Swap, possibly recursively, the single struct pointed to by offset*/
	if((ret=walkStruct(compiler,topvar,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return THROW(ret);
}

static int
walkStruct(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    int i;
    void* offset;
    NCD4node* basetype = var->basetype;

    offset = *offsetp;
    for(i=0;i<nclistlength(basetype->vars);i++) {
	NCD4node* field = (NCD4node*)nclistget(basetype->vars,i);
	NCD4node* fieldbase = field->basetype;
        switch (fieldbase->subsort) {
        default:
	    if((ret=walkAtomicVar(compiler,topvar,field,&offset))) goto done;
  	    break;
        case NC_STRUCT:
	    if((ret=walkStructArray(compiler,topvar,field,&offset))) goto done;
  	    break;
        case NC_SEQ:
	    if((ret=walkSeqArray(compiler,topvar,field,&offset))) goto done;
	    break;
        }
    }
    *offsetp = offset;
done:
    return THROW(ret);
}

static int
walkSeqArray(NCD4meta* compiler, NCD4node* topvar, NCD4node* var,	 void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    for(i=0;i<dimproduct;i++) {
	/* Swap, possibly recursively, the single seq pointed to by offset*/
	if((ret=walkSeq(compiler,topvar,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return THROW(ret);
}

static int
walkSeq(NCD4meta* compiler, NCD4node* topvar, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    int i;
    void* offset;
    d4size_t recordcount;

    offset = *offsetp;

    /* process the record count */
    if(compiler->checksumming)
        topvar->data.localchecksum = CRC32(topvar->data.localchecksum, offset, COUNTERSIZE);
	
    if(compiler->swap)
        swapinline64(offset);

    recordcount = GETCOUNTER(offset);
    SKIPCOUNTER(offset);

    for(i=0;i<recordcount;i++) {
	/* We can treat each record like a structure instance */
	if((ret=walkStruct(compiler,topvar,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return THROW(ret);
}
