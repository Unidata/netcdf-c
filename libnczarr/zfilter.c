/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * distribution tree, or in https://support.hdfgroup.org/ftp/hdf5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @file @internal Internal functions for filters
 *
 * This file contains functions internal to the netcdf4 library. None of
 * the functions in this file are exposed in the exetnal API. These
 * functions all relate to the manipulation of netcdf-4 filters
 *
 * @author Dennis Heimbigner
 *
 * This file is very similar to libhdf5/hdf5filters.c, so changes
 * should be propagated if needed.
 *
 */

#include "config.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "zincludes.h"
#include "zfilter.h"
#include "ncpathmgr.h"
#include "ncpoco.h"
#include "netcdf_filter.h"
#include "netcdf_aux.h"
#include "netcdf_filter_build.h"
#include "zfilter.h"
#include "zplugins.h"

#if 0
#define DEBUG
#define DEBUGF
#define DEBUGL
#endif

/* If set, then triage potential shared libraries based on extension */
#define NAMEOPT

#define NULLIFY(x) ((x)?(x):"NULL")

NCZ_Codec
NCZ_codec_empty(void)
{
    NCZ_Codec empty = {NULL, NULL, 0};
    return empty;
}

NCZ_HDF5
NCZ_hdf5_empty(void)
{
    NCZ_HDF5 empty = {0, {0,NULL}, {0,NULL}};
    return empty;
}

/**************************************************/

#define FILTERINCOMPLETE(f) ((f)->flags & FLAG_INCOMPLETE?1:0)
#define FILTERSUPPRESSED(f) ((f)->flags & FLAG_SUPPRESS?1:0)

/* WARNING: GLOBAL DATA */
/* TODO: move to common global state */

static int NCZ_filter_initialized = 0;

/**************************************************/

#ifdef ZTRACING
static const char*
NCJtrace(const NCjson* j)
{
    static char jstat[4096];
    char* js = NULL;
    jstat[0] = '\0';
    if(j) {
        (void)NCJunparse(j,0,&js);
	if(js) strlcat(jstat,js,sizeof(jstat));
	nullfree(js);
    }
    return jstat;
}

#define IEXISTS(x,p) (((x) && *(x)? (*(x))-> p : 0xffffffff))
#define SEXISTS(x,p) (((x) && *(x)? (*(x))-> p : "null"))
#endif

#if defined(DEBUGF) || defined(DEBUGL)

static char*
printparams(size_t nparams, const unsigned* params)
{
    static char ppbuf[4096];
    if(nparams == 0)
        snprintf(ppbuf,4096,"{0,%p}",params);
    else 
        snprintf(ppbuf,4096,"{%u %s}",(unsigned)nparams,nczprint_paramvector(nparams,params));
    return ppbuf;
}

static char*
printnczparams(const NCZ_Params p)
{
    return printparams(p.nparams,p.params);
}

static const char*
printcodec(const NCZ_Codec c)
{
    static char pcbuf[4096];
    snprintf(pcbuf,sizeof(pcbuf),"{id=%s codec=%s}",
		c.id,NULLIFY(c.codec));
    return pcbuf;
}

static const char*
printhdf5(const NCZ_HDF5 h)
{
    static char phbuf[4096];
    snprintf(phbuf,sizeof(phbuf),"{id=%u visible=%s working=%s}",
    		h.id, printnczparams(h.visible), printnczparams(h.working));
    return phbuf;
}

static const char*
printfilter(const NCZ_Filter* f)
{
    static char pfbuf[4096];

    if(f == NULL) return "NULL";
    snprintf(pfbuf,sizeof(pfbuf),"{flags=%d hdf5=%s codec=%s plugin=%p}",
		f->flags, printhdf5(f->hdf5),printcodec(f->codec),f->plugin);
    return pfbuf;
}

#endif


/* Forward */
static int NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, NCZ_Filter** specp);
static int ensure_working(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCZ_Filter* filter);
static int paramclone(unsigned** dstp, const unsigned* src, size_t nparams);
static int paramnczclone(NCZ_Params* dst, const NCZ_Params* src);
static int NCZ_filter_freelist1(NClist* filters);
static int NCZ_overwrite_filter(NC_FILE_INFO_T* file, NCZ_Filter* src, NCZ_Filter* dst);
static int checkfilterconflicts(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, unsigned id, size_t nparams, const unsigned int* params);

