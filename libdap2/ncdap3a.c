/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdap3.h"
#include "nc3dispatch.h"
#include "ncd3dispatch.h"
#include "dapalign.h"
#include "oc.h"
#include "ocdrno.h"
#include "dapdump.h"


#define getncid(drno) (((NC*)drno)->ext_ncid)

/*Forward*/
static NCerror getseqdimsize(NCDAPCOMMON*, CDFnode* seq, size_t* sizep);
static int fieldindex(CDFnode* parent, CDFnode* child);
static NCerror countsequence(NCDAPCOMMON*, CDFnode* node, size_t*);
static NCerror makeseqdim(NCDAPCOMMON*, CDFnode* node, size_t, CDFnode**);
static NCerror computeminconstraints3(NCDAPCOMMON*,CDFnode*,NCbytes*);

void
freegetvara(Getvara* vara)
{
    if(vara == NULL) return;
    dcefree((DCEnode*)vara->varaprojection);
    nullfree(vara);
}

NCerror
cleanNCDAPCOMMON(NCDAPCOMMON* dapcomm)
{
    /* abort the metadata file */
    (void)nc_abort(getncid(dapcomm));
    freenccache(dapcomm,dapcomm->cdf.cache);
    nclistfree(dapcomm->cdf.varnodes);
    nclistfree(dapcomm->cdf.seqnodes);
    nclistfree(dapcomm->cdf.gridnodes);
    nclistfree(dapcomm->cdf.usertypes);
    nullfree(dapcomm->cdf.recorddim);

    /* free the trees */
    freecdfroot34(dapcomm->cdf.ddsroot);
    dapcomm->cdf.ddsroot = NULL;
    if(dapcomm->oc.ocdasroot != NULL)
        oc_root_free(dapcomm->oc.conn,dapcomm->oc.ocdasroot);
    dapcomm->oc.ocdasroot = NULL;
    oc_close(dapcomm->oc.conn); /* also reclaims remaining OC trees */
    ocurifree(dapcomm->oc.uri);
    nullfree(dapcomm->oc.urltext);

    dcefree((DCEnode*)dapcomm->oc.dapconstraint);
    dapcomm->oc.dapconstraint = NULL;

    return NC_NOERR;
}

#ifdef IGNORE
/* Given a path, collect the set of dimensions along that path */
static void
collectdims3(NClist* path, NClist* dimset)
{
    int i,j;
    nclistclear(dimset);
    for(i=0;i<nclistlength(path);i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(node->nctype == NC_Sequence) {
	    CDFnode* sqdim = (CDFnode*)nclistget(node->array.dimensions,0);
	    if(DIMFLAG(sqdim,CDFDIMUNLIM))
		nclistclear(dimset); /* unlimited is always first */
        }
	for(j=0;j<nclistlength(node->array.dimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
	    nclistpush(dimset,(ncelem)dim);
	}
	if(node->array.stringdim != NULL) 
	    nclistpush(dimset,(ncelem)node->array.stringdim);
    }
}
#endif

NCerror
addstringdims(NCDAPCOMMON* dapcomm)
{
    /* for all variables of string type, we will need another dimension
       to represent the string; Accumulate the needed sizes and create
       the dimensions with a specific name: either as specified
       in DODS{...} attribute set or defaulting to the variable name.
       All such dimensions are global.
    */
    int i;
    NClist* varnodes = dapcomm->cdf.varnodes;
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	CDFnode* sdim = NULL;
	char dimname[4096];
	size_t dimsize;

	if(var->etype != NC_STRING && var->etype != NC_URL) continue;
	/* check is a string length was specified */
	if(var->dodsspecial.maxstrlen > 0)
	    dimsize = var->dodsspecial.maxstrlen;
	else
	    dimsize = var->maxstringlength;
	/* create a psuedo dimension for the charification of the string*/
	if(var->dodsspecial.dimname != NULL) {
	    strncpy(dimname,var->dodsspecial.dimname,sizeof(dimname));
	} else {
	    snprintf(dimname,sizeof(dimname),"maxStrlen%lu",
			(unsigned long)dimsize);
	}
	sdim = makecdfnode34(dapcomm, dimname, OC_Dimension, OCNULL,
                             dapcomm->cdf.ddsroot);
	if(sdim == NULL) return THROW(NC_ENOMEM);
	nclistpush(dapcomm->cdf.ddsroot->tree->nodes,(ncelem)sdim);
	sdim->dim.dimflags |= CDFDIMSTRING;
	sdim->dim.declsize = dimsize;
	nullfree(sdim->ncbasename);
	nullfree(sdim->ncfullname);
	sdim->ncbasename = cdflegalname3(sdim->name);
	sdim->ncfullname = nulldup(sdim->ncbasename);
	/* tag the variable with its string dimension*/
	var->array.stringdim = sdim;
    }
    return NC_NOERR;
}

