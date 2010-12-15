/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.c,v 1.40 2010/05/27 21:34:07 dmh Exp $
 *********************************************************************/
#include "ncdap3.h"
#include "dapodom.h"
#include "dapdebug.h"
#include "dapdump.h"

static NCerror mergeprojection31(NCprojection*, NCprojection*);

static NCerror matchpartialname3(NClist*, NClist*, CDFnode**);
static void collectsegmentnames3(NClist* segments, NClist* path);
static void completesegments3(NClist* fullpath, NClist* segments);

static NCerror qualifyconstraints3(NCconstraint*);
static NCerror qualifyprojectionnames3(NCprojection*);
static NCerror qualifyprojectionsizes3(NCprojection*);
static NCerror qualifyselectionnames3(NCselection*);
static NClist* unifyprojectionnodes3(NClist* varlist);
static int treecontains3(CDFnode* var, CDFnode* root);

static void ceallnodesr(NCany* node, NClist* allnodes, NCsort which);
static void tostringncunknown(NCbytes* buf);

/* Parse incoming url constraints, if any,
   to check for syntactic correctness */ 
NCerror
parsedapconstraints(NCDAPCOMMON* nccomm, char* constraints,
		    NCconstraint* dapconstraint)
{
    NCerror ncstat = NC_NOERR;
    char* errmsg;

    ASSERT(dapconstraint != NULL);
    nclistclear(dapconstraint->projections);
    nclistclear(dapconstraint->selections);

    ncstat = ncceparse(constraints,0,dapconstraint,&errmsg);
    if(ncstat) {
	oc_log(OCLOGWARN,"DAP constraint parse failure: %s",errmsg);
	efree(errmsg);
        nclistclear(dapconstraint->projections);
        nclistclear(dapconstraint->selections);
    }
#ifdef DEBUG
fprintf(stderr,"constraint: %s",dumpconstraint(dapconstraint));
#endif
    return ncstat;
}

/* Map constrain paths to a CDFnode path; the difficulty
   is that suffix paths are legal.
*/

NCerror
mapconstraints3(NCDAPCOMMON* nccomm)
{
    int i;
    NCerror ncstat = NC_NOERR;
    CDFnode* root = nccomm->cdf.ddsroot;
    NClist* nodes = root->tree->nodes;
    NClist* dapprojections;
    NClist* dapselections;

    dapprojections = nccomm->oc.dapconstraint->projections;
    dapselections = nccomm->oc.dapconstraint->selections;

    /* Convert the projection paths to leaves in the dds tree */
    for(i=0;i<nclistlength(dapprojections);i++) {
	NCprojection* proj = (NCprojection*)nclistget(dapprojections,i);
	if(proj->discrim != NS_VAR) continue;
	ncstat = matchpartialname3(nodes,proj->var->segments,&proj->var->leaf);
	if(ncstat) goto done;
    }

    /* Convert the selection paths to leaves in the dds tree */
    for(i=0;i<nclistlength(dapselections);i++) {
	NCselection* sel = (NCselection*)nclistget(dapselections,i);
	if(sel->lhs->discrim != NS_VAR) continue;
	ncstat = matchpartialname3(nodes,sel->lhs->var->segments,&sel->lhs->var->leaf);
	if(ncstat) goto done;
    }
   
    /* Convert the selection path values to leaves in the dds tree */
    for(i=0;i<nclistlength(dapselections);i++) {
	int j;
	NCselection* sel = (NCselection*)nclistget(dapselections,i);
	for(j=0;j<nclistlength(sel->rhs);j++) {
	    NCvalue* value = (NCvalue*)nclistget(sel->rhs,j);
	    if(value->discrim != NS_VAR) continue;
	    ncstat = matchpartialname3(nodes,value->var->segments,&value->var->node);
	    if(ncstat) goto done;
	}
    }
    /* Fill in segment information */
    ncstat = qualifyconstraints3(nccomm->oc.dapconstraint);
    if(ncstat != NC_NOERR) goto done;

#ifdef DEBUG
fprintf(stderr,"mapconstraint.projections: %s\n",
		dumpprojections(dapprojections));
fprintf(stderr,"mapconstraint.selections: %s\n",
		dumpselections(dapselections));
#endif

done:
    return THROW(ncstat);
}

/* Fill in:
    1. projection segments
    2. projection segment slices declsize
    3. selection path
*/
static NCerror
qualifyconstraints3(NCconstraint* constraint)
{
    NCerror ncstat = NC_NOERR;
    int i;
    if(constraint != NULL) {
        for(i=0;i<nclistlength(constraint->projections);i++) {  
            NCprojection* p = (NCprojection*)nclistget(constraint->projections,i);
            ncstat = qualifyprojectionnames3(p);
            ncstat = qualifyprojectionsizes3(p);
        }
        for(i=0;i<nclistlength(constraint->selections);i++) {   
            NCselection* s = (NCselection*)nclistget(constraint->selections,i);
            ncstat = qualifyselectionnames3(s);
        }
    }
    return ncstat;
}

static void
completesegments3(NClist* fullpath, NClist* segments)
{
    int i,delta;
    /* add path nodes to create full path */
    delta = (nclistlength(fullpath) - nclistlength(segments));
    ASSERT((delta >= 0));
    for(i=0;i<delta;i++) {
	int j;
        NCsegment* seg = createncsegment();
        CDFnode* node = (CDFnode*)nclistget(fullpath,i);
        seg->name = nulldup(node->name);
        seg->node = node;
        for(j=0;j<nclistlength(node->array.dimensions);j++) {
            CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions0,j);
            makewholeslice3(&seg->slices[j],dim);
        }
        seg->slicerank = nclistlength(node->array.dimensions);
        nclistinsert(segments,j,(ncelem)seg);
    }
    /* Now modify the segments to point to the appropriate node
       and fill in the slices.
    */
    for(i=delta;i<nclistlength(segments);i++) {
        NCsegment* seg = (NCsegment*)nclistget(segments,i);
        CDFnode* node = (CDFnode*)nclistget(fullpath,i);
	seg->node = node;
        if(!seg->slicesdefined) {
	    makewholesegment3(seg,node);
	}
    }
}