/**************************************************/
/**
 * @file
 * @internal
 * Internal netcdf nczarr filter functions.
 *
 * This file contains functions internal to the libnczarr library.
 * None of the functions in this file are exposed in the exernal API. These
 * functions all relate to the manipulation of netcdf-4's var->filters list.
 *
 * @author Dennis Heimbigner
 */

int
NCZ_filter_freelists(NC_VAR_INFO_T* var)
{
    int stat=NC_NOERR;

    ZTRACE(6,"var=%s",var->hdr.name);
    NCZ_filter_freelist1((NClist*)var->filters);
    var->filters = NULL;
    return ZUNTRACE(stat);
}

static int
NCZ_filter_freelist1(NClist* filters)
{
    int stat=NC_NOERR;
    size_t i;
    NCZ_Filter* spec = NULL;
    
    ZTRACE(6,"|filters|=%zu",nclistlength(filters));
    if(filters == NULL) goto done;
    /* Free the filter list elements */
    for(i=0;i<nclistlength(filters);i++) {
	spec = nclistget(filters,i);
	if((stat = NCZ_filter_free(spec))) goto done;
    }
    nclistfree(filters);
done:
    return ZUNTRACE(stat);
}

int
NCZ_filter_free(NCZ_Filter* spec)
{
    if(spec == NULL) return NC_NOERR;
    ncz_hdf5_clear(&spec->hdf5);
    ncz_codec_clear(&spec->codec);
    free(spec);
    return NC_NOERR;
}

/**************************************************/

int
NCZ_filter_remove(NC_VAR_INFO_T* var, unsigned int id)
{
    int stat = NC_NOERR;
    NClist* flist = (NClist*)var->filters;

    ZTRACE(6,"var=%s id=%u",var->hdr.name,id);
    /* Walk backwards */
    for(size_t k = nclistlength(flist); k-->0;) {
	NCZ_Filter* f = (NCZ_Filter*)nclistget(flist,k);
        if(f->hdf5.id == id) {
	    /* Remove from variable */
    	    nclistremove(flist,k);
	    /* Reclaim */
	    NCZ_filter_free(f); f = NULL;
	    goto done;
	}
    }
    stat = THROW(NC_ENOFILTER);
done:
    return ZUNTRACE(stat);
}

static int
NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, NCZ_Filter** specp)
{
    size_t i;
    NClist* flist = (NClist*)var->filters;
    
    ZTRACE(6,"var=%s id=%u",var->hdr.name,id);

    if(specp) *specp = NULL;
    if(flist == NULL) {
	if((flist = nclistnew())==NULL)
	    return NC_ENOMEM;
	var->filters = (void*)flist;
    }
    for(i=0;i<nclistlength(flist);i++) {
	NCZ_Filter* spec = nclistget(flist,i);
	assert(spec != NULL);
	if(spec->hdf5.id == id && !FILTERINCOMPLETE(spec)) {
	    if(specp) *specp = spec;
	    break;
	}
    }
    return ZUNTRACEX(NC_NOERR,"spec=%d",IEXISTS(specp,hdf5.id));
}