NCerror
defrecorddim3(NCDAPCOMMON* dapcomm)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    NClist* alldims;

    ASSERT((dapcomm->cdf.recorddim != NULL));

    /* Locate the dimension matching the record dim */
    alldims = getalldims3(dapcomm->cdf.varnodes,1);
    for(i=0;i<nclistlength(alldims);i++) {
        CDFnode* dim = (CDFnode*)nclistget(alldims,i);
	if(dim->nctype != NC_Dimension) continue;    
	if(dim->dim.basedim != NULL) continue; /* not the controlling dim */
	if(strcmp(dim->name,dapcomm->cdf.recorddim) != 0) continue;
	if(DIMFLAG(dim,CDFDIMCLONE)) PANIC("cloned record dim");
	if(dapcomm->cdf.unlimited != NULL) PANIC("Multiple unlimited");
        DIMFLAGSET(dim,CDFDIMUNLIM|CDFDIMRECORD);
	dapcomm->cdf.unlimited = dim;
    }
    nclistfree(alldims);
    /* Now, locate all the string dims and see if they are the record dim,
       then replace */
    if(dapcomm->cdf.unlimited != NULL) {
	CDFnode* unlim = dapcomm->cdf.unlimited;
        for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
            CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
	    CDFnode* sdim = var->array.stringdim;
            if(sdim == NULL) continue;
	    if(strcmp(sdim->ncfullname,unlim->ncfullname)==0
	       && sdim->dim.declsize == unlim->dim.declsize) {
	        var->array.stringdim = unlim;
	        nclistpop(var->array.dimensions);
	        nclistpush(var->array.dimensions,(ncelem)dapcomm->cdf.unlimited);
	    }
	}
    }

    return ncstat;
}

NCerror
defseqdims(NCDAPCOMMON* dapcomm)
{
    unsigned int i;
    CDFnode* unlimited = NULL;
    NCerror ncstat = NC_NOERR;
    int seqdims = 1; /* default is to compute seq dims counts */

    /* Does the user want to see which dims are sequence dims? */
    if(paramcheck34(dapcomm,"show","seqdims")) seqdims = 0;
  
    /* Build the unlimited node if needed */
    if(!FLAGSET(dapcomm->controls,NCF_NOUNLIM)) {
        unlimited = makecdfnode34(dapcomm,"unlimited",OC_Dimension,OCNULL,dapcomm->cdf.ddsroot);
        nclistpush(dapcomm->cdf.ddsroot->tree->nodes,(ncelem)unlimited);
        nullfree(unlimited->ncbasename);
        nullfree(unlimited->ncfullname);
        unlimited->ncbasename = cdflegalname3(unlimited->name);
        unlimited->ncfullname = nulldup(unlimited->ncbasename);
        DIMFLAGSET(unlimited,CDFDIMUNLIM);
        dapcomm->cdf.unlimited = unlimited;
    }

    /* Compute and define pseudo dimensions for all sequences */

    for(i=0;i<nclistlength(dapcomm->cdf.seqnodes);i++) {
        CDFnode* seq = (CDFnode*)nclistget(dapcomm->cdf.seqnodes,i);
	CDFnode* sqdim;
	size_t seqsize;

        seq->array.dimensions = nclistnew();

	if(!seq->usesequence) {
	    /* Mark sequence with unlimited dimension */
	    seq->array.seqdim = unlimited;
	    nclistpush(seq->array.dimensions,(ncelem)unlimited);
	    continue;
	}

	/* Does the user want us to compute the sequence dim size? */
	sqdim = NULL;
	if(seqdims) {
	    ncstat = getseqdimsize(dapcomm,seq,&seqsize);
	    if(ncstat != NC_NOERR) {
                /* Cannot get DATADDDS; convert to unlimited */
		sqdim = unlimited;
	    }
	} else { /* !seqdims default to size = 1 */
	    seqsize = 1; 
	}
	if(sqdim == NULL) {
	    /* Note: we are making the dimension in the dds root tree */
            ncstat = makeseqdim(dapcomm,seq,seqsize,&sqdim);
            if(ncstat) goto fail;
	}
        seq->array.seqdim = sqdim;
	nclistpush(seq->array.dimensions,(ncelem)sqdim);
    }

fail:
    return ncstat;
}

