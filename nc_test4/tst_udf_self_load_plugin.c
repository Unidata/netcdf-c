/* This is part of the netCDF package. Copyright 2005-2025 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Minimal UDF plugin that uses the self-registration convention:
   the init function returns NC_Dispatch* instead of int.

   This plugin is built as a shared library and loaded by
   tst_udf_self_load via the RC-based plugin loader to verify
   that the loader correctly handles NC_Dispatch*-returning init
   functions.

   Note on dispatch table initialization: The NC_Dispatch struct is
   populated at runtime in tst_udf_self_load_init() rather than via a
   static initializer. This is required for MSVC compatibility.  On
   Windows, functions imported from a DLL (declared with
   __declspec(dllimport) via the EXTERNL/MSC_EXTRA macros) do not have
   compile-time-constant addresses, so MSVC rejects them in aggregate
   initializers with error C2099 ("initializer is not a constant").
   Assigning the function pointers at runtime avoids this limitation
   and works on all platforms.

   Ed Hartnett
*/

#include "config.h"
#include <string.h>
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

/* The dispatch table for this test plugin. Initialized at runtime
 * because MSVC cannot use dllimport function addresses as static
 * initializers (error C2099). */
static NC_Dispatch tst_self_load_dispatcher;
static int initialized = 0;

/* Export macro for the plugin's own symbols. EXTERNL cannot be used
 * here because it expands to __declspec(dllimport) when consuming the
 * netcdf DLL, and MSVC forbids defining a dllimport function (C2491).
 * The plugin must dllexport its init function instead. */
#if defined(_MSC_VER)
#  define PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#  define PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#  define PLUGIN_EXPORT
#endif

/* Self-registration init function: returns NC_Dispatch* instead of int.
 * This is the new convention used when HAVE_NETCDF_UDF_SELF_REGISTRATION
 * is defined. The plugin loader must call this through an
 * NC_Dispatch* (*)(void) function pointer and register the returned
 * table via nc_def_user_format(). */
