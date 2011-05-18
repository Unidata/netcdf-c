/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <curl/curl.h>
#include "curlwrap.h"

#include "ncbytes.h"
#include "nclist.h"
#include "nclog.h"

#include "netcdf.h"
#include "nc.h"
#include "nc4internal.h"
#include "ncdispatch.h"
#include "nc4dispatch.h"

#include "nccr.h"
#include "crdebug.h"
#include "nccrdispatch.h"
#include "ast.h"
#include "nccrnode.h"
#include "ncStreamx.h"
#include "nccrproto.h"
#include "cceconstraints.h"
#include "nccrmeta.h"

/* Mnemonic */
#define getncid(drno) (((NC*)drno)->ext_ncid)

extern NC_FILE_INFO_T* nc_file;

static void freeNCCDMR(NCCDMR* cdmr);
static int nccr_compute_projection_names(NCCDMR* cdmr);
static int nccr_map_projections(NCCDMR* cdmr);

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
    Header* hdr = NULL;
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
	    }
	}
    }

    /* fetch (unconstrained) meta data */
    buf = bytes_t_null;
    curlurl = nc_uribuild(nccr->cdmr->uri,NULL,"?req=header",0);
    if(curlurl == NULL) {ncstat=NC_ENOMEM; goto done;}
    ncstat = nccr_fetchurl(nccr->cdmr->curl.curl,curlurl,&buf,&filetime);
    free(curlurl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Parse the meta data */
    ncstat = nccr_decodeheader(&buf,&hdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    if(buf.bytes != NULL) free(buf.bytes);

    /* Compute various things about the Header tree */

    /* Collect all nodes and fill in the CRnode part*/
    cdmr->streamnodes = nclistnew();
    ncstat = nccr_walk_Header(hdr,cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Compute the stream pathnames */
    ncstat = nccr_compute_pathnames(cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Map dimension references to matching declaration */
    ncstat = nccr_map_dimensions(cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Replace dimension references with matching declaration */
    nccr_deref_dimensions(cdmr->streamnodes);

    /* Deal with any constraint in the URL */
    nccr->cdmr->urlconstraint = (CCEconstraint*)ccecreate(CES_CONSTRAINT);
    /* Parse url constraint to test syntactically correctness */
    ncstat = cdmparseconstraint(nccr->cdmr->uri->constraint,nccr->cdmr->urlconstraint);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
#ifdef DEBUG
fprintf(stderr,"url constraint: %s\n",
	ccetostring((CCEnode*)nccr->cdmr->urlconstraint));
#endif

    /* Compute the projection pathnames */
    ncstat = nccr_compute_projection_names(nccr->cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* map url projection variables to corresponding stream variable nodes,
       and collect set of so mapped variables */
    ncstat = nccr_map_projections(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* build the netcdf-4 pseudo metadata */
    ncstat = nccr_buildnc(nccr,hdr);
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

/* Compute the projection pathnames */
static int
nccr_compute_projection_names(NCCDMR* cdmr)
{
    int i,j;
    CCEconstraint* constraint = cdmr->urlconstraint;
    NCbytes* pathname = ncbytesnew();
    if(constraint == NULL) {
	constraint = (CCEconstraint*)ccecreate(CES_CONSTRAINT);
	cdmr->urlconstraint = constraint;
    }
    if(constraint->projections == NULL)
	constraint->projections = nclistnew();
    for(i=0;i<nclistlength(constraint->projections);i++) {	
	CCEprojection* p = (CCEprojection*)nclistget(constraint->projections,i);
	ncbytesclear(pathname);
	for(j=0;j<nclistlength(p->segments);j++) {
	    CCEsegment* seg = (CCEsegment*)nclistget(p->segments,j);
	    if(j > 0) ncbytescat(pathname,".");
	    ncbytescat(pathname,seg->name);
	}
	p->pathname = ncbytesextract(pathname);
    }
    return NC_NOERR;
}

static int
nccr_map_projections(NCCDMR* cdmr)
{
    int ncstat = NC_NOERR;
    int i,j;
    NClist* projections = cdmr->urlconstraint->projections;
    cdmr->variables = nclistnew(); /* variables in the url constraint */
    if(nclistlength(projections) > 0) {
        for(i=0;i<nclistlength(cdmr->streamnodes);i++) {
	    int found = 0;
	    CRnode* cdmnode = (CRnode*)nclistget(cdmr->streamnodes,i);
            for(j=0;j<nclistlength(projections);j++) {	
	        CCEprojection* p = (CCEprojection*)nclistget(projections,j);
	        if(strcmp(p->pathname,cdmnode->pathname)==0) {
		    nclistpush(cdmr->variables,(ncelem)cdmnode);
		    found = 1;
		}
	    }
	    if(!found) {ncstat = NC_ENOTVAR; goto done;}
	}
    }

done:
    return ncstat;
}

