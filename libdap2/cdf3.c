/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/cdf3.c,v 1.33 2009/12/03 03:42:37 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "daputil.h"
#include "dapdump.h"

CDFnode* v4node = NULL;

/* Forward*/
static NCerror sequencecheck3r(CDFnode* node, NClist* vars, CDFnode* topseq);
static NCerror restruct3r(CDFnode*, CDFnode*);
static NCerror structwrap3(CDFnode*, CDFnode*, int, CDFnode*, int);
static CDFnode* makenewstruct3(CDFnode* node, CDFnode* template);
static NCerror mapnodes3r(CDFnode*, CDFnode*, int depth);
static NCerror mapfcn(CDFnode* dstnode, CDFnode* srcnode);
static NCerror definedimsetplus3(NCDAPCOMMON* nccomm, CDFnode* node);
static NCerror definedimsetall3(NCDAPCOMMON* nccomm, CDFnode* node);

/* Accumulate useful node sets  */
NCerror
computecdfnodesets3(NCDAPCOMMON* nccomm)
{
    unsigned int i;
    NClist* varnodes = nclistnew(); 
    NClist* allnodes = nccomm->cdf.ddsroot->tree->nodes;

    if(nccomm->cdf.seqnodes == NULL) nccomm->cdf.seqnodes = nclistnew();
    if(nccomm->cdf.gridnodes == NULL) nccomm->cdf.gridnodes = nclistnew();
    nclistclear(nccomm->cdf.seqnodes);
    nclistclear(nccomm->cdf.gridnodes);

    computevarnodes3(nccomm,allnodes,varnodes);
    nclistfree(nccomm->cdf.varnodes);
    nccomm->cdf.varnodes = varnodes;

    /* Now compute other sets of interest */
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	if(!node->visible) continue;
	switch (node->nctype) {
	case NC_Sequence:
	    nclistpush(nccomm->cdf.seqnodes,(ncelem)node);
	    break;
	case NC_Grid:
	    nclistpush(nccomm->cdf.gridnodes,(ncelem)node);
	    break;
	default: break;
	}
    }
    return NC_NOERR;
}

NCerror
computevarnodes3(NCDAPCOMMON* nccomm, NClist* allnodes, NClist* varnodes)
{
    unsigned int i,len;
    NClist* allvarnodes = nclistnew();
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	/* If this node has a bad name, repair it */
	if(dap_badname(node->ocname)) {
	    char* newname = dap_repairname(node->ocname);
	    nullfree(node->ocname);
	    node->ocname = newname;
	}
	if(!node->visible) continue;
	if(node->nctype == NC_Atomic)
	    nclistpush(allvarnodes,(ncelem)node);
    }
    /* Further process the variable nodes to get the final set */
    /* Use toplevel vars first */
    len = nclistlength(allvarnodes);
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(allvarnodes,i);
	if(node == NULL) continue;
        if(daptoplevel(node)) {
	    nclistpush(varnodes,(ncelem)node);
	    nclistset(allvarnodes,i,(ncelem)NULL);
	}
    }
    /*... then grid arrays and maps.
      but exclude the coordinate variables if we are trying to
      exactly mimic nc-dap
    */
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(allvarnodes,i);
	if(node == NULL) continue;
	if(dapgridarray(node)) {
	    nclistpush(varnodes,(ncelem)node);
	    nclistset(allvarnodes,i,(ncelem)NULL);
        } else if(dapgridmap(node)) {
	    if(!FLAGSET(nccomm->controls,NCF_NCDAP))
		nclistpush(varnodes,(ncelem)node);
	    nclistset(allvarnodes,i,(ncelem)NULL);
	}
    }
    /*... then all others */
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(allvarnodes,i);
	if(node == NULL) continue;
        nclistpush(varnodes,(ncelem)node);
    }
    nclistfree(allvarnodes);
