/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <curl/curl.h>
#include "curlwrap.h"

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc.h"
#include "nc4internal.h"

#include "nccr.h"
#include "crdebug.h"
#include "ast.h"

int 
NCCR_get_vara(int ncid, int varid,
	      const size_t* startp,
	      const size_t* countp,
	      void* data,
	      nc_type externaltype0)
{
    NCerror ncstat = NC_NOERR;
#ifdef IGNORE
    OCerror ocstat = OC_NOERR;
    unsigned int i;
    NC_GRP_INFO_T *grp; 
    NC_HDF5_FILE_INFO_T *h5;
    NC_VAR_INFO_T *var;
    NCCR* nccr;
    CDFnode* cdfvar; /* cdf node mapping to var*/
    NClist* varnodes;
    CRGetvara* varainfo = NULL;
    char* constraint = NULL;
    NCprojection* varaprojection = NULL;
    NCcachenode* cachenode = NULL;
    nc_type externaltype = externaltype0;
    NCCDMR* cdmr;

    LOG((2, "nccr_get_vara: ncid 0x%x varid %d", ncid, varid));

    if((ncstat = nc4_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5)))
	{THROWCHK(ncstat); goto fail;}

    cdmr = nccr->cdmr;

    /* Find the netcdf-4 var structure for this varid */
    for(i=0;i<

    

    for(var=grp->var;var!=NULL;var=var->next) {
	if (var->varid == varid) break;
    }
    if(var == NULL) {ncstat = NC_ENOTVAR; goto fail;}

    /* Find cdfnode corresponding to the var.*/
    varnodes = nccomm->cdf.varnodes;
    cdfvar = NULL;
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(varnodes,i);
	if(node->ncid == varid) {
	    cdfvar = node;
	    break;
	}
    }
    ASSERT((cdfvar != NULL));
    ASSERT((strcmp(cdfvar->ncfullname,var->name)==0));

#ifdef DEBUG
{ NClist* dims = cdfvar->array.dimensions;
fprintf(stderr,"getvarx: %s/%d",cdfvar->ncfullname,(int)nclistlength(dims));
if(nclistlength(dims) > 0) {int i;
for(i=0;i<nclistlength(dims);i++) 
fprintf(stderr,"[%lu:%lu]",(unsigned long)startp[i],(unsigned long)countp[i]);
fprintf(stderr," -> ");
for(i=0;i<nclistlength(dims);i++) 
fprintf(stderr,"[%lu:%lu]",(unsigned long)startp[i],(unsigned long)((startp[i]+countp[i])-1));
}
fprintf(stderr,"\n");
}
#endif

    /* Default to using the var type */
    if(externaltype == NC_NAT) externaltype = cdfvar->externaltype;

    /* Validate any implied type conversion*/
    if(cdfvar->nctype == NC_Primitive
       && cdfvar->etype != externaltype && externaltype == NC_CHAR) {
	/* The only disallowed conversion is to/from char and non-byte
           numeric types*/
	switch (cdfvar->etype) {
	case NC_STRING: case NC_URL:
	case NC_CHAR: case NC_BYTE: case NC_UBYTE:
	    break;
	default:
	    THROWCHK(NC_ECHAR);
	    goto fail;
	}
    }

    ncstat = makegetvar34(nccomm,cdfvar,data,externaltype,&varainfo);
    if(ncstat) {THROWCHK(NC_ENOMEM); goto fail;}
#ifdef IGNORE
    freegetvara(nccomm->vara);
    nccomm->vara = varainfo;
#endif

    ncstat = buildvaraprojection4(varainfo,
				  startp,countp,dapsinglestride3,
			          &varaprojection);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    if(FLAGSET(drno->dap.controls,NCF_UNCONSTRAINABLE)) {
#ifdef DEBUG
fprintf(stderr,"Unconstrained: reusing prefetch\n");
#endif
	cachenode = nccomm->cdf.cache->prefetch;
	ASSERT((cachenode != NULL));
    } else if(iscached(&drno->dap,varaprojection->var->leaf,&cachenode)) {
#ifdef DEBUG
fprintf(stderr,"Reusing cached fetch constraint: %s\n",
	dumpconstraint(cachenode->constraint));
#endif
    } else { /* Load with constraints */
	NClist* vars = nclistnew();
	NCconstraint* constraint;
	nclistpush(vars,(ncelem)varainfo->target);

	constraint = createncconstraint();
        constraint->projections = clonencprojections(nccomm->oc.dapconstraint->projections);
        if(!FLAGSET(drno->dap.controls,NCF_CACHE)) {
	    /* If we are not caching, then merge the getvara projections */
	    NClist* tmp = nclistnew();
	    NCprojection* clone = clonencprojection(varaprojection);
	    nclistpush(tmp,(ncelem)clone);
            ncstat = mergeprojections3(constraint->projections,tmp);
	    nclistfree(tmp);
	    freencprojection(clone);
            if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
#ifdef DEBUG
fprintf(stderr,"vara merge: %s\n",
	dumpprojections(constraint->projections));
#endif
        }

        restrictprojection34(vars,constraint->projections);
        constraint->selections = clonencselections(nccomm->oc.dapconstraint->selections);

	/* buildcachenode3 will also fetch the corresponding datadds */
        ncstat = buildcachenode34(nccomm,constraint,vars,&cachenode,0);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
#ifdef DEBUG
fprintf(stderr,"cache.datadds=%s\n",dumptree(cachenode->datadds));
#endif
    }

    /* attach DATADDS to DDS */
    unattach34(nccomm->cdf.ddsroot);
    ncstat = attachsubset34(cachenode->datadds,nccomm->cdf.ddsroot);
    if(ncstat) goto fail;	

    /* Fix up varainfo to use the cache */
    varainfo->cache = cachenode;
    /* However use this current vara projection */
    varainfo->varaprojection = varaprojection;
    varaprojection = NULL;

    /* Now, get the relevant instance*/

    target = varainfo->target;
    /* xtarget is in the datadds space */
    xtarget = target->attachment;
    if(xtarget == NULL) 
	{THROWCHK(ncstat=NC_ENODATA); goto fail;}
    /* Switch to datadds tree space*/
    varainfo->target = xtarget;
    ncstat = getcontent4(drno,varainfo,cachenode->datadds,data);
    if(ncstat != OC_NOERR) {THROWCHK(ncstat); goto fail;}
    goto ok;
fail:
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
ok:
    efree(constraint);
    freegetvara(varainfo);
    freencprojection(varaprojection);
#endif
    return THROW(ncstat);
}
