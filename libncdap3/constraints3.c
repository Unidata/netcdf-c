/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.c,v 1.40 2010/05/27 21:34:07 dmh Exp $
 *********************************************************************/
#include "ncdap3.h"
#include "dapodom.h"
#include "dapdump.h"

static NCerror mergeprojection31(NCDAPCOMMON*, NCprojection*, NCprojection*);

static NCerror matchpartialname3(NClist*, NClist*, CDFnode**);
static void collectsegmentnames3(NClist* segments, NClist* path);
static void makeprojectionstring13(NCprojection*, NCbytes*, char*);
static void makeslicestring3(NCslice*, unsigned int, NCbytes*);
static NClist* clonesegments3(NClist* segments);
static void completesegments3(NClist* fullpath, NClist* segments);

static NCerror convertdapconstraint3(NCDAPCOMMON*, CDFnode*, char*, char*, NCconstraint*);
static NCerror qualifyconstraints3(NCDAPCOMMON*, NCconstraint*);
static NCerror qualifyprojectionnames3(NCDAPCOMMON*, NCprojection*);
static NCerror qualifyprojectionsizes3(NCDAPCOMMON*, NCprojection*);
static NCerror qualifyselectionnames3(NCDAPCOMMON*, NCselection*);
static void freesegments(NClist* path);

/*
Merge the dap constraints with the nc constraints
Processing the constraints is a multi-step action.
       1. retrieve the dds
       2. convert the nc constraint names to be dap constraint
          names
       3. make all dap constraint names be fully qualified
       4. merge the dap and nc constraints
*/
NCerror
buildconstraints3(NCDAPCOMMON* nccomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    NCconstraint dapconstraint = {NULL,NULL};
    NCconstraint nullconstraint = {NULL,NULL};
    char* dapprojectionstring = NULL;
    char* dapselectionstring  = NULL;
    CDFnode* ddsroot = nccomm->cdf.ddsroot;
    NClist* pmerge = NULL;
    NClist* smerge = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
	/* ignore all constraints */
        nccomm->oc.dapconstraint = nullconstraint;
	goto done;
    }

    /* Modify the dap projections */
    dapprojectionstring = nccomm->oc.url.projection;
    dapselectionstring = nccomm->oc.url.selection;
    if(dapprojectionstring != NULL || dapselectionstring != NULL) {
        ncstat = convertdapconstraint3(nccomm,
				 ddsroot,
				 dapprojectionstring,
				 dapselectionstring,
				 &dapconstraint);
        if(ncstat != NC_NOERR) goto done;
        ncstat = qualifyconstraints3(nccomm, &dapconstraint);
        if(ncstat != NC_NOERR) goto done;
    } else {
	dapconstraint.projections = nclistnew();
	dapconstraint.selections = nclistnew();
    }

#ifdef DEBUG
fprintf(stderr,"buildconstraint: url:%s\n",nccomm->oc.url.url);
fprintf(stderr,"buildconstraint.projections: %s\n",
		dumpprojections(dapconstraint.projections));
fprintf(stderr,"buildconstraint.selections: %s\n",
		dumpselections(dapconstraint.selections));
#endif

    /* Save the constraints */
    nccomm->oc.dapconstraint = dapconstraint;
    dapconstraint = nullconstraint;    

    /* Now, merge the (dap+netcdf) projections and
       (dap+netcdf) selections
    */
    pmerge = cloneprojections(nccomm->oc.dapconstraint.projections);

    /* combine selections */
    if(nccomm->oc.dapconstraint.selections == NULL) {
	smerge = nclistnew();
    } else {
	smerge = cloneselections(nccomm->oc.dapconstraint.selections);
    }

    clearncconstraint(&nccomm->oc.constraint);
    nccomm->oc.constraint.projections = pmerge;
    nccomm->oc.constraint.selections = smerge;

#ifdef DEBUG
fprintf(stderr,"buildconstraints3: merge = %s\n",
	dumpconstraint(&nccomm->oc.constraint));
#endif

done:
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    if(ncstat) {
	clearncconstraint(&dapconstraint);
    }
    return THROW(ncstat);
}

/* Convert a dap projection path to a CDFnode path; the difficulty
   is that suffix paths are legal.
*/

static NCerror
convertdapconstraint3(NCDAPCOMMON* nccomm, CDFnode* root,
		      char* dapprojectionstring, char* dapselectionstring,
		      NCconstraint* dapconstraintp)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NClist* nodes = root->tree->nodes;
    NClist* dapprojections = NULL;
    NClist* dapselections = NULL;
    int errcode;
    char* errmsg;

    errcode = ncceparse(dapprojectionstring,0,&dapprojections,NULL,&errmsg);
    if(errcode) {
	oc_log(OCLOGWARN,"DAP projection parse failure: %s",errmsg);
	efree(errmsg);
	freencprojections(dapprojections);
	dapprojections = nclistnew();
    }
