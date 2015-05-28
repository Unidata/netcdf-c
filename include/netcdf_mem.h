/*
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

/**
Open a netcdf file taking the content from a chunk of memory.
It determines the underlying file format automatically. Use the same call
to open a netCDF classic, 64-bit offset, or netCDF-4 file.

\param path File name; this must be non-null, but is ignored except
as noted below.
 
\param mode the mode flags.

\param size The size of the chunk of memory.

\param memory The chunk of memory containing the content.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

<h2>Open Mode</h2>

Note that this procedure uses a limited set of flags.  This
is because it assumes the equivalent of
NC_NOWRITE|NC_DISKLESS|NC_INMEMORY.

It is not necessary to pass any information about the format of the
file being opened. The file type will be detected automatically by the
netCDF library.
 
nc_open_mem()returns the value NC_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

\returns ::NC_NOERR No error.

\returns ::NC_ENOMEM Out of memory.

\returns ::NC_EDISKLESS Error in establishing the in-memory content.

\returns ::NC_EHDFERR HDF5 error. (NetCDF-4 files only.)

\returns ::NC_EINVAL Invalid arguments (e.g. a null memory pointer).
*/

extern int
nc_open_mem(const char* path, int mode, size_t size, void* memory, int* ncidp);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_MEM_H */