#ifdef DEBUG2
for(i=0;i<nclistlength(varnodes);i++) {
CDFnode* node = (CDFnode*)nclistget(varnodes,i);
if(node == NULL) continue;
fprintf(stderr,"computevarnodes: var: %s\n",makecdfpathstring3(node,"."));
}
#endif
    return NC_NOERR;
}

NCerror
fixgrids3(NCDAPCOMMON* nccomm)
{
    unsigned int i;
    NClist* gridnodes = nccomm->cdf.gridnodes;

    for(i=0;i<nclistlength(gridnodes);i++) {
        CDFnode* grid = (CDFnode*)nclistget(gridnodes,i);
        (void)fixgrid34(nccomm,grid);
	/* Ignore mal-formed grids */
    }
    return NC_NOERR;
}

/*
Figure out the names for variables.
*/
NCerror
computecdfvarnames3(NCDAPCOMMON* nccomm, CDFnode* root, NClist* varnodes)
{
    unsigned int i,j,d;

    /* clear all elided marks; except for dataset and grids */
    for(i=0;i<nclistlength(root->tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(root->tree->nodes,i);
	node->elided = 0;
	if(node->nctype == NC_Grid || node->nctype == NC_Dataset)
	    node->elided = 1;
    }

    /* ensure all variables have an initial full name defined */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	nullfree(var->ncfullname);
	var->ncfullname = makecdfpathstring3(var,nccomm->cdf.separator);
#ifdef DEBUG2
fprintf(stderr,"var names: %s %s %s\n",
	var->ocname,var->ncbasename,var->ncfullname);
#endif
    }

    /*  unify all variables with same fullname and dimensions
	basevar fields says: "for duplicate grid variables";
        when does this happen?
    */
    if(FLAGSET(nccomm->controls,NCF_NC3)) {
        for(i=0;i<nclistlength(varnodes);i++) {
	    int match;
	    CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	    for(j=0;j<i;j++) {
	        CDFnode* testnode = (CDFnode*)nclistget(varnodes,j);
		match = 1;
	        if(testnode->array.basevar != NULL)
		    continue; /* already processed */
	        if(strcmp(var->ncfullname,testnode->ncfullname) != 0)
		    match = 0;
		else if(nclistlength(testnode->array.dimsetall)
			!= nclistlength(var->array.dimsetall))
		    match = 0;
	        else for(d=0;d<nclistlength(testnode->array.dimsetall);d++) {
		    CDFnode* vdim = (CDFnode*)nclistget(var->array.dimsetall,d);
		    CDFnode* tdim = (CDFnode*)nclistget(testnode->array.dimsetall,d);
	            if(vdim->dim.declsize != tdim->dim.declsize) {
		        match = 0;
			break;
		    }
		}
		if(match) {
		    testnode->array.basevar = var;
fprintf(stderr,"basevar invoked: %s\n",var->ncfullname);
		}
	    }
	}
    }

    /* Finally, verify unique names */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var1 = (CDFnode*)nclistget(varnodes,i);
	if(var1->array.basevar != NULL) continue;
	for(j=0;j<i;j++) {
	    CDFnode* var2 = (CDFnode*)nclistget(varnodes,j);
	    if(var2->array.basevar != NULL) continue;
	    if(strcmp(var1->ncfullname,var2->ncfullname)==0) {
		PANIC1("duplicate var names: %s",var1->ncfullname);
	    }
	}
    }
    return NC_NOERR;
}


/* locate and connect usable sequences and vars.
A sequence is usable iff:
1. it has a path from one of its subnodes to a leaf and that
   path does not contain a sequence.
2. No parent container has dimensions.
*/

NCerror
sequencecheck3(NCDAPCOMMON* nccomm)
{
    (void)sequencecheck3r(nccomm->cdf.ddsroot,nccomm->cdf.varnodes,NULL);    
    return NC_NOERR;
}


