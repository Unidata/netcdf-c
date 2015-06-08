/* \file netcdf_mem.h
 * Copyright 2010 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * See \ref copyright file for more info.
 *
 */

#ifndef NETCDF_MEM_H
#define NETCDF_MEM_H 1

#if defined(__cplusplus)
extern "C" {
#endif

/* Declaration modifiers for DLL support (MSC et al) */
#if defined(DLL_NETCDF) /* define when library is a DLL */
#  if defined(DLL_EXPORT) /* define when building the library */
#   define MSC_EXTRA __declspec(dllexport)
#  else
#   define MSC_EXTRA __declspec(dllimport)
#  endif
#include <io.h>
#else
#define MSC_EXTRA
#endif	/* defined(DLL_NETCDF) */

# define EXTERNL MSC_EXTRA extern

/**
Open a netCDF file with the contents taken from a block of memory.

\param path Must be non-null, but otherwise only used to set the dataset name.

\param mode the mode flags; Note that this procedure uses a limited set of flags because it forcibly sets NC_NOWRITE|NC_DISKLESS|NC_INMEMORY.

\param size The length of the block of memory being passed.

\param memory Pointer to the block of memory containing the contents
of a netcdf file.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

\returns ::NC_NOERR No error.

\returns ::NC_ENOMEM Out of memory.

\returns ::NC_EDISKLESS diskless io is not enabled for fails.

\returns ::NC_EINVAL, etc. other errors also returned by nc_open.

<h1>Examples</h1>

Here is an example using nc_open_mem() to open an existing netCDF dataset
named foo.nc for read-only, non-shared access. It differs from the nc_open()
example in that it assumes the contents of foo.nc have been read into memory.

@code
#include <netcdf.h>
#include <netcdf_mem.h>
   ...
int status = NC_NOERR;
int ncid;
size_t size;
void* memory;
   ...
size = <compute file size of foo.nc in bytes>;
memory = malloc(size);
   ...
status = nc_open_mem("foo.nc", 0, size, memory, &ncid);
if (status != NC_NOERR) handle_error(status);
@endcode
*/
EXTERNL int
nc_open_mem(const char* path, int mode, size_t size, void* memory, int* ncidp);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_MEM_H */
