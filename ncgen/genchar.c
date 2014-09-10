/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "odom.h"

/******************************************************/
/* Code for generating char variables etc; mostly
   language independent */
/******************************************************/

static size_t gen_charconstant(NCConstant*, Bytebuffer*, int fillchar);
static int getfillchar(Datalist* fillsrc);
static void gen_chararrayr(Dimset*, Odometer*, int dimindex, Bytebuffer*, Datalist*, int fillchar);

/*
Matching strings to char variables, attributes, and vlen
constants is challenging because it is desirable to mimic
the original ncgen(3). The "algorithms" used there have no
simple characterization (such as "abc" == {'a','b','c'}).
So, this rather ugly code is kept in this file
and a variety of heuristics are used to mimic ncgen3.

The core algorithm is as follows.
1. Assume we have a set of dimensions D1..Dn.
   Any of the Di may be unlimited,
   but it is assumed that the sizes of the Di are all known.
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

Two special cases:
1. character vlen: char(*) vlen_t.
    For this case, we simply concat all the elements.
2. character attribute.
    For this case, we simply concat all the elements.
*/

void
gen_chararray(Dimset* dimset, Datalist* data, Bytebuffer* databuf, Datalist* fillsrc)
{
    int fillchar = getfillchar(fillsrc);
    Odometer* odom;

    ASSERT(bbLength(databuf) == 0);

    odom = newodometer(dimset,NULL,NULL);
    gen_chararrayr(dimset,odom,0,databuf,data,fillchar);
    odometerfree(odom);
}

/* Recursive helper */
static void
gen_chararrayr(Dimset* dimset, Odometer* odom, int dimindex,
               Bytebuffer* databuf, Datalist* data, int fillchar)
{
    NCConstant* con;

    /* If the data is a list of lists, then recurse */
    if(islistconst(datalistith(data,0))){ 
	/* Iterate over this odometer's dimension */
        Odometer* thisodom = newsubodometer(odom,dimset,dimindex,dimindex+1);
	while(odometermore(thisodom)) {
	    Datalist* sublist;
	    size_t offset = odometeroffset(thisodom);
	    /* Get the offset'th datalist; exception top-level list */
	    if(dimindex == 0) {
	        sublist = data;
	    } else {
	        con = datalistith(data,offset);
		if(isnilconst(con)) {
		    /* list is too short */
		    semerror(constline(con),"Datalist is shorter than corresponding dimension");
	            return;
	        }	    
	        if(!islistconst(con)) {
		    semerror(constline(con),"Expected data list {...}, constant found");
		    return;
	        }
		sublist = const2list(con);
	    }
            /* recurse using the helper function */
            gen_chararrayr(dimset,odom,dimindex+1,databuf,sublist,fillchar);
	    odometerincr(thisodom);	
	}
	odometerfree(thisodom);
    } else {/* Construct a single character constant according to the rules above */
	int len,i;
        size_t expectedsize,xproduct, unitsize;
        int rank = dimset->ndims;

        /* Compute crossproduct upto (but not including) the last dimension */
        xproduct = crossproduct(dimset,dimindex,rank-1);
        if(rank == 0)
	    unitsize = 1;
        else
            unitsize = dimset->dimsyms[rank-1]->dim.declsize;
	expectedsize = (xproduct * unitsize);

	/* get each char const, pad, and concat */        
	len = datalistlen(data);
	for(i=0;i<len;i++) {
	    NCConstant* con = datalistith(data,i);
	    if(isstringable(consttype(con))) {
		int j;
	        size_t constsize;
	        constsize = gen_charconstant(con,databuf,fillchar);
		if(constsize % unitsize > 0) {
	            size_t padsize = unitsize - (constsize % unitsize);
	            for(j=0;j<padsize;j++) bbAppend(databuf,fillchar);
		}
	    } else {
	        semwarn(constline(con),
		       "Encountered non-string and non-char constant in datalist; ignored");
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
	    if(bufsize % expectedsize > 0) {
	        size_t padsize = expectedsize - (bufsize % expectedsize);
                for(i=0;i<padsize;i++)
		    bbAppend(databuf,fillchar);
	    }
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

#ifndef CHARBUG
void
gen_leafchararray(Dimset* dimset, int lastunlim, Datalist* data,
                   Bytebuffer* databuf, Datalist* fillsrc)
{
    int i;
    size_t expectedsize,xproduct,unitsize;
    int ndims = dimset->ndims;
    int fillchar = getfillchar(fillsrc);
    int hasunlimited = (lastunlim < dimset->ndims);

    ASSERT(bbLength(databuf) == 0);

    /* Assume dimindex is the last unlimited (or 0 if their are
       no unlimiteds => we should be at a list of simple constants
    */

    /* Compute crossproduct starting at the last unlimited */
    if(hasunlimited)
	xproduct = crossproduct(dimset,lastunlim,ndims);
    else
	xproduct = crossproduct(dimset,0,ndims);

    /* Compute the required size (after padding) of each string constant */
    /* expected size is the size of concat of the string constants
       after padding
    */
    if(ndims == 0 || !hasunlimited) {
	unitsize = 1;
        expectedsize = (xproduct * unitsize);
    } else
    if(lastunlim == ndims-1) {/* last dimension is unlimited */
        unitsize = 1;
#if 0
        expectedsize = (xproduct*dimset->dimsyms[lastunlim]->dim.declsize);
#else
        expectedsize = (xproduct*unitsize);
#endif
    } else
    { /* last dim is not unlimited */
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
    } else {
	size_t bufsize = bbLength(databuf);
	/* Pad to size dimproduct size */
	if(bufsize % expectedsize > 0) {
	    size_t padsize = expectedsize - (bufsize % expectedsize);
            for(i=0;i<padsize;i++) bbAppend(databuf,fillchar);
	}
    }
}
#endif /*!CHARBUG*/
