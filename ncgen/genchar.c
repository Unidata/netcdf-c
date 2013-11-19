/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"

/******************************************************/
/* Code for generating char variables etc; mostly
   language independent */
/******************************************************/

static size_t gen_charconstant(NCConstant*, Bytebuffer*, int fillchar);
static int getfillchar(Datalist* fillsrc);
static void gen_chararrayr(Dimset*,int,int,Bytebuffer*,Datalist*,int,int,int);

/*
Matching strings to char variables, attributes, and vlen
constants is challenging because it is desirable to mimic
the original ncgen. The "algorithms" used there have no
simple characterization (such as "abc" == {'a','b','c'}).
So, this rather ugly code is kept in this file
and a variety of heuristics are used to mimic ncgen.

The core algorithm is as follows.
1. Assume we have a set of dimensions D1..Dn,
   where D1 may optionally be an Unlimited dimension.
   It is assumed that the sizes of the Di are all known.
2. Given a sequence of string or character constants
   C1..Cm, our goal is to construct a single string
   whose length is the cross product of D1 thru Dn.
3. For purposes of this algorithm, character constants
   are treated as strings of size 1.
4. Construct Dx = cross product of D1 thru D(n-1).
5. For each constant Ci, add fill characters, if necessary,
   so that its length is a multiple of Dn.
6. Concatenate the modified C1..Cm to produce string S.
7. Add fill characters to S to make its size be a multiple of
   Dn.
8. If S is longer than the Dx * Dn, then truncate
   and generate a warning.

Two other cases:
1. character vlen: char(*) vlen_t.
    For this case, we simply concat all the elements.
2. character attribute.
    For this case, we simply concat all the elements.
*/

void
gen_chararray(Dimset* dimset, Datalist* data, Bytebuffer* databuf, Datalist* fillsrc)
{
    int ndims,lastunlim;
    int fillchar = getfillchar(fillsrc);
    size_t expectedsize,xproduct;
    size_t unitsize;

    ASSERT(bbLength(databuf) == 0);

    ndims = dimset->ndims;


    /* Find the last unlimited */
    lastunlim = findlastunlimited(dimset);

    /* Compute crossproduct upto the last dimension,
       starting at the last unlimited
    */
    if(lastunlim < 0)
	xproduct = crossproduct(dimset,0,ndims-1);
    else
	xproduct = crossproduct(dimset,lastunlim,ndims-1);
    if(ndims == 0) {
	unitsize = 1;
    } else if(lastunlim == ndims-1) {/* last dimension is unlimited */
        unitsize = 1;
    } else { /* last dim is not unlimited */
        unitsize = dimset->dimsyms[ndims-1]->dim.declsize;
    }

    expectedsize = (xproduct * unitsize);

    gen_chararrayr(dimset,0,lastunlim,databuf,data,fillchar,unitsize,expectedsize);
}

/* Recursive helper */
static void
gen_chararrayr(Dimset* dimset, int dimindex, int lastunlim,
               Bytebuffer* databuf, Datalist* data, int fillchar,
	       int unitsize, int expectedsize)
{
    int i;
    size_t dimsize = dimset->dimsyms[dimindex]->dim.declsize;

    if(lastunlim >= 0 && dimindex < lastunlim) {
	/* keep recursing */
        for(i=0;i<dimsize;i++) {
	    NCConstant* c = datalistith(data,i);
	    ASSERT(islistconst(c));
	    gen_chararrayr(dimset,dimindex+1,lastunlim,databuf,
			   c->value.compoundv,fillchar,unitsize,expectedsize);
	}
    } else {/* we should be at a list of simple constants */
	for(i=0;i<data->length;i++) {
	    NCConstant* c = datalistith(data,i);
	    ASSERT(!islistconst(c));
	    if(isstringable(c->nctype)) {
		int j;
	        size_t constsize;
	        constsize = gen_charconstant(c,databuf,fillchar);
		if(constsize == 0 || constsize % unitsize > 0) {
	            size_t padsize = unitsize - (constsize % unitsize);
	            for(j=0;j<padsize;j++) bbAppend(databuf,fillchar);
		}
	    } else {
	        semwarn(constline(c),
		       "Encountered non-string and non-char constant in datalist; ignored");
	    }
	}
    }
    /* If |databuf| > expectedsize, complain: exception is zero length */
    if(bbLength(databuf) == 0 && expectedsize == 1) {
	/* this is okay */
    } else if(bbLength(databuf) > expectedsize) {
	semwarn(data->data[0].lineno,"character data list too long");
    } else {
	size_t bufsize = bbLength(databuf);
	/* Pad to size dimproduct size */
	if(bufsize == 0 || bufsize % expectedsize > 0) {
	    size_t padsize = expectedsize - (bufsize % expectedsize);
            for(i=0;i<padsize;i++) bbAppend(databuf,fillchar);
	}
    }
}

void
gen_charattr(Datalist* data, Bytebuffer* databuf)
{
    gen_charvlen(data,databuf);
}

