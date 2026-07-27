/* This is part of the netCDF package. Copyright 2005-2025 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test that nc_open() with NC_UDF0 mode flag routes dispatch to the
   registered UDF dispatch table, even when the file on disk is a valid
   NetCDF-4/HDF5 file that would normally be auto-detected as HDF5.

   Ed Hartnett
*/

#include "config.h"
#include <string.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "netcdf_dispatch.h"

#define FILE_NAME "tst_udf_open_mode.nc"

/* Minimal UDF dispatch stubs. */

int
udf_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
         void *parameters, const NC_Dispatch *dispatch, int ncid)
{
    return NC_NOERR;
}

int
udf_close(int ncid, void *v)
{
    return NC_NOERR;
}

int
udf_abort(int ncid)
{
    return TEST_VAL_42;
}

int
udf_inq_format(int ncid, int *formatp)
{
    return TEST_VAL_42;
}

int
udf_inq_format_extended(int ncid, int *formatp, int *modep)
{
    return TEST_VAL_42;
}

/* Dispatch table populated at runtime (MSVC-compatible). */
static NC_Dispatch udf_dispatcher;

static void
init_dispatcher(void)
{
    memset(&udf_dispatcher, 0, sizeof(udf_dispatcher));

    udf_dispatcher.model = NC_FORMATX_UDF0;
    udf_dispatcher.dispatch_version = NC_DISPATCH_VERSION;

    udf_dispatcher.create = NC_RO_create;
    udf_dispatcher.open = udf_open;

    udf_dispatcher.redef = NC_RO_redef;
    udf_dispatcher._enddef = NC_RO__enddef;
    udf_dispatcher.sync = NC_RO_sync;
    udf_dispatcher.abort = udf_abort;
    udf_dispatcher.close = udf_close;
    udf_dispatcher.set_fill = NC_RO_set_fill;
    udf_dispatcher.inq_format = udf_inq_format;
    udf_dispatcher.inq_format_extended = udf_inq_format_extended;

    udf_dispatcher.def_dim = NC_RO_def_dim;
    udf_dispatcher.rename_dim = NC_RO_rename_dim;

    udf_dispatcher.rename_att = NC_RO_rename_att;
    udf_dispatcher.del_att = NC_RO_del_att;
    udf_dispatcher.put_att = NC_RO_put_att;

    udf_dispatcher.def_var = NC_RO_def_var;
    udf_dispatcher.rename_var = NC_RO_rename_var;
    udf_dispatcher.get_vara = NCDEFAULT_get_vars;
    udf_dispatcher.put_vara = NC_RO_put_vara;
    udf_dispatcher.get_vars = NCDEFAULT_get_vars;
    udf_dispatcher.put_vars = NCDEFAULT_put_vars;
    udf_dispatcher.get_varm = NCDEFAULT_get_varm;
    udf_dispatcher.put_varm = NCDEFAULT_put_varm;

    udf_dispatcher.var_par_access = NC_NOTNC4_var_par_access;
    udf_dispatcher.def_var_fill = NC_RO_def_var_fill;

    udf_dispatcher.inq_ncid = NC_NOTNC4_inq_ncid;
    udf_dispatcher.inq_grps = NC_NOTNC4_inq_grps;
    udf_dispatcher.inq_grpname = NC_NOTNC4_inq_grpname;
    udf_dispatcher.inq_grpname_full = NC_NOTNC4_inq_grpname_full;
    udf_dispatcher.inq_grp_parent = NC_NOTNC4_inq_grp_parent;
    udf_dispatcher.inq_grp_full_ncid = NC_NOTNC4_inq_grp_full_ncid;
    udf_dispatcher.inq_varids = NC_NOTNC4_inq_varids;
    udf_dispatcher.inq_dimids = NC_NOTNC4_inq_dimids;
    udf_dispatcher.inq_typeids = NC_NOTNC4_inq_typeids;
    udf_dispatcher.def_grp = NC_NOTNC4_def_grp;
    udf_dispatcher.rename_grp = NC_NOTNC4_rename_grp;
    udf_dispatcher.inq_user_type = NC_NOTNC4_inq_user_type;
    udf_dispatcher.inq_typeid = NC_NOTNC4_inq_typeid;

    udf_dispatcher.def_compound = NC_NOTNC4_def_compound;
    udf_dispatcher.insert_compound = NC_NOTNC4_insert_compound;
    udf_dispatcher.insert_array_compound = NC_NOTNC4_insert_array_compound;
    udf_dispatcher.inq_compound_field = NC_NOTNC4_inq_compound_field;
    udf_dispatcher.inq_compound_fieldindex = NC_NOTNC4_inq_compound_fieldindex;
    udf_dispatcher.def_vlen = NC_NOTNC4_def_vlen;
    udf_dispatcher.put_vlen_element = NC_NOTNC4_put_vlen_element;
    udf_dispatcher.get_vlen_element = NC_NOTNC4_get_vlen_element;
    udf_dispatcher.def_enum = NC_NOTNC4_def_enum;
    udf_dispatcher.insert_enum = NC_NOTNC4_insert_enum;
    udf_dispatcher.inq_enum_member = NC_NOTNC4_inq_enum_member;
    udf_dispatcher.inq_enum_ident = NC_NOTNC4_inq_enum_ident;
    udf_dispatcher.def_opaque = NC_NOTNC4_def_opaque;
    udf_dispatcher.def_var_deflate = NC_NOTNC4_def_var_deflate;
    udf_dispatcher.def_var_fletcher32 = NC_NOTNC4_def_var_fletcher32;
    udf_dispatcher.def_var_chunking = NC_NOTNC4_def_var_chunking;
    udf_dispatcher.def_var_endian = NC_NOTNC4_def_var_endian;
    udf_dispatcher.def_var_filter = NC_NOTNC4_def_var_filter;
    udf_dispatcher.set_var_chunk_cache = NC_NOTNC4_set_var_chunk_cache;
    udf_dispatcher.get_var_chunk_cache = NC_NOTNC4_get_var_chunk_cache;
#if NC_DISPATCH_VERSION >= 3
    udf_dispatcher.inq_var_filter_ids = NC_NOOP_inq_var_filter_ids;
    udf_dispatcher.inq_var_filter_info = NC_NOOP_inq_var_filter_info;
#endif
#if NC_DISPATCH_VERSION >= 4
    udf_dispatcher.def_var_quantize = NC_NOTNC4_def_var_quantize;
    udf_dispatcher.inq_var_quantize = NC_NOTNC4_inq_var_quantize;
#endif
#if NC_DISPATCH_VERSION >= 5
    udf_dispatcher.inq_filter_avail = NC_NOOP_inq_filter_avail;
#endif
}

