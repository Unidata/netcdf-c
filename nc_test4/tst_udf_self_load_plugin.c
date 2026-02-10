/* This is part of the netCDF package. Copyright 2005-2025 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Minimal UDF plugin that uses the self-registration convention:
   the init function returns NC_Dispatch* instead of int.

   This plugin is built as a shared library and loaded by
   tst_udf_self_load via the RC-based plugin loader to verify
   that the loader correctly handles NC_Dispatch*-returning init
   functions.

   Ed Hartnett
*/

#include "config.h"
#include "netcdf.h"
#include "netcdf_dispatch.h"

/* Stub open function that always succeeds. */
static int
plugin_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
            void *parameters, const NC_Dispatch *dispatch, int ncid)
{
    return NC_NOERR;
}

/* Stub close function that always succeeds. */
static int
plugin_close(int ncid, void *v)
{
    return NC_NOERR;
}

/* Stub inq_format that returns a recognizable value. */
static int
plugin_inq_format(int ncid, int *formatp)
{
    return NC_NOERR;
}

/* Stub inq_format_extended. */
static int
plugin_inq_format_extended(int ncid, int *formatp, int *modep)
{
    return NC_NOERR;
}

/* Stub abort. */
static int
plugin_abort(int ncid)
{
    return NC_NOERR;
}

/* The dispatch table for this test plugin. Uses NC_RO_* and
 * NC_NOTNC4_* stubs exported by libnetcdf for all functions
 * that this plugin does not implement. */
static NC_Dispatch tst_self_load_dispatcher = {

    NC_FORMATX_UDF0,
    NC_DISPATCH_VERSION,

    NC_RO_create,
    plugin_open,

    NC_RO_redef,
    NC_RO__enddef,
    NC_RO_sync,
    plugin_abort,
    plugin_close,
    NC_RO_set_fill,
    plugin_inq_format,
    plugin_inq_format_extended,

    NULL, /* inq */
    NULL, /* inq_type */

    NC_RO_def_dim,
    NULL, /* inq_dimid */
    NULL, /* inq_dim */
    NULL, /* inq_unlimdim */
    NC_RO_rename_dim,

    NULL, /* inq_att */
    NULL, /* inq_attid */
    NULL, /* inq_attname */
    NC_RO_rename_att,
    NC_RO_del_att,
    NULL, /* get_att */
    NC_RO_put_att,

    NC_RO_def_var,
    NULL, /* inq_varid */
    NC_RO_rename_var,
    NULL, /* get_vara */
    NC_RO_put_vara,
    NULL, /* get_vars */
    NULL, /* put_vars */
    NULL, /* get_varm */
    NULL, /* put_varm */

    NULL, /* inq_var_all */

    NC_NOTNC4_var_par_access,
    NC_RO_def_var_fill,

    NULL, /* show_metadata */
    NULL, /* inq_unlimdims */

    NC_NOTNC4_inq_ncid,
    NC_NOTNC4_inq_grps,
    NC_NOTNC4_inq_grpname,
    NC_NOTNC4_inq_grpname_full,
    NC_NOTNC4_inq_grp_parent,
    NC_NOTNC4_inq_grp_full_ncid,
    NC_NOTNC4_inq_varids,
    NC_NOTNC4_inq_dimids,
    NC_NOTNC4_inq_typeids,
    NULL, /* inq_type_equal */
    NC_NOTNC4_def_grp,
    NC_NOTNC4_rename_grp,
    NC_NOTNC4_inq_user_type,
    NC_NOTNC4_inq_typeid,

    NC_NOTNC4_def_compound,
    NC_NOTNC4_insert_compound,
    NC_NOTNC4_insert_array_compound,
    NC_NOTNC4_inq_compound_field,
    NC_NOTNC4_inq_compound_fieldindex,
    NC_NOTNC4_def_vlen,
    NC_NOTNC4_put_vlen_element,
    NC_NOTNC4_get_vlen_element,
    NC_NOTNC4_def_enum,
    NC_NOTNC4_insert_enum,
    NC_NOTNC4_inq_enum_member,
    NC_NOTNC4_inq_enum_ident,
    NC_NOTNC4_def_opaque,
    NC_NOTNC4_def_var_deflate,
    NC_NOTNC4_def_var_fletcher32,
    NC_NOTNC4_def_var_chunking,
    NC_NOTNC4_def_var_endian,
    NC_NOTNC4_def_var_filter,
    NC_NOTNC4_set_var_chunk_cache,
    NC_NOTNC4_get_var_chunk_cache,
#if NC_DISPATCH_VERSION >= 3
    NC_NOOP_inq_var_filter_ids,
    NC_NOOP_inq_var_filter_info,
#endif
#if NC_DISPATCH_VERSION >= 4
    NC_NOTNC4_def_var_quantize,
    NC_NOTNC4_inq_var_quantize,
#endif
#if NC_DISPATCH_VERSION >= 5
    NC_NOOP_inq_filter_avail,
#endif
};

/* Self-registration init function: returns NC_Dispatch* instead of int.
 * This is the new convention used when HAVE_NETCDF_UDF_SELF_REGISTRATION
 * is defined. The plugin loader must call this through an
 * NC_Dispatch* (*)(void) function pointer and register the returned
 * table via nc_def_user_format(). */
EXTERNL NC_Dispatch*
tst_udf_self_load_init(void)
{
    return &tst_self_load_dispatcher;
}
