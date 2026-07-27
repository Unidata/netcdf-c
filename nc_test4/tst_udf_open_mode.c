/* This is part of the netCDF package. Copyright 2005-2025 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test that passing NC_UDFx in the open mode selects the corresponding
   registered UDF dispatch table, even when the file on disk is a valid
   NetCDF-4/HDF5 file that would normally be auto-detected as HDF5.

   This test exercises all 10 UDF slots (UDF0 through UDF9) by
   registering a minimal dispatch table in each slot and verifying that
   nc_open() with the matching mode flag dispatches to the UDF handler
   instead of to the HDF5 handler.

   See https://github.com/Unidata/netcdf-c/issues/3417

   Ed Hartnett
*/

#include "config.h"
#include <string.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "netcdf_dispatch.h"
#include "ncdispatch.h"

#define FILE_NAME "tst_udf_open_mode.nc"

static const int udf_modes[NC_MAX_UDF_FORMATS] = {
    NC_UDF0, NC_UDF1, NC_UDF2, NC_UDF3, NC_UDF4,
    NC_UDF5, NC_UDF6, NC_UDF7, NC_UDF8, NC_UDF9
};

static const int udf_models[NC_MAX_UDF_FORMATS] = {
    NC_FORMATX_UDF0, NC_FORMATX_UDF1, NC_FORMATX_UDF2, NC_FORMATX_UDF3,
    NC_FORMATX_UDF4, NC_FORMATX_UDF5, NC_FORMATX_UDF6, NC_FORMATX_UDF7,
    NC_FORMATX_UDF8, NC_FORMATX_UDF9
};

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

int
udf_get_vara(int ncid, int varid, const size_t *start, const size_t *count,
             void *value, nc_type t)
{
    return NC_NOERR;
}

/* Dispatch tables populated at runtime (MSVC-compatible). */
static NC_Dispatch udf_dispatchers[NC_MAX_UDF_FORMATS];

static void
init_dispatchers(void)
{
    int i;

    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        NC_Dispatch *dsp = &udf_dispatchers[i];
        memset(dsp, 0, sizeof(NC_Dispatch));

        dsp->model = udf_models[i];
        dsp->dispatch_version = NC_DISPATCH_VERSION;

        dsp->create = NC_RO_create;
        dsp->open = udf_open;

        dsp->redef = NC_RO_redef;
        dsp->_enddef = NC_RO__enddef;
        dsp->sync = NC_RO_sync;
        dsp->abort = udf_abort;
        dsp->close = udf_close;
        dsp->set_fill = NC_RO_set_fill;
        dsp->inq_format = udf_inq_format;
        dsp->inq_format_extended = udf_inq_format_extended;

        dsp->def_dim = NC_RO_def_dim;
        dsp->rename_dim = NC_RO_rename_dim;

        dsp->rename_att = NC_RO_rename_att;
        dsp->del_att = NC_RO_del_att;
        dsp->put_att = NC_RO_put_att;

        dsp->def_var = NC_RO_def_var;
        dsp->rename_var = NC_RO_rename_var;
        dsp->get_vara = udf_get_vara;
        dsp->put_vara = NC_RO_put_vara;
        dsp->get_vars = NCDEFAULT_get_vars;
        dsp->put_vars = NCDEFAULT_put_vars;
        dsp->get_varm = NCDEFAULT_get_varm;
        dsp->put_varm = NCDEFAULT_put_varm;

        dsp->var_par_access = NC_NOTNC4_var_par_access;
        dsp->def_var_fill = NC_RO_def_var_fill;

        dsp->inq_ncid = NC_NOTNC4_inq_ncid;
        dsp->inq_grps = NC_NOTNC4_inq_grps;
        dsp->inq_grpname = NC_NOTNC4_inq_grpname;
        dsp->inq_grpname_full = NC_NOTNC4_inq_grpname_full;
        dsp->inq_grp_parent = NC_NOTNC4_inq_grp_parent;
        dsp->inq_grp_full_ncid = NC_NOTNC4_inq_grp_full_ncid;
        dsp->inq_varids = NC_NOTNC4_inq_varids;
        dsp->inq_dimids = NC_NOTNC4_inq_dimids;
        dsp->inq_typeids = NC_NOTNC4_inq_typeids;
        dsp->def_grp = NC_NOTNC4_def_grp;
        dsp->rename_grp = NC_NOTNC4_rename_grp;
        dsp->inq_user_type = NC_NOTNC4_inq_user_type;
        dsp->inq_typeid = NC_NOTNC4_inq_typeid;

        dsp->def_compound = NC_NOTNC4_def_compound;
        dsp->insert_compound = NC_NOTNC4_insert_compound;
        dsp->insert_array_compound = NC_NOTNC4_insert_array_compound;
        dsp->inq_compound_field = NC_NOTNC4_inq_compound_field;
        dsp->inq_compound_fieldindex = NC_NOTNC4_inq_compound_fieldindex;
        dsp->def_vlen = NC_NOTNC4_def_vlen;
        dsp->put_vlen_element = NC_NOTNC4_put_vlen_element;
        dsp->get_vlen_element = NC_NOTNC4_get_vlen_element;
        dsp->def_enum = NC_NOTNC4_def_enum;
        dsp->insert_enum = NC_NOTNC4_insert_enum;
        dsp->inq_enum_member = NC_NOTNC4_inq_enum_member;
        dsp->inq_enum_ident = NC_NOTNC4_inq_enum_ident;
        dsp->def_opaque = NC_NOTNC4_def_opaque;
        dsp->def_var_deflate = NC_NOTNC4_def_var_deflate;
        dsp->def_var_fletcher32 = NC_NOTNC4_def_var_fletcher32;
        dsp->def_var_chunking = NC_NOTNC4_def_var_chunking;
        dsp->def_var_endian = NC_NOTNC4_def_var_endian;
        dsp->def_var_filter = NC_NOTNC4_def_var_filter;
        dsp->set_var_chunk_cache = NC_NOTNC4_set_var_chunk_cache;
        dsp->get_var_chunk_cache = NC_NOTNC4_get_var_chunk_cache;
#if NC_DISPATCH_VERSION >= 3
        dsp->inq_var_filter_ids = NC_NOOP_inq_var_filter_ids;
        dsp->inq_var_filter_info = NC_NOOP_inq_var_filter_info;
#endif
#if NC_DISPATCH_VERSION >= 4
        dsp->def_var_quantize = NC_NOTNC4_def_var_quantize;
        dsp->inq_var_quantize = NC_NOTNC4_inq_var_quantize;
#endif
#if NC_DISPATCH_VERSION >= 5
        dsp->inq_filter_avail = NC_NOOP_inq_filter_avail;
#endif
    }
}

