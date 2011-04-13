/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/* Parser actions for constraint expressions */

/* Since oc does not use the constraint parser,
   they functions all just abort if called.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "netcdf.h"

#include "nclist.h"
#include "ncbytes.h"
#include "ncconstraints.h"

#include "ceparselex.h"
#include "ceparse.h"

#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nullfree
#define nullfree(s) if((s)!=NULL) {free(s);} else {}
#endif

static Object collectlist(Object list0, Object decl);

void
projections(CEparsestate* state, Object list0)
{
    NClist* list = (NClist*)list0;
    if(list != NULL) {
        nclistfree(state->constraint->projections);
        state->constraint->projections = list;
    }
#ifdef DEBUG
fprintf(stderr,"	ce.projections: %s\n",
	ncctostring((NCCnode*)state->constraint->projections));
#endif
}

void
selections(CEparsestate* state, Object list0)
{
    NClist* list = (NClist*)list0;
    if(list != NULL) {
        nclistfree(state->constraint->selections);
        state->constraint->selections = list;
    }
#ifdef DEBUG
fprintf(stderr,"	ce.selections: %s\n",
	ncctostring((NCCnode*)state->constraint->selections));
#endif
}


Object
projectionlist(CEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

Object
projection(CEparsestate* state, Object varorfcn)
{
    NCCprojection* p = (NCCprojection*)ncccreate(NS_PROJECT);
    NCCsort tag = *(NCCsort*)varorfcn;
    if(tag == NS_FCN)
	p->fcn = varorfcn;
    else
	p->var = varorfcn;
    p->discrim = tag;
#ifdef DEBUG
fprintf(stderr,"	ce.projection: %s\n",
	ncctostring((NCCnode*)p));
#endif
    return p;
}

Object
segmentlist(CEparsestate* state, Object var0, Object decl)
{
    /* watch out: this is non-standard */
    NClist* list;
    NCCvar* v = (NCCvar*)var0;
    if(v==NULL) v = (NCCvar*)ncccreate(NS_VAR);
    list = v->segments;
    if(list == NULL) list = nclistnew();
    nclistpush(list,(ncelem)decl);
    v->segments = list;
    return v;
}

Object
segment(CEparsestate* state, Object name, Object slices0)
{
    int i;
    NCCsegment* segment = (NCCsegment*)ncccreate(NS_SEGMENT);
    NClist* slices = (NClist*)slices0;
    segment->node.name = strdup((char*)name);
    if(slices != NULL) {
        segment->slicesdefined = 1;
	for(i=0;i<nclistlength(slices);i++) {
	    NCCslice* slice = (NCCslice*)nclistget(slices,i);
	    segment->slices[i] = *slice;
	    free(slice);
	}
	nclistfree(slices);
    } else
        segment->slicesdefined = 0;
#ifdef DEBUG
fprintf(stderr,"	ce.segment: %s\n",
	dumpsegment(segment));
#endif
    return segment;
}