/* convert all names in projections in paths to be fully qualified
   by adding prefix segment objects. Also verify ranks
*/
static NCerror
qualifyprojectionnames3(NCprojection* proj)
{
    NCerror ncstat = NC_NOERR;
    NClist* fullpath = nclistnew();

    ASSERT((proj->discrim == NS_VAR && proj->var->leaf != NULL && proj->var->leaf->dds != OCNULL));
    collectnodepath3(proj->var->leaf,fullpath,!WITHDATASET);
#ifdef DEBUG
fprintf(stderr,"qualify: %s -> ",
	dumpprojection(proj));
#endif
    /* Now add path nodes to create full path */
    completesegments3(fullpath,proj->var->segments);

#ifdef DEBUG
fprintf(stderr,"%s\n",
	dumpprojection(proj));
#endif
    nclistfree(fullpath);
    return ncstat;
}

/* Make sure that the slice declsizes are all defined for this projection */
static NCerror
qualifyprojectionsizes3(NCprojection* proj)
{
    int i,j;
    ASSERT(proj->discrim == NS_VAR);
    for(i=0;i<nclistlength(proj->var->segments);i++) {
        NCsegment* seg = (NCsegment*)nclistget(proj->var->segments,i);
	NClist* dimset = NULL;
	int rank;
	ASSERT(seg->node != NULL);
	/* Must use dimensions0 (the original set) because
           the segments are wrt the underlying dap DDS */
        dimset = seg->node->array.dimensions0;
        rank = nclistlength(dimset);
        for(j=0;j<rank;j++) {
	    CDFnode* dim = (CDFnode*)nclistget(dimset,j);
            seg->slices[j].declsize = dim->dim.declsize;	    
	}
    }
    return NC_NOERR;
}

   
/* convert all names in selections to be fully qualified */
static NCerror
qualifyselectionnames3(NCselection* sel)
{
    NCerror ncstat = NC_NOERR;
    int i;
    NClist* segments = NULL;
    NClist* fullpath = nclistnew();

    ASSERT(sel->lhs->discrim == NS_VAR);
    collectnodepath3(sel->lhs->var->leaf,fullpath,!WITHDATASET);
#ifdef DEBUG
fprintf(stderr,"qualify.sel: %s -> ",
	dumpselection(sel));
#endif
    /* Now add path nodes to create full path */
    completesegments3(fullpath,sel->lhs->var->segments);
    for(i=0;i<nclistlength(sel->rhs);i++) {
        NCvalue* value = (NCvalue*)nclistget(sel->rhs,i);
        if(value->discrim != NS_VAR) continue;
        nclistclear(fullpath);
        collectnodepath3(value->var->node,fullpath,!WITHDATASET);
	completesegments3(fullpath,value->var->segments);
    }
    nclistfree(segments);
    nclistfree(fullpath);
    return THROW(ncstat);
}

/*
We are given a set of segments (in path)
representing a partial path for a DAP variable.
Our goal is to locate all matching
DAP variables for which the path of that
variable has a suffix matching
the given partial path.
If one node matches exactly, then use that one;
otherwise there had better be exactly one
match else ambiguous.
Additional constraints (4/12/2010):
1. if a segment is dimensioned, then use that info
   to distinguish e.g a grid node from a possible
   grid array within it of the same name.
   Treat sequences as of rank 1.
2. if there are two matches, and one is the grid
   and the other is the grid array within that grid,
   then choose the grid array.
3. If there are multiple matches choose the one with the
   shortest path
4. otherwise complain about ambiguity
*/

static int
matchsuffix3(NClist* matchpath, NClist* segments, int index0)
{
    int i;
    int nsegs = nclistlength(segments);
    ASSERT(index0 >= 0 && (index0+nsegs) <= nclistlength(matchpath));
    for(i=0;i<nsegs;i++) {
	NCsegment* seg = (NCsegment*)nclistget(segments,i);
	CDFnode* node = (CDFnode*)nclistget(matchpath,index0+i);
	int match;
	/* Do the names match */
	if(strcmp(seg->name,node->name) != 0) return 0; /* no match */
	/* Do the ranks match (watch out for sequences) */
	if(seg->slicerank == 0) /* matches any set of dimensions */
	    match = 1;
	else if(node->nctype == NC_Sequence)
	    match = (seg->slicerank == 1?1:0);
	else /*!NC_Sequence*/
	    match = (seg->slicerank == nclistlength(node->array.dimensions0)?1:0);
	if(!match) return 0;
   }
   return 1;
}

static NCerror
matchpartialname3(NClist* nodes, NClist* segments, CDFnode** nodep)
{
    int i,j,nsegs;
    NCerror ncstat = NC_NOERR;
    NCsegment* lastseg = NULL;
    NClist* namematches = nclistnew();
    NClist* matches = nclistnew();
    NClist* matchpath = nclistnew();

    /* Locate all nodes with the same name
       as the last element in the path
    */
    nsegs = nclistlength(segments);
    lastseg = (NCsegment*)nclistget(segments,nsegs-1);
    for(i=0;i<nclistlength(nodes);i++) {
        CDFnode* node = (CDFnode*)nclistget(nodes,i);
        if(node->nctype != NC_Sequence
               && node->nctype != NC_Structure
               && node->nctype != NC_Grid
               && node->nctype != NC_Primitive
        ) continue;
        if(strcmp(node->name,lastseg->name) != 0) continue;
	nclistpush(namematches,(ncelem)node);
    }    
    if(nclistlength(namematches)==0) {
        oc_log(OCLOGERR,"No match for projection name: %s",lastseg->name);
        ncstat = NC_EDDS;
	goto done;
    }

    /* Now, collect and compare paths of the matching nodes */
    for(i=0;i<nclistlength(namematches);i++) {
        CDFnode* matchnode = (CDFnode*)nclistget(namematches,i);
	nclistclear(matchpath);
	collectnodepath3(matchnode,matchpath,0);
	/* Do a suffix match */
	/* starting at each node in matchpath in the path in turn,
           try to suffix match */
	for(j=0;j<nclistlength(matchpath);j++) {
	    if(nclistlength(matchpath)- j < nsegs)
	        continue; /* cannot match */
	    if(matchsuffix3(matchpath,segments,j)) {
		nclistpush(matches,(ncelem)matchnode);
		break;
	    }
	}
    }
    /* |matches|==0 => no match; |matches|>1 => ambiguity */
    switch (nclistlength(matches)) {
    case 0:
        oc_log(OCLOGERR,"No match for projection name: %s",lastseg->name);
        ncstat = NC_EDDS;
	break;
    case 1:
        if(nodep)
	    *nodep = (CDFnode*)nclistget(matches,0);
	break;
    default: {
	CDFnode* minnode = NULL;
	int minpath = 0;
	int nmin = 0; /* to catch multiple ones with same short path */
	/* ok, see if one of the matches has a path that is shorter
           then all the others */
	for(i=0;i<nclistlength(matches);i++) {
	    CDFnode* candidate = (CDFnode*)nclistget(matches,i);
	    nclistclear(matchpath);
	    collectnodepath3(candidate,matchpath,0);
	    if(minpath == 0) {
		minpath = nclistlength(matchpath);
		minnode = candidate;
	    } else if(nclistlength(matchpath) == minpath) {
	        nmin++;		
	    } else if(nclistlength(matchpath) < minpath) {
		minpath = nclistlength(matchpath);
		minnode = candidate;
		nmin = 1;
	    }
	} /*for*/
	if(minnode == NULL || nmin > 1) {	
	    oc_log(OCLOGERR,"Ambiguous match for projection name: %s",
			lastseg->name);
            ncstat = NC_EDDS;
	} else if(nodep)
	    *nodep = minnode;
	} break;
    }

done:
    return THROW(ncstat);
}