static NCerror
getseqdimsize(NCDAPCOMMON* dapcomm, CDFnode* seq, size_t* sizep)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCconnection conn = dapcomm->oc.conn;
    OCdata rootcontent = OCNULL;
    OCobject ocroot;
    CDFnode* dxdroot;
    CDFnode* xseq;
    NCbytes* minconstraints = ncbytesnew();
    size_t seqsize;

    /* Read the minimal amount of data in order to get the count */
    /* If the url is unconstrainable, then get the whole thing */
    computeminconstraints3(dapcomm,seq,minconstraints);
#ifdef DEBUG
fprintf(stderr,"minconstraints: %s\n",ncbytescontents(minconstraints));
#endif
    /* Obtain the record counts for the sequence */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
        ocstat = dap_oc_fetch(dapcomm,conn,NULL,OCDATADDS,&ocroot);
    else
        ocstat = dap_oc_fetch(dapcomm,conn,ncbytescontents(minconstraints),OCDATADDS,&ocroot);
    if(ocstat) goto fail;
    ncstat = buildcdftree34(dapcomm,ocroot,OCDATA,&dxdroot);
    if(ncstat) goto fail;	
    /* attach DATADDS to DDS */
    ncstat = attach34(dxdroot,seq);
    if(ncstat) goto fail;	
    /* WARNING: we are now switching to datadds tree */
    xseq = seq->attachment;
    ncstat = countsequence(dapcomm,xseq,&seqsize);
    if(ncstat) goto fail;
#ifdef DEBUG
fprintf(stderr,"sequencesize: %s = %lu\n",seq->name,(unsigned long)seqsize);
#endif
    /* throw away the fetch'd trees */
    unattach34(dapcomm->cdf.ddsroot);
    freecdfroot34(dxdroot);
    if(ncstat != NC_NOERR) {
        /* Cannot get DATADDDS; convert to unlimited */
	char* code;
	char* msg;
	long httperr;
	oc_svcerrordata(dapcomm->oc.conn,&code,&msg,&httperr);
	if(code != NULL) {
	    nclog(NCLOGERR,"oc_fetch_datadds failed: %s %s %l",
			code,msg,httperr);
	}
	ocstat = OC_NOERR;
    }		
    if(sizep) *sizep = seqsize;

fail:
    ncbytesfree(minconstraints);
    oc_data_free(conn,rootcontent);
    if(ocstat) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

