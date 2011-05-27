/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <curl/curl.h>

#include "nclist.h"
#include "ncbytes.h"

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc.h"
#include "nc4internal.h"

#include "ast.h"

#include "nccr.h"
#include "nccrdispatch.h"
#include "nccrnode.h"
#include "cceconstraints.h"
#include "nccrgetvarx.h"
#include "crdebug.h"
#include "curlwrap.h"
#include "ncStreamx.h"
#include "nccrmeta.h"
#include "nccrproto.h"
#include "nccrgetvarx.h"


#define DATAPREFIX "?req=data&var="


/* Forward */
static int makegetvar(NCCDMR*, CRnode*, void*, nc_type, NCCRgetvarx**);

static int buildvarxprojection(NCCRgetvarx* varxinfo,
		     const size_t* startp,
                     const size_t* countp,
		     const ptrdiff_t* stridep,
		     CCEprojection** projectionp);


static int crfetchdata(NCCDMR* cdmr, char* p, bytes_t* bufp, long* filetimep);

static int nccr_getcontent(NCCCDMR* cdmr, NCCRgetvarx* varxinfo, void* data);

/**************************************************/
int 
NCCR_getvarx(int ncid, int varid,
	      const size_t* startp,
	      const size_t* countp,
	      const ptrdiff_t* stridep,
	      void* data,
	      nc_type externaltype0)
{
    int ncstat = NC_NOERR;
    unsigned int i;
    NC_GRP_INFO_T *grp; 
    NC_HDF5_FILE_INFO_T *h5;
    NC_VAR_INFO_T *var;
    NCCR* nccr;
    CRnode* cdfvar; /* cdf node mapping to var*/
    NClist* varnodes;
    NCCRgetvarx* varxinfo = NULL;
    char* constraintstring = NULL;
    CCEprojection* varxprojection = NULL;
    NCCDMR* cdmr;
    size_t localcount[NC_MAX_VAR_DIMS];
    NClist* vars = NULL;
    CCEconstraint* constraint = NULL;
    NClist* tmp = NULL;
    CCEprojection* clone = NULL;
    DataType datatype;
    nc_type internaltype;
    nc_type externaltype = externaltype0;
    CRshape shape;
    bytes_t buf;
    char* projectionstring = NULL;

    LOG((2, "nccr_get_varx: ncid 0x%x varid %d", ncid, varid));

    if((ncstat = nc4_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5)))
	{THROWCHK(ncstat); goto fail;}

    cdmr = nccr->cdmr;

    /* Find the netcdf-4 var structure for this varid */
    for(var=grp->var;var!=NULL;var=var->next) {
	if (var->varid == varid) break;
    }
    if(var == NULL) {ncstat = NC_ENOTVAR; goto fail;}

    ASSERT((cdfvar != NULL));
    ASSERT((cdfvar->sort == _Variable || cdfvar->sort == _Structure));

    /* Get the dimension info and typing */
    if(cdfvar->sort == _Variable) {
        Variable* var = (Variable*)cdfvar;
	crextractshape((CRnode*)var,&shape);
	datatype = var->dataType;
    } else if(cdfvar->sort == _Structure) {
        Structure* var = (Structure*)cdfvar;
	crextractshape((CRnode*)var,&shape);
	datatype = var->dataType;
    }

    /* Fill in missing arguments */
    if(startp == NULL)
	startp = nccrzerostart;

    if(countp == NULL) {
        /* Accumulate the dimension sizes */
        for(i=0;i<shape.rank;i++) {
	    Dimension* dim = shape.dims[i];
	    localcount[i] = dimsize(dim);
	}
	countp = localcount;
    }

    if(stridep == NULL)
	stridep = nccrsinglestride;

    /* Validate the dimension sizes */
    for(i=0;i<shape.rank;i++) {
        Dimension* dim = shape.dims[i];
	if(startp[i] > dimsize(dim)
	   || startp[i]+countp[i] > dimsize(dim)) {
	    ncstat = NC_EINVALCOORDS;
	    goto fail;	    
	}
    }	     

    /* Default to using the var type */
    internaltype = cvtstreamtonc(datatype);
    if(externaltype == NC_NAT) externaltype = internaltype;

    /* Validate any implied type conversion*/
    if(internaltype != externaltype && externaltype == NC_CHAR) {
	/* The only disallowed conversion is to/from char and non-byte
           numeric types*/
	switch (internaltype) {
	case NC_STRING:
	case NC_CHAR: case NC_BYTE: case NC_UBYTE:
	    break;
	default:
	    THROWCHK(NC_ECHAR);
	    goto fail;
	}
    }

    /* Find protobuf node corresponding to the var.*/
    varnodes = cdmr->variables;
    varnodes = NULL;
    for(i=0;i<nclistlength(varnodes);i++) {
	CRnode* node = (CRnode*)nclistget(varnodes,i);
	if(node->ncid == varid) {
	    cdfvar = node;
	    break;
	}
    }

    ncstat = makegetvar(cdmr,cdfvar,data,externaltype,&varxinfo);
    if(ncstat) {THROWCHK(NC_ENOMEM); goto fail;}

    ncstat = buildvarxprojection(varxinfo,
				  startp,countp,nccrsinglestride,
			          &varxprojection);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Load with constraints */
    vars = nclistnew();
    nclistpush(vars,(ncelem)varxinfo->target);

    constraint = (CCEconstraint*)ccecreate(CES_CONSTRAINT);
    constraint->projections = cceclonelist(cdmr->urlconstraint->projections);
    /* merge the getvarx projections */
    tmp = nclistnew();
    clone = (CCEprojection*)cceclone((CCEnode*)varxprojection);
    nclistpush(tmp,(ncelem)clone);
    ncstat = ccemergeprojections(constraint->projections,tmp);
    nclistfree(tmp);
    ccefree((CCEnode*)clone);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