static NCerror
sequencecheck3r(CDFnode* node, NClist* vars, CDFnode* topseq)
{
    unsigned int i;
    NCerror err = NC_NOERR;
    int ok = 0;
    if(topseq == NULL && nclistlength(node->array.dimset0) > 0) {
	err = NC_EINVAL; /* This container has dimensions, so no sequence within it
                            can be usable */
    } else if(node->nctype == NC_Sequence) {
	/* Recursively walk the path for each subnode of this sequence node
           looking for a path without any sequence */
	for(i=0;i<nclistlength(node->subnodes);i++) {
	    CDFnode* sub = (CDFnode*)nclistget(node->subnodes,i);
	    err = sequencecheck3r(sub,vars,node);
	    if(err == NC_NOERR) ok = 1; /* there is at least 1 usable var below */
	}
	if(topseq == NULL && ok == 1) {
	    /* this sequence is usable because it has scalar container
               (by construction) and has a path to a leaf without an intermediate
               sequence. */
	    err = NC_NOERR;
	    node->usesequence = 1;
	} else {
	    /* this sequence is unusable because it has no path
               to a leaf without an intermediate sequence. */
	    node->usesequence = 0;
	    err = NC_EINVAL;
	}
    } else if(nclistcontains(vars,(ncelem)node)) {
	/* If we reach a leaf, then topseq is usable, so save it */
	node->array.sequence = topseq;
    } else { /* Some kind of non-sequence container node with no dimensions */
	/* recursively compute usability */
	for(i=0;i<nclistlength(node->subnodes);i++) {
	    CDFnode* sub = (CDFnode*)nclistget(node->subnodes,i);
	    err = sequencecheck3r(sub,vars,topseq);
	    if(err == NC_NOERR) ok = 1;
	}
	err = (ok?NC_NOERR:NC_EINVAL);
    }
    return err;
}

/*
OPeNDAP is in the process of changing servers so that
partial grids are converted to structures.  However, not all
servers do this, and not consistently.
The rules that most old-style servers appear to adhere to are these.
1. Asking for just a grid array or a single grid map
   returns just the array not wrapped in a structure.
2. Asking for a subset of the fields (array plus map) of a grid
   returns those fields wrapped in a structure.
3. However, there is an odd situation: asking for a grid array
   plus any subset of maps that includes the last map in the grid
   returns a malformed grid. This is clearly a bug.

For case 1, we insert a structure node so that case 1 is consistent
with case 2. Case 3 should cause an error with a malformed grid.

[Note: for some reason, this code has been difficult to get right;
I have rewritten 6 times and it probably is still not right.]

Input is
(1) the root of the dds that needs to be re-gridded
(2) the full datadds tree that defines where the grids are.
(3) the projections that were used to produce (1) from (2).

*/

NCerror
restruct3(CDFnode* ddsroot, CDFnode* template, NClist* projections)
{
    NCerror ncstat = NC_NOERR;

    /* The current restruct assumes that the ddsroot tree
       has missing grids compared to the template.
       It is also assumed that order of the nodes
       in the ddsroot is the same as in the template.
    */
    if(ddsroot->tree->restructed) return NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"restruct: ddsroot=%s\n",dumptree(ddsroot));
fprintf(stderr,"restruct: template=%s\n",dumptree(template));
#endif

    /* Try to match this node against the template */
    if(!simplenodematch34(ddsroot,template))
	{ncstat = NC_EDATADDS; goto done;}

    ncstat = restruct3r(ddsroot,template);
    ddsroot->tree->restructed = 1;

done:
    return THROW(ncstat);
}