static NCerror
makeseqdim(NCDAPCOMMON* dapcomm, CDFnode* seq, size_t count, CDFnode** sqdimp)
{
    CDFnode* sqdim;
    CDFnode* root = seq->root;
    CDFtree* tree = root->tree;

    /* build the dimension with given size */
    sqdim = makecdfnode34(dapcomm,seq->name,OC_Dimension,OCNULL,root);
    if(sqdim == NULL) return THROW(NC_ENOMEM);
    nclistpush(tree->nodes,(ncelem)sqdim);
    nullfree(sqdim->ncbasename);
    nullfree(sqdim->ncfullname);
    sqdim->ncbasename = cdflegalname3(seq->name);
    sqdim->ncfullname = nulldup(sqdim->ncbasename);
    sqdim->dim.declsize = count;
    DIMFLAGSET(sqdim,CDFDIMSEQ);
    sqdim->dim.array = seq;
    if(sqdimp) *sqdimp = sqdim;
    return NC_NOERR;
}

static NCerror
countsequence(NCDAPCOMMON* dapcomm, CDFnode* xseq, size_t* sizep)
{
    unsigned int i;
    NClist* path = nclistnew();
    OCdata parent = OCNULL;
    OCdata child = OCNULL;
    OCdata tmp;
    CDFnode* prev = NULL;
    int index;
    OCerror ocstat = OC_NOERR;
    NCerror ncstat = NC_NOERR;
    OCconnection conn = dapcomm->oc.conn;
    size_t recordcount;
    CDFnode* xroot;

    ASSERT((xseq->nctype == NC_Sequence));

    parent = oc_data_new(conn);
    child = oc_data_new(conn);

    collectnodepath3(xseq,path,WITHDATASET);

    prev = (CDFnode*)nclistget(path,0);
    ASSERT((prev->nctype == NC_Dataset));

    xroot = xseq->root;
    ocstat = oc_data_root(conn,xroot->tree->ocroot,parent);
    if(ocstat) goto fail;

    for(i=1;i<nclistlength(path);i++) {
	xseq = (CDFnode*)nclistget(path,i);
	index = fieldindex(prev,xseq);
	ocstat = oc_data_ith(conn,parent,index,child);
	if(ocstat) goto fail;
	prev = xseq;
	/* swap the content markers */
	tmp = parent;
	parent = child;
	child = tmp;
    }
    oc_data_count(conn,parent,&recordcount);
    if(sizep) *sizep = recordcount;

fail:
    nclistfree(path);
    if(ocstat) ncstat = ocerrtoncerr(ocstat);
    oc_data_free(conn,parent);
    oc_data_free(conn,child);
    return ncstat;
}

