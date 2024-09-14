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

#if 0
#define DEBUG
#define DEBUGF
#define DEBUGL
#define DEBUGPL
#endif

/**************************************************/
/* Forward */

static int NCZ_plugin_path_initialize(void** statep, const NClist* initialpaths);
static int NCZ_plugin_path_finalize(void** statep);
static int NCZ_plugin_path_read(void* state, size_t* ndirsp, char** dirs);
static int NCZ_plugin_path_write(void* state, size_t ndirs, char** const dirs);

static int NCZ_load_plugin(const char* path, NCZ_Plugin** plugp);
static int NCZ_unload_plugin(NCZ_Plugin* plugin);
static int NCZ_load_plugin_dir(GlobalNCZarr* gz, const char* path);
static int NCZ_plugin_save(GlobalNCZarr* gz, size_t filterid, NCZ_Plugin* p);
static int getentries(const char* path, NClist* contents);
static int loadcodecdefaults(GlobalNCZarr* gz, const char* path, const NCZ_codec_t** cp, NCPSharedLib* lib, int* lib_usedp);

#if defined NAMEOPT || defined _WIN32
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

/**************************************************/

/**************************************************/
/* The NCZarr Plugin Path Dispatch table and functions */
NC_PluginPathDispatch NCZ_pluginpathdispatch = {
    NC_FORMATX_NCZARR,
    NC_PLUGINPATH_DISPATCH_VERSION,
    NCZ_plugin_path_initialize,
    NCZ_plugin_path_finalize,
    NCZ_plugin_path_read,
    NCZ_plugin_path_write,
};

NC_PluginPathDispatch* NCZ_pluginpathtable = &NCZ_pluginpathdispatch;

/**************************************************/
/**
 * This function is called as part of nc_initialize.
 * Its purpose is to initialize the plugin paths state.
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/
static int
NCZ_plugin_path_initialize(void** statep, const NClist* initialpaths)
{
    int stat = NC_NOERR;
    GlobalNCZarr* gz = NULL;

    assert(statep != NULL);
    if(*statep != NULL) goto done; /* already initialized */

    if((gz = (GlobalNCZarr*)calloc(1,sizeof(GlobalNCZarr)))==NULL) {stat = NC_ENOMEM; goto done;}

    gz->pluginpaths = nclistnew();
    gz->default_libs = nclistnew();
    gz->codec_defaults = nclistnew();
    gz->loaded_plugins = (struct NCZ_Plugin**)calloc(H5Z_FILTER_MAX+1,sizeof(struct NCZ_Plugin*));
    if(gz->loaded_plugins == NULL) {stat = NC_ENOMEM; goto done;}

    /* Preload with set of initial paths (typically coming from HDF5_PLUGIN_PATH) */
    if(nclistlength(initialpaths) > 0) {
	size_t i;
	for(i=0;i<nclistlength(initialpaths);i++) {
	    const char* dir = (const char*)nclistget(initialpaths,i);
	    if(dir != NULL) nclistpush(gz->pluginpaths,strdup(dir));
	}
    }
    
    *statep = (void*)gz; gz = NULL;
done:
    nullfree(gz);
    return THROW(stat);
}

/**
 * This function is called as part of nc_finalize()
 * Its purpose is to clean-up plugin path state.
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/
static int
NCZ_plugin_path_finalize(void** statep)
{
    int stat = NC_NOERR;
    GlobalNCZarr* gz = NULL;

    assert(statep != NULL);
    if(*statep == NULL) goto done; /* already finalized */
    gz = (GlobalNCZarr*)(*statep);

    /* Reclaim all loaded filters */
#ifdef DEBUGL
    fprintf(stderr,">>>  DEBUGL: finalize reclaim:\n");
#endif
    { size_t i;
    for(i=1;i<=gz->loaded_plugins_max;i++) {
	if(gz->loaded_plugins[i]) {
            NCZ_unload_plugin(gz->loaded_plugins[i]);
	    gz->loaded_plugins[i] = NULL;
	}
    }
    }
    /* Reclaim the codec defaults */
    if(nclistlength(gz->codec_defaults) > 0) {
        size_t i;
        for(i=0;i<nclistlength(gz->codec_defaults);i++) {
	    struct CodecAPI* ca = (struct CodecAPI*)nclistget(gz->codec_defaults,i);
    	    nullfree(ca);
	}
    }
    /* Reclaim the defaults library contents; Must occur as last act */
    if(nclistlength(gz->default_libs) > 0) {
	size_t i;
        for(i=0;i<nclistlength(gz->default_libs);i++) {
	    NCPSharedLib* l = (NCPSharedLib*)nclistget(gz->default_libs,i);
#ifdef DEBUGL
   fprintf(stderr,">>> DEBUGL: NCZ_filter_finalize: reclaim default_lib[i]=%p\n",l);
#endif
    	    if(l != NULL) (void)ncpsharedlibfree(l);
	}
    }
    gz->loaded_plugins_max = 0;
    nullfree(gz->loaded_plugins); gz->loaded_plugins = NULL;