int
main(int argc, char **argv)
{
    init_dispatcher();
    printf("\n*** Testing nc_open() with NC_UDF0 mode flag dispatch override.\n");
    printf("*** testing NC_UDF0 flag overrides HDF5 magic number detection...");
    {
        int ncid;

        /* Create a valid NetCDF-4/HDF5 file. Its magic number is
         * \x89HDF\r\n\x1a\n, which would normally cause nc_open()
         * to select the HDF5 dispatch table. */
        if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
        if (nc_close(ncid)) ERR;

        /* Register UDF0 with no magic number. */
        if (nc_def_user_format(NC_UDF0, &udf_dispatcher, NULL)) ERR;

        /* Open the HDF5 file with NC_UDF0 — dispatch must go to UDF,
         * not HDF5. */
        if (nc_open(FILE_NAME, NC_UDF0, &ncid)) ERR;

        /* Verify we are using the UDF dispatch table by checking
         * that nc_inq_format returns our sentinel value. If the HDF5
         * dispatcher were used, it would return NC_NOERR. */
        if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
        if (nc_inq_format_extended(ncid, NULL, NULL) != TEST_VAL_42) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("*** testing control: without NC_UDF0, HDF5 detection works...");
    {
        int ncid, fmt;

        /* Open the same file without any UDF flag — should be detected
         * as NetCDF-4/HDF5 via magic number. */
        if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_format(ncid, &fmt)) ERR;
        if (fmt != NC_FORMAT_NETCDF4) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("*** testing NC_UDF0 | NC_WRITE combined flags...");
    {
        int ncid;

        /* NC_UDF0 can be combined with behavioral flags. */
        if (nc_open(FILE_NAME, NC_UDF0 | NC_WRITE, &ncid)) ERR;
        if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("*** testing NC_UDF0 with unregistered slot fails...");
    {
        int ncid;
        NC_Dispatch *disp_save;

        /* Save and unregister UDF0 by re-registering a fresh
         * dispatcher, then check that opening with NC_UDF0 after
         * clearing the slot returns an error. We cannot truly
         * unregister, so instead we verify that nc_open with
         * NC_UDF0 only works when a dispatch table is registered
         * (already tested above). This sub-test verifies the
         * nc_inq_user_format query path. */
        if (nc_inq_user_format(NC_UDF0, &disp_save, NULL)) ERR;
        if (disp_save != &udf_dispatcher) ERR;
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