#ifdef DEBUG
fprintf(stderr,"varx merge: %s\n",
	ccelisttostring(constraint->projections,","));
#endif
    ccerestrictprojection(vars,constraint->projections);

    /* Convert the projections into a string for use
       with crfetchdata*/
    
    projectionstring = ccelisttostring(constraint->projections,",");

    /* Fetch */

    buf = bytes_t_null;
    ncstat = crfetchdata(cdmr,projectionstring,&buf,NULL);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    
    if(projectionstring) free(projectionstring);

    /* Parse the data header */
    ncstat = nccr_decodedataheader(&buf,&cdmr->datahdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Map the variables in the data header to those in the ncstreamdata */
    ncstat = nccr_mapdataheader(cdmr,cdmr->ncstreamhdr,cdmr->datahdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    
    if(buf.bytes != NULL) free(buf.bytes);

    /* Use this current varx projection */
    varxinfo->projection = varxprojection;
    varxprojection = NULL;

    /* move the data into user's memory */
    ncstat = nccr_getcontent(cdmr,varxinfo,data);
    if(ncstat != OC_NOERR) {THROWCHK(ncstat); goto fail;}
    goto ok;

done:
    efree(constraint);
    freegetvarx(varxinfo);
    ccefree((CCEnode*)varxprojection);
    return THROW(ncstat);
}

static int
makegetvar(NCCDMR* cdmr, CRnode* var, void* data, nc_type dsttype, Getvarx** getvarp)
{
    Getvarx* getvar;
    int ncstat = NC_NOERR;

    getvar = (Getvarx*)emalloc(sizeof(Getvarx));
    MEMCHECK(getvar,NC_ENOMEM);
    memset((void*)getvar,0,sizeof(Getvarx));
    if(getvarp) *getvarp = getvar;

    getvar->target = var;
    getvar->memory = data;
    getvar->dsttype = dsttype;
    getvar->target = var;
    if(ncstat) efree(getvar);
    return ncstat;
}

/* Convert an DCEprojection instance into a string
   that can be used with the url
*/

static char*
crprojectionstring(NClist* projections)
{
    char* pstring NULL;
    NCbytes* buf = ncbytesnew();
    ccelisttobuffer(projections,buf,",");
    if(ncbyteslength(buf) > 0)
        pstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return pstring;
}

static int
crfetch(NCDAPCOMMON* nccomm,
        CCEconstraint* constraint,
	NClist* varlist)
{
    int ncstat = NC_NOERR;
    char* ce = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
        ce = NULL;
    else {
        ce = crconstraintstring(constraint);
    }

    /* Fetch the relevant data */
    if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
	if(ce == NULL)
	    nclog(NCLOGNOTE,"fetch: %s.%s",nccomm->oc.uri->uri,ext);
	else
	    nclog(NCLOGNOTE,"fetch: %s.%s?%s",nccomm->oc.uri->uri,ext,ce);
    }
    ocstat = oc_fetch(conn,ce,dxd,rootp);
    if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
	nclog(NCLOGNOTE,"fetch complete.");
    }
    return ocstat;



    ocstat = dap_oc_fetch(nccomm,conn,ce,OCDATADDS,&ocroot);
    nullfree(ce);
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
    cachenode->constraint = constraint;
    cachenode->wholevariable = iswholeconstraint(cachenode->constraint);

    /* save the root content*/
    cachenode->ocroot = ocroot;
    cachenode->content = oc_data_new(conn);
    ocstat = oc_data_root(conn,ocroot,cachenode->content);
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
	while(cache->cachesize + cachenode->xdrsize > cache->cachelimit) {
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
        nclistpush(nccomm->cdf.cache->nodes,(ncelem)cachenode);
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

/* In order to construct the projection,
we need to make sure to match the relevant dimensions
against the relevant nodes in which the ultimate target
is contained.
*/
static int
buildvarxprojection(NCCRgetvarx* varxinfo,
		     const size_t* startp,
                     const size_t* countp,
		     const ptrdiff_t* stridep,
		     CCEprojection** projectionp)
{
    int i;
    int ncstat = NC_NOERR;
    CRnode* var = varxinfo->target;
    CCEprojection* projection = NULL;
    NClist* segments = NULL;
    CCEsegment* segment;
    CRshape shape;

    segment = (CCEsegment*)ccecreate(CES_SEGMENT);
    segment->node = var;
    ASSERT((segment->node != NULL));
    segment->name = nulldup(segment->node->name);
    segment->slicesdefined = 0; /* temporary */
    segment->slicesdeclized = 0; /* temporary */
    segments = nclistnew();
    nclistpush(segments,(ncelem)segment);

    projection = (CCEprojection*)ccecreate(CES_PROJECT);
    projection->decl = ?;

    /* All slices are assigned to the first (and only segment) */
    
    crextractshape((CRnode*)var,&shape);
    segment->rank = shape.rank;
    for(i=0;i<segment->rank;i++) { 
        DCEslice* slice = &segment->slices[i];
	Dimension* dim = shape.dims[i];
        slice->first = startp[i];
	slice->stride = stridep[i];
	slice->count = countp[i];
        slice->length = slice->count * slice->stride;
	slice->stop = (slice->first + slice->length);
	ASSERT(dimsize(dim) > 0);
    	slice->declsize = dimsize(dim);
    }
    segment->slicesdefined = 1;
    segment->slicesdeclized = 1;

    if(projectionp) *projectionp = projection;
    if(ncstat) ccefree((DCEnode*)projection);
    return ncstat;
}

static int
crfetchdata(NCCDMR* cdmr, char* projection, bytes_t* bufp, long* filetimep)
{
    int ncstat = NC_NOERR;
    char* curlurl;
    long filetime;
    bytes_t buf;
    char* fullprojection;
    size_t len;

    /* Construct the complete projection suffix */
    len = strlen(projection);
    len += strlen(DATAPREFIX);
    len++; /* null terminator */
    fullprojection = (char*) malloc(len);
    if(fullprojection == NULL) {ncstat=NC_ENOMEM; goto done;}
    strcpy(fullprojection,DATAPREFIX);
    strcat(fullprojection,projection);

    /* fetch data */
    buf = bytes_t_null;
    curlurl = nc_uribuild(cdmr->uri,NULLfullprojection",0);
    free(fullprojection);
    if(curlurl == NULL) {ncstat=NC_ENOMEM; goto done;}
    ncstat = nccr_fetchurl(cdmr->curl.curl,curlurl,&buf,&filetime);
    free(curlurl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    if(filetimep) *filetimep = filetime;
    if(bufp) *bufp = buf;
    
    return ncstat;
}

/*
Assumptions:
1. there is only 1 projection variable in response

Not-Assumed:
*/


static int
nccr_getcontent(NCCCDMR* cdmr, NCCRgetvarx* varxinfo, void* data)
{
    int ncstat = NC_NOERR;
    int i,j;
    CCEprojection* proj = varxinfo->projection;
    /* Match against the Data.varName */
    	
	CRnode* var = locatevar(proj,
	
    }
    
done:
    return ncstat;    
}
