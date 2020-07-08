/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */
/**
 * @file @internal Internal netcdf-4 functions for filters
 *
 * This file contains functions internal to the netcdf4 library. None of
 * the functions in this file are exposed in the exetnal API. These
 * functions all relate to the manipulation of netcdf-4 filters
 *
 * @author Dennis Heimbigner
 */

#include "config.h"
#include <stdlib.h>
#include "hdf5internal.h"
#include "hdf5debug.h"
#include "ncfilter.h"

#define HAVE_H5_DEFLATE

/* Mnemonic */
#define FILTERACTIVE 1


/**************************************************/
/* Filter registration support */

#ifdef ENABLE_CLIENTSIDE_FILTERS

/* WARNING: GLOBAL VARIABLE */
/* Define list of registered filters */
static NClist* NC4_registeredfilters = NULL; /** List<NC_FILTER_CLIENT_HDF5*> */

/**************************************************/
/* Filter registration support */

static int
clientfilterlookup(unsigned int id)
{
    int i;
    if(NC4_registeredfilters == NULL)
	NC4_registeredfilters = nclistnew();
    for(i=0;i<nclistlength(NC4_registeredfilters);i++) {
	NC_FILTER_CLIENT_HDF5* x = nclistget(NC4_registeredfilters,i);
	if(x != NULL && x->id == id) {
	    return i; /* return position */
	}
    }
    return -1;
}

static void
reclaiminfo(NC_FILTER_CLIENT_HDF5* info)
{
    nullfree(info);
}

static int
filterremove(int pos)
{
    NC_FILTER_CLIENT_HDF5* info = NULL;
    if(NC4_registeredfilters == NULL)
	return THROW(NC_EINVAL);
    if(pos < 0 || pos >= nclistlength(NC4_registeredfilters))
	return THROW(NC_EINVAL);
    info = nclistget(NC4_registeredfilters,pos);
    reclaiminfo(info);
    nclistremove(NC4_registeredfilters,pos);
    return NC_NOERR;
}

static NC_FILTER_CLIENT_HDF5*
dupfilterinfo(NC_FILTER_CLIENT_HDF5* info)
{
    NC_FILTER_CLIENT_HDF5* dup = NULL;
    if(info == NULL) goto fail;
    if((dup = calloc(1,sizeof(NC_FILTER_CLIENT_HDF5))) == NULL) goto fail;
    *dup = *info;
    return dup;
fail:
    reclaiminfo(dup);
    return NULL;
}

