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

int main()
{
    int dataset_id, time_id, dummyvar_id, ret, idx;
    size_t start[1] = {0};
    size_t count[1] = {100};
    double data[100];
    struct rusage r_usage;

    for (idx = 0; idx < 100; idx++)
    {
        data[idx]=-99;
    };

    printf("\n*** Testing netcdf-4 memory use with unlimited dimensions.\n");
    printf("*** testing with user-contributed code...");
    {
        if (nc_create("test.nc", NC_CLOBBER | NC_NETCDF4, &dataset_id)) ERR;
        /*ret=nc_create("test.nc", NC_CLOBBER | NC_64BIT_OFFSET, &dataset_id);*/
        if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}
        ret=nc_def_dim(dataset_id, "time", NC_UNLIMITED, &time_id);
        if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}
        ret=nc_def_var(dataset_id, "dummy", NC_DOUBLE, 1, &time_id, &dummyvar_id);
        if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}
        /*ret=nc_enddef(dataset_id);*/
        ret=nc_put_vara(dataset_id, dummyvar_id, start, count, data);
        if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}
        ret=nc_close(dataset_id);
        if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}

        for (idx = 0; idx < 100000; idx++) {
            ret=nc_open("test.nc", NC_NOWRITE, &dataset_id);
            if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}
            ret=nc_close(dataset_id);
            if(ret!=NC_NOERR) {printf("%s\n",nc_strerror(ret)); return 1;}
            getrusage(RUSAGE_SELF,&r_usage);
            /* printf("Memory usage: %ld kilobytes\n",r_usage.ru_maxrss); */
        };
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