Object
rangelist(CEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

Object
range(CEparsestate* state, Object sfirst, Object sstride, Object slast)
{
    NCCslice* slice = (NCCslice*)ncccreate(NS_SLICE);
    unsigned long first,stride,last;

    /* Note: that incoming arguments are strings; we must convert to size_t;
       but we do know they are legal integers or NULL */
    sscanf((char*)sfirst,"%lu",&first); /* always defined */
    if(slast != NULL)
        sscanf((char*)slast,"%lu",&last);
    else
	last = first;
    if(sstride != NULL)
        sscanf((char*)sstride,"%lu",&stride);
    else
	stride = 1; /* default */

    if(stride == 0)
    	ceerror(state,"Illegal index for range stride");
    if(last < first)
	ceerror(state,"Illegal index for range last index");
    slice->first  = first;
    slice->stride = stride;
    slice->stop   = last + 1;
    slice->length  = slice->stop - slice->first;
    slice->count  = slice->length / slice->stride;
#ifdef DEBUG
fprintf(stderr,"	ce.slice: %s\n",
	dumpslice(slice));
#endif
    return slice;
}

Object
range1(CEparsestate* state, Object rangenumber)
{
    int range = -1;
    sscanf((char*)rangenumber,"%u",&range);
    if(range < 0) {
    	ceerror(state,"Illegal range index");
    }
    return rangenumber;
}

Object
clauselist(CEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

Object
sel_clause(CEparsestate* state, int selcase,
	   Object lhs, Object relop0, Object values)
{
    NCCselection* sel = (NCCselection*)ncccreate(NS_SELECT);
    sel->operator = (NCCsort)relop0;
    sel->lhs = (NCCvalue*)lhs;
    if(selcase == 2) {/*singleton value*/
	sel->rhs = nclistnew();
	nclistpush(sel->rhs,(ncelem)values);
    } else
        sel->rhs = (NClist*)values;
    return sel;
}

Object
indexpath(CEparsestate* state, Object list0, Object index)
{
    return collectlist(list0,index);
}

Object
array_indices(CEparsestate* state, Object list0, Object indexno)
{
    NCCslice* slice;
    long long start = -1;
    NClist* list = (NClist*)list0;
    if(list == NULL) list = nclistnew();
    sscanf((char*)indexno,"%lld",&start);
    if(start < 0) {
    	ceerror(state,"Illegal array index");
	start = 1;
    }    
    slice = (NCCslice*)ncccreate(NS_SLICE);
    slice->first = start;
    slice->stride = 1;
    slice->count = 1;
    slice->length = 1;
    slice->stop = start+1;
    nclistpush(list,(ncelem)slice);
    return list;
}

Object
indexer(CEparsestate* state, Object name, Object indices)
{
    int i;
    NClist* list = (NClist*)indices;
    NCCsegment* seg = (NCCsegment*)ncccreate(NS_SEGMENT);
    seg->node.name = strdup((char*)name);
    for(i=0;i<nclistlength(list);i++) {    
	NCCslice* slice = (NCCslice*)nclistget(list,i);
        seg->slices[i] = *slice;
	free(slice);
    }
    nclistfree(indices);
    return seg;    
}

Object
function(CEparsestate* state, Object fcnname, Object args)
{
    NCCfcn* fcn = (NCCfcn*)ncccreate(NS_FCN);
    fcn->node.name = nulldup((char*)fcnname);
    fcn->args = args;
    return fcn;
}

Object
arg_list(CEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}


Object
value_list(CEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

Object
value(CEparsestate* state, Object val)
{
    NCCvalue* ncvalue = (NCCvalue*)ncccreate(NS_VALUE);
    NCCsort tag = *(NCCsort*)val;
    switch (tag) {
    case NS_VAR: ncvalue->var = (NCCvar*)val; break;
    case NS_FCN: ncvalue->fcn = (NCCfcn*)val; break;
    case NS_CONST: ncvalue->constant = (NCCconstant*)val; break;
    default: abort(); break;
    }
    ncvalue->discrim = tag;
    return ncvalue;
}

Object
var(CEparsestate* state, Object indexpath)
{
    NCCvar* v = (NCCvar*)ncccreate(NS_VAR);
    v->segments = (NClist*)indexpath;        
    return v;
}

Object
constant(CEparsestate* state, Object val, int tag)
{
    NCCconstant* con = (NCCconstant*)ncccreate(NS_CONST);
    char* text = (char*)val;
    char* endpoint = NULL;
    switch (tag) {
    case SCAN_STRINGCONST:
	con->discrim = NS_STR;
	con->text = nulldup(text);
	break;
    case SCAN_NUMBERCONST:
	con->intvalue = strtoll(text,&endpoint,10);
	if(*text != '\0' && *endpoint == '\0') {
	    con->discrim = NS_INT;
	} else {
	    con->floatvalue = strtod(text,&endpoint);
	    if(*text != '\0' && *endpoint == '\0')
	        con->discrim = NS_FLOAT;
	    else abort();
	}
	break;
    default: abort(); break;
    }
    return con;
}

static Object
collectlist(Object list0, Object decl)
{
    NClist* list = (NClist*)list0;
    if(list == NULL) list = nclistnew();
    nclistpush(list,(ncelem)decl);
    return list;
}

Object
makeselectiontag(NCCsort tag)
{
    return (Object) tag;
}

int
ceerror(CEparsestate* state, char* msg)
{
    strcpy(state->errorbuf,msg);
    state->errorcode=1;
    return 0;
}

static void
ce_parse_cleanup(CEparsestate* state)
{
    celexcleanup(&state->lexstate); /* will free */
}

static CEparsestate*
ce_parse_init(char* input, NCCconstraint* constraint)
{
    CEparsestate* state = NULL;
    if(input==NULL) {
        ceerror(state,"ce_parse_init: no input buffer");
    } else {
        state = (CEparsestate*)calloc(1,sizeof(CEparsestate));
        if(state==NULL) return (CEparsestate*)NULL;
        state->errorbuf[0] = '\0';
        state->errorcode = 0;
        celexinit(input,&state->lexstate);
	state->constraint = constraint;
    }
    return state;
}

#ifdef PARSEDEBUG
extern int cedebug;
#endif

/* Wrapper for ceparse */
int
ncceparse(char* input, NCCconstraint* constraint, char** errmsgp)
{
    CEparsestate* state;
    int errcode = 0;

#ifdef PARSEDEBUG
cedebug = 1;
#endif

    if(input != NULL) {
#ifdef DEBUG
fprintf(stderr,"ncceparse: input=%s\n",input);
#endif
        state = ce_parse_init(input,constraint);
        if(ceparse(state) == 0) {
#ifdef DEBUG
if(nclistlength(constraint->projections) > 0)
fprintf(stderr,"ncceparse: projections=%s\n",
        ncctostring((NCCnode*)constraint->projections));
#endif
#ifdef DEBUG
if(nclistlength(constraint->selections)  > 0)
fprintf(stderr,"ncceparse: selections=%s\n",
	dumpselections(constraint->selections));
#endif
	} else {
	    if(errmsgp) *errmsgp = nulldup(state->errorbuf);
	}
	errcode = state->errorcode;
        ce_parse_cleanup(state);
    }
    return errcode;
}

#ifdef PARSEDEBUG
Object
debugobject(Object o)
{
    return o;
}
#endif