#ifdef DEBUG
fprintf(stderr,"projections: %s",dumpprojections(dapprojections));
#endif

#ifdef IGNORE
    errcode = ncceparse(dapselectionstring,0,NULL,&dapselections,&errmsg);
    if(errcode) {
	oc_log(OCLOGWARN,"DAP selection parse failure: %s",errmsg);
	efree(errmsg);
	freencselections(dapselections);
	dapselections = nclistnew();
    }
#ifdef DEBUG
fprintf(stderr,"selections: %s",dumpselections(dapselections));
#endif
#endif

    /* Convert the projection paths to leaves in the dds tree */
    for(i=0;i<nclistlength(dapprojections);i++) {
	NCprojection* proj = (NCprojection*)nclistget(dapprojections,i);
	ncstat = matchpartialname3(nodes,proj->segments,&proj->leaf);
	if(ncstat) goto done;
    }

    /* Convert the selection paths to leaves in the dds tree */
    for(i=0;i<nclistlength(dapselections);i++) {
	NCselection* sel = (NCselection*)nclistget(dapselections,i);
	ncstat = matchpartialname3(nodes,sel->segments,&sel->leaf);
	if(ncstat) goto done;
    }
   
    /* Convert the selection path values to leaves in the dds tree */
    for(i=0;i<nclistlength(dapselections);i++) {
	int j;
	NCselection* sel = (NCselection*)nclistget(dapselections,i);
	for(j=0;j<nclistlength(sel->values);j++) {
	    NCvalue* value = (NCvalue*)nclistget(sel->values,j);
	    if(value->kind == ST_VAR) {
	        ncstat = matchpartialname3(nodes,value->value.var.segments,&value->value.var.node);
		if(ncstat) goto done;
	    }
	}
    }

    if(dapconstraintp) {
	dapconstraintp->projections = dapprojections;
	dapconstraintp->selections = dapselections;
	dapprojections = NULL;
	dapselections = NULL;
    }

done:
    if(ncstat) {
	freencprojections(dapprojections);
	freencselections(dapselections);
    }
    return THROW(ncstat);
}