int
NCZ_plugin_lookup(const char* codecid, NCZ_Plugin** pluginp)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_Plugin* plugin = NULL;
    char digits[64];
    const char* trueid = NULL;
    struct NCglobalstate* gs = NC_getglobalstate();
    
    if(pluginp == NULL) return NC_NOERR;

    /* Find the plugin for this codecid */
    for(i=1;i<=gs->zarr.loaded_plugins_max;i++) {
	NCZ_Plugin* p = gs->zarr.loaded_plugins[i];
        if(p == NULL) continue;
        if(p == NULL|| p->codec.codec == NULL) continue; /* no plugin or no codec */
	if((p->codec.ishdf5raw)) {
	    /* get true id */
	    snprintf(digits,sizeof(digits),"%d",p->hdf5.filter->id);
	    trueid = digits;
	} else {
	    trueid = p->codec.codec->codecid;
	}
	if(strcmp(codecid, trueid) == 0)
	    {plugin = p; break;}
    }
    if(pluginp) *pluginp = plugin;
    return stat;
}

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
int
NCZ_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams,
                   const unsigned int* params)
{
    int stat = NC_NOERR;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NCZ_Filter* spec = NULL;

    ZTRACE(1,"ncid=%d varid=%d id=%u nparams=%u params=%s",ncid,varid,id,(unsigned)nparams,nczprint_paramvector(nparams,params));

    if((stat = NCZ_filter_initialize())) goto done;
    
    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    if (h5->parallel) {stat = THROW(NC_EINVAL); goto done;}

    /* If the NCZARR dataset has already been created, then it is too
     * late to set all the extra stuff. */
    if (!(h5->flags & NC_INDEF))
	{stat = THROW(NC_EINDEFINE); goto done;}
    if (!var->ndims)
	{stat = NC_EINVAL; goto done;} /* For scalars, complain */
    if (var->created)
        {stat = THROW(NC_ELATEDEF); goto done;}

    if((stat = checkfilterconflicts(h5,var,id,nparams,params))) goto done;
    if((spec = (NCZ_Filter*)calloc(1,sizeof(NCZ_Filter)))==NULL) {stat = NC_ENOMEM; goto done;}	

    /* Filter => chunking */
    var->storage = NC_CHUNKED;
    /* Determine default chunksizes for this variable unless already specified */
    if(var->chunksizes && !var->chunksizes[0]) {
	/* Should this throw error? */
	if((stat = nc4_find_default_chunksizes2(grp, var))) goto done;
    }

    /* addfilter can handle case where filter is already defined, and will just replace parameters */
    spec->hdf5.id = id;
    spec->hdf5.visible.nparams = nparams;
    if((stat = paramclone(&spec->hdf5.visible.params,params,nparams))) goto done;
    spec->flags |= FLAG_VISIBLE;
    if((stat = NCZ_addfilter(h5,var,&spec))) goto done; /* addfilter will control spec memory */

done:
    NCZ_filter_free(spec);
    return ZUNTRACE(stat);
}

int
NCZ_addfilter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCZ_Filter** fip)
{
    int stat = NC_NOERR;
    NCZ_Filter* fi = NULL;
    NCZ_Filter* oldfi = NULL;
    int exists = 0;

    ZTRACE(6,"file=%s var=%s",file->hdr.name,var->hdr.name);
    
    assert(fip != NULL && *fip != NULL);
    
    fi = *fip;

    if(fi->hdf5.visible.nparams > 0 && fi->hdf5.visible.params == NULL) {stat = NC_EINVAL; goto done;}

    /* Lookup incoming id to see if already defined */
    if((stat=NCZ_filter_lookup(var,fi->hdf5.id,&oldfi))) goto done;

    /* If incoming filter not already defined, then check for conflicts */
    if(oldfi == NULL) {
	if((stat = checkfilterconflicts(file,var,fi->hdf5.id,fi->hdf5.visible.nparams,fi->hdf5.visible.params))) goto done;
	exists = 0;
    } else
        exists = 1;

    /* Warning if filter already exists, fi will be changed to be that filter and old fi will be reclaimed */
    /* If it already exists, then overwrite the parameters */
    if(exists) {
	/* Overwrite old filter and let caller reclaim  *fip */
	if((stat = NCZ_overwrite_filter(file,fi,oldfi))) goto done;
    } else { /*!exists*/
	NCZ_Plugin* plugin = NULL;
        /* Find the matching plugin, if any */
        if((stat = NCZ_plugin_loaded(fi->hdf5.id,&plugin))) goto done;
	assert(fi->plugin == NULL || fi->plugin == plugin);
        fi->plugin = plugin;
        if(fi->plugin == NULL || plugin->incomplete) fi->incomplete = 1;
        /* Add to filters list  */
	if(var->filters == NULL) var->filters = nclistnew();
	nclistpush((NClist*)var->filters, fi);
	fi = NULL;
   }
   if(fip) *fip = fi; /* assert control */

done:
    return ZUNTRACE(stat);
}