static int
fieldindex(CDFnode* parent, CDFnode* child)
{
    unsigned int i;
    for(i=0;i<nclistlength(parent->subnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(parent->subnodes,i);
	if(node == child) return i;
    }
    return -1;
}

NCerror
showprojection3(NCDAPCOMMON* dapcomm, CDFnode* var)
{
    int i,rank;
    NCerror ncstat = NC_NOERR;
    NCbytes* projection = ncbytesnew();
    NClist* path = nclistnew();
    NC* drno = dapcomm->controller;

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
    ncstat = nc_put_att_text(getncid(drno),var->ncid,
                               "_projection",
		               ncbyteslength(projection),
			       ncbytescontents(projection));
    return ncstat;
}

#ifdef IGNORE
NCerror
detachdatadds3(NCDAPCOMMON* dapcomm)
{
    int i;
    for(i=0;i<nclistlength(dapcomm->cdf.dds->tree.nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(dapcomm->cdf.dds->tree.nodes,i);
	node->active = 0;
	node->dim.datasize = node->dim.declsize;
   }
   return NC_NOERR;
}

NCerror
attachdatadds3(NCDAPCOMMON* dapcomm)
{
    int i;
    NClist* cdfnodes = dapcomm->cdf.dds->tree.nodes;
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
	OCobject dds = node->dds;
	if(dds == OCNULL) continue;
	node->active = oc_datadds_active(dapcomm->oc.conn,dds);
	if(node->nctype == NC_Dimension) {
	    oc_datadds_dimsize(dapcomm->oc.conn,node->dds,&node->dim.datasize);
	}
    }
    return NC_NOERR;
}
#endif

/*
This is more complex than one might think. We want to find
a path to a variable inside the given node so that we can
ask for a single instance of that variable to minimize the
amount of data we retrieve. However, we want to avoid passing
through any nested sequence. This is possible because of the way
that sequencecheck() works.
*/
static NCerror
computeminconstraints3(NCDAPCOMMON* dapcomm, CDFnode* seq, NCbytes* minconstraints)
{
    NClist* path = nclistnew();
    CDFnode* var;
    CDFnode* candidate;
    unsigned int i,j,ndims;
    char* prefix;

    /* Locate a variable that is inside this sequence */
    /* Preferably one that is a numeric type*/
    for(candidate=NULL,var=NULL,i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
	if(node->array.sequence == seq) {
	    if(node->nctype == NC_Primitive) {
		switch(node->etype) {
		case NC_BYTE: case NC_SHORT: case NC_INT:
		case NC_FLOAT: case NC_DOUBLE:
		case NC_UBYTE: case NC_USHORT: case NC_UINT:
		case NC_INT64: case NC_UINT64:
		    if(var == NULL) {
			var = node; /* good choice */
		    }
		    break;
		case NC_CHAR: case NC_STRING:
		default:
		    candidate = node; /* usable */
		    break;
		}
	    }
	}
    }
    if(var == NULL && candidate != NULL) var = candidate;
    else if(var == NULL) return THROW(NC_EINVAL);

    /* collect seq path prefix */
    prefix = makecdfpathstring3(seq->container,".");
    ncbytescat(minconstraints,prefix);
    if(strlen(prefix) > 0) ncbytescat(minconstraints,".");

    /* Compute a short path from the var back to and including
       the sequence
    */
    collectnodepath3(var,path,WITHOUTDATASET);
    while(nclistlength(path) > 0) {
	CDFnode* node = (CDFnode*)nclistget(path,0);
	if(node == seq) break;
	nclistremove(path,0);
    }
    ASSERT((nclistlength(path) > 0));

    /* construct the projection path using minimal index values */
    for(i=0;i<nclistlength(path);i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(minconstraints,".");
	ncbytescat(minconstraints,node->name);
	if(node == seq) {
	    /* Use the limit */
	    if(node->sequencelimit > 0) {
		char tmp[64];
		snprintf(tmp,sizeof(tmp),"[0:%lu]",
		         (unsigned long)(node->sequencelimit - 1));
		ncbytescat(minconstraints,tmp);
	    }
	} else if(nclistlength(node->array.dimensions) > 0) {
	    ndims = nclistlength(node->array.dimensions);
	    for(j=0;j<ndims;j++) {
		CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
		if(dim->dim.dimflags & CDFDIMSTRING) {
		    ASSERT((j == (ndims - 1)));
		    break;
		}
		ncbytescat(minconstraints,"[0]");
	    }
	}
    }
    nclistfree(path);
    /* Finally, add in any selection from the original URL */
    if(dapcomm->oc.uri->selection != NULL)
        ncbytescat(minconstraints,dapcomm->oc.uri->selection);
    nullfree(prefix);
    return NC_NOERR;
}

static unsigned long
cdftotalsize3(NClist* dimensions)
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

/* Estimate variables sizes and then resort the variable list
   by that size
*/
void
estimatevarsizes3(NCDAPCOMMON* dapcomm)
{
    int ivar;
    unsigned int rank;
    size_t totalsize = 0;

    for(ivar=0;ivar<nclistlength(dapcomm->cdf.varnodes);ivar++) {
        CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,ivar);
	NClist* ncdims = var->array.dimensions;
	rank = nclistlength(ncdims);
	if(rank == 0) { /* use instance size of the type */
	    var->estimatedsize = nctypesizeof(var->etype);
#ifdef DEBUG
fprintf(stderr,"scalar %s.estimatedsize = %lu\n",
	makecdfpathstring3(var,"."),var->estimatedsize);
#endif
	} else {
	    unsigned long size = cdftotalsize3(ncdims);
	    size *= nctypesizeof(var->etype);
#ifdef DEBUG
fprintf(stderr,"array %s(%u).estimatedsize = %lu\n",
	makecdfpathstring3(var,"."),rank,size);
#endif
	    var->estimatedsize = size;
	}
	totalsize += var->estimatedsize;
    }
#ifdef DEBUG
fprintf(stderr,"total estimatedsize = %lu\n",totalsize);
#endif
    dapcomm->cdf.totalestimatedsize = totalsize;
}