assert(gz->default_libs != NULL);
    nclistfree(gz->default_libs); gz->default_libs = NULL;
    nclistfree(gz->codec_defaults); gz->codec_defaults = NULL;
    nclistfreeall(gz->pluginpaths); gz->pluginpaths = NULL;

    *statep = NULL;

done:
    nullfree(gz);
    return THROW(stat);
}

/**
 * Return the current sequence of directories in the internal plugin path list.
 * Since this function does not modify the plugin path, it can be called at any time.
 *
 * @param state the per-dispatcher state
 * @param ndirsp return the number of paths in the path list
 * @param dirs copy the sequence of directories in the path list into this; caller must free the copied strings
 *
 * @return ::NC_NOERR
 * @return ::NC_EINVAL if formatx is unknown or zero
 *
 * @author Dennis Heimbigner
 *
 * As a rule, this function needs to be called twice.  The first time
 * with ndirsp not NULL and dirs set to NULL to get the size of
 * the path list. The second time with dirs not NULL to get the
 * actual sequence of paths.
 * Note also that it is not possible to read a subset of the plugin path.
 * If the dirs argument is not NULL, then this code assumes it points
 * to enough memory to hold the whole plugin path.
*/

static int
NCZ_plugin_path_read(void* state, size_t* ndirsp, char** dirs)
{
    int stat = NC_NOERR;
    GlobalNCZarr* gz = (GlobalNCZarr*)state;
    size_t ndirs = 0;

    ndirs = nclistlength(gz->pluginpaths);
    if(ndirsp) *ndirsp = ndirs;

    if(ndirs > 0 && dirs != NULL) {
	size_t i;
	for(i=0;i<ndirs;i++) {
	    const char* dir = (const char*)nclistget(gz->pluginpaths,i);
	    dirs[i] = strdup(dir);
	}
    }
    return THROW(stat);
}

/**
 * Empty the current internal path sequence
 * and replace with the sequence of directories
 * specified in the arguments.
 * If ndirs == 0 the path list will be cleared
 *
 * @param state the per-dispatcher state
 * @param ndirs number of entries in dirs arg
 * @param dirs the actual directory paths
 *
 * @return ::NC_NOERR
 * @return ::NC_EINVAL if formatx is unknown or ndirs > 0 and dirs == NULL
 *
 * @author Dennis Heimbigner
*/

static int
NCZ_plugin_path_write(void* state, size_t ndirs, char** const dirs)
{
    int stat = NC_NOERR;
    GlobalNCZarr* gz = (GlobalNCZarr*)state;

    if(ndirs > 0 && dirs == NULL) {stat = NC_EINVAL; goto done;}

    /* Clear the current path list */
    nclistfreeall(gz->pluginpaths);
    gz->pluginpaths = nclistnew();

    if(ndirs > 0 && dirs != NULL) {
	size_t i;
	for(i=0;i<ndirs;i++)
	    nclistpush(gz->pluginpaths,nulldup(dirs[i]));
    }

done:
    if(gz->pluginpaths == NULL)
        gz->pluginpaths = nclistnew();
    return NCTHROW(stat);
}

/**************************************************/
/* Filter<->Plugin interface */

