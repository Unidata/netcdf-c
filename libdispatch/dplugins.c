/**************************************************/
/* Global state plugin path implementation */

/*
 * Copyright 2018, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */
/**
 * @file
 * Functions for working with plugins. 
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include "netcdf.h"
#include "netcdf_filter.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nclog.h"
#include "ncbytes.h"
#include "ncplugins.h"
#include "netcdf_aux.h"

/*
Unified plugin related code
*/
/**************************************************/
/* Plugin-path API */ 

/* list of environment variables to check for plugin roots */
#define PLUGIN_ENV "HDF5_PLUGIN_PATH"
#define PLUGIN_DIR_UNIX "/usr/local/hdf5/plugin"
#define PLUGIN_DIR_WIN "%s/hdf5/lib/plugin"
#define WIN32_ROOT_ENV "ALLUSERSPROFILE"

/* Control path verification */
#define PLUGINPATHVERIFY "NC_PLUGIN_PATH_VERIFY"

/*Forward*/
static int builddefault(NCPluginList* dirs);

static int NC_plugin_path_initialized = 0;
static int NC_plugin_path_verify = 1;

/**
 * This function is called as part of nc_initialize.
 * Its purpose is to initialize the plugin paths state.
 *
 * @return NC_NOERR
 *
 * @author Dennis Heimbigner
*/

EXTERNL int
nc_plugin_path_initialize(void)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NULL;
    NCPluginList dirs = {0,NULL};
    int hdf5found = 0; /* 1 => we got a legit plugin path set from HDF5 */

    if(!NC_initialized) nc_initialize();
    if(NC_plugin_path_initialized != 0) goto done;
    NC_plugin_path_initialized = 1;

    if(getenv(PLUGINPATHVERIFY) != NULL) NC_plugin_path_verify = 1;

    gs = NC_getglobalstate();

   /**
    * When the netcdf-c library initializes itself, it chooses an initial
    * global plugin path using the following rules, which are those used
    * by the HDF5 library:
    * 1. If HDF5_PLUGIN_PATH environment variable is defined,
    * then its value is used as the initial plugin path.
    * 2. If HDF5_PLUGIN_PATH is not defined, then the initial plugin path is either:
    *     /usr/local/hdf5/plugin -- for Linux/Unix/Cygwin,
    *     %ALLUSERSPROFILE%/hdf5/lib/plugin -- for Windows/Mingw.
    * This initial global plugin path will be propagated to HDF5 and NCZarr.
    */

    /* Initialize the implementations */
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    if((stat = NCZ_plugin_path_initialize())) goto done;    
#endif
#ifdef USE_HDF5
    if((stat = NC4_hdf5_plugin_path_initialize())) goto done;
#endif

#ifdef USE_HDF5
    /* Case (a,b): Get the initial HDF5 plugin path set */
    if((stat = NC4_hdf5_plugin_path_get(&dirs))) goto done;
    if(dirs.ndirs > 0) hdf5found = 1;
#endif /*USE_HDF5*/

    if(!hdf5found) { /* Case: (c) HDF5 not enabled or otherwise empty */
        if((stat = builddefault(&dirs))) goto done; /* Construct a default */
    }

    /* Sync to the actual implementations */
#ifdef USE_HDF5
    if(!hdf5found)
	{if((stat = NC4_hdf5_plugin_path_set(&dirs))) goto done;}
#endif
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    if((stat = NCZ_plugin_path_set(&dirs))) goto done;
#endif
    /* Set the global plugin dirs sequence */
    assert(gs->pluginpaths == NULL);
    gs->pluginpaths = nclistnew();
    nclistsetlength(gs->pluginpaths,dirs.ndirs);
    memcpy(((char**)gs->pluginpaths->content),dirs.dirs,dirs.ndirs*sizeof(char*));
done:
    return NCTHROW(stat);
}

/**
 * This function is called as part of nc_finalize()
 * Its purpose is to clean-up plugin path state.
 *
 * @return NC_NOERR
 *
 * @author Dennis Heimbigner
*/

int
nc_plugin_path_finalize(void)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(NC_plugin_path_initialized == 0) goto done;
    NC_plugin_path_initialized = 0;

    NC_plugin_path_verify = 0;

    /* Finalize the actual implementatios */
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    if((stat = NCZ_plugin_path_finalize())) goto done;    
#endif
#ifdef USE_HDF5
    if((stat = NC4_hdf5_plugin_path_finalize())) goto done;
#endif

    nclistfreeall(gs->pluginpaths); gs->pluginpaths = NULL;