static void
collectsegmentnames3(NClist* segments, NClist* path)
{
    int i;
    ncbytesclear(path);
    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* segment = (NCsegment*)nclistget(segments,i);
	nclistpush(path,(ncelem)segment->name);
    }
}


/*
Compute the projection using heuristics
that take into account the current
fetch limit and the various variable sizes.

Define the heuristic tactics for
retrieving data.

The rules are as follows:
1. if we can read the whole dataset under the limit,
   then do so => tactic_all
2. if we can read the vara variable + all the small
   variables, then do so => tactic_partial
3. if we can read the containing grid
   then do so => tactic_grid
3. (default) read the containing vara variable => tactic_var

Note in particular, that the retrieval may return a result
that is a superset of the vara parameters and so the data
walking routines must be able to handle this situation.  So,
a secondary goal is to produce a projection that can be used
by the data walking routines to obtain the proper subset of
the returned data.

Note also that we need to make sure to match the relevant
dimensions against the relevant nodes in which the ultimate
target is contained. Must take sequence-related dimensions
and string pseudo-dimension into account.

This routine returns (in getvar argument) two results:
1. the set of NCprojection instances that are to be used
   for retrieval (getvar->tactic.fetch).
2. the set of NCprojection instances that are to be used
   for walking the data (getvar->tactic.walk).

*/


/*
The original URL projections
will define the maximum set of
variables that will be retrieved.
However, our tactic may restrict that
set further, so we modify the projection
set to remove projections not
referring to the specified variables.
Additionally, try to merge projections
into larger projections when possible.
We also need to watch out for one projection
enlarging on another (e.g. types.i32 vs types).
The larger one must be removed to avoid
changing the DDS metadata in a way that is
inconsistent with the DDS metadata.
*/

void
restrictprojection34(NClist* varlist, NClist* projections)
{
    int i,j,len;

#ifdef DEBUG
fprintf(stderr,"restriction.before=|%s|\n",
		dumpprojections(projections));
#endif

    if(nclistlength(varlist) == 0) goto done; /* nothing to add or remove */

    /* If the projection list is empty, then add
       a projection for every variable in varlist
    */
    if(nclistlength(projections) == 0) {
        NClist* path = nclistnew();
	NClist* nodeset = NULL;
	/* Attempt to unify the vars into larger units
	   (like a complete grid) */
	nodeset = unifyprojectionnodes3(varlist);	
        for(i=0;i<nclistlength(nodeset);i++) {
	    CDFnode* var = (CDFnode*)nclistget(nodeset,i);
	    NCprojection* newp = createncprojection();
	    newp->discrim = NS_VAR;
	    newp->var = createncvar();
	    newp->var->leaf = var;
	    nclistclear(path);
	    collectnodepath3(var,path,!WITHDATASET);
	    newp->var->segments = nclistnew();
	    for(j=0;j<nclistlength(path);j++) {
	        CDFnode* node = (CDFnode*)nclistget(path,j);
	        NCsegment* newseg = createncsegment();
	        newseg->name = nulldup(node->name);
	        newseg->slicesdefined = 1; /* treat as simple projections */
	        newseg->node = node;
	        makewholesegment3(newseg,node);
	        nclistpush(newp->var->segments,(ncelem)newseg);
	    }
	    nclistpush(projections,(ncelem)newp);
	}
	nclistfree(path);
	nclistfree(nodeset);
    } else {
       /* Otherwise, walk all the projections and see if they
	   intersect any of the variables. If not,
	   then remove from the projection list.
	*/
	len = nclistlength(projections);
	for(i=len-1;i>=0;i--) {/* Walk backward to facilitate removal*/
	    int intersect = 0;
	    NCprojection* proj = (NCprojection*)nclistget(projections,i);
	    if(proj->discrim != NS_VAR) continue;
	    for(j=0;j<nclistlength(varlist);j++) {
		CDFnode* var = (CDFnode*)nclistget(varlist,j);
		/* Note that intersection could go either way */
		if(treecontains3(var,proj->var->leaf)
		   || treecontains3(proj->var->leaf,var)) {intersect = 1; break;}
	    }	    
	    if(!intersect) {
		/* suppress this projection */
		NCprojection* p = (NCprojection*)nclistremove(projections,i);
		freencprojection(p);
	    }
	}
	/* Now looks for containment between projections and only keep
           the more restrictive. Is this algorithm stable against reordering?.
	*/
	for(;;) {
	    int removed = 0;
	    for(i=0;i<nclistlength(projections);i++) {
	        NCprojection* pi = (NCprojection*)nclistget(projections,i);
	        if(pi->discrim != NS_VAR) continue;
	        for(j=0;j<i;j++) {
	            NCprojection* pj = (NCprojection*)nclistget(projections,j);
	            if(pj->discrim != NS_VAR) continue;
		    if(treecontains3(pi->var->leaf,pj->var->leaf)) {
		        NCprojection* p = (NCprojection*)nclistremove(projections,j);
			freencprojection(p);
			removed = 1;
			break;
		    } else if(treecontains3(pj->var->leaf,pi->var->leaf)) {
		        NCprojection* p = (NCprojection*)nclistremove(projections,i);
			freencprojection(p);
			removed = 1;
			break;
		    }
		}
	    }
	    if(!removed) break;
	}
    }
    
done:
#ifdef DEBUG
fprintf(stderr,"restriction.after=|%s|\n",
		dumpprojections(projections));
#endif
    return;
}

