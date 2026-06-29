/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
 * @file
 * Library initialization and finalization.
 *
 * These functions handle the initialization and finalization of the
 * netCDF library, including all dispatch tables and external
 * libraries.
 */

#include "config.h"

#ifdef USE_PARALLEL
#include <mpi.h>
#endif

#include "ncdispatch.h"

#ifdef USE_NETCDF4
#include "nc4internal.h"
#endif

#ifdef USE_HDF5
#include "hdf5internal.h"
extern int NC_HDF5_initialize(void);
extern int NC_HDF5_finalize(void);
#endif

/*
 * NETCDF_ENABLE_DAP is the top-level option that enables DAP support.
 * Setting it implies NETCDF_ENABLE_DAP2; NETCDF_ENABLE_DAP4 is also
 * set when HDF5 support is available. The extern declarations here
 * use the protocol-specific macros, while nc_initialize() below
 * guards the NCD2_initialize() call with the parent NETCDF_ENABLE_DAP
 * macro since DAP2 is always enabled when DAP is enabled.
 */
#ifdef NETCDF_ENABLE_DAP2
extern int NCD2_initialize(void);
extern int NCD2_finalize(void);
#endif

#ifdef NETCDF_ENABLE_DAP4
extern int NCD4_initialize(void);
extern int NCD4_finalize(void);
#endif


#ifdef USE_PNETCDF
extern int NCP_initialize(void);
extern int NCP_finalize(void);
#endif

#ifdef USE_HDF4
extern int NC_HDF4_initialize(void);
extern int NC_HDF4_finalize(void);
#endif

#ifdef NETCDF_ENABLE_S3
EXTERNL int NC_s3sdkinitialize(void);
EXTERNL int NC_s3sdkfinalize(void);
#endif

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

int NC_initialized = 0;
int NC_finalized = 1;

#ifdef NETCDF_ENABLE_ATEXIT_FINALIZE
/* Provide the void function to give to atexit() */
static void
finalize_atexit(void)
{
    (void)nc_finalize();
}
#endif

/**
 * Initialize the netCDF library.
 *
 * This function sets up all internal dispatch tables and initializes
 * any external libraries required by the enabled protocols (e.g.,
 * HDF5, DAP2, DAP4, PnetCDF, NCZarr). It is called automatically on
 * first use of the library, so most users do not need to call it
 * directly.
 *
 * It is safe to call this function more than once; subsequent calls
 * are no-ops.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EXXX An error occurred during initialization.
 * @see nc_finalize
 */

int
nc_initialize()
{
    int stat = NC_NOERR;

    if(NC_initialized) return NC_NOERR;
    NC_initialized = 1;
    NC_finalized = 0;

    /* Do general initialization */
    if((stat = NCDISPATCH_initialize())) goto done;

    /* Initialize each active protocol */
    if((stat = NC3_initialize())) goto done;
#ifdef NETCDF_ENABLE_DAP
    if((stat = NCD2_initialize())) goto done;
#endif
#ifdef NETCDF_ENABLE_DAP4
    if((stat = NCD4_initialize())) goto done;
#endif
#ifdef USE_PNETCDF
    if((stat = NCP_initialize())) goto done;
#endif
#ifdef USE_NETCDF4
    if((stat = NC4_initialize())) goto done;
#endif /* USE_NETCDF4 */
#ifdef USE_HDF5
    if((stat = NC_HDF5_initialize())) goto done;
#endif
#ifdef USE_HDF4
    if((stat = NC_HDF4_initialize())) goto done;
#endif
#ifdef NETCDF_ENABLE_S3
    if((stat = NC_s3sdkinitialize())) goto done;
#endif
#ifdef NETCDF_ENABLE_NCZARR
    if((stat = NCZ_initialize())) goto done;
#endif

#ifdef NETCDF_ENABLE_ATEXIT_FINALIZE
    /* Use atexit() to invoke nc_finalize */
    if(atexit(finalize_atexit))
	fprintf(stderr,"atexit failed\n");
#endif

done:
    return stat;
}

/**
 * Finalize the netCDF library.
 *
 * This function releases all internal resources and finalizes any
 * external libraries initialized by nc_initialize(). After calling
 * this function, the library may be re-initialized by calling
 * nc_initialize() again.
 *
 * If ::NETCDF_ENABLE_ATEXIT_FINALIZE is set, this function is
 * registered with atexit() and called automatically at program exit.
 *
 * It is safe to call this function more than once; subsequent calls
 * are no-ops.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EXXX An error occurred during finalization.
 * @see nc_initialize
 */

int
nc_finalize(void)
{
    int stat = NC_NOERR;
    int failed = stat;

    if(NC_finalized) goto done;
    NC_initialized = 0;
    NC_finalized = 1;

    /* Finalize each active protocol */

#ifdef NETCDF_ENABLE_DAP2
    if((stat = NCD2_finalize())) failed = stat;
#endif
#ifdef NETCDF_ENABLE_DAP4
    if((stat = NCD4_finalize())) failed = stat;
#endif

#ifdef USE_PNETCDF
    if((stat = NCP_finalize())) failed = stat;
#endif

#ifdef USE_HDF4
    if((stat = NC_HDF4_finalize())) failed = stat;
#endif /* USE_HDF4 */

#ifdef USE_NETCDF4
    if((stat = NC4_finalize())) failed = stat;
#endif /* USE_NETCDF4 */

#ifdef USE_HDF5
    if((stat = NC_HDF5_finalize())) failed = stat;
#endif

#ifdef NETCDF_ENABLE_NCZARR
    if((stat = NCZ_finalize())) failed = stat;
#endif

#ifdef NETCDF_ENABLE_S3
    if((stat = NC_s3sdkfinalize())) failed = stat;
#endif

    if((stat = NC3_finalize())) failed = stat;

    /* Do general finalization */
    if((stat = NCDISPATCH_finalize())) failed = stat;

done:
    if(failed) fprintf(stderr,"nc_finalize failed: %d\n",failed);
    return failed;
}
