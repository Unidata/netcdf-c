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
 * the COPYING file, which can be found at the root of the source code       *
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
#include "netcdf_filter_build.h"
#include "netcdf_aux.h"

#undef DEBUG
#undef DEBUGF
#undef DEBUGL

#define NULLIFY(x) ((x)?(x):"NULL")

/* Hold the loaded filter plugin information */
typedef struct NCZ_Plugin {
    struct HDF5API {
        const H5Z_class2_t* filter;
        NCPSharedLib* hdf5lib; /* source of the filter */
    } hdf5;
    struct CodecAPI {
	const NCZ_codec_t* codec;
	NCPSharedLib* codeclib; /* of the source codec; null if same as hdf5 */
    } codec;
} NCZ_Plugin;

/* The NC_VAR_INFO_T->filters field is an NClist of this struct */
/*
Each filter can have two parts: HDF5 and Codec.
The NC_VAR_INFO_T.filters list only holds entries where both the HDF5 info
and the codec info are defined.
The NCZ_VAR_INFO_T.codecs list holds the codec info when reading a Zarr file.
Note that it is not possible to have an entry on the filters list that does not
have both HDF5 and codec. This is because nc_def_var_filter will fail if the codec
part is not available. If a codec is read from a file and there is no available
corresponding HDF5 implementation, then that codec will not appear in the filters list.
It is possible that some subset of the codecs do have a corresponding HDF5, but we
enforce the rule that no entries go into the filters list unless all are defined.
It is still desirable for a user to be able to see what filters and codecs are defined
for a variable. This is accommodated by providing two special attributes:
1, "_Filters" attribute shows the HDF5 filters defined on the variable, if any.
2, "_Codecs" attribute shows the codecs defined on the variable; for zarr, this list
   should always be defined.
*/

/* Codec Info */
typedef struct NCZ_Codec {
    char* id;              /**< The NumCodecs ID */
    char* codec;           /**< The Codec from the file; NULL if creating */
} NCZ_Codec;

static NCZ_Codec codec_empty = {NULL, NULL};

static void
ncz_codec_clear(NCZ_Codec* codec) {
    nullfree(codec->id); nullfree(codec->codec);
    *codec = codec_empty;
}

typedef struct NCZ_Params {size_t nparams; unsigned* params;} NCZ_Params;

/* HDF5 Info */
typedef struct NCZ_HDF5 {
    unsigned id;           /**< HDF5 id corresponding to filterid. */
    NCZ_Params visible;
    NCZ_Params working;
} NCZ_HDF5;

static NCZ_HDF5 hdf5_empty = {0, {0,NULL}, {0,NULL}};

static void
ncz_hdf5_clear(NCZ_HDF5* h) {
    nullfree(h->visible.params);
    nullfree(h->working.params);
    *h = hdf5_empty;
}

typedef struct NCZ_Filter {
    int flags;             	/**< Flags describing state of this filter. */
#	define FLAG_VISIBLE 1 /* If set, then visible parameters are defined */
#	define FLAG_WORKING 2 /* If set, then WORKING parameters are defined */
#	define FLAG_CODEC 4 /* If set, then visbile parameters come from an existing codec string */
#	define FLAG_HDF5  8 /* If set, => visible parameters came from nc_def_var_filter */
#	define FLAG_NEWVISIBLE  16 /* If set, => visible parameters  were modified */
    NCZ_HDF5 hdf5;
    NCZ_Codec codec;
    struct NCZ_Plugin* plugin;  /**< Implementation of this filter. */
} NCZ_Filter;

/* WARNING: GLOBAL DATA */
/* All possible HDF5 filter plugins */
/* Convert to linked list or hash table or equivalent since very sparse */
NCZ_Plugin* loaded_plugins[H5Z_FILTER_MAX];
int loaded_plugins_max = -1;

static NCZ_codec_t** codec_defaults; /* NULL terminated */
static NCPSharedLib* default_lib; /* source of the defaults */

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


#ifdef DEBUGF

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
    snprintf(plbuf,sizeof(plbuf),"plugin={%s %s}",plbuf1,plbuf2);
    return plbuf;
}

static const char*
printparams(size_t nparams, const unsigned* params)
{
    static char ppbuf[4096];

    if(nparams == 0)
        snprintf(ppbuf,sizeof(ppbuf),"{0,%p}",params);
    else 
        snprintf(ppbuf,sizeof(ppbuf),"{%u %s}",(unsigned)nparams,nczprint_paramvector(nparams,params));
    return ppbuf;
}

static const char*
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
static int NCZ_plugin_loaded(int filterid, NCZ_Plugin** pp);
static int NCZ_plugin_save(int filterid, NCZ_Plugin* p);
static int NCZ_filter_free(NCZ_Filter* spec);
static int NCZ_filter_hdf5_clear(NCZ_HDF5* spec);
static int NCZ_filter_codec_clear(NCZ_Codec* spec);
static int NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, struct NCZ_Filter** specp);

static int getentries(const char* path, NClist* contents);
static int NCZ_split_plugin_path(const char* path0, NClist* list);

static int ensure_working(const NC_VAR_INFO_T* var, NCZ_Filter* filter);