static int
NCZ_overwrite_filter(NC_FILE_INFO_T* file, NCZ_Filter* src, NCZ_Filter* dst)
{
    int stat = NC_NOERR;

    NC_UNUSED(file);

    /* (over)write the HDF5 parameters */
    ncz_hdf5_clear(&dst->hdf5);
    ncz_codec_clear(&dst->codec);
    /* Fill in the hdf5 and codec*/
    dst->hdf5 = src->hdf5; /* get non-pointer fields */
    /* Avoid taking control of params */
    if((stat = paramclone(&dst->hdf5.visible.params,src->hdf5.visible.params,src->hdf5.visible.nparams))) goto done;
    assert(src->hdf5.working.nparams == 0 && src->hdf5.working.params == NULL);
    dst->codec = src->codec; /* get non-pointer fields */
    /* Avoid taking control of fields */    
    dst->codec.id = nulldup(src->codec.id);
    dst->codec.codec = nulldup(src->codec.codec);

done:
    return ZUNTRACE(stat);
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

    ZTRACE(1,"ncid=%d varid=%d",ncid,varid);

    if((stat = NC_check_id(ncid,&nc))) goto done;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var))) {stat = THROW(stat); goto done;}
    assert(h5 && var && var->hdr.id == varid);

    /* Make sure all the filters are defined */
    if((stat = NCZ_filter_initialize())) goto done;

    flist = var->filters;

    nfilters = nclistlength(flist); /* including incomplets */
    if(nfilters > 0 && ids != NULL) {
	size_t k;
	for(k=0;k<nfilters;k++) {
	    NCZ_Filter* f = (NCZ_Filter*)nclistget(flist,k);
            ids[k] = f->hdf5.id;
	}
    }
    if(nfiltersp) *nfiltersp = nfilters;
 
done:
    return ZUNTRACEX(stat, "nfilters=%u", nfilters);
}

int
NCZ_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparamsp, unsigned int* params)
{
    int stat = NC_NOERR;
    NC *nc;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NCZ_Filter* spec = NULL;

    ZTRACE(1,"ncid=%d varid=%d id=%u",ncid,varid,id);
    
    if((stat = NC_check_id(ncid,&nc))) goto done;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var))) {stat = THROW(stat); goto done;}
    assert(h5 && var && var->hdr.id == varid);

    /* Make sure all the plugins are defined */
    if((stat = NCZ_filter_initialize())) goto done;

    if((stat = NCZ_filter_lookup(var,id,&spec))) goto done;
    if(spec != NULL) {
	/* return the current visible parameters */
        if(nparamsp) *nparamsp = spec->hdf5.visible.nparams;
        if(params && spec->hdf5.visible.nparams > 0)
	    memcpy(params,spec->hdf5.visible.params,sizeof(unsigned int)*spec->hdf5.visible.nparams);
    } else {
        stat = THROW(NC_ENOFILTER);
    } 
done:
    return ZUNTRACEX(stat,"nparams=%u",(unsigned)(nparamsp?*nparamsp:0));
}

/* Test if a specific filter is available.
   @param file for which use of a filter is desired
   @param id the filter id of interest   
   @return NC_NOERR if the filter is available
   @return NC_ENOFILTER if the filter is not available
   @return NC_EBADID if ncid is invalid
   @return NC_EFILTER if ncid format does not support filters
*/
int
NCZ_inq_filter_avail(int ncid, unsigned id)
{
    int stat = NC_NOERR;
    struct NCZ_Plugin* plug = NULL;

    NC_UNUSED(ncid);
    ZTRACE(1,"ncid=%d id=%u",ncid,id);
    if((stat = NCZ_filter_initialize())) goto done;
    /* Check the available filters list */
    if((stat = NCZ_plugin_loaded((size_t)id, &plug))) goto done;
    if(plug == NULL || plug->incomplete)
        stat = THROW(NC_ENOFILTER);
done:
    return ZUNTRACE(stat);
}