void
gen_charvlen(Datalist* data, Bytebuffer* databuf)
{
    int i;
    NCConstant* c;

    ASSERT(bbLength(databuf) == 0);

    for(i=0;i<data->length;i++) {
	c = datalistith(data,i);
	if(isstringable(c->nctype)) {
	    (void)gen_charconstant(c,databuf,NC_FILL_CHAR);
	} else {
	    semerror(constline(c),
		     "Encountered non-string and non-char constant in datalist");
	    return;
	}
    }
}

static size_t
gen_charconstant(NCConstant* con, Bytebuffer* databuf, int fillchar)
{
    /* Following cases should be consistent with isstringable */
    size_t constsize = 1;
    switch (con->nctype) {
    case NC_CHAR:
        bbAppend(databuf,con->value.charv);
        break;
    case NC_BYTE:
        bbAppend(databuf,con->value.int8v);
        break;
    case NC_UBYTE:
        bbAppend(databuf,con->value.uint8v);
        break;
    case NC_STRING:
	constsize = con->value.stringv.len;
        bbAppendn(databuf,con->value.stringv.stringv,
                         con->value.stringv.len);
        bbNull(databuf);
        break;
    case NC_FILL:
        bbAppend(databuf,fillchar);
        break;
    default:
	PANIC("unexpected constant type");
    }
    return constsize;
}

static int
getfillchar(Datalist* fillsrc)
{
    /* Determine the fill char */
    int fillchar = 0;
    if(fillsrc != NULL && fillsrc->length > 0) {
	NCConstant* ccon = fillsrc->data;
	if(ccon->nctype == NC_CHAR) {
	    fillchar = ccon->value.charv;
	} else if(ccon->nctype == NC_STRING) {	    
	    if(ccon->value.stringv.len > 0) {
	        fillchar = ccon->value.stringv.stringv[0];
	    }
	}
    }
    if(fillchar == 0) fillchar = NC_FILL_CHAR; /* default */
    return fillchar;
}

void
gen_leafchararray(Dimset* dimset, DimProduct* dimproduct, Datalist* data,
                   Bytebuffer* databuf, Datalist* fillsrc)
{
    int i;
    size_t expectedsize,xproduct,unitsize;
    int ndims = dimset->ndims;
    int fillchar = getfillchar(fillsrc);

    ASSERT(bbLength(databuf) == 0);

    /* dimproduct->index ... dimproduct->rank - 1
       should reference a simple set of 
       no unlimiteds => we should be at a list of simple constants
    */

    /* Compute crossproduct upto (but not including) the last dimension,
       starting at dimindex; note that it is possible
       that dimindex == ndims-1 => xproduct will be 1
    */
    if(dimindex == ndims-1)
	xproduct = 1;
    else
	xproduct = crossproduct(dimset,dimindex,ndims-1);

    /* Basically, we need to construct 'unitsize' string constants
       where the unitsize is the size of the last dimension.
       Note that the actual constants we have to work with may need
       to be padded to The unitsize.
       The number of such unitsize chunks is determined by the
       cross-product of the dimensions starting with dimindex
       upto but not including the last dimension (ndims - 1).
    */
    if(ndims == 0) { /* scalar case => create 1 long string*/
	unitsize = 1;
        expectedsize = (xproduct * unitsize);
    } else if(dimindex == ndims-1) {
	ASSERT (ndims == 1)
        unitsize = 1;
        expectedsize = (unitsize*dimset->dimsyms[dimindex]->dim.declsize);
    } else { /* ndims > 1 && last dim is not unlimited */
        unitsize = dimset->dimsyms[ndims-1]->dim.declsize;
        expectedsize = (xproduct * unitsize);
    }

    for(i=0;i<data->length;i++) {
	NCConstant* c = datalistith(data,i);
	ASSERT(!islistconst(c));
	if(isstringable(c->nctype)) {
	    int j;
	    size_t constsize;
	    constsize = gen_charconstant(c,databuf,fillchar);
	    if(constsize == 0 || constsize % unitsize > 0) {
	        size_t padsize = unitsize - (constsize % unitsize);
	        for(j=0;j<padsize;j++) bbAppend(databuf,fillchar);
	    }
	} else {
	    semwarn(constline(c),"Encountered non-string and non-char constant in datalist; ignored");
	}
    }
    /* If |databuf| > expectedsize, complain: exception is zero length */
    if(bbLength(databuf) == 0 && expectedsize == 1) {
	/* this is okay */
    } else if(bbLength(databuf) > expectedsize) {
	semwarn(data->data[0].lineno,"character data list too long");
    } else { /* bbLength(databuf) <= expectedsize */
	size_t bufsize = bbLength(databuf);
	/* Pad to size dimproduct size */
	if(bufsize % expectedsize > 0) {
	    size_t padsize = expectedsize - (bufsize % expectedsize);
            for(i=0;i<padsize;i++) bbAppend(databuf,fillchar);
	}
    }
}
#endif /*!CHARBUG*/
