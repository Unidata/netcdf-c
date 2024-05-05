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
#include "netcdf_aux.h"
#include "netcdf_filter.h"
#include "netcdf_filter_build.h"

#if 0
#define DEBUG
#define DEBUGF
#define DEBUGL
#endif

/* If set, then triage potential shared libraries based on extension */
#define NAMEOPT

#define NULLIFY(x) ((x)?(x):"NULL")

#define H5Z_FILTER_RAW 0
#define H5Z_CODEC_RAW "hdf5raw"

NCZ_Codec NCZ_codec_empty = {NULL, NULL, 0};
NCZ_HDF5 NCZ_hdf5_empty = {0, {0,NULL}, {0, NULL}};

/* WARNING: GLOBAL DATA */
/* TODO: move to common global state */

/* Plugin global data */
static struct PluginGlobal {
    /* All possible HDF5 filter plugins */
    /* Consider onverting to linked list or hash table or equivalent since very sparse */
    NCZ_Plugin* loaded_plugins[H5Z_FILTER_MAX];
    size_t loaded_plugins_max; /* max define plugin id */
    const struct CodecAPI* hdf5raw;
    NClist* codec_defaults; /* NClist<struct CodecAPI*> */
    NClist* default_libs; /* NClist<NCPSharedLib>; sources of the defaults */
} plugins;

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

const char*
printplugin(const NCZ_Plugin* plugin)
{
    static char plbuf[4096];
    char plbuf2[2000];
    char plbuf1[2000];

    if(plugin == NULL) return "plugin=NULL";
    plbuf2[0] = '\0'; plbuf1[0] = '\0';
    if(plugin->hdf5.filter)
        snprintf(plbuf1,sizeof(plbuf1),"hdf5={id=%u name=%s}",plugin->hdf5.filter->id,plugin->hdf5.filter->name);
    if(plugin->codec.codec)
        snprintf(plbuf2,sizeof(plbuf2),"codec={codecid=%s hdf5id=%u}",plugin->codec.codec->codecid,plugin->codec.codec->hdf5id);
    snprintf(plbuf,4096,"plugin={%s %s}",plbuf1,plbuf2);
    return plbuf;
}

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
static int NCZ_load_all_plugins(void);
static int NCZ_load_plugin_dir(const char* path);
static int NCZ_load_plugin(const char* path, NCZ_Plugin** plugp);
static int NCZ_unload_plugin(NCZ_Plugin* plugin);
static int NCZ_plugin_loaded(unsigned filterid, NCZ_Plugin** pp);
static int NCZ_plugin_save(unsigned filterid, NCZ_Plugin* p);
static int NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, NCZ_Filter** specp);
static int NCZ_addfilter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, unsigned int id, size_t nparams, const unsigned int* params);

static int getentries(const char* path, NClist* contents);
static int NCZ_split_plugin_path(const char* path0, NClist* list);

static int ensure_working(NC_FILE_INFO_T*, NC_VAR_INFO_T* var, NCZ_Filter* filter);

static int paramnczclone(const NCZ_Params* src, NCZ_Params* dst);
static int paramclone(size_t nparams, const unsigned* src, unsigned** dstp);

#ifdef NAMEOPT
static int pluginnamecheck(const char* name);
#endif

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

int
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
    NCZ_filter_hdf5_clear(&spec->hdf5);
    NCZ_filter_codec_clear(&spec->codec);
    free(spec);
    return NC_NOERR;
}

int
NCZ_filter_hdf5_clear(NCZ_HDF5* spec)
{
    ZTRACE(6,"spec=%d",spec->id);
    if(spec == NULL) goto done;
    nullfree(spec->visible.params); spec->visible.params = NULL;
    nullfree(spec->working.params); spec->working.params = NULL;
done:
    return ZUNTRACE(NC_NOERR);
}

int
NCZ_filter_codec_clear(NCZ_Codec* spec)
{
    ZTRACE(6,"spec=%d",(spec?spec->id:"null"));
    if(spec == NULL) goto done;
    nullfree(spec->id); spec->id = NULL;
    nullfree(spec->codec); spec->codec = NULL;
done:
    return ZUNTRACE(NC_NOERR);
}

/**************************************************/

