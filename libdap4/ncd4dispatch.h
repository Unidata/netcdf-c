/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/** @file ncd4dispatch.h
 * @brief NC_Dispatch entry points for the DAP4 client.
 *
 * Declares the functions that are registered in the DAP4 dispatch table
 * and called by the netCDF library when a DAP4 URL is opened.
 * @author Dennis Heimbigner
 */

#ifndef _NCD4DISPATCH_H
#define _NCD4DISPATCH_H

#include <stddef.h> /* size_t, ptrdiff_t */
#include "netcdf.h"
#include "ncdispatch.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Open a DAP4 URL as a netCDF file.
 * @param path           DAP4 URL string.
 * @param mode           Open mode flags (NC_NOWRITE, etc.).
 * @param basepe         MPI base PE (unused for DAP4).
 * @param chunksizehintp Chunk-size hint (unused for DAP4).
 * @param mpidata        MPI communicator/info (unused for DAP4).
 * @param dispatch       Active NC_Dispatch table.
 * @param ncid           Pre-allocated external ncid.
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int
NCD4_open(const char *path, int mode,
          int basepe, size_t *chunksizehintp,
          void *mpidata, const struct NC_Dispatch *dispatch, int ncid);

/**
 * Close a DAP4 file and release all associated resources.
 * @param ncid External ncid of the file to close.
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int NCD4_close(int ncid, void* unused);

/**
 * Abort (close without saving) a DAP4 file.
 * @param ncid External ncid of the file to abort.
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int NCD4_abort(int ncid);

/**
 * Inquire the name and length of a dimension.
 * @param ncid  Group ncid.
 * @param dimid Dimension id.
 * @param name  Receives the dimension name (may be NULL).
 * @param lenp  Receives the dimension length (may be NULL).
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int NCD4_inq_dim(int ncid, int dimid, char* name, size_t* lenp);

/**
 * Read a contiguous hyperslab of a variable.
 * @param ncid    Group ncid.
 * @param varid   Variable id.
 * @param start   Start indices (one per dimension).
 * @param edges   Edge lengths (one per dimension).
 * @param value   Destination buffer.
 * @param memtype In-memory type for the values.
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int
NCD4_get_vara(int ncid, int varid,
              const size_t *start, const size_t *edges,
              void *value, nc_type memtype);

/**
 * Read a strided hyperslab of a variable.
 * @param ncid    Group ncid.
 * @param varid   Variable id.
 * @param start   Start indices (one per dimension).
 * @param edges   Edge lengths (one per dimension).
 * @param strides Strides (one per dimension).
 * @param value   Destination buffer.
 * @param memtype In-memory type for the values.
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int
NCD4_get_vars(int ncid, int varid,
              const size_t *start, const size_t *edges, const ptrdiff_t* strides,
              void *value, nc_type memtype);

/** Initialize the DAP4 dispatch layer; called once at library startup. */
extern int NCD4_initialize(void);

#if defined(__cplusplus)
}
#endif

#endif /*_NCD4DISPATCH_H*/
