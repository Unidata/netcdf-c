/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "includes.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Mnemonic */
#define getncid(drno) (((NC*)drno)->ext_ncid)

extern NC_FILE_INFO_T* nc_file;

static void freeNCCDMR(NCCDMR* cdmr);
static int nccr_process_projections(NCCDMR* cdmr);

static int nccr_collect_allvariables(NClist* streamnodes, NClist* varset);
static int nccr_mark_segment_decls(CCEprojection* p, CRnode* leaf);
static int nccr_collect_projection_variables(NCCDMR* cdmr);
static int nccr_mark_visible(NCCDMR* cdmr);

/**************************************************/
int
NCCR_new_nc(NC** ncpp)
{
    NCCR* ncp;
    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(struct NCCR)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}

/**************************************************/
/* See ncd4dispatch.c for other version */
int
NCCR_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    NCerror ncstat = NC_NOERR;
    NC_URI* tmpurl;
    NCCR* nccr = NULL; /* reuse the ncdap3 structure*/
    NCCDMR* cdmr = NULL;
    NC_HDF5_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T *grp = NULL;
    int ncid = -1;
    int fd;
    char* tmpname = NULL;
    const char* lookups = NULL;
    bytes_t buf;
    long filetime;
    ast_err aststat = AST_NOERR;
    char* curlurl = NULL;

    LOG((1, "nc_open_file: path %s mode %d", path, mode));

    if(!nc_uriparse(path,&tmpurl)) PANIC("libcdmr: non-url path");
    nc_urifree(tmpurl); /* no longer needed */

    /* Check for legal mode flags */
    if((mode & NC_WRITE) != 0) ncstat = NC_EINVAL;
    else if(mode & (NC_WRITE|NC_CLOBBER)) ncstat = NC_EPERM;
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    mode = (mode & ~(NC_MPIIO | NC_MPIPOSIX));
    /* Despite the above check, we want the file to be initially writable */
    mode |= (NC_WRITE|NC_CLOBBER);

    /* Use NCCR code to establish a pseudo file */
    tmpname = nulldup(PSEUDOFILE);
    fd = mkstemp(tmpname);
    if(fd < 0) {THROWCHK(errno); goto done;}
    /* Now, use the file to create the hdf5 file */
    ncstat = NC4_create(tmpname,NC_NETCDF4|NC_CLOBBER,
			0,0,NULL,0,NULL,dispatch,(NC**)&nccr);
    ncid = nccr->info.ext_ncid;
    /* unlink the temp file so it will automatically be reclaimed */
    unlink(tmpname);
    free(tmpname);
    /* Avoid fill */
    dispatch->set_fill(ncid,NC_NOFILL,NULL);
    if(ncstat)
	{THROWCHK(ncstat); goto done;}
    /* Find our metadata for this file. */
    ncstat = nc4_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5);
    if(ncstat)
	{THROWCHK(ncstat); goto done;}

    /* Setup tentative NCCR state*/
    nccr->info.dispatch = dispatch;
    cdmr = (NCCDMR*)calloc(1,sizeof(NCCDMR));
    if(cdmr == NULL) {ncstat = NC_ENOMEM; goto done;}
    nccr->cdmr = cdmr;
    nccr->cdmr->controller = (NC*)nccr;
    nccr->cdmr->urltext = nulldup(path);
    nc_uriparse(nccr->cdmr->urltext,&nccr->cdmr->uri);

#ifdef WORDS_BIGENDIAN
    nccr->cdmr->controls |= BIGENDIAN;
