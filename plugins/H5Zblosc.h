/*
 * Dynamically loaded filter plugin for HDF5 blosc filter.
 *
 * Author: Kiyoshi Masui <kiyo@physics.ubc.ca>
 * Created: 2014
 *
 *
 * Header file
 * -----------
 *
 * This provides dynamically loaded HDF5 filter functionality (introduced
 * in HDF5-1.8.11, May 2013) to the blosc HDF5 filter.
 *
 * Usage: compile as a shared library and install either to the default
 * search location for HDF5 filter plugins (on Linux 
 * /usr/local/hdf5/lib/plugin) or to a location pointed to by the
 * HDF5_PLUGIN_PATH environment variable.
 *
 */


#ifndef H5ZBLOSC_H
#define H5ZBLOSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "netcdf_filter_build.h"
#include "blosc.h"

#ifdef _MSC_VER
  #ifdef DLL_EXPORT /* define when building the library */
    #define DECLSPEC __declspec(dllexport)
  #else
    #define DECLSPEC __declspec(dllimport)
  #endif
#else
  #define DECLSPEC extern
#endif

/* Filter revision number, starting at 1 */
/* #define FILTER_BLOSC_VERSION 1 */
#define FILTER_BLOSC_VERSION 2	/* multiple compressors since Blosc 1.3 */

#define DEFAULT_LEVEL		9
#define DEFAULT_BLOCKSIZE	1
#define DEFAULT_TYPESIZE	1
#define DEFAULT_COMPCODE	BLOSC_LZ4

/* HDF5 Plugin API */
DECLSPEC H5PL_type_t H5PLget_plugin_type(void);
DECLSPEC const void* H5PLget_plugin_info(void);

/* NCZarr API */
DECLSPEC const void* NCZ_get_plugin_info(void);

#ifdef __cplusplus
}
#endif

#endif /*H5ZBLOSC_H*/
