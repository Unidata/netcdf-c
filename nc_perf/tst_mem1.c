/* This is part of the netCDF package. Copyright 2020 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test internal netcdf-4 file code for memory leaks. This test was
   suggest by Jeff Whitaker. See
   https://github.com/Unidata/netcdf-c/issues/1575.

   Ed Hartnett 2/9/20
*/

#include <config.h>
#include <nc_tests.h>
#include <err_macros.h>
#include <sys/resource.h>

#define FILE_NAME "tst_mem1.nc"
#define NUM_FILE_OPENS 100

int main()
{
    int dataset_id, time_id, dummyvar_id, idx;
    size_t start[1] = {0};
    size_t count[1] = {100};
    double data[100];
    struct rusage r_usage;

    for (idx = 0; idx < 100; idx++)
    {
        data[idx]=-99;
    };

    printf("\n*** Testing netcdf-4 memory use.\n");
    printf("*** testing mem use opening/closing file...");
    {
        long my_rss = 0;

        if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &dataset_id)) ERR;
        if (nc_def_dim(dataset_id, "time", NC_UNLIMITED, &time_id)) ERR;
        if (nc_def_var(dataset_id, "dummy", NC_DOUBLE, 1, &time_id, &dummyvar_id)) ERR;
        if (nc_put_vara(dataset_id, dummyvar_id, start, count, data)) ERR;
        if (nc_close(dataset_id)) ERR;

        for (idx = 0; idx < NUM_FILE_OPENS; idx++)
        {
            if (nc_open(FILE_NAME, NC_NOWRITE, &dataset_id)) ERR;
            if (nc_close(dataset_id)) ERR;
            getrusage(RUSAGE_SELF, &r_usage);
            printf("Memory usage: %ld kilobytes\n",r_usage.ru_maxrss);
            if (!my_rss)
                my_rss = r_usage.ru_maxrss;
            else
                if (my_rss != r_usage.ru_maxrss) ERR;
        };
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