/* Return 1 if the specified var is in
the projection's leaf's subtree and is
visible
*/

static int
treecontains3(CDFnode* var, CDFnode* root)
{
    int i;

    if(root->visible == 0) return 0;
    if(var == root) return 1;
    for(i=0;i<nclistlength(root->subnodes);i++) {
        CDFnode* subnode = (CDFnode*)nclistget(root->subnodes,i);
	if(treecontains3(var,subnode)) return 1;
    }
    return 0; 
}

/* See if we can unify sets of nodes to be projected
   into larger units.
*/
static NClist*
unifyprojectionnodes3(NClist* varlist)
{
    int i;
    NClist* nodeset = nclistnew();
    NClist* containerset = nclistnew();
    NClist* containernodes = nclistnew();

    nclistsetalloc(nodeset,nclistlength(varlist));
    nclistsetalloc(containerset,nclistlength(varlist));
    /* Duplicate the varlist so we can modify it;
       simultaneously collect unique container set.
    */
    for(i=0;i<nclistlength(varlist);i++) {
	CDFnode* var = (CDFnode*)nclistget(varlist,i);
	CDFnode* container = var->container;
	nclistpush(nodeset,(ncelem)var);
	switch (container->nctype) {
	case NC_Sequence: case NC_Structure: case NC_Grid: case NC_Dataset:
	    /* add (uniquely) to container set */
	    if(!nclistcontains(containerset,(ncelem)container)) 
	        nclistpush(containerset,(ncelem)container);
	    break;
	default: break;
	}
    }

    /* Now, try to find containers whose subnodes are all in the
	varlist; repeat until no more changes */
    for(;;) {
	int changed = 0;
        for(i=0;i<nclistlength(containerset);i++) {
            int j, allfound;
            CDFnode* container = (CDFnode*)nclistget(containerset,i);
	    if(container == NULL) continue;
            nclistclear(containernodes);
            for(allfound=1,j=0;j<nclistlength(container->subnodes);j++) {
                CDFnode* subnode = (CDFnode*)nclistget(container->subnodes,j);
                if(!nclistcontains(varlist,(ncelem)subnode)) {allfound=0;break;}
                nclistpush(containernodes,(ncelem)subnode);
            }
            if(allfound) {
                nclistpush(nodeset,(ncelem)container);
                nclistset(containerset,i,(ncelem)NULL); /* remove */
                for(j=nclistlength(nodeset)-1;j>=0;j--) { /* walk backwards */
                    CDFnode* testnode = (CDFnode*)nclistget(nodeset,j);
                    if(nclistcontains(containernodes,(ncelem)testnode))
                        nclistremove(nodeset,j);/* remove */
                }
		changed = 1;
            }
        }
	if(!changed) break; /* apparently we have reached a stable situation */
    }
    /* If there is only the dataset left as a projection, then remove it */
    if(nclistlength(nodeset) == 1) {
	CDFnode* thenode = (CDFnode*)nclistget(nodeset,0);
	if(thenode->nctype == NC_Dataset) nclistclear(nodeset);
    }
    nclistfree(containerset);
    nclistfree(containernodes);
    return nodeset;
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

NCerror
slicemerge3(NCslice* dst, NCslice* src)
{
    NCerror err = NC_NOERR;
    NCslice tmp;

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
Assume that name qualification has occured.
*/
NCerror
mergeprojections3(NClist* dst, NClist* src)
{
    int i;
    NClist* cat = nclistnew();
    NCerror ncstat = NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"mergeprojection: dst = %s\n",dumpprojections(dst));
fprintf(stderr,"mergeprojection: src = %s\n",dumpprojections(src));
#endif

    /* get dst concat clone(src) */
    nclistsetalloc(cat,nclistlength(dst)+nclistlength(src));
    for(i=0;i<nclistlength(dst);i++) {
	NCprojection* p = (NCprojection*)nclistget(dst,i);
	nclistpush(cat,(ncelem)p);
    }    
    for(i=0;i<nclistlength(src);i++) {
	NCprojection* p = (NCprojection*)nclistget(src,i);
	nclistpush(cat,(ncelem)clonencprojection(p));
    }    

    nclistclear(dst);

    /* Repeatedly pull elements from the concat,
       merge with all duplicates, and stick into
       the dst
    */
    while(nclistlength(cat) > 0) {
	NCprojection* target = (NCprojection*)nclistremove(cat,0);
	if(target == NULL) continue;
        if(target->discrim != NS_VAR) continue;
        for(i=0;i<nclistlength(cat);i++) {
	    NCprojection* p2 = (NCprojection*)nclistget(cat,i);
	    if(p2 == NULL) continue;
	    if(p2->discrim != NS_VAR) continue;
	    if(target->var->leaf != p2->var->leaf) continue;
	    /* This entry matches our current target; merge  */
	    ncstat = mergeprojection31(target,p2);
	    /* null out this merged entry and release it */
	    nclistset(cat,i,(ncelem)NULL);	    
	    freencprojection(p2);	    
	}		    
	/* Capture the clone */
	nclistpush(dst,(ncelem)target);
    }	    
    nclistfree(cat);
    return ncstat;
}

static NCerror
mergeprojection31(NCprojection* dst, NCprojection* src)
{
    NCerror ncstat = NC_NOERR;
    int i,j;

    /* merge segment by segment;
       |dst->segments| == |src->segments|
       by construction
    */
    ASSERT((dst->discrim == NS_VAR && src->discrim == NS_VAR));
    ASSERT((nclistlength(dst->var->segments) == nclistlength(src->var->segments)));    
    for(i=0;i<nclistlength(dst->var->segments);i++) {
	NCsegment* dstseg = (NCsegment*)nclistget(dst->var->segments,i);
	NCsegment* srcseg = (NCsegment*)nclistget(src->var->segments,i);
	ASSERT((dstseg->node == srcseg->node)); /* by construction */
	for(j=0;j<dstseg->slicerank;j++) {
	    slicemerge3(&dstseg->slices[j],&srcseg->slices[j]);
	}
    }
    return ncstat;
}

/* Convert an NCprojection instance into a string
   that can be used with the url
*/

char*
buildprojectionstring3(NClist* projections)
{
    char* pstring;
    NCbytes* buf = ncbytesnew();
    tostringncprojections(projections,buf);
    pstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return pstring;
}

char*
buildselectionstring3(NClist* selections)
{
    NCbytes* buf = ncbytesnew();
    char* sstring;
    tostringncselections(selections,buf);
    sstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return sstring;
}

char*
buildconstraintstring3(NCconstraint* constraints)
{
    NCbytes* buf = ncbytesnew();
    char* result = NULL;
    tostringncconstraint(constraints,buf);
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

/* Remove all CDFnode* references from constraint */
void
dereference3(NCconstraint* constraint)
{
    int i;
    NClist* allnodes = ceallnodes((NCany*)constraint,NS_NIL);
    for(i=0;i<nclistlength(allnodes);i++) {
	NCany* node = (NCany*)nclistget(allnodes,i);
	switch(node->sort) {
	case NS_SEGMENT:
	    ((NCsegment*)node)->node = NULL;
	    break;
	case NS_VAR:
	    ((NCvar*)node)->node = NULL;
	    ((NCvar*)node)->leaf = NULL;
	    break;
	default: break; /* ignore */
	}
    }
    nclistfree(allnodes);
}

static NCerror
fillsegmentpath(NCprojection* p, NClist* nodes)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NClist* path = nclistnew();

    ASSERT(p->discrim == NS_VAR);
    collectsegmentnames3(p->var->segments,path);
    ncstat = matchpartialname3(nodes,path,&p->var->leaf);
    if(ncstat) goto done;
    /* Now complete the segment path */
    nclistclear(path);
    collectnodepath3(p->var->leaf,path,!WITHDATASET);
    if(nclistlength(path) != nclistlength(p->var->segments)) {
	ncstat = NC_EINVAL;
	goto done;
    }
    for(i=0;i<nclistlength(p->var->segments);i++) {
        NCsegment* seg = (NCsegment*)nclistget(p->var->segments,i);
	CDFnode* node = (CDFnode*)nclistget(path,i);
	seg->node = node;
#ifdef DEBUG
fprintf(stderr,"reref: %s -> %s\n",seg->name,node->name);
#endif
    }
    
done:
    nclistfree(path);
    return ncstat;
}

static NCerror
fillselectionpath(NCselection* s, NClist* nodes)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NClist* path = nclistnew();
    NCvar* var;

    ASSERT(s->lhs->discrim == NS_VAR);
    var = s->lhs->var;
    ncstat = matchpartialname3(nodes,var->segments,&var->leaf);
    if(ncstat) goto done;
    /* Now find the value paths */
    for(i=0;i<nclistlength(s->rhs);i++) {
        NCvalue* v = (NCvalue*)nclistget(s->rhs,i);
	if(v->discrim != NS_VAR) continue;
        ncstat = matchpartialname3(nodes,v->var->segments,&v->var->node);
        if(ncstat) goto done;
    }
    
done:
    nclistfree(path);
    return ncstat;
}

