/* This is part of the netCDF package.
 * Copyright 2018 University Corporation for Atmospheric Research/Unidata
 * See COPYRIGHT file for conditions of use.
 *
 * Test whether nc_inq_var_fill() reports fill mode correctly after calling
 * nc_set_fill().
 *
 * Author: Wei-keng Liao.
 */

#include "tests.h"
#include "err_macros.h"

int main(int argc, char** argv) {
    char filename[256];
    int j, k, err, nerrs=0, ncid, varid0, varid1, no_fill;
    char *fmats[5]={"NC_FORMAT_CLASSIC",
                    "NC_FORMAT_64BIT_OFFSET",
                    "NC_FORMAT_CDF5",
                    "NC_FORMAT_NETCDF4",
                    "NC_FORMAT_NETCDF4_CLASSIC"};
    int formats[5]={NC_FORMAT_CLASSIC,
                    NC_FORMAT_64BIT_OFFSET,
                    NC_FORMAT_CDF5,
                    NC_FORMAT_NETCDF4,
                    NC_FORMAT_NETCDF4_CLASSIC};
    int fill_mode[2]={NC_FILL, NC_NOFILL};

    if (argc > 2) {
        printf("Usage: %s [filename]\n",argv[0]);
        return 1;
    }
    if (argc == 2) snprintf(filename, 256, "%s", argv[1]);
    else           strcpy(filename, "tst_var_fill.nc");

    char *cmd_str = (char*)malloc(strlen(argv[0]) + 256);
    sprintf(cmd_str, "*** TESTING C   %s for variable fill mode ", argv[0]);
    printf("%-66s ------ ", cmd_str); fflush(stdout);
    free(cmd_str);

    for (k=0; k<5; k++) {
#ifndef ENABLE_CDF5
        if (formats[k] == NC_FORMAT_CDF5) continue;
#endif
#ifndef USE_NETCDF4
        if (formats[k] == NC_FORMAT_NETCDF4 ||
            formats[k] == NC_FORMAT_NETCDF4_CLASSIC)
            continue;
#endif
        nc_set_default_format(formats[k], NULL);

        for (j=0; j<2; j++) {
            /* create a new file for writing --------------------------------*/
            err = nc_create(filename, NC_CLOBBER, &ncid); CHECK_ERR(err)
            err = nc_def_var(ncid, "var0", NC_INT, 0, NULL, &varid0); CHECK_ERR(err)

            /* set fill mode for the entire file */
            err = nc_set_fill(ncid, fill_mode[j], NULL); CHECK_ERR(err)

            /* all variables defined after nc_set_fill() should inherit the
             * fill mode */
            err = nc_def_var(ncid, "var1", NC_INT, 0, NULL, &varid1); CHECK_ERR(err)
            err = nc_enddef(ncid); CHECK_ERR(err)

            err = nc_inq_var_fill(ncid, varid0, &no_fill, NULL); CHECK_ERR(err)
            if (no_fill && fill_mode[j] == NC_FILL) {
                printf("\nFAIL: %s var0 expect NOFILL but got FILL\n",fmats[k]);
                nerrs++;
            }
            else if (!no_fill && fill_mode[j] == NC_NOFILL) {
                printf("\nFAIL: %s var0 expect FILL but got NOFILL\n",fmats[k]);
                nerrs++;
            }
            err = nc_inq_var_fill(ncid, varid1, &no_fill, NULL); CHECK_ERR(err)
            if (no_fill && fill_mode[j] == NC_FILL) {
                printf("\nFAIL: %s var1 expect NOFILL but got FILL\n",fmats[k]);
                nerrs++;
            }
            else if (!no_fill && fill_mode[j] == NC_NOFILL) {
                printf("\nFAIL: %s var1 expect FILL but got NOFILL\n",fmats[k]);
                nerrs++;
            }
            err = nc_close(ncid); CHECK_ERR(err)
        }
    }

    if (nerrs) printf("fail with %d mismatches\n",nerrs);
    else       printf("pass\n");

    return (nerrs > 0);
}

