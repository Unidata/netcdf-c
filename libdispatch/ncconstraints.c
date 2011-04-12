/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.c,v 1.40 2010/05/27 21:34:07 dmh Exp $
 *********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "nclist.h"
#include "ncbytes.h"
#include "nclog.h"

#include "netcdf.h"
#include "ncconstraints.h"

#define DEBUG

#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nullfree
#define nullfree(s) if((s)!=NULL) {free(s);} else {}
#endif

static char* opstrings[] = OPSTRINGS ;

static int mergeprojection(NCCprojection* dst, NCCprojection* src);
static void ceallnodesr(NCCnode* node, NClist* allnodes, NCCsort which);

/* Parse incoming url constraints, if any,
   to check for syntactic correctness
*/ 
int
ncparseconstraints(char* constraints, NCCconstraint* ncconstraint)
{
    int ncstat = NC_NOERR;
    char* errmsg;

    assert(ncconstraint != NULL);
    nclistclear(ncconstraint->projections);
    nclistclear(ncconstraint->selections);

    ncstat = ncceparse(constraints,ncconstraint,&errmsg);
    if(ncstat) {
	nclog(NCLOGWARN,"DAP constraint parse failure: %s",errmsg);
	if(errmsg) free(errmsg);
        nclistclear(ncconstraint->projections);
        nclistclear(ncconstraint->selections);
    }

#ifdef DEBUG
fprintf(stderr,"constraint: %s",ncctostring((NCCnode*)ncconstraint));
#endif
    return ncstat;
}

/* Worksheet

mg.st = md.st * ms.st
mg.f  = md.f+(ms.f*md.st)
mg.l  = ((ms.l-1) / ms.st) * mg.st + 1
mg.p  = mg.f + mg.l
mg.c  = mg.l / mg.st

0000000000111111111122222222223
0123456789012345678901234567890
          xxxxxx
                xxxxxx
 0  1  2  3  4  5  6  7  8        md=(st=3 f=1 l=25 p=26)
          0     1     2           ms=(st=2 f=3 l=5  p=8 )
		                  ----------------------------
                                  mg=(st=6 f=10 p=23 l=13)
c = 4 / 2 = 2
l = 2 * 6 + 1 = 13

0000000000111111
0123456789012345
 0 1 2 3 4                md=(st=2 f=1 l=9 p=10)
     0 1 2                ms=(st=1 f=2 l=3 p=5)
                          ----------------------------
                          mg=(st=2 f=5 p=10 l=5 )
c = 2/1 = 2
l = 2 * 2 + 1 = 13

0000000000111111111
0123456789012345678
 0 1 2 3 4 5 6 7 8        md=(st=2 f=1 l=17 p=18)
       0   1   2          ms=(st=2 f=3 l=5 p=8)
		          ----------------------------
                          mg=(st=4 f=7 p=16 l=9 )
c = 4/2 = 2
l = 2 * 4 + 1 = 9

0000000000111111111
0123456789012345678
 0 1 2 3 4                md=(st=2 f=1 l=9 p=10)
 0 1 2 3 4                ms=(st=1 f=0 l=5 p=5)
		          ----------------------------
                          mg=(st=2 f=1 p=10 l=9 )
c = 4/1 = 4
l = 4 * 2 + 1 = 9

00000
01234
01                        md=(st=1 f=0 l=2 p=2)
0                         ms=(st=1 f=0 l=1 p=1)
		          ----------------------------
                          mg=(st=1 f=0 p=1 l=1 )
c = 0/1 = 0
l = 0 * 1 + 1 = 1

000000000011
012345678901
012                       md=(st=1 f=0 l=3 p=3)
012                       ms=(st=1 f=0 l=3 p=2)
		          ----------------------------
                          mg=(st=1 f=0 p=3 l=3 )
c = 2/1 = 2
l = 2 * 1 + 1 = 3

*/

/* Merge slice src into slice dst; dst != src */

