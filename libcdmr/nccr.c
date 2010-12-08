#include "nccr.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "nccr.h"
#include "nccrdispatch.h"
#include "nc4dispatch.h"
#include "crdebug.h"

/* Mnemonic */
#define getncid(drno) (((NC*)drno)->ext_ncid)

extern NC_FILE_INFO_T* nc_file;

extern NCerror cleanNCCR(NCCDMR* drno);

static void nccrdinitialize(void);
#ifdef IGNORE
static NCerror buildnccr(NCCR* drno);
static NCerror builddims4(NCCR*);
static NCerror buildtypes4(NCCR*);
static NCerror buildtypes4r(NCCR* drno, CDFnode* tnode);
static NCerror buildvars4(NCCR*);
static NCerror buildglobalattrs4(NCCR*, int, CDFnode* root);
static NCerror buildattribute4a(NCCR* drno, NCattribute* att, int varid, int ncid);
static NCerror showprojection4(NCDAPCOMMON* nccomm, CDFnode* var);
static size_t estimatesizes4r(NCDAPCOMMON* nccomm, CDFnode* node);
static void estimatesizes4(NCDAPCOMMON* nccomm);
static NCerror fixzerodims4(NCDAPCOMMON* nccomm);
static NCerror fixzerodims4r(NCDAPCOMMON* nccomm, CDFnode* node);
static NCerror cvtunlimiteddim(NCDAPCOMMON* nccomm, CDFnode* dim);
static void applyclientparamcontrols4(NCDAPCOMMON* nccomm);
#endif

static int nccrdinitialized = 0;

