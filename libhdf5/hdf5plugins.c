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
#include "netcdf.h"
#include "ncbytes.h"
#include "hdf5internal.h"
#include "hdf5debug.h"
#include "ncplugins.h"

#undef TPLUGINS

/**************************************************/
/* Forward */

static int NC4_hdf5_plugin_path_initialize(void** statep, const NClist* initialpaths);
static int NC4_hdf5_plugin_path_finalize(void** statep);
static int NC4_hdf5_plugin_path_read(void* state, size_t* ndirsp, char** dirs);
static int NC4_hdf5_plugin_path_write(void* state, size_t ndirs, char** const dirs);

/**************************************************/
/**
 * @file
 * @internal
 * Internal netcdf hdf5 plugin path functions.
 *
 * @author Dennis Heimbigner
 */
/**************************************************/
/* The HDF5 Plugin Path Dispatch table and functions */

NC_PluginPathDispatch NC4_hdf5_pluginpathdispatch = {
    NC_FORMATX_NC_HDF5,
    NC_PLUGINPATH_DISPATCH_VERSION,
    NC4_hdf5_plugin_path_initialize,
    NC4_hdf5_plugin_path_finalize,
    NC4_hdf5_plugin_path_read,
    NC4_hdf5_plugin_path_write
};

NC_PluginPathDispatch* NC4_hdf5_pluginpathtable = &NC4_hdf5_pluginpathdispatch;

/**************************************************/

/**
 * This function is called as part of nc_initialize.
 * Its purpose is to initialize the plugin paths state.
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/
static int
NC4_hdf5_plugin_path_initialize(void** statep, const NClist* initialpaths)
{
    int stat = NC_NOERR;
    GlobalHDF5* g5 = NULL;

    NC_UNUSED(initialpaths); /* Let HDF5 do its own thing */
    
    assert(statep != NULL);
    if(*statep != NULL) goto done; /* already initialized */

    if((g5 = (GlobalHDF5*)calloc(1,sizeof(GlobalHDF5)))==NULL) {stat = NC_ENOMEM; goto done;}
    *statep = (void*)g5; g5 = NULL;
done:
    nullfree(g5);
    return THROW(stat);
}

/**
 * This function is called as part of nc_finalize()
 * Its purpose is to clean-up plugin path state.
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/
static int
NC4_hdf5_plugin_path_finalize(void** statep)
{
    int stat = NC_NOERR;
    GlobalHDF5* g5 = NULL;

    assert(statep != NULL);
    if(*statep == NULL) goto done; /* already finalized */
    g5 = (GlobalHDF5*)(*statep);
    *statep = NULL;

done:
    nullfree(g5);
    return THROW(stat);
}

/**
 * Return the current sequence of directories in the internal plugin path list.
 * Since this function does not modify the plugin path, it can be called at any time.
 *
 * @param stat the per-dispatcher global state
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

static int
NC4_hdf5_plugin_path_read(void* state, size_t* ndirsp, char** dirs)
{
    int stat = NC_NOERR;
    herr_t hstat = 0;
    unsigned ndirs = 0;  
    char* dir = NULL;

    if((hstat = H5PLsize(&ndirs))<0) goto done;
    if(ndirsp) *ndirsp = ndirs;
    if(ndirs > 0 && dirs != NULL) {
	unsigned i;
	ssize_t dirlen = 0;
	for(i=0;i<ndirs;i++) {
	    /* Get length of the ith element */
	    if((dirlen=H5PLget(i,NULL,0))<0) {hstat = dirlen; goto done;}
	    /* Alloc space for the dir name */
	    if((dir = (char*)malloc((size_t)dirlen+1))==NULL) {stat = NC_ENOMEM; goto done;}
	    if((hstat=H5PLget(i,dir,(size_t)dirlen+1))<0) goto done;
	    dir[dirlen] = '\0';
	    dirs[i] = dir; dir = NULL;
	}
    }
done:
    nullfree(dir);
    if(stat == NC_NOERR && hstat < 0) stat = NC_EHDFERR;
    return THROW(stat);
}

/**
 * Empty the current internal path sequence
 * and replace with the sequence of directories
 * specified in the arguments.
 * If ndirs == 0 the path list will be cleared
 *
 * @param state the per-dispatcher global state
 * @param ndirs number of entries in dirs arg
 * @param dirs the actual directory paths
 *
 * @return ::NC_NOERR
 * @return ::NC_EINVAL if ndirs > 0 and dirs == NULL
 *
 * @author Dennis Heimbigner
*/

static int
NC4_hdf5_plugin_path_write(void* state, size_t ndirs,  char** dirs)
{
    int stat = NC_NOERR;
    herr_t hstat = 0;
    unsigned hndirs = 0;  

    /* Clear the current path list */
    if((hstat = H5PLsize(&hndirs))<0) goto done;
    if(hndirs > 0) {
	unsigned i;
	for(i=0;i<hndirs;i++) {
	    /* remove i'th element */
	    if((hstat=H5PLremove(i))<0) goto done;
	}
    }
    /* Insert the new path list */
    if(ndirs > 0 && dirs != NULL) {
	size_t i;
	for(i=0;i<ndirs;i++) {
	    if((hstat = H5PLappend(dirs[i]))<0) goto done;	    
	}
    }

done:
    if(stat == NC_NOERR && hstat < 0) stat = NC_EHDFERR;
    return THROW(stat);
}