static int
checkfilterconflicts(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, unsigned id, size_t nparams, const unsigned int* params)
{
    int stat = NC_NOERR;
    NCZ_Filter* deflate = NULL;
    NCZ_Filter* szip = NULL;
    int havedeflate, haveszip;

    NC_UNUSED(file);

    /* See if deflate &/or szip is defined */
    if((stat = NCZ_filter_lookup(var,H5Z_FILTER_DEFLATE,&deflate))) goto done;
    havedeflate = (deflate == NULL ? 0 : 1);
    stat = NC_NOERR; /* reset */

    if((stat = NCZ_filter_lookup(var,H5Z_FILTER_SZIP,&szip))) goto done;
    haveszip = (szip == NULL ? 0 : 1);
    stat = NC_NOERR; /* reset */
    
    if(id == H5Z_FILTER_DEFLATE) {
	int level;
	if(nparams != 1) {stat = THROW(NC_EFILTER); goto done;}/* incorrect no. of parameters */
	level = (int)params[0];
	if (level < NC_MIN_DEFLATE_LEVEL || level > NC_MAX_DEFLATE_LEVEL)
	    {stat = THROW(NC_EINVAL); goto done;}
	/* If szip compression is already applied, return error. */
	if(haveszip) {stat = THROW(NC_EINVAL); goto done;}
    }
    if(id == H5Z_FILTER_SZIP) { /* Do error checking */
	if(nparams != 2) {stat = THROW(NC_EFILTER); goto done;}/* incorrect no. of parameters */
	/* Pixels per block must be an even number, < 32. */
	if (params[1] % 2 || params[1] > NC_MAX_PIXELS_PER_BLOCK) {stat = THROW(NC_EINVAL); goto done;}
	/* If zlib compression is already applied, return error. */
	if(havedeflate) {stat = THROW(NC_EINVAL); goto done;}
    }
    /* More error checking */
    if(id == H5Z_FILTER_SZIP) { /* szip X chunking error checking */
        /* For szip, the pixels_per_block parameter must not be greater
         * than the number of elements in a chunk of data. */
	size_t num_elem = 1;
	size_t d;
	for (d = 0; d < var->ndims; d++) {
	    if (var->dim[d]->len) num_elem *= var->dim[d]->len;
	}
	/* Pixels per block must be <= number of elements. */
	if (params[1] > num_elem) {stat = THROW(NC_EINVAL); goto done;}
    }
done:
    return THROW(stat);
}

#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/

/**************************************************/
/* Filter application functions */

int
NCZ_filter_initialize(void)
{
    int stat = NC_NOERR;
    ZTRACE(6,"");

    if(NCZ_filter_initialized) goto done;

    NCZ_filter_initialized = 1;

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    if((stat = NCZ_load_all_plugins())) goto done;
#endif
done:
    return ZUNTRACE(stat);
}

int
NCZ_filter_finalize(void)
{
    int stat = NC_NOERR;
    ZTRACE(6,"");
    if(!NCZ_filter_initialized) goto done;
    NCZ_filter_initialized = 0;

done:
    return ZUNTRACE(stat);
}
int
NCZ_applyfilterchain(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* chain, size_t inlen, void* indata, size_t* outlenp, void** outdatap, int encode)
{
    int stat = NC_NOERR;
    size_t i;
    void* lastbuffer = NULL; /* if not null, then last allocated buffer */
    
    ZTRACE(6,"|chain|=%u inlen=%u indata=%p encode=%d", (unsigned)nclistlength(chain), (unsigned)inlen, indata, encode);

    /* Make sure all the filters are loaded && setup */
    for(i=0;i<nclistlength(chain);i++) {
	NCZ_Filter* f = (NCZ_Filter*)nclistget(chain,i);
	assert(f != NULL);
	if(FILTERINCOMPLETE(f)) {stat = THROW(NC_ENOFILTER); goto done;}
	if(FILTERSUPPRESSED(f)) continue;
	assert(f->hdf5.id > 0 && f->plugin != NULL);
	if(!(f->flags & FLAG_WORKING)) {/* working not yet available */
	    if((stat = ensure_working(file,var,f))) goto done;
	}
    }

    {
	NCZ_Filter* f = NULL;
	const H5Z_class2_t* ff = NULL;
	size_t current_alloc = inlen;
	void* current_buf = indata;
	size_t current_used = inlen;
	size_t next_alloc = 0;
	void* next_buf = NULL;
	size_t next_used = 0;

#ifdef DEBUG
fprintf(stderr,">>> current: alloc=%u used=%u buf=%p\n",(unsigned)current_alloc,(unsigned)current_used,current_buf);
#endif
	/* Apply in proper order */
	if(encode) {
	    for(i=0;i<nclistlength(chain);i++) {
		f = (NCZ_Filter*)nclistget(chain,i);	
		if(f->flags & FLAG_SUPPRESS) continue; /* this filter should not be applied */		
		ff = f->plugin->hdf5.filter;
		/* code can be simplified */
		next_alloc = current_alloc;
		next_buf = current_buf;
		next_used = 0;
		next_used = ff->filter(0,f->hdf5.working.nparams,f->hdf5.working.params,current_used,&next_alloc,&next_buf);
#ifdef DEBUG
fprintf(stderr,">>> next: alloc=%u used=%u buf=%p\n",(unsigned)next_alloc,(unsigned)next_used,next_buf);
#endif
		if(next_used == 0) {stat = NC_EFILTER; lastbuffer = next_buf; goto done; }
		/* If the filter did not need to create a new buffer, then next == current else current was reclaimed */
		current_buf = next_buf;
		current_alloc = next_alloc;
		current_used = next_used;
	    }
	} else {
	    /* Apply in reverse order */
	    for(size_t k=nclistlength(chain); k-->0;) {
	      f = (NCZ_Filter*)nclistget(chain, k);
		if(f->flags & FLAG_SUPPRESS) continue; /* this filter should not be applied */
		ff = f->plugin->hdf5.filter;
		/* code can be simplified */
		next_alloc = current_alloc;
		next_buf = current_buf;
		next_used = 0;
		next_used = ff->filter(H5Z_FLAG_REVERSE,f->hdf5.working.nparams,f->hdf5.working.params,current_used,&next_alloc,&next_buf);
#ifdef DEBUG
fprintf(stderr,">>> next: alloc=%u used=%u buf=%p\n",(unsigned)next_alloc,(unsigned)next_used,next_buf);
#endif
		if(next_used == 0) {stat = NC_EFILTER; lastbuffer = next_buf; goto done;}
		/* If the filter did not need to create a new buffer, then next == current else current was reclaimed */
		current_buf = next_buf;
		current_alloc = next_alloc;
		current_used = next_used;
	    }
	}
#ifdef DEBUG
fprintf(stderr,">>> current: alloc=%u used=%u buf=%p\n",(unsigned)current_alloc,(unsigned)current_used,current_buf);
#endif
	/* return results */
	if(outlenp) {*outlenp = current_used;} /* or should it be current_alloc? */
	if(outdatap) {*outdatap = current_buf;}
    }

done:
    if(lastbuffer != NULL && lastbuffer != indata) nullfree(lastbuffer); /* cleanup */
    return ZUNTRACEX(stat,"outlen=%u outdata=%p",(unsigned)*outlenp,*outdatap);
}