#endif

    /* Create the curl connection (does not make the server connection)*/
    ncstat = nccr_curlopen(&nccr->cdmr->curl.curl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Turn on logging; only do this after open*/
    if(nc_urilookup(nccr->cdmr->uri,"log",&lookups)) {
	ncloginit();
        ncsetlogging(1);
        nclogopen(lookups);
    }

    if(nc_urilookup(nccr->cdmr->uri,"show",&lookups)) {
	int i;
	for(i=0;i<strlen(lookups);i++) {
	    if(lookups[i] ==  ',') continue;
	    if(strcmp("fetch",lookups+i)==0) {
	        nccr->cdmr->controls |= SHOWFETCH;
		break;
	    } else if(strcmp("datavars",lookups+i)==0) {
	        nccr->cdmr->controls |= DATAVARS;
		break;
	    }
	}
    }

    /* fetch (unconstrained) meta data */
    buf = bytes_t_null;
    curlurl = nc_uribuild(nccr->cdmr->uri,NULL,"?req=header",0);
    if(curlurl == NULL) {ncstat=NC_ENOMEM; goto done;}
    ncstat = nccr_fetchurl(nccr->cdmr,nccr->cdmr->curl.curl,curlurl,&buf,&filetime);
    free(curlurl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Parse the meta data */
    ncstat = nccr_decodeheadermessage(&buf,&cdmr->ncstreamhdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    if(buf.bytes != NULL) free(buf.bytes);

    /* Compute various things about the Header tree */

    /* Collect all nodes and fill in the CRnode part*/
    cdmr->streamnodes = nclistnew();
    ncstat = nccr_walk_Header(cdmr->ncstreamhdr,cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Compute the stream pathnames */
    ncstat = nccr_compute_pathnames(cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Map dimension references to matching declaration */
    ncstat = nccr_map_dimensions(cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Replace dimension references with matching declaration */
    nccr_deref_dimensions(cdmr->streamnodes);

    /* Collect all potential variables */
    cdmr->allvariables = nclistnew();
    ncstat = nccr_collect_allvariables(cdmr->streamnodes,cdmr->allvariables);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Deal with any constraint in the URL */
    nccr->cdmr->urlconstraint = (CCEconstraint*)ccecreate(CES_CONSTRAINT);
    if(nccr->cdmr->uri->constraint != NULL
       && strlen(nccr->cdmr->uri->constraint) > 0) {
        /* Parse url constraint to test syntactically correctness */
        ncstat = cdmparseconstraint(nccr->cdmr->uri->constraint,nccr->cdmr->urlconstraint);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
#ifdef DEBUG
fprintf(stderr,"url constraint: %s\n",
	ccetostring((CCEnode*)nccr->cdmr->uri->constraint));
#endif
    }

    ncstat = nccr_process_projections(nccr->cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = nccr_collect_projection_variables(nccr->cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = nccr_mark_visible(nccr->cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* build the netcdf-4 pseudo metadata */
    ncstat = nccr_buildnc(nccr,cdmr->ncstreamhdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Mark as no longer indef and no longer writable*/
    h5->flags &= ~(NC_INDEF);
    h5->no_write = 1;

done:
    if(aststat != AST_NOERR) {ncstat = nccr_cvtasterr(aststat);}
    if(ncstat) {
        if(nccr != NULL) {
	    int ncid = nccr->info.ext_ncid;
            NCCR_abort(ncid);
        }
    } else {
        if(ncpp) *ncpp = (NC*)nccr;
    }
    return THROW(ncstat);
}

int
NCCR_close(int ncid)
{
    NC_GRP_INFO_T *grp;
    NC_HDF5_FILE_INFO_T *h5;
    NCCR* nccr = NULL;
    int ncstat = NC_NOERR;

    LOG((1, "nc_close: ncid 0x%x", ncid));

    /* Avoid repeated close  */
    ncstat = NC_check_id(ncid, (NC**)&nccr); 
    if(ncstat != NC_NOERR) return THROW(ncstat);

    /* Find our metadata for this file. */
    ncstat = nc4_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5);
    if(ncstat != NC_NOERR) return THROW(ncstat);

    /* This must be the root group. */
    if (grp->parent) ncstat = NC_EBADGRPID;

    freeNCCDMR(nccr->cdmr);

    /* Destroy/close the NC_FILE_INFO_T state */
    NC4_abort(ncid);

    return THROW(ncstat);
}

/**************************************************/
/* Auxilliary routines                            */
/**************************************************/

static void
freeNCCDMR(NCCDMR* cdmr)
{
    if(cdmr == NULL) return;
    if(cdmr->urltext) free(cdmr->urltext);
    nc_urifree(cdmr->uri);
    if(cdmr->curl.curl) nccr_curlclose(cdmr->curl.curl);
    if(cdmr->curl.host) free(cdmr->curl.host);
    if(cdmr->curl.useragent) free(cdmr->curl.useragent);
    if(cdmr->curl.cookiefile) free(cdmr->curl.cookiefile);
    if(cdmr->curl.certificate) free(cdmr->curl.certificate);
    if(cdmr->curl.key) free(cdmr->curl.key);
    if(cdmr->curl.keypasswd) free(cdmr->curl.keypasswd);
    if(cdmr->curl.cainfo) free(cdmr->curl.cainfo);
    if(cdmr->curl.capath) free(cdmr->curl.capath);
    if(cdmr->curl.username) free(cdmr->curl.username);
    if(cdmr->curl.password) free(cdmr->curl.password);
    free(cdmr);
}

static int
nccr_collect_allvariables(NClist* streamnodes, NClist* varset)
{
    int ncstat = NC_NOERR;
    int i;
	/* A node is a variable if
	 * 1. its sort is _Variable or _Structure
	 * 2. its parent is a group
	*/
    /* Walk the set of all groups */
    for(i=0;i<nclistlength(streamnodes);i++) {
        CRnode* node = (CRnode*)nclistget(streamnodes,i);	
	if(node->sort == _Group) {
	    int j;
            Group* grp = (Group*)node;
	    /* Walk the variables */
	    for(j=0;j<grp->vars.count;j++) {
		nclistpush(varset,(ncelem)grp->vars.values[j]);		
	    }
	    /* Walk the structures */
	    for(j=0;j<grp->structs.count;j++) {
		nclistpush(varset,(ncelem)grp->structs.values[j]);
	    }
	}
    }

    /* Mark as initially invisible */
    for(i=0;i<nclistlength(varset);i++) {
        CRnode* node = (CRnode*)nclistget(varset,i);	
	node->flags.visible = 0;
    }

    return ncstat;
}

/*
1. Compute the complete set of projections
2. map projection variables to corresponding stream variable nodes
*/

static int
nccr_process_projections(NCCDMR* cdmr)
{
    int i,j;
    int ncstat = NC_NOERR;
    CCEconstraint* constraint = cdmr->urlconstraint;
    NClist* projections;
    ASSERT((constraint != NULL));
    /* Setup the constraint */
    if(constraint->projections == NULL)
	constraint->projections = nclistnew();

    /* If there are no constraints, then we will
       make every known variable be in the projection set */
    if(nclistlength(constraint->projections) == 0) {
        projections = constraint->projections;
	ASSERT((projections != NULL));
        for(i=0;i<nclistlength(cdmr->allvariables);i++) {/*Walk the set of known variables */
	    CCEprojection* newp;
	    CCEsegment* newseg;
	    CRnode* var = (CRnode*)nclistget(cdmr->allvariables,i);

	    ASSERT((var->sort == _Variable || var->sort == _Structure));

	    /* Construct a corresponding projection for this variable */
	    newp = (CCEprojection*)ccecreate(CES_PROJECT);
	    newp->decl = var;
	    newp->segments = nclistnew();
	    newseg = (CCEsegment*)ccecreate(CES_SEGMENT);
	    newseg->name = nulldup(nccr_getname(var));
	    ccemakewholesegment(newseg,var);/*treat as simple projection*/
            newseg->decl = var;
	    /* Do we need to compute crpaths? */
            nclistpush(newp->segments,(ncelem)newseg);
	    nclistpush(projections,(ncelem)newp);
	}
    } else {/* nclistlength(projections) > 0 */
        /* map projection variables to corresponding known stream variable */
        projections = constraint->projections;
        if(nclistlength(projections) > 0) {
	    int found = 0;
            for(i=0;i<nclistlength(cdmr->allvariables) && !found;i++) {
	        CRnode* cdmnode = (CRnode*)nclistget(cdmr->allvariables,i);
                for(j=0;j<nclistlength(projections);j++) {	
	            CCEprojection* p = (CCEprojection*)nclistget(projections,j);
	            if(pathmatch(p->segments,cdmnode->pathname)) {
			/* Mark the projection decl and the segment decls */
			p->decl = cdmnode;
			ncstat = nccr_mark_segment_decls(p,cdmnode);
		        found = 1;
		        break;
		    }
		}
	    }
            if(!found) {ncstat = NC_ENOTVAR; goto done;}
	}
    }
#ifdef DEBUG
{ int ix;
/* Report projection variables */
fprintf(stderr,"projections:");
for(ix=0;ix<nclistlength(projections);ix++) {
fprintf(stderr," %s",nccr_getname(((CCEprojection*)nclistget(projections,ix))->decl));
}
fprintf(stderr,"\n");
}
#endif


done:
    return ncstat;
}

static int
nccr_collect_projection_variables(NCCDMR* cdmr)
{
    int i;
    /* Walk the set of projections to collect nodes as the variable set*/
    ASSERT((cdmr->urlconstraint != NULL && cdmr->urlconstraint->projections != NULL));
    if(cdmr->variables == NULL)
	cdmr->variables = nclistnew();
    nclistclear(cdmr->variables);
    for(i=0;i<nclistlength(cdmr->urlconstraint->projections);i++) {
	CCEprojection* p = (CCEprojection*)nclistget(cdmr->urlconstraint->projections,i);
	ASSERT(p->decl != NULL);
	nclistpush(cdmr->variables,(ncelem)p->decl);
	/* Mark as visible */
	p->decl->flags.visible = 1;
    }
#ifdef DEBUG
{ int ix;
fprintf(stderr,"visible:");
for(ix=0;ix<nclistlength(cdmr->urlconstraint->projections);ix++) {
fprintf(stderr," %s",nccr_getname(((CCEprojection*)nclistget(cdmr->urlconstraint->projections,ix))->decl));
}
fprintf(stderr,"\n");
}
#endif
    return NC_NOERR;
}

static int
nccr_mark_segment_decls(CCEprojection* p, CRnode* leaf)
{
    int i;
    int ncstat = NC_NOERR;
    NClist* path = nclistnew();
    crcollectnodepath(leaf,path);    
    /* # of path nodes must be same as number of segments */
    if(nclistlength(path) != nclistlength(p->segments))
	{THROW((ncstat = NC_EINVAL)); goto done;}
    for(i=0;i<nclistlength(path);i++) {
        CRnode* elem = (CRnode*)nclistget(path,i);
	CCEsegment* seg = (CCEsegment*)nclistget(p->segments,i);
	seg->decl = elem;
    }

done:
    nclistfree(path);
    return ncstat;
}

#ifdef IGNORE
Given a new variable, say from nc_get_vara(),
Use it to 
static int
nccr_projection_restrict(NClist* varlist, NClist* projections)
{
    int ncstat = NC_NOERR;
    int i,j,len;

#ifdef DEBUG
fprintf(stderr,"collect_variables: projections=|%s|\n",
		ccelisttostring(projections, ","));
#endif

    if(nclistlength(varlist) == 0) goto done; /* nothing to add or remove */

    /* If the projection list is empty, then add
       a projection for every variable in varlist
    */
    if(nclistlength(projections) == 0) {
        NClist* path = nclistnew();
	NClist* nodeset = NULL;

        for(i=0;i<nclistlength(varlist);i++) {
	    CCEprojection* newp;
	    CRnode* var = (CRnode*)nclistget(varlist,i);
#ifdef DEBUG
fprintf(stderr,"restriction.candidate=|%s|\n",crpathstring(var->pathname));
#endif
	    newp = (CCEprojection*)ccecreate(CES_PROJECT);
	    newp->decl = var;
	    nclistclear(path);
	    crcollectnodepath(var,path);
	    newp->segments = nclistnew();
	    for(j=0;j<nclistlength(path);j++) {
	        CRnode* node = (CRnode*)nclistget(path,j);
	        CCEsegment* newseg = (CCEsegment*)ccecreate(CES_SEGMENT);
	        newseg->name = nulldup(nccr_getname(node));
	        ccemakewholesegment(newseg,node);/*treat as simple projections*/
	        newseg->decl = node;
	        nclistpush(newp->segments,(ncelem)newseg);
	    }
	    nclistpush(projections,(ncelem)newp);
	}
	nclistfree(path);
	nclistfree(nodeset);
    } else {
       /* Otherwise, walk all the projections remove duplicates,
          (WARNING) where duplicate means same node but with possibly different
          slices
	*/
	len = nclistlength(projections);
	for(i=len-1;i>=0;i--) {/* Walk backward to facilitate removal*/
	    CCEprojection* proj = (CCEprojection*)nclistget(projections,i);
	    int found;
	    for(j=0;j<nclistlength(varlist);j++) {
		CRnode* var;
		found = 0;
		var = (CRnode*)nclistget(varlist,j);
		if(var == proj->decl) {found = 1; break;}
	    }	    
	    if(found) {
		/* suppress this projection */
		CCEprojection* p = (CCEprojection*)nclistremove(projections,i);
		ccefree((CCEnode*)p);
	    }
	}
    }
    
done:
#ifdef DEBUG
fprintf(stderr,"restriction.after=|%s|\n",
		ccelisttostring(projections,","));
#endif
    return;
}
#endif

static int
nccr_mark_visible(NCCDMR* cdmr)
{
    int ncstat = NC_NOERR;
    int i,j;

    /* Mark visible variables */
    for(i=0;i<nclistlength(cdmr->variables);i++) {
	CRnode* var = (CRnode*)nclistget(cdmr->variables,i);
	CRshape shape;
	var->flags.visible = 1;
        /* For each variable, mark any dimensions it uses as also visible */
	crextractshape(var,&shape);
	for(j=0;j<shape.rank;j++) {
	    Dimension* dim = shape.dims[j];
	    ((CRnode*)dim)->flags.visible = 1;
	}
    }
    return ncstat;
}
