/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"

#ifdef USE_PARALLEL
#include <mpi.h>
#endif

#include "ncdispatch.h"
#include "ncglobal.h"

extern int NC3_initialize(void);
extern int NC3_finalize(void);

#ifdef USE_NETCDF4
#include "nc4internal.h"
extern int NC4_initialize(void);
extern int NC4_finalize(void);
#endif

#ifdef USE_DAP
extern int NCD2_initialize(void);
extern int NCD2_finalize(void);
#endif

#ifdef USE_PNETCDF
extern int NCP_initialize(void);
extern int NCP_finalize(void);
#endif

/**
This procedure invokes all defined
initializers, and there is an initializer
for every known dispatch table.
So if you modify the format of NC_Dispatch,
then you need to fix it everywhere.
It also initializes appropriate external libraries.
*/

int
nc_initialize()
{
    int stat = NC_NOERR;

    LOCK;
    if(nc_global->initialized) {UNLOCK; goto done;}

    nc_global_init();

    nc_global->initialized = 1;
    nc_global->finalized = 0;
    UNLOCK;

    /* Do general initialization */
    if((stat = NCDISPATCH_initialize())) goto done;

    /* Initialize each active protocol */
    if((stat = NC3_initialize())) goto done;
#ifdef USE_DAP
    if((stat = NCD2_initialize())) goto done;
#endif
#ifdef USE_PNETCDF
    if((stat = NCP_initialize())) goto done;
#endif
#ifdef USE_NETCDF4
    if((stat = NC4_initialize())) goto done;
    stat = NC4_fileinfo_init();
#endif /* USE_NETCDF4 */

done:
    return stat;
}

/**
This procedure invokes all defined
finalizers, and there should be one
for every known dispatch table.
So if you modify the format of NC_Dispatch,
then you need to fix it everywhere.
It also finalizes appropriate external libraries.
*/

int
nc_finalize(void)
{
    int stat = NC_NOERR;

    LOCK;
    if(nc_global->finalized) {UNLOCK; goto done;}
    nc_global->initialized = 0;
    nc_global->finalized = 1;
    UNLOCK;

    /* Finalize each active protocol */

#ifdef USE_DAP
    if((stat = NCD2_finalize())) return stat;
#endif

#ifdef USE_PNETCDF
    if((stat = NCP_finalize())) return stat;
#endif

#ifdef USE_NETCDF4
    if((stat = NC4_finalize())) return stat;
#endif /* USE_NETCDF4 */

    if((stat = NC3_finalize())) return stat;

    /* Do general finalization */
    if((stat = NCDISPATCH_finalize())) return stat;

done:
    return NC_NOERR;
}
