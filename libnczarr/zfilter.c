/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */
/**
 * @file @internal Internal functions for filters
 *
 * This file contains functions internal to the netcdf4 library. None of
 * the functions in this file are exposed in the exetnal API. These
 * functions all relate to the manipulation of netcdf-4 filters
 *
 * @author Dennis Heimbigner
 */

#include "config.h"
#include <stdlib.h>
#include "zincludes.h"

/* Mnemonic */
#define FILTERACTIVE 1

/* WARNING: GLOBAL VARIABLE */

#ifdef ENABLE_CLIENT_FILTERS
/* Define list of registered filters */
static NClist* NCZ_registeredfilters = NULL; /** List<NC_FILTER_CLIENT_ZARR*> */

/**************************************************/
/* Filter registration support */

static int
filterlookup(conbst char* id)
{
    int i;
    if(NCZ_registeredfilters == NULL)
	NCZ_registeredfilters = nclistnew();
    for(i=0;i<nclistlength(NCZ_registeredfilters);i++) {
	NCX_FILTER_CLIENT* x = nclistget(NCZ_registeredfilters,i);
	if(x != NULL && x->id == id) return i; /* return position */
    }
    return -1;
}

static void
reclaiminfo(NCX_FILTER_CLIENT* info)
{
    nullfree(info);
}

static int
filterremove(int pos)
{
    NCX_FILTER_CLIENT* info = NULL;
    if(NCZ_registeredfilters == NULL)
	return THROW(NC_EINVAL);
    if(pos < 0 || pos >= nclistlength(NCZ_registeredfilters))
	return THROW(NC_EINVAL);
    info = nclistget(NCZ_registeredfilters,pos);
    reclaiminfo(info);
    nclistremove(NCZ_registeredfilters,pos);
    return NC_NOERR;
}

static NCX_FILTER_CLIENT*
dupfilterinfo(NCX_FILTER_CLIENT* info)
{
    NCX_FILTER_CLIENT* dup = NULL;
    if(info == NULL) goto fail;
    if((dup = calloc(1,sizeof(NCX_FILTER_CLIENT))) == NULL) goto fail;
    *dup = *info;
    return dup;
fail:
    reclaiminfo(dup);
    return NULL;
}

int
NCZ_global_filter_action(int op, unsigned int id, NC_FILTER_OBJ_HDF5* infop)
{
    int stat = NC_NOERR;
    H5Z_class2_t* h5filterinfo = NULL;
    herr_t herr;
    int pos = -1;
    NCX_FILTER_CLIENT* dup = NULL;
    NCX_FILTER_CLIENT* elem = NULL;
    NCX_FILTER_CLIENT ncf;

    switch (op) {
    case NCFILTER_CLIENT_REG: /* Ignore id argument */
        if(infop == NULL) {stat = NC_EINVAL; goto done;}
	assert(NC_FILTER_FORMAT_HDF5 == infop->hdr.format);
	assert(NC_FILTER_SORT_CLIENT == infop->sort);
	elem = (NCX_FILTER_CLIENT*)&infop->u.client;
        h5filterinfo = elem->info;
        /* Another sanity check */
        if(id != h5filterinfo->id)
	    {stat = NC_EINVAL; goto done;}
	/* See if this filter is already defined */
	if((pos = filterlookup(id)) >= 0)
	    {stat = NC_ENAMEINUSE; goto done;} /* Already defined */
	if((herr = H5Zregister(h5filterinfo)) < 0)
	    {stat = NC_EFILTER; goto done;}
	/* Save a copy of the passed in info */
	ncf.id = id;
	ncf.info = elem->info;
	if((dup=dupfilterinfo(&ncf)) == NULL)
	    {stat = NC_ENOMEM; goto done;}		
	nclistpush(NCZ_registeredfilters,dup);	
	break;
    case NCFILTER_CLIENT_UNREG:
	if(id <= 0)
	    {stat = NC_EFILTER; goto done;}
	/* See if this filter is already defined */
	if((pos = filterlookup(id)) < 0)
	    {stat = NC_ENOFILTER; goto done;} /* Not defined */
	if((herr = H5Zunregister(id)) < 0)
	    {stat = NC_EFILTER; goto done;}
	if((stat=filterremove(pos))) goto done;
	break;
    case NCFILTER_CLIENT_INQ:
	if(infop == NULL) goto done;
        /* Look up the id in our local table */
   	if((pos = filterlookup(id)) < 0)
	    {stat = NC_ENOFILTER; goto done;} /* Not defined */
        elem = (NCX_FILTER_CLIENT*)nclistget(NCZ_registeredfilters,pos);
	if(elem == NULL) {stat = NC_EINTERNAL; goto done;}
	if(infop != NULL) {
	    infop->u.client = *elem;
	}
	break;
    default:
	{stat = NC_EINTERNAL; goto done;}	
    }
done:
    return THROW(stat);
} 
#endif /*ENABLE_CLIENT_FILTERS*/