/* Fill in:
    1. projection segments
    2. projection segment slices declsize
    3. selection path
*/
static NCerror
qualifyconstraints3(NCDAPCOMMON* nccomm, NCconstraint* constraint)
{
    NCerror ncstat = NC_NOERR;
    int i;
    if(constraint != NULL) {
        for(i=0;i<nclistlength(constraint->projections);i++) {  
            NCprojection* p = (NCprojection*)nclistget(constraint->projections,i);
            ncstat = qualifyprojectionnames3(nccomm,p);
            ncstat = qualifyprojectionsizes3(nccomm,p);
        }
        for(i=0;i<nclistlength(constraint->selections);i++) {   
            NCselection* s = (NCselection*)nclistget(constraint->selections,i);
            ncstat = qualifyselectionnames3(nccomm,s);
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
qualifyprojectionnames3(NCDAPCOMMON* nccomm, NCprojection* proj)
{
    NCerror ncstat = NC_NOERR;
    NClist* fullpath = nclistnew();

    ASSERT((proj->leaf != NULL && proj->leaf->dds != OCNULL));
    collectnodepath3(proj->leaf,fullpath,!WITHDATASET);
#ifdef DEBUG
fprintf(stderr,"qualify: %s -> ",
	dumpprojection1(proj));
#endif
    /* Now add path nodes to create full path */
    completesegments3(fullpath,proj->segments);

#ifdef DEBUG
fprintf(stderr,"%s\n",
	dumpprojection1(proj));
#endif
    nclistfree(fullpath);
    return ncstat;
}

/* Make sure that the slice declsizes are all defined for this projection */
static NCerror
qualifyprojectionsizes3(NCDAPCOMMON* nccomm, NCprojection* proj)
{
    int i,j;
    for(i=0;i<nclistlength(proj->segments);i++) {
        NCsegment* seg = (NCsegment*)nclistget(proj->segments,i);
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

   
#ifdef UNUSED
/* Modify projection segments to make their rank
   match the node's true rank (! |ncdimension|)
*/
static NCerror
completeslicing3(NCDAPCOMMON* nccomm, NClist* projections)
{
    int i;
    NCerror ncstat = NC_NOERR;
    for(i=0;i<nclistlength(projections);i++) {
	NCprojection* p = (NCprojection*)nclistget(projections,i);
	completeslicing31(nccomm,p);
    }
    return ncstat;    
}

static NCerror
completeslicing31(NCDAPCOMMON* nccomm, NCprojection* proj)
{
    int i,j;
    NCerror ncstat = NC_NOERR;
    NClist* segments = proj->segments;

    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* seg = (NCsegment*)nclistget(segments,i);
	CDFnode* node = seg->node;
        int rank = nclistlength(node->array.dimensions);
        for(j=seg->slicerank;j<rank;j++) {
            CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
            makewholeslice3(&seg->slices[j],dim);
	    seg->wholesegment = 1;
        }
	seg->slicerank = rank;
    }
#ifdef DEBUG
fprintf(stderr,"completion: %s\n",dumpprojection1(proj));
#endif
    return ncstat;
}

static NCerror
makesimpleprojection(NCDAPCOMMON* nccomm, NCprojection* proj)
{
    int i,j;
    NCerror ncstat = NC_NOERR;
    NClist* segments = proj->segments;

    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* seg = (NCsegment*)nclistget(segments,i);
	CDFnode* node = seg->node;
        int rank = nclistlength(node->array.dimensions);
        for(j=seg->slicerank;j<rank;j++) {
            CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
            makewholeslice3(&seg->slices[j],dim);
	    seg->wholesegment = 1;
        }
	seg->slicerank = rank;
    }
#ifdef DEBUG
fprintf(stderr,"simpleprojection: %s\n",dumpprojection1(proj));
#endif
    return ncstat;
}
#endif

/* convert all names in selections to be fully qualified */
static NCerror
qualifyselectionnames3(NCDAPCOMMON* nccomm, NCselection* sel)
{
    NCerror ncstat = NC_NOERR;
    int i;
    NClist* segments = NULL;
    NClist* fullpath = nclistnew();

    collectnodepath3(sel->leaf,fullpath,!WITHDATASET);
#ifdef DEBUG
fprintf(stderr,"qualify.sel: %s -> ",
	dumpselection1(sel));
#endif
    /* Now add path nodes to create full path */
    completesegments3(fullpath,sel->segments);
    for(i=0;i<nclistlength(sel->values);i++) {
        NCvalue* value = (NCvalue*)nclistget(sel->values,i);
        if(value->kind == ST_VAR) {
	    nclistclear(fullpath);
            collectnodepath3(value->value.var.node,fullpath,!WITHDATASET);
	    completesegments3(fullpath,value->value.var.segments);
        }
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
#ifdef IGNORE
    /* If there is only one name match, then assume that is the correct one*/
    if(nclistlength(namematches)==1) {
        if(nodep)
	    *nodep = (CDFnode*)nclistget(namematches,0);
	goto done;
    }
#endif

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

#ifdef IGNORE
static CDFnode*
subpathmatch(NClist* path, CDFnode* node, int depth)
{
    int i;
    char* name;
    CDFnode* leaf = NULL;
    int matches;

    /* invariant: segments 0.depth-1 have already matched */
    if(depth >= nclistlength(path)) return node; /* => 0.|path|-1 match */
    name = (char*)nclistget(path,depth);
    matches = 0;
    for(i=0;i<nclistlength(node->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
	CDFnode* candidate = NULL;
	if(strcmp(subnode->name,name)!=0) continue;
	/* check remainder of the path */
	candidate = subpathmatch(path,subnode,depth+1);
	if(candidate == NULL) continue;
	matches++;
	leaf = candidate;
    }	
    return (matches == 1?leaf:NULL);
}
#endif

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

#ifdef IGNORE
/* Caller must free */
static char*
simplesegmentstring(NClist* segments, char* separator)
{
    NClist* names = nclistnew();
    char* result;
    collectsegmentnames3(segments,names);
    result = simplepathstring3(names,separator);
    nclistfree(names);
    return result;
}

/* Assume this is a DAP projection path */
static void
parsepath3(char* path, NClist* segments)
{
    int c;
    char* p = path;
    char* q = path;
    size_t len;
    char* seg;
    for(;(c=*p);p++) {
	if(c == '.') {
	    /* Capture this segment before the dot */
	    len = (p-q);
	    if(len > 0) {
	        seg = (char*)emalloc(len+1);
	        memcpy((void*)seg,q,len);
	        seg[len] = '\0';
	        nclistpush(segments,(ncelem)seg);
	    }
	    q = p+1;
	}
    }
    /* Capture last segment */
    len = strlen(q);
    if(len > 0) {
	seg = nulldup(q);
        nclistpush(segments,(ncelem)seg);
    }
}
#endif


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

/* Return 1 if we can reuse cached data to address
   the current get_vara request; return 0 otherwise.
   Target is in the constrained tree space.
   Currently, if the target matches a cache not that is not
   a whole variable, then match is false.
*/
int
iscached(NCDAPCOMMON* nccomm, CDFnode* target, NCcachenode** cachenodep)
{
    int i,j,found,index;
    NCcache* cache;
    NCcachenode* cachenode;

    found = 0;
    if(target == NULL) goto done;

    if(!FLAGSET(nccomm->controls,NCF_CACHE)) goto done;

    /* match the target variable against elements in the cache */

    index = 0;
    cache = &nccomm->cdf.cache;
    cachenode = cache->prefetch;

    /* always check prefetch (if it exists) */
    if(cachenode!= NULL) {
        for(found=0,i=0;i<nclistlength(cachenode->vars);i++) {
            CDFnode* var = (CDFnode*)nclistget(cachenode->vars,i);
	    if(var == target) {found=1; break;}
	}
    }
    if(!found) {/*search other cache nodes starting at latest first */
        for(i=nclistlength(cache->nodes)-1;i>=0;i--) {
            cachenode = (NCcachenode*)nclistget(cache->nodes,i);
            for(found=0,j=0;j<nclistlength(cachenode->vars);j++) {
                CDFnode* var = (CDFnode*)nclistget(cachenode->vars,j);
	        if(var == target) {found=1;index=i;break;}
	    }
	    if(found) break;
	}	
    }

    if(found) {
        ASSERT((cachenode != NULL));
        if(cachenode != cache->prefetch && nclistlength(cache->nodes) > 1) {
	    /* Manage the cache nodes as LRU */
	    nclistremove(cache->nodes,index);
	    nclistpush(cache->nodes,(ncelem)cachenode);
	}
        if(cachenodep) *cachenodep = cachenode;
    }
done:
#ifdef DEBUG
fprintf(stderr,"iscached: search: %s\n",makesimplepathstring3(target));
if(found)
   fprintf(stderr,"iscached: found: %s\n",dumpcachenode(cachenode));
else
   fprintf(stderr,"iscached: notfound\n");
#endif
    return found;
}

static NClist*
clonesegments3(NClist* segments)
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

NCprojection*
cloneprojection1(NCprojection* p)
{
    NCprojection* newp;
    if(p == NULL) return NULL;
    newp = createncprojection();
    *newp = *p;
    newp->segments = clonesegments3(p->segments);
    return newp;
}

NClist*
cloneprojections(NClist* projectionlist)
{
    int i;
    NClist* clone = nclistnew();
    for(i=0;i<nclistlength(projectionlist);i++) {
	NCprojection* p = (NCprojection*)nclistget(projectionlist,i);
	NCprojection* newp = cloneprojection1(p);
	nclistpush(clone,(ncelem)newp);
    }
    return clone;
}

NCselection*
cloneselection1(NCselection* s)
{
    int i;
    NCselection* news;
    if(s==NULL) return NULL;
    news = createncselection();
    *news = *s;    
    news->segments = clonesegments3(s->segments);
    news->values = nclistnew();
    for(i=0;i<nclistlength(s->values);i++) {	
	NCvalue* v = (NCvalue*)nclistget(s->values,i);
	NCvalue* newv = createncvalue();
	*newv = *v;
	if(newv->kind == ST_STR)
	    newv->value.text = nulldup(newv->value.text);
	else if(newv->kind == ST_VAR) {
	    newv->value.var.segments = clonesegments3(v->value.var.segments);
	}
	nclistpush(news->values,(ncelem)newv);
    }
    return news;
}

NClist*
cloneselections(NClist* selectionlist)
{
    int i;
    NClist* clone = nclistnew();
    for(i=0;i<nclistlength(selectionlist);i++) {
	NCselection* s = (NCselection*)nclistget(selectionlist,i);
	nclistpush(clone,(ncelem)cloneselection1(s));
    }
    return clone;
}

NCconstraint
cloneconstraint(NCconstraint* con)
{
    NCconstraint clone;
    clone.projections = cloneprojections(con->projections);
    clone.selections = cloneselections(con->selections);
    return clone;
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
mergeprojections3(NCDAPCOMMON* nccomm,
		 NClist* dst,
		 NClist* src)
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
	nclistpush(cat,(ncelem)cloneprojection1(p));
    }    

    nclistclear(dst);

    /* Repeatedly pull elements from the concat,
       merge with all duplicates, and stick into
       the dst
    */
    while(nclistlength(cat) > 0) {
	NCprojection* target = (NCprojection*)nclistremove(cat,0);
	if(target == NULL) continue;
        for(i=0;i<nclistlength(cat);i++) {
	    NCprojection* p2 = (NCprojection*)nclistget(cat,i);
	    if(p2 == NULL || target->leaf != p2->leaf)
		continue;
	    /* This entry matches our current target; merge  */
	    ncstat = mergeprojection31(nccomm,target,p2);
	    /* null out this merged entry and release it */
	    nclistset(cat,i,(ncelem)NULL);	    
	    freencprojection1(p2);	    
	}		    
	/* Capture the clone */
	nclistpush(dst,(ncelem)target);
    }	    
    nclistfree(cat);
    return ncstat;
}

static NCerror
mergeprojection31(NCDAPCOMMON* nccomm, NCprojection* dst, NCprojection* src)
{
    NCerror ncstat = NC_NOERR;
    int i,j;

    /* merge segment by segment;
       |dst->segments| == |src->segments|
       by construction
    */
    ASSERT((nclistlength(dst->segments) == nclistlength(src->segments)));    
    for(i=0;i<nclistlength(dst->segments);i++) {
	NCsegment* dstseg = (NCsegment*)nclistget(dst->segments,i);
	NCsegment* srcseg = (NCsegment*)nclistget(src->segments,i);
	ASSERT((dstseg->node == srcseg->node)); /* by construction */
	for(j=0;j<dstseg->slicerank;j++) {
	    slicemerge3(&dstseg->slices[j],&srcseg->slices[j]);
	}
    }
    return ncstat;
}

#ifdef IGNORE
/*
Given a projection list
and a projection, merge
src into dst taking
overlapping projections into acct.
*/
static NCerror
mergesingleprojection3(NCDAPCOMMON* nccomm,NClist* dst,NCprojection* src)
{
    NCerror stat;
    NClist* tmp = nclistnew();
    nclistpush(tmp,(ncelem)src);
    stat = mergeprojections3(nccomm,dst,tmp);
    nclistfree(tmp);
    return stat;
}    
#endif

/* Convert an NCprojection instance into a string
   that can be used with the url
*/

char*
makeprojectionstring3(NClist* projections)
{
    int i;
    NCbytes* buf = ncbytesnew();
    char* pstring;
    for(i=0;i<nclistlength(projections);i++) {
	NCprojection* p = (NCprojection*)nclistget(projections,i);
        if(i > 0) ncbytescat(buf,",");
	makeprojectionstring13(p,buf,".");
    }
    pstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return pstring;
}

static void
makeprojectionstring13(NCprojection* p, NCbytes* buf, char* separator)
{
    makesegmentstring3(p->segments,buf,separator);
}

void
makesegmentstring3(NClist* segments, NCbytes* buf, char* separator)
{
    int i;
    for(i=0;i<nclistlength(segments);i++) {
        NCsegment* segment = (NCsegment*)nclistget(segments,i);
	if(i > 0) ncbytescat(buf,separator);
	ncbytescat(buf,(segment->name?segment->name:"<unknown>"));
	if(segment->node->nctype == NC_Sequence) continue;
	if(!iswholesegment(segment)) {
	    int rank = segment->slicerank;
	    makeslicestring3(segment->slices,rank,buf);
	}
    }
}

/* This should be consistent with dumpslices in dapdump.c */
static void
makeslicestring3(NCslice* slice, unsigned int rank, NCbytes* buf)
{
    int i;

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

static char* opstrings[] =
{"?","=","!=",">=",">","<=","<","=~","?","?","?","?"};

char*
makeselectionstring3(NClist* selections)
{
    int i;
    NCbytes* buf = ncbytesnew();
    NCbytes* segbuf = ncbytesnew();
    char* sstring;
    for(i=0;i<nclistlength(selections);i++) {
	int j;
	NCselection* sel = (NCselection*)nclistget(selections,i);
	ncbytescat(buf,"&");
	ncbytesclear(segbuf);
	makesegmentstring3(sel->segments,segbuf,".");
	ncbytescat(buf,ncbytescontents(segbuf));
	if(sel->operator == ST_FCN) {
	    ncbytescat(buf,"(");
	} else {
  	    ncbytescat(buf,opstrings[sel->operator]);
	    ncbytescat(buf,"{");
	}
        for(j=0;j<nclistlength(sel->values);j++) {
	    NCvalue* value = (NCvalue*)nclistget(sel->values,j);
	    char tmp[64];
	    if(j > 0) ncbytescat(buf,",");
	    switch (value->kind) {
	    case ST_STR:
                ncbytescat(buf,value->value.text);
		break;		
	    case ST_INT:
	        snprintf(tmp,sizeof(tmp),"%lld",value->value.intvalue);
                ncbytescat(buf,tmp);
		break;
	    case ST_FLOAT:
	        snprintf(tmp,sizeof(tmp),"%g",value->value.floatvalue);
                ncbytescat(buf,tmp);
		break;
	    case ST_VAR:
		ncbytesclear(segbuf);
		makesegmentstring3(value->value.var.segments,segbuf,".");
		ncbytescat(buf,ncbytescontents(segbuf));
		break;
	    default: PANIC1("unexpected tag: %d",(int)value->kind);
	    }
	}
	if(sel->operator == ST_FCN)
	    ncbytescat(buf,"(");
	else
            ncbytescat(buf,"}");
    }
    sstring = ncbytesdup(buf);
    ncbytesfree(buf);
    ncbytesfree(segbuf);
    return sstring;
}

char*
makeconstraintstring3(NClist* projections, char* selectionstring)
{
    NCbytes* buf = ncbytesnew();
    char* result = NULL;
    if(nclistlength(projections) > 0)  {
	char* pstring = makeprojectionstring3(projections);
        ncbytescat(buf,pstring);
	efree(pstring);
    }
    if(selectionstring != NULL && strlen(selectionstring) > 0) {
        ncbytescat(buf,selectionstring);
    }

    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

void
freencsegment(NCsegment* seg)
{
    efree(seg->name);
    efree(seg);
}

void
freencprojection1(NCprojection* p)
{
    int i;
    if(p == NULL) return;
    for(i=0;i<nclistlength(p->segments);i++) {
	NCsegment* seg = (NCsegment*)nclistget(p->segments,i);
	freencsegment(seg);
    }
    nclistfree(p->segments);    
    efree(p);
}

void
freencprojections(NClist* plist)
{
    int i;
    if(plist == NULL) return;
    for(i=0;i<nclistlength(plist);i++) {
        NCprojection* p = (NCprojection*)nclistget(plist,i);
	freencprojection1(p);
    }
    nclistfree(plist);
}

static void
freesegments(NClist* segments)
{
    int i;
    for(i=0;i<nclistlength(segments);i++) {
	NCsegment* seg = (NCsegment*)nclistget(segments,i);
	efree((void*)seg->name);
        efree((void*)seg);
    }
    nclistfree(segments);
}

void
freencselection1(NCselection* s)
{
    int i;
    if(s == NULL) return;
    freesegments(s->segments);
    for(i=0;i<nclistlength(s->values);i++) {
	NCvalue* v = (NCvalue*)nclistget(s->values,i);
        if(v->kind == ST_STR) efree(v->value.text);
        else if(v->kind == ST_VAR)
	    freesegments(v->value.var.segments);
	efree(v);
    }
    nclistfree(s->values);
    efree(s);
}

void
freencselections(NClist* slist)
{
    int i;
    if(slist == NULL) return;
    for(i=0;i<nclistlength(slist);i++) {
        NCselection* s = (NCselection*)nclistget(slist,i);
	freencselection1(s);
    }
    nclistfree(slist);
}

/* WARNING: do not free the instance */
void
clearncconstraint(NCconstraint* con)
{
    if(con->projections) freencprojections(con->projections);
    if(con->selections) freencselections(con->selections);
    con->projections = NULL;
    con->selections = NULL;
}

NCsegment*
createncsegment(void)
{
    NCsegment* mem = NULL;
    mem = (NCsegment*)emalloc(sizeof(NCsegment));
    memset((void*)mem,0,sizeof(NCsegment));
    return mem;
}

NCprojection*
createncprojection(void)
{
    NCprojection* mem = (NCprojection*)emalloc(sizeof(NCprojection));
    memset((void*)mem,0,sizeof(NCprojection));
    return mem;
}

NCselection*
createncselection(void)
{
    NCselection* sel = (NCselection*)emalloc(sizeof(NCselection));
    memset((void*)sel,0,sizeof(NCselection));
    sel->segments = nclistnew();
    sel->values = nclistnew();
    sel->operator = ST_NIL;
    return sel;
}

NCvalue*
createncvalue(void)
{
    NCvalue* mem = (NCvalue*)emalloc(sizeof(NCvalue));
    memset((void*)mem,0,sizeof(NCvalue));
    mem->kind = ST_NIL;
    return mem;
}

NCslice*
createncslice(void)
{
    NCslice* mem = (NCslice*)emalloc(sizeof(NCslice));
    memset((void*)mem,0,sizeof(NCslice));
    return mem;
}

/* Remove all CDFnode* references from constraint */
void
dereference3(NCconstraint* constraint)
{
    int i,j;
    for(i=0;i<nclistlength(constraint->projections);i++) {
	NCprojection* p = (NCprojection*)nclistget(constraint->projections,i);
	p->leaf = NULL;
        for(j=0;j<nclistlength(p->segments);j++) {
	    NCsegment* seg = (NCsegment*)nclistget(p->segments,j);
	    seg->node = NULL;
	}	
    }
    for(i=0;i<nclistlength(constraint->selections);i++) {
	NCselection* s = (NCselection*)nclistget(constraint->selections,i);
	s->leaf = NULL;
        for(j=0;j<nclistlength(s->values);j++) {
	    NCvalue* v = (NCvalue*)nclistget(s->values,j);
	    if(v->kind == ST_VAR) v->value.var.node = NULL;
	}	
    }
}


static NCerror
fillsegmentpath(NCprojection* p, NClist* nodes)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NClist* path = nclistnew();

    collectsegmentnames3(p->segments,path);
    ncstat = matchpartialname3(nodes,path,&p->leaf);
    if(ncstat) goto done;
    /* Now complete the segment path */
    nclistclear(path);
    collectnodepath3(p->leaf,path,!WITHDATASET);
    if(nclistlength(path) != nclistlength(p->segments)) {
	ncstat = NC_EINVAL;
	goto done;
    }
    for(i=0;i<nclistlength(p->segments);i++) {
        NCsegment* seg = (NCsegment*)nclistget(p->segments,i);
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

    ncstat = matchpartialname3(nodes,s->segments,&s->leaf);
    if(ncstat) goto done;
    /* Now find the value paths */
    for(i=0;i<nclistlength(s->values);i++) {
        NCvalue* v = (NCvalue*)nclistget(s->values,i);
	if(v->kind != ST_VAR) continue;
        ncstat = matchpartialname3(nodes,v->value.var.segments,
				   &v->value.var.node);
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
buildvaraprojection3(NCDAPCOMMON* nccomm, Getvara* getvar,
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
    projection->leaf = var;
    projection->segments = segments;

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
    if(ncstat) freencprojection1(projection);
    return ncstat;
}

/* Compute the set of prefetched data */
NCerror
prefetchdata3(NCDAPCOMMON* nccomm)
{
    int i,j;
    NCerror ncstat = NC_NOERR;
    NClist* allvars = nccomm->cdf.varnodes;
    NCconstraint* constraint = &nccomm->oc.constraint;
    NClist* vars = nclistnew();
    NCcachenode* cache = NULL;
    NCconstraint newconstraint = {NULL,NULL};

    /* If caching is off, and we can do constraints, then
       don't even do prefetch
    */
    if(!FLAGSET(nccomm->controls,NCF_CACHE)
	&& !FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
	nccomm->cdf.cache.prefetch = NULL;
	goto done;
    }

    for(i=0;i<nclistlength(allvars);i++) {
	CDFnode* var = (CDFnode*)nclistget(allvars,i);
	size_t nelems = 1;
	/* Compute the # of elements in the variable */
	for(j=0;j<nclistlength(var->array.dimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(var->array.dimensions,j);
	    nelems *= dim->dim.declsize;
	}
	/* If we cannot constrain, then pull in everything */
	if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)
           ||nelems <= nccomm->cdf.smallsizelimit)
	    nclistpush(vars,(ncelem)var);
    }
    /* If we cannot constrain, then pull in everything */
    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE) || nclistlength(vars) == 0) {
	newconstraint.projections = NULL;
	newconstraint.selections= NULL;
    } else {/* Construct the projections for this set of vars */
        /* Initially, the constraints are same as the merged constraints */
        newconstraint.projections = cloneprojections(constraint->projections);
        restrictprojection34(nccomm,vars,newconstraint.projections);
        /* similar for selections */
        newconstraint.selections = cloneselections(constraint->selections);
    }

if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
oc_log(OCLOGNOTE,"prefetch.");
}

    if(nclistlength(vars) == 0)
        cache = NULL;
    else {
        ncstat = buildcachenode34(nccomm,&newconstraint,vars,&cache,1);
        if(ncstat) goto done;
    }
    /* Make cache node be the prefetch node */
    nccomm->cdf.cache.prefetch = cache;

#ifdef DEBUG
/* Log the set of prefetch variables */
NCbytes* buf = ncbytesnew();
ncbytescat(buf,"prefetch.vars: ");
for(i=0;i<nclistlength(vars);i++) {
CDFnode* var = (CDFnode*)nclistget(vars,i);
ncbytescat(buf," ");
ncbytescat(buf,makesimplepathstring3(var));
}
ncbytescat(buf,"\n");
oc_log(OCLOGNOTE,"%s",ncbytescontents(buf));
ncbytesfree(buf);
#endif

done:
    nclistfree(vars);
    clearncconstraint(&newconstraint);    
    if(ncstat) freenccachenode(nccomm,cache);
    return THROW(ncstat);
}

NCerror
buildcachenode34(NCDAPCOMMON* nccomm,
	        NCconstraint* constraint,
		NClist* varlist,
		NCcachenode** cachep,
		int isprefetch)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCconnection conn = nccomm->oc.conn;
    OCobject ocroot = OCNULL;
    CDFnode* dxdroot = NULL;
    NCcachenode* cachenode = NULL;
    char* ce = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
        ce = NULL;
    else
        ce = makeconstraintstring3(constraint->projections,
				   nccomm->oc.url.selection);

    ocstat = dap_oc_fetch(nccomm,conn,ce,OCDATADDS,&ocroot);
    efree(ce);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

    ncstat = buildcdftree34(nccomm,ocroot,OCDATA,&dxdroot);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* regrid */
    if(!FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
        ncstat = regrid3(dxdroot,nccomm->cdf.ddsroot,constraint->projections);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* create the cache node */
    cachenode = createnccachenode();
    cachenode->prefetch = isprefetch;
    cachenode->vars = nclistclone(varlist);
    cachenode->datadds = dxdroot;
    cachenode->constraint = *constraint;
    constraint->projections = NULL;
    constraint->selections = NULL;

    /* save the root content*/
    cachenode->ocroot = ocroot;
    cachenode->content = oc_data_new(conn);
    ocstat = oc_data_root(conn,ocroot,cachenode->content);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

    /* capture the packet size */
    ocstat = oc_raw_xdrsize(conn,ocroot,&cachenode->xdrsize);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

    /* Insert into the cache */

    if(!FLAGSET(nccomm->controls,NCF_CACHE)) goto done;

    if(isprefetch) {
        cachenode->prefetch = 1;
	nccomm->cdf.cache.prefetch = cachenode;
    } else {
	NCcache* cache = &nccomm->cdf.cache;
	if(cache->nodes == NULL) cache->nodes = nclistnew();
	/* remove cache nodes to get below the max cache size */
	while(cache->cachesize + cachenode->xdrsize > cache->cachelimit) {
	    NCcachenode* node = (NCcachenode*)nclistremove(cache->nodes,0);
#ifdef DEBUG
fprintf(stderr,"buildcachenode: purge cache node: %s\n",
	dumpcachenode(cachenode));
#endif
	    cache->cachesize -= node->xdrsize;
	    freenccachenode(nccomm,node);
	}
	/* remove cache nodes to get below the max cache count */
	while(nclistlength(cache->nodes) >= cache->cachecount) {
	    NCcachenode* node = (NCcachenode*)nclistremove(cache->nodes,0);
#ifdef DEBUG
fprintf(stderr,"buildcachenode: count purge cache node: %s\n",
	dumpcachenode(cachenode));
#endif
	    cache->cachesize -= node->xdrsize;
	    freenccachenode(nccomm,node);
        }
        nclistpush(nccomm->cdf.cache.nodes,(ncelem)cachenode);
        cache->cachesize += cachenode->xdrsize;
    }

#ifdef DEBUG
fprintf(stderr,"buildcachenode: %s\n",dumpcachenode(cachenode));
#endif

done:
    if(cachep) *cachep = cachenode;
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    if(ncstat) {
	freecdfroot34(dxdroot);
	freenccachenode(nccomm,cachenode);
    }
    return THROW(ncstat);
}

NCcachenode*
createnccachenode(void)
{
    NCcachenode* mem = (NCcachenode*)emalloc(sizeof(NCcachenode));
    memset((void*)mem,0,sizeof(NCcachenode));
    return mem;
}

void
freenccachenode(NCDAPCOMMON* nccomm, NCcachenode* node)
{
    if(node == NULL) return;
    oc_data_free(nccomm->oc.conn,node->content);
    oc_data_free(nccomm->oc.conn,node->content);
    clearncconstraint(&node->constraint);
    freecdfroot34(node->datadds);
    nclistfree(node->vars);
    efree(node);
}

void
clearnccache(NCDAPCOMMON* nccomm, NCcache* cache)
{
    int i;
    if(cache == NULL) return;
    freenccachenode(nccomm,cache->prefetch);
    for(i=0;i<nclistlength(cache->nodes);i++) {
	freenccachenode(nccomm,(NCcachenode*)nclistget(cache->nodes,i));
    }
}

#ifdef IGNORE
static NCerror
verifyprojectionrank3(NCsegment* seg)
{
    unsigned int rank;
    if(seg->node->array.ncdimensions != NULL)
	rank = nclistlength(seg->node->array.ncdimensions);
    else
	rank = nclistlength(seg->node->array.dimensions);
    if(rank != seg->slicerank) return THROW(NC_EDAPCONSTRAINT);
    return NC_NOERR;  
}
#endif

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
    
    ASSERT((proj->segments != NULL));

    whole = 1; /* assume so */
    for(i=0;i<nclistlength(proj->segments);i++) {
        NCsegment* segment = (NCsegment*)nclistget(proj->segments,i);
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
