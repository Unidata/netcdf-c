/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "nccr.h"

#ifdef IGNORE
static NCerror crbuilddims(NCCR*);
static NCerror crbuildtypes(NCCR*);
static NCerror crbuildtypesr(NCCR*);
static NCerror crbuildvars(NCCR*);
static NCerror crbuildglobalattrs(NCCR*);
static NCerror crbuildattribute(NCCR*, int varid, int ncid);
#endif

/*
Fetch the metadata and define in the temporary netcdf-4 file
*/
NCerror
crbuildnc(NCCR* nccr)
{
    NCerror ncstat = NC_NOERR;
#ifdef IGNORE
    CDFnode* dds = nccr->cdmr->cdf.ddsroot;
    ncstat = crbuildglobalattrs(cdmr,getncid(cdmr),dds);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = crbuilddims(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = crbuildtypes(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    ncstat = crbuildvars(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
done:
#endif
    return THROW(ncstat);
}

#ifdef IGNORE
/* Define dim info for top-level dims */
static crNCerror
builddims(NCCR* nccr)
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

static crNCerror
buildtypes(NCCR* nccr)
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
buildtypes4r(NCCR* nccr, CDFnode* tnode)
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
static crNCerror
buildvars(NCCR* nccr)
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
		ncstat = crbuildattribute(cdmr,att,varid,ncid);
        	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	    }
	}
	/* Tag the variable with its DAP path cr*/
	if(paramcheck3(&nccr->dap,"show","projection"))
	    crshowprojection(&nccr->dap,var);
    }
    
done:
    return THROW(ncstat);
}

static crNCerror
buildglobalattrs(NCCR* nccr, int ncid, CDFnode* root)
{
    int i;
    const char* txt;
    char *nltxt, *p;
    NCerror ncstat = NC_NOERR;

    if(root->attributes != NULL) {
        for(i=0;i<nclistlength(root->attributes);i++) {
   	    NCattribute* att = (NCattribute*)nclistget(root->attributes,i);
	    ncstat = crbuildattribute(cdmr,att,NC_GLOBAL,ncid);
            if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	}
    }

    /* Define some additional system global attributes depending
       on show= clientparams*/
    /* Ignore doneures*/

    crif(paramcheck3(&nccr->dap,"show","translate")) {
        /* Add a global attribute to show the translation */
        ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_translate",
	           strlen("netcdf-4"),"netcdf-4");
    }

    crif(paramcheck3(&nccr->dap,"show","url")) {
	if(nccr->cdmr->oc.urltext != NULL)
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_url",
				       strlen(nccr->cdmr->oc.urltext),nccr->cdmr->oc.urltext);
    }
    crif(paramcheck3(&nccr->dap,"show","dds")) {
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
    crif(paramcheck3(&nccr->dap,"show","das")) {
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
crbuildattribute(NCCR* nccr, NCattribute* att, int varid, int ncid)
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

#endif