PLUGIN_EXPORT NC_Dispatch*
tst_udf_self_load_init(void)
{
    if (!initialized) {
        memset(&tst_self_load_dispatcher, 0, sizeof(tst_self_load_dispatcher));

        tst_self_load_dispatcher.model = NC_FORMATX_UDF0;
        tst_self_load_dispatcher.dispatch_version = NC_DISPATCH_VERSION;

        tst_self_load_dispatcher.create = NC_RO_create;
        tst_self_load_dispatcher.open = plugin_open;

        tst_self_load_dispatcher.redef = NC_RO_redef;
        tst_self_load_dispatcher._enddef = NC_RO__enddef;
        tst_self_load_dispatcher.sync = NC_RO_sync;
        tst_self_load_dispatcher.abort = plugin_abort;
        tst_self_load_dispatcher.close = plugin_close;
        tst_self_load_dispatcher.set_fill = NC_RO_set_fill;
        tst_self_load_dispatcher.inq_format = plugin_inq_format;
        tst_self_load_dispatcher.inq_format_extended = plugin_inq_format_extended;

        tst_self_load_dispatcher.def_dim = NC_RO_def_dim;
        tst_self_load_dispatcher.rename_dim = NC_RO_rename_dim;

        tst_self_load_dispatcher.rename_att = NC_RO_rename_att;
        tst_self_load_dispatcher.del_att = NC_RO_del_att;
        tst_self_load_dispatcher.put_att = NC_RO_put_att;

        tst_self_load_dispatcher.def_var = NC_RO_def_var;
        tst_self_load_dispatcher.rename_var = NC_RO_rename_var;
        tst_self_load_dispatcher.put_vara = NC_RO_put_vara;

        tst_self_load_dispatcher.var_par_access = NC_NOTNC4_var_par_access;
        tst_self_load_dispatcher.def_var_fill = NC_RO_def_var_fill;

        tst_self_load_dispatcher.inq_ncid = NC_NOTNC4_inq_ncid;
        tst_self_load_dispatcher.inq_grps = NC_NOTNC4_inq_grps;
        tst_self_load_dispatcher.inq_grpname = NC_NOTNC4_inq_grpname;
        tst_self_load_dispatcher.inq_grpname_full = NC_NOTNC4_inq_grpname_full;
        tst_self_load_dispatcher.inq_grp_parent = NC_NOTNC4_inq_grp_parent;
        tst_self_load_dispatcher.inq_grp_full_ncid = NC_NOTNC4_inq_grp_full_ncid;
        tst_self_load_dispatcher.inq_varids = NC_NOTNC4_inq_varids;
        tst_self_load_dispatcher.inq_dimids = NC_NOTNC4_inq_dimids;
        tst_self_load_dispatcher.inq_typeids = NC_NOTNC4_inq_typeids;
        tst_self_load_dispatcher.def_grp = NC_NOTNC4_def_grp;
        tst_self_load_dispatcher.rename_grp = NC_NOTNC4_rename_grp;
        tst_self_load_dispatcher.inq_user_type = NC_NOTNC4_inq_user_type;
        tst_self_load_dispatcher.inq_typeid = NC_NOTNC4_inq_typeid;

        tst_self_load_dispatcher.def_compound = NC_NOTNC4_def_compound;
        tst_self_load_dispatcher.insert_compound = NC_NOTNC4_insert_compound;
        tst_self_load_dispatcher.insert_array_compound = NC_NOTNC4_insert_array_compound;
        tst_self_load_dispatcher.inq_compound_field = NC_NOTNC4_inq_compound_field;
        tst_self_load_dispatcher.inq_compound_fieldindex = NC_NOTNC4_inq_compound_fieldindex;
        tst_self_load_dispatcher.def_vlen = NC_NOTNC4_def_vlen;
        tst_self_load_dispatcher.put_vlen_element = NC_NOTNC4_put_vlen_element;
        tst_self_load_dispatcher.get_vlen_element = NC_NOTNC4_get_vlen_element;
        tst_self_load_dispatcher.def_enum = NC_NOTNC4_def_enum;
        tst_self_load_dispatcher.insert_enum = NC_NOTNC4_insert_enum;
        tst_self_load_dispatcher.inq_enum_member = NC_NOTNC4_inq_enum_member;
        tst_self_load_dispatcher.inq_enum_ident = NC_NOTNC4_inq_enum_ident;
        tst_self_load_dispatcher.def_opaque = NC_NOTNC4_def_opaque;
        tst_self_load_dispatcher.def_var_deflate = NC_NOTNC4_def_var_deflate;
        tst_self_load_dispatcher.def_var_fletcher32 = NC_NOTNC4_def_var_fletcher32;
        tst_self_load_dispatcher.def_var_chunking = NC_NOTNC4_def_var_chunking;
        tst_self_load_dispatcher.def_var_endian = NC_NOTNC4_def_var_endian;
        tst_self_load_dispatcher.def_var_filter = NC_NOTNC4_def_var_filter;
        tst_self_load_dispatcher.set_var_chunk_cache = NC_NOTNC4_set_var_chunk_cache;
        tst_self_load_dispatcher.get_var_chunk_cache = NC_NOTNC4_get_var_chunk_cache;
#if NC_DISPATCH_VERSION >= 3
        tst_self_load_dispatcher.inq_var_filter_ids = NC_NOOP_inq_var_filter_ids;
        tst_self_load_dispatcher.inq_var_filter_info = NC_NOOP_inq_var_filter_info;
#endif
#if NC_DISPATCH_VERSION >= 4
        tst_self_load_dispatcher.def_var_quantize = NC_NOTNC4_def_var_quantize;
        tst_self_load_dispatcher.inq_var_quantize = NC_NOTNC4_inq_var_quantize;
#endif
#if NC_DISPATCH_VERSION >= 5
        tst_self_load_dispatcher.inq_filter_avail = NC_NOOP_inq_filter_avail;
#endif
        initialized = 1;
    }
    return &tst_self_load_dispatcher;
}