done:
    return NCTHROW(stat);
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
nc_plugin_path_ndirs(size_t* ndirsp)
{
    int stat = NC_NOERR;
    size_t ndirs = 0;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(gs->pluginpaths == NULL) gs->pluginpaths = nclistnew(); /* suspenders and belt */
    ndirs = nclistlength(gs->pluginpaths);

    /* Verify that the implementation plugin paths are consistent in length*/
    if(NC_plugin_path_verify) {
#ifdef NETCDF_ENABLE_HDF5
	{
	    size_t ndirs5 = 0;
	    if((stat=NC4_hdf5_plugin_path_ndirs(&ndirs5))) goto done;
	    assert(ndirs5 == ndirs);
	}
#endif /*NETCDF_ENABLE_HDF5*/
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
	{
	    size_t ndirsz = 0;
	    if((stat=NCZ_plugin_path_ndirs(&ndirsz))) goto done;
	    assert(ndirsz == ndirs);
        }
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
    }
    if(ndirsp) *ndirsp = ndirs;
done:
    return NCTHROW(stat);
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
nc_plugin_path_get(NCPluginList* dirs)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();
    size_t i;

    if(gs->pluginpaths == NULL) gs->pluginpaths = nclistnew(); /* suspenders and belt */
    if(dirs == NULL) goto done;
    dirs->ndirs = nclistlength(gs->pluginpaths);
    if(dirs->dirs == NULL) {
	if((dirs->dirs = (char**)calloc(dirs->ndirs,sizeof(char*)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
    }
    for(i=0;i<dirs->ndirs;i++) {
	const char* dir = nclistget(gs->pluginpaths,i);
	dirs->dirs[i] = nulldup(dir);
    }

    /* Verify that the implementation plugin paths are consistent */
    if(NC_plugin_path_verify) {
#ifdef NETCDF_ENABLE_HDF5
	{
	    size_t i;
	    NCPluginList l5 = {0,NULL};
	    if((stat=NC4_hdf5_plugin_path_get(&l5))) goto done;
	    assert(l5.ndirs == nclistlength(gs->pluginpaths));
	    for(i=0;i<l5.ndirs;i++) {
		assert(strcmp(dirs->dirs[i],l5.dirs[i])==0);
		nullfree(l5.dirs[i]);
	    }
	}
#endif /*NETCDF_ENABLE_HDF5*/
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
	{
	    size_t i;
	    NCPluginList lz = {0,NULL};
	    if((stat=NCZ_plugin_path_get(&lz))) goto done;
	    assert(lz.ndirs == nclistlength(gs->pluginpaths));
	    for(i=0;i<lz.ndirs;i++) {
		assert(strcmp(dirs->dirs[i],lz.dirs[i])==0);
		nullfree(lz.dirs[i]);
	    }
        }
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
    }
done:
    return NCTHROW(stat);
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
nc_plugin_path_set(NCPluginList* dirs)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(dirs == NULL) {stat = NC_EINVAL; goto done;}

    /* Clear the current dir list */
    nclistfreeall(gs->pluginpaths);
    gs->pluginpaths = nclistnew();

    if(dirs->ndirs > 0) {
	size_t i;
        assert(gs->pluginpaths != NULL);
	for(i=0;i<dirs->ndirs;i++) {
	    nclistpush(gs->pluginpaths,nulldup(dirs->dirs[i]));
	}
    }

    /* Sync the global plugin path set to the individual implementations */
#ifdef NETCDF_ENABLE_HDF5
    if((stat = NC4_hdf5_plugin_path_set(dirs))) goto done;
#endif
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    if((stat = NCZ_plugin_path_set(dirs))) goto done;
#endif

done:
    return NCTHROW(stat);
}

/* Setup the plugin path default */
static int
builddefault(NCPluginList* dirs)
{
    int stat = NC_NOERR;
    char* hdf5defaultpluginpath = NULL;
    const char* pluginroots = NULL;
    size_t i;
#ifdef _WIN32
    const char* win32_root;
    char dfalt[4096];
#endif

    /* Find the plugin directory root(s) */
    pluginroots = getenv(PLUGIN_ENV); /* Usually HDF5_PLUGIN_PATH */
    if(pluginroots != NULL) {
	if((stat = ncaux_plugin_path_parse(pluginroots,'\0',dirs))) goto done;
    } else {
#ifdef _WIN32
	win32_root = getenv(WIN32_ROOT_ENV);
	if(win32_root != NULL && strlen(win32_root) > 0) {
	    snprintf(dfalt,sizeof(dfalt),PLUGIN_DIR_WIN,win32_root);
	    hdf5defaultpluginpath = strdup(dfalt);
	}
#else /*!_WIN32*/
        hdf5defaultpluginpath = strdup(PLUGIN_DIR_UNIX);
#endif
        /* Use the default as the only value in the default list */
        if(hdf5defaultpluginpath != NULL) {
	    ncaux_plugin_path_clear(dirs); /* make sure it is empty */
            if((stat=ncaux_plugin_path_append(dirs,hdf5defaultpluginpath)))
		goto done;
	}
    }
done:
    nullfree(hdf5defaultpluginpath);
    return stat;
}
