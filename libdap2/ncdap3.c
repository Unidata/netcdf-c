/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/ncdap3.c,v 1.94 2010/05/28 01:05:34 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "nc3dispatch.h"
#include "ncd3dispatch.h"
#include "dapalign.h"
#include "oc.h"
#include "ocdrno.h"
#include "dapdump.h"

static NCerror buildncstructures3(NCDAPCOMMON*);
static NCerror builddims(NCDAPCOMMON*);
static NCerror buildvars(NCDAPCOMMON*);
static NCerror buildglobalattrs3(NCDAPCOMMON*,CDFnode* root);
static NCerror buildattribute3a(NCDAPCOMMON*, NCattribute*, nc_type, int);


extern CDFnode* v4node;
int nc3dinitialized = 0;

/**************************************************/
/* Add an extra function whose sole purpose is to allow
   configure(.ac) to test for the presence of thiscode.
*/
int nc__opendap(void) {return 0;}

/**************************************************/
/* Do local initialization */

int
nc3dinitialize(void)
{
    int i;
    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
        dapzerostart3[i] = 0;
	dapsinglecount3[i] = 1;
	dapsinglestride3[i] = 1;
    }
    compute_nccalignments();
    nc3dinitialized = 1;
    return NC_NOERR;
}

/**************************************************/
#ifdef NOTUSED
int
NCD3_new_nc(NC** ncpp)
{
    NCDAPCOMMON* ncp;
    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(struct NCDAP3)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}
#endif
/**************************************************/

/* See ncd3dispatch.c for other version */
int
NCD3_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    NC* drno = NULL;
    NCDAPCOMMON* dapcomm = NULL;
    char* modifiedpath;
    OCURI* tmpurl;
    int ncid = -1;
    const char* value;
    char* tmpname = NULL;

    if(!nc3dinitialized) nc3dinitialize();

    if(!ocuriparse(path,&tmpurl)) PANIC("libncdap3: non-url path");
    ocurifree(tmpurl); /* no longer needed */

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
    dapcomm->oc.urltext = modifiedpath;
    ocuriparse(dapcomm->oc.urltext,&dapcomm->oc.uri);
    if(!constrainable34(dapcomm->oc.uri))
	SETFLAG(dapcomm->controls,NCF_UNCONSTRAINABLE);
    dapcomm->cdf.separator = ".";
    dapcomm->cdf.smallsizelimit = DFALTSMALLLIMIT;
    dapcomm->cdf.cache = createnccache();

    /* Use libsrc code for storing metadata */
    tmpname = nulldup(PSEUDOFILE);
    /* Now, use the file to create the netcdf file */
    if(sizeof(size_t) == sizeof(unsigned int))
	ncstat = nc_create(tmpname,NC_CLOBBER,&drno->substrate);
    else
	ncstat = nc_create(tmpname,NC_CLOBBER|NC_64BIT_OFFSET,&drno->substrate);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    /* free the filename so it will automatically go away*/
#ifdef NOTUSED
    {
        /* break the abstraction to get the fd */
	NC* nc;
        ncstat = NC_check_id(drno->substrate, &nc);
	close(nc->int_ncid);
    }
#endif
    /* unlink the temp file so it will automatically be reclaimed */
    unlink(tmpname);
    nullfree(tmpname);

    /* Avoid fill */
    nc_set_fill(drno->ext_ncid,NC_NOFILL,NULL);

    /* process control client parameters */
    applyclientparamcontrols3(dapcomm);

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
#ifdef DEBUG
fprintf(stderr,"parsed constraint: %s\n",
	dumpconstraint(dapcomm->oc.dapconstraint));
