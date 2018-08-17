/* This is part of the netCDF package.
 * Copyright 2018 University Corporation for Atmospheric Research/Unidata
 * See COPYRIGHT file for conditions of use.
 *
 * Test whether calling nc_put_att_double() to write _FillValue attribute of
 * int type performs type conversion properly.
 * nc_set_fill().
 *
 * Author: Wei-keng Liao.
 */

#include "tests.h"
#include "err_macros.h"

#define MY_FILLV 52

int main(int argc, char** argv) {
    char filename[256];
    int i, err, nerrs=0, ncid, varid, cmode, buf, no_fill, ifillv;
    double dfillv;
    int formats[5]={0, NC_64BIT_OFFSET, NC_CDF5,
                    NC_NETCDF4, NC_NETCDF4 | NC_CLASSIC_MODEL};

    if (argc > 2) {
        printf("Usage: %s [filename]\n",argv[0]);
        return 1;
    }
    if (argc == 2) snprintf(filename, 256, "%s", argv[1]);
    else           strcpy(filename, "tst_fill_nc4.nc");

    char *cmd_str = (char*)malloc(strlen(argv[0]) + 256);
    sprintf(cmd_str, "*** TESTING C   %s for variable fill mode ", argv[0]);
    printf("%-66s ------ ", cmd_str); fflush(stdout);
    free(cmd_str);

    for (i=0; i<5; i++) {
#ifndef ENABLE_CDF5
        if (formats[i] | NC_CDF5) continue;
#endif
#ifndef USE_NETCDF4
        if (formats[i] | NC_NETCDF4) continue;
#endif
        cmode = NC_CLOBBER | formats[i];
        err = nc_create(filename, cmode, &ncid); CHECK_ERR(err)
        err = nc_def_var(ncid, "var", NC_INT, 0, NULL, &varid); CHECK_ERR(err)
        err = nc_set_fill(ncid, NC_FILL, NULL); CHECK_ERR(err)
        dfillv = (double)MY_FILLV;
        err = nc_put_att_double(ncid, varid, "_FillValue", NC_INT, 1, &dfillv); CHECK_ERR(err)
        err = nc_enddef(ncid); CHECK_ERR(err)

        err = nc_inq_var_fill(ncid, varid, &no_fill, &ifillv); CHECK_ERR(err)
        if (no_fill) {
            printf("Errir: %s expect FILL mode on, but got NOFILL\n",__FILE__);
            nerrs++;
        }
        if (ifillv != MY_FILLV) {
            printf("Errir: %s expect fill value %d, but got %d\n",__FILE__,MY_FILLV,ifillv);
            nerrs++;
        }

        buf = -1;
        err = nc_get_var_int(ncid, varid, &buf); CHECK_ERR(err)
        if (buf != MY_FILLV) {
            printf("Errir: %s expect read value %d, but got %d\n",__FILE__,MY_FILLV,buf);
            nerrs++;
        }

        err = nc_close(ncid); CHECK_ERR(err)
    }

    if (nerrs) printf("fail with %d mismatches\n",nerrs);
    else       printf("pass\n");

    return (nerrs > 0);
}

