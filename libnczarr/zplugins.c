/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */
/**
 * @file @internal netcdf-4 functions for the plugin list.
 *
 * @author Dennis Heimbigner
 */

#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include "zincludes.h"
#include "ncpathmgr.h"
#include "ncpoco.h"
#include "netcdf_filter.h"
#include "netcdf_filter_build.h"
#include "zfilter.h"
#include "ncplugins.h"
#include "zplugins.h"
#ifdef _WIN32
#include <windows.h>
#endif

/**************************************************/
/* Forward */
static int NCZ_load_plugin(const char* path, NCZ_Plugin** plugp);
static int NCZ_unload_plugin(NCZ_Plugin* plugin);
static int NCZ_reclaim_plugin(NCZ_Plugin* plugin);
static int NCZ_load_plugin_dir(const char* path);
static int NCZ_plugin_save(NCZ_Plugin* p);
static int getentries(const char* path, NClist* contents);
static int loadcodecdefaults(const char* path, const NCZ_codec_t** cp, NCPSharedLib* lib, int* lib_usedp);
static void NCZ_sortedinsert(NCZ_Plugin* plugin, NClist* plugins);
static int setpluginid(NCZ_Plugin* plugin);
static int verifypluginid(NCZ_Plugin* plugin);

#if defined(NAMEOPT) || defined(_WIN32)
static int pluginnamecheck(const char* name);
#endif


/**************************************************/
/**
 * @file
 * @internal
 * Internal netcdf zarr plugin path functions.
 *
 * @author Dennis Heimbigner
 */

/**
 * This function is called as part of nc_plugin_path_initialize.
 * Its purpose is to initialize the plugin state.
 *
 * @return NC_NOERR
 *
 * @author Dennis Heimbigner
*/

int
NCZ_plugin_path_initialize(void)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    gs->zarr.pluginpaths = nclistnew();
    gs->zarr.default_libs = nclistnew();
    gs->zarr.codec_defaults = nclistnew();
    gs->zarr.loaded_plugins = nclistnew();
    if(gs->zarr.loaded_plugins == NULL) {stat = NC_ENOMEM; goto done;}

done:
    return stat;
}

/**
 * This function is called as part of nc_plugin_path_finalize.
 * Its purpose is to clean-up the plugin state.
 *
 * @return NC_NOERR
 *
 * @author Dennis Heimbigner
*/
int
NCZ_plugin_path_finalize(void)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();
    size_t i;

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Reclaim all loaded filters */
    while(nclistlength(gs->zarr.loaded_plugins) > 0) {
	NCZ_Plugin* plugin = nclistremove(gs->zarr.loaded_plugins,0);
#ifdef ZDEBUGF
	fprintf(stderr,"%s: reclaim loaded: %s\n",__func__,printplugin(plugin));
#endif
	if(plugin != NULL) {
            NCZ_unload_plugin(plugin);
	}
    }
    /* Reclaim the codec defaults */
    if(nclistlength(gs->zarr.codec_defaults) > 0) {
        for(i=0;i<nclistlength(gs->zarr.codec_defaults);i++) {
	    struct CodecAPI* ca = (struct CodecAPI*)nclistget(gs->zarr.codec_defaults,i);
    	    nullfree(ca);
	}
    }
    /* Reclaim the defaults library contents; Must occur as last act */
    if(nclistlength(gs->zarr.default_libs) > 0) {
        for(i=0;i<nclistlength(gs->zarr.default_libs);i++) {
	    NCPSharedLib* l = (NCPSharedLib*)nclistget(gs->zarr.default_libs,i);
    	    if(l != NULL) (void)ncpsharedlibfree(l);
	}
    }
#endif
    nclistfree(gs->zarr.loaded_plugins); gs->zarr.loaded_plugins = NULL;
    nclistfree(gs->zarr.default_libs); gs->zarr.default_libs = NULL;
    nclistfree(gs->zarr.codec_defaults); gs->zarr.codec_defaults = NULL;
    nclistfreeall(gs->zarr.pluginpaths); gs->zarr.pluginpaths = NULL;
    return THROW(stat);
}