int
main(int argc, char **argv)
{
    int ncid;
    int i;
    NC_Dispatch *disp_save;

    init_dispatchers();
    printf("\n*** Testing nc_open() with NC_UDFx mode flag dispatch override.\n");

    printf("*** creating a valid NetCDF-4/HDF5 file...");
    /* Its magic number is \x89HDF\r\n\x1a\n, which would normally cause
     * nc_open() to select the HDF5 dispatch table. */
    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
    if (nc_close(ncid)) ERR;
    SUMMARIZE_ERR;

    printf("*** testing that opening with unregistered UDF slots fails...");
    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        if (nc_open(FILE_NAME, udf_modes[i], &ncid) == NC_NOERR) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** testing all UDF slots can be registered...");
    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        if (nc_def_user_format(udf_modes[i], &udf_dispatchers[i], NULL)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** testing each UDF slot overrides HDF5 magic number detection...");
    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        if (nc_open(FILE_NAME, udf_modes[i], &ncid)) ERR;
        if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
        if (nc_inq_format_extended(ncid, NULL, NULL) != TEST_VAL_42) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** testing control: without NC_UDFx, HDF5 detection works...");
    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    {
        int fmt;
        if (nc_inq_format(ncid, &fmt)) ERR;
        if (fmt != NC_FORMAT_NETCDF4) ERR;
    }
    if (nc_close(ncid)) ERR;
    SUMMARIZE_ERR;

    printf("*** testing each NC_UDFx | NC_WRITE combined flags...");
    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        if (nc_open(FILE_NAME, udf_modes[i] | NC_WRITE, &ncid)) ERR;
        if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
        if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** testing nc_inq_user_format returns registered dispatchers...");
    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        if (nc_inq_user_format(udf_modes[i], &disp_save, NULL)) ERR;
        if (disp_save != &udf_dispatchers[i]) ERR;
    }
    SUMMARIZE_ERR;

    FINAL_RESULTS;
}
