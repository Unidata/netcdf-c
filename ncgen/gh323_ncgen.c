#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>


typedef struct obs_t {
    float obs_t_PERIOD_lat;
    float obs_t_PERIOD_lon;
    char* obs_t_PERIOD_stid;
    double obs_t_PERIOD_time;
    float obs_t_PERIOD_temperature;
} obs_t;


void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main() {/* create gh323.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* group ids */
    int stations_grp;

    /* type ids */
    int obs_t_typ;

    /* dimension ids */
    int n_dim;

    /* dimension lengths */
    size_t n_len = NC_UNLIMITED;

    /* variable ids */
    int obs_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_obs 1

    /* variable shapes */
    int obs_dims[RANK_obs];

    /* enter define mode */
    stat = nc_create("gh323.nc", NC_CLOBBER|NC_NETCDF4, &ncid);
    check_err(stat,__LINE__,__FILE__);
    stations_grp = ncid;

    stat = nc_def_compound(stations_grp, sizeof(obs_t), "obs_t", &obs_t_typ);    check_err(stat,__LINE__,__FILE__);
    {
      size_t lat_offset = NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_lat);
      size_t lon_offset = NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_lon);
      size_t stid_offset = NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_stid);
      size_t time_offset = NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_time);
      size_t temp_offset = NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_temperature);

      printf("sizeof(obs_t): %ld\n", sizeof(obs_t));
      printf("sizeof(float): %ld\n", sizeof(float));
      printf("sizeof(char*): %ld\n", sizeof(char*));
      printf("sizeof(double): %ld\n", sizeof(double));

      printf("lat_offset  (float): %zu\n",lat_offset);
      printf("lon_offset  (float): %zu\n",lon_offset);
      printf("stid_offset (char*): %zu\n",stid_offset);
      printf("time_offset (double): %zu\n",time_offset);
      printf("temp_offset (float): %zu\n",temp_offset);

    stat = nc_insert_compound(stations_grp, obs_t_typ, "lat", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_lat), NC_FLOAT);    check_err(stat,__LINE__,__FILE__);
    stat = nc_insert_compound(stations_grp, obs_t_typ, "lon", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_lon), NC_FLOAT);    check_err(stat,__LINE__,__FILE__);
    stat = nc_insert_compound(stations_grp, obs_t_typ, "stid", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_stid), NC_STRING);    check_err(stat,__LINE__,__FILE__);
    stat = nc_insert_compound(stations_grp, obs_t_typ, "time", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_time), NC_DOUBLE);    check_err(stat,__LINE__,__FILE__);
    stat = nc_insert_compound(stations_grp, obs_t_typ, "temperature", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_temperature), NC_FLOAT);    check_err(stat,__LINE__,__FILE__);
    }


    /* define dimensions */
    stat = nc_def_dim(stations_grp, "n", n_len, &n_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    obs_dims[0] = n_dim;
    stat = nc_def_var(stations_grp, "obs", obs_t_typ, RANK_obs, obs_dims, &obs_id);
    check_err(stat,__LINE__,__FILE__);

    /* leave define mode */
    stat = nc_enddef (stations_grp);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    {
    obs_t obs_data[2] = {{((float)39.900002), ((float)-104.9), "KDEN", ((double)7776000), ((float)15)}, {((float)40), ((float)-105), "KBOU", ((double)7776000), ((float)16)}} ;
    size_t obs_startset[1] = {0} ;
    size_t obs_countset[1] = {2};
    stat = nc_put_vara(stations_grp, obs_id, obs_startset, obs_countset, obs_data);
    check_err(stat,__LINE__,__FILE__);
    }


    stat = nc_close(stations_grp);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
