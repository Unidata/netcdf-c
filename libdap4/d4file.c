/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdispatch.h"
#include "d4includes.h"

int
NCD4_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC* d4info)
{
    NCerror ncstat = NC_NOERR;
#if 0
    NCDAPCOMMON* dapcomm = NULL;
    const char* value;
    int nc3id = -1;

    if(path == NULL)
	return NC_EDAPURL;
    if(dispatch == NULL) PANIC("NC3D_open: no dispatch table");

    /* Setup our NC and NCDAPCOMMON state*/

    dapcomm = (NCDAPCOMMON*)calloc(1,sizeof(NCDAPCOMMON));
    if(dapcomm == NULL) {ncstat = NC_ENOMEM; goto done;}

    NCD4_DATA_SET(d4info,dapcomm);
    d4info->int_ncid = nc__pseudofd(); /* create a unique id */
    dapcomm->controller = (NC*)d4info;

    dapcomm->cdf.separator = ".";
    dapcomm->cdf.smallsizelimit = DFALTSMALLLIMIT;
    dapcomm->cdf.cache = createnccache();

#ifdef HAVE_GETRLIMIT
    { struct rlimit rl;
      if(getrlimit(RLIMIT_NOFILE, &rl) >= 0) {
	dapcomm->cdf.cache->cachecount = (size_t)(rl.rlim_cur / 2);
      }
    }
#endif

#ifdef OCCOMPILEBYDEFAULT
    /* set the compile flag by default */
    dapcomm->oc.rawurltext = (char*)emalloc(strlen(path)+strlen("[compile]")+1);
    strcpy(dapcomm->oc.rawurltext,"[compile]");
    strcat(dapcomm->oc.rawurltext, path);
#else
    dapcomm->oc.rawurltext = strdup(path);
#endif

    ncuriparse(dapcomm->oc.rawurltext,&dapcomm->oc.url);

    /* parse the client parameters */
    ncuridecodeparams(dapcomm->oc.url);

    if(!constrainable(dapcomm->oc.url))
	SETFLAG(dapcomm->controls,NCF_UNCONSTRAINABLE);

#ifdef COLUMBIA_HACK
    {
	const char* p;
	/* Does this url look like it is from columbia? */
	if(dapcomm->oc.url->host != NULL) {
	    for(p=dapcomm->oc.url->host;*p;p++) {
	        if(strncmp(p,COLUMBIA_HACK,strlen(COLUMBIA_HACK))==0)
		    SETFLAG(dapcomm->controls,NCF_COLUMBIA);
	    }
	}
    }
#endif

    /* fail if we are unconstrainable but have constraints */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE)) {
	if(dapcomm->oc.url->constraint != NULL) {
	    nclog(NCLOGWARN,"Attempt to constrain an unconstrainable data source: %s",
		   dapcomm->oc.url->constraint);
	    ncstat = THROW(NC_EDAPCONSTRAINT);
	    goto done;
	}
    }

    /* Use libsrc code (netcdf-3) for storing metadata */
    {
	char tmpname[32];

        /* Create fake file name: exact name must be unique,
           but is otherwise irrelevant because we are using NC_DISKLESS
        */
        snprintf(tmpname,sizeof(tmpname),"%d",d4info->int_ncid);

        /* Now, use the file to create the hidden, in-memory netcdf file.
	   We want this hidden file to always be NC_CLASSIC, so we need to
           force default format temporarily in case user changed it.
	*/
	{
	    int new = NC_CLASSIC_MODEL;
	    int old = 0;
	    nc_set_default_format(new,&old); /* save and change */
            ncstat = nc_create(tmpname,NC_DISKLESS|NC_CLASSIC_MODEL,&nc3id);
	    nc_set_default_format(old,&new); /* restore */
	}
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	dapcomm->nc3id = nc3id;
	/* Avoid fill */
	nc_set_fill(nc3id,NC_NOFILL,NULL);

    }

    dapcomm->oc.dapconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
    dapcomm->oc.dapconstraint->projections = nclistnew();
    dapcomm->oc.dapconstraint->selections = nclistnew();

     /* Parse constraints to make sure they are syntactically correct */
     ncstat = dapparsedapconstraints(dapcomm,dapcomm->oc.url->constraint,dapcomm->oc.dapconstraint);
     if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Construct a url for oc minus any constraint and params*/
    dapcomm->oc.urltext = ncuribuild(dapcomm->oc.url,NULL,NULL,
				      (NCURISTD ^ NCURICONSTRAINTS));

    /* Pass to OC */
    ocstat = oc_open(dapcomm->oc.urltext,&dapcomm->oc.conn);
    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto done;}