/* Inverse of dereference:
   add back all CDFnode* references to
   constraint based on a new set of varnodes,
   which by construction should be a 1-1
   with the constraint vars.
*/
NCerror
rereference3(NCconstraint* constraint, NClist* nodeset)
{
    int i;
    NCerror ncstat = NC_NOERR;
    for(i=0;i<nclistlength(constraint->projections);i++) {
        NCprojection* p = (NCprojection*)nclistget(constraint->projections,i);
        ncstat = fillsegmentpath(p, nodeset);
	if(!ncstat) goto done;
    }
    for(i=0;i<nclistlength(constraint->selections);i++) {
        NCselection* s = (NCselection*)nclistget(constraint->selections,i);
	ncstat = fillselectionpath(s, nodeset);
	if(!ncstat) goto done;
    }
done:
    return ncstat;
}

NCerror
buildvaraprojection3(Getvara* getvar,
		     const size_t* startp, const size_t* countp, const ptrdiff_t* stridep,
		     NCprojection** projectionp)
{
    int i,j;
    int ncrank;
    NCerror ncstat = NC_NOERR;
    CDFnode* var = getvar->target;
    NClist* vardims = var->array.dimensions;
    NCprojection* projection = NULL;
    NClist* path = nclistnew();
    NClist* segments = NULL;
    int dimindex;
    NClist* dimset = NULL;

    /* Collect the nodes needed to construct the projection segment */    
    collectnodepath3(var,path,!WITHDATASET);

    dimset = var->array.dimensions;
    ncrank = nclistlength(dimset);

    segments = nclistnew();
    nclistsetalloc(segments,nclistlength(path));
    for(i=0;i<nclistlength(path);i++) {
	NCsegment* segment = createncsegment();
	CDFnode* n = (CDFnode*)nclistget(path,i);
	segment->node = n;
	ASSERT((segment->node != NULL));
        segment->name = nulldup(segment->node->name);
	segment->slicesdefined = 0; /* temporary */
	nclistpush(segments,(ncelem)segment);
    }
    
    projection = createncprojection();
    projection->discrim = NS_VAR;
    projection->var = createncvar();
    projection->var->leaf = var;
    projection->var->segments = segments;

    /* We need to assign slices to each segment */
    dimindex = 0; /* point to next subset of slices */
    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* segment = (NCsegment*)nclistget(segments,i);
	int localrank = nclistlength(segment->node->array.dimensions0);
        if(segment->node->array.stringdim != NULL) localrank++;
        if(segment->node->array.seqdim != NULL) localrank++;
	segment->slicerank = localrank;
        for(j=0;j<localrank;j++) {
	    NCslice* slice = &segment->slices[j];
	    CDFnode* dim = (CDFnode*)nclistget(vardims,dimindex+j);
	    slice->first = startp[dimindex+j];
	    slice->stride = stridep[dimindex+j];
	    slice->count = countp[dimindex+j];
	    slice->length = slice->count * slice->stride;
	    if(slice->length > dim->dim.declsize)
		slice->length = dim->dim.declsize;
	    slice->stop = (slice->first + slice->length);
	    if(slice->stop > dim->dim.declsize)
		slice->stop = dim->dim.declsize;
	    slice->declsize = dim->dim.declsize;
	}
	segment->slicesdefined = 1;
	dimindex += localrank;
	ASSERT((dimindex <= ncrank));
    }

    ASSERT((dimindex == ncrank));

    if(projectionp) *projectionp = projection;

    nclistfree(path);
    if(ncstat) freencprojection(projection);
    return ncstat;
}

