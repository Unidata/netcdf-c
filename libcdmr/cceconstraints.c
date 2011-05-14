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
#include "cceconstraints.h"
#include "crdebug.h"
#include "cceparselex.h"

#define DEBUG

#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nullfree
#define nullfree(s) if((s)!=NULL) {free(s);} else {}
#endif

static int mergeprojection(CCEprojection* dst, CCEprojection* src);
static void ceallnodesr(CCEnode* node, NClist* allnodes, CEsort which);

/* Parse incoming url constraints, if any,
   to check for syntactic correctness
*/ 
int
cdmparseconstraint(char* constraints, CCEconstraint* constraint)
{
    int ncstat = NC_NOERR;
    char* errmsg;

    assert(constraint != NULL);
    if(constraint->projections == NULL)
	constraint->projections = nclistnew();
    nclistclear(constraint->projections);

    ncstat = cdmceparse(constraints,constraint,&errmsg);
    if(ncstat) {
	nclog(NCLOGWARN,"DAP constraint parse failure: %s",errmsg);
	if(errmsg) free(errmsg);
        nclistclear(constraint->projections);
    }

#ifdef DEBUG
fprintf(stderr,"constraint: %s",ccetostring((CCEnode*)constraint));
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
cceslicemerge(CCEslice* dst, CCEslice* src)
{
    int err = NC_NOERR;
    CCEslice tmp;

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
*/
int
cdmmergeprojections(NClist* dst, NClist* src)
{
    int i;
    NClist* cat = nclistnew();
    int ncstat = NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"dapmergeprojection: dst = %s\n",ccetostring((CCEnode*)dst));
fprintf(stderr,"dapmergeprojection: src = %s\n",ccetostring((CCEnode*)src));
#endif

    /* get dst concat clone(src) */
    nclistsetalloc(cat,nclistlength(dst)+nclistlength(src));
    for(i=0;i<nclistlength(dst);i++) {
	CCEprojection* p = (CCEprojection*)nclistget(dst,i);
	nclistpush(cat,(ncelem)p);
    }    
    for(i=0;i<nclistlength(src);i++) {
	CCEprojection* p = (CCEprojection*)nclistget(src,i);
	nclistpush(cat,(ncelem)cceclone((CCEnode*)p));
    }    

    nclistclear(dst);

    /* Repeatedly pull elements from the concat,
       merge with all duplicates, and stick into
       the dst
    */
    while(nclistlength(cat) > 0) {
	CCEprojection* target = (CCEprojection*)nclistremove(cat,0);
	if(target == NULL) continue;
        for(i=0;i<nclistlength(cat);i++) {
	    CCEprojection* p2 = (CCEprojection*)nclistget(cat,i);
	    if(p2 == NULL) continue;
	    if(ccesamepath(target->segments,
			   p2->segments)!=0) continue;
	    /* This entry matches our current target; merge  */
	    ncstat = mergeprojection(target,p2);
	    /* null out this merged entry and release it */
	    nclistset(cat,i,(ncelem)NULL);	    
	    ccefree((CCEnode*)p2);	    
	}		    
	/* Capture the clone */
	nclistpush(dst,(ncelem)target);
    }	    
    nclistfree(cat);
    return ncstat;
}

static int
mergeprojection(CCEprojection* dst, CCEprojection* src)
{
    int ncstat = NC_NOERR;
    int i,j;

    /* merge segment by segment;
       |dst->segments| == |src->segments|
       by construction
    */
    assert((nclistlength(dst->segments) == nclistlength(src->segments)));    
    for(i=0;i<nclistlength(dst->segments);i++) {
	CCEsegment* dstseg = (CCEsegment*)nclistget(dst->segments,i);
	CCEsegment* srcseg = (CCEsegment*)nclistget(src->segments,i);
	for(j=0;j<dstseg->rank;j++) {
	    cceslicemerge(dstseg->slices+j,
			  srcseg->slices+j);
	}
    }
    return ncstat;
}

/* Convert an CCEprojection instance into a string
   that can be used with the url
*/

char*
cdmbuildprojectionstring(NClist* projections)
{
    char* pstring;
    NCbytes* buf = ncbytesnew();
    ccelisttobuffer(projections,buf,",");
    pstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return pstring;
}

char*
cdmbuildselectionstring(NClist* selections)
{
    NCbytes* buf = ncbytesnew();
    char* sstring;
    ccelisttobuffer(selections,buf,",");
    sstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return sstring;
}

char*
cdmbuildprojection(NClist* projections)
{
    NCbytes* buf = ncbytesnew();
    char* result = NULL;
    ccelisttobuffer(projections,buf,",");
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

CCEnode*
cceclone(CCEnode* node)
{
    CCEnode* result = NULL;

    result = (CCEnode*)ccecreate(node->sort);
    if(result == NULL) goto done;

    switch (node->sort) {

    case CES_SLICE: {
	CCEslice* clone = (CCEslice*)result;
	CCEslice* orig = (CCEslice*)node;
	*clone = *orig;
    } break;

    case CES_SEGMENT: {
	CCEsegment* clone = (CCEsegment*)result;
	CCEsegment* orig = (CCEsegment*)node;
	*clone = *orig;	
        clone->name = nulldup(orig->name);
	if(orig->rank > 0)
	    memcpy(clone->slices,orig->slices,orig->rank*sizeof(CCEslice));
    } break;

    case CES_PROJECT: {
	CCEprojection* clone = (CCEprojection*)result;
	CCEprojection* orig = (CCEprojection*)node;
	*clone = *orig;	
        clone->segments = cceclonelist(orig->segments);
    } break;

    case CES_CONSTRAINT: {
	CCEconstraint* clone = (CCEconstraint*)result;
	CCEconstraint* orig = (CCEconstraint*)node;
	*clone = *orig;	
	clone->projections = cceclonelist(orig->projections);	
    } break;

    default:
	assert(0);
    }

done:
    return result;
}

NClist*
cceclonelist(NClist* list)
{
    int i;
    NClist* clone;
    if(list == NULL) return NULL;
    clone = nclistnew();
    for(i=0;i<nclistlength(list);i++) {
	CCEnode* node = (CCEnode*)nclistget(list,i);
	CCEnode* newnode = cceclone((CCEnode*)node);
	nclistpush(clone,(ncelem)newnode);
    }
    return clone;
}

void
ccefree(CCEnode* node)
{
    if(node == NULL) return;

    switch (node->sort) {

    case CES_CONSTRAINT: {
	CCEconstraint* target = (CCEconstraint*)node;
	ccefreelist(target->projections);
    } break;

    case CES_PROJECT: {
	CCEprojection* target = (CCEprojection*)node;
	ccefreelist(target->segments); break;
    } break;

    case CES_SEGMENT: {
	CCEsegment* target = (CCEsegment*)node;
	target->rank = 0;
        nullfree(target->name);
    } break;

    case CES_SLICE: {
    } break;

    default:
	assert(0);
    }

    /* final action */
    free(node);
}

void
ccefreelist(NClist* list)
{
    int i;
    if(list == NULL) return;
    for(i=0;i<nclistlength(list);i++) {
	CCEnode* node = (CCEnode*)nclistget(list,i);
	ccefree((CCEnode*)node);
    }
    nclistfree(list);
}

char*
ccetostring(CCEnode* node)
{
    char* s;
    NCbytes* buf = ncbytesnew();
    ccetobuffer(node,buf);
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return s;
}

void
ccetobuffer(CCEnode* node, NCbytes* buf)
{
    int i;
    char tmp[1024];

    if(buf == NULL) return;
    if(node == NULL) {ncbytesappend(buf,'?'); return;}

    switch (node->sort) {

    case CES_SLICE: {
	    CCEslice* slice = (CCEslice*)node;
	    size_t last = (slice->first+slice->length)-1;
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

    case CES_SEGMENT: {
	CCEsegment* segment = (CCEsegment*)node;
        int rank = segment->rank;
	char* name = (segment->name?segment->name:"<unknown>");
	ncbytescat(buf,nulldup(name));
        if(!cceiswholesegment(segment)) {
	    for(i=0;i<rank;i++) {
	        CCEslice* slice = segment->slices+i;
                ccetobuffer((CCEnode*)slice,buf);
	    }
	}
    } break;

    case CES_PROJECT: {
	CCEprojection* target = (CCEprojection*)node;
        ccelisttobuffer(target->segments,buf,".");
    } break;

    case CES_CONSTRAINT: {
	CCEconstraint* con = (CCEconstraint*)node;
        if(con->projections != NULL && nclistlength(con->projections) > 0) {
            ccelisttobuffer(con->projections,buf,",");
	}
    } break;

    case CES_NIL: {
	ncbytescat(buf,"<nil>");
    } break;

    default:
	assert(0);
    }
}

char*
ccelisttostring(NClist* list, char* sep)
{
    char* s;
    NCbytes* buf = ncbytesnew();
    ccelisttobuffer(list,buf,sep);
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return s;
}

void
ccelisttobuffer(NClist* list, NCbytes* buf, char* sep)
{
    int i;
    if(list == NULL || buf == NULL) return;
    if(sep == NULL) sep = ",";
    for(i=0;i<nclistlength(list);i++) {
	CCEnode* node = (CCEnode*)nclistget(list,i);
	if(i>0) ncbytescat(buf,sep);
	ccetobuffer((CCEnode*)node,buf);
    }
}

/* Collect all nodes within a specified constraint tree */
/* Caller frees result */
NClist*
cceallnodes(CCEnode* node, CEsort which)
{
    NClist* allnodes = nclistnew();
    ceallnodesr(node,allnodes,which);
    return allnodes;
}

static void
ceallnodesr(CCEnode* node, NClist* allnodes, CEsort which)
{
    int i;
    if(node == NULL) return;
    if(nclistcontains(allnodes,(ncelem)node)) return;
    if(which == CES_NIL || node->sort == which)
        nclistpush(allnodes,(ncelem)node);
    switch(node->sort) {
    case CES_PROJECT: {
	CCEprojection* projection = (CCEprojection*)node;
	for(i=0;i<nclistlength(projection->segments);i++)
	    ceallnodesr((CCEnode*)nclistget(projection->segments,i),allnodes,which);
    } break;
    case CES_CONSTRAINT: {
	CCEconstraint* constraint = (CCEconstraint*)node;
	for(i=0;i<nclistlength(constraint->projections);i++)
	    ceallnodesr((CCEnode*)nclistget(constraint->projections,i),allnodes,which);
    } break;

    /* All others have no subnodes */
    default:
	break;
    }
}

CCEnode*
ccecreate(CEsort sort)
{
    CCEnode* node = NULL;

    switch (sort) {

    case CES_SLICE: {
	CCEslice* target = (CCEslice*)calloc(1,sizeof(CCEslice));
	if(target == NULL) return NULL;
	node = (CCEnode*)target;
    } break;

    case CES_SEGMENT: {
	int i;
	CCEsegment* target = (CCEsegment*)calloc(1,sizeof(CCEsegment));
	if(target == NULL) return NULL;
	/* Initialize the sort of the slices */
	for(i=0;i<NC_MAX_VAR_DIMS;i++)
	    target->slices[i].node.sort = CES_SLICE;		
	node = (CCEnode*)target;
    } break;

    case CES_PROJECT: {
	CCEprojection* target = (CCEprojection*)calloc(1,sizeof(CCEprojection));
	if(target == NULL) return NULL;
	node = (CCEnode*)target;
    } break;

    case CES_CONSTRAINT: {
	CCEconstraint* target = (CCEconstraint*)calloc(1,sizeof(CCEconstraint));
	if(target == NULL) return NULL;
	node = (CCEnode*)target;
    } break;

    default:
	assert(0);
    }

    /* final action */
    node->sort = sort;
    return node;
}

void
ccemakewholeslice(CCEslice* slice, size_t declsize)
{
    slice->first = 0;
    slice->stride = 1;
    slice->length = declsize;
    slice->stop = declsize;
    slice->declsize = declsize;
    slice->count = declsize;
}

int
cceiswholeslice(CCEslice* slice)
{
    if(slice->first != 0 || slice->stride != 1) return 0;
    return 1;
}

int
cceiswholesegment(CCEsegment* seg)
{
    int i,whole;
    
    if(!seg->slicesdefined) return 0; /* actually, we don't know */
    whole = 1; /* assume so */
    for(i=0;i<seg->rank;i++) {
	if(!cceiswholeslice(&seg->slices[i])) {whole = 0; break;}	
    }
    return whole;
}

int
cceiswholesegmentlist(NClist* list)
{
    int i,whole;
    whole = 1; /* assume so */
    for(i=0;i<nclistlength(list);i++) {
        CCEsegment* segment = (CCEsegment*)nclistget(list,i);
	if(!cceiswholesegment(segment)) {whole = 0; break;}	
    }
    return whole;
}

int
ccesamepath(NClist* list1, NClist* list2)
{
    int i;
    int len = nclistlength(list1);
    if(len != nclistlength(list2)) return 0;
    for(i=0;i<len;i++) {
	CCEsegment* s1 = (CCEsegment*)nclistget(list1,i);
	CCEsegment* s2 = (CCEsegment*)nclistget(list2,i);
	if(strcmp(s1->name,s2->name) != 0) return 0;
    }
    return 1;
}