int
nc4_global_filter_action(int op, unsigned int id, NC_FILTER_OBJ_HDF5* infop)
{
    int stat = NC_NOERR;
    H5Z_class2_t* h5filterinfo = NULL;
    herr_t herr;
    int pos = -1;
    NC_FILTER_CLIENT_HDF5* dup = NULL;
    NC_FILTER_CLIENT_HDF5* elem = NULL;
    NC_FILTER_CLIENT_HDF5 ncf;

    NC_UNUSED(format);
    
    switch (op) {
    case NCFILTER_CLIENT_REG: /* Ignore id argument */
        if(infop == NULL) {stat = NC_EINVAL; goto done;}
	assert(NC_FILTER_FORMAT_HDF5 == infop->hdr.format);
	assert(NC_FILTER_SORT_CLIENT == infop->sort);
	elem = (NC_FILTER_CLIENT_HDF5*)&infop->u.client;
        h5filterinfo = elem->info;
        /* Another sanity check */
        if(id != h5filterinfo->id)
	    {stat = NC_EINVAL; goto done;}
	/* See if this filter is already defined */
	if((pos = clientfilterlookup(id)) >= 0)
	    {stat = NC_ENAMEINUSE; goto done;} /* Already defined */
	if((herr = H5Zregister(h5filterinfo)) < 0)
	    {stat = NC_EFILTER; goto done;}
	/* Save a copy of the passed in info */
	ncf.id = id;
	ncf.info = elem->info;
	if((dup=dupfilterinfo(&ncf)) == NULL)
	    {stat = NC_ENOMEM; goto done;}		
	nclistpush(NC4_registeredfilters,dup);	
	break;
    case NCFILTER_CLIENT_UNREG:
	if(id <= 0)
	    {stat = NC_ENOTNC4; goto done;}
	/* See if this filter is already defined */
	if((pos = clientfilterlookup(id)) < 0)
	    {stat = NC_ENOFILTER; goto done;} /* Not defined */
	if((herr = H5Zunregister(id)) < 0)
	    {stat = NC_EFILTER; goto done;}
	if((stat=filterremove(pos))) goto done;
	break;
    case NCFILTER_CLIENT_INQ:
	if(infop == NULL) goto done;
        /* Look up the id in our local table */
   	if((pos = clientfilterlookup(id)) < 0)
	    {stat = NC_ENOFILTER; goto done;} /* Not defined */
        elem = (NC_FILTER_CLIENT_HDF5*)nclistget(NC4_registeredfilters,pos);
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

#endif /*ENABLE_CLIENTSIDE_FILTERS*/

/**************************************************/

int
NC4_hdf5_addfilter(NC_VAR_INFO_T* var, int active, unsigned int id, size_t nparams, unsigned int* inparams)
{
    int stat = NC_NOERR;
    char* idx = NULL;
    char** paramsx = NULL;

    if((nparams > 0 && inparams == NULL) || id == 0)
        return THROW(NC_EINVAL);
    if((stat=NC_cvtI2X_id(id,&idx,0))) goto done;
    if(inparams != NULL) {
        if((paramsx = malloc(sizeof(char*)*nparams)) == NULL)
	    return THROW(NC_ENOMEM);
	if((stat = NC_cvtI2X_params(nparams,inparams,paramsx))) goto done;
    }
    if((stat = NC4_filterx_add(var,active,idx,nparams,(const char**)paramsx))) goto done;
done:
    nullfree(idx);
    NC_filterx_freestringvec(nparams,paramsx);
    return THROW(stat);
}

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
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_EFILTER Filter error.
 * @returns ::NC_EINVAL Invalid input
 * @author Dennis Heimbigner
 */
int
NC4_filter_actions(int ncid, int varid, int op, void* args)
{
    int stat = NC_NOERR;
    NC_GRP_INFO_T *grp = NULL;
    NC_FILE_INFO_T *h5 = NULL;
    NC_VAR_INFO_T *var = NULL;
    NC_FILTERX_OBJ* obj = (NC_FILTERX_OBJ*)args;
    unsigned int id = 0;
    size_t nparams = 0;
    unsigned int* params = NULL;
    size_t nfilters = 0;
    char* idx = NULL;
    NC_FILTERX_SPEC* oldspec = NULL;
    int haveszip;
#ifdef HAVE_H5Z_SZIP
    int havedeflate;
#endif /* HAVE_H5Z_SZIP */

    LOG((2, "%s: ncid 0x%x varid %d op=%d", __func__, ncid, varid, op));

    if(args == NULL) {stat = THROW(NC_EINVAL); goto done;}

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = nc4_hdf5_find_grp_h5_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    nfilters = nclistlength(var->filters);

    switch (op) {
    case NCFILTER_DEF: {
        if(obj->usort != NC_FILTER_UNION_SPEC)
	    {stat = THROW(NC_EFILTER); goto done;}
        /* If the HDF5 dataset has already been created, then it is too
         * late to set all the extra stuff. */
        if (!(h5->flags & NC_INDEF))
	    {stat = THROW(NC_EINDEFINE); goto done;}
        if (!var->ndims)
	    {stat = NC_EINVAL; goto done;} /* For scalars, complain */
        if (var->created)
             {stat = THROW(NC_ELATEDEF); goto done;}
        /* Can't turn on parallel and szip before HDF5 1.10.2. */
#ifdef USE_PARALLEL
#ifndef HDF5_SUPPORTS_PAR_FILTERS
        if (h5->parallel == NC_TRUE)
            {stat = THROW(NC_EINVAL); goto done;}
#endif /* HDF5_SUPPORTS_PAR_FILTERS */
#endif /* USE_PARALLEL */
	nparams = obj->u.spec.nparams;
	/* Lookup incoming id to see if already defined */
	if((stat = NC_cvtX2I_id(obj->u.spec.filterid,&id))) goto done;
	if((params = calloc(sizeof(unsigned int),nparams))==NULL) {stat = NC_ENOMEM; goto done;}
	if((stat = NC_cvtX2I_params(nparams,(const char**)obj->u.spec.params,params))) goto done;
	oldspec = NULL;
        switch((stat=NC4_filterx_lookup(var,obj->u.spec.filterid,&oldspec))) {
	case NC_NOERR: break; /* already defined */
        case NC_ENOFILTER: break; /*not defined*/
        default: goto done;
	}
	/* See if deflate &/or szip is defined */
	if((stat = NC_cvtI2X_id(H5Z_FILTER_DEFLATE,&idx,0))) goto done;
#ifdef HAVE_H5Z_SZIP
	switch ((stat = NC4_filterx_lookup(var,idx,NULL))) {
	case NC_NOERR: havedeflate = 1; break;
	case NC_ENOFILTER: havedeflate = 0; break;	
	default: goto done;
	}
	nullfree(idx); idx = NULL;
#endif /* HAVE_H5Z_SZIP */
	if((stat = NC_cvtI2X_id(H5Z_FILTER_SZIP,&idx,0))) goto done;
	switch ((stat = NC4_filterx_lookup(var,idx,NULL))) {
	case NC_NOERR: haveszip = 1; break;
	case NC_ENOFILTER: haveszip = 0; break;	
	default: goto done;
	}
	nullfree(idx); idx = NULL;

	/* If incoming filter not already defined, then check for conflicts */
	if(oldspec == NULL) {
#ifdef HAVE_H5_DEFLATE
            if(id == H5Z_FILTER_DEFLATE) {
		int level;
                if(nparams != 1)
                    {stat = THROW(NC_EFILTER); goto done;}/* incorrect no. of parameters */
   	        level = (int)params[0];
                if (level < NC_MIN_DEFLATE_LEVEL || level > NC_MAX_DEFLATE_LEVEL)
                    {stat = THROW(NC_EINVAL); goto done;}
                /* If szip compression is already applied, return error. */
	        if(haveszip) {stat = THROW(NC_EINVAL); goto done;}
            }
#else /*!HAVE_H5_DEFLATE*/
            if(id == H5Z_FILTER_DEFLATE)
                {stat = THROW(NC_EFILTER); /* Not allowed */ goto done;}
#endif
#ifdef HAVE_H5Z_SZIP
            if(id == H5Z_FILTER_SZIP) { /* Do error checking */
                if(nparams != 2)
                    {stat = THROW(NC_EFILTER); goto done;}/* incorrect no. of parameters */
                /* Pixels per block must be an even number, < 32. */
                if (params[1] % 2 || params[1] > NC_MAX_PIXELS_PER_BLOCK)
                    {stat = THROW(NC_EINVAL); goto done;}
                /* If zlib compression is already applied, return error. */
	        if(havedeflate) {stat = THROW(NC_EINVAL); goto done;}
            }
#else /*!HAVE_H5Z_SZIP*/
            if(id == H5Z_FILTER_SZIP)
                {stat = THROW(NC_EFILTER); goto done;} /* Not allowed */
#endif
            /* Filter => chunking */
	    var->storage = NC_CHUNKED;
            /* Determine default chunksizes for this variable unless already specified */
            if(var->chunksizes && !var->chunksizes[0]) {
	        /* Should this throw error? */
                if((stat = nc4_find_default_chunksizes2(grp, var)))
	            goto done;
                /* Adjust the cache. */
                if ((stat = nc4_adjust_var_cache(grp, var)))
                    goto done;
            }
	}
#ifdef HAVE_H5Z_SZIP
	/* More error checking */
        if(id == H5Z_FILTER_SZIP) { /* szip X chunking error checking */
	    /* For szip, the pixels_per_block parameter must not be greater
	     * than the number of elements in a chunk of data. */
            size_t num_elem = 1;
            int d;
            for (d = 0; d < var->ndims; d++)
                if (var->dim[d]->len)
		    num_elem *= var->dim[d]->len;
            /* Pixels per block must be <= number of elements. */
            if (params[1] > num_elem)
                {stat = THROW(NC_EINVAL); goto done;}
        }
#endif
	/* addfilter can handle case where filter is already defined, and will just replace parameters */
        if((stat = NC4_hdf5_addfilter(var,!FILTERACTIVE,id,nparams,params)))
                goto done;
#ifdef USE_PARALLEL
#ifdef HDF5_SUPPORTS_PAR_FILTERS
        /* Switch to collective access. HDF5 requires collevtive access
         * for filter use with parallel I/O. */
        if (h5->parallel)
            var->parallel_access = NC_COLLECTIVE;
#else
        if (h5->parallel)
            {stat = THROW(NC_EINVAL); goto done;}
#endif /* HDF5_SUPPORTS_PAR_FILTERS */
#endif /* USE_PARALLEL */
	} break;
    case NCFILTER_FILTERIDS: {
        if(obj->usort != NC_FILTER_UNION_IDS)
	     {stat = THROW(NC_EFILTER); goto done;}
	nfilters = nclistlength(var->filters);
	obj->u.ids.nfilters = nfilters;
        if(nfilters > 0) {
	    int k;
	    if((obj->u.ids.filterids = calloc(sizeof(char*),nfilters+1))==NULL)
	        {stat = NC_ENOMEM; goto done;}
	    for(k=0;k<nfilters;k++) {
		NC_FILTERX_SPEC* f = (NC_FILTERX_SPEC*)nclistget(var->filters,k);
		if((obj->u.ids.filterids[k] = strdup(f->filterid)) == NULL)
		    {stat = NC_ENOMEM; goto done;}
	    }
	}
	} break;
    case NCFILTER_INFO: {
	int k,found;
        if(obj->usort != NC_FILTER_UNION_SPEC)
	    {stat = THROW(NC_EFILTER); goto done;}
        for(found=0,k=0;k<nfilters;k++) {
	    NC_FILTERX_SPEC* f = (NC_FILTERX_SPEC*)nclistget(var->filters,k);
	    if(strcmp(f->filterid,obj->u.spec.filterid)==0) {
	        obj->u.spec.nparams = f->nparams;
		if(f->params != NULL && f->nparams > 0) {
		    if((stat=NC_filterx_copy(f->nparams,(const char**)f->params,&obj->u.spec.params))) goto done;
		}
		found = 1;
		break;
	    }
	}
	if(!found) {stat = NC_ENOFILTER; goto done;}
	} break;
    case NCFILTER_REMOVE: {
        if (!(h5->flags & NC_INDEF))
	    {stat = THROW(NC_EINDEFINE); goto done;}
        if(obj->usort != NC_FILTER_UNION_SPEC)
	    {stat = THROW(NC_EFILTER); goto done;}
	/* Lookup filter */
	if((stat = NC4_filterx_lookup(var,obj->u.spec.filterid,&oldspec))) goto done;
	if(oldspec && oldspec->active)/* Cannot remove */
	    {stat = NC_EFILTER; goto done;}
        if((stat = NC4_filterx_remove(var,obj->u.spec.filterid))) goto done;
	} break;
    default:
	{stat = NC_EINTERNAL; goto done;}	
    }

done:
    nullfree(idx);
    nullfree(params);
    return THROW(stat);
}

void
NC4_freefilterspec(NC_FILTERX_SPEC* f)
{
    if(f) {
        if(f->params != NULL) {NC_filterx_freestringvec(f->nparams,f->params);}
	free(f);
    }
}
