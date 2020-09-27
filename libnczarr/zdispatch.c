/* Copyright 2005-2018 University Corporation for Atmospheric
   Research/Unidata. */

/**
 * @file
 * @internal This header file contains prototypes and initialization
 * for the ZARR dispatch layer.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#include "zincludes.h"

/* Forward */
static int NCZ_var_par_access(int ncid, int varid, int par_access);
static int NCZ_show_metadata(int ncid);


static const NC_Dispatch NCZ_dispatcher = {

    NC_FORMATX_NCZARR,
    NC_DISPATCH_VERSION,

    NCZ_create,
    NCZ_open,

    NCZ_redef,
    NCZ__enddef,
    NCZ_sync,
    NCZ_abort,
    NCZ_close,
    NCZ_set_fill,
    NC4_inq_format,
    NCZ_inq_format_extended,

    NCZ_inq,
    NC4_inq_type,

    NCZ_def_dim,
    NC4_inq_dimid,
    NCZ_inq_dim,
    NC4_inq_unlimdim,
    NCZ_rename_dim,

    NCZ_inq_att,
    NCZ_inq_attid,
    NC4_inq_attname,
    NCZ_rename_att,
    NCZ_del_att,
    NCZ_get_att,
    NCZ_put_att,

    NCZ_def_var,
    NC4_inq_varid,
    NCZ_rename_var,
    NCZ_get_vara,
    NCZ_put_vara,
    NCZ_get_vars,
    NCZ_put_vars,
    NCDEFAULT_get_varm,
    NCDEFAULT_put_varm,

    NCZ_inq_var_all,

    NCZ_var_par_access,
    NCZ_def_var_fill,

    NCZ_show_metadata,
    NCZ_inq_unlimdims,

    NC4_inq_ncid,
    NC4_inq_grps,
    NC4_inq_grpname,
    NC4_inq_grpname_full,
    NC4_inq_grp_parent,
    NC4_inq_grp_full_ncid,
    NC4_inq_varids,
    NC4_inq_dimids,
    NC4_inq_typeids,
    NCZ_inq_type_equal,
    NCZ_def_grp,
    NCZ_rename_grp,
    NC4_inq_user_type,
    NC4_inq_typeid,

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
    NCZ_def_var_chunking,
    NCZ_def_var_endian,
    NC_NOTNC4_def_var_filter,
    NCZ_set_var_chunk_cache,
    NC4_get_var_chunk_cache,
    NCZ_inq_var_filter_ids,
    NCZ_inq_var_filter_info,
};

const NC_Dispatch* NCZ_dispatch_table = NULL; /* moved here from ddispatch.c */

/**
 * @internal Initialize the ZARR dispatch layer.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int ncz_initialized = 0; /**< True if initialization has happened. */

int
NCZ_initialize(void)
{
    int stat;
    NCZ_dispatch_table = &NCZ_dispatcher;
    if (!ncz_initialized)
        NCZ_initialize_internal();
    stat = NCZ_provenance_init();
    if(stat) ncz_initialized = 1;
    return stat;
}

/**
 * @internal Finalize the ZARR dispatch layer.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
NCZ_finalize(void)
{
    NCZ_finalize_internal();
    NCZ_provenance_finalize();
    return NC_NOERR;
}

static int
NCZ_var_par_access(int ncid, int varid, int par_access)
{
    return NC_NOERR; /* no-op */
}

static int
NCZ_show_metadata(int ncid)
{
    return NC_NOERR;
}

