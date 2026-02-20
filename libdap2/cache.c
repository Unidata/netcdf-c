/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "dapincludes.h"
#include "dapdump.h"
#include <stddef.h>

/*
Grads servers always require a constraint,
which does not necessarily happen during prefetch.
So this flag controls this. By default, it is on.
*/
#define GRADS_PREFETCH

static int iscacheableconstraint(DCEconstraint* con);

/**
Test whether a variable's data can be satisfied from the cache.
Searches the prefetch node first, then other cache nodes (LRU order).
Only whole-variable cache nodes are considered for matching.
Target must be in the constrained tree space.
@param nccomm the common DAP state containing the cache
@param target the CDFnode variable to look up in the cache
@param cachenodep output pointer to the matching cache node, or unchanged if not found
@return 1 if a usable cache node was found, 0 otherwise
*/
int
iscached(NCDAPCOMMON* nccomm, CDFnode* target, NCcachenode** cachenodep)
{
    int found;
    size_t j;
    size_t index;
    NCcache* cache;
    NCcachenode* cachenode;

    found = 0;
    if(target == NULL) goto done;

    /* Match the target variable against the prefetch, if any */
    /* Note that prefetches are always whole variable */
    cache = nccomm->cdf.cache;
    cachenode = cache->prefetch;
    if(cachenode!= NULL) {
        found = 0;
        for(size_t i=0;i<nclistlength(cachenode->vars);i++) {
            CDFnode* var = (CDFnode*)nclistget(cachenode->vars,i);
            if(var == target) {
                if(cachenodep) *cachenodep = cachenode;
                found=1;
                goto done;
            }
        }
    }

    /*search other cache nodes starting at latest first */
    index = 0;
    for(size_t i = nclistlength(cache->nodes); i-->0;) {
        cachenode = (NCcachenode*)nclistget(cache->nodes,i);
	/* We currently do not try to match constraints;
           If the cachenode is constrained by more than
           simple wholevariable projections, then skip it.
        */
	if(!cachenode->wholevariable) continue;
        for(found=0,j=0;j<nclistlength(cachenode->vars);j++) {
            CDFnode* var = (CDFnode*)nclistget(cachenode->vars,j);
            if(var == target) {found=1;index=i;break;}
	}
	if(found) break;
    }

    if(found) {
        ASSERT((cachenode != NULL));
        if(nclistlength(cache->nodes) > 1) {
	    /* Manage the cache nodes as LRU */
	    nclistremove(cache->nodes,index);
	    nclistpush(cache->nodes,(void*)cachenode);
	}
        if(cachenodep) *cachenodep = cachenode;
    }

done:
#ifdef DEBUG
fprintf(stderr,"iscached: search: %s\n",makecdfpathstring(target,"."));
if(found)
   fprintf(stderr,"iscached: found: %s\n",dumpcachenode(cachenode));
else
   fprintf(stderr,"iscached: notfound\n");
#endif
    return found;
}