static int paramnczclone(NCZ_Params* dst, const NCZ_Params* src);
static int paramclone(size_t nparams, unsigned** dstp, const unsigned* src);

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
NCZ_filter_freelist(NC_VAR_INFO_T* var)
{
    int i, stat=NC_NOERR;
    NClist* filters = (NClist*)var->filters;

    ZTRACE(6,"var=%s",var->hdr.name);
    if(filters == NULL) goto done;
    /* Free the filter list elements */
    for(i=0;i<nclistlength(filters);i++) {
	struct NCZ_Filter* spec = nclistget(filters,i);
	if((stat = NCZ_filter_free(spec))) goto done;
    }
    nclistfree(filters);
    var->filters = NULL;
done:
    return ZUNTRACE(stat);
}

static int
NCZ_filter_free(NCZ_Filter* spec)
{
    if(spec == NULL) return NC_NOERR;
    NCZ_filter_hdf5_clear(&spec->hdf5);
    NCZ_filter_codec_clear(&spec->codec);
    free(spec);
    return NC_NOERR;
}

static int
NCZ_filter_hdf5_clear(NCZ_HDF5* spec)
{
    ZTRACE(6,"spec=%d",spec->id);
    if(spec == NULL) goto done;
    nullfree(spec->visible.params);
    nullfree(spec->working.params);
done:
    return ZUNTRACE(NC_NOERR);
}

static int
NCZ_filter_codec_clear(NCZ_Codec* spec)
{
    ZTRACE(6,"spec=%d",(spec?spec->id:"null"));
    if(spec == NULL) goto done;
    nullfree(spec->id);
    nullfree(spec->codec);
done:
    return ZUNTRACE(NC_NOERR);
}

/* From NCZ_def_var_filter */
int
NCZ_addfilter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, unsigned int id, size_t nparams, const unsigned int* params)
{
    int stat = NC_NOERR;
    struct NCZ_Filter* fi = NULL;
    NCZ_Plugin* plugin = NULL;
    NCZ_HDF5 hdf5 = hdf5_empty;

    ZTRACE(6,"file=%s var=%s id=%u nparams=%u params=%p",file->hdr.name,var->hdr.name,id,nparams,params);
    
    if(nparams > 0 && params == NULL)
	{stat = NC_EINVAL; goto done;}
    
    if(var->filters == NULL) var->filters = (void*)nclistnew();

    /* Before anything else, find the matching plugin */
    if((stat = NCZ_plugin_loaded(id,&plugin))) goto done;
    if(plugin == NULL || plugin->codec.codec == NULL) { /* fail */
	ZLOG(NCLOGWARN,"no such plugin: %u",(unsigned)id);
	stat = NC_ENOFILTER;
	goto done;
    }

    /* Fill in the hdf5 */
    hdf5 = hdf5_empty;
    hdf5.id = id;
    /* Capture the visible parameters */
    hdf5.visible.nparams = nparams;
    if(nparams > 0) {
	if((stat = paramclone(nparams,&hdf5.visible.params,params))) goto done;
    }

    /* Find the NCZ_Filter */
    if((stat=NCZ_filter_lookup(var,id,&fi))) goto done;
    if(fi != NULL) {
	if(fi->plugin != plugin)
	    {stat = NC_EINTERNAL; goto done;}
    } else {
	stat = NC_NOERR;
        if((fi = calloc(1,sizeof(struct NCZ_Filter))) == NULL)
	    {stat = NC_ENOMEM; goto done;}
        fi->plugin = plugin;
	nclistpush((NClist*)var->filters, fi);
    }    

    /* (over)write the HDF5 parameters */
    nullfree(fi->hdf5.visible.params);
    nullfree(fi->hdf5.working.params);
    fi->hdf5.working.nparams = 0;
    fi->hdf5.working.params = NULL;
    fi->hdf5 = hdf5;
    hdf5 = hdf5_empty;
    fi->flags |= FLAG_VISIBLE;

    fi = NULL; /* either way,its in the var->filters list */

done:
    if(fi) NCZ_filter_free(fi);    
    return ZUNTRACE(stat);
}

int
NCZ_filter_remove(NC_VAR_INFO_T* var, unsigned int id)
{
    int k, stat = NC_NOERR;
    NClist* flist = (NClist*)var->filters;

    ZTRACE(6,"var=%s id=%u",var->hdr.name,id);
    /* Walk backwards */
    for(k=nclistlength(flist)-1;k>=0;k--) {
	struct NCZ_Filter* f = (struct NCZ_Filter*)nclistget(flist,k);
        if(f->hdf5.id == id) {
	    /* Remove from variable */
    	    nclistremove(flist,k);
	    /* Reclaim */
	    NCZ_filter_free(f);
	    goto done;
	}
    }
    ZLOG(NCLOGERR,"no such filter: %u",(unsigned)id);
    stat = NC_ENOFILTER;
done:
    return ZUNTRACE(stat);
}

static int
NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, struct NCZ_Filter** specp)
{
    int i;
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
	if(spec->hdf5.id == id) {
	    if(specp) *specp = spec;
	    break;
	}
    }
    return ZUNTRACEX(NC_NOERR,"spec=%d",IEXISTS(specp,hdf5.id));
}

#if 0
static int
NCZ_codec_lookup(NClist* codecs, const char* id, NCZ_Codec** codecp)
{
    int i;
    
    ZTRACE(6,"|codecs|=%u id=%u", (unsigned)nclistlength(codecs), id);
    if(codecp) *codecp = NULL;

    if(codecs == NULL) return NC_NOERR;
    for(i=0;i<nclistlength(codecs);i++) {
	NCZ_Codec* spec = nclistget(codecs,i);
	assert(spec != NULL);
	if(strcmp(spec->id,id)==0) {
	    if(codecp) *codecp = spec;
	    break;
	}
    }
    return ZUNTRACEX(NC_NOERR,"codec=%s",SEXISTS(codecp,id));
}

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
 * not netCDF-4/NCZARR.
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

    stat = NC4_nczarr_filter_remove(var,filterid);

    return stat;
}
#endif