/**
 * Return the length of the current sequence of directories
 * in the internal global plugin path list.
 * @param ndirsp length is returned here
 * @return NC_NOERR | NC_EXXX
 *
 * @author Dennis Heimbigner
 */
int
NCZ_plugin_path_ndirs(size_t* ndirsp)
{
    int stat = NC_NOERR;
    size_t ndirs = 0;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(gs->zarr.pluginpaths == NULL) gs->zarr.pluginpaths = nclistnew(); /* suspenders and belt */

    ndirs = nclistlength(gs->zarr.pluginpaths);
    if(ndirsp) *ndirsp = ndirs;
    return THROW(stat);
}

/**
 * Return the current sequence of directories in the internal global
 * plugin path list. Since this function does not modify the plugin path,
 * it can be called at any time.
 * @param dirs pointer to an NCPluginList object
 * @return NC_NOERR | NC_EXXX
 * @author Dennis Heimbigner
 *
 * WARNING: if dirs->dirs is NULL, then space for the directory
 * vector will be allocated. If not NULL, then the specified space will
 * be overwritten with the vector.
 *
 * @author: Dennis Heimbigner
*/
int
NCZ_plugin_path_get(NCPluginList* dirs)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(dirs == NULL) {stat = NC_EINVAL; goto done;}

    if(gs->zarr.pluginpaths == NULL) gs->zarr.pluginpaths = nclistnew(); /* suspenders and belt */

    dirs->ndirs = nclistlength(gs->zarr.pluginpaths);
    if(dirs->dirs == NULL && dirs->ndirs > 0) {
	if((dirs->dirs = (char**)calloc(dirs->ndirs,sizeof(char*)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
    }
    if(dirs->ndirs > 0) {
	size_t i;
	for(i=0;i<dirs->ndirs;i++) {
	    const char* dir = (const char*)nclistget(gs->zarr.pluginpaths,i);
	    dirs->dirs[i] = nulldup(dir);
	}
    }
done:
    return THROW(stat);
}

/**
 * Empty the current internal path sequence
 * and replace with the sequence of directories argument.
 * Using a dirs->ndirs argument of 0 will clear the set of plugin dirs.
 *
 * @param dirs to overwrite the current internal dir list
 * @return NC_NOERR | NC_EXXX
 *
 * @author Dennis Heimbigner
*/
int
NCZ_plugin_path_set(NCPluginList* dirs)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(dirs == NULL) {stat = NC_EINVAL; goto done;}
    if(dirs->ndirs > 0 && dirs->dirs == NULL) {stat = NC_EINVAL; goto done;}

    /* Clear the current dir list */
    nclistfreeall(gs->zarr.pluginpaths);
    gs->zarr.pluginpaths = nclistnew();

    if(dirs->ndirs > 0) {
	size_t i;
	for(i=0;i<dirs->ndirs;i++) {
	    nclistpush(gs->zarr.pluginpaths,nulldup(dirs->dirs[i]));
	}
    }
done:
    return THROW(stat);
}

/**************************************************/
/* Filter<->Plugin interface */

int
NCZ_load_all_plugins(void)
{
    int ret = NC_NOERR;
    size_t i,j;
    struct NCglobalstate* gs = NC_getglobalstate();
    struct stat buf;
    NClist* dirs = nclistnew();
    char* defaultpluginpath = NULL;

    ZTRACE(6,"");

    for(i=0;i<nclistlength(gs->pluginpaths);i++) {
	const char* dir = (const char*)nclistget(gs->pluginpaths,i);
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

    /* Try to provide default for any HDF5 filters without matching Codec. */
    if(nclistlength(gs->zarr.codec_defaults)) {
        /* Search the loaded_plugins */
        for(i=0;i<nclistlength(gs->zarr.loaded_plugins);i++) {
	    int matched = 0;
	    NCZ_Plugin* p = (NCZ_Plugin*)nclistget(gs->zarr.loaded_plugins,i);
	    /* Check if plugin has codec */
	    if(p != NULL && p->hdf5.filter != NULL && p->codec.codec == NULL) {
		/* Find for a default for this */
		matched = 0;
		for(j=0;!matched && j<nclistlength(gs->zarr.codec_defaults);j++) {
	            struct CodecAPI* dfalt = (struct CodecAPI*)nclistget(gs->zarr.codec_defaults,j);
		    if(dfalt->codec->hdf5id == p->hdf5id && !dfalt->ishdf5raw) {
			p->codec = *dfalt;
			p->codec.defaulted = 1;
			matched = 1;
		    }
		}
		/* Last chance: use hdfraw */
		if(!matched && gs->zarr.hdf5raw != NULL) {
		    p->codec = *gs->zarr.hdf5raw;
		    p->codec.defaulted = 1;
		}
	    }
	}
    }

    /* Mark all plugins for which we do not have both HDF5 and codec */
    {
        size_t i;
	NCZ_Plugin* p;
	for(i=0;i<nclistlength(gs->zarr.loaded_plugins);i++) {
	    if((p = nclistget(gs->zarr.loaded_plugins,i)) != NULL) {
		if(p->hdf5.filter == NULL || p->codec.codec == NULL) {
		    /* mark this entry as incomplete */
		    p->incomplete = 1;
		}
	    }
	}
    }

    /* Verify all plugins */
    {
        size_t i;
	NCZ_Plugin* p;
	for(i=0;i<nclistlength(gs->zarr.loaded_plugins);i++) {
	    if((p = nclistget(gs->zarr.loaded_plugins,i)) != NULL) {
		if((ret = verifypluginid(p))) goto done;
	    }
	}
    }

    /* Initalize all remaining plugins */
    {
        size_t i;
	NCZ_Plugin* p;
	NCproplist* props = ncproplistnew();
	for(i=0;i<nclistlength(gs->zarr.loaded_plugins);i++) {
	    if((p = nclistget(gs->zarr.loaded_plugins,i)) != NULL) {
		if(p->incomplete) continue;
		if(p->hdf5.filter != NULL && p->codec.codec != NULL) {
		    if(p->codec.codec && p->codec.codec->NCZ_codec_initialize)
			p->codec.codec->NCZ_codec_initialize(props);
		}
	    }
	}
	ncproplistfree(props);
    }
#ifdef ZDEBUGF
    fprintf(stderr,"%s:\n%s",__func__,printloadedplugins());
#endif
    
done:
    nullfree(defaultpluginpath);
    nclistfreeall(dirs);
    errno = 0;
    return ZUNTRACE(ret);
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
    struct NCglobalstate* gs = NC_getglobalstate();

    ZTRACE(7,"path=%s",path);


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
	stat = NCZ_load_plugin(file,&plugin);
	switch (stat) {
	case NC_NOERR: break;
	case NC_ENOFILTER: case NC_ENOTFOUND:
	    stat = NC_NOERR;
	    break; /* will cause it to be ignored */
	default: goto done;
	}
	if(plugin != NULL) {
	    size_t index = 0;
	    if((id=setpluginid(plugin))<=0) {stat = NC_ENCZARR; goto done;}
	    /* See if plugin already exists for this id */
	    if(!NCZ_plugin_index(id,&index)) {/* not found */
		/* sorted insertion of the new plugin */
		nclistinsert(gs->zarr.loaded_plugins,index,plugin);
	    } else {
	        NCZ_reclaim_plugin(plugin); /* its a duplicate */
	    }
	} else
	    stat = NC_NOERR; /*ignore failure */
    }	

done:
    nullfree(file);
    nclistfreeall(contents);
    return ZUNTRACE(stat);
}

/**
Locate the index of a plugin in the list of loaded plugins.
@param key id of plugin to locate
@param indexp contain location or insertion point of the plugin
@return 1 if found; 0 if not found
*/
int
NCZ_plugin_index(int key, size_t* indexp)
{
    int found = 0;
    struct NCglobalstate* gs = NC_getglobalstate();
    size_t L,R;
    const NClist* plugins = gs->zarr.loaded_plugins;
    size_t n = nclistlength(plugins);
    const NCZ_Plugin** list = NULL;

    if(n == 0) {found = 0; L = 0; goto done;} /* insert at position 0 */
    list = (const NCZ_Plugin**)nclistcontents(plugins);
    L = 0; R = n;
    while(L < R) {
#define ifloor(x) ((size_t)(x))
	size_t m = ifloor((L+R)/2);
	int mthkey = list[m]->hdf5id;
	assert(mthkey > 0);
	if(mthkey == key) found = 1;
	if(mthkey < key) {L = (m+1);} else {R = m;}
    }
    if(!found) L++; /* nclistinsert index */
done:
    if(indexp) *indexp = L;
    return found;
}

/**
Do a sorted insertion of a new plugin.
Constraint: plugin is not already in the plugin list.
@param plugin to insert
@param plugins plugin list
*/
static void
NCZ_sortedinsert(NCZ_Plugin* plugin, NClist* plugins)
{
    size_t i,nplugins,insertpoint;
    int ismaxkey;
    int key = plugin->hdf5id;

    assert(key > 0);
    nplugins = nclistlength(plugins);
    insertpoint = 0;
    ismaxkey = 1;
    for(i=0;i<nplugins;i++) {
	NCZ_Plugin* cur = (NCZ_Plugin*)nclistget(plugins,(size_t)i);
	int curkey = cur->hdf5id;
	assert(curkey != key);
	if(curkey > key) {insertpoint = i; ismaxkey = 0; break;}
    }
    if(ismaxkey)
        nclistpush(plugins,plugin); /* key is greater than any current key */
    else
	nclistinsert(plugins,(size_t)insertpoint,plugin); /* key is greater than any current key below insertpoint */
}

int
NCZ_load_plugin(const char* path, struct NCZ_Plugin** plugp)
{
    int stat = NC_NOERR;
    NCZ_Plugin* plugin = NULL;
    const H5Z_class2_t* h5class = NULL;
    H5PL_type_t h5type = 0;
    const NCZ_codec_t** cdfalts = NULL;
    const NCZ_codec_t* codec = NULL;
    NCPSharedLib* lib = NULL;
    int flags = NCP_GLOBAL;
    int h5id = 0;
    int newplugin = 0;
    
    assert(path != NULL && strlen(path) > 0 && plugp != NULL);

    ZTRACE(8,"path=%s",path);

    if(plugp) *plugp = NULL;

#if defined NAMEOPT || defined _WIN32
    /*triage because visual studio does a popup if the file will not load*/
    if(!pluginnamecheck(path)) {stat = NC_ENOFILTER; goto done;}
#endif

    /* load the shared library */
    if((stat = ncpsharedlibnew(&lib))) goto done;
    if((stat = ncpload(lib,path,flags))) goto done;

    /* See what we have */
    {
	/* h5tp is function indicating hdf5 is filter: H5PL_TYPE_FILTER */
	const H5PL_get_plugin_type_proto h5tp =  (H5PL_get_plugin_type_proto)ncpgetsymbol(lib,"H5PLget_plugin_type");
	/* h5fip is function returning hdf5 filter informtion struct: H5Z_class2_t* */
	const H5PL_get_plugin_info_proto h5fip =  (H5PL_get_plugin_info_proto)ncpgetsymbol(lib,"H5PLget_plugin_info");
	/* cip is function returning codec information struct: NCZ_codec_t* */
	const NCZ_get_codec_info_proto  cip =  (NCZ_get_codec_info_proto)ncpgetsymbol(lib,"NCZ_get_codec_info");
	/* cdp is function returning codec defaults vector: NCZ_codec_t** */
	const NCZ_codec_info_defaults_proto  cdp =  (NCZ_codec_info_defaults_proto)ncpgetsymbol(lib,"NCZ_codec_info_defaults");

        if(h5tp == NULL && h5fip == NULL && cip == NULL && cdp == NULL)
	    {stat = THROW(NC_ENOFILTER); goto done;}

#ifdef ZDEBUGF
	fprintf(stderr,"%s: NCZ_load_plugin: path=%s\n",__func__,path);
#endif

	/* We can have cdp  or we can have ((h5tp and h5fip) and/or cip) but not both sets */
	if(cdp != NULL) {
	    /* Get vector of default codecs */
	    cdfalts = (const NCZ_codec_t**)cdp();
	    if(cdfalts != NULL) {
		int used = 0;
	        if((stat = loadcodecdefaults(path,cdfalts,lib,&used))) goto done;
		if(used) lib = NULL;
	    }
   	    goto done;
        }
	/* if !cdp, then we can have h5tp and h5fip or we can have cip only or we can have all three */
        if(h5tp != NULL && h5fip != NULL) { /* get HDF5 info */
	    h5type = h5tp();
            h5class = h5fip();        
#ifdef ZDEBUGF
	    fprintf(stderr,"%s: load hdf5 class: %s\n",__func__,printhdf5class(h5class));
#endif
            /* Verify */
            if(h5type != H5PL_TYPE_FILTER) {stat = NC_EPLUGIN; goto done;}
            if(h5class->version != H5Z_CLASS_T_VERS) {stat = NC_EFILTER; goto done;}
        }
        if(cip != NULL) {/* get Codec info */
	    codec = cip();
#ifdef ZDEBUGF
	    fprintf(stderr,"%s: load codec class: %s\n",__func__,printcodecclass(codec));
#endif
            /* Verify */
            if(codec->version != NCZ_CODEC_CLASS_VER) {stat = NC_EPLUGIN; goto done;}
            if(codec->sort != NCZ_CODEC_HDF5) {stat = NC_EPLUGIN; goto done;}
        }
    }

    /* Verify codec consistency */
    if(h5class != NULL && codec != NULL) {
	/* Verify consistency of the HDF5 and the Codec */
	if(h5class->id != codec->hdf5id) goto done; /* ignore */
    } 

    /* There are several cases to consider:
    1. This library has both HDF5 API (h5tp+h5fip) and Codec API (cip) => merge
    2. This library has HDF5 API only => save and wait for Codec API to show up
    3. This library has Codec API only => save and wait for HD5 API to show up
    4. This library has HDF5 API only and Codec API was already found in another library => merge
    5. This library has Codec API only and HDF5 API was already found in another library => merge    
    */

    /* Get any previous plugin entry for this id; may be NULL */
    if(h5class != NULL) {
	h5id = (size_t)h5class->id;
    } else if(codec != NULL) {
	h5id = (int)codec->hdf5id;
    } else {stat = NC_ENOFILTER; goto done;}
    /* Find the corresponding (possibly incomplete) plugin */
    if((stat = NCZ_plugin_loaded(h5id,&plugin))) goto done;

    if(plugin == NULL) {
	/* create new entry */
	if((plugin = (NCZ_Plugin*)calloc(1,sizeof(NCZ_Plugin)))==NULL) {stat = NC_ENOMEM; goto done;}
	newplugin = 1;
#ifdef ZDEBUGF
	fprintf(stderr,"%s: create plugin\n",__func__);
#endif
    }
    
    /* Fill in the plugin */
    if(h5class != NULL && plugin->hdf5.filter == NULL) { /* case 2 or 4 */
	plugin->hdf5.filter = h5class;
	plugin->hdf5.hdf5lib = lib;
	lib = NULL;
#ifdef ZDEBUGF
	fprintf(stderr,"%s: fillin plugin hdf5: %s\n",__func__,printhdf5class(h5class));
#endif
    }
    if(codec != NULL && plugin->codec.codec == NULL) { /* case 3 or 5 */
	plugin->codec.codec = codec;
	plugin->codec.codeclib = lib;
	lib = NULL;
#ifdef ZDEBUGF
	fprintf(stderr,"%s: fillin plugin codec: %s\n",__func__,printcodecclass(codec));
#endif
    }
    /* Set the plugin HDF5 id */
    if(setpluginid(plugin)<=0) {stat = NC_ENCZARR; goto done;}

#ifdef ZDEBUGF
    fprintf(stderr,"%s: load plugin: %s\n",__func__,printplugin(plugin));
#endif
    /* Cleanup */
    if(plugin->hdf5.hdf5lib == plugin->codec.codeclib) /* Works for NULL case also */
	plugin->codec.codeclib = NULL;
    if(newplugin) {
	if((stat=NCZ_plugin_save(plugin))) goto done;
    } else assert(plugin->hdf5id > 0 && verifypluginid(plugin)==NC_NOERR);
    plugin = NULL;

done:
    if(lib)
       (void)ncpsharedlibfree(lib);
    if(plugin && newplugin) NCZ_unload_plugin(plugin);
    return ZUNTRACEX(stat,"plug=%p",*plugp);
}

static int
NCZ_unload_plugin(NCZ_Plugin* plugin)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    ZTRACE(9,"plugin=%p",plugin);
    if(plugin) {
	int id;
	size_t index = 0;
	if((id=setpluginid(plugin))<=0) {stat = NC_ENCZARR; goto done;}
	if(NCZ_plugin_index(id,&index)) {
	    /* Remove from plugin list */
	    nclistremove(gs->zarr.loaded_plugins,index);
	}	
#ifdef ZDEBUGF
    fprintf(stderr,"%s: unload plugin: %s\n",__func__,printplugin(plugin));
#endif
	if((stat=NCZ_reclaim_plugin(plugin))) goto done;
    }
done:
    return ZUNTRACE(stat);
}

