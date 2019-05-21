/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test multiple cycles of nc_initialize...nc_finalize
*/

#include "config.h"
#include "stdlib.h"
#include "stdio.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "nc_tests.h"
#include "err_macros.h"
#include "netcdf.h"

#undef DEBUG

/* Created file */
#define NCFILE "tst_cycle.nc"

#define ERR2 { \
    err++; \
    fprintf(stderr, "Sorry! Unexpected result, %s, line: %d (%s)\n", \
    __FILE__, __LINE__, nc_strerror(stat));                          \
    exit(1); \
}

int
main(int argc, char **argv)
{
    int stat = NC_NOERR;
    int ncid;
    char ncprop[8192];

    printf("\n*** Testing initialize/finalize cycle.\n");

    /* Invoke nc_initialize...nc_finalize multiple times.*/
    /* In between try to open/close a file */

    /* Ideally do this test with some kind of memory checker to look
    for memory errors
    */

    /* Create a file */
    if((stat = nc_create(NCFILE,NC_CLOBBER|NC_NETCDF4,&ncid))) ERR; /* Indirectly call nc_initialize() */
    /* finalize and ensure that created file gets aborted */
    if((stat = nc_finalize())) ERR;
    /* Check that file does not exist */
    {
	FILE* f = fopen(NCFILE,"r");
	if(f != NULL) {
	    fprintf(stderr,"*** Fail: nc_finalize failed to abort %s\n",NCFILE);
	    ERR;
	    goto done;	    
	}
    }

    /* Create a and keep file */
    if((stat = nc_create(NCFILE,NC_CLOBBER|NC_NETCDF4,&ncid))) ERR; /* Indirectly call nc_initialize() */
    if((stat = nc_close(ncid))) ERR;
    if((stat = nc_finalize())) ERR;

    /* Re-open (including re-initialize) */
    if((stat = nc_open(NCFILE,NC_NOCLOBBER,&ncid))) ERR; /* Indirectly call nc_initialize() */
    /* Read the _NCProperties attribute */
    if((stat = nc_get_att_text(ncid,NC_GLOBAL,"_NCProperties",ncprop))) ERR;
    printf("|%s|\n",ncprop);
    /* Do not close so that finalize will be forced to abort it */    
    if((stat = nc_finalize())) ERR;

    /* Once more */
    if((stat = nc_initialize())) ERR;   
    if((stat = nc_open(NCFILE,NC_NOCLOBBER,&ncid))) ERR; /* Indirectly call nc_initialize() */
    if((stat = nc_close(ncid))) ERR;    
    if((stat = nc_finalize())) ERR;

done:
    (void)unlink(NCFILE);

    SUMMARIZE_ERR;
    
    FINAL_RESULTS;
}
