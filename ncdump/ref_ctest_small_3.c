#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>


void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main() {/* create small.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* variable ids */
    int t_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_t 0

    /* enter define mode */
    stat = nc_create("small.nc", NC_CLOBBER, &ncid);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    stat = nc_def_var(ncid, "t", NC_BYTE, RANK_t, 0, &t_id);
    check_err(stat,__LINE__,__FILE__);

    /* leave define mode */
    stat = nc_enddef (ncid);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    {
    size_t count = 0;
    static signed char t_data[1] = {1};
    stat = nc_put_var1(ncid, t_id, &count, t_data);
    check_err(stat,__LINE__,__FILE__);
    }


    stat = nc_close(ncid);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