int
NCZ_codec_plugin_lookup(const char* codecid, NCZ_Plugin** pluginp)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_Plugin* plugin = NULL;
    char digits[64];
    const char* trueid = NULL;
    
    /* Find the plugin for this codecid */
    for(i=1;i<=plugins.loaded_plugins_max;i++) {
	NCZ_Plugin* p = plugins.loaded_plugins[i];
        if(p == NULL) continue;
        if(p == NULL|| p->codec.codec == NULL) continue; /* no plugin or no codec */
	if(p->codec.ishdf5raw) {
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

int
NCZ_filter_remove(NC_VAR_INFO_T* var, unsigned int id)
{
    int stat = NC_NOERR;
    size_t k;
    NClist* flist = (NClist*)var->filters;

    ZTRACE(6,"var=%s id=%u",var->hdr.name,id);
    /* Walk backwards */
    for(size_t k = nclistlength(flist); k-->0;) {
	struct NCZ_Filter* f = (struct NCZ_Filter*)nclistget(flist,k);
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
	if((spec->hdf5.id == id) && !spec->incomplete) {
	    if(specp) *specp = spec;
	    break;
	}
    }
    return ZUNTRACEX(NC_NOERR,"spec=%d",IEXISTS(specp,hdf5.id));
}

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
int
NCZ_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams,
                   const unsigned int* params)
{
    int stat = NC_NOERR;
    NC *nc;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NCZ_Filter* oldspec = NULL;
    NCZ_Filter* tmp = NULL;
    int havedeflate = 0;
    int haveszip = 0;

    ZTRACE(1,"ncid=%d varid=%d id=%u nparams=%u params=%s",ncid,varid,id,(unsigned)nparams,nczprint_paramvector(nparams,params));

    if((stat = NCZ_filter_initialize())) goto done;
    
    if((stat = NC_check_id(ncid,&nc))) return stat;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    /* If the NCZARR dataset has already been created, then it is too
     * late to set all the extra stuff. */
    if (!(h5->flags & NC_INDEF))
	{stat = THROW(NC_EINDEFINE); goto done;}
    if (!var->ndims)
	{stat = NC_EINVAL; goto done;} /* For scalars, complain */
    if (var->created)
        {stat = THROW(NC_ELATEDEF); goto done;}

    /* Lookup incoming id to see if already defined */
    if((stat=NCZ_filter_lookup(var,id,&oldspec))) goto done;

    /* See if deflate &/or szip is defined */
    if((stat = NCZ_filter_lookup(var,H5Z_FILTER_DEFLATE,&tmp))) goto done;
    havedeflate = (tmp == NULL ? 0 : 1);
    stat = NC_NOERR; /* reset */

    if((stat = NCZ_filter_lookup(var,H5Z_FILTER_SZIP,&tmp))) goto done;
    haveszip = (tmp == NULL ? 0 : 1);
    stat = NC_NOERR; /* reset */
    
    /* If incoming filter not already defined, then check for conflicts */
    if(oldspec == NULL) {
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
        if(id == H5Z_FILTER_SZIP) { /* Do error checking */
                if(nparams != 2)
                    {stat = THROW(NC_EFILTER); goto done;}/* incorrect no. of parameters */
                /* Pixels per block must be an even number, < 32. */
                if (params[1] % 2 || params[1] > NC_MAX_PIXELS_PER_BLOCK)
                    {stat = THROW(NC_EINVAL); goto done;}
                /* If zlib compression is already applied, return error. */
	        if(havedeflate) {stat = THROW(NC_EINVAL); goto done;}
        }
        /* Filter => chunking */
        var->storage = NC_CHUNKED;
        /* Determine default chunksizes for this variable unless already specified */
        if(var->chunksizes && !var->chunksizes[0]) {
	        /* Should this throw error? */
                if((stat = nc4_find_default_chunksizes2(grp, var)))
	            goto done;
        }
     }
     /* More error checking */
    if(id == H5Z_FILTER_SZIP) { /* szip X chunking error checking */
	    /* For szip, the pixels_per_block parameter must not be greater
	     * than the number of elements in a chunk of data. */
            size_t num_elem = 1;
            int d;
            for (d = 0; d < (int)var->ndims; d++)
                if (var->dim[d]->len)
		    num_elem *= var->dim[d]->len;
            /* Pixels per block must be <= number of elements. */
            if (params[1] > num_elem)
                {stat = THROW(NC_EINVAL); goto done;}
    }
    /* addfilter can handle case where filter is already defined, and will just replace parameters */
    if((stat = NCZ_addfilter(h5,var,id,nparams,params)))
        goto done;
    if (h5->parallel)
        {stat = THROW(NC_EINVAL); goto done;}

done:
    return ZUNTRACE(stat);
}

/* From NCZ_def_var_filter */
static int
NCZ_addfilter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, unsigned int id, size_t nparams, const unsigned int* params)
{
    int stat = NC_NOERR;
    int exists = 0;
    NCZ_Filter* fi = NULL;
    NCZ_HDF5 hdf5 = NCZ_hdf5_empty;

    ZTRACE(6,"file=%s var=%s id=%u nparams=%u params=%p",file->hdr.name,var->hdr.name,id,nparams,params);
    
    if(nparams > 0 && params == NULL) {stat = NC_EINVAL; goto done;}

    /* Warning if filter already exists, fi will be changed to be that filter and old fi will be reclaimed */
    /* If it already exists, then overwrite the parameters */
    if((stat=NCZ_filter_lookup(var,id,&fi))) goto done;
    if(fi != NULL) {
	exists = 1;
    } else {
        if((fi = calloc(1,sizeof(NCZ_Filter))) == NULL) {stat = NC_ENOMEM; goto done;}
    }

    hdf5.id = id;
    hdf5.visible.nparams = nparams;
    hdf5.visible.params = (unsigned*)params;
    if((stat = NCZ_insert_filter(file,(NClist*)var->filters,&hdf5,NULL,fi,exists))) goto done;

done:
    return ZUNTRACE(stat);
}

/* Use this instead of NCZ_addfilter when filter has already been constructed */
int
NCZ_insert_filter(NC_FILE_INFO_T* file, NClist* filterlist, NCZ_HDF5* hdf5, NCZ_Codec* codec, NCZ_Filter* fi, int exists)
{
    int stat = NC_NOERR;
    NCZ_Plugin* plugin = NULL;

    ZTRACE(6,"file=%s",file->hdr.name);
    
    assert(filterlist != NULL);
    assert(fi != NULL);
    assert(hdf5 != NULL);

    /* cleanup */
    if((stat=NCZ_filter_hdf5_clear(&fi->hdf5))) goto done;
    if((stat=NCZ_filter_codec_clear(&fi->codec))) goto done;

    /* Find the matching plugin, if any */
    if((stat = NCZ_plugin_loaded(hdf5->id,&plugin))) goto done;
    fi->plugin = plugin;
    if(fi->plugin == NULL || plugin->incomplete) fi->incomplete = 1;

    if((stat = NCZ_fillin_filter(file,fi,hdf5,(codec!=NULL?codec:NULL)))) goto done;
    
    /* Add to filters list  */
    if(!exists)
	nclistpush(filterlist, fi);

done:
    return ZUNTRACE(stat);
}