#ifdef DEBUG1
    (void)oc_trace_curl(dapcomm->oc.conn);
#endif

    nullfree(dapcomm->oc.urltext); /* clean up */
    dapcomm->oc.urltext = NULL;

    /* process control client parameters */
    applyclientparamcontrols(dapcomm);

    /* Turn on logging; only do this after oc_open*/
    if((value = dapparamvalue(dapcomm,"log")) != NULL) {
	ncloginit();
        if(nclogopen(value))
	    ncsetlogging(1);
	ncloginit();
        if(nclogopen(value))
	    ncsetlogging(1);
    }

    /* fetch and build the unconstrained DDS for use as
       pattern */
    ncstat = fetchpatternmetadata(dapcomm);
    if(ncstat != NC_NOERR) goto done;

    /* Operations on the pattern tree */

    /* Accumulate useful nodes sets  */
    ncstat = computecdfnodesets(dapcomm,dapcomm->cdf.fullddsroot->tree);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Define the dimsettrans list */
    ncstat = definedimsettrans(dapcomm,dapcomm->cdf.fullddsroot->tree);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Mark the nodes of the pattern that are eligible for prefetch */
    ncstat = markprefetch(dapcomm);

    /* fetch and build the constrained DDS */
    ncstat = fetchconstrainedmetadata(dapcomm);
    if(ncstat != NC_NOERR) goto done;

