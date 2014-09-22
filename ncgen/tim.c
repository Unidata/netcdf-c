#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>


#define e1_PERIOD_pass ((signed char)0)
#define e1_PERIOD_fail ((signed char)1)
#define e1_PERIOD_undefined ((signed char)2)
typedef signed char e1;
#define e2_PERIOD_defined ((signed char)0)
#define e2_PERIOD_undefined ((signed char)1)
typedef signed char e2;


void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main() {/* create tim.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* group ids */
    int tim_grp;

    /* type ids */
    int e1_typ;
    int e2_typ;

    /* variable ids */
    int test1_id;
    int test2_id;
    int test3_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_test1 0
#   define RANK_test2 0
#   define RANK_test3 0

    /* enter define mode */
    stat = nc_create("tim.nc", NC_CLOBBER|NC_NETCDF4, &ncid);
    check_err(stat,__LINE__,__FILE__);
    tim_grp = ncid;

    {
    signed char econst;
    stat = nc_def_enum(tim_grp, NC_BYTE, "e1", &e1_typ);
    check_err(stat,__LINE__,__FILE__);
    econst = 0;
    stat = nc_insert_enum(tim_grp, e1_typ, "pass", &econst);
    econst = 1;
    stat = nc_insert_enum(tim_grp, e1_typ, "fail", &econst);
    econst = 2;
    stat = nc_insert_enum(tim_grp, e1_typ, "undefined", &econst);
    }

    {
    signed char econst;
    stat = nc_def_enum(tim_grp, NC_BYTE, "e2", &e2_typ);
    check_err(stat,__LINE__,__FILE__);
    econst = 0;
    stat = nc_insert_enum(tim_grp, e2_typ, "defined", &econst);
    econst = 1;
    stat = nc_insert_enum(tim_grp, e2_typ, "undefined", &econst);
    }


    /* define variables */

    stat = nc_def_var(tim_grp, "test1", e1_typ, RANK_test1, 0, &test1_id);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_def_var(tim_grp, "test2", e2_typ, RANK_test2, 0, &test2_id);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_def_var(tim_grp, "test3", e1_typ, RANK_test3, 0, &test3_id);
    check_err(stat,__LINE__,__FILE__);

    /* leave define mode */
    stat = nc_enddef (tim_grp);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    {
    size_t count = 0;
    static e1 test1_data[1] = {0};
    stat = nc_put_var1(tim_grp, test1_id, &count, test1_data);
    check_err(stat,__LINE__,__FILE__);
    }


    {
    size_t count = 0;
    static e2 test2_data[1] = {1};
    stat = nc_put_var1(tim_grp, test2_id, &count, test2_data);
    check_err(stat,__LINE__,__FILE__);
    }


    {
    size_t count = 0;
    static e1 test3_data[1] = {1};
    stat = nc_put_var1(tim_grp, test3_id, &count, test3_data);
    check_err(stat,__LINE__,__FILE__);
    }


    stat = nc_close(tim_grp);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