/**************************************************/

/**
 * @internal Define filter settings. Called by nc_def_var_filter().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param id Filter ID
 * @param nparams Number of parameters for filter.
 * @param parms Filter parameters.
 *
 * @returns ::NC_NOERR for success
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNCZ Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_EFILTER Filter error.
 * @returns ::NC_EINVAL Invalid input
 * @author Dennis Heimbigner
 */
int
NCZ_filter_actions(int ncid, int varid, int op, void* args)
{
    int stat = NC_NOERR;
    NC_GRP_INFO_T *grp = NULL;
    NC_FILE_INFO_T *h5 = NULL;
    NC_VAR_INFO_T *var = NULL;
    NC_FILTERX_OBJ* obj = (NC_FILTERX_OBJ*)args;
    NC_FILTERX_SPEC* spec = NULL;
    char** xfilterids = NULL;
    size_t nfilters = 0;

    LOG((2, "%s: ncid 0x%x varid %d op=%d", __func__, ncid, varid, op));

    if(args == NULL) return THROW(NC_EINVAL);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var)))
	return THROW(stat);

    assert(h5 && var && var->hdr.id == varid);

    nfilters = nclistlength(var->filters);

    switch (op) {
    case NCFILTER_DEF: {
        if(obj->usort != NC_FILTER_UNION_SPEC) return THROW(NC_EFILTER);
        /* If the dataset has already been created, then it is too
         * late to set all the extra stuff. */
        if (!(h5->flags & NC_INDEF)) {stat = THROW(NC_EINDEFINE); goto done;}
        if (!var->ndims) goto done; /* For scalars, ignore */
        if (var->created)
             {stat = THROW(NC_ELATEDEF); goto done;}
        /* Filter => chunking */
        var->storage = NC_CHUNKED;
        /* Determine default chunksizes for this variable unless already specified */
        if(var->chunksizes && !var->chunksizes[0]) {
	    /* Should this throw error? */
	    if((stat = ncz_find_default_chunksizes2(grp, var)))
	        goto done;
        }
        spec = &obj->u.spec;
	if((stat = NC4_filterx_add(var,!FILTERACTIVE,spec->filterid,spec->nparams,(const char**)spec->params)))
            goto done;
    } break;
    case NCFILTER_FILTERIDS: {
        if(obj->usort != NC_FILTER_UNION_IDS) return THROW(NC_EFILTER);
	if((obj->u.ids.filterids = calloc(sizeof(char*),nfilters))==NULL)
	    {stat = NC_ENOMEM; goto done;}
        obj->u.ids.nfilters = nfilters;
        if(nfilters > 0) {
	    int k;
	    for(k=0;k<nfilters;k++) {
		NC_FILTERX_SPEC* f = (NC_FILTERX_SPEC*)nclistget(var->filters,k);
		if((obj->u.ids.filterids[k] = strdup(f->filterid)) == NULL)
	    	    {stat = NC_ENOMEM; goto done;}
	    }
	}
	} break;
    case NCFILTER_INFO: {
	int k,found;
        if(obj->usort != NC_FILTER_UNION_SPEC) return THROW(NC_EFILTER);
        for(found=0,k=0;k<nfilters;k++) {
	    NC_FILTERX_SPEC* f = (NC_FILTERX_SPEC*)nclistget(var->filters,k);
	    if(strcasecmp(f->filterid,obj->u.spec.filterid)==0) {
		obj->u.spec.nparams = f->nparams;
		obj->u.spec.active = f->active;
		if((stat = NC_filterx_copy(f->nparams,(const char**)f->params,&obj->u.spec.params)))
		    goto done;
		found = 1;
		break;
	    }
	}
	if(!found) {stat = NC_ENOFILTER; goto done;}
	} break;
    case NCFILTER_REMOVE:
        if (!(h5->flags & NC_INDEF)) return THROW(NC_EINDEFINE);
        if(obj->usort != NC_FILTER_UNION_SPEC) return THROW(NC_EFILTER);
	if((stat = NC4_filterx_lookup(var,obj->u.spec.filterid,&spec))) goto done;
	if(spec && spec->active)
	    {stat = NC_EFILTER; goto done;}
	if((stat = NC4_filterx_remove(var,obj->u.spec.filterid))) goto done;
	break;
    default:
	{stat = NC_EINTERNAL; goto done;}	
    }

done:
    NC_filterx_freestringvec(nfilters,xfilterids);
    return THROW(stat);
}