int
NCZ_load_all_plugins(void)
{
    int ret = NC_NOERR;
    size_t i,j;
    struct stat buf;
    NClist* dirs = nclistnew();
    char* defaultpluginpath = NULL;
    NCglobalstate* gs = NC_getglobalstate();
    GlobalNCZarr* gz = (GlobalNCZarr*)gs->formatxstate.state[NC_FORMATX_NCZARR];
    ZTRACE(6,"");

#ifdef DEBUGL
   fprintf(stderr,">>> DEBUGL: NCZ_load_all_plugins\n");
#endif

    for(i=0;i<nclistlength(gz->pluginpaths);i++) {
	const char* dir = (const char*)nclistget(gz->pluginpaths,i);
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
        if((ret = NCZ_load_plugin_dir(gz,dir))) goto done;
    }
#ifdef DEBUGL
    { size_t i;
	fprintf(stderr,"gz->codec_defaults:");
	for(i=0;i<nclistlength(gz->codec_defaults);i++) {
	    struct CodecAPI* codec = (struct CodecAPI*)nclistget(gz->codec_defaults,i);
	    fprintf(stderr," %d",codec->codec->hdf5id);	    
	}
	fprintf(stderr,"\n");
    }
#endif
    if(nclistlength(gz->codec_defaults)) { /* Try to provide default for any HDF5 filters without matching Codec. */
        /* Search the defaults */
	for(j=0;j<nclistlength(gz->codec_defaults);j++) {
            struct CodecAPI* dfalt = (struct CodecAPI*)nclistget(gz->codec_defaults,j);
	    if(dfalt->codec != NULL) {
	        const NCZ_codec_t* codec = dfalt->codec;
	        size_t hdf5id = codec->hdf5id;
		NCZ_Plugin* p = NULL;
		if(hdf5id <= 0 || hdf5id > gz->loaded_plugins_max) {ret = NC_EFILTER; goto done;}
	        p = gz->loaded_plugins[hdf5id]; /* get candidate */
	        if(p != NULL && p->hdf5.filter != NULL
                   && p->codec.codec == NULL) {
#ifdef DEBUGL
	            fprintf(stderr,">>> DEBUGL: plugin defaulted: id=%u, codec=%s src=%s\n",hdf5id,codec->codecid,dfalt->codeclib->path);
#endif
		    p->codec.codec = codec;
		    p->codec.codeclib = dfalt->codeclib;
		    p->codec.defaulted = 1;
		}
	    }
	}
    }

    /* Mark all plugins for which we do not have both HDF5 and codec */
    {
        size_t i;
	NCZ_Plugin* p;
	for(i=1;i<gz->loaded_plugins_max;i++) {
	    if((p = gz->loaded_plugins[i]) != NULL) {
		if(p->hdf5.filter == NULL || p->codec.codec == NULL) {
		    /* mark this entry as incomplete */
		    p->incomplete = 1;
#ifdef DEBUGL
		    fprintf(stderr,">>>  DEBUGL: Incomplete plugin: id=%u; reasons: %s %s\n",i,
		    		(p->hdf5.filter==NULL?"hdf5":""),(p->codec.codec==NULL?"codec":""));
#endif
		}
#ifdef DEBUGL
		else
		    fprintf(stderr,">>> DEBUGL: plugin accepted: id=%u\n",i);
#endif
	    }
	}
    }
    /* Iniitalize all remaining plugins */
    {
        size_t i;
	NCZ_Plugin* p;
	for(i=1;i<gz->loaded_plugins_max;i++) {
	    if((p = gz->loaded_plugins[i]) != NULL) {
		if(p->incomplete) continue;
		if(p->hdf5.filter != NULL && p->codec.codec != NULL) {
		    if(p->codec.codec && p->codec.codec->NCZ_codec_initialize)
			p->codec.codec->NCZ_codec_initialize();
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

/* Load all the filters within a specified directory */
static int
NCZ_load_plugin_dir(GlobalNCZarr* gz, const char* path)
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
	size_t id;
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
	    id = (size_t)plugin->hdf5.filter->id;
	    if(gz->loaded_plugins[id] == NULL) {
	        gz->loaded_plugins[id] = plugin;
		if(id > gz->loaded_plugins_max) gz->loaded_plugins_max = id;
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
    size_t h5id = 0;
    NCglobalstate* gs = NC_getglobalstate();
    GlobalNCZarr* gz = (GlobalNCZarr*)gs->formatxstate.state[NC_FORMATX_NCZARR];
    
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
        if((stat = loadcodecdefaults(gz,path,cp,lib,&used))) goto done;
	if(used) lib = NULL;
	goto done;
    }

    if(h5class != NULL && codec != NULL) {
	/* Verify consistency of the HDF5 and the Codec */
	if(((size_t)h5class->id) != codec->hdf5id) goto done; /* ignore */
    } 

    /* There are several cases to consider:
    1. This library has both HDF5 API and Codec API => merge
    2. This library has HDF5 API only and Codec API was already found in another library => merge
    3. This library has Codec API only and HDF5 API was already found in another library => merge    
    */

    /* Get any previous plugin entry for this id; may be NULL */
    if(h5class != NULL) {
	h5id = (size_t)h5class->id;
	if((stat = NCZ_plugin_loaded(h5id,&plugin))) goto done;
    } else if(codec != NULL) {
	h5id = (size_t)codec->hdf5id;
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
    if((stat=NCZ_plugin_save(gz,h5id,plugin))) goto done;
    plugin = NULL;

done:
    if(lib)
       (void)ncpsharedlibfree(lib);
    if(plugin) NCZ_unload_plugin(plugin);
    return ZUNTRACEX(stat,"plug=%p",*plugp);
}

int
NCZ_unload_plugin(NCZ_Plugin* plugin)
{
    NCglobalstate* gs = NC_getglobalstate();
    GlobalNCZarr* gz = (GlobalNCZarr*)gs->formatxstate.state[NC_FORMATX_NCZARR];

    ZTRACE(9,"plugin=%p",plugin);

    if(plugin) {
#ifdef DEBUGL
        fprintf(stderr,">>> DEBUGL: unload: %s\n",printplugin(plugin));
#endif
	if(plugin->codec.codec && plugin->codec.codec->NCZ_codec_finalize)
		plugin->codec.codec->NCZ_codec_finalize();
        if(plugin->hdf5.filter != NULL) gz->loaded_plugins[plugin->hdf5.filter->id] = NULL;
	if(plugin->hdf5.hdf5lib != NULL) (void)ncpsharedlibfree(plugin->hdf5.hdf5lib);
	if(!plugin->codec.defaulted && plugin->codec.codeclib != NULL) (void)ncpsharedlibfree(plugin->codec.codeclib);
memset(plugin,0,sizeof(NCZ_Plugin));
	free(plugin);
    }
    return ZUNTRACE(NC_NOERR);
}

int
NCZ_plugin_loaded(size_t filterid, NCZ_Plugin** pp)
{
    int stat = NC_NOERR;
    NCglobalstate* gs = NC_getglobalstate();
    GlobalNCZarr* gz = (GlobalNCZarr*)gs->formatxstate.state[NC_FORMATX_NCZARR];

    struct NCZ_Plugin* plug = NULL;
    ZTRACE(6,"filterid=%d",filterid);
    if(filterid <= 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    if(filterid <= gz->loaded_plugins_max) 
        plug = gz->loaded_plugins[filterid];
    if(pp) *pp = plug;
done:
    return ZUNTRACEX(stat,"plugin=%p",*pp);
}

int
NCZ_plugin_loaded_byname(const char* name, NCZ_Plugin** pp)
{
    int stat = NC_NOERR;
    size_t i;
    struct NCZ_Plugin* plug = NULL;
    NCglobalstate* gs = NC_getglobalstate();
    GlobalNCZarr* gz = (GlobalNCZarr*)gs->formatxstate.state[NC_FORMATX_NCZARR];

    ZTRACE(6,"pluginname=%s",name);
    if(name == NULL) {stat = NC_EINVAL; goto done;}
    for(i=1;i<=gz->loaded_plugins_max;i++) {
        if (!gz->loaded_plugins[i]) continue;
        if(!gz->loaded_plugins[i] || !gz->loaded_plugins[i]->codec.codec) continue; /* no plugin or no codec */
        if(strcmp(name, gz->loaded_plugins[i]->codec.codec->codecid) == 0)
	    {plug = gz->loaded_plugins[i]; break;}
    }
    if(pp) *pp = plug;
done:
    return ZUNTRACEX(stat,"plugin=%p",*pp);
}

static int
NCZ_plugin_save(GlobalNCZarr* gz, size_t filterid, NCZ_Plugin* p)
{
    int stat = NC_NOERR;

    ZTRACE(6,"filterid=%d p=%p",filterid,p);
    if(filterid <= 0 || filterid >= H5Z_FILTER_MAX)
	{stat = NC_EINVAL; goto done;}
    if(filterid > gz->loaded_plugins_max) gz->loaded_plugins_max = filterid;
    gz->loaded_plugins[filterid] = p;
done:
    return ZUNTRACE(stat);
}

static int
loadcodecdefaults(GlobalNCZarr* gz, const char* path, const NCZ_codec_t** cp, NCPSharedLib* lib, int* lib_usedp)
{
    int stat = NC_NOERR;
    int lib_used = 0;

    nclistpush(gz->default_libs,lib);
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
	nclistpush(gz->codec_defaults,c0); c0 = NULL;
    }
done:
    if(lib_usedp) *lib_usedp = lib_used;
    return THROW(stat);
}

#if defined NAMEOPT || defined _WIN32
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

/**************************************************/


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
#endif /* defined(DEBUGF) || defined(DEBUGL) */

/* Suppress selected unused static functions */
static void static_unused(void)
{
    void* p = NULL;
    p = p;
    p = static_unused;
#if defined(DEBUGF) || defined(DEBUGL)
(void)printplugin(NULL);
(void)printparams(0, NULL);
(void)printnczparams(const NCZ_Params p);
(void)printcodec(const NCZ_Codec c);
(void)printhdf5(const NCZ_HDF5 h);
#endif
}