static int
ensure_working(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCproplist* props = NULL;
	    
    if(FILTERINCOMPLETE(filter)) {stat = THROW(NC_ENOFILTER); goto done;}
    if(!(filter->flags & FLAG_WORKING)) {
	const size_t oldnparams = filter->hdf5.visible.nparams;
	const unsigned* oldparams = filter->hdf5.visible.params;

	assert(filter->flags & FLAG_VISIBLE);
	/* Convert the visible parameters to working parameters; may also modify the visible params */
#ifdef DEBUGF
	fprintf(stderr,">>> DEBUGF: NCZ_modify_parameters: before: visible=%s working=%s\n",
		printparams(filter->hdf5.visible.nparams,filter->hdf5.visible.params),
		printparams(filter->hdf5.working.nparams,filter->hdf5.working.params));
#endif
	if(filter->plugin && filter->plugin->codec.codec->NCZ_modify_parameters) {
	    props = ncproplistnew();
	    if((stat=ncproplistclone(zfile->zarr.zarr_format==2?NCplistzarrv2:NCplistzarrv3,props))) goto done;
	    ncproplistadd(props,"fileid",(size_t)ncidfor(var));
	    ncproplistadd(props,"varid",(uintptr_t)var->hdr.id);
	    stat = filter->plugin->codec.codec->NCZ_modify_parameters(props,&filter->hdf5.id,
				&filter->hdf5.visible.nparams, &filter->hdf5.visible.params,
				&filter->hdf5.working.nparams, &filter->hdf5.working.params);
#ifdef DEBUGF
	    fprintf(stderr,">>> DEBUGF: NCZ_modify_parameters: stat=%d ncid=%d varid=%d filter=%s\n",stat, (int)ncidfor(var),(int)var->hdr.id,
			printfilter(filter));
#endif
	    if(stat) goto done;
	    /* See if the visible parameters were changed */
	    if(oldnparams != filter->hdf5.visible.nparams || oldparams != filter->hdf5.visible.params)
		filter->flags |= FLAG_NEWVISIBLE;
	} else {
	    /* assume visible are unchanged */
	    assert(oldnparams == filter->hdf5.visible.nparams && oldparams == filter->hdf5.visible.params); /* unchanged */
	    /* Just copy the visible parameters */
	    nullfree(filter->hdf5.working.params); filter->hdf5.working.params = NULL;
	    if((stat = paramnczclone(&filter->hdf5.working,&filter->hdf5.visible))) goto done;
	}
#ifdef DEBUGF
	fprintf(stderr,">>> DEBUGF: NCZ_modify_parameters: after: visible=%s working=%s\n",
		printparams(filter->hdf5.visible.nparams,filter->hdf5.visible.params),
		printparams(filter->hdf5.working.nparams,filter->hdf5.working.params));
#endif
	filter->flags |= FLAG_WORKING;
    }
#ifdef DEBUGF
    fprintf(stderr,">>> DEBUGF: ensure_working_parameters: ncid=%lu varid=%u filter=%s\n", ncidfor(var), (unsigned)var->hdr.id,printfilter(filter));
#endif
done:
    ncproplistfree(props);
    return THROW(stat);
}

