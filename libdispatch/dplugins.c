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
#include "netcdf_aux.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nclog.h"
#include "ncbytes.h"
#include "ncplugins.h"

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

static int NC_plugin_path_initialized = 0;

/**
 * This function is called as part of nc_initialize.
 * Its purpose is to initialize the plugin paths state.
 *
 * @return ::NC_NOERR
 *
 * @author Dennis Heimbigner
*/

EXTERNL int
nc_plugin_path_initialize(void)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NULL;
    char* defaultpluginpath = NULL;
    const char* pluginroots = NULL;
    NClist* dirs = NULL;
    size_t ndirs;
    int i;

    if(NC_plugin_path_initialized != 0) goto done;
    NC_plugin_path_initialized = 1;

    gs = NC_getglobalstate();
    dirs = nclistnew();

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
    if((stat = ncaux_plugin_path_parse(pluginroots,0,&ndirs,NULL))) goto done;
    if(ndirs > 0) {
	char** contents;
	nclistsetlength(dirs,ndirs); /* may modify contents memory */
	contents = (char**)nclistcontents(dirs);
	if((stat = ncaux_plugin_path_parse(pluginroots,0,&ndirs,contents))) goto done;
    }
    /* Add the default to end of the dirs list if not already there */
    if(defaultpluginpath != NULL && !nclistmatch(dirs,defaultpluginpath,0)) {
        nclistpush(dirs,defaultpluginpath);
	defaultpluginpath = NULL;
    }

    /* Initialize all the plugin path dispatchers and state*/
#ifdef USE_HDF5
    gs->formatxstate.pluginapi[NC_FORMATX_NC_HDF5] = NC4_hdf5_pluginpathtable;
#endif
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    gs->formatxstate.pluginapi[NC_FORMATX_NCZARR] = NCZ_pluginpathtable;
#endif
    /* Initialize all the plugin path dispatcher states */
    for(i=0;i<NC_FORMATX_COUNT;i++) {    
	if(gs->formatxstate.pluginapi[i] != NULL) {
	    if((stat = gs->formatxstate.pluginapi[i]->initialize(&gs->formatxstate.state[i], dirs))) goto done;
	    assert(gs->formatxstate.state[i] != NULL);
	}
    }

done:
    nullfree(defaultpluginpath);
    nclistfreeall(dirs);
    return NCTHROW(stat);
}

/**
 * This function is called as part of nc_finalize()
 * Its purpose is to clean-up plugin path state.
 *
 * @return ::NC_NOERR
 *
 * @author Dennis Heimbigner
*/

int
nc_plugin_path_finalize(void)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();
    int i;

    if(NC_plugin_path_initialized == 0) goto done;
    NC_plugin_path_initialized = 0;

    /* Finalize all the plugin path dispatchers */
    for(i=0;i<NC_FORMATX_COUNT;i++) {    
	if(gs->formatxstate.state[i] != NULL) {
	    if((stat = gs->formatxstate.pluginapi[i]->finalize(&gs->formatxstate.state[i]))) goto done;
	    gs->formatxstate.state[i] = NULL;
	}
    }
done:
    return NCTHROW(stat);
}

/**
 * Return the current sequence of directories in the internal plugin path list.
 * Since this function does not modify the plugin path, it can be called at any time.
 *
 * @param formatx the dispatcher from which to get the info
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
*/

EXTERNL int
nc_plugin_path_read(int formatx, size_t* ndirsp, char** dirs)
{
    int stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(formatx < 0 || formatx >= NC_FORMATX_COUNT) {stat = NC_EINVAL; goto done;}
    if(!NC_initialized) nc_initialize();
    /* read functions can only apply to specific formatx */
    if(formatx == 0) {stat = NC_EINVAL; goto done;}

    if(gs->formatxstate.pluginapi[formatx] == NULL || gs->formatxstate.state[formatx] == NULL) {stat = NC_EINVAL; goto done;}
    if((stat = gs->formatxstate.pluginapi[formatx]->read(gs->formatxstate.state[formatx],ndirsp,dirs))) goto done;
done:
    return NCTHROW(stat);
}

/**
 * Empty the current internal path sequence
 * and replace with the sequence of directories
 * specified in the arguments.
 * If ndirs == 0 the path list will be cleared
 *
 * @param formatx the dispatcher to which to write; zero means all dispatchers
 * @param ndirs number of entries in dirs arg
 * @param dirs the actual directory paths
 *
 * @return ::NC_NOERR
 * @return ::NC_EINVAL if formatx is unknown or ndirs > 0 and dirs == NULL
 *
 * @author Dennis Heimbigner
 * 
 * Note that modifying the plugin paths must be done "atomically".
 * That is, in a multi-threaded environment, it is important that
 * the sequence of actions involved in setting up the plugin paths
 * must be done by a single processor or in some other way as to
 * guarantee that two or more processors are not simultaneously
 * accessing the plugin path read/write operations.
 * 
 * As an example, assume there exists a mutex lock called PLUGINLOCK.
 * Then any processor accessing the plugin paths should operate
 * as follows:
 * <pre>
 * lock(PLUGINLOCK);
 * nc_plugin_path_read(...);
 * <rebuild plugin path>
 * nc_plugin_path_write(...);
 * unlock(PLUGINLOCK);
 * </pre>
*/

EXTERNL int
nc_plugin_path_write(int formatx, size_t ndirs, char** const dirs)
{
    int i,stat = NC_NOERR;
    struct NCglobalstate* gs = NC_getglobalstate();

    if(formatx < 0 || formatx >= NC_FORMATX_COUNT) {stat = NC_EINVAL; goto done;}
    if(ndirs > 0 && dirs == NULL) {stat = NC_EINVAL; goto done;}
    if(!NC_initialized) nc_initialize();
    /* forall dispatchers */
    for(i=1;i<NC_FORMATX_COUNT;i++) {
	if(i == formatx || formatx == 0) {
	    if(gs->formatxstate.pluginapi[i] == NULL || gs->formatxstate.state[i] == NULL) continue;
	    if((stat=gs->formatxstate.pluginapi[i]->write(gs->formatxstate.state[i],ndirs,dirs))) goto done;
	}
    }
done:
    return NCTHROW(stat);
}
