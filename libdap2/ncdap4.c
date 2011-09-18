#include "ncdap4.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "dapdebug.h"
#include "dapalign.h"

#include "netcdf.h"
#include "nc4dispatch.h"
#include "ncd4dispatch.h"

#ifdef DEBUG
#include "dapdump.h"
#endif

#define DFALTMODELFLAGS (NCF_NC3|NCF_NCDAP)


ptrdiff_t dapsinglestride4[NC_MAX_VAR_DIMS];

extern NC_FILE_INFO_T* nc_file;

extern NCerror cleanNCDAP4(NCDAP4*);

static void nc4dinitialize(void);
static NCerror buildncstructures4(NCDAPCOMMON* dapcomm);
static NCerror builddims4(NCDAPCOMMON*);
static NCerror buildtypes4(NCDAPCOMMON*);
static NCerror buildtypes4r(NCDAPCOMMON*, CDFnode* tnode);
static NCerror buildvars4(NCDAPCOMMON*);
static NCerror buildglobalattrs4(NCDAPCOMMON*, int, CDFnode* root);
static NCerror buildattribute4a(NCDAPCOMMON*, NCattribute* att, int varid);
static NCerror showprojection4(NCDAPCOMMON* dapcomm, CDFnode* var);
static size_t estimatesizes4r(NCDAPCOMMON* dapcomm, CDFnode* node);
static void estimatesizes4(NCDAPCOMMON* dapcomm);
static NCerror fixzerodims4(NCDAPCOMMON* dapcomm);
static NCerror fixzerodims4r(NCDAPCOMMON* dapcomm, CDFnode* node);
static NCerror cvtunlimiteddim(NCDAPCOMMON* dapcomm, CDFnode* dim);
static void applyclientparamcontrols4(NCDAPCOMMON* dapcomm);

static int nc4dinitialized = 0;

/**************************************************/
#ifdef NOTUSED
int
NCD4_new_nc(NC** ncpp)
{
    NCDAP4* ncp;
    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(struct NCDAP4)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}
#endif

/**************************************************/
/* See ncd4dispatch.c for other version */
int
NCD4_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCURI* tmpurl;
    NC* drno = NULL;
    NCDAPCOMMON* dapcomm = NULL;
    int ncid = -1;
    char* modifiedpath = NULL;
    const char* value;
    char* tmpname = NULL;

    LOG((1, "nc_open_file: path %s mode %d", path, mode));

    if(!nc4dinitialized) nc4dinitialize();

    if(!ocuriparse(path,&tmpurl)) PANIC("libncdap4: non-url path");
    ocurifree(tmpurl); /* no longer needed */

    /* Check for legal mode flags */
    if((mode & NC_WRITE) != 0) ncstat = NC_EINVAL;
    else if(mode & (NC_WRITE|NC_CLOBBER)) ncstat = NC_EPERM;
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    mode = (mode & ~(NC_MPIIO | NC_MPIPOSIX));
    /* Despite the above check, we want the file to be initially writable */
    mode |= (NC_WRITE|NC_CLOBBER);

#ifdef DEBUG
extern int ocdebug;
ocdebug = 1;
#endif

#ifdef OCCOMPILEBYDEFAULT
    /* set the compile flag by default */
    modifiedpath = (char*)emalloc(strlen(path)+strlen("[compile]")+1);
    strcpy(modifiedpath,"[compile]");
    strcat(modifiedpath,path);    
#else
    modifiedpath = nulldup(path);
#endif

    /* Setup our NC and NCDAPCOMMON state*/
    drno = (NC*)calloc(1,sizeof(NC));
    if(drno == NULL) {ncstat = NC_ENOMEM; goto done;}
    /* compute an ncid */
    ncstat = add_to_NCList(drno);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncid = drno->ext_ncid;

    dapcomm = (NCDAPCOMMON*)calloc(1,sizeof(NCDAPCOMMON));
    if(dapcomm == NULL) {ncstat = NC_ENOMEM; goto done;}

    drno->dispatch = dispatch;
    drno->dispatchdata = dapcomm;

    dapcomm->controller = (NC*)drno;
    dapcomm->cdf.separator = ".";
    dapcomm->oc.urltext = modifiedpath;
    ocuriparse(dapcomm->oc.urltext,&dapcomm->oc.uri);
    if(!constrainable34(dapcomm->oc.uri))
	SETFLAG(dapcomm->controls,NCF_UNCONSTRAINABLE);
    dapcomm->cdf.smallsizelimit = DFALTSMALLLIMIT;
    dapcomm->cdf.cache = createnccache();
