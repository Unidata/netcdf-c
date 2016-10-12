/** \file
Global data

This file stores all known global data for the netcdf-c library.
It provides two structure types and instances.
1. Mutable data
2. Constant data 

Both must be locked for initialization, but after that the constant data
can be accessed without locking.  Access to the mutable data must always
occur in a critical section.
*/

/*
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
#include "ncauxinternal.h"
#include "ncalign.h"
#include "nclog.h"
#include "nc4internal.h"
#include "ncglobal.h"

NC_GLOBAL_VARS* nc_global = NULL;
NC_GLOBAL_CONSTANTS* nc_constants = NULL;

int
nc_global_init()
{
    int status = NC_NOERR;
    int i;

    NC_mutex_initialize();

    NCLOCK();

    if(nc_global != NULL) goto done;

    nc_global = (NC_GLOBAL_VARS*)calloc(1,sizeof(NC_GLOBAL_VARS));
    if(nc_global == NULL) {status = NC_ENOMEM; goto done;}
    nc_constants = (NC_GLOBAL_CONSTANTS*)calloc(1,sizeof(NC_GLOBAL_CONSTANTS));
    if(nc_constants == NULL) {status = NC_ENOMEM; goto done;}

    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
	nc_constants->sizevector0[i] = 0;
        nc_constants->sizevector1[i] = 1;
        nc_constants->ptrdiffvector1[i] = 1;
    }
    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
	nc_constants->coord_one[i] = 1;
	nc_constants->coord_zero[i] = 0;
    }

    nc_global->default_create_format = NC_FORMAT_CLASSIC;

    nc_global->files.boundary = 1; /* position 0 is not used */
    nc_global->files.maxfiles = NCFILELISTLENGTH;
    nc_global->files.filelist = (NC**)calloc(1, sizeof(NC*)*NCFILELISTLENGTH);
    if(nc_global->files.filelist == NULL) {status = NC_ENOMEM; goto done;}

    ncaux_init();
    ncloginit();
    ncd2aligninit();
    nc4_global_init();
#ifdef ENABLE_DAP
    oc_initialize();
#endif

#if 0
    /* The subroutines in error.c emit no messages unless NC_VERBOSE bit
     * is on.  They call exit() when NC_FATAL bit is on. */
    nc_global->ncopts = (NC_FATAL | NC_VERBOSE) ;
    nc_global->ncerr = NC_NOERR ;
#endif
#if 0
    nc_constants->argc = 1;
    nc_constants->argv = (char**)malloc(sizeof(char*)*2);
    nc_constants->argv[0] = strdup("nc_initialize");
    nc_constants->argv[1] = NULL;
#endif

done:
    NCUNLOCK();
    return status;
}

/*! \} */ /* End of defgroup. */