/**************************************************/
int
NCD4_new_nc(NC** ncpp)
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
NCD4_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    NCerror ncstat = NC_NOERR;
    NC_URL* tmpurl;
    NCCR* nccr = NULL; /* reuse the ncdap3 structure*/
    NC_HDF5_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T *grp = NULL;
    int ncid = -1;
    const char* value;
    int fd;
    char* tmpname = NULL;

    LOG((1, "nc_open_file: path %s mode %d", path, mode));

    if(!nccrdinitialized) nccrdinitialize();

    if(!nc_urlparse(path,&tmpurl)) PANIC("libcdmr: non-url path");
    nc_urlfree(tmpurl); /* no longer needed */

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
    nccr->cdmr->controller = (NC*)nccr;
    nccr->cdmr->urltext = nulldup(path);
    nc_urlparse(nccr->cdmr->urltext,&nccr->cdmr->url);
    nccr->info.dispatch = dispatch;

    /* Create the curl connection */
    ncstat = nc_curlopen(nccr->cdmr->curl.curl);
    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto done;}

 X   if(nc_urllookup(nccr->cdmr->url,"show","fetch"))
	SETFLAG(nccr->cdmr->controls,NCF_SHOWFETCH);

    /* Turn on logging */
    value = oc_clientparam_get(nccr->cdmr->oc.conn,"log");
    if(value != NULL) {
	oc_loginit();
        oc_setlogging(1);
        oc_logopen(value);
    }

    /* fetch and build the unconstrained DDS */
    ncstat = fetchtemplatemetadata3(&nccr->dap);
    if(ncstat != NC_NOERR) goto done;

    /* Process the constraints to map the CDF tree */
    ncstat = mapconstraints3(&nccr->dap);
    if(ncstat != NC_NOERR) goto done;

    /* fetch and build the constrained DDS */
    ncstat = fetchconstrainedmetadata3(&nccr->dap);
    if(ncstat != NC_NOERR) goto done;

    /* The following actions are WRT to the constrained tree */

    /* Accumulate useful nodes sets  */
    ncstat = computecdfnodesets4(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Fix grids */
    ncstat = fixgrids4(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* apply client parameters (after computcdfinfo and computecdfvars)*/
    ncstat = applyclientparams34(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Accumulate the nodes representing user types*/
    ncstat = computeusertypes4(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the type names*/
    ncstat = shortentypenames4(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the dimension names*/
    ncstat = computecdfdimnames34(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* deal with zero-size dimensions */
    ncstat = fixzerodims4(&nccr->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Estimate the variable sizes */
    estimatesizes4(&nccr->dap);

    ncstat = buildnccr(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Do any necessary data prefetch */
    ncstat = prefetchdata3(&nccr->dap);
    if(ncstat != NC_NOERR)
	{THROWCHK(ncstat); goto done;}

    /* Mark as no longer indef and no longer writable*/
    h5->flags &= ~(NC_INDEF);
    h5->no_write = 1;

done:
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    if(ncstat) {
        if(cdmr != NULL) {
	    int ncid = cdmr->info.ext_ncid;
            freeNCCR(cdmr);
            NCCR_abort(ncid);
        }
    } else {
        if(ncpp) *ncpp = (NC*)cdmr;
    }
    return THROW(ncstat);
}

int
NCD4_close(int ncid)
{
    NC_GRP_INFO_T *grp;
    NC_HDF5_FILE_INFO_T *h5;
    NCCR* cdmr = NULL;
    int ncstat = NC_NOERR;

    LOG((1, "nc_close: ncid 0x%x", ncid));
    /* Find our metadata for this file. */
    ncstat = nccr_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5);
    if(ncstat != NC_NOERR) return THROW(ncstat);

    /* This must be the root group. */
    if (grp->parent) ncstat = NC_EBADGRPID;

    oc_logclose();

    /* Destroy/close the NCCR state */
    cleanNCCR(cdmr);
    NCCR_abort(ncid);

    return THROW(ncstat);
}

/**************************************************/
/* Auxilliary routines                            */
/**************************************************/

static void
nccrdinitialize()
{
    nc3dinitialize();
    nccrdinitialized = 1;
}

NCerror
cleanNCCR(NCCDMR* cdmr)
{
    return cleanNCDAPCOMMON(&nccr->dap);
}

/*
Note: never use any of the libncdap3 code to call
netcdf API functions because it will use the netcdf-3 API.
*/
static NCerror
buildnccr(NCCR* cdmr)
{
    NCerror ncstat = NC_NOERR;
    CDFnode* dds = nccr->cdmr->cdf.ddsroot;
    ncstat = buildglobalattrs4(cdmr,getncid(cdmr),dds);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = builddims4(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = buildtypes4(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = buildvars4(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
done:
    return THROW(ncstat);
}

/* Define dim info for top-level dims */
static NCerror
builddims4(NCCR* cdmr)
{
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;
    int dimid;
    int ncid = getncid(cdmr);
    NClist* dimset = nclistnew();

    /* collect all dimensions from variables,
       including duplicates; note we use array.dimensions
       not array.ncdimensions.
    */
    for(i=0;i<nclistlength(nccr->cdmr->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccr->cdmr->cdf.varnodes,i);
        if(!var->visible) continue;
	nclistextend(dimset,nclistlength(var->array.dimensions));
        for(j=0;j<nclistlength(var->array.dimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(var->array.dimensions,j);
	    int k,inserted = 0;
            /* Sort by fullname just for the fun of it */
            for(k=0;k<nclistlength(dimset);k++) {
	        CDFnode* kdim = (CDFnode*)nclistget(dimset,k);
		if(strcmp(kdim->ncfullname,dim->ncfullname) > 0) {
		    nclistinsert(dimset,k,(ncelem)dim);
		    inserted = 1;
		    break;
		}
	    }
	    if(!inserted) nclistpush(dimset,(ncelem)dim);
	}
    }

    /* Define those top-level dims */
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	if(dim->dim.basedim != NULL) continue;
        ncstat = nc_def_dim(ncid,dim->ncfullname,dim->dim.declsize,&dimid);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
        dim->ncid = dimid;
    }

    /* Make all duplicate dims have same dimid as basedim*/
    /* (see computecdfdimnames)*/
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        if(dim->dim.basedim != NULL) {
	    dim->ncid = dim->dim.basedim->ncid;
	}
    }
/*ok:*/
done:
    nclistfree(dimset);
    return THROW(ncstat);
}

static NCerror
buildtypes4(NCCR* cdmr)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;

    /* Define user types in postorder */
    for(i=0;i<nclistlength(nccr->cdmr->cdf.usertypes);i++) {
	CDFnode* node = (CDFnode*)nclistget(nccr->cdmr->cdf.usertypes,i);
	if(!node->visible) continue;
	ncstat = buildtypes4r(cdmr,node);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
done:
    return THROW(ncstat);
}

static NCerror
buildtypes4r(NCCR* cdmr, CDFnode* tnode)
{
    unsigned int i,j;
    int typeid;
    NCerror ncstat = NC_NOERR;

    if(!tnode->visible) goto done;

    switch (tnode->nctype) {
    case NC_Sequence:
	/* Look for sequences that have a single field whose
           type is primitive; for these, we will not generate
           the compound type.
        */
	if(tnode->singleton) {
	    /* ok, just generate the vlen type using the sequence's
               singleton field */
	    /* Find the first primitive visible field */
	    CDFnode* prim = getsingletonfield(tnode->subnodes);
	    ASSERT((prim != NULL));
	    ncstat = nc_def_vlen(getncid(cdmr),tnode->vlenname,
                                     prim->etype,&typeid);
	    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    tnode->basetypeid = prim->etype;
	    tnode->typeid = typeid;
	    break;
	}
        /* fall thru */
    case NC_Grid:
    case NC_Structure:
	ncstat = nc_def_compound(getncid(cdmr),tnode->typesize.instance.size,
				tnode->typename,&typeid);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	tnode->typeid = typeid;
	for(i=0;i<nclistlength(tnode->subnodes);i++) {
	    CDFnode* field = (CDFnode*)nclistget(tnode->subnodes,i);
	    if(!field->visible) continue;
	    if(nclistlength(field->array.dimensions) == 0) {
   	        ncstat = nc_insert_compound(getncid(cdmr),typeid,
					field->ncbasename,
				        field->typesize.field.offset,
					field->typeid);
	        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    } else {
	        int dimsizes[NC_MAX_VAR_DIMS];
	        for(j=0;j<nclistlength(field->array.dimensions);j++) {
		    CDFnode* dim=(CDFnode*)nclistget(field->array.dimensions,j);
		    dimsizes[j] = dim->dim.declsize;
		}
   	        ncstat = nc_insert_array_compound(getncid(cdmr),typeid,
					field->ncbasename,
				        field->typesize.field.offset,
					field->typeid,
					nclistlength(field->array.dimensions),
                                        dimsizes);
	        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    }
	}
	/* If the node is a sequence, also define the corresponding vlen type*/
	if(tnode->nctype == NC_Sequence) {
	    ncstat = nc_def_vlen(getncid(cdmr),tnode->vlenname,tnode->typeid,&typeid);
	    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    tnode->basetypeid = typeid;
	    tnode->typeid = typeid;
	}
        break;

    case NC_Primitive:
	break;

    default: PANIC1("unexpected nctype: %d",tnode->nctype);
    }

done:
    return THROW(ncstat);
}

/* Simultaneously build any associated attributes */
static NCerror
buildvars4(NCCR* cdmr)
{
    /* Variables (in this translation) are (mostly)
       the direct fields of the Dataset*/
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;
    int varid;
    int ncid = getncid(cdmr);

    for(i=0;i<nclistlength(nccr->cdmr->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccr->cdmr->cdf.varnodes,i);
	NClist* vardims = var->array.dimensions;
	int dimids[NC_MAX_VAR_DIMS];
	int ncrank,dimindex=0;

	if(!var->visible) continue;

	ncrank = nclistlength(vardims);
	if(ncrank > 0) {
	    dimindex = 0;
            for(j=0;j<ncrank;j++) {
                CDFnode* dim = (CDFnode*)nclistget(vardims,j);
                dimids[dimindex++] = dim->ncid;
 	    }
        }   
	setvarbasetype(&nccr->dap,var);
	ASSERT((var->typeid > 0));
        ncstat = nc_def_var(getncid(cdmr),var->ncfullname,
			    var->typeid,
                            nclistlength(var->array.dimensions),
                            (ncrank==0?NULL:dimids),
                            &varid);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	var->ncid = varid;
	if(var->attributes != NULL) {
	    for(j=0;j<nclistlength(var->attributes);j++) {
		NCattribute* att = (NCattribute*)nclistget(var->attributes,j);
		ncstat = buildattribute4a(cdmr,att,varid,ncid);
        	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    }
	}
	/* Tag the variable with its DAP path */
	if(paramcheck34(&nccr->dap,"show","projection"))
	    showprojection4(&nccr->dap,var);
    }
    
done:
    return THROW(ncstat);
}

static NCerror
buildglobalattrs4(NCCR* cdmr, int ncid, CDFnode* root)
{
    int i;
    const char* txt;
    char *nltxt, *p;
    NCerror ncstat = NC_NOERR;

    if(root->attributes != NULL) {
        for(i=0;i<nclistlength(root->attributes);i++) {
   	    NCattribute* att = (NCattribute*)nclistget(root->attributes,i);
	    ncstat = buildattribute4a(cdmr,att,NC_GLOBAL,ncid);
            if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	}
    }

    /* Define some additional system global attributes depending
       on show= clientparams*/
    /* Ignore doneures*/

    if(paramcheck34(&nccr->dap,"show","translate")) {
        /* Add a global attribute to show the translation */
        ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_translate",
	           strlen("netcdf-4"),"netcdf-4");
    }

    if(paramcheck34(&nccr->dap,"show","url")) {
	if(nccr->cdmr->oc.urltext != NULL)
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_url",
				       strlen(nccr->cdmr->oc.urltext),nccr->cdmr->oc.urltext);
    }
    if(paramcheck34(&nccr->dap,"show","dds")) {
	txt = NULL;
	if(nccr->cdmr->cdf.ddsroot != NULL)
  	    txt = oc_inq_text(nccr->cdmr->oc.conn,nccr->cdmr->cdf.ddsroot->dds);
	if(txt != NULL) {
	    /* replace newlines with spaces*/
	    nltxt = nulldup(txt);
	    for(p=nltxt;*p;p++) {if(*p == '\n' || *p == '\r' || *p == '\t') {*p = ' ';}};
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_DDS",strlen(nltxt),nltxt);
	    efree(nltxt);
	}
    }
    if(paramcheck34(&nccr->dap,"show","das")) {
	txt = NULL;
	if(nccr->cdmr->oc.ocdasroot != OCNULL)
  	    txt = oc_inq_text(nccr->cdmr->oc.conn,nccr->cdmr->oc.ocdasroot);
	if(txt != NULL) {
	    nltxt = nulldup(txt);
	    for(p=nltxt;*p;p++) {if(*p == '\n' || *p == '\r' || *p == '\t') {*p = ' ';}};
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_DAS",strlen(nltxt),nltxt);
	    efree(nltxt);
	}
    }

done:
    return THROW(ncstat);
}

static NCerror
buildattribute4a(NCCR* cdmr, NCattribute* att, int varid, int ncid)
{
    NCerror ncstat = NC_NOERR;
    char* cname = cdflegalname3(att->name);
    unsigned int nvalues = nclistlength(att->values);
    unsigned int typesize = nctypesizeof(att->etype);
    void* mem = emalloc(typesize * nvalues);

    ncstat = dapcvtattrval3(att->etype,mem,att->values);
    ncstat = nc_put_att(ncid,varid,cname,att->etype,nvalues,mem);
    if(att->etype == NC_STRING) {
	int i;
	for(i=0;i<nvalues;i++) efree(((char**)mem)[i]);
    }
    efree(mem);
    free(cname);
    return THROW(ncstat);
}

static NCerror
showprojection4(NCDAPCOMMON* nccomm, CDFnode* var)
{
    int i,rank;
    NCerror ncstat = NC_NOERR;
    NCbytes* projection = ncbytesnew();
    NClist* path = nclistnew();

    /* If this is not a true leaf variable, then ignore it */
    if(var->nctype == NC_Sequence) return NC_NOERR;
    
    /* Collect the set of DDS node name forming the xpath */
    collectnodepath3(var,path,WITHOUTDATASET);
    for(i=0;i<nclistlength(path);i++) {
        CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(projection,".");
	ncbytescat(projection,node->name);
    }
    /* Now, add the dimension info */
    rank = nclistlength(var->array.dimensions);
    for(i=0;i<rank;i++) {
	CDFnode* dim = (CDFnode*)nclistget(var->array.dimensions,i);
	char tmp[32];
	ncbytescat(projection,"[");
	snprintf(tmp,sizeof(tmp),"%lu",(unsigned long)dim->dim.declsize);
	ncbytescat(projection,tmp);
	ncbytescat(projection,"]");
    }    
    /* Define the attribute */
    ncstat = nc_put_att_text(getncid(nccomm),var->ncid,
                               "_projection",
		               ncbyteslength(projection),
			       ncbytescontents(projection));
    return ncstat;
}


static unsigned long
cdftotalsize4(NClist* dimensions)
{
    unsigned int i;
    unsigned long total = 1;
    if(dimensions != NULL) {
	for(i=0;i<nclistlength(dimensions);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(dimensions,i);
	    total *= dim->dim.declsize;
	}
    }
    return total;
}

static size_t
estimatesizes4r(NCDAPCOMMON* nccomm, CDFnode* node)
{
    int i;
    size_t size = 0;
    size_t totalsize = 0;
    NClist* ncdims;
    unsigned int rank;
    unsigned long dimsize;

    for(i=0;i<nclistlength(node->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
        size += estimatesizes4r(nccomm,subnode);
    }
    switch (node->nctype) {
    case NC_Primitive:
	node->estimatedsize = nctypesizeof(node->etype);
	break;
    case NC_Dataset:
    case NC_Structure:
    case NC_Sequence:
    case NC_Grid:
	node->estimatedsize = size;
	break;
    default:
	PANIC1("Estimatesize: Unexpected node type: %d",node->nctype);
	break;
    }
    /* Now, take dimensions into account */
    ncdims = node->array.dimensions;
    rank = nclistlength(ncdims);
    dimsize = cdftotalsize4(ncdims);
    totalsize = (dimsize * node->estimatedsize);
    if(!node->visible) totalsize = 0;
#ifdef DEBUG
fprintf(stderr,"estimatedsize: %s%s/%u = %lu (= %lu = %lu * %lu)\n",
	(node->visible?"":"*"),
	node->name,rank,
	totalsize,
	node->estimatedsize,
	size,dimsize);
#endif
    node->estimatedsize = totalsize;
    return totalsize;
}


static void
estimatesizes4(NCDAPCOMMON* nccomm)
{
    size_t totalsize;
    CDFnode* root = nccomm->cdf.ddsroot;
    /* Recursively compute the sizes of each node */
    totalsize = estimatesizes4r(nccomm,root);
}

/*
For variables which have a zero size dimension,
either use unlimited, or make them invisible.
*/
static NCerror
fixzerodims4(NCDAPCOMMON* nccomm)
{
    int i;
    NCerror ncstat = NC_NOERR;
    for(i=0;i<nclistlength(nccomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.varnodes,i);
	ncstat = fixzerodims4r(nccomm,var);
    }
    return ncstat;
}

static NCerror
fixzerodims4r(NCDAPCOMMON* nccomm, CDFnode* node)
{
    int i;
    NCerror ncstat = NC_NOERR;
    if(nclistlength(node->array.dimensions) > 0) {
        for(i=0;i<nclistlength(node->array.dimensions);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,i);
	    if(dim->dim.declsize == 0) {
	        if(node->container->nctype == NC_Dataset) { /* use unlimited */
		    ncstat = cvtunlimiteddim(nccomm,dim);
		} else { /* make node invisible */
		    node->visible = 0;
		    node->zerodim = 1;
		}
	    }
	}
    }
    /* walk the subnodes */    
    for(i=0;i<nclistlength(node->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
	ncstat = fixzerodims4r(nccomm,subnode);
    }
    return ncstat;
}

/* Convert a dimension to unlimited */
static NCerror
cvtunlimiteddim(NCDAPCOMMON* nccomm, CDFnode* dim)
{
    DIMFLAGSET(dim,CDFDIMUNLIM);
    nccomm->cdf.unlimited = dim;
    return NC_NOERR;
}

static void
applyclientparamcontrols4(NCDAPCOMMON* nccomm)
{
    NClist* params = NULL;
    const char* value;

    /* Get client parameters */
    params = dapparamdecode(nccomm->oc.url.params);

    /* enable/disable caching */
    value = dapparamlookup(params,"cache");    
    if(value == NULL)
	SETFLAG(nccomm->controls,DFALTCACHEFLAG);
    else if(strlen(value) == 0)
	SETFLAG(nccomm->controls,NCF_CACHE);
    else if(strcmp(value,"1")==0 || value[0] == 'y')
	SETFLAG(nccomm->controls,NCF_CACHE);

    /* Set the translation base  */
    SETFLAG(nccomm->controls,NCF_NCCR);

    /* No longer need params */
    dapparamfree(params);
}