static int
NCZ_reclaim_plugin(NCZ_Plugin* plugin)
{
    ZTRACE(9,"plugin=%p",plugin);
    if(plugin) {
	NCproplist* props = ncproplistnew();
	if(plugin->codec.codec && plugin->codec.codec->NCZ_codec_finalize)
		plugin->codec.codec->NCZ_codec_finalize(props);
	if(plugin->hdf5.hdf5lib != NULL) (void)ncpsharedlibfree(plugin->hdf5.hdf5lib);
	if(!plugin->codec.defaulted && plugin->codec.codeclib != NULL) (void)ncpsharedlibfree(plugin->codec.codeclib);
	memset(plugin,0,sizeof(NCZ_Plugin));
	free(plugin);
	ncproplistfree(props);
    }
    return ZUNTRACE(NC_NOERR);
}

int
NCZ_plugin_loaded(int filterid, NCZ_Plugin** pp)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();
    size_t index;
    NCZ_Plugin* p = NULL;

    ZTRACE(6,"filterid=%d",filterid);
    if(filterid <= 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    
    if(NCZ_plugin_index((int)filterid,&index)) {/* found */
	p = nclistget(gs->zarr.loaded_plugins,index);
    }
    if(pp) *pp = p;        
done:
    return ZUNTRACEX(stat,"plugin=%p",*pp);
}