int
nccslicemerge(NCCslice* dst, NCCslice* src)
{
    int err = NC_NOERR;
    NCCslice tmp;

    tmp.stride   = (dst->stride * src->stride);
    tmp.first    = (dst->first+((src->first)*(dst->stride)));
    tmp.length   = (((src->length - 1) / src->stride) * tmp.stride) + 1;
    tmp.stop     = tmp.first + tmp.length;
    tmp.count    = tmp.length / tmp.stride;
    tmp.declsize = dst->declsize;
    if(tmp.length % tmp.stride != 0) tmp.count++;
    if(tmp.first >= dst->stop || tmp.stop > dst->stop)
	err = NC_EINVALCOORDS;
    else
	*dst = tmp;
    return err;
}


/*
Given two projection lists, merge
src into dst taking
overlapping projections into acct.
Assume that fullnames have been computed.
*/
int
nccmergeprojections(NClist* dst, NClist* src)
{
    int i;
    NClist* cat = nclistnew();
    int ncstat = NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"nccmergeprojection: dst = %s\n",ncctostring((NCCnode*)dst));
fprintf(stderr,"nccmergeprojection: src = %s\n",ncctostring((NCCnode*)src));
#endif

    /* get dst concat clone(src) */
    nclistsetalloc(cat,nclistlength(dst)+nclistlength(src));
    for(i=0;i<nclistlength(dst);i++) {
	NCCprojection* p = (NCCprojection*)nclistget(dst,i);
	nclistpush(cat,(ncelem)p);
    }    
    for(i=0;i<nclistlength(src);i++) {
	NCCprojection* p = (NCCprojection*)nclistget(src,i);
	nclistpush(cat,(ncelem)nccclone((NCCnode*)p));
    }    

    nclistclear(dst);

    /* Repeatedly pull elements from the concat,
       merge with all duplicates, and stick into
       the dst
    */
    while(nclistlength(cat) > 0) {
	NCCprojection* target = (NCCprojection*)nclistremove(cat,0);
	if(target == NULL) continue;
        if(target->discrim != NS_VAR) continue;
        for(i=0;i<nclistlength(cat);i++) {
	    NCCprojection* p2 = (NCCprojection*)nclistget(cat,i);
	    if(p2 == NULL) continue;
	    if(p2->discrim != NS_VAR) continue;
	    if(strcmp(target->node.fullname,p2->node.fullname)!=0) continue;
	    /* This entry matches our current target; merge  */
	    ncstat = mergeprojection(target,p2);
	    /* null out this merged entry and release it */
	    nclistset(cat,i,(ncelem)NULL);	    
	    nccfree((NCCnode*)p2);	    
	}		    
	/* Capture the clone */
	nclistpush(dst,(ncelem)target);
    }	    
    nclistfree(cat);
    return ncstat;
}

static int
mergeprojection(NCCprojection* dst, NCCprojection* src)
{
    int ncstat = NC_NOERR;
    int i,j;

    /* merge segment by segment;
       |dst->segments| == |src->segments|
       by construction
    */
    assert((dst->discrim == NS_VAR && src->discrim == NS_VAR));
    assert((nclistlength(dst->var->segments) == nclistlength(src->var->segments)));    
    for(i=0;i<nclistlength(dst->var->segments);i++) {
	NCCsegment* dstseg = (NCCsegment*)nclistget(dst->var->segments,i);
	NCCsegment* srcseg = (NCCsegment*)nclistget(src->var->segments,i);
	for(j=0;j<dstseg->slicerank;j++) {
	    nccslicemerge(&dstseg->slices[j],&srcseg->slices[j]);
	}
    }
    return ncstat;
}

/* Convert an NCCprojection instance into a string
   that can be used with the url
*/

char*
buildprojectionstring(NClist* projections)
{
    char* pstring;
    NCbytes* buf = ncbytesnew();
    ncc_listtobuffer(projections,buf);
    pstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return pstring;
}

