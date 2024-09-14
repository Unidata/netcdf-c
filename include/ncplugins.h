/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See COPYRIGHT for license information.
*/

/*
Common functionality for plugin paths/
For internal use only.
*/

#ifndef NCPLUGINS_H
#define NCPLUGINS_H

/* Opaque */
struct NClist;

/* Define the plugin path management dispatch table */

typedef struct NC_PluginPathDispatch {
    int model; /* one of the NC_FORMATX #'s */
    int dispatch_version;
    int (*initialize)(void** statep, const struct NClist* initialpaths);
    int (*finalize)(void** statep);
    int (*read)(void* state, size_t* ndirsp, char** dirs);
    int (*write)(void* state, size_t ndirs, char** const dirs);
} NC_PluginPathDispatch;

#if defined(__cplusplus)
extern "C" {
#endif

/* Known Plugin Dispatchers */
#ifdef USE_HDF5
EXTERNL NC_PluginPathDispatch* NC4_hdf5_pluginpathtable;
#endif
#ifdef NETCDF_ENABLE_NCZARR
EXTERNL NC_PluginPathDispatch* NCZ_pluginpathtable;
#endif

/* See the file netcdf_aux.h for plugin-related utility functions */

#if defined(__cplusplus)
}
#endif

#endif /*NCPLUGINS_H*/