int
iswholeslice(NCslice* slice, CDFnode* dim)
{
    if(slice->first != 0 || slice->stride != 1) return 0;
    if(dim != NULL) {
	if(slice->stop != dim->dim.declsize) return 0;
    } else if(dim == NULL) {
	if(slice->declsize == 0
	   || slice->count != slice->declsize) return 0;
    }
    return 1;
}

int
iswholesegment(NCsegment* seg)
{
    int i,whole;
    NCslice* slice = seg->slices;
    NClist* dimset = NULL;
    unsigned int rank;
    
    if(!seg->slicesdefined) return 1;
    if(seg->node == NULL) return 0;
    dimset = seg->node->array.dimensions;
    rank = nclistlength(dimset);
    whole = 1; /* assume so */
    for(i=0;i<rank;i++,slice++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	if(!iswholeslice(slice,dim)) {whole = 0; break;}	
    }
    return whole;
}

int
iswholeprojection(NCprojection* proj)
{
    int i,whole;
    
    ASSERT((proj->discrim == NS_VAR));

    whole = 1; /* assume so */
    for(i=0;i<nclistlength(proj->var->segments);i++) {
        NCsegment* segment = (NCsegment*)nclistget(proj->var->segments,i);
	if(!iswholesegment(segment)) {whole = 0; break;}	
    }
    return whole;
}

void
makewholeslice3(NCslice* slice, CDFnode* dim)
{
    slice->first = 0;
    slice->stride = 1;
    slice->length = dim->dim.declsize;
    slice->stop = dim->dim.declsize;
    slice->declsize = dim->dim.declsize;
    slice->count = dim->dim.declsize;
}

void
makewholesegment3(NCsegment* seg, CDFnode* node)
{
    int i;
    NClist* dimset = NULL;
    unsigned int rank;

    dimset = node->array.dimensions;
    rank = nclistlength(dimset);
    for(i=0;i<rank;i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	makewholeslice3(&seg->slices[i],dim);
    }
    seg->slicerank = rank;
    seg->slicesdefined = 1;
}

NCsegment*
createncsegment(void)
{
    NCsegment* mem = NULL;
    mem = (NCsegment*)emalloc(sizeof(NCsegment));
    memset((void*)mem,0,sizeof(NCsegment));
    mem->sort = NS_SEGMENT;
    return mem;
}

NCslice*
createncslice(void)
{
    NCslice* mem = (NCslice*)emalloc(sizeof(NCslice));
    memset((void*)mem,0,sizeof(NCslice));
    mem->sort = NS_SLICE;
    return mem;
}


NCconstant*
createncconstant(void)
{
    NCconstant* mem = (NCconstant*)emalloc(sizeof(NCconstant));
    memset((void*)mem,0,sizeof(NCconstant));
    mem->sort = NS_CONST;
    mem->discrim = NS_NIL;
    return mem;
}

NCvalue*
createncvalue(void)
{
    NCvalue* mem = (NCvalue*)emalloc(sizeof(NCvalue));
    memset((void*)mem,0,sizeof(NCvalue));
    mem->sort = NS_VALUE;
    mem->discrim = NS_NIL;
    return mem;
}

NCfcn*
createncfcn(void)
{
    NCfcn* mem = (NCfcn*)emalloc(sizeof(NCfcn));
    memset((void*)mem,0,sizeof(NCfcn));
    mem->sort = NS_FCN;
    return mem;
}

NCvar*
createncvar(void)
{
    NCvar* mem = (NCvar*)emalloc(sizeof(NCvar));
    memset((void*)mem,0,sizeof(NCvar));
    mem->sort = NS_VAR;
    return mem;
}

NCprojection*
createncprojection(void)
{
    NCprojection* mem = (NCprojection*)emalloc(sizeof(NCprojection));
    memset((void*)mem,0,sizeof(NCprojection));
    mem->sort = NS_PROJECT;
    return mem;
}

NCselection*
createncselection(void)
{
    NCselection* sel = (NCselection*)emalloc(sizeof(NCselection));
    memset((void*)sel,0,sizeof(NCselection));
    sel->sort = NS_SELECT;
    sel->lhs = NULL;
    sel->rhs = NULL;
    sel->operator = NS_NIL;
    return sel;
}

NCconstraint*
createncconstraint(void)
{
    NCconstraint* con = (NCconstraint*)emalloc(sizeof(NCconstraint));
    memset((void*)con,0,sizeof(NCconstraint));
    con->sort = NS_CONSTRAINT;
    return con;
}

NCslice
clonencslice(NCslice slice)
{
    return slice;
}

NClist*
clonencsegments(NClist* segments)
{
    int i,j;
    NClist* clones = NULL;
    if(segments == NULL) return NULL;
    clones = nclistnew();
    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* seg = (NCsegment*)nclistget(segments,i);
	NCsegment* newseg = createncsegment();
	*newseg = *seg;
	newseg->name = nulldup(newseg->name);
        for(j=0;j<seg->slicerank;j++) newseg->slices[j] = seg->slices[j];
	nclistpush(clones,(ncelem)newseg);
    }
    return clones;
}




NCvar*
clonencvar(NCvar* var)
{
    NCvar* newvar;
    if(var == NULL) return NULL;
    newvar = createncvar();
    *newvar = *var;
    newvar->segments = clonencsegments(var->segments);
    return newvar;
}

NCfcn*
clonencfcn(NCfcn* fcn)
{
    int i;
    NCfcn* newfcn;
    if(fcn == NULL) return NULL;
    newfcn = createncfcn();
    newfcn->sort = NS_FCN;
    newfcn->name = nulldup(fcn->name);
    newfcn->args = nclistnew();
    for(i=0;i<nclistlength(fcn->args);i++) {
	NCvalue* arg = (NCvalue*)nclistget(fcn->args,i);
	NCvalue* newarg = clonencvalue(arg);
	nclistpush(newfcn->args,(ncelem)newarg);
    }
    return newfcn;
}