#ifdef DEBUG2
fprintf(stderr,"constrained dds: %s\n",dumptree(dapcomm->cdf.ddsroot));
#endif

    /* Operations on the constrained tree */

    /* Accumulate useful nodes sets  */
    ncstat = computecdfnodesets(dapcomm,dapcomm->cdf.ddsroot->tree);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Fix grids */
    ncstat = fixgrids(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Locate and mark usable sequences */
    ncstat = sequencecheck(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* suppress variables not in usable sequences */
    ncstat = suppressunusablevars(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* apply client parameters */
    ncstat = applyclientparams(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Add (as needed) string dimensions*/
    ncstat = addstringdims(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    if(nclistlength(dapcomm->cdf.ddsroot->tree->seqnodes) > 0) {
	/* Build the sequence related dimensions */
        ncstat = defseqdims(dapcomm);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* Define the dimsetplus and dimsetall lists */
    ncstat = definedimsets(dapcomm,dapcomm->cdf.ddsroot->tree);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the dimension names*/
    ncstat = computecdfdimnames(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Deal with zero size dimensions */
    ncstat = fixzerodims(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Attempt to use the DODS_EXTRA info to turn
       one of the dimensions into unlimited.
       Assume computecdfdimnames34 has already been called.
    */
    ncstat = defrecorddim(dapcomm);
    if(ncstat) {THROWCHK(ncstat); goto done;}
    if(dapcomm->cdf.recorddimname != NULL
       && nclistlength(dapcomm->cdf.ddsroot->tree->seqnodes) > 0) {
	/*nclog(NCLOGWARN,"unlimited dimension specified, but sequences exist in DDS");*/
	PANIC("unlimited dimension specified, but sequences exist in DDS");
    }

    /* Re-compute the var names*/
    ncstat = computecdfvarnames(dapcomm,
				 dapcomm->cdf.ddsroot,
				 dapcomm->cdf.ddsroot->tree->varnodes);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Transfer data from the unconstrained DDS data to the unconstrained DDS */
    ncstat = dimimprint(dapcomm);
    if(ncstat) goto done;

    /* Process the constraints to map to the constrained CDF tree */
    /* (must follow fixgrids3) */
    ncstat = dapmapconstraints(dapcomm->oc.dapconstraint,dapcomm->cdf.ddsroot);
    if(ncstat != NC_NOERR) goto done;

    /* Canonicalize the constraint */
    ncstat = dapfixprojections(dapcomm->oc.dapconstraint->projections);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Fill in segment information */
    ncstat = dapqualifyconstraints(dapcomm->oc.dapconstraint);
    if(ncstat != NC_NOERR) goto done;

    /* Accumulate set of variables in the constraint's projections */
    ncstat = dapcomputeprojectedvars(dapcomm,dapcomm->oc.dapconstraint);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* using the modified constraint, rebuild the constraint string */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE)) {
	/* ignore all constraints */
	dapcomm->oc.urltext = ncuribuild(dapcomm->oc.url,NULL,NULL,0);
    } else {
	char* constraintstring = dcebuildconstraintstring(dapcomm->oc.dapconstraint);
        ncurisetconstraints(dapcomm->oc.url,constraintstring);
	nullfree(constraintstring);
        dapcomm->oc.urltext = ncuribuild(dapcomm->oc.url,NULL,NULL,NCURICONSTRAINTS);
    }

#ifdef DEBUG
fprintf(stderr,"ncdap3: final constraint: %s\n",dapcomm->oc.url->constraint);
#endif

    /* Estimate the variable sizes */
    estimatevarsizes(dapcomm);

    /* Build the meta data */
    ncstat = buildncstructures(dapcomm);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Explicitly do not call enddef because it will complain
       about variables that are too large.
    */
#if 0
    ncstat = nc_endef(nc3id,NC_NOFILL,NULL);
    if(ncstat != NC_NOERR && ncstat != NC_EVARSIZE)
        {THROWCHK(ncstat); goto done;}
#endif

    { /* (for now) break abstractions*/
	    NC* ncsub;
	    NC3_INFO* nc3i;
	    CDFnode* unlimited = dapcomm->cdf.recorddim;
            /* get the dispatch data for the substrate */
            ncstat = NC_check_id(nc3id,&ncsub);
	    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    nc3i = (NC3_INFO*)ncsub->dispatchdata;
	    /* This must be checked after all dds and data processing
               so we can figure out the value of numrecs.
	    */
            if(unlimited != NULL) { /* Set the effective size of UNLIMITED */
                NC_set_numrecs(nc3i,unlimited->dim.declsize);
	    }
            /* Pretend the substrate is read-only */
	    NC_set_readonly(nc3i);
    }

    /* Do any necessary data prefetch */
    if(FLAGSET(dapcomm->controls,NCF_PREFETCH)
       && FLAGSET(dapcomm->controls,NCF_PREFETCH_EAGER)) {
        ncstat = prefetchdata(dapcomm);
        if(ncstat != NC_NOERR) {
            del_from_NCList((NC*)d4info); /* undefine here */
	    {THROWCHK(ncstat); goto done;}
	}
    }

    return ncstat;

done:
    if(d4info != NULL) NCD4_close(d4info->ext_ncid);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
#endif
    return THROW(ncstat);
}

int
NCD4_close(int ncid)
{
    int ncstatus = NC_NOERR;
#if 0
    NC* d4info;
    NCDAPCOMMON* dapcomm;

    ncstatus = NC_check_id(ncid, (NC**)&d4info);
    if(ncstatus != NC_NOERR) return THROW(ncstatus);
    dapcomm = (NCDAPCOMMON*)d4info->dispatchdata;

    /* We call abort rather than close to avoid
       trying to write anything or try to pad file length
     */
    ncstatus = nc_abort(getnc3id(d4info));

    /* clean NC* */
    freeNCDAPCOMMON(dapcomm);
#endif
    return THROW(ncstatus);
}


int
NCD4_abort(int ncid)
{
    return NCD4_close(ncid);
}