/*
Locate nodes in the tree rooted at node
that correspond to a single grid field in the template
when the template is a grid.
Wrap that grid field in a synthesized structure.
*/
static NCerror
restruct3r(CDFnode* node, CDFnode* template)
{
    unsigned int inode, itemp, i;
    NCerror ncstat = NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"restruct: matched: %s -> %s\n",
node->ocname,template->ocname);
#endif

    /* this part is tricky; except for nodes needing
       wrapping, the set of children of the node better
       be a subset of the children in the template.
    */

    /* walk each node child and locate its match
       in the template's children; recurse on matches,
       non-matches should be nodes needing wrapping.
    */
    for(inode=0;inode<nclistlength(node->subnodes);inode++) {
        CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,inode);
        CDFnode* wrap = NULL;           
	int wrapindex = 0;
	int match = 0;
        for(itemp=0;itemp<nclistlength(template->subnodes);itemp++) {
            CDFnode* subtemp = (CDFnode*)nclistget(template->subnodes,itemp);
            if(simplenodematch34(subnode,subtemp)) {
                /* this subnode of the node matches the corresponding
                   node of the template, so it is ok =>
                   recurse looking for nested mis-matches
                */
                ncstat = restruct3r(subnode,subtemp);
                if(ncstat != NC_NOERR) return THROW(ncstat);
                match = 1; /* indicate that we matched */
                break;
            } else if(subtemp->nctype == NC_Grid || subtemp->nctype == NC_Structure) {
                /* See if we can match this subnode to a subnode
                    of the grid or structure */
                for(i=0;i<nclistlength(subtemp->subnodes);i++) {
                    CDFnode* level2 = (CDFnode*)nclistget(subtemp->subnodes,i);
                    if(simplenodematch34(subnode,level2)) {
                        /* ASSUME that subnode needs rewrap wrt subtemp */
                        wrap = subtemp;                         
			wrapindex = i;
                        break;
                    }
                }   
            }
        }
        if(!match && wrap == NULL) {
            /* This should never occur */
            ncstat = NC_EDATADDS; goto done;
        }
        if(wrap != NULL) { /* This subnode should need wrapping */
            ASSERT(wrap->nctype == NC_Grid || wrap->nctype == NC_Structure);
            ncstat = structwrap3(subnode,node,inode,wrap,wrapindex);
            if(ncstat != NC_NOERR) goto done;
	    /* since grids are terminal, there will be no
               need to continue the recursion */
       }
    }
done:
    return THROW(ncstat);

}

/* Wrap the node wrt the template grid or template struct */

static NCerror
structwrap3(CDFnode* node, CDFnode* parent, int parentindex,
                           CDFnode* templategrid, int gridindex)
{
    NCerror ncstat = NC_NOERR;
    CDFnode* newstruct;

    ASSERT((templategrid->nctype == NC_Grid || templategrid->nctype == NC_Structure));
    newstruct = makenewstruct3(node,templategrid);
    if(newstruct == NULL) {ncstat = NC_ENOMEM; goto done;}
    /* replace the node with the new structure
       in the parent's list of children*/
    nclistremove(parent->subnodes,parentindex);
    nclistinsert(parent->subnodes,parentindex,(ncelem)newstruct);
    /* Update the list of all nodes in the tree */
    nclistpush(node->root->tree->nodes,(ncelem)newstruct);

done:
    return ncstat;
}

/* Create a structure to surround projected grid array or map;
   this occurs because some servers (that means you ferret and you thredds!)
   do not adhere to the DAP2 protocol spec.
*/
  
static CDFnode*
makenewstruct3(CDFnode* node, CDFnode* templatenode)
{
    CDFnode* newstruct = (CDFnode*)calloc(1,sizeof(CDFnode));
    if(newstruct == NULL) return NULL;
    newstruct->nctype = NC_Structure;
    newstruct->virtual = 1;
    newstruct->ocname = nulldup(templatenode->ocname);
    newstruct->ocnode = templatenode->ocnode;
    newstruct->ncbasename = nulldup(templatenode->ncbasename);
    newstruct->subnodes = nclistnew();
    newstruct->container = node->container;
    newstruct->template = templatenode;
    node->container = newstruct;
    nclistpush(newstruct->subnodes,(ncelem)node);
    return newstruct;
}

/**
Make the constrained dds nodes (root)
point to the corresponding unconstrained
dds nodes (fullroot).
 */