int
NCZ_fillin_filter(NC_FILE_INFO_T* file,
				NCZ_Filter* filter,
				NCZ_HDF5* hdf5,
				NCZ_Codec* codec)
{
    int stat = NC_NOERR;

    /* (over)write the HDF5 parameters */
    NCZ_filter_hdf5_clear(&filter->hdf5);
    NCZ_filter_codec_clear(&filter->codec);
    /* Fill in the hdf5 and codec*/
    filter->hdf5 = *hdf5; /* get non-pointer fields */
    /* Avoid taking control of params */
    if((stat = paramclone(hdf5->visible.nparams,hdf5->visible.params,&filter->hdf5.visible.params))) goto done;
    assert(hdf5->working.nparams == 0 && hdf5->working.params == NULL);
    if(codec != NULL) {
        filter->codec = *codec; /* get non-pointer fields */
        /* Avoid taking control of fields */    
        filter->codec.id = nulldup(codec->id);
        filter->codec.codec = nulldup(codec->codec);
    } else
        filter->codec = NCZ_codec_empty;
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
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    /* Make sure all the filters are defined */
    if((stat = NCZ_filter_initialize())) goto done;

    flist = var->filters;

    nfilters = nclistlength(flist); /* including incomplete filters */
    if(nfilters > 0 && ids != NULL) {
	size_t k;
	for(k=0;k<nfilters;k++) {
	    NCZ_Filter* f = (NCZ_Filter*)nclistget(flist,k);
	    if(f->hdf5.id == 0) continue; /* No way to show filter without hdf5 id */
            ids[k] = (unsigned int)(f->hdf5.id);
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
    if ((stat = nc4_find_grp_h5_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

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
    if((stat = NCZ_plugin_loaded(id, &plug))) goto done;
    if(plug == NULL || plug->incomplete)
        stat = THROW(NC_ENOFILTER);
done:
    return ZUNTRACE(stat);
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

    memset(plugins.loaded_plugins,0,sizeof(plugins.loaded_plugins));
    plugins.loaded_plugins_max = 0;
    plugins.hdf5raw = NULL;
    plugins.codec_defaults = nclistnew();
    plugins.default_libs = nclistnew();
    NCZ_filter_initialized = 1;

    {/* Make empty plugin */
	NCZ_Plugin* ep = (NCZ_Plugin*)calloc(1,sizeof(NCZ_Plugin));
	if(ep == NULL) {stat = NC_ENOMEM; goto done;}
        plugins.loaded_plugins[PLUGIN_EMPTY_ID] = ep;
	ep->incomplete = 1;
    }

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
    size_t i;
    ZTRACE(6,"");
    if(!NCZ_filter_initialized) goto done;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Reclaim all loaded filters */
#ifdef DEBUGL
    fprintf(stderr,">>>  DEBUGL: finalize reclaim:\n");
#endif
    for(i=0;i<=plugins.loaded_plugins_max;i++) {
	if(plugins.loaded_plugins[i]) {
            NCZ_unload_plugin(plugins.loaded_plugins[i]);
	    plugins.loaded_plugins[i] = NULL;
	}
    }
    /* Reclaim the codec defaults */
    if(nclistlength(plugins.codec_defaults) > 0) {
	size_t j;
        for(j=0;j<nclistlength(plugins.codec_defaults);j++) {
	    struct CodecAPI* ca = (struct CodecAPI*)nclistget(plugins.codec_defaults,j);
    	    nullfree(ca);
	}
    }
    /* Reclaim the defaults library contents; Must occur as last act */
    if(nclistlength(plugins.default_libs) > 0) {
	size_t j;
        for(j=0;j<nclistlength(plugins.default_libs);j++) {
	    NCPSharedLib* l = (NCPSharedLib*)nclistget(plugins.default_libs,j);
#ifdef DEBUGL
   fprintf(stderr,">>> DEBUGL: NCZ_filter_finalize: reclaim default_lib[j]=%p\n",l);
#endif
    	    if(l != NULL) (void)ncpsharedlibfree(l);
	}
    }
#else
    memset(plugins.loaded_plugins,0,sizeof(plugins.loaded_plugins));
#endif
    nclistfree(plugins.default_libs); plugins.default_libs = NULL;
    nclistfree(plugins.codec_defaults); plugins.codec_defaults = NULL;
done:
    NCZ_filter_initialized = 0;
    return ZUNTRACE(stat);
}

static int
NCZ_plugin_save(unsigned filterid, NCZ_Plugin* p)
{
    int stat = NC_NOERR;
    ZTRACE(6,"filterid=%d p=%p",filterid,p);
    if(filterid < 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    if(filterid > plugins.loaded_plugins_max) plugins.loaded_plugins_max = filterid;
    plugins.loaded_plugins[filterid] = p;
done:
    return ZUNTRACE(stat);
}

static int
NCZ_plugin_loaded(unsigned filterid, NCZ_Plugin** pp)
{
    int stat = NC_NOERR;
    struct NCZ_Plugin* plug = NULL;
    ZTRACE(6,"filterid=%d",filterid);
    if(filterid > 0 && filterid < H5Z_FILTER_MAX) {
        if(filterid <= plugins.loaded_plugins_max) 
            plug = plugins.loaded_plugins[filterid];
    }
    if(pp) *pp = plug;
    return ZUNTRACEX(stat,"plugin=%p",*pp);
}

int
NCZ_applyfilterchain(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* chain, size_t inlen, void* indata, size_t* outlenp, void** outdatap, int encode)
{
    size_t i;
    int stat = NC_NOERR;
    void* lastbuffer = NULL; /* if not null, then last allocated buffer */
    
    ZTRACE(6,"|chain|=%u inlen=%u indata=%p encode=%d", (unsigned)nclistlength(chain), (unsigned)inlen, indata, encode);

    /* Make sure all the filters are loaded && setup */
    for(i=0;i<nclistlength(chain);i++) {
	NCZ_Filter* f = (NCZ_Filter*)nclistget(chain,i);
	assert(f != NULL);
	if(f->incomplete || f->suppress) continue; /* ignore bad filters and vlen variable filters */
	assert(f->hdf5.id > 0 && f->plugin != NULL);
	if(!(f->flags & FLAG_WORKING)) {/* working not yet available */
	    if((stat = ensure_working(file, var,f))) goto done;
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
		if(f->incomplete || f->suppress) continue; /* this filter should not be applied */		
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
                f = (struct NCZ_Filter*)nclistget(chain, k);
		if(f->suppress) continue; /* this filter should not be applied */
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

/**************************************************/
/* JSON Parse/unparse of filter codecs */
/* Note that we do not care the about the resulting format, so it works for V2 and V3
   by using NCZF_hdf2codec
*/

int
NCZ_filter_jsonize(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter, NCjson** jfilterp)
{
    int stat = NC_NOERR;
    NCjson* jfilter = NULL;

    ZTRACE(6,"var=%s filter=%s",var->hdr.name,(filter != NULL && filter->codec.id != NULL?filter->codec.id:"null"));

    /* We need to ensure the the current visible parameters are defined and had the opportunity to come
       from the working parameters */
    assert((filter->flags & (FLAG_VISIBLE | FLAG_WORKING)) == (FLAG_VISIBLE | FLAG_WORKING));

    /* Convert the visible parameters back to codec */
    if((stat = NCZF_hdf2codec(file,var,filter))) goto done;

    /* Parse the codec as the return */
    if(NCJparse(filter->codec.codec,0,&jfilter) < 0) {stat = NC_EFILTER; goto done;}
    if(jfilterp) {*jfilterp = jfilter; jfilter = NULL;}

done:
    NCJreclaim(jfilter);
    return ZUNTRACEX(stat,"codec=%s",NULLIFY(filter->codec.codec));
}


/**************************************************/
/* Filter loading */

/*
Get entries in a path that is assumed to be a directory.
*/

#ifdef _WIN32

static int
getentries(const char* path, NClist* contents)
{
    /* Iterate over the entries in the directory */
    int ret = NC_NOERR;
    errno = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE dir = NULL;
    char* ffpath = NULL;
    char* lpath = NULL;
    size_t len;
    char* d = NULL;

    ZTRACE(6,"path=%s",path);

    /* We need to process the path to make it work with FindFirstFile */
    len = strlen(path);
    /* Need to terminate path with '/''*' */
    ffpath = (char*)malloc(len+2+1);
    memcpy(ffpath,path,len);
    if(path[len-1] != '/') {
	ffpath[len] = '/';	
	len++;
    }
    ffpath[len] = '*'; len++;
    ffpath[len] = '\0';

    /* localize it */
    if((ret = nczm_localize(ffpath,&lpath,LOCALIZE))) goto done;
    dir = FindFirstFile(lpath, &FindFileData);
    if(dir == INVALID_HANDLE_VALUE) {
	/* Distinquish not-a-directory from no-matching-file */
        switch (GetLastError()) {
	case ERROR_FILE_NOT_FOUND: /* No matching files */ /* fall thru */
	    ret = NC_NOERR;
	    goto done;
	case ERROR_DIRECTORY: /* path is not a directory ; fall thru */
	default:
            ret = NC_EEMPTY; 
	    goto done;
	}
    }
    do {
	char* p = NULL;
	const char* name = NULL;
        name = FindFileData.cFileName;
	if(strcmp(name,".")==0 || strcmp(name,"..")==0)
	    continue;
	nclistpush(contents,strdup(name));
    } while(FindNextFile(dir, &FindFileData));

done:
    if(dir) FindClose(dir);
    nullfree(lpath);
    nullfree(ffpath);
    nullfree(d);
    errno = 0;
    return ZUNTRACEX(ret,"|contents|=%d",(int)nclistlength(contents));
}

#else /* !_WIN32 */

int
getentries(const char* path, NClist* contents)
{
    int ret = NC_NOERR;
    errno = 0;
    DIR* dir = NULL;

    ZTRACE(6,"path=%s",path);

    dir = NCopendir(path);
    if(dir == NULL)
        {ret = (errno); goto done;}
    for(;;) {
	const char* name = NULL;
	struct dirent* de = NULL;
	errno = 0;
        de = readdir(dir);
        if(de == NULL)
	    {ret = (errno); goto done;}
	if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0)
	    continue;
	name = de->d_name;
	nclistpush(contents,strdup(name));
    }
done:
    if(dir) NCclosedir(dir);
    errno = 0;
    return ZUNTRACEX(ret,"|contents|=%d",(int)nclistlength(contents));
}
#endif /*_WIN32*/

static int
NCZ_load_all_plugins(void)
{
    size_t i,j;
    int ret = NC_NOERR;
    char* pluginroots = NULL;
    struct stat buf;
    NClist* dirs = nclistnew();
    char* defaultpluginpath = NULL;
    

    ZTRACE(6,"");

#ifdef DEBUGL
   fprintf(stderr,">>> DEBUGL: NCZ_load_all_plugins\n");
#endif

   /* Setup the plugin path default */
   {
#ifdef _WIN32
	const char* win32_root;
	char dfalt[4096];
	win32_root = getenv(WIN32_ROOT_ENV);
	if(win32_root != NULL && strlen(win32_root) > 0) {
	    snprintf(dfalt,sizeof(dfalt),PLUGIN_DIR_WIN,win32_root);
	    defaultpluginpath = strdup(dfalt);
	}
#else /*!_WIN32*/
	defaultpluginpath = strdup(PLUGIN_DIR_UNIX);
#endif
    }

    /* Find the plugin directory root(s) */
    pluginroots = getenv(PLUGIN_ENV); /* Usually HDF5_PLUGIN_PATH */
    if(pluginroots  != NULL && strlen(pluginroots) == 0) pluginroots = NULL;
    if(pluginroots == NULL) {
	pluginroots = defaultpluginpath;
    }
    assert(pluginroots != NULL);
    ZTRACEMORE(6,"pluginroots=%s",(pluginroots?pluginroots:"null"));

    if((ret = NCZ_split_plugin_path(pluginroots,dirs))) goto done;

    /* Add the default to end of the dirs list if not already there */
    if(!nclistmatch(dirs,defaultpluginpath,0)) {
        nclistpush(dirs,defaultpluginpath);
	defaultpluginpath = NULL;
    }

    for(i=0;i<nclistlength(dirs);i++) {
	const char* dir = (const char*)nclistget(dirs,i);
        /* Make sure the root is actually a directory */
        errno = 0;
        ret = NCstat(dir, &buf);
#if 1
        ZTRACEMORE(6,"stat: ret=%d, errno=%d st_mode=%d",ret,errno,buf.st_mode);
#endif
        if(ret < 0) {errno = 0; ret = NC_NOERR; continue;} /* ignore unreadable directories */
	if(! S_ISDIR(buf.st_mode))
            ret = NC_EINVAL;
        if(ret) goto done;

        /* Try to load plugins from this directory */
        if((ret = NCZ_load_plugin_dir(dir))) goto done;
    }
#ifdef DEBUGL
    { size_t i;
	fprintf(stderr,"plugins.codec_defaults:");
	for(i=0;i<nclistlength(plugins.codec_defaults);i++) {
	    struct CodecAPI* codec = (struct CodecAPI*)nclistget(plugins.codec_defaults,i);
	    fprintf(stderr," %d",codec->codec->hdf5id);	    
	}
	fprintf(stderr,"\n");
    }
#endif
    { /* See if we have the hdf5raw codec */
    	size_t i;
	for(i=0;i<nclistlength(plugins.codec_defaults);i++) {
	    struct CodecAPI* codec = (struct CodecAPI*)nclistget(plugins.codec_defaults,i);
	    if(codec->codec->hdf5id == H5Z_FILTER_RAW && strcasecmp(codec->codec->codecid,H5Z_CODEC_RAW)==0) {
	        plugins.hdf5raw = codec;
	    }
	}
    }

    if(nclistlength(plugins.codec_defaults)) { /* Try to provide default for any HDF5 filters without matching Codec. */
	unsigned hdf5id;
	/* Search all the plugins */
	for(hdf5id=1;hdf5id<=plugins.loaded_plugins_max;hdf5id++) {
	    NCZ_Plugin* p = plugins.loaded_plugins[hdf5id];
	    if(p == NULL) continue;
	    if(p->hdf5.filter == NULL) continue; /* plugin has no hdf5 filter defined */
    	    if(p->codec.codec != NULL) continue; /* plugin already complete */
	    /* Search the defaults for a matching codec */
	    for(j=0;j<nclistlength(plugins.codec_defaults);j++) {
                struct CodecAPI* dfalt = (struct CodecAPI*)nclistget(plugins.codec_defaults,j);
		if(dfalt->codec->hdf5id != hdf5id) continue; /* not a matching codec */
		/* we have a matching codec */	    
#ifdef DEBUGL
	        fprintf(stderr,">>> DEBUGL: plugin defaulted: id=%u, codec=%s src=%s\n",hdf5id,dfalt->codec->codecid,dfalt->codeclib->path);
#endif
		p->codec.codec = dfalt->codec;
		p->codec.codeclib = dfalt->codeclib;
		p->codec.defaulted = 1;
		break; /* Do not search for other matches */
	    }
	}
    }

    /* Make a second pass over plugins to (1) apply hdf5raw codec if possible or (2) mark as incomplete */
    {
        unsigned hdf5id;
	NCZ_Plugin* p;
	for(hdf5id=1;hdf5id<=plugins.loaded_plugins_max;hdf5id++) {
	    p = plugins.loaded_plugins[hdf5id];
    	    if(p != NULL) {
		if(p->hdf5.filter == NULL) {
		    /* no matching hdf5 filter, so mark this entry as incomplete */
		    p->incomplete = 1;
		} else if(p->codec.codec == NULL) {
		    if(plugins.hdf5raw) {
		        /* We can default to using the hdf5raw codec manager */
		        p->codec.codec = plugins.hdf5raw->codec;
		        p->codec.codeclib = plugins.hdf5raw->codeclib;
		        p->codec.defaulted = 1;
			p->codec.ishdf5raw = 1;
#ifdef DEBUGL
			fprintf(stderr,">>> DEBUGL: plugin accepted via hdf5raw: id=%u\n",hdf5id);
#endif
		    } else /*plugins.hdf5raw == NULL*/
		        p->incomplete = 1; /* oh well */
		}
#ifdef DEBUGL
		if(p->incomplete)
		    fprintf(stderr,">>>  DEBUGL: Incomplete plugin: id=%u; reasons: %s %s\n",hdf5id,
		    		(p->hdf5.filter==NULL?"hdf5":""),(p->codec.codec==NULL?"codec":""));
#endif
#ifdef DEBUGL
		if(p->hdf5.filter != NULL && p->codec.codec != NULL)
		    fprintf(stderr,">>> DEBUGL: plugin accepted: id=%u\n",hdf5id);
#endif
	    }
	}
    }

    /* Initialize all accepted plugins */
    {
        size_t i;
	NCZ_Plugin* p;
	for(i=0;i<plugins.loaded_plugins_max;i++) {
	    if((p = plugins.loaded_plugins[i]) != NULL) {
		if(p->incomplete) continue;
		if(p->hdf5.filter != NULL && p->codec.codec != NULL) {
		    if(p->codec.codec && p->codec.codec->NCZ_codec_initialize)
			p->codec.codec->NCZ_codec_initialize(NULL);
#ifdef DEBUGL
		    fprintf(stderr,">>> DEBUGL: plugin initialized: id=%u\n",p->hdf5.filter->id);
#endif
		}
	    }
	}
    }
    
done:
    nullfree(defaultpluginpath);
    nclistfreeall(dirs);
    errno = 0;
    return ZUNTRACE(ret);
}

static int
NCZ_split_plugin_path(const char* path0, NClist* list)
{
    int i,stat = NC_NOERR;
    char* path = NULL;
    char* p;
    int count;
    size_t plen;
#ifdef _WIN32
    const char* seps = ";";
#else
    const char* seps = ";:";
#endif    

    if(path0 == NULL || path0[0] == '\0') goto done;
    plen = strlen(path0);
    if((path = malloc(plen+1+1))==NULL) {stat = NC_ENOMEM; goto done;}
    memcpy(path,path0,plen);
    path[plen] = '\0'; path[plen+1] = '\0';  /* double null term */
    for(count=0,p=path;*p;p++) {
	if(strchr(seps,*p) != NULL) {*p = '\0'; count++;}
    }
    count++; /* for last piece */
    for(p=path,i=0;i<count;i++) {
	size_t len = strlen(p);
	if(len > 0)
	    nclistpush(list,strdup(p));
        p = p+len+1; /* point to next piece */
    }

done:
    nullfree(path);
    return stat;
}


/* Load all the filters within a specified directory */
static int
NCZ_load_plugin_dir(const char* path)
{
    size_t i;
    int stat = NC_NOERR;
    size_t pathlen;
    NClist* contents = nclistnew();
    char* file = NULL;

    ZTRACE(7,"path=%s",path);

#ifdef DEBUGL
   fprintf(stderr,">>> DEBUGL: NCZ_load_plugin_dir: path=%s\n",path);
#endif

    if(path == NULL) {stat = NC_EINVAL; goto done;}
    pathlen = strlen(path);
    if(pathlen == 0) {stat = NC_EINVAL; goto done;}

    if((stat = getentries(path,contents))) goto done;
    for(i=0;i<nclistlength(contents);i++) {
        const char* name = (const char*)nclistget(contents,i);
	size_t nmlen = strlen(name);
	size_t flen = pathlen+1+nmlen+1;
	unsigned id;
	NCZ_Plugin* plugin = NULL;

	assert(nmlen > 0);
	nullfree(file); file = NULL;
	if((file = (char*)malloc(flen))==NULL) {stat = NC_ENOMEM; goto done;}
	file[0] = '\0';
	strlcat(file,path,flen);
	strlcat(file,"/",flen);
	strlcat(file,name,flen);
#if defined NAMEOPT || defined _WIN32
        /*triage because visual studio does a popup if the file will not load*/
        if(!pluginnamecheck(file)) continue;
#endif
	/* See if can load the file */
	stat = NCZ_load_plugin(file,&plugin);
	switch (stat) {
	case NC_NOERR: break;
	case NC_ENOFILTER: case NC_ENOTFOUND:
	    stat = NC_NOERR;
	    break; /* will cause it to be ignored */
	default: goto done;
	}
	if(plugin != NULL) {
	    id = (unsigned)plugin->hdf5.filter->id;
	    if(plugins.loaded_plugins[id] == NULL) {
		if((stat = NCZ_plugin_save(id,plugin))) goto done;
#ifdef DEBUGL
		fprintf(stderr,">>> DEBUGL: plugin loaded: %s\n",printplugin(plugin));
#endif
	    } else {
#ifdef DEBUGL
		fprintf(stderr,">>> DEBUGL: plugin duplicate: %s\n",printplugin(plugin));
#endif
	        NCZ_unload_plugin(plugin); /* its a duplicate */
	    }
	} else
	    stat = NC_NOERR; /*ignore failure */
    }	

done:
    nullfree(file);
    nclistfreeall(contents);
    return ZUNTRACE(stat);
}

int
loadcodecdefaults(const char* path, const NCZ_codec_t** cp, NCPSharedLib* lib, int* lib_usedp)
{
    int stat = NC_NOERR;
    int lib_used = 0;

    nclistpush(plugins.default_libs,lib);
    for(;*cp;cp++) {
        struct CodecAPI* c0;
#ifdef DEBUGL
        fprintf(stderr,"@@@ %s: %s = %u\n",path,(*cp)->codecid,(*cp)->hdf5id);
#endif
        c0 = (struct CodecAPI*)calloc(1,sizeof(struct CodecAPI));
	if(c0 == NULL) {stat = NC_ENOMEM; goto done;}
        c0->codec = *cp;
	c0->codeclib = lib;
	lib_used = 1; /* remember */
	nclistpush(plugins.codec_defaults,c0); c0 = NULL;
    }
done:
    if(lib_usedp) *lib_usedp = lib_used;
    return stat;
}

static int
NCZ_load_plugin(const char* path, struct NCZ_Plugin** plugp)
{
    int stat = NC_NOERR;
    NCZ_Plugin* plugin = NULL;
    const H5Z_class2_t* h5class = NULL;
    H5PL_type_t h5type = 0;
    const NCZ_codec_t** cp = NULL;
    const NCZ_codec_t* codec = NULL;
    NCPSharedLib* lib = NULL;
    int flags = NCP_GLOBAL;
    unsigned h5id = 0;
    
    assert(path != NULL && strlen(path) > 0 && plugp != NULL);

    ZTRACE(8,"path=%s",path);

    if(plugp) *plugp = NULL;

    /* load the shared library */
    if((stat = ncpsharedlibnew(&lib))) goto done;
    if((stat = ncpload(lib,path,flags))) goto done;

#ifdef DEBUGL
   fprintf(stderr,">>> DEBUGL: NCZ_load_plugin: path=%s lib=%p\n",path,lib);
#endif

    /* See what we have */
    {
	const H5PL_get_plugin_type_proto gpt =  (H5PL_get_plugin_type_proto)ncpgetsymbol(lib,"H5PLget_plugin_type");
	const H5PL_get_plugin_info_proto gpi =  (H5PL_get_plugin_info_proto)ncpgetsymbol(lib,"H5PLget_plugin_info");
	const NCZ_get_codec_info_proto  npi =  (NCZ_get_codec_info_proto)ncpgetsymbol(lib,"NCZ_get_codec_info");
	const NCZ_codec_info_defaults_proto  cpd =  (NCZ_codec_info_defaults_proto)ncpgetsymbol(lib,"NCZ_codec_info_defaults");

        if(gpt == NULL && gpi == NULL && npi == NULL && cpd == NULL)
	    {stat = THROW(NC_ENOFILTER); goto done;}

	/* We can have cpd  or we can have (gpt && gpi && npi) but not both sets */
	if(cpd != NULL) {
	    cp = (const NCZ_codec_t**)cpd();
        } else {/* cpd => !gpt && !gpi && !npi */
            if(gpt != NULL && gpi != NULL) { /* get HDF5 info */
                h5type = gpt();
                h5class = gpi();        
                /* Verify */
                if(h5type != H5PL_TYPE_FILTER) {stat = NC_EPLUGIN; goto done;}
                if(h5class->version != H5Z_CLASS_T_VERS) {stat = NC_EFILTER; goto done;}
            }
            if(npi != NULL) {/* get Codec info */
		codec = npi();
                /* Verify */
                if(codec->version != NCZ_CODEC_CLASS_VER) {stat = NC_EPLUGIN; goto done;}
                if(codec->sort != NCZ_CODEC_HDF5) {stat = NC_EPLUGIN; goto done;}
	    }
        }
    }

#ifdef DEBUGL
fprintf(stderr,">>> DEBUGL: load: %s:",path);
if(h5class) fprintf(stderr,">>>  %u",(unsigned)h5class->id);
if(codec) fprintf(stderr,">>>  %u/%s",codec->hdf5id,codec->codecid);
fprintf(stderr,">>> \n");
#endif

    /* Handle defaults separately */
    if(cp != NULL) {
	int used = 0;
#ifdef DEBUGL
        fprintf(stderr,"@@@ %s: default codec library found: %p\n",path,cp);
#endif
        if((stat = loadcodecdefaults(path,cp,lib,&used))) goto done;
	if(used) lib = NULL;
	goto done;
    }

    if(h5class != NULL && codec != NULL) {
	/* Verify consistency of the HDF5 and the Codec */
	if(codec->hdf5id != (unsigned)h5class->id) goto done; /* ignore */
    } 

    /* There are several cases to consider:
    1. This library has both HDF5 API and Codec API => merge
    2. This library has HDF5 API only and Codec API was already found in another library => merge
    3. This library has Codec API only and HDF5 API was already found in another library => merge    
    */

    /* Get any previous plugin entry for this id; may be NULL */
    if(h5class != NULL) {
	h5id = (unsigned)h5class->id;
	if((stat = NCZ_plugin_loaded(h5id,&plugin))) goto done;
    } else if(codec != NULL) {
	h5id = codec->hdf5id;
	if((stat = NCZ_plugin_loaded(h5id,&plugin))) goto done;
    }

    if(plugin == NULL) {
	/* create new entry */
	if((plugin = (NCZ_Plugin*)calloc(1,sizeof(NCZ_Plugin)))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
    /* Fill in the plugin */
    if(h5class != NULL && plugin->hdf5.filter == NULL) {
	plugin->hdf5.filter = h5class;
	plugin->hdf5.hdf5lib = lib;
	lib = NULL;
    }
    if(codec != NULL && plugin->codec.codec == NULL) {
	plugin->codec.codec = codec;
	plugin->codec.codeclib = lib;
	lib = NULL;
    }
#ifdef DEBUGL
    if(plugin)
       fprintf(stderr,">>> DEBUGL: load_plugin: %s\n",printplugin(plugin));
#endif
    /* Cleanup */
    if(plugin->hdf5.hdf5lib == plugin->codec.codeclib) /* Works for NULL case also */
	plugin->codec.codeclib = NULL;
    if((stat=NCZ_plugin_save(h5id,plugin))) goto done;
    plugin = NULL;

done:
    if(lib)
       (void)ncpsharedlibfree(lib);
    if(plugin) NCZ_unload_plugin(plugin);
    return ZUNTRACEX(stat,"plug=%p",*plugp);
}

static int
NCZ_unload_plugin(NCZ_Plugin* plugin)
{
    ZTRACE(9,"plugin=%p",plugin);

    if(plugin) {
#ifdef DEBUGL
        fprintf(stderr,">>> DEBUGL: unload: %s\n",printplugin(plugin));
#endif
	if(plugin->codec.codec && plugin->codec.codec->NCZ_codec_finalize)
		plugin->codec.codec->NCZ_codec_finalize(NULL);
        if(plugin->hdf5.filter != NULL) plugins.loaded_plugins[plugin->hdf5.filter->id] = NULL;
	if(plugin->hdf5.hdf5lib != NULL) (void)ncpsharedlibfree(plugin->hdf5.hdf5lib);
	if(!plugin->codec.defaulted && plugin->codec.codeclib != NULL) (void)ncpsharedlibfree(plugin->codec.codeclib);
memset(plugin,0,sizeof(NCZ_Plugin));
	free(plugin);
    }
    return ZUNTRACE(NC_NOERR);
}

#ifdef NAMEOPT
static int
pluginnamecheck(const char* name)
{
   size_t count,len;
   long i;
   const char* p;
   if(name == NULL) return 0;
   /* get basename */
   p = strrchr(name,'/');
   if(p != NULL) name = (p+1);
   len = strlen(name);
   if(len == 0) return 0;
   i = (long)(len-1);
   count = 1;
   p = name+i;
   for(;i>=0;i--,count++,p--) {
	char c = *p;
	if(c == '/') break;
	if(c == '.') {
	    /* exclude e.g xxx.so.1.2.3 in favor of xxx.so */
	    if(count >= 3 && strcmp(p,".so")==0) return 1;
    	    if(count >= 4 && strcmp(p,".dll")==0) return 1;
       	    if(count >= 6 && strcmp(p,".dylib")==0) return 1;
	}
   }
   return 0;
}
#endif

/**************************************************/
/* _Codecs attribute */

int
NCZ_codec_attr(const NC_VAR_INFO_T* var, size_t* lenp, void* data)
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

static int
ensure_working(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    if(filter->incomplete) {stat = THROW(NC_ENOFILTER); goto done;}
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
	    NCproplist* props = ncplistnew();
	    ncplistadd(props,"zarr_format",(uintptr_t)zfile->zarr.zarr_format);
    	    ncplistadd(props,"fileid",(size_t)ncidfor(var));
    	    ncplistadd(props,"varid",(uintptr_t)var->hdr.id);
	    stat = filter->plugin->codec.codec->NCZ_modify_parameters(props,&filter->hdf5.id,
				&filter->hdf5.visible.nparams, &filter->hdf5.visible.params,
				&filter->hdf5.working.nparams, &filter->hdf5.working.params);
	    ncplistfree(props);
#ifdef DEBUGF
	    fprintf(stderr,">>> DEBUGF: NCZ_modify_parameters: stat=%d ncid=%d varid=%d filter=%s\n",stat, (int)ncidfor(var),(int)var->hdr.id,
			printfilter(filter));
#endif
	    if(stat) goto done;
	} else {
	    /* assume visible are unchanged */
	    assert(oldnparams == filter->hdf5.visible.nparams && oldparams == filter->hdf5.visible.params); /* unchanged */
	    /* Just copy the visible parameters */
	    nullfree(filter->hdf5.working.params);
	    if((stat = paramnczclone(&filter->hdf5.visible,&filter->hdf5.working))) goto done;
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
    return THROW(stat);
}


/* Called by NCZ_enddef to ensure that the working parameters are defined */
int
NCZ_filter_setup(NC_VAR_INFO_T* var)
{
    size_t i;
    int stat = NC_NOERR;
    NClist* filters = NULL;
    NC_FILE_INFO_T* file = var->container->nc4_info;

    ZTRACE(6,"var=%s",var->hdr.name);

    filters = (NClist*)var->filters;
    for(i=0;i<nclistlength(filters);i++) {    
	NCZ_Filter* filter = (NCZ_Filter*)nclistget(filters,i);
        assert(filter != NULL);
 	/* verify */
        if((stat=NCZ_filter_verify(filter,var->type_info->varsized))) goto done;
        if(filter->incomplete) continue; /* ignore these */
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

/*
Check for filter incompleteness and suppression, etc/
*/
int
NCZ_filter_verify(NCZ_Filter* filter, int varsized)
{
    /* If this variable is not fixed size, mark filter as suppressed */
    if(varsized) {
	filter->suppress = 1;
	nclog(NCLOGWARN,"Filters cannot be applied to variable length data types; filter will be ignored");
    }
    if(filter->plugin == NULL) filter->incomplete = 1;
    else if(filter->plugin->incomplete) filter->incomplete = 1;
    if(!filter->incomplete)
        assert(filter->hdf5.id > 0 && (filter->hdf5.visible.nparams == 0 || filter->hdf5.visible.params != NULL));
    assert(filter->flags == 0);
    if(filter->hdf5.visible.nparams == 0 || filter->hdf5.visible.params != NULL) filter->flags |= FLAG_VISIBLE;
    return NC_NOERR;
}

/**************************************************/

/* Clone an hdf5 parameter set */
static int
paramclone(size_t nparams, const unsigned* src, unsigned** dstp)
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
paramnczclone(const NCZ_Params* src, NCZ_Params* dst)
{
    assert(src != NULL && dst != NULL && dst->params == NULL);
    *dst = *src;
    return paramclone(src->nparams,src->params,&dst->params);
}