NCerror
fetchtemplatemetadata3(NCDAPCOMMON* dapcomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCobject ocroot = OCNULL;
    CDFnode* ddsroot = NULL;
    char* ce = NULL;

    /* Temporary hack: we need to get the selection string
       from the url
    */
    /* Get (almost) unconstrained DDS; In order to handle functions
       correctly, those selections must always be included
    */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    else
        ce = nulldup(dapcomm->oc.uri->selection);

    /* Get selection constrained DDS */
    ocstat = dap_oc_fetch(dapcomm,dapcomm->oc.conn,ce,OCDDS,&ocroot);
    if(ocstat != OC_NOERR) {
	/* Special Hack. If the protocol is file, then see if
           we can get the dds from the .dods file
        */
	if(strcmp(dapcomm->oc.uri->protocol,"file") != 0) {
	    THROWCHK(ocstat); goto done;
	}
	/* Fetch the data dds */
        ocstat = dap_oc_fetch(dapcomm,dapcomm->oc.conn,ce,OCDATADDS,&ocroot);
        if(ocstat != OC_NOERR) {
	    THROWCHK(ocstat); goto done;
	}
	/* Note what we did */
	nclog(NCLOGWARN,"Cannot locate .dds file, using .dods file");
    }

    /* Get selection constrained DAS */
    ocstat = dap_oc_fetch(dapcomm,dapcomm->oc.conn,ce,OCDAS,&dapcomm->oc.ocdasroot);
    if(ocstat != OC_NOERR) {
	/* Ignore but complain */
	nclog(NCLOGWARN,"Could not read DAS; ignored");
        dapcomm->oc.ocdasroot = OCNULL;	
	ocstat = OC_NOERR;
    }

    /* Construct our parallel dds tree */
    ncstat = buildcdftree34(dapcomm,ocroot,OCDDS,&ddsroot);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    dapcomm->cdf.fullddsroot = ddsroot;

#ifdef NOTUSED
    /* Combine DDS and DAS */
    ncstat = dapmerge3(dapcomm,ddsroot,dapcomm->oc.ocdasroot);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
#endif

done:
    nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

NCerror
fetchconstrainedmetadata3(NCDAPCOMMON* dapcomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCobject ocroot;
    CDFnode* ddsroot; /* constrained */
    char* ce = NULL;

    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    else
        ce = buildconstraintstring3(dapcomm->oc.dapconstraint);

#ifdef NOTUSED
    if(ce == NULL || strlen(ce) == 0) {
	/* no need to get the dds again; just imprint on self */
        ncstat = imprintself3(dapcomm->cdf.ddsroot);
        if(ncstat) goto fail;
    } else
#endif
    {
        ocstat = dap_oc_fetch(dapcomm,dapcomm->oc.conn,ce,OCDDS,&ocroot);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}

        /* Construct our parallel dds tree; including attributes*/
        ncstat = buildcdftree34(dapcomm,ocroot,OCDDS,&ddsroot);
        if(ncstat) goto fail;

	dapcomm->cdf.ddsroot = ddsroot;

        if(!FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE)) {
            /* fix DAP server problem by adding back any missing grid structure nodes */
            ncstat = regrid3(ddsroot,dapcomm->cdf.fullddsroot,dapcomm->oc.dapconstraint->projections);    
            if(ncstat) goto fail;
	}

#ifdef DEBUG
fprintf(stderr,"constrained:\n%s",dumptree(ddsroot));
#endif

        /* Combine DDS and DAS */
	if(dapcomm->oc.ocdasroot != NULL) {
            ncstat = dapmerge3(dapcomm,ddsroot,dapcomm->oc.ocdasroot);
            if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
	}

        /* map the constrained DDS to the unconstrained DDS */
        ncstat = mapnodes3(ddsroot,dapcomm->cdf.fullddsroot);
        if(ncstat) goto fail;
#ifdef NOTUSED
        /* Imprint the constrained DDS data over the unconstrained DDS */
        ncstat = imprint3(dapcomm->cdf.ddsroot,ddsroot);
        if(ncstat) goto fail;
        /* Throw away the constrained DDS */
        freecdfroot34(ddsroot);
#endif
    }