/* Called by NCZ_enddef to ensure that the working parameters are defined */
int
NCZ_filter_setup(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    size_t i;
    int stat = NC_NOERR;
    NClist* filters = NULL;

    ZTRACE(6,"var=%s",var->hdr.name);

    filters = (NClist*)var->filters;
    for(i=0;i<nclistlength(filters);i++) {    
	NCZ_Filter* filter = (NCZ_Filter*)nclistget(filters,i);
	assert(filter != NULL);
	if(FILTERINCOMPLETE(filter)) continue; /* ignore these */
	assert(filter->plugin != NULL);
	assert((filter->flags & FLAG_VISIBLE)); /* Assume visible params are defined */
	/* verify */
	assert(filter->hdf5.id > 0 && (filter->hdf5.visible.nparams == 0 || filter->hdf5.visible.params != NULL));
	/* Initialize the working parameters */
	if((stat = ensure_working(file,var,filter))) goto done;
#ifdef DEBUGF
	fprintf(stderr,">>> DEBUGF: NCZ_filter_setup: ncid=%d varid=%d filter=%s\n", (int)ncidfor(var),(int)var->hdr.id,
			printfilter(filter));
#endif
    }

done:
    return ZUNTRACE(stat);
}

/* _Codecs attribute */
int
NCZ_codec_attr(NC_VAR_INFO_T* var, size_t* lenp, void* data)
{
    size_t i;
    int stat = NC_NOERR;
    size_t len;
    char* contents = NULL;
    NCbytes* buf = NULL;
    NClist* filters = (NClist*)var->filters;
    size_t nfilters;
    
    ZTRACE(6,"var=%s",var->hdr.name);

    nfilters = nclistlength(filters);

    if(nfilters == 0)
        {stat = NC_ENOTATT; goto done;}

    buf = ncbytesnew(); ncbytessetalloc(buf,1024);
    ncbytescat(buf,"[");
    for(i=0;i<nfilters;i++) {
       	NCZ_Filter* spec = (NCZ_Filter*)nclistget(filters,i);
        if(i > 0) ncbytescat(buf,",");
	ncbytescat(buf,spec->codec.codec);
    }
    ncbytescat(buf,"]");

    len = ncbyteslength(buf);
    contents = nclistcontents(buf);
    if(lenp) *lenp = len;
    if(data) strncpy((char*)data,contents,len+1);
done:
    ncbytesfree(buf);
    return ZUNTRACEX(stat,"len=%u data=%p",(unsigned)len,data);
}

/**************************************************/

/* Clone an hdf5 parameter set */
static int
paramclone(unsigned** dstp, const unsigned* src, size_t nparams)
{
    unsigned* dst = NULL;
    if(nparams > 0) {
	if(src == NULL) return NC_EINVAL;
	if((dst = (unsigned*)malloc(sizeof(unsigned) * nparams))==NULL)
	    return NC_ENOMEM;
	memcpy(dst,src,sizeof(unsigned) * nparams);
    }
    if(dstp) *dstp = dst;
    return NC_NOERR;
}

static int
paramnczclone(NCZ_Params* dst, const NCZ_Params* src)
{
    assert(src != NULL && dst != NULL && dst->params == NULL);
    *dst = *src;
    return paramclone(&dst->params,src->params,src->nparams);
}

void
ncz_hdf5_clear(NCZ_HDF5* h)
{
    nullfree(h->visible.params);
    nullfree(h->working.params);
    *h = NCZ_hdf5_empty();
}

void
ncz_codec_clear(NCZ_Codec* c)
{
    nullfree(c->id);
    nullfree(c->codec);
    *c = NCZ_codec_empty();
}