NCconstant*
clonencconstant(NCconstant* con)
{
    NCconstant* newcon;
    if(con == NULL) return NULL;
    newcon = createncconstant();
    *newcon = *con;
    if(newcon->discrim ==  NS_STR)
	 newcon->text = nulldup(con->text);
    return newcon;
}

NCvalue*
clonencvalue(NCvalue* val)
{
    NCvalue* newval;
    if(val == NULL) return NULL;
    newval = createncvalue();
    *newval = *val;
    switch (newval->discrim) {
    case NS_CONST: newval->constant = clonencconstant(val->constant); break;
    case NS_VAR: newval->var = clonencvar(val->var); break;
    case NS_FCN: newval->fcn = clonencfcn(val->fcn); break;
    default: abort();
    }
    return newval;
}


NCprojection*
clonencprojection(NCprojection* p)
{
    NCprojection* newp;
    if(p == NULL) return NULL;
    newp = createncprojection();
    *newp = *p;
    if(p->discrim == NS_VAR)
        newp->var = clonencvar(p->var);
    else
        newp->fcn = clonencfcn(p->fcn);
    return newp;
}

NClist*
clonencprojections(NClist* projectionlist)
{
    int i;
    NClist* clone;
    if(projectionlist == NULL) return NULL;
    clone = nclistnew();
    for(i=0;i<nclistlength(projectionlist);i++) {
	NCprojection* p = (NCprojection*)nclistget(projectionlist,i);
	NCprojection* newp = clonencprojection(p);
	nclistpush(clone,(ncelem)newp);
    }
    return clone;
}

NCselection*
clonencselection(NCselection* s)
{
    int i;
    NCselection* news;
    if(s==NULL) return NULL;
    news = createncselection();
    *news = *s;    
    news->lhs = clonencvalue(s->lhs);
    news->rhs = nclistnew();
    for(i=0;i<nclistlength(s->rhs);i++) {	
	NCvalue* v = (NCvalue*)nclistget(s->rhs,i);
	NCvalue* newv = clonencvalue(v);
	nclistpush(news->rhs,(ncelem)newv);
    }
    return news;
}

NClist*
clonencselections(NClist* selectionlist)
{
    int i;
    NClist* clone;
    if(selectionlist == NULL) return NULL;
    clone = nclistnew();
    for(i=0;i<nclistlength(selectionlist);i++) {
	NCselection* s = (NCselection*)nclistget(selectionlist,i);
	nclistpush(clone,(ncelem)clonencselection(s));
    }
    return clone;
}

NCconstraint*
clonencconstraint(NCconstraint* con)
{
    NCconstraint* clone;
    if(con == NULL) return NULL;
    clone = createncconstraint();
    clone->projections = clonencprojections(con->projections);
    clone->selections = clonencselections(con->selections);
    return clone;
}

void
freencslice(NCslice* s)
{
    if(s != NULL) efree(s);
}

void
freencsegment(NCsegment* seg)
{
    if(seg != NULL) {
        efree(seg->name);
        efree(seg);
    }
}

void
freencsegments(NClist* segments)
{
    int i;
    if(segments == NULL) return;
    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* seg = (NCsegment*)nclistget(segments,i);
	efree((void*)seg->name);
        efree((void*)seg);
    }
    nclistfree(segments);
}

void
freencvar(NCvar* var)
{
    if(var == NULL) return;
    freencsegments(var->segments);
    efree(var);
}

void
freencfcn(NCfcn* fcn)
{
    int i;
    if(fcn == NULL) return;
    efree(fcn->name);
    if(fcn->args != NULL) for(i=0;i<nclistlength(fcn->args);i++) {
	NCvalue* val = (NCvalue*)nclistget(fcn->args,i);
	freencvalue(val);
    }
    efree(fcn);
}

void
freencconstant(NCconstant* con)
{
    if(con == NULL) return;
    if(con->discrim ==  NS_STR)
	efree(con->text);
    efree(con);
}

void
freencvalue(NCvalue* val)
{
    if(val == NULL) return;
    switch(val->discrim) {
    case NS_CONST: freencconstant(val->constant); break;    
    case NS_VAR: freencvar(val->var); break;    
    case NS_FCN: freencfcn(val->fcn); break;    
    default: abort();
    }
    efree(val);
}

void
freencprojection(NCprojection* p)
{
    if(p == NULL) return;
    if(p == NULL) return;
    if(p->discrim == NS_VAR)
        freencvar(p->var);
    else
        freencfcn(p->fcn);
    efree(p);
}

void
freencprojections(NClist* plist)
{
    int i;
    if(plist == NULL) return;
    for(i=0;i<nclistlength(plist);i++) {
        NCprojection* p = (NCprojection*)nclistget(plist,i);
	freencprojection(p);
    }
    nclistfree(plist);
}

void
freencselection(NCselection* s)
{
    int i;
    if(s == NULL) return;
    freencvalue(s->lhs);
    for(i=0;i<nclistlength(s->rhs);i++) {
	NCvalue* v = (NCvalue*)nclistget(s->rhs,i);
	freencvalue(v);
    }
    nclistfree(s->rhs);
    efree(s);
}

void
freencselections(NClist* slist)
{
    int i;
    if(slist == NULL) return;
    for(i=0;i<nclistlength(slist);i++) {
        NCselection* s = (NCselection*)nclistget(slist,i);
	freencselection(s);
    }
    nclistfree(slist);
}

void
freencconstraint(NCconstraint* con)
{
    if(con == NULL) return;
    if(con->projections) freencprojections(con->projections);
    if(con->selections) freencselections(con->selections);
    efree((void*)con);
}

void
tostringncslices(NCslice* slice, int rank, NCbytes* buf)
{
    int i;
    if(slice == NULL) {ncbytesappend(buf,'?'); return;}
    for(i=0;i<rank;i++,slice++) {
        char tmp[1024];
	unsigned long last = (slice->first+slice->length)-1;
	ASSERT(slice->declsize > 0);
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
    }
}

void
tostringncsegments(NClist* segments, NCbytes* buf)
{
    int i;
    char* separator = ".";
    if(segments == NULL) {tostringncunknown(buf); return;}
    for(i=0;i<nclistlength(segments);i++) {
        NCsegment* segment = (NCsegment*)nclistget(segments,i);
	if(i > 0) ncbytescat(buf,separator);
	ncbytescat(buf,(segment->name?segment->name:"<unknown>"));
	if(segment->node != NULL && segment->node->nctype == NC_Sequence) continue;
	if(!iswholesegment(segment)) {
	    int rank = segment->slicerank;
	    tostringncslices(segment->slices,rank,buf);
	}
    }
}