/**
Fetch and cache the set of prefetchable variables.
All prefetches are whole-variable fetches. If the dataset is
unconstrainable, the entire dataset is prefetched.
@param nccomm the common DAP state
@return NC_NOERR on success, an NC error code on failure
*/
NCerror
prefetchdata(NCDAPCOMMON* nccomm)
{
    size_t i;
    NCFLAGS flags;
    NCerror ncstat = NC_NOERR;
    NClist* allvars = nccomm->cdf.ddsroot->tree->varnodes;
    DCEconstraint* urlconstraint = nccomm->oc.dapconstraint;
    NClist* vars = nclistnew();
    NCcachenode* cache = NULL;
    DCEconstraint* newconstraint = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
        /* If we cannot constrain and caching is enabled,
           then pull in everything */
        if(FLAGSET(nccomm->controls,NCF_CACHE)) {
	    for(i=0;i<nclistlength(allvars);i++) {
	        nclistpush(vars,nclistget(allvars,i));
	    }
	} else { /* do no prefetching */
    	    nccomm->cdf.cache->prefetch = NULL;
	    goto done;
	}
    } else {
	/* pull in those variables previously marked as prefetchable */
        for(i=0;i<nclistlength(allvars);i++) {
            CDFnode* var = (CDFnode*)nclistget(allvars,i);

	    /* Most of the important testing was already done */
	    if(!var->basenode->prefetchable)
		continue;

	    /* Do not attempt to prefetch any variables in the
               nc_open url's projection list
	    */
	    if(nclistcontains(nccomm->cdf.projectedvars,(void*)var))
		continue;

	    /* Should be prefetchable */
	    nclistpush(vars,(void*)var);
if(SHOWFETCH) {
nclog(NCLOGDEBUG,"prefetch: %s",var->ncfullname);
}
	}
    }

    /* If there are no vars, then do nothing */
    if(nclistlength(vars) == 0) {
	nccomm->cdf.cache->prefetch = NULL;
	goto done;
    }

    /* Create a single constraint consisting of the projections for the variables;
       each projection is whole variable. The selections are passed on as is.
       Conditionally, The exception is if we are prefetching everything.
    */

    newconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
    newconstraint->projections = nclistnew();
    newconstraint->selections = dceclonelist(urlconstraint->selections);

    for(i=0;i<nclistlength(vars);i++) {
	DCEprojection* varprojection;
        CDFnode* var = (CDFnode*)nclistget(vars,i);
	/* Ignore invisible vars */
	if(var->invisible) continue; 
	/* convert var to a projection */
	ncstat = dapvar2projection(var,&varprojection);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	nclistpush(newconstraint->projections,(void*)varprojection);
    }
if(SHOWFETCH) {
char* s = dumpprojections(newconstraint->projections);
LOG1(NCLOGNOTE,"prefetch.final: %s",s);
nullfree(s);
}

    flags = NCF_PREFETCH;
#ifndef GRADS_PREFETCH
    if(nclistlength(allvars) == nclistlength(vars)) flags |= NCF_PREFETCH_ALL;
#endif
    ncstat = buildcachenode(nccomm,newconstraint,vars,&cache,flags);
    newconstraint = NULL; /* buildcachenodetakes control of newconstraint */
    if(ncstat != OC_NOERR) goto done;
    else if(cache == NULL) goto done;
    else
      cache->wholevariable = 1; /* All prefetches are whole variable */
    /* Make cache node be the prefetch node */
    nccomm->cdf.cache->prefetch = cache;
if(SHOWFETCH) {
LOG0(NCLOGNOTE,"prefetch.complete");
}

if(SHOWFETCH) {
char* s = NULL;
/* Log the set of prefetch variables */
NCbytes* buf = ncbytesnew();
ncbytescat(buf,"prefetch.vars: ");
for(i=0;i<nclistlength(vars);i++) {
CDFnode* var = (CDFnode*)nclistget(vars,i);
ncbytescat(buf," ");
s = makecdfpathstring(var,".");
ncbytescat(buf,s);
nullfree(s);
 }
ncbytescat(buf,"\n");
nclog(NCLOGNOTE,"%s",ncbytescontents(buf));
ncbytesfree(buf);
 }

done:
    nclistfree(vars);
    dcefree((DCEnode*)newconstraint);
    if(ncstat && cache != NULL) freenccachenode(nccomm,cache);
    return THROW(ncstat);
}

/**
Fetch data from the server and build a new cache node.
Issues a constrained DATADDS fetch, restructures the resulting tree,
and inserts the new node into the cache (evicting old entries as needed
to stay within the cache size and count limits).
@param nccomm the common DAP state
@param constraint the DAP constraint to use for the fetch; ownership is transferred
@param varlist the list of CDFnode variables covered by this cache node
@param cachep output pointer to the newly created cache node
@param flags cache control flags (e.g., NCF_PREFETCH)
@return NC_NOERR on success, an NC error code on failure
*/
NCerror
buildcachenode(NCDAPCOMMON* nccomm,
	        DCEconstraint* constraint,
		NClist* varlist,
		NCcachenode** cachep,
		NCFLAGS flags)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OClink conn = nccomm->oc.conn;
    OCddsnode ocroot = NULL;
    CDFnode* dxdroot = NULL;
    NCcachenode* cachenode = NULL;
    char* ce = NULL;
    int isprefetch = 0;

    if((flags & NCF_PREFETCH) != 0)
	isprefetch = 1;