int
NCZ_plugin_loaded_byname(const char* name, NCZ_Plugin** pp)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_Plugin* plug = NULL;
    struct NCglobalstate* gs = NC_getglobalstate();
    NClist* plugins = gs->zarr.loaded_plugins;

    ZTRACE(6,"pluginname=%s",name);
    if(name == NULL) {stat = NC_EINVAL; goto done;}
    for(i=0;i<nclistlength(plugins);i++) {
	NCZ_Plugin* p = (NCZ_Plugin*)nclistget(plugins,i);
        if(p == NULL || p->codec.codec == NULL) continue; /* no plugin or no codec */
        if(strcmp(name, p->codec.codec->codecid) == 0) {plug = p; break;}
    }
    if(pp) *pp = plug;
done:
    return ZUNTRACEX(stat,"plugin=%p",*pp);
}

static int
NCZ_plugin_save(NCZ_Plugin* p)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();
    int filterid ;

    if((filterid=setpluginid(p))<=0) {stat = NC_ENCZARR; goto done;}
    ZTRACE(6,"filterid=%d p=%p",filterid,p);
    if(filterid <= 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    NCZ_sortedinsert(p,gs->zarr.loaded_plugins);
done:
    return ZUNTRACE(stat);
}

static int
loadcodecdefaults(const char* path, const NCZ_codec_t** cp, NCPSharedLib* lib, int* lib_usedp)
{
    int stat = NC_NOERR;
    int lib_used = 0;
    struct NCglobalstate* gs = NC_getglobalstate();

    NC_UNUSED(path);

    nclistpush(gs->zarr.default_libs,lib);
    for(;*cp;cp++) {
	size_t i;
        struct CodecAPI* c0;
        c0 = (struct CodecAPI*)calloc(1,sizeof(struct CodecAPI));
	if(c0 == NULL) {stat = NC_ENOMEM; goto done;}
        c0->codec = *cp;
	c0->codeclib = lib;
	lib_used = 1; /* remember */
#ifdef ZDEBUGF
	fprintf(stderr,"%s: codec default: %s\n",__func__,printcodecapi(c0));
#endif
	/* Replace duplicates */
	for(i=0;i<nclistlength(gs->zarr.codec_defaults);i++) {
	    struct CodecAPI* cold = (struct CodecAPI*)nclistget(gs->zarr.codec_defaults,i);
	    if(cold != NULL && strcmp(c0->codec->codecid,cold->codec->codecid)==0) {
	        cold = (struct CodecAPI*)nclistremove(gs->zarr.codec_defaults,i);
#ifdef ZDEBUGF
		fprintf(stderr,"%s: remove old codec default: %s\n",__func__,printcodecapi(cold));
#endif
		nullfree(cold);
		break;
	    }
	}
	nclistpush(gs->zarr.codec_defaults,c0); c0 = NULL;
	/* Was this the hdf5raw codec? */
	for(i=0;i<nclistlength(gs->zarr.codec_defaults);i++) {
	    struct CodecAPI* codec = (struct CodecAPI*)nclistget(gs->zarr.codec_defaults,i);
	    if(codec->codec->hdf5id == H5Z_FILTER_RAW && strcasecmp(codec->codec->codecid,H5Z_CODEC_RAW)==0) {
	        gs->zarr.hdf5raw = codec; /* Overwrite any previous */
	    }
	}
    }
done:
    if(lib_usedp) *lib_usedp = lib_used;
    return stat;
}