#ifdef IGNORE
    dapcomm->cdf.cache->cachelimit = DFALTCACHELIMIT;
    dapcomm->cdf.cache->cachesize = 0;
    dapcomm->cdf.cache->nodes = nclistnew();
    dapcomm->cdf.cache->cachecount = DFALTCACHECOUNT;
#endif
#ifdef HAVE_GETRLIMIT
    { struct rlimit rl;
      if(getrlimit(RLIMIT_NOFILE, &rl) >= 0) {
	dapcomm->cdf.cache->cachecount = (size_t)(rl.rlim_cur / 2);
      }
    }
#endif

    /* Use libsrc4 code for storing metadata */
    tmpname = nulldup(PSEUDOFILE);
    /* Now, use the file to create the netcdf file */
    ncstat = nc_create(tmpname,NC_CLOBBER|NC_NETCDF4,&drno->substrate);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* free the filename so it will automatically go away*/
    unlink(tmpname);
    nullfree(tmpname);

    /* Avoid fill */
    nc_set_fill(drno->ext_ncid,NC_NOFILL,NULL);

    /* Re-scan the client parameters */
    applyclientparamcontrols4(dapcomm);

    dapcomm->oc.dapconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
    dapcomm->oc.dapconstraint->projections = nclistnew();
    dapcomm->oc.dapconstraint->selections = nclistnew();

    /* Check to see if we are unconstrainable */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE)) {
	if(dapcomm->oc.uri->constraint != NULL
	   && strlen(dapcomm->oc.uri->constraint) > 0) {
	    nclog(NCLOGWARN,"Attempt to constrain an unconstrainable data source: %s",
		   dapcomm->oc.uri->constraint);
	}
	/* ignore all constraints */
    } else {
        /* Parse constraints to make sure that they are syntactically correct */
        ncstat = parsedapconstraints(dapcomm,dapcomm->oc.uri->constraint,dapcomm->oc.dapconstraint);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }

    ocstat = oc_open(dapcomm->oc.urltext,&dapcomm->oc.conn);
    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto done;}

    if(paramcheck34(dapcomm,"show","fetch"))
	SETFLAG(dapcomm->controls,NCF_SHOWFETCH);

    /* Turn on logging */
    value = oc_clientparam_get(dapcomm->oc.conn,"log");
    if(value != NULL) {
	ncloginit();
        ncsetlogging(1);
        nclogopen(value);
	oc_loginit();
        oc_setlogging(1);
        oc_logopen(value);
    }

    /* fetch and build the (almost) unconstrained DDS */
    ncstat = fetchtemplatemetadata3(dapcomm);
    if(ncstat != NC_NOERR) goto done;

    /* fetch and build the constrained DDS */
    ncstat = fetchconstrainedmetadata3(dapcomm);
    if(ncstat != NC_NOERR) goto done;

    /* The following actions are WRT to the constrained tree */

    /* Process the constraints to map the CDF tree */
    ncstat = mapconstraints3(dapcomm->oc.dapconstraint,dapcomm->cdf.ddsroot);
    if(ncstat != NC_NOERR) goto done;

    /* Accumulate useful nodes sets  */
    ncstat = computecdfnodesets4(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Fix grids */
    ncstat = fixgrids4(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* apply client parameters (after computcdfinfo and computecdfvars)*/
    ncstat = applyclientparams34(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Accumulate the nodes representing user types*/
    ncstat = computeusertypes4(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the type names*/
    ncstat = shortentypenames4(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the dimension names*/
    ncstat = computecdfdimnames34(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* deal with zero-size dimensions */
    ncstat = fixzerodims4(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Estimate the variable sizes */
    estimatesizes4(dapcomm);

    ncstat = buildncstructures4(dapcomm);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Do any necessary data prefetch */
    ncstat = prefetchdata3(dapcomm);
    if(ncstat != NC_NOERR)
	{THROWCHK(ncstat); goto done;}

    /* Mark as no longer indef and no longer writable*/
    {
        /* Mark as no longer writable and no longer indef;
           requires breaking abstraction  */
	NC* nc;
        NC_FILE_INFO_T* nfit = NULL;
        NC_HDF5_FILE_INFO_T* h5 = NULL;
        NC_GRP_INFO_T *grp = NULL;
        ncstat = NC_check_id(drno->substrate, &nc);
        /* Find our metadata for this file. */
        ncstat = nc4_find_nc_grp_h5(drno->substrate, &nfit, &grp, &h5);
        if(ncstat) {THROWCHK(ncstat); goto done;}
        /* Mark as no longer indef (do NOT use nc_enddef until diskless is working)*/
        h5->flags &= ~(NC_INDEF);
        /* Mark as no longer writeable */
        h5->no_write = 1;
    }

    if(ncpp) *ncpp = (NC*)drno;

    return ncstat;

done:
    if(drno != NULL) NCD4_abort(drno->ext_ncid);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

int
NCD4_abort(int ncid)
{
    NC* drno;
    NCDAPCOMMON* dapcomm;
    int ncstat = NC_NOERR;

    LOG((1, "nc_abort: ncid 0x%x", ncid));

    ncstat = NC_check_id(ncid, (NC**)&drno); 
    if(ncstat != NC_NOERR) return THROW(ncstat);

    dapcomm = (NCDAPCOMMON*)drno->dispatchdata;
    ncstat = nc_abort(drno->substrate);

    /* remove ourselves from NClist */
    del_from_NCList(drno);
    /* clean NC* */
    cleanNCDAPCOMMON(dapcomm);
    if(drno->path != NULL) free(drno->path);
    free(drno);
    return THROW(ncstat);
}

/**************************************************/
/* Auxilliary routines                            */
/**************************************************/

static void
nc4dinitialize()
{
    nc3dinitialize();
    nc4dinitialized = 1;
}

/*
Note: never use any of the libncdap3 code to call
netcdf API functions because it will use the netcdf-3 API.
*/
static NCerror
buildncstructures4(NCDAPCOMMON* dapcomm)
{
    NCerror ncstat = NC_NOERR;
    CDFnode* dds = dapcomm->cdf.ddsroot;
    NC* drno = dapcomm->controller;

    ncstat = buildglobalattrs4(dapcomm,drno->substrate,dds);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = builddims4(dapcomm);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = buildtypes4(dapcomm);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    ncstat = buildvars4(dapcomm);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

done:
    return THROW(ncstat);
}

/* Define dim info for top-level dims */
static NCerror
builddims4(NCDAPCOMMON* dapcomm)
{
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;
    int dimid;
    NClist* dimset = nclistnew();
    NC* drno = dapcomm->controller;

    /* collect all dimensions from variables,
       including duplicates; note we use array.dimensions
       not array.ncdimensions.
    */
    for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
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
        ncstat = nc_def_dim(drno->substrate,
                            dim->ncfullname,
                            dim->dim.declsize,
                            &dimid);
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
buildtypes4(NCDAPCOMMON* dapcomm)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;

    /* Define user types in postorder */
    for(i=0;i<nclistlength(dapcomm->cdf.usertypes);i++) {
	CDFnode* node = (CDFnode*)nclistget(dapcomm->cdf.usertypes,i);
	if(!node->visible) continue;
	ncstat = buildtypes4r(dapcomm,node);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
done:
    return THROW(ncstat);
}

static NCerror
buildtypes4r(NCDAPCOMMON* dapcomm, CDFnode* tnode)
{
    unsigned int i,j;
    int typeid;
    NCerror ncstat = NC_NOERR;
    NC* drno = dapcomm->controller;

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
	    ncstat = nc_def_vlen(drno->substrate,tnode->vlenname,
                                     prim->etype,&typeid);
	    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    tnode->basetypeid = prim->etype;
	    tnode->typeid = typeid;
	    break;
	}
        /* fall thru */
    case NC_Grid:
    case NC_Structure:
	ncstat = nc_def_compound(drno->substrate,tnode->typesize.instance.size,
				tnode->typename,&typeid);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	tnode->typeid = typeid;
	for(i=0;i<nclistlength(tnode->subnodes);i++) {
	    CDFnode* field = (CDFnode*)nclistget(tnode->subnodes,i);
	    if(!field->visible) continue;
	    if(nclistlength(field->array.dimensions) == 0) {
   	        ncstat = nc_insert_compound(drno->substrate,typeid,
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
   	        ncstat = nc_insert_array_compound(drno->substrate,typeid,
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
	    ncstat = nc_def_vlen(drno->substrate,tnode->vlenname,tnode->typeid,&typeid);
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
buildvars4(NCDAPCOMMON* dapcomm)
{
    /* Variables (in this translation) are (mostly)
       the direct fields of the Dataset*/
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;
    int varid;
    NC* drno = dapcomm->controller;

    for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
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
	setvarbasetype(dapcomm,var);
	ASSERT((var->typeid > 0));
        ncstat = nc_def_var(drno->substrate,var->ncfullname,
			    var->typeid,
                            nclistlength(var->array.dimensions),
                            (ncrank==0?NULL:dimids),
                            &varid);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	var->ncid = varid;
	if(var->attributes != NULL) {
	    for(j=0;j<nclistlength(var->attributes);j++) {
		NCattribute* att = (NCattribute*)nclistget(var->attributes,j);
		ncstat = buildattribute4a(dapcomm,att,varid);
        	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    }
	}
	/* Tag the variable with its DAP path */
	if(paramcheck34(dapcomm,"show","projection"))
	    showprojection4(dapcomm,var);
    }
    
done:
    return THROW(ncstat);
}

static NCerror
buildglobalattrs4(NCDAPCOMMON* dapcomm, int ncid, CDFnode* root)
{
    int i;
    const char* txt;
    char *nltxt, *p;
    NCerror ncstat = NC_NOERR;
    NC* drno = dapcomm->controller;
    if(root->attributes != NULL) {
        for(i=0;i<nclistlength(root->attributes);i++) {
   	    NCattribute* att = (NCattribute*)nclistget(root->attributes,i);
	    ncstat = buildattribute4a(dapcomm,att,NC_GLOBAL);
            if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	}
    }

    /* Define some additional system global attributes depending
       on show= clientparams*/
    /* Ignore doneures*/

    if(paramcheck34(dapcomm,"show","translate")) {
        /* Add a global attribute to show the translation */
        ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_translate",
	           strlen("netcdf-4"),"netcdf-4");
    }

    if(paramcheck34(dapcomm,"show","url")) {
	if(dapcomm->oc.urltext != NULL)
            ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_url",
				       strlen(dapcomm->oc.urltext),dapcomm->oc.urltext);
    }
    if(paramcheck34(dapcomm,"show","dds")) {
	txt = NULL;
	if(dapcomm->cdf.ddsroot != NULL)
  	    txt = oc_inq_text(dapcomm->oc.conn,dapcomm->cdf.ddsroot->dds);
	if(txt != NULL) {
	    /* replace newlines with spaces*/
	    nltxt = nulldup(txt);
	    for(p=nltxt;*p;p++) {if(*p == '\n' || *p == '\r' || *p == '\t') {*p = ' ';}};
            ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_DDS",strlen(nltxt),nltxt);
	    nullfree(nltxt);
	}
    }
    if(paramcheck34(dapcomm,"show","das")) {
	txt = NULL;
	if(dapcomm->oc.ocdasroot != OCNULL)
  	    txt = oc_inq_text(dapcomm->oc.conn,dapcomm->oc.ocdasroot);
	if(txt != NULL) {
	    nltxt = nulldup(txt);
	    for(p=nltxt;*p;p++) {if(*p == '\n' || *p == '\r' || *p == '\t') {*p = ' ';}};
            ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_DAS",strlen(nltxt),nltxt);
	    nullfree(nltxt);
	}
    }

done:
    return THROW(ncstat);
}

static NCerror
buildattribute4a(NCDAPCOMMON* dapcomm, NCattribute* att, int varid)
{
    NCerror ncstat = NC_NOERR;
    char* cname = cdflegalname3(att->name);
    unsigned int nvalues = nclistlength(att->values);
    unsigned int typesize = nctypesizeof(att->etype);
    void* mem = malloc(typesize * nvalues);
    NC* drno = dapcomm->controller;

    ncstat = dapcvtattrval3(att->etype,mem,att->values);
    ncstat = nc_put_att(drno->substrate,varid,cname,att->etype,nvalues,mem);
    if(att->etype == NC_STRING) {
	int i;
	for(i=0;i<nvalues;i++) nullfree(((char**)mem)[i]);
    }
    nullfree(mem);
    free(cname);
    return THROW(ncstat);
}

static NCerror
showprojection4(NCDAPCOMMON* dapcomm, CDFnode* var)
{
    int i,rank;
    NCerror ncstat = NC_NOERR;
    NCbytes* projection = ncbytesnew();
    NClist* path = nclistnew();
    NC* drno = dapcomm->controller;

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
    ncstat = nc_put_att_text(drno->substrate,var->ncid,
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
estimatesizes4r(NCDAPCOMMON* dapcomm, CDFnode* node)
{
    int i;
    size_t size = 0;
    size_t totalsize = 0;
    NClist* ncdims;
    unsigned int rank;
    unsigned long dimsize;

    for(i=0;i<nclistlength(node->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
        size += estimatesizes4r(dapcomm,subnode);
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
estimatesizes4(NCDAPCOMMON* dapcomm)
{
    size_t totalsize;
    CDFnode* root = dapcomm->cdf.ddsroot;
    /* Recursively compute the sizes of each node */
    totalsize = estimatesizes4r(dapcomm,root);
}

/*
For variables which have a zero size dimension,
either use unlimited, or make them invisible.
*/
static NCerror
fixzerodims4(NCDAPCOMMON* dapcomm)
{
    int i;
    NCerror ncstat = NC_NOERR;
    for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
	ncstat = fixzerodims4r(dapcomm,var);
    }
    return ncstat;
}

static NCerror
fixzerodims4r(NCDAPCOMMON* dapcomm, CDFnode* node)
{
    int i;
    NCerror ncstat = NC_NOERR;
    if(nclistlength(node->array.dimensions) > 0) {
        for(i=0;i<nclistlength(node->array.dimensions);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,i);
	    if(dim->dim.declsize == 0) {
	        if(node->container->nctype == NC_Dataset) { /* use unlimited */
		    ncstat = cvtunlimiteddim(dapcomm,dim);
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
	ncstat = fixzerodims4r(dapcomm,subnode);
    }
    return ncstat;
}

/* Convert a dimension to unlimited */
static NCerror
cvtunlimiteddim(NCDAPCOMMON* dapcomm, CDFnode* dim)
{
    DIMFLAGSET(dim,CDFDIMUNLIM);
    dapcomm->cdf.unlimited = dim;
    return NC_NOERR;
}

static void
applyclientparamcontrols4(NCDAPCOMMON* dapcomm)
{
    OCURI* uri = dapcomm->oc.uri;
    const char* value;

    /* enable/disable caching */
    value = ocurilookup(uri,"cache");    
    if(value == NULL)
	SETFLAG(dapcomm->controls,DFALTCACHEFLAG);
    else if(strlen(value) == 0)
	SETFLAG(dapcomm->controls,NCF_CACHE);
    else if(strcmp(value,"1")==0 || value[0] == 'y')
	SETFLAG(dapcomm->controls,NCF_CACHE);

    /* Set the translation base  */
    SETFLAG(dapcomm->controls,NCF_NC4);
}
