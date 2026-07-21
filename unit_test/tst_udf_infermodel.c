/* This is part of the netCDF package. Copyright 2026 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test for netCDF-C issue #3410. A UDF magic number could identify
   a UDF implementation while leaving NCmodel.format unset. Callers
   need both fields: implementation selects the UDF dispatch table and
   format records the data model and supplies the corresponding mode.

   The test begins with a preselected UDF implementation and a zero
   format, then uses a UDF magic number to complete inference. It
   verifies that the UDF is retained, NC_FORMAT_NETCDF4 is assigned,
   and NC_NETCDF4 is returned in the open mode.

   Author: Edward Hartnett, 2026-07-21
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "ncdispatch.h"

#define FILE_NAME "tst_udf_infermodel.udf"

int
main(int argc, char **argv)
{
    NC_Dispatch dispatch;
    NCmodel model;
    FILE *file;
    int omode = NC_NOWRITE;

    printf("\n*** Testing UDF magic format inference.\n");
    printf("*** testing UDF magic establishes the enhanced data model...");

    memset(&dispatch, 0, sizeof(dispatch));
    dispatch.model = NC_FORMATX_UDF0;
    dispatch.dispatch_version = NC_DISPATCH_VERSION;
    if (nc_def_user_format(NC_UDF0, &dispatch, "TINF")) ERR;

    if (!(file = fopen(FILE_NAME, "wb"))) ERR;
    if (fwrite("TINF00000", 1, 9, file) != 9) ERR;
    if (fclose(file)) ERR;

    memset(&model, 0, sizeof(model));
    model.impl = NC_FORMATX_UDF0;
    if (NC_infermodel(FILE_NAME, &omode, 0, 0, NULL, &model, NULL)) ERR;
    if (model.impl != NC_FORMATX_UDF0) ERR;
    if (model.format != NC_FORMAT_NETCDF4) ERR;
    if (!(omode & NC_NETCDF4)) ERR;

    remove(FILE_NAME);
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
