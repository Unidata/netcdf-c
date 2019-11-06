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
main() {/* create ref_tst_small.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* group ids */
    int tst_small_grp;

    /* dimension ids */
    int Time_dim;
    int DateStrLen_dim;

    /* dimension lengths */
    size_t Time_len = NC_UNLIMITED;
    size_t DateStrLen_len = 19;

    /* variable ids */
    int Times_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_Times 2

    /* variable shapes */
    int Times_dims[RANK_Times];

    /* enter define mode */
    stat = nc_create("ref_tst_small.nc", NC_CLOBBER|NC_NETCDF4, &ncid);
    check_err(stat,__LINE__,__FILE__);
    tst_small_grp = ncid;

    /* define dimensions */
    stat = nc_def_dim(tst_small_grp, "Time", Time_len, &Time_dim);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_def_dim(tst_small_grp, "DateStrLen", DateStrLen_len, &DateStrLen_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    Times_dims[0] = Time_dim;
    Times_dims[1] = DateStrLen_dim;
    stat = nc_def_var(tst_small_grp, "Times", NC_CHAR, RANK_Times, Times_dims, &Times_id);
    check_err(stat,__LINE__,__FILE__);

    /* assign global attributes */

    {
    stat = nc_put_att_text(tst_small_grp, NC_GLOBAL, "TITLE", 31, " OUTPUT FROM WRF V2.0.3.1 MODEL");
    check_err(stat,__LINE__,__FILE__);
    }


    /* leave define mode */
    stat = nc_enddef (tst_small_grp);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    {
    char* Times_data = "2005-04-11_12:00:002005-04-11_13:00:00" ;
    size_t Times_startset[2] = {0, 0} ;
    size_t Times_countset[2] = {2, 19};
    stat = nc_put_vara(tst_small_grp, Times_id, Times_startset, Times_countset, Times_data);
    check_err(stat,__LINE__,__FILE__);
    }


    stat = nc_close(tst_small_grp);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
