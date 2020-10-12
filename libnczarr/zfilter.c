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

#undef TFILTERS

/**************************************************/
#ifdef TFILTERS
static void printfilter1(struct NCZ_Filter* nfs);
static void printfilter(struct NCZ_Filter* nfs, const char* tag, int line);
static void printfilterlist(NC_VAR_INFO_T* var, const char* tag, int line);
#define PRINTFILTER(nfs, tag) printfilter(nfs,tag,__LINE__)
#define PRINTFILTERLIST(var,tag) printfilterlist(var,tag,__LINE__)
#else
#define PRINTFILTER(nfs, tag)
#define PRINTFILTERLIST(var,tag)
#endif /*TFILTERS*/

/**************************************************/
/**
 * @file
 * @internal
 * Internal netcdf hdf5 filter functions.
 *
 * This file contains functions internal to the libhdf5 library.
 * None of the functions in this file are exposed in the exernal API. These
 * functions all relate to the manipulation of netcdf-4's var->filters list.
 *
 * @author Dennis Heimbigner
 */

int
NCZ_filter_freelist(NC_VAR_INFO_T* var)
{
    int i, stat=NC_NOERR;
    NClist* filters = (NClist*)var->filters;

    if(filters == NULL) goto done;
PRINTFILTERLIST(var,"free: before");
    /* Free the filter list backward */
    for(i=nclistlength(filters)-1;i>=0;i--) {
	struct NCZ_Filter* spec = nclistremove(filters,i);
	if(spec->nparams > 0) nullfree(spec->params);
	nullfree(spec);
    }
PRINTFILTERLIST(var,"free: after");
    nclistfree(filters);
    var->filters = NULL;
done:
    return stat;
}

static int
NCZ_filter_free(struct NCZ_Filter* spec)
{
    if(spec == NULL) goto done;
PRINTFILTER(spec,"free");
    if(spec->nparams > 0) nullfree(spec->params)
    free(spec);
done:
    return NC_NOERR;
}