fail:
    nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

/* Suppress variables not in usable sequences */
NCerror
suppressunusablevars3(NCDAPCOMMON* dapcomm)
{
    int i,j;
    int found = 1;
    NClist* path = nclistnew();
    while(found) {
	found = 0;
	for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	    CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
	    /* See if this var is under an unusable sequence */
	    nclistclear(path);
	    collectnodepath3(var,path,WITHOUTDATASET);
	    for(j=0;j<nclistlength(path);j++) {
		CDFnode* node = (CDFnode*)nclistget(path,j);
		if(node->nctype == NC_Sequence
		   && !node->usesequence) {
#ifdef DEBUG
fprintf(stderr,"suppressing sequence var: %s\n",node->ncfullname);
#endif
		    nclistremove(dapcomm->cdf.varnodes,i);
		    found = 1;
		    break;
		}
	    }
	    if(found) break;
	}
    }
    nclistfree(path);
    return NC_NOERR;
}


/*
For variables which have a zero size dimension,
either use unlimited, or make them invisible.
*/
NCerror
fixzerodims3(NCDAPCOMMON* dapcomm)
{
    int i,j;
    for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
        NClist* ncdims = var->array.dimensions;
	if(nclistlength(ncdims) == 0) continue;
        for(j=0;j<nclistlength(ncdims);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(ncdims,j);
	    if(DIMFLAG(dim,CDFDIMUNLIM)) continue;
	    if(dim->dim.declsize == 0) {
		if(j == 0) {/* can make it unlimited */
		    nclistset(ncdims,j,(ncelem)dapcomm->cdf.unlimited);
		} else { /* make node invisible */
		    var->visible = 0;
		    var->zerodim = 1;
		}
	    }
	}
    }
    return NC_NOERR;
}

void
applyclientparamcontrols3(NCDAPCOMMON* dapcomm)
{
    const char* value;
    OCURI* uri = dapcomm->oc.uri;

    /* enable/disable caching */
    value = ocurilookup(uri,"cache");    
    if(value == NULL)
	SETFLAG(dapcomm->controls,DFALTCACHEFLAG);
    else if(strlen(value) == 0)
	SETFLAG(dapcomm->controls,NCF_CACHE);
    else if(strcmp(value,"1")==0 || value[0] == 'y')
	SETFLAG(dapcomm->controls,NCF_CACHE);

    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
	SETFLAG(dapcomm->controls,NCF_CACHE);

    nclog(NCLOGNOTE,"Caching=%d",FLAGSET(dapcomm->controls,NCF_CACHE));

    SETFLAG(dapcomm->controls,(NCF_NC3|NCF_NCDAP));

}

/* Accumulate a set of all the known dimensions */
NClist*
getalldims3(NClist* vars, int visibleonly)
{
    int i,j;
    NClist* dimset = nclistnew();

    /* get bag of all dimensions */
    for(i=0;i<nclistlength(vars);i++) {
	CDFnode* var = (CDFnode*)nclistget(vars,i);
	if(!visibleonly || var->visible) {
            NClist* vardims = var->array.dimensions;
   	    for(j=0;j<nclistlength(vardims);j++) {
	        CDFnode* dim = (CDFnode*)nclistget(vardims,j);
	        nclistpush(dimset,(ncelem)dim);
	    }
	}
    }
    /* make unique */
    nclistunique(dimset);
    return dimset;
}
