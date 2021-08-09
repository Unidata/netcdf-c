/* This is part of the netCDF package. Copyright 2020 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test internal netcdf-4 file code for memory leaks. This test was
   suggest by Jeff Whitaker. See
   https://github.com/Unidata/netcdf-c/issues/1575.

   This was removed from the list of tests run in this
   directory. Sometimes it fails on very busy systems even though
   everything is fine. Ed - 8/9/21

   Ed Hartnett 2/9/20
*/

#include <config.h>
#include <nc_tests.h>
#include <err_macros.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#else
#error "cannot use rusage"
#endif

#define FILE_NAME "tst_mem1.nc"
#define NUM_FILE_OPENS 100

int main()
{
    int ncid, varid, idx;
    struct rusage r_usage;

    printf("\n*** Testing netcdf-4 memory use.\n");
    printf("*** testing mem use opening/closing file...");
    {
        long my_rss = 0;
	int my_idx = -1;
	int at_max = 0;

	NC_UNUSED(my_idx); NC_UNUSED(at_max);
	
        if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
        /* if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR; */
        if (nc_def_var(ncid, "dummy", NC_DOUBLE, 0, NULL, &varid)) ERR;
        if (nc_close(ncid)) ERR;

        for (idx = 0; idx < NUM_FILE_OPENS; idx++)
        {
            if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
            if (nc_close(ncid)) ERR;
            getrusage(RUSAGE_SELF, &r_usage);
            /* if (!(idx % 100)) */
            /*     printf("Memory usage: %ld kilobytes\n",r_usage.ru_maxrss); */

            /* Memory usage goes up in the first couple of opens, but
             * should then remain steady. Check that it does not
             * change after some number of iterations; the expected number is 10. */
#if 1
            if (!my_rss || idx < 10)
                my_rss = r_usage.ru_maxrss;
            else
                if (my_rss != r_usage.ru_maxrss) ERR;
#else
	    /* Locate the monotonic maximum */
	    if(my_rss < r_usage.ru_maxrss) {
		my_rss = r_usage.ru_maxrss;
		my_idx = idx;
	    } else if(my_rss > r_usage.ru_maxrss) {
		fprintf(stderr,"decrease: from: [%d] %ld to [%d] %ld\n",my_idx,my_rss,idx,r_usage.ru_maxrss);
		ERR;
	    } else {
		if(!at_max)
		    fprintf(stderr,"maximum: [%d] %ld\n",idx,r_usage.ru_maxrss);
		at_max = 1;
	    }
#endif
        };
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