int
NCZ_addfilter(NC_VAR_INFO_T* var, unsigned int id, size_t nparams, const unsigned int* params)
{
    int stat = NC_NOERR;
    struct NCZ_Filter* fi = NULL;
    int olddef = 0; /* 1=>already defined */
    NClist* flist = (NClist*)var->filters;
    
    if(nparams > 0 && params == NULL)
	{stat = NC_EINVAL; goto done;}
    
    if((stat=NCZ_filter_lookup(var,id,&fi))==NC_NOERR) {
	assert(fi != NULL);
        /* already exists */
	olddef = 1;	
    } else {
	stat = NC_NOERR;
        if((fi = calloc(1,sizeof(struct NCZ_Filter))) == NULL)
	    {stat = NC_ENOMEM; goto done;}
        fi->filterid = id;
	olddef = 0;
    }    
    fi->nparams = nparams;
    if(fi->params != NULL) {
	nullfree(fi->params);
	fi->params = NULL;
    }
    assert(fi->params == NULL);
    if(fi->nparams > 0) {
	if((fi->params = (unsigned int*)malloc(sizeof(unsigned int)*fi->nparams)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
        memcpy(fi->params,params,sizeof(unsigned int)*fi->nparams);
    }
    if(!olddef) {
        nclistpush(flist,fi);
PRINTFILTERLIST(var,"add");
    }
    fi = NULL; /* either way,its in the var->filters list */

done:
    if(fi) NCZ_filter_free(fi);    
    return THROW(stat);
}

int
NCZ_filter_remove(NC_VAR_INFO_T* var, unsigned int id)
{
    int k;
    NClist* flist = (NClist*)var->filters;

    /* Walk backwards */
    for(k=nclistlength(flist)-1;k>=0;k--) {
	struct NCZ_Filter* f = (struct NCZ_Filter*)nclistget(flist,k);
        if(f->filterid == id) {
	    /* Remove from variable */
    	    nclistremove(flist,k);
#ifdef TFILTERS
PRINTFILTERLIST(var,"remove");
fprintf(stderr,"\tid=%s\n",id);
#endif
	    /* Reclaim */
	    NCZ_filter_free(f);
	    return NC_NOERR;
	}
    }
    return NC_ENOFILTER;
}

int
NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, struct NCZ_Filter** specp)
{
    int i;
    NClist* flist = (NClist*)var->filters;
    
    if(flist == NULL) {
	if((flist = nclistnew())==NULL)
	    return NC_ENOMEM;
	var->filters = (void*)flist;
    }
    for(i=0;i<nclistlength(flist);i++) {
	struct NCZ_Filter* spec = nclistget(flist,i);
	if(id == spec->filterid) {
	    if(specp) *specp = spec;
	    return NC_NOERR;
	}
    }
    return NC_ENOFILTER;
}

#if 0
/**
 * @internal Remove a filter from filter list for a variable
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param id filter id to remove
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_ENOTINDEFINE Not in define mode.
 * @returns ::NC_EINVAL Invalid input
 * @author Dennis Heimbigner
 */
int
nc_var_filter_remove(int ncid, int varid, unsigned int filterid)
{
    NC_VAR_INFO_T *var = NULL;
    int stat;

    /* Get pointer to the var. */
    if ((stat = ncz_find_grp_file_var(ncid, varid, NULL, NULL, &var)))
        return stat;
    assert(var);

    stat = NC4_hdf5_filter_remove(var,filterid);

    return stat;
}
#endif

int
NCZ_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams,
                   const unsigned int* params)
{
    int stat = NC_NOERR;
    NC *nc;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    struct NCZ_Filter* oldspec = NULL;
    int havedeflate = 0;
#ifdef HAVE_H5Z_SZIP
    int haveszip = 0;
#endif

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    if((stat = NC_check_id(ncid,&nc))) return stat;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = ncz_find_grp_file_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

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

	/* Lookup incoming id to see if already defined */
        switch((stat=NCZ_filter_lookup(var,id,&oldspec))) {
	case NC_NOERR: break; /* already defined */
        case NC_ENOFILTER: break; /*not defined*/
        default: goto done;
	}
	/* See if deflate &/or szip is defined */
	switch ((stat = NCZ_filter_lookup(var,H5Z_FILTER_DEFLATE,NULL))) {
	case NC_NOERR: havedeflate = 1; break;
	case NC_ENOFILTER: havedeflate = 0; break;	
	default: goto done;
	}
#ifdef HAVE_H5Z_SZIP
	switch ((stat = NCZ_filter_lookup(var,H5Z_FILTER_SZIP,NULL))) {
	case NC_NOERR: haveszip = 1; break;
	case NC_ENOFILTER: haveszip = 0; break;	
	default: goto done;
	}
#endif /* HAVE_H5Z_SZIP */

	/* If incoming filter not already defined, then check for conflicts */
	if(oldspec == NULL) {
            if(id == H5Z_FILTER_DEFLATE) {
		int level;
                if(nparams != 1)
                    {stat = THROW(NC_EFILTER); goto done;}/* incorrect no. of parameters */
   	        level = (int)params[0];
                if (level < NC_MIN_DEFLATE_LEVEL || level > NC_MAX_DEFLATE_LEVEL)
                    {stat = THROW(NC_EINVAL); goto done;}
#ifdef HAVE_H5Z_SZIP
                /* If szip compression is already applied, return error. */
	        if(haveszip) {stat = THROW(NC_EINVAL); goto done;}
#endif
            }
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
                if ((stat = NCZ_adjust_var_cache(grp, var)))
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
        if((stat = NCZ_addfilter(var,id,nparams,params)))
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

done:
    return stat;
}

int
NCZ_inq_var_filter_ids(int ncid, int varid, size_t* nfiltersp, unsigned int* ids)
{
    int stat = NC_NOERR;
    NC *nc;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NClist* flist = NULL;
    size_t nfilters;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    if((stat = NC_check_id(ncid,&nc))) return stat;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = ncz_find_grp_file_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    flist = var->filters;

    nfilters = nclistlength(flist);
    if(nfilters > 0 && ids != NULL) {
	int k;
	for(k=0;k<nfilters;k++) {
	    struct NCZ_Filter* f = (struct NCZ_Filter*)nclistget(flist,k);
	    ids[k] = f->filterid;
	}
    }
    if(nfiltersp) *nfiltersp = nfilters;
 
done:
    return stat;

}

int
NCZ_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparamsp, unsigned int* params)
{
    int stat = NC_NOERR;
    NC *nc;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    struct NCZ_Filter* spec = NULL;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    if((stat = NC_check_id(ncid,&nc))) return stat;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = ncz_find_grp_file_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    if((stat = NCZ_filter_lookup(var,id,&spec))) goto done;
    if(nparamsp) *nparamsp = spec->nparams;
    if(params && spec->nparams > 0) {
	memcpy(params,spec->params,sizeof(unsigned int)*spec->nparams);
    }
 
done:
    return stat;

}


/**************************************************/
/* Debug functions */

#ifdef TFILTERS
static void
printfilter1(struct NCZ_Filter* nfs)
{
    int i;
    if(nfs == NULL) {
	fprintf(stderr,"{null}");
	return;
    }
    fprintf(stderr,"{%u,(%u)",nfs->filterid,(int)nfs->nparams);
    for(i=0;i<nfs->nparams;i++) {
      fprintf(stderr," %s",nfs->params[i]);
    }
    fprintf(stderr,"}");
}

static void
printfilter(struct NCZ_Filter* nfs, const char* tag, int line)
{
    fprintf(stderr,"%s: line=%d: ",tag,line);
    printfilter1(nfs);
    fprintf(stderr,"\n");
}

static void
printfilterlist(NC_VAR_INFO_T* var, const char* tag, int line)
{
    int i;
    const char* name;
    if(var == NULL) name = "null";
    else if(var->hdr.name == NULL) name = "?";
    else name = var->hdr.name;
    fprintf(stderr,"%s: line=%d: var=%s filters=",tag,line,name);
    if(var != NULL) {
        for(i=0;i<nclistlength((NClist*)var->filters);i++) {
	    struct NCZ_Filter* nfs = nclistget((NClist*)var->filters,i);
	    fprintf(stderr,"[%d]",i);
	    printfilter1(nfs);
	}
    }
    fprintf(stderr,"\n");
}
#endif /*TFILTERS*/