void
tostringncvar(NCvar* var, NCbytes* buf)
{
    if(var == NULL) {tostringncunknown(buf); return;}
    tostringncsegments(var->segments,buf);
}

void
tostringncfcn(NCfcn* fcn, NCbytes* buf)
{
    int i;
    if(fcn == NULL) {tostringncunknown(buf); return;}
    ncbytescat(buf,fcn->name);
    ncbytescat(buf,"(");
    for(i=0;i<nclistlength(fcn->args);i++) {
	if(i > 0) ncbytescat(buf,",");
	tostringncvalue((NCvalue*)nclistget(fcn->args,i),buf);
    }
    ncbytescat(buf,")");
}

void
tostringncconstant(NCconstant* value, NCbytes* buf)
{
    char tmp[64];
    if(value == NULL) {tostringncunknown(buf); return;}
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
    default: abort(); break;
    }
}

void
tostringncvalue(NCvalue* value, NCbytes* buf)
{
    if(value == NULL) {tostringncunknown(buf); return;}
    switch (value->discrim) {
    case NS_CONST:
	tostringncconstant(value->constant,buf);
	break;		
    case NS_VAR:
	tostringncvar(value->var,buf);
	break;
    case NS_FCN:
	tostringncfcn(value->fcn,buf);
	break;
    default: PANIC1("unexpected discriminator: %d",(int)value->discrim);
    }
}

void
tostringncprojection(NCprojection* p, NCbytes* buf)
{
    if(p == NULL) {tostringncunknown(buf); return;}
    if(p->discrim == NS_VAR)
        tostringncsegments(p->var->segments,buf);
    else
        tostringncfcn(p->fcn,buf);
}

void
tostringncprojections(NClist* projections, NCbytes* buf)
{
    int i;
    if(projections == NULL) {tostringncunknown(buf); return;}
    for(i=0;i<nclistlength(projections);i++) {
	NCprojection* p = (NCprojection*)nclistget(projections,i);
        if(i > 0) ncbytescat(buf,",");
	tostringncprojection(p,buf);
    }
}

static char* opstrings[] = OPSTRINGS ;

void
tostringncselection(NCselection* sel, NCbytes* buf)
{
    int i;
    if(sel == NULL) {tostringncunknown(buf); return;}
    tostringncvalue(sel->lhs,buf);
    if(sel->operator == NS_NIL) return;
    ncbytescat(buf,opstrings[(int)sel->operator]);
    if(nclistlength(sel->rhs) > 1)
        ncbytescat(buf,"{");
    for(i=0;i<nclistlength(sel->rhs);i++) {
	NCvalue* value = (NCvalue*)nclistget(sel->rhs,i);
	if(i > 0) ncbytescat(buf,",");
	tostringncvalue(value,buf);
    }
    if(nclistlength(sel->rhs) > 1)
	ncbytescat(buf,"}");
}


void
tostringncselections(NClist* selections, NCbytes* buf)
{
    int i;
    if(selections == NULL) {tostringncunknown(buf); return;}
    for(i=0;i<nclistlength(selections);i++) {
	NCselection* sel = (NCselection*)nclistget(selections,i);
	ncbytescat(buf,"&");
	tostringncselection(sel,buf);
    }
}

void
tostringncconstraint(NCconstraint* con, NCbytes* buf)
{
    if(con == NULL) {tostringncunknown(buf); return;}
    if(con->projections != NULL)
        tostringncprojections(con->projections,buf);
    if(con->selections != NULL)
        tostringncselections(con->selections,buf);
}

static void
tostringncunknown(NCbytes* buf)
{
    ncbytescat(buf,"<unknown>");
}


/* Collect all nodes within a specified constraint tree */
/* Caller frees result */
NClist*
ceallnodes(NCany* node, NCsort which)
{
    NClist* allnodes = nclistnew();
    ceallnodesr(node,allnodes,which);
    return allnodes;
}

static void
ceallnodesr(NCany* node, NClist* allnodes, NCsort which)
{
    int i;
    if(node == NULL) return;
    if(nclistcontains(allnodes,(ncelem)node)) return;
    if(which == NS_NIL || node->sort == which)
        nclistpush(allnodes,(ncelem)node);
    switch(node->sort) {
    case NS_FCN: {
	NCfcn* fcn = (NCfcn*)node;
	for(i=0;i<nclistlength(fcn->args);i++) {
	    ceallnodesr((NCany*)nclistget(fcn->args,i),allnodes,which);
	}
    } break;
    case NS_VAR: {
	NCvar* var = (NCvar*)node;
	for(i=0;i<nclistlength(var->segments);i++) {
	    ceallnodesr((NCany*)nclistget(var->segments,i),allnodes,which);
	}
    } break;
    case NS_VALUE: {
	NCvalue* value = (NCvalue*)node;
	if(value->discrim == NS_VAR)
	    ceallnodesr((NCany*)value->var,allnodes,which);
	else if(value->discrim == NS_FCN)
	    ceallnodesr((NCany*)value->fcn,allnodes,which);
	else
	    ceallnodesr((NCany*)value->constant,allnodes,which);
    } break;
    case NS_SELECT: {
	NCselection* selection = (NCselection*)node;
        ceallnodesr((NCany*)selection->lhs,allnodes,which);
	for(i=0;i<nclistlength(selection->rhs);i++)
            ceallnodesr((NCany*)nclistget(selection->rhs,i),allnodes,which);
    } break;
    case NS_PROJECT: {
	NCprojection* projection = (NCprojection*)node;
	if(projection->discrim == NS_VAR)
	    ceallnodesr((NCany*)projection->var,allnodes,which);
	else
	    ceallnodesr((NCany*)projection->fcn,allnodes,which);
    } break;
    case NS_CONSTRAINT: {
	NCconstraint* constraint = (NCconstraint*)node;
	for(i=0;i<nclistlength(constraint->projections);i++)
	    ceallnodesr((NCany*)nclistget(constraint->projections,i),allnodes,which);
	for(i=0;i<nclistlength(constraint->selections);i++)
	    ceallnodesr((NCany*)nclistget(constraint->selections,i),allnodes,which);
    } break;

    /* All others have no subnodes */
    default:
	break;
    }
}