static int
setpluginid(NCZ_Plugin* plugin)
{
    if(plugin->hdf5id <= 0) {
        if(plugin->hdf5.filter) plugin->hdf5id = plugin->hdf5.filter->id;
	else if(plugin->codec.codec) plugin->hdf5id = plugin->codec.codec->hdf5id;
    }
    return plugin->hdf5id;
}

static int
verifypluginid(NCZ_Plugin* plugin)
{
    int stat = NC_NOERR;
    int plugid = 0;
    int hdf5id = 0;
    int codecid = 0;
    
    plugid = plugin->hdf5id;
    if(plugin->hdf5.filter)
	hdf5id = plugin->hdf5.filter->id;
    else if(plugin->codec.codec)
	codecid = plugin->codec.codec->hdf5id;
    if(plugid == 0) {stat = NC_ENCZARR; goto done;}
    if((hdf5id != 0 && hdf5id != plugid) || (codecid != 0 && codecid != plugid))
	{stat = NC_ENCZARR; goto done;}
done:
    return stat;    
}

#if defined(NAMEOPT) || defined(_WIN32)
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
	    if(count >= 3 && memcmp(p,".so",3)==0) return 1;
    	    if(count >= 4 && memcmp(p,".dll",4)==0) return 1;
       	    if(count >= 6 && memcmp(p,".dylib",6)==0) return 1;
	}
   }
   return 0;
}
#endif

/**************************************************/
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
