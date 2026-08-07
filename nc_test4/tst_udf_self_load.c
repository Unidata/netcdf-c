/* This is part of the netCDF package. Copyright 2005-2025 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test that the UDF plugin loader correctly handles init functions
   that return NC_Dispatch* (self-registration convention) when
   loading plugins via RC file configuration.

   This test:
   1. Triggers nc_initialize() via a netcdf call
   2. Injects RC entries programmatically via NC_rcfile_insert()
   3. Calls NC_udf_load_plugins() to load the test plugin .so
   4. Verifies the dispatch table was registered correctly

   Ed Hartnett
*/

#include "config.h"
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "netcdf_dispatch.h"
#include "ncrc.h"
#include "ncudfplugins.h"

/* Path to the test plugin shared library, set by CMake. */
#ifndef TEST_PLUGIN_PATH
#error "TEST_PLUGIN_PATH must be defined by the build system"
#endif

#define FILE_NAME "tst_udf_self_load.nc"

int
main(int argc, char **argv)
{
    printf("\n*** Testing UDF self-registration plugin loading.\n");
    printf("*** testing plugin loader with NC_Dispatch*-returning init...");
    {
        int ncid;
        NC_Dispatch *disp_in = NULL;

        /* Trigger nc_initialize() so the RC subsystem is ready. */
        if (nc_create(FILE_NAME, 0, &ncid)) ERR;
        if (nc_close(ncid)) ERR;

        /* Inject RC entries to configure UDF0 to load our test plugin.
         * NC_rcfile_insert() adds entries to the in-memory RC table
         * without needing a .ncrc file on disk. */
        if (NC_rcfile_insert("NETCDF.UDF0.LIBRARY", NULL, NULL,
                             TEST_PLUGIN_PATH)) ERR;
        if (NC_rcfile_insert("NETCDF.UDF0.INIT", NULL, NULL,
                             "tst_udf_self_load_init")) ERR;

        /* Call the plugin loader directly. This will:
         * 1. dlopen() the plugin library
         * 2. dlsym() the init function
         * 3. Call it (expecting NC_Dispatch* return when
         *    HAVE_NETCDF_UDF_SELF_REGISTRATION is defined)
         * 4. Register the dispatch table via nc_def_user_format()
         *
         * BUG (before fix): The loader would cast the init function
         * as int (*)(void), truncating the NC_Dispatch* pointer to
         * int, and interpret the non-zero value as an error code. */
        if (NC_udf_load_plugins()) ERR;

        /* Verify the dispatch table was registered for UDF0. */
        if (nc_inq_user_format(NC_UDF0, &disp_in, NULL)) ERR;
        if (disp_in == NULL) ERR;

        /* Verify the dispatch table has the correct ABI version. */
        if (disp_in->dispatch_version != NC_DISPATCH_VERSION) ERR;

        /* Verify the dispatch table has the correct model. */
        if (disp_in->model != NC_FORMATX_UDF0) ERR;
    }
    SUMMARIZE_ERR;
    printf("*** testing plugin loader with magic number...");
    {
        NC_Dispatch *disp_in = NULL;
        char magic_in[NC_MAX_MAGIC_NUMBER_LEN + 1];

        /* Re-insert with a magic number this time. */
        if (NC_rcfile_insert("NETCDF.UDF1.LIBRARY", NULL, NULL,
                             TEST_PLUGIN_PATH)) ERR;
        if (NC_rcfile_insert("NETCDF.UDF1.INIT", NULL, NULL,
                             "tst_udf_self_load_init")) ERR;
        if (NC_rcfile_insert("NETCDF.UDF1.MAGIC", NULL, NULL,
                             "TUSL")) ERR;

        /* Load plugins again — UDF1 should now be configured. */
        if (NC_udf_load_plugins()) ERR;

        /* Verify UDF1 dispatch table was registered. */
        memset(magic_in, 0, sizeof(magic_in));
        if (nc_inq_user_format(NC_UDF1, &disp_in, magic_in)) ERR;
        if (disp_in == NULL) ERR;
        if (disp_in->dispatch_version != NC_DISPATCH_VERSION) ERR;
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