#ifndef GRADS_PREFETCH
    if((flags & NCF_PREFETCH_ALL) == 0)
#endif
        ce = dcebuildconstraintstring(constraint);

    ncstat = dap_fetch(nccomm,conn,ce,OCDATADDS,&ocroot);
    nullfree(ce);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = buildcdftree(nccomm,ocroot,OCDATA,&dxdroot);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* re-struct*/
    if(!FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
        ncstat = restruct(nccomm,dxdroot,nccomm->cdf.fullddsroot,
			   constraint->projections);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* create the cache node */
    cachenode = createnccachenode();
    cachenode->isprefetch = isprefetch;
    cachenode->vars = nclistclone(varlist,0);
    cachenode->datadds = dxdroot;
    /* Give the constraint over to the cachenode */
    cachenode->constraint = constraint;
    constraint = NULL;
    cachenode->wholevariable = iscacheableconstraint(cachenode->constraint);

    /* save the root content*/
    cachenode->ocroot = ocroot;
    ocstat = oc_data_getroot(conn,ocroot,&cachenode->content);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

    /* capture the packet size */
    ocstat = oc_raw_xdrsize(conn,ocroot,&cachenode->xdrsize);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

#ifdef DEBUG
fprintf(stderr,"buildcachenode: new cache node: %s\n",
	dumpcachenode(cachenode));
#endif
    /* Insert into the cache. If not caching, then
       remove any previous cache node
    */
    if(!isprefetch) {
	NCcache* cache = nccomm->cdf.cache;
	if(cache->nodes == NULL) cache->nodes = nclistnew();
	/* remove cache nodes to get below the max cache size */
	while(cache->cachesize + cachenode->xdrsize > cache->cachelimit
	      && nclistlength(cache->nodes) > 0) {
	    NCcachenode* node = (NCcachenode*)nclistremove(cache->nodes,0);
#ifdef DEBUG
fprintf(stderr,"buildcachenode: purge cache node: %s\n",
	dumpcachenode(cachenode));
#endif
	    cache->cachesize -= node->xdrsize;
	    freenccachenode(nccomm,node);
	}
	/* Remove cache nodes to get below the max cache count */
	/* If not caching, then cachecount should be 0 */
	while(nclistlength(cache->nodes) > cache->cachecount) {
	    NCcachenode* node = (NCcachenode*)nclistremove(cache->nodes,0);
#ifdef DEBUG
fprintf(stderr,"buildcachenode: count purge cache node: %s\n",
	dumpcachenode(node));
#endif
	    cache->cachesize -= node->xdrsize;
	    freenccachenode(nccomm,node);
        }
        nclistpush(nccomm->cdf.cache->nodes,(void*)cachenode);
        cache->cachesize += cachenode->xdrsize;
    }

#ifdef DEBUG
fprintf(stderr,"buildcachenode: %s\n",dumpcachenode(cachenode));
#endif

done:
    if(constraint != NULL) dcefree((DCEnode*)constraint);
    if(cachep) *cachep = cachenode;
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    if(ncstat != OC_NOERR) {
	freecdfroot(dxdroot);
	freenccachenode(nccomm,cachenode);
	if(cachep) *cachep = NULL;
    }
    return THROW(ncstat);
}

/**
Allocate and zero-initialize a new NCcachenode.
@return pointer to the new NCcachenode, or NULL on allocation failure
*/
NCcachenode*
createnccachenode(void)
{
    NCcachenode* mem = (NCcachenode*)calloc(1,sizeof(NCcachenode));
    return mem;
}

