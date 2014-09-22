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
main() {/* create test.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* dimension ids */
    int station_dim;
    int obs_dim;

    /* dimension lengths */
    size_t station_len = 207;
    size_t obs_len = 1183;

    /* variable ids */
    int wave_direction_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_wave_direction 2

    /* variable shapes */
    int wave_direction_dims[RANK_wave_direction];

    /* enter define mode */
    stat = nc_create("test.nc", NC_CLOBBER, &ncid);
    check_err(stat,__LINE__,__FILE__);

    /* define dimensions */
    stat = nc_def_dim(ncid, "station", station_len, &station_dim);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_def_dim(ncid, "obs", obs_len, &obs_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    wave_direction_dims[0] = station_dim;
    wave_direction_dims[1] = obs_dim;
    stat = nc_def_var(ncid, "wave_direction", NC_CHAR, RANK_wave_direction, wave_direction_dims, &wave_direction_id);
    check_err(stat,__LINE__,__FILE__);

    /* assign per-variable attributes */

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "long_name", 14, "wave_direction");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "standard_name", 14, "wave_direction");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "nodc_name", 14, "wave direction");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "units", 1, "");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float KOD_serial_oceanographic_data_surface_2006_scale_factor_att[1] = {((float)0)} ;
    stat = nc_put_att_float(ncid, wave_direction_id, "scale_factor", NC_FLOAT, 1, KOD_serial_oceanographic_data_surface_2006_scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float KOD_serial_oceanographic_data_surface_2006_add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(ncid, wave_direction_id, "add_offset", NC_FLOAT, 1, KOD_serial_oceanographic_data_surface_2006_add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "coordinates", 12, "time lat lon");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "_FillValue", 1, "\000");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, wave_direction_id, "comment", 41, "three character, 16 point compass bearing");
    check_err(stat,__LINE__,__FILE__);
    }


    /* leave define mode */
    stat = nc_enddef (ncid);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    stat = nc_close(ncid);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