NCerror
mapnodes3(CDFnode* root, CDFnode* fullroot)
{
    NCerror ncstat = NC_NOERR;
    ASSERT(root != NULL && fullroot != NULL);
    if(!simplenodematch34(root,fullroot))
	{THROWCHK(ncstat=NC_EINVAL); goto done;}
    /* clear out old associations*/
    unmap3(root);
    ncstat = mapnodes3r(root,fullroot,0);
done:
    return ncstat;
}

static NCerror
mapnodes3r(CDFnode* connode, CDFnode* fullnode, int depth)
{
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;

    ASSERT((simplenodematch34(connode,fullnode)));
    
#ifdef DEBUG
{
char* path1 = makecdfpathstring3(fullnode,".");
char * path2 = makecdfpathstring3(connode,".");
fprintf(stderr,"mapnode: %s->%s\n",path1,path2);
nullfree(path1); nullfree(path2);
}
#endif

    /* Map node */
    mapfcn(connode,fullnode);

    /* Try to match connode subnodes against fullnode subnodes */
    ASSERT(nclistlength(connode->subnodes) <= nclistlength(fullnode->subnodes));

    for(i=0;i<nclistlength(connode->subnodes);i++) {
        CDFnode* consubnode = (CDFnode*)nclistget(connode->subnodes,i);
	/* Search full subnodes for a matching subnode from con */
        for(j=0;j<nclistlength(fullnode->subnodes);j++) {
            CDFnode* fullsubnode = (CDFnode*)nclistget(fullnode->subnodes,j);
            if(simplenodematch34(fullsubnode,consubnode)) {
                ncstat = mapnodes3r(consubnode,fullsubnode,depth+1);
   	        if(ncstat) goto done;
	    }
	}
    }
done:
    return THROW(ncstat);
}


/* The specific actions of a map are defined
   by this function.
*/
static NCerror
mapfcn(CDFnode* dstnode, CDFnode* srcnode)
{
    /* Mark node as having been mapped */
    dstnode->visible = 1;
    dstnode->basenode = srcnode;
    return NC_NOERR;
}

void
unmap3(CDFnode* root)
{
    unsigned int i;
    CDFtree* tree = root->tree;
    for(i=0;i<nclistlength(tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(tree->nodes,i);
	node->basenode = NULL;
        node->visible = 0;
    }
}

/* 
Move dimension data from basenodes to nodes
*/

NCerror
dimimprint3(NCDAPCOMMON* nccomm)
{
    NCerror ncstat = NC_NOERR;
    NClist* allnodes;
    int i,j;
    CDFnode* basenode;

    allnodes = nccomm->cdf.ddsroot->tree->nodes;
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	int noderank, baserank;
        /* Do dimension imprinting */
	basenode = node->basenode;
	if(basenode == NULL) continue;
	noderank = nclistlength(node->array.dimset0);
	baserank = nclistlength(basenode->array.dimset0);
	if(noderank == 0) continue;
        ASSERT(noderank == baserank);
#ifdef DEBUG
fprintf(stderr,"dimimprint %s/%d -> %s/%d\n",
	makecdfpathstring3(basenode,"."),
	noderank,
	makecdfpathstring3(node,"."),
	baserank);
#endif
        for(j=0;j<noderank;j++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimset0,j);
	    CDFnode* basedim = (CDFnode*)nclistget(basenode->array.dimset0,j);
	    dim->dim.declsize0 = basedim->dim.declsize;	
#ifdef DEBUG
fprintf(stderr,"dimimprint: %d: %lu -> %lu\n",i,basedim->dim.declsize,dim->dim.declsize0);
#endif
        }
    }
    return ncstat;
}

