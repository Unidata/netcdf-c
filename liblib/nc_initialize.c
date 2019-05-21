/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"

#ifdef USE_PARALLEL
#include <mpi.h>
#endif

#include "ncdispatch.h"

extern int NC3_initialize(void);
extern int NC3_finalize(void);

#ifdef USE_NETCDF4
#include "nc4internal.h"
#include "hdf5internal.h"
extern int NC4_initialize(void);
extern int NC4_finalize(void);
#endif

#ifdef USE_HDF5
extern int NC_HDF5_initialize(void);
extern int NC_HDF5_finalize(void);
#endif

#ifdef ENABLE_DAP2
extern int NCD2_initialize(void);
extern int NCD2_finalize(void);
#endif

#ifdef ENABLE_DAP4
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

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

int NC_initialized = 0;
int NC_finalized = 0;

/**
 * This function initializes the netcdf-c library.
 * Calling this is not usually necessary because
 * it will be automatically invoked when other functions
 * are called and if the library has not been initialized.
 * 
 * This funcion invokes all defined
 * initializers, and there is an initializer
 * for every known dispatch table.
 * It also initializes appropriate external libraries.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */

int
nc_initialize()
{
    int stat = NC_NOERR;

    if(NC_initialized) return NC_NOERR;
    /* Allow repeated calls if nc_finalize has been called in-between */
    NC_initialized = 1;
    NC_finalized = 0;

    /* Do general initialization */
    if((stat = NCDISPATCH_initialize())) goto done;

    /* Initialize each active protocol */
    if((stat = NC3_initialize())) goto done;
#ifdef ENABLE_DAP
    if((stat = NCD2_initialize())) goto done;
#endif
#ifdef ENABLE_DAP4
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

done:
    return stat;
}

/**
 * This function finalizes the netcdf-c library.
 * Calling this function is not strictly necessary
 * If, however, one is debugging memory use using, for example,
 * valgrind, then it will report memory that has not been
 * deallocated. Calling nc_finalize should reclaim all such memory.
 * 
 * Once this function is called, it will be necessary to call
 * nc_initialize again.
 * 
 * This funcion invokes all defined
 * finalizers, and there is a finalizer
 * for every known dispatch table.
 * It also finalizes appropriate external libraries.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */

/*This procedure invokes all defined
finalizers, and there should be one
for every known dispatch table.
So if you modify the format of NC_Dispatch,
then you need to fix it everywhere.
It also finalizes appropriate external libraries.
If there are open files when this is called, then
all of those open files are aborted.
*/

int
nc_finalize(void)
{
    int stat = NC_NOERR;
    int index;
    NC* ncp;

    if(NC_finalized) return NC_NOERR;
    NC_initialized = 0;
    NC_finalized = 1;

    /* Abort any open files */
    index = 0;
    ncp = NULL;
    for(;;) {
	ncp = iterate_NCList(&index);
	if(ncp == NULL) break;
        (void)nc_abort(ncp->ext_ncid); /* don't care if fails */
    }        

    /* Finalize each active protocol */

#ifdef ENABLE_DAP2
    if((stat = NCD2_finalize())) return stat;
#endif
#ifdef ENABLE_DAP4
    if((stat = NCD4_finalize())) return stat;
#endif

#ifdef USE_PNETCDF
    if((stat = NCP_finalize())) return stat;
#endif

#ifdef USE_HDF4
    if((stat = NC_HDF4_finalize())) return stat;
#endif /* USE_HDF4 */

#ifdef USE_NETCDF4
    if((stat = NC4_finalize())) return stat;
#endif /* USE_NETCDF4 */

#ifdef USE_HDF5
    if((stat = NC_HDF5_finalize())) return stat;
#endif

    if((stat = NC3_finalize())) return stat;

    /* Do general finalization */
    if((stat = NCDISPATCH_finalize())) return stat;

    return NC_NOERR;
}
