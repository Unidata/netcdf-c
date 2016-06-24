/** \file
Global data

This file stores all known global data for the netcdf-c library.
It provides two structure types and instances.
1. Mutable data
2. Constant data 

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "nc.h"
#include "ncglobal.h"

NC_GLOBAL_VARS* nc_global;
NC_GLOBAL_CONSTANTS* nc_constants;

int
nc_global_init()
{
    int status = NC_NOERR;
    int i;

    nc_global = (NC_GLOBAL_VARS*)calloc(1,sizeof(NC_GLOBAL_VARS));
    nc_constants = (NC_GLOBAL_CONSTANTS*)calloc(1,sizeof(NC_GLOBAL_CONSTANTS));

    SPINLOCKINIT;

    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
	nc_constants->sizevector0[i] = 0;
        nc_constants->sizevector1[i] = 1;
        nc_constants->ptrdiffvector1[i] = 1;
    }
    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
	nc_constants->coord_one[i] = 1;
	nc_constants->coord_zero[i] = 0;
    }

    nc_constants->argc = 1;
    nc_constants->argv = (char**)malloc(sizeof(char*)*2);
    nc_constants->argv[0] = strdup("nc_initialize");
    nc_constants->argv[1] = NULL;

    nc_global->initialized = 0;
    nc_global->finalized = 1;

    /* The subroutines in error.c emit no messages unless NC_VERBOSE bit
     * is on.  They call exit() when NC_FATAL bit is on. */
    nc_global->ncopts = (NC_FATAL | NC_VERBOSE) ;
    nc_global->ncerr = NC_NOERR ;

    nc_global->default_create_format = NC_FORMAT_CLASSIC;

    ncaux_init();

    ncloginit();

    ncd2aligninit();

    nc4_hdf5_initialize();

    return status;
}

/*! \} */ /* End of defgroup. */