static CDFnode*
clonedim(NCDAPCOMMON* nccomm, CDFnode* dim, CDFnode* var)
{
    CDFnode* clone;
    clone = makecdfnode34(nccomm,dim->ocname,OC_Dimension,
			  NULL,dim->container);
    /* Record its existence */
    nclistpush(dim->container->root->tree->nodes,(ncelem)clone);
    clone->dim = dim->dim; /* copy most everything */
    clone->dim.dimflags |= CDFDIMCLONE;
    clone->dim.array = var;
    return clone;
}

static NClist*
clonedimset3(NCDAPCOMMON* nccomm, NClist* dimset, CDFnode* var)
{
    NClist* result = nclistnew();
    int i;
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	nclistpush(result,(ncelem)clonedim(nccomm,dim,var));
    }
    return result;
}

/* Define the dimsetplus list for a node */
static NCerror
definedimsetplus3(NCDAPCOMMON* nccomm, CDFnode* node)
{
    int ncstat = NC_NOERR;
    NClist* dimset;
    CDFnode* clone;

    ASSERT(node->array.dimsetplus == NULL);
    if(node->array.dimset0 == NULL)
	dimset = nclistnew();
    else { /* copy the dimset0 into dimset */
        dimset = nclistclone(node->array.dimset0);
    }
    /* Insert the sequence or string dims */
    if(node->array.stringdim != NULL) {
	clone = node->array.stringdim;
        nclistpush(dimset,(ncelem)clone);
    }
    if(node->array.seqdim != NULL) {
	clone = node->array.seqdim;
        nclistpush(dimset,(ncelem)clone);
    }
    node->array.dimsetplus = dimset;
    return ncstat;
}

/* Define the dimsetall list for a node */
static NCerror
definedimsetall3(NCDAPCOMMON* nccomm, CDFnode* node)
{
    int i;
    int ncstat = NC_NOERR;
    NClist* dimsetall;

    /* Because of upward recursion (see below) the dimsetall may
       already be defined */
    if(node->array.dimsetall != NULL)
	return ncstat;
    if(node->container != NULL) {
        if(node->container->array.dimsetall == NULL) {
#ifdef DEBUG1
fprintf(stderr,"dimsetall: recurse to container%s\n",node->container->ocname);
#endif
	    ncstat = definedimsetall3(nccomm,node->container);
	    if(ncstat != NC_NOERR) return ncstat;
        }
	/* We need to clone the parent dimensions because we will be assigning
           indices vis-a-vis this variable */
        dimsetall = clonedimset3(nccomm,node->container->array.dimsetall,node);
    } else
	dimsetall = nclistnew();
    // concat parentall and dimset;
    for(i=0;i<nclistlength(node->array.dimsetplus);i++) {
	CDFnode* clone = (CDFnode*)nclistget(node->array.dimsetplus,i);
	nclistpush(dimsetall,(ncelem)clone);
    }
    node->array.dimsetall = dimsetall;
#ifdef DEBUG1
fprintf(stderr,"dimsetall: |%s|=%d\n",node->ocname,nclistlength(dimsetall));
#endif
    return ncstat;
}

/* Define the dimsetplus and dimsetall lists for
   all nodes with dimensions
*/
NCerror
definedimsets3(NCDAPCOMMON* nccomm)
{
    int i;
    int ncstat = NC_NOERR;
    NClist* allnodes = nccomm->cdf.ddsroot->tree->nodes;

    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* rankednode = (CDFnode*)nclistget(allnodes,i);
	if(rankednode->nctype == NC_Dimension) continue; //ignore
	ASSERT((rankednode->array.dimsetplus == NULL));
	ncstat = definedimsetplus3(nccomm,rankednode);
	if(ncstat != NC_NOERR) return ncstat;
    }
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* rankednode = (CDFnode*)nclistget(allnodes,i);
	if(rankednode->nctype == NC_Dimension) continue; //ignore
	ASSERT((rankednode->array.dimsetplus != NULL));
	ncstat = definedimsetall3(nccomm,rankednode);
	if(ncstat != NC_NOERR) return ncstat;
    }     
    return NC_NOERR;
}