#endif

    /* Pass to OC */
    ocstat = oc_open(dapcomm->oc.urltext,&dapcomm->oc.conn);
    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto done;}

    if(paramcheck34(dapcomm,"show","fetch"))
	SETFLAG(dapcomm->controls,NCF_SHOWFETCH);

    /* Turn on logging; only do this after oc_open*/
    value = oc_clientparam_get(dapcomm->oc.conn,"log");
    if(value != NULL) {
	ncloginit();
        ncsetlogging(1);
        nclogopen(value);
	oc_loginit();
        oc_setlogging(1);
        oc_logopen(value);
    }

    /* fetch and build the (almost) unconstrained DDS for use as
       template */
    ncstat = fetchtemplatemetadata3(dapcomm);
    if(ncstat != NC_NOERR) goto done;

    /* fetch and build the constrained DDS */
    ncstat = fetchconstrainedmetadata3(dapcomm);
    if(ncstat != NC_NOERR) goto done;

    /* The following actions are (mostly) WRT to the constrained tree */

    /* Process the constraints to map to the constrained CDF tree */
    ncstat = mapconstraints3(dapcomm->oc.dapconstraint,dapcomm->cdf.ddsroot);
    if(ncstat != NC_NOERR) goto done;

    /* Accumulate useful nodes sets  */
    ncstat = computecdfnodesets3(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Fix grids */
    ncstat = fixgrids3(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Locate and mark usable sequences */
    ncstat = sequencecheck3(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Conditionally suppress variables not in usable
       sequences */
    if(FLAGSET(dapcomm->controls,NCF_NOUNLIM)) {
        ncstat = suppressunusablevars3(dapcomm);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* apply client parameters (after computcdfinfo and computecdfvars)*/
    ncstat = applyclientparams34(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Add (as needed) string dimensions*/
    ncstat = addstringdims(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    if(nclistlength(dapcomm->cdf.seqnodes) > 0) {
	/* Build the sequence related dimensions */
        ncstat = defseqdims(dapcomm);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* Build a cloned set of dimensions for every variable */
    ncstat = clonecdfdims34(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the dimension names*/
    ncstat = computecdfdimnames34(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Deal with zero size dimensions */
    ncstat = fixzerodims3(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    if(nclistlength(dapcomm->cdf.seqnodes) == 0
       && dapcomm->cdf.recorddim != NULL) {
	/* Attempt to use the DODS_EXTRA info to turn
           one of the dimensions into unlimited. Can only do it
           in a sequence free DDS.
        */
        ncstat = defrecorddim3(dapcomm);
        if(ncstat) {THROWCHK(ncstat); goto done;}
   }

    /* Re-compute the var names*/
    ncstat = computecdfvarnames3(dapcomm,dapcomm->cdf.ddsroot,dapcomm->cdf.varnodes);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Estimate the variable sizes */
    estimatevarsizes3(dapcomm);

    /* Build the meta data */
    ncstat = buildncstructures3(dapcomm);

    if(ncstat != NC_NOERR) {
	{THROWCHK(ncstat); goto done;}
    }

    /* Do any necessary data prefetch */
    ncstat = prefetchdata3(dapcomm);
    if(ncstat != NC_NOERR) {
        del_from_NCList((NC*)drno); /* undefine here */
	{THROWCHK(ncstat); goto done;}
    }

    {
        /* Mark as no longer writable and no longer indef;
           requires breaking abstraction  */
	NC* nc;
        ncstat = NC_check_id(drno->substrate, &nc);
        /* Mark as no longer writeable */
        fClr(nc->nciop->ioflags, NC_WRITE);
        /* Mark as no longer indef;
           (do NOT use nc_enddef until diskless is working)*/
	fSet(nc->flags, NC_INDEF);	
    }

    if(ncpp) *ncpp = (NC*)drno;

    return ncstat;

done:
    if(drno != NULL) NCD3_abort(drno->ext_ncid);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

int
NCD3_abort(int ncid)
{
    NC* drno;
    NCDAPCOMMON* dapcomm;
    int ncstatus = NC_NOERR;

    ncstatus = NC_check_id(ncid, (NC**)&drno); 
    if(ncstatus != NC_NOERR) return THROW(ncstatus);

    dapcomm = (NCDAPCOMMON*)drno->dispatchdata;
    ncstatus = nc_abort(drno->substrate);

    /* remove ourselves from NClist */
    del_from_NCList(drno);
    /* clean NC* */
    cleanNCDAPCOMMON(dapcomm);
    if(drno->path != NULL) free(drno->path);
    free(drno);
    return THROW(ncstatus);
}

/**************************************************/
static NCerror
buildncstructures3(NCDAPCOMMON* dapcomm)
{
    NCerror ncstat = NC_NOERR;
    CDFnode* dds = dapcomm->cdf.ddsroot;
    NC* ncsub;
    NC_check_id(dapcomm->controller->substrate,&ncsub);

    ncstat = buildglobalattrs3(dapcomm,dds);
    if(ncstat != NC_NOERR) goto done;

    ncstat = builddims(dapcomm);
    if(ncstat != NC_NOERR) goto done;

    ncstat = buildvars(dapcomm);
    if(ncstat != NC_NOERR) goto done;

done:
    return THROW(ncstat);
}

static NCerror
builddims(NCDAPCOMMON* dapcomm)
{
    int i;
    NCerror ncstat = NC_NOERR;
    int dimid;
    int defunlimited = 0;
    NClist* dimset = NULL;
    NC* drno = dapcomm->controller;
    NC* ncsub;

    ncstat = NC_check_id(drno->substrate,&ncsub);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* collect all dimensions from variables */
    dimset = getalldims3(dapcomm->cdf.varnodes,1);
    /* exclude unlimited */
    for(i=nclistlength(dimset)-1;i>=0;i--) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        if(DIMFLAG(dim,CDFDIMUNLIM)) {
	    defunlimited = 1;
	    nclistremove(dimset,i);
        }
    }
    /* Sort by fullname just for the fun of it */
    for(;;) {
	int last = nclistlength(dimset) - 1;
	int swap = 0;
        for(i=0;i<last;i++) {
	    CDFnode* dim1 = (CDFnode*)nclistget(dimset,i);
	    CDFnode* dim2 = (CDFnode*)nclistget(dimset,i+1);
   	    if(strcmp(dim1->ncfullname,dim2->ncfullname) > 0) {
		nclistset(dimset,i,(ncelem)dim2);
		nclistset(dimset,i+1,(ncelem)dim1);
		swap = 1;
		break;
	    }
	}
	if(!swap) break;
    }
    /* Define unlimited only if needed */ 
    if(defunlimited && dapcomm->cdf.unlimited != NULL) {
	CDFnode* unlimited = dapcomm->cdf.unlimited;
	size_t unlimsize;
        ncstat = nc_def_dim(drno->substrate,
			unlimited->name,
			NC_UNLIMITED,
			&unlimited->ncid);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
        if(DIMFLAG(unlimited,CDFDIMRECORD)) {
	    /* This dimension was defined as unlimited by DODS_EXTRA */
	    unlimsize = unlimited->dim.declsize;
	} else { /* Sequence UNLIMITED */
	    unlimsize = 0;
	}
        /* Set the effective size of UNLIMITED;
           note that this cannot be done thru the normal API.*/
        NC_set_numrecs(ncsub,unlimsize);
    }

    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        if(dim->dim.basedim != NULL) continue; /* handle below */
        ncstat = nc_def_dim(drno->substrate,dim->ncfullname,dim->dim.declsize,&dimid);
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
done:
    nclistfree(dimset);
    return THROW(ncstat);
}

/* Simultaneously build any associated attributes*/
/* and any necessary pseudo-dimensions for string types*/
static NCerror
buildvars(NCDAPCOMMON* dapcomm)
{
    int i,j,dimindex;
    NCerror ncstat = NC_NOERR;
    int varid;
    NClist* varnodes = dapcomm->cdf.varnodes;
    NC* drno = dapcomm->controller;

    ASSERT((varnodes != NULL));
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
        int dimids[NC_MAX_VAR_DIMS];
	unsigned int ncrank;
        NClist* vardims = NULL;

	if(!var->visible) continue;
	if(var->array.basevar != NULL) continue;

#ifdef DEBUG
fprintf(stderr,"buildvars.candidate=|%s|\n",var->ncfullname);
#endif

	vardims = var->array.dimensions;
	ncrank = nclistlength(vardims);
	if(ncrank > 0) {
	    dimindex = 0;
            for(j=0;j<ncrank;j++) {
                CDFnode* dim = (CDFnode*)nclistget(vardims,j);
                dimids[dimindex++] = dim->ncid;
 	    }
        }   
        ncstat = nc_def_var(drno->substrate,var->ncfullname,
                        var->externaltype,
                        ncrank,
                        (ncrank==0?NULL:dimids),
                        &varid);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
        var->ncid = varid;
	if(var->attributes != NULL) {
	    for(j=0;j<nclistlength(var->attributes);j++) {
		NCattribute* att = (NCattribute*)nclistget(var->attributes,j);
		ncstat = buildattribute3a(dapcomm,att,var->etype,varid);
        	if(ncstat != NC_NOERR) goto done;
	    }
	}
	/* Tag the variable with its DAP path */
	if(paramcheck34(dapcomm,"show","projection"))
	    showprojection3(dapcomm,var);
    }    
done:
    return THROW(ncstat);
}

static NCerror
buildglobalattrs3(NCDAPCOMMON* dapcomm, CDFnode* root)
{
    int i;
    NCerror ncstat = NC_NOERR;
    const char* txt;
    char *nltxt, *p;
    NCbytes* buf = NULL;
    NClist* cdfnodes;
    NC* drno = dapcomm->controller;

    if(root->attributes != NULL) {
        for(i=0;i<nclistlength(root->attributes);i++) {
   	    NCattribute* att = (NCattribute*)nclistget(root->attributes,i);
	    ncstat = buildattribute3a(dapcomm,att,NC_NAT,NC_GLOBAL);
            if(ncstat != NC_NOERR) goto done;
	}
    }

    /* Add global attribute identifying the sequence dimensions */
    if(paramcheck34(dapcomm,"show","seqdims")) {
        buf = ncbytesnew();
        cdfnodes = dapcomm->cdf.ddsroot->tree->nodes;
        for(i=0;i<nclistlength(cdfnodes);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(cdfnodes,i);
	    if(dim->nctype != NC_Dimension) continue;
	    if(DIMFLAG(dim,CDFDIMSEQ)) {
	        char* cname = cdflegalname3(dim->name);
	        if(ncbyteslength(buf) > 0) ncbytescat(buf,", ");
	        ncbytescat(buf,cname);
	        nullfree(cname);
	    }
	}
        if(ncbyteslength(buf) > 0) {
            ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_sequence_dimensions",
	           ncbyteslength(buf),ncbytescontents(buf));
	}
    }

    /* Define some additional system global attributes
       depending on show= clientparams*/
    /* Ignore failures*/

    if(paramcheck34(dapcomm,"show","translate")) {
        /* Add a global attribute to show the translation */
        ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_translate",
	           strlen("netcdf-3"),"netcdf-3");
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
            ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_dds",strlen(nltxt),nltxt);
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
            ncstat = nc_put_att_text(drno->substrate,NC_GLOBAL,"_das",strlen(nltxt),nltxt);
	    nullfree(nltxt);
	}
    }

done:
    ncbytesfree(buf);
    return THROW(ncstat);
}

static NCerror
buildattribute3a(NCDAPCOMMON* dapcomm, NCattribute* att, nc_type vartype, int varid)
{
    int i;
    NCerror ncstat = NC_NOERR;
    char* cname = cdflegalname3(att->name);
    unsigned int nvalues = nclistlength(att->values);
    NC* drno = dapcomm->controller;

    /* If the type of the attribute is string, then we need*/
    /* to convert to a single character string by concatenation.
	modified: 10/23/09 to insert newlines.
	modified: 10/28/09 to interpret escapes
    */
    if(att->etype == NC_STRING || att->etype == NC_URL) {
	char* newstring;
	size_t newlen = 0;
	for(i=0;i<nvalues;i++) {
	    char* s = (char*)nclistget(att->values,i);
	    newlen += (1+strlen(s));
	}
	newstring = (char*)malloc(newlen);
        MEMCHECK(newstring,NC_ENOMEM);
	newstring[0] = '\0';
	for(i=0;i<nvalues;i++) {
	    char* s = (char*)nclistget(att->values,i);
	    if(i > 0) strcat(newstring,"\n");
	    strcat(newstring,s);
	}
        dapexpandescapes(newstring);
	if(newstring[0]=='\0')
	    ncstat = nc_put_att_text(drno->substrate,varid,cname,1,newstring);
	else
	    ncstat = nc_put_att_text(drno->substrate,varid,cname,strlen(newstring),newstring);
	free(newstring);
    } else {
	nc_type atype;
	unsigned int typesize;
	void* mem;
	/* It turns out that some servers upgrade the type
           of _FillValue in order to correctly preserve the
           original value. However, since the type of the
           underlying variable is not changes, we get a type
           mismatch. So, make sure the type of the fillvalue
           is the same as that of the controlling variable.
	*/
        if(varid != NC_GLOBAL && strcmp(att->name,"_FillValue")==0)
	    atype = nctypeconvert(dapcomm,vartype);
	else
	    atype = nctypeconvert(dapcomm,att->etype);
	typesize = nctypesizeof(atype);
	mem = malloc(typesize * nvalues);
        ncstat = dapcvtattrval3(atype,mem,att->values);
        ncstat = nc_put_att(drno->substrate,varid,cname,atype,nvalues,mem);
	nullfree(mem);
    }
    free(cname);
    return THROW(ncstat);
}
