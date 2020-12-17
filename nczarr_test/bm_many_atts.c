/* This is part of the netCDF package. Copyright 2005-2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program benchmarks creating a netCDF file with many objects.

   Ed Hartnett, Dennis Heimbigner
*/

#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */

#include "bm_utils.h"

int
main(int argc, char **argv)
{
    long long delta;
    time_t starttime, endtime;
    struct timeval start_time, end_time, diff_time;
    double sec;
    int ncid;
    int data[] = {42};
    int g, grp;
    char gname[16];
    int a, an;
    char* path = NULL;

    NCCHECK(getoptions(&argc,&argv,&bmoptions));
    NCCHECK(nc4_buildpath(&bmoptions,&path));

    if(bmoptions.debug) {
        reportoptions(&bmoptions);
        reportmetaoptions(&bmoptions.meta);
    }

    starttime = 0;
    endtime = 0;
    time(&starttime);
    
    /*  create new file */
    if (nc_create(path, NC_NETCDF4, &ncid)) ERR;

    /* create N group/global attributes, printing time after every 100. */

    if (gettimeofday(&start_time, NULL))
	ERR;

    for(a=0, g = 1; g < bmoptions.meta.ngroups + 1; g++) {
	sprintf(gname, "group%d", g);
	if (nc_def_grp(ncid, gname, &grp)) ERR;
	for(an = 1; an < bmoptions.meta.ngroupattrs + 1; an++) {
	    char aname[20];
	    sprintf(aname, "attribute%d", a);
	    if (nc_put_att_int(grp, NC_GLOBAL, aname, NC_INT, 1, data)) ERR;
	    if(a%100 == 0) {		/* only print every 100th attribute name */
		if (gettimeofday(&end_time, NULL)) ERR;
		if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
		sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
		printf("%s/%s\t%.3g sec\n", gname, aname, sec);
	    }
	    a++;
	}
    }
    nc_close(ncid);

    time(&endtime);
    /* Compute the delta 1 second resolution is fine for this */
    delta = (long long)(endtime - starttime);
    printf("delta.create.%s = %lld\n",formatname(&bmoptions),delta);

    nullfree(path);
    clearoptions(&bmoptions);
    FINAL_RESULTS;
}