char*
buildselectionstring(NClist* selections)
{
    NCbytes* buf = ncbytesnew();
    char* sstring;
    ncc_listtobuffer(selections,buf);
    sstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return sstring;
}

char*
buildconstraintstring(NCCconstraint* constraints)
{
    NCbytes* buf = ncbytesnew();
    char* result = NULL;
    ncctobuffer((NCCnode*)constraints,buf);
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

NCCnode*
nccclone(NCCnode* node)
{
    int i;
    NCCnode* result = NULL;

    result = (NCCnode*)ncccreate(node->sort);
    if(result == NULL) goto done;
    result->name = nulldup(node->name);
    result->fullname = nulldup(node->fullname);

    switch (node->sort) {

    case NS_SLICE: {
	NCCslice* clone = (NCCslice*)result;
	NCCslice* orig = (NCCslice*)node;
	*clone = *orig;
    } break;

    case NS_SEGMENT: {
	NCCsegment* clone = (NCCsegment*)result;
	NCCsegment* orig = (NCCsegment*)node;
	*clone = *orig;	
        for(i=0;i<clone->slicerank;i++) {
	    NCCslice* slicep = (NCCslice*)nccclone((NCCnode*)orig->slices+i);
	    /* special case handling */
	    if(slicep != NULL)
	        clone->slices[i] = *slicep;
	    nccfree((NCCnode*)slicep);
	}
    } break;

    case NS_VAR: {
	NCCvar* clone = (NCCvar*)result;
	NCCvar* orig = (NCCvar*)node;
	*clone = *orig;
	clone->segments = ncc_clonelist(clone->segments);
    } break;

    case NS_FCN: {
	NCCfcn* clone = (NCCfcn*)result;
	NCCfcn* orig = (NCCfcn*)node;
	*clone = *orig;
	clone->args = ncc_clonelist(orig->args);
    } break;

    case NS_CONST: {
	NCCconstant* clone = (NCCconstant*)result;
	NCCconstant* orig = (NCCconstant*)node;
	*clone = *orig;
        if(clone->discrim ==  NS_STR)
	    clone->text = nulldup(clone->text);
    } break;

    case NS_VALUE: {
	NCCvalue* clone = (NCCvalue*)result;
	NCCvalue* orig = (NCCvalue*)node;
	*clone = *orig;
        switch (clone->discrim) {
        case NS_CONST:
	    clone->constant = (NCCconstant*)nccclone((NCCnode*)orig->constant); break;
        case NS_VAR:
	    clone->var = (NCCvar*)nccclone((NCCnode*)orig->var); break;
        case NS_FCN:
	    clone->fcn = (NCCfcn*)nccclone((NCCnode*)orig->fcn); break;
        default: assert(0);
        }
    } break;

    case NS_PROJECT: {
	NCCprojection* clone = (NCCprojection*)result;
	NCCprojection* orig = (NCCprojection*)node;
	*clone = *orig;	
	switch (orig->discrim) {
	case NS_VAR:
            clone->var = (NCCvar*)nccclone((NCCnode*)orig->var); break;
	case NS_FCN:
            clone->fcn = (NCCfcn*)nccclone((NCCnode*)orig->fcn); break;
	default: assert(0);
	}
    } break;

    case NS_SELECT: {
	NCCselection* clone = (NCCselection*)result;
	NCCselection* orig = (NCCselection*)node;
	*clone = *orig;	
	clone->lhs = (NCCvalue*)nccclone((NCCnode*)orig->lhs);
	clone->rhs = ncc_clonelist(orig->rhs);
    } break;

    case NS_CONSTRAINT: {
	NCCconstraint* clone = (NCCconstraint*)result;
	NCCconstraint* orig = (NCCconstraint*)node;
	*clone = *orig;	
	clone->projections = ncc_clonelist(orig->projections);	
	clone->selections = ncc_clonelist(orig->selections);
    } break;

    default:
	assert(0);
    }

done:
    return result;
}

NClist*
ncc_clonelist(NClist* list)
{
    int i;
    NClist* clone;
    if(list == NULL) return NULL;
    clone = nclistnew();
    for(i=0;i<nclistlength(list);i++) {
	NCCnode* node = (NCCnode*)nclistget(list,i);
	NCCnode* newnode = nccclone((NCCnode*)node);
	nclistpush(clone,(ncelem)newnode);
    }
    return clone;
}

void
nccfree(NCCnode* node)
{
    int i;

    if(node == NULL) return;

    nullfree(node->fullname);
    nullfree(node->name);
    nullfree(node->notes);

    switch (node->sort) {

    case NS_VAR: {
	NCCvar* target = (NCCvar*)node;
	ncc_freelist(target->segments);	
    } break;

    case NS_FCN: {
	NCCfcn* target = (NCCfcn*)node;
	ncc_freelist(target->args);	
    } break;

    case NS_CONST: {
	NCCconstant* target = (NCCconstant*)node;
	if(target->discrim == NS_STR)
	    nullfree(target->text);
    } break;

    case NS_VALUE: {
	NCCvalue* target = (NCCvalue*)node;
	switch(target->discrim) {
        case NS_CONST: nccfree((NCCnode*)target->constant); break;    
        case NS_VAR: nccfree((NCCnode*)target->var); break;    
        case NS_FCN: nccfree((NCCnode*)target->fcn); break;    
        default: assert(0);
        }
    } break;

    case NS_PROJECT: {
	NCCprojection* target = (NCCprojection*)node;
	switch (target->discrim) {
	case NS_VAR: nccfree((NCCnode*)target->var); break;
	case NS_FCN: nccfree((NCCnode*)target->fcn); break;
	default: assert(0);
	}
    } break;

    case NS_SELECT: {
	NCCselection* target = (NCCselection*)node;
	ncc_freelist(target->rhs);
	nccfree((NCCnode*)target->lhs);
    } break;

    case NS_CONSTRAINT: {
	NCCconstraint* target = (NCCconstraint*)node;
	ncc_freelist(target->projections);
	ncc_freelist(target->selections);
    } break;

    case NS_SEGMENT:
    case NS_SLICE:
	break;

    default:
	assert(0);
    }

    /* final action */
    free(node);
}

void
ncc_freelist(NClist* list)
{
    int i;
    if(list == NULL) return;
    for(i=0;i<nclistlength(list);i++) {
	NCCnode* node = (NCCnode*)nclistget(list,i);
	nccfree((NCCnode*)node);
    }
    nclistfree(list);
}

char*
ncctostring(NCCnode* node)
{
    char* s;
    NCbytes* buf = ncbytesnew();
    ncctobuffer(node,buf);
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return s;
}

void
ncctobuffer(NCCnode* node, NCbytes* buf)
{
    int i;
    char tmp[1024];

    if(buf == NULL) return;
    if(node == NULL) {ncbytesappend(buf,'?'); return;}

    switch (node->sort) {

    case NS_SLICE: {
	NCCslice* slice = (NCCslice*)node;
	unsigned long last = (slice->first+slice->length)-1;
	assert(slice->declsize > 0);
	if(last > slice->declsize && slice->declsize > 0)
	    last = slice->declsize - 1;
        if(slice->count == 1) {
            snprintf(tmp,sizeof(tmp),"[%lu]",
	        (unsigned long)slice->first);
        } else if(slice->stride == 1) {
            snprintf(tmp,sizeof(tmp),"[%lu:%lu]",
	        (unsigned long)slice->first,
	        (unsigned long)last);
        } else {
	    snprintf(tmp,sizeof(tmp),"[%lu:%lu:%lu]",
		    (unsigned long)slice->first,
		    (unsigned long)slice->stride,
		    (unsigned long)last);
	}
        ncbytescat(buf,tmp);
    } break;

    case NS_SEGMENT: {
	NCCsegment* segment = (NCCsegment*)node;
        int rank = segment->slicerank;
	ncbytescat(buf,(segment->node.name?segment->node.name:"<unknown>"));
	for(i=0;i<rank;i++) {
	    NCCslice* slice = (segment->slices+i);
	    if(i > 0) ncbytescat(buf,",");
	    ncctobuffer((NCCnode*)slice,buf);
	}
    } break;

    case NS_VAR: {
	NCCvar* var = (NCCvar*)node;
	ncc_listtobuffer(var->segments,buf);
    } break;

    case NS_FCN: {
	NCCfcn* fcn = (NCCfcn*)node;
        ncbytescat(buf,fcn->node.name);
        ncbytescat(buf,"(");
	ncc_listtobuffer(fcn->args,buf);
        ncbytescat(buf,")");
    } break;

    case NS_CONST: {
	NCCconstant* value = (NCCconstant*)node;
        switch (value->discrim) {
        case NS_STR:
	    ncbytescat(buf,value->text);
	    break;		
        case NS_INT:
            snprintf(tmp,sizeof(tmp),"%lld",value->intvalue);
            ncbytescat(buf,tmp);
    	break;
        case NS_FLOAT:
            snprintf(tmp,sizeof(tmp),"%g",value->floatvalue);
            ncbytescat(buf,tmp);
	    break;
        default: assert(0);
	}
    } break;

    case NS_VALUE: {
	NCCvalue* value = (NCCvalue*)node;
        switch (value->discrim) {
        case NS_CONST:
    	    ncctobuffer((NCCnode*)value->constant,buf);
      	    break;		
        case NS_VAR:
    	    ncctobuffer((NCCnode*)value->var,buf);
    	    break;
        case NS_FCN:
    	    ncctobuffer((NCCnode*)value->fcn,buf);
    	    break;
        default: assert(0);
        }
    } break;

    case NS_PROJECT: {
	NCCprojection* target = (NCCprojection*)node;
	switch (target->discrim) {
	case NS_VAR: ncctobuffer((NCCnode*)target->var,buf); break;
	case NS_FCN: ncctobuffer((NCCnode*)target->fcn,buf); break;
	default: assert(0);
	}
    } break;

    case NS_SELECT: {
	NCCselection* sel = (NCCselection*)node;
	ncctobuffer((NCCnode*)sel->rhs,buf);
        if(sel->operator == NS_NIL) break;
        ncbytescat(buf,opstrings[(int)sel->operator]);
        if(nclistlength(sel->rhs) > 1)
            ncbytescat(buf,"{");
	ncc_listtobuffer(sel->rhs,buf);
        if(nclistlength(sel->rhs) > 1)
	    ncbytescat(buf,"}");
    } break;

    case NS_CONSTRAINT: {
	NCCconstraint* con = (NCCconstraint*)node;
	if(con->projections != NULL)
            ncc_listtobuffer(con->projections,buf);
        if(con->selections != NULL) {
	    ncbytescat(buf,"?");
            ncc_listtobuffer(con->selections,buf);
	}
    } break;

    default:
	assert(0);
    }

    /* final action */
    free(node);
}

char*
ncc_listtostring(NClist* list)
{
    char* s;
    NCbytes* buf = ncbytesnew();
    ncc_listtobuffer(list,buf);
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return s;
}

void
ncc_listtobuffer(NClist* list, NCbytes* buf)
{
    int i;
    if(list == NULL || buf == NULL) return;
    for(i=0;i<nclistlength(list);i++) {
	NCCnode* node = (NCCnode*)nclistget(list,i);
	if(i>0) ncbytescat(buf,",");
	ncctobuffer((NCCnode*)node,buf);
    }
}

/* Collect all nodes within a specified constraint tree */
/* Caller frees result */
NClist*
ceallnodes(NCCnode* node, NCCsort which)
{
    NClist* allnodes = nclistnew();
    ceallnodesr(node,allnodes,which);
    return allnodes;
}

static void
ceallnodesr(NCCnode* node, NClist* allnodes, NCCsort which)
{
    int i;
    if(node == NULL) return;
    if(nclistcontains(allnodes,(ncelem)node)) return;
    if(which == NS_NIL || node->sort == which)
        nclistpush(allnodes,(ncelem)node);
    switch(node->sort) {
    case NS_FCN: {
	NCCfcn* fcn = (NCCfcn*)node;
	for(i=0;i<nclistlength(fcn->args);i++) {
	    ceallnodesr((NCCnode*)nclistget(fcn->args,i),allnodes,which);
	}
    } break;
    case NS_VAR: {
	NCCvar* var = (NCCvar*)node;
	for(i=0;i<nclistlength(var->segments);i++) {
	    ceallnodesr((NCCnode*)nclistget(var->segments,i),allnodes,which);
	}
    } break;
    case NS_VALUE: {
	NCCvalue* value = (NCCvalue*)node;
	if(value->discrim == NS_VAR)
	    ceallnodesr((NCCnode*)value->var,allnodes,which);
	else if(value->discrim == NS_FCN)
	    ceallnodesr((NCCnode*)value->fcn,allnodes,which);
	else
	    ceallnodesr((NCCnode*)value->constant,allnodes,which);
    } break;
    case NS_SELECT: {
	NCCselection* selection = (NCCselection*)node;
        ceallnodesr((NCCnode*)selection->lhs,allnodes,which);
	for(i=0;i<nclistlength(selection->rhs);i++)
            ceallnodesr((NCCnode*)nclistget(selection->rhs,i),allnodes,which);
    } break;
    case NS_PROJECT: {
	NCCprojection* projection = (NCCprojection*)node;
	if(projection->discrim == NS_VAR)
	    ceallnodesr((NCCnode*)projection->var,allnodes,which);
	else
	    ceallnodesr((NCCnode*)projection->fcn,allnodes,which);
    } break;
    case NS_CONSTRAINT: {
	NCCconstraint* constraint = (NCCconstraint*)node;
	for(i=0;i<nclistlength(constraint->projections);i++)
	    ceallnodesr((NCCnode*)nclistget(constraint->projections,i),allnodes,which);
	for(i=0;i<nclistlength(constraint->selections);i++)
	    ceallnodesr((NCCnode*)nclistget(constraint->selections,i),allnodes,which);
    } break;

    /* All others have no subnodes */
    default:
	break;
    }
}

NCCnode*
ncccreate(NCCsort sort)
{
    NCCnode* node = NULL;

    switch (sort) {

    case NS_SLICE: {
	NCCsegment* target = (NCCsegment*)calloc(1,sizeof(NCCsegment));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
    } break;

    case NS_SEGMENT: {
	NCCsegment* target = (NCCsegment*)calloc(1,sizeof(NCCsegment));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
    } break;

    case NS_CONST: {
	NCCconstant* target = (NCCconstant*)calloc(1,sizeof(NCCconstant));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
	target->discrim == NS_NIL;
    } break;

    case NS_VALUE: {
	NCCvalue* target = (NCCvalue*)calloc(1,sizeof(NCCvalue));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
	target->discrim == NS_NIL;
    } break;

    case NS_VAR: {
	NCCvar* target = (NCCvar*)calloc(1,sizeof(NCCvar));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
    } break;

    case NS_FCN: {
	NCCfcn* target = (NCCfcn*)calloc(1,sizeof(NCCfcn));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
    } break;

    case NS_PROJECT: {
	NCCprojection* target = (NCCprojection*)calloc(1,sizeof(NCCprojection));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
    } break;

    case NS_SELECT: {
	NCCselection* target = (NCCselection*)calloc(1,sizeof(NCCselection));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
	target->operator = NO_NIL;
    } break;

    case NS_CONSTRAINT: {
	NCCconstraint* target = (NCCconstraint*)calloc(1,sizeof(NCCconstraint));
	if(target == NULL) return NULL;
	node = (NCCnode*)target;
    } break;

    default:
	assert(0);
    }

    /* final action */
    node->sort = sort;
    return node;
}