/**
Free an NCcachenode and all resources it owns.
Releases the OC data content, constraint, datadds tree, and variable list.
@param nccomm the common DAP state (used for the OC connection)
@param node the cache node to free; no-op if NULL
*/
void
freenccachenode(NCDAPCOMMON* nccomm, NCcachenode* node)
{
    if(node == NULL) return;
#ifdef DEBUG
fprintf(stderr,"freecachenode: %s\n",
	dumpcachenode(node));
#endif
    oc_data_free(nccomm->oc.conn,node->content);
    dcefree((DCEnode*)node->constraint);
    freecdfroot(node->datadds);
    nclistfree(node->vars);
    nullfree(node);

}

/**
Free an NCcache and all cache nodes it contains.
@param nccomm the common DAP state (passed to freenccachenode)
@param cache the cache to free; no-op if NULL
*/
void
freenccache(NCDAPCOMMON* nccomm, NCcache* cache)
{
    size_t i;
    if(cache == NULL) return;
    freenccachenode(nccomm,cache->prefetch);
    for(i=0;i<nclistlength(cache->nodes);i++) {
	freenccachenode(nccomm,(NCcachenode*)nclistget(cache->nodes,i));
    }
    nclistfree(cache->nodes);
    nullfree(cache);
}

/**
Allocate and initialize a new NCcache with default limits.
@return pointer to the new NCcache, or NULL on allocation failure
*/
NCcache*
createnccache(void)
{
    NCcache* c = (NCcache*)calloc(1,sizeof(NCcache));
    if(c == NULL)
	return NULL;
    c->cachelimit = DFALTCACHELIMIT;
    c->cachesize = 0;
    c->nodes = nclistnew();
    c->cachecount = DFALTCACHECOUNT;
    return c;
}

static int
iscacheableprojection(DCEprojection* proj)
{
    size_t i;
    int cacheable;
    if(proj->discrim != CES_VAR) return 0;
    cacheable = 1; /* assume so */
    for(i=0;i<nclistlength(proj->var->segments);i++) {
        DCEsegment* segment = (DCEsegment*)nclistget(proj->var->segments,i);
	if(!dapiswholesegment(segment)) {cacheable = 0; break;}
    }
    return cacheable;
}

static int
iscacheableconstraint(DCEconstraint* con)
{
    size_t i;
    if(con == NULL) return 1;
    if(con->selections != NULL && nclistlength(con->selections) > 0)
	return 0; /* can't deal with selections */
    for(i=0;i<nclistlength(con->projections);i++) {
        if(!iscacheableprojection((DCEprojection*)nclistget(con->projections,i)))
	    return 0;
    }
    return 1;
}

/**
Mark variables in the unconstrained DDS as eligible for prefetching.
A variable is prefetchable if:
1. it is atomic,
2. its total element count is within the smallsizelimit, and
3. it is not contained in a sequence.
@param nccomm the common DAP state
@return NC_NOERR always
*/
NCerror
markprefetch(NCDAPCOMMON* nccomm)
{
    size_t i,j;
    NClist* allvars = nccomm->cdf.fullddsroot->tree->varnodes;
    assert(allvars != NULL);
    /* mark those variables of sufficiently small size */
    for(i=0;i<nclistlength(allvars);i++) {
	CDFnode* var = (CDFnode*)nclistget(allvars,i);
	size_t nelems;

	/* If var is not atomic, then it is not prefetchable */
	if(var->nctype != NC_Atomic)
	    continue;

        /* if var is under a sequence, then never prefetch */
        if(dapinsequence(var))
	    continue;

        /* Compute the # of elements in the variable */
        for(nelems=1,j=0;j<nclistlength(var->array.dimsettrans);j++) {
            CDFnode* dim = (CDFnode*)nclistget(var->array.dimsettrans,j);
            nelems *= dim->dim.declsize;
	}
        if(nelems <= nccomm->cdf.smallsizelimit
           && FLAGSET(nccomm->controls,NCF_PREFETCH)) {
          var->prefetchable = 1;
          if(SHOWFETCH)
            {
              extern char* ocfqn(OCddsnode);
              char *tmp = ocfqn(var->ocnode);
              nclog(NCLOGDEBUG,"prefetchable: %s=%lu",
                    tmp,(unsigned long)nelems);
              free(tmp);
            }
        }
    }
    return NC_NOERR;
}