#ifdef ENABLE_NCZARR_FILTERS
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
    if ((stat = ncz_find_grp_file_var(ncid, varid, &h5, &grp, &var)))
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

    if((stat = NCZ_filter_lookup(var,H5Z_FILTER_SZIP,&tmp))) goto done;
    haveszip = (tmp == NULL ? 0 : 1);

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
            for (d = 0; d < var->ndims; d++)
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

int
NCZ_inq_var_filter_ids(int ncid, int varid, size_t* nfiltersp, unsigned int* ids)
{
    int stat = NC_NOERR;
    NC *nc;
    NC_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NClist* flist = NULL;
    size_t nfilters = 0;

    ZTRACE(1,"ncid=%d varid=%d",ncid,varid);

    if((stat = NC_check_id(ncid,&nc))) goto done;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = ncz_find_grp_file_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    /* Make sure all the filters are defined */
    if((stat = NCZ_filter_initialize())) goto done;

    flist = var->filters;

    nfilters = nclistlength(flist);
    if(nfilters > 0 && ids != NULL) {
	int k;
	for(k=0;k<nfilters;k++) {
	    struct NCZ_Filter* f = (struct NCZ_Filter*)nclistget(flist,k);
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
    struct NCZ_Filter* spec = NULL;

    ZTRACE(1,"ncid=%d varid=%d id=%u",ncid,varid,id);
    
    if((stat = NC_check_id(ncid,&nc))) goto done;
    assert(nc);

    /* Find info for this file and group and var, and set pointer to each. */
    if ((stat = ncz_find_grp_file_var(ncid, varid, &h5, &grp, &var)))
	{stat = THROW(stat); goto done;}

    assert(h5 && var && var->hdr.id == varid);

    /* Make sure all the plugins are defined */
    if((stat = NCZ_filter_initialize())) goto done;

    if((stat = NCZ_filter_lookup(var,id,&spec))) goto done;
    if(spec != NULL) {
#if 0
	if(spec->flags & FLAG_WORKING) {/* working params are available */
	    if(spec->plugin->codec.codec->NCZ_visible_parameters) {
		stat = spec->plugin->codec.codec->NCZ_visible_parameters(ncid,varid,
								spec->hdf5.working.nparams,spec->hdf5.working.params,
								&spec->hdf5.visible.nparams,&spec->hdf5.visible.params);
#ifdef DEBUGF
	    fprintf(stderr,"DEBUGF: NCZ_visible_parameters: ncid=%d varid=%d working=%s visible=%s\n",ncid,varid,
			printnczparams(spec->hdf5.visible),printnczparams(spec->hdf5.working));
#endif
	        if(stat) goto done;
	    }
	    spec->flags |= FLAG_VISIBLE;
	}
#endif
	/* return the current visible parameters */
        if(nparamsp) *nparamsp = spec->hdf5.visible.nparams;
        if(params && spec->hdf5.visible.nparams > 0)
	    memcpy(params,spec->hdf5.visible.params,sizeof(unsigned int)*spec->hdf5.visible.nparams);
    } else {
        ZLOG(NCLOGWARN,"no such filter: %u",(unsigned)id);
        stat = NC_ENOFILTER;
    } 
done:
    return ZUNTRACEX(stat,"nparams=%u",(unsigned)(nparamsp?*nparamsp:0));
}
#endif /*ENABLE_NCZARR_FILTERS*/

/**************************************************/
/* Filter application functions */

int
NCZ_filter_initialize(void)
{
    int stat = NC_NOERR;
    ZTRACE(6,"");
    if(NCZ_filter_initialized) goto done;
    {
        NCZ_filter_initialized = 1;
        memset(loaded_plugins,0,sizeof(loaded_plugins));
#ifdef ENABLE_NCZARR_FILTERS
        if((stat = NCZ_load_all_plugins())) goto done;
#endif
    }
done:
    return ZUNTRACE(stat);
}

int
NCZ_filter_finalize(void)
{
    int stat = NC_NOERR;
    int i;
    ZTRACE(6,"");
    if(!NCZ_filter_initialized) goto done;
#ifdef ENABLE_NCZARR_FILTERS
    /* Reclaim all loaded filters */
    for(i=0;i<=loaded_plugins_max;i++) {
        NCZ_unload_plugin(loaded_plugins[i]);
	loaded_plugins[i] = NULL;
    }
    /* Reclaim the defaults library; Must occur as last act */
    if(default_lib != NULL) {(void)ncpsharedlibfree(default_lib); default_lib = NULL; codec_defaults = NULL;}
#else
    memset(loaded_plugins,0,sizeof(loaded_plugins));
#endif
done:
    NCZ_filter_initialized = 0;
    return ZUNTRACE(stat);
}

static int
NCZ_plugin_save(int filterid, NCZ_Plugin* p)
{
    int stat = NC_NOERR;
    ZTRACE(6,"filterid=%d p=%p",filterid,p);
    if(filterid <= 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    if(filterid > loaded_plugins_max) loaded_plugins_max = filterid;
    loaded_plugins[filterid] = p;
done:
    return ZUNTRACE(stat);
}

static int
NCZ_plugin_loaded(int filterid, NCZ_Plugin** pp)
{
    int stat = NC_NOERR;
    struct NCZ_Plugin* plug = NULL;
    ZTRACE(6,"filterid=%d",filterid);
    if(filterid <= 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    if(filterid <= loaded_plugins_max) 
        plug = loaded_plugins[filterid];
    if(pp) *pp = plug;
done:
    return ZUNTRACEX(stat,"plugin=%p",*pp);
}

int
NCZ_applyfilterchain(const NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* chain, size_t inlen, void* indata, size_t* outlenp, void** outdatap, int encode)
{
    int i, stat = NC_NOERR;
    void* lastbuffer = NULL; /* if not null, then last allocated buffer */

    ZTRACE(6,"|chain|=%u inlen=%u indata=%p encode=%d", (unsigned)nclistlength(chain), (unsigned)inlen, indata, encode);

    /* Make sure all the filters are loaded && setup */
    for(i=0;i<nclistlength(chain);i++) {
	struct NCZ_Filter* f = (struct NCZ_Filter*)nclistget(chain,i);
	assert(f != NULL && f->hdf5.id > 0 && f->plugin != NULL);
	if(!(f->flags & FLAG_WORKING)) {/* working not yet available */
	    if((stat = ensure_working(var,f))) goto done;
	}
    }

    {
	struct NCZ_Filter* f = NULL;
	const H5Z_class2_t* ff = NULL;
	size_t current_alloc = inlen;
	void* current_buf = indata;
	size_t current_used = inlen;
	size_t next_alloc = 0;
	void* next_buf = NULL;
	size_t next_used = 0;

#ifdef DEBUG
fprintf(stderr,"current: alloc=%u used=%u buf=%p\n",(unsigned)current_alloc,(unsigned)current_used,current_buf);
#endif
        /* Apply in proper order */
        if(encode) {
            for(i=0;i<nclistlength(chain);i++) {
	        f = (struct NCZ_Filter*)nclistget(chain,i);	
	        ff = f->plugin->hdf5.filter;
	        /* code can be simplified */
	        next_alloc = current_alloc;
	        next_buf = current_buf;
	        next_used = 0;
	        next_used = ff->filter(0,f->hdf5.working.nparams,f->hdf5.working.params,current_used,&next_alloc,&next_buf);
#ifdef DEBUG
fprintf(stderr,"next: alloc=%u used=%u buf=%p\n",(unsigned)next_alloc,(unsigned)next_used,next_buf);
#endif
		if(next_used == 0) {stat = NC_EFILTER; lastbuffer = next_buf; goto done; }
		/* If the filter did not need to create a new buffer, then next == current else current was reclaimed */
	        current_buf = next_buf;
	        current_alloc = next_alloc;
	        current_used = next_used;
	    }
	} else {
	    /* Apply in reverse order */
            for(i=nclistlength(chain)-1;i>=0;i--) {
	        f = (struct NCZ_Filter*)nclistget(chain,i);	
	        ff = f->plugin->hdf5.filter;
	        /* code can be simplified */
	        next_alloc = current_alloc;
	        next_buf = current_buf;
	        next_used = 0;
	        next_used = ff->filter(H5Z_FLAG_REVERSE,f->hdf5.working.nparams,f->hdf5.working.params,current_used,&next_alloc,&next_buf);
#ifdef DEBUG
fprintf(stderr,"next: alloc=%u used=%u buf=%p\n",(unsigned)next_alloc,(unsigned)next_used,next_buf);
#endif
		if(next_used == 0) {stat = NC_EFILTER; lastbuffer = next_buf; goto done;}
		/* If the filter did not need to create a new buffer, then next == current else current was reclaimed */
	        current_buf = next_buf;
	        current_alloc = next_alloc;
	        current_used = next_used;
	    }
	}
#ifdef DEBUG
fprintf(stderr,"current: alloc=%u used=%u buf=%p\n",(unsigned)current_alloc,(unsigned)current_used,current_buf);
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
/* JSON Parse/unparse of filters */
int
NCZ_filter_jsonize(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter, NCjson** jfilterp)
{
    int stat = NC_NOERR;
    NCjson* jfilter = NULL;
    
    ZTRACE(6,"var=%s filter=%s",var->hdr.name,(filter != NULL && filter->codec.id != NULL?filter->codec.id:"null"));

    /* assumptions */
    assert(filter->flags & FLAG_WORKING);

    /* Convert the HDF5 id + parameters to the codec form */

    /* We need to ensure the the current visible parameters are defined and had the opportunity to come
       from the working parameters */
    assert((filter->flags & (FLAG_VISIBLE | FLAG_WORKING)) == (FLAG_VISIBLE | FLAG_WORKING));
#if 0
    if((stat = rebuild_visible(var,filter))) goto done;
#endif

    /* Convert the visible parameters back to codec */
    /* Clear any previous codec */
    nullfree(filter->codec.id); filter->codec.id = NULL;
    nullfree(filter->codec.codec); filter->codec.codec = NULL;
    filter->codec.id = strdup(filter->plugin->codec.codec->codecid);
    if(filter->plugin->codec.codec->NCZ_hdf5_to_codec) {
	stat = filter->plugin->codec.codec->NCZ_hdf5_to_codec(filter->hdf5.visible.nparams,filter->hdf5.visible.params,&filter->codec.codec);
#ifdef DEBUGF
	fprintf(stderr,"DEBUGF: NCZ_hdf5_to_codec: visible=%s codec=%s\n",printnczparams(filter->hdf5.visible),filter->codec.codec);
#endif
        if(stat) goto done;
    } else
        {stat = NC_EFILTER; goto done;}

    /* Parse the codec as the return */
    if(NCJparse(filter->codec.codec,0,&jfilter) < 0) {stat = NC_EFILTER; goto done;}
    if(jfilterp) {*jfilterp = jfilter; jfilter = NULL;}

done:
    NCJreclaim(jfilter);
    return ZUNTRACEX(stat,"codec=%s",NULLIFY(filter->codec.codec));
}


/* Build filter from parsed Zarr metadata */
int
NCZ_filter_build(const NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const NCjson* jfilter)
{
    int i,stat = NC_NOERR;
    NCZ_Filter* filter = NULL;
    NCjson* jvalue = NULL;
    NCZ_Plugin* plugin = NULL;
    NCZ_Codec codec = codec_empty;
    NCZ_HDF5 hdf5 = hdf5_empty;

    ZTRACE(6,"file=%s var=%s jfilter=%s",file->hdr.name,var->hdr.name,NCJtrace(jfilter));

    if(var->filters == NULL) var->filters = nclistnew();

    /* Get the id of this codec filter */
    if(NCJdictget(jfilter,"id",&jvalue)<0) {stat = NC_EFILTER; goto done;}
    if(NCJsort(jvalue) != NCJ_STRING) {
        ZLOG(NCLOGERR,"no such filter: %s",NCJstring(jvalue));
	stat = NC_ENOFILTER; goto done;
    }

    /* Build the codec */
    if((codec.id = strdup(NCJstring(jvalue)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    if(NCJunparse(jfilter,0,&codec.codec)<0) {stat = NC_EFILTER; goto done;}

    /* Find the plugin for this filter */
    for(i=0;i<=loaded_plugins_max;i++) {
        if (!loaded_plugins[i]) continue;
        if(strcmp(NCJstring(jvalue), loaded_plugins[i]->codec.codec->codecid) == 0)
	    {plugin = loaded_plugins[i]; break;}
    }

    if(plugin != NULL) {
	/* Save the hdf5 id */
	hdf5.id = plugin->codec.codec->hdf5id;
	/* Convert the codec to hdf5 form visible parameters */
        if(plugin->codec.codec->NCZ_codec_to_hdf5) {
            stat = plugin->codec.codec->NCZ_codec_to_hdf5(codec.codec,&hdf5.visible.nparams,&hdf5.visible.params);
#ifdef DEBUGF
	    fprintf(stderr,"DEBUGF: NCZ_codec_to_hdf5: codec=%s, hdf5=%s\n",printcodec(codec),printhdf5(hdf5));
#endif
	    if(stat) goto done;
	}

	if((filter = calloc(1,sizeof(NCZ_Filter)))==NULL) {stat = NC_ENOMEM; goto done;}		
	filter->flags |= FLAG_VISIBLE;
	filter->hdf5 = hdf5; hdf5 = hdf5_empty;
	filter->codec = codec; codec = codec_empty;
	filter->flags |= FLAG_CODEC;
        filter->plugin = plugin; plugin = NULL;
    }

    if(filter != NULL) {
        NClist* filterlist = (NClist*)var->filters;
        nclistpush(filterlist,filter);
        filter = NULL;
    }
    
done:
    ncz_hdf5_clear(&hdf5);
    ncz_codec_clear(&codec);
    NCZ_filter_free(filter);
    return ZUNTRACE(stat);
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
	case ERROR_DIRECTORY: /* not a directory */
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
    int i,ret = NC_NOERR;
    const char* pluginroot = NULL;
    struct stat buf;
    NClist* dirs = nclistnew();
#ifdef _WIN32
    char pluginpath32[4096];
#endif

   ZTRACE(6,"");

#ifdef DEBUGL
   fprintf(stderr,"DEBUGL: NCZ_load_all_plugins\n");
#endif

   /* Find the plugin directory root(s) */
    pluginroot = getenv(plugin_env);
    if(pluginroot == NULL || strlen(pluginroot) == 0) {
#ifdef _WIN32
	const char* win32_root;
	win32_root = getenv(win32_root_env);
	if(win32_root != NULL && strlen(win32_root) > 0) {
	    snprintf(pluginpath32,sizeof(pluginpath32),plugin_dir_win,win32_root);
	    pluginroot = pluginpath32;
	} else
	    pluginroot = NULL;
#else /*!_WIN32*/
	pluginroot = plugin_dir_unix;
#endif
    }

    ZTRACEMORE(6,"pluginroot=%s",(pluginroot?pluginroot:"null"));
    if(pluginroot == NULL) {
        ZLOG(NCLOGERR,"no pluginroot: %s",plugin_env);
	ret = NC_ENOFILTER; goto done;
    }

    if((ret = NCZ_split_plugin_path(pluginroot,dirs))) goto done;

    for(i=0;i<nclistlength(dirs);i++) {
	const char* dir = (const char*)nclistget(dirs,i);
        /* Make sure the root is actually a directory */
        errno = 0;
        ret = NCstat(dir, &buf);
#if 1
        ZTRACEMORE(6,"stat: ret=%d, errno=%d st_mode=%d",ret,errno,buf.st_mode);
#endif
        if(ret < 0) {
	    ret = (errno);
        } else if(! S_ISDIR(buf.st_mode))
            ret = NC_EINVAL;
        if(ret) goto done;

        /* Try to load plugins from this directory */
        if((ret = NCZ_load_plugin_dir(dir))) goto done;
    }

    if(codec_defaults != NULL) { /* Try to provide default for any HDF5 filters without matching Codec. */
        NCZ_codec_t** dfalts = NULL;
        /* Search the defaults */
        for(dfalts = codec_defaults;*dfalts;dfalts++) {
	    NCZ_codec_t* codec = *dfalts;
	    int hdf5id = codec->hdf5id;
	    NCZ_Plugin* p = loaded_plugins[hdf5id]; /* get candidate */
	    if(p != NULL && p->hdf5.filter != NULL && p->codec.codec == NULL) {
#ifdef DEBUGL
	        fprintf(stderr,"DEBUGL: plugin defaulted: id=%u, codec=%s\n",hdf5id,codec->codecid);
#endif
		p->codec.codec = codec; codec = NULL;
		p->codec.codeclib = NULL;
	    }
	}
    }

    /* Expunge all plugins for which we do not have both HDF5 and codec */
    {
        int i;
	NCZ_Plugin* p;
	for(i=0;i<loaded_plugins_max;i++) {
	    if((p = loaded_plugins[i]) != NULL) {
		if(p->hdf5.filter == NULL || p->codec.codec == NULL) {
#ifdef DEBUGL
		    fprintf(stderr,"DEBUGL: plugin expunged: id=%u\n",p->hdf5.filter->id);
#endif
		    /* expunge this entry */
		    (void)NCZ_unload_plugin(p);
		    loaded_plugins[i] = NULL;
		}
	    }
	}
    }
    /* Iniitalize all remaining plugins */
    {
        int i;
	NCZ_Plugin* p;
	for(i=0;i<loaded_plugins_max;i++) {
	    if((p = loaded_plugins[i]) != NULL) {
		if(p->hdf5.filter != NULL && p->codec.codec != NULL) {
		    if(p->codec.codec && p->codec.codec->NCZ_codec_initialize)
			p->codec.codec->NCZ_codec_initialize();
#ifdef DEBUGL
		    fprintf(stderr,"DEBUGL: plugin initialized: id=%u\n",p->hdf5.filter->id);
#endif
		}
	    }
	}
    }
    
done:
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
    int i,stat = NC_NOERR;
    size_t pathlen;
    NClist* contents = nclistnew();
    char* file = NULL;

    ZTRACE(7,"path=%s",path);

#ifdef DEBUGL
   fprintf(stderr,"DEBUGL: NCZ_load_plugin_dir: path=%s\n",path);
#endif

    if(path == NULL) {stat = NC_EINVAL; goto done;}
    pathlen = strlen(path);
    if(pathlen == 0) {stat = NC_EINVAL; goto done;}

    if((stat = getentries(path,contents))) goto done;
    for(i=0;i<nclistlength(contents);i++) {
        const char* name = (const char*)nclistget(contents,i);
	size_t nmlen = strlen(name);
	size_t flen = pathlen+1+nmlen+1;
	int id;
	NCZ_Plugin* plugin = NULL;

	assert(nmlen > 0);
	nullfree(file); file = NULL;
	if((file = (char*)malloc(flen))==NULL) {stat = NC_ENOMEM; goto done;}
	file[0] = '\0';
	strlcat(file,path,flen);
	strlcat(file,"/",flen);
	strlcat(file,name,flen);
	/* See if can load the file */
	switch ((stat = NCZ_load_plugin(file,&plugin))) {
	case NC_NOERR: break;
	case NC_ENOFILTER: case NC_ENOTFOUND: stat = NC_NOERR; break; /* will cause it to be ignored */
	default: goto done;
	}
	if(plugin != NULL) {
	    id = plugin->hdf5.filter->id;
	    if(loaded_plugins[id] == NULL) {
	        loaded_plugins[id] = plugin;
		if(id > loaded_plugins_max) loaded_plugins_max = id;
#ifdef DEBUGL
		fprintf(stderr,"DEBUGL: plugin loaded: %s\n",printplugin(plugin));
#endif
	    } else {
#ifdef DEBUGL
		fprintf(stderr,"DEBUGL: plugin duplicate: %s\n",printplugin(plugin));
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

static int
NCZ_load_plugin(const char* path, struct NCZ_Plugin** plugp)
{
    int stat = NC_NOERR;
    NCZ_Plugin* plugin = NULL;
    const H5Z_class2_t* h5class = NULL;
    const NCZ_codec_t* codec = NULL;
    NCPSharedLib* lib = NULL;
    int flags = NCP_GLOBAL;
    int h5id = -1;
    
    assert(path != NULL && strlen(path) > 0 && plugp != NULL);

    ZTRACE(8,"path=%s",path);

#ifdef DEBUGL
   fprintf(stderr,"DEBUGL: NCZ_load_plugin: path=%s\n",path);
#endif

    if(plugp) *plugp = NULL;

#ifdef _WIN32
    /*triage because visual studio does a popup if the file will not load*/
    if(memcmp(path+(strlen(path)-4),".dll",4) != 0) {
	stat = NC_ENOFILTER; goto done;
    }
#endif

    /* load the shared library */
    if((stat = ncpsharedlibnew(&lib))) goto done;
    if((stat = ncpload(lib,path,flags))) goto done;

    /* See what we have */
    {
	H5PL_get_plugin_type_proto gpt =  (H5PL_get_plugin_type_proto)ncpgetsymbol(lib,"H5PLget_plugin_type");
	H5PL_get_plugin_info_proto gpi =  (H5PL_get_plugin_info_proto)ncpgetsymbol(lib,"H5PLget_plugin_info");
	NCZ_get_codec_info_proto  npi =  (NCZ_get_codec_info_proto)ncpgetsymbol(lib,"NCZ_get_codec_info");
	NCZ_codec_info_defaults_proto  cpd =  (NCZ_codec_info_defaults_proto)ncpgetsymbol(lib,"NCZ_codec_info_defaults");

	/* Deal with defaults first */
	if(cpd != NULL) {
	    if(codec_defaults == NULL) {
		codec_defaults = (NCZ_codec_t**)cpd();
		default_lib = lib; lib = NULL;
	    }
	    goto done;
	}	

	if(gpt != NULL && gpi != NULL) {
	    /* get HDF5 info */
	    H5PL_type_t h5type = gpt();
	    h5class = gpi();	    
	    /* Verify */
	    if(h5type != H5PL_TYPE_FILTER) {stat = NC_EPLUGIN; goto done;}
	    if(h5class->version != H5Z_CLASS_T_VERS) {stat = NC_EFILTER; goto done;}
	}
	
	if(npi != NULL) {
	    /* get Codec info */
	    codec = npi();
	    /* Verify */
            if(codec->version != NCZ_CODEC_CLASS_VER) {stat = NC_EPLUGIN; goto done;}
	    if(codec->sort != NCZ_CODEC_HDF5) {stat = NC_EPLUGIN; goto done;}
	}
    }

    /* Ignore this library if neither h5class nor codec are defined */
    if(h5class == NULL && codec == NULL) {stat = NC_ENOFILTER; goto done;}

#ifdef DEBUGL
fprintf(stderr,"DEBUGL: load: %s:",path);
if(h5class) fprintf(stderr," %u",(unsigned)h5class->id);
if(codec) fprintf(stderr," %u/%s",codec->hdf5id,codec->codecid);
fprintf(stderr,"\n");
#endif

    if(h5class != NULL && codec != NULL) {
	/* Verify consistency of the HDF5 and the Codec */
	if(h5class->id != codec->hdf5id) goto done; /* ignore */
    } 

    /* There are several cases to consider:
    1. This library has both HDF5 API and Codec API => merge
    2. This library has HDF5 API only and Codec API was already found in another library => merge
    3. This library has Codec API only and HDF5 API was already found in another library => merge    
    */

    /* Get any previous plugin entry for this id; may be NULL */
    if(h5class != NULL) {
	h5id = h5class->id;
	if((stat = NCZ_plugin_loaded(h5class->id,&plugin))) goto done;
    } else if(codec != NULL) {
	h5id = codec->hdf5id;
	if((stat = NCZ_plugin_loaded(codec->hdf5id,&plugin))) goto done;
    }
    if(plugin == NULL) {
	/* create new entry */
	if((plugin = (NCZ_Plugin*)calloc(1,sizeof(NCZ_Plugin)))==NULL) {stat = NC_ENOMEM; goto done;}
    } 

    /* Fill in the plugin */
    if(plugin->hdf5.filter == NULL) {
	plugin->hdf5.filter = h5class;
	plugin->hdf5.hdf5lib = lib;
	lib = NULL;
    }
    if(plugin->codec.codec == NULL) {
	plugin->codec.codec = codec;
	plugin->codec.codeclib = lib;
	lib = NULL;
    }

#ifdef DEBUGL
   if(plugin)
       fprintf(stderr,"DEBUGL: load_plugin: %s\n",printplugin(plugin));
#endif


    /* Cleanup */
    if(plugin->hdf5.hdf5lib == plugin->codec.codeclib)
	    plugin->codec.codeclib = NULL;
    if((stat=NCZ_plugin_save(h5id,plugin))) goto done;
    plugin = NULL;

done:
    if(lib) {
        (void)ncpsharedlibfree(lib);
    }
    if(plugin) NCZ_unload_plugin(plugin);
    return ZUNTRACEX(stat,"plug=%p",*plugp);
}

static int
NCZ_unload_plugin(NCZ_Plugin* plugin)
{
    ZTRACE(9,"plugin=%p",plugin);

    if(plugin) {
#ifdef DEBUGL
        fprintf(stderr,"DEBUGL: unload: %s\n",printplugin(plugin));
#endif
	if(plugin->codec.codec && plugin->codec.codec->NCZ_codec_finalize)
		plugin->codec.codec->NCZ_codec_finalize();
        if(plugin->hdf5.filter != NULL) loaded_plugins[plugin->hdf5.filter->id] = NULL;
	if(plugin->hdf5.hdf5lib != NULL) (void)ncpsharedlibfree(plugin->hdf5.hdf5lib);
	if(plugin->codec.codeclib != NULL) (void)ncpsharedlibfree(plugin->codec.codeclib);
	memset(plugin,0,sizeof(NCZ_Plugin));
	free(plugin);
    }
    return ZUNTRACE(NC_NOERR);
}

/**************************************************/
/* _Codecs attribute */

int
NCZ_codec_attr(const NC_VAR_INFO_T* var, size_t* lenp, void* data)
{
    int i,stat = NC_NOERR;
    size_t len;
    char* contents = NULL;
    NCbytes* buf = NULL;
    NClist* filters = (NClist*)var->filters;

    ZTRACE(6,"var=%s",var->hdr.name);
    if(nclistlength(filters) == 0) {stat = NC_ENOTATT; goto done;}
    buf = ncbytesnew(); ncbytessetalloc(buf,1024);
    ncbytescat(buf,"[");
    for(i=0;i<nclistlength(filters);i++) {
       	NCZ_Filter* spec = nclistget(filters,i);
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
ensure_working(const NC_VAR_INFO_T* var, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
    if(!(filter->flags & FLAG_WORKING)) {
	const size_t oldnparams = filter->hdf5.visible.nparams;
	const unsigned* oldparams = filter->hdf5.visible.params;

	assert(filter->flags & FLAG_VISIBLE);
        /* Convert the visible parameters to working parameters; may also modify the visible params */
        if(filter->plugin && filter->plugin->codec.codec->NCZ_modify_parameters) {
	    stat = filter->plugin->codec.codec->NCZ_modify_parameters(ncidfor(var),var->hdr.id,
				&filter->hdf5.visible.nparams, &filter->hdf5.visible.params,
				&filter->hdf5.working.nparams, &filter->hdf5.working.params);
#ifdef DEBUGF
	    fprintf(stderr,"DEBUGF: NCZ_modify_parameters: ncid=%d varid=%d filter=%s\n", (int)ncidfor(var),(int)var->hdr.id,
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
	    nullfree(filter->hdf5.working.params);
	    if((stat = paramnczclone(&filter->hdf5.working,&filter->hdf5.visible))) goto done;
	}
	filter->flags |= FLAG_WORKING;
    }
#ifdef DEBUGF
    fprintf(stderr,"DEBUGF: ensure_working_parameters: ncid=%lu varid=%u filter=%s\n", ncidfor(var), (unsigned)var->hdr.id,printfilter(filter));
#endif
done:
    return THROW(stat);
}

#if 0
static int
rebuild_visible(const NC_VAR_INFO_T* var, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
    int nvisible0;
    unsigned* visible0 = NULL;

    assert(filter->flags & FLAG_WORKING);
    /* If the visible parameters are previously defined, save them */
    if(filter->flags & FLAG_VISIBLE) {
        nvisible0 = filter->hdf5.visible.nparams;
	visible0 = filter->hdf5.visible.params;
        filter->hdf5.visible.nparams = 0;
	filter->hdf5.visible.params = NULL; /* temporary */
    }
    /* Cases to consider:
       1. visible already defined && NCZ_visible_parameters defined => apply 
       2. visible not defined && NCZ_visible_parameters defined defined => apply
       3. visible already defined && NCZ_visible_parameters not defined => keep originals
       4. visible not defined && NCZ_visible_parameters not defined => use working parameters
    */

    /* Cases 1 and 2 */
    /* Convert the working parameters to visibleparameters, overwriting any existing visibles */
    if(filter->plugin->codec.codec->NCZ_visible_parameters) {
        stat = filter->plugin->codec.codec->NCZ_visible_parameters(ncidfor(var),var->hdr.id,
				filter->hdf5.working.nparams, filter->hdf5.working.params,
				&filter->hdf5.visible.nparams, &filter->hdf5.visible.params);
	        if(stat) goto done;
    } else if(filter->flags & FLAG_CODEC) {/* Case 3 */
        filter->hdf5.visible.nparams = nvisible0;
	filter->hdf5.visible.params = visible0; visible0 = NULL;
    } else {/* Case 4 */
	/* Use the working parameters as the visible parameters */
        filter->hdf5.visible.nparams = filter->hdf5.working.nparams;
	if(filter->hdf5.working.nparams > 0) {
	    if((stat = paramnczclone(&filter->hdf5.visible,&filter->hdf5.working))) goto done;
	}
    }
    filter->flags |= FLAG_VISIBLE;
#ifdef DEBUGF
    fprintf(stderr,"DEBUGF: rebuild_visible_parameters: ncid=%lu varid=%u filter=%s\n", ncidfor(var), (unsigned)var->hdr.id,printfilter(filter));
#endif
done:
    nullfree(visible0);  
    return THROW(stat);
}
#endif

/* Called by NCZ_enddef to ensure that the working parameters are defined */
int
NCZ_filter_setup(NC_VAR_INFO_T* var)
{
    int i,stat = NC_NOERR;
    NClist* filters = NULL;

    ZTRACE(6,"var=%s",var->hdr.name);

    filters = (NClist*)var->filters;
    for(i=0;i<nclistlength(filters);i++) {    
	NCZ_Filter* filter = (NCZ_Filter*)nclistget(filters,i);
        assert(filter != NULL && filter->plugin != NULL);
        assert((filter->flags & FLAG_VISIBLE)); /* Assume visible params are defined */
 	/* verify */
	assert(filter->hdf5.id > 0 && (filter->hdf5.visible.nparams == 0 || filter->hdf5.visible.params != NULL));
	/* Initialize the working parameters */
	if((stat = ensure_working(var,filter))) goto done;
#ifdef DEBUGF
	fprintf(stderr,"DEBUGF: NCZ_filter_setup: ncid=%d varid=%d filter=%s\n", (int)ncidfor(var),(int)var->hdr.id,
			printfilter(filter));
#endif
    }

done:
    return ZUNTRACE(stat);
}

/**************************************************/

/* Clone an hdf5 parameter set */
static int
paramclone(size_t nparams, unsigned** dstp, const unsigned* src)
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
    return paramclone(src->nparams,&dst->params,src->params);
}
