/* This is part of the netCDF package. Copyright 2026 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Minimal self-registering UDF0 plugin for tst_udf_format.sh. It
   provides an empty root dataset and reports NC_FORMATX_UDF0 through
   both format inquiry functions.

   This plugin reproduces netCDF-C issue #3411 without depending on
   an external UDF implementation. Before the fix, ncdump could read
   the UDF dataset but rejected extended format code 8 while generating
   its virtual _Format attribute or processing -K output.

   Author: Edward Hartnett, 2026-07-21
*/

#include "config.h"
#include <string.h>
#include "netcdf.h"
#include "netcdf_dispatch.h"

static NC_Dispatch dispatcher;

static int
plugin_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
            void *parameters, const NC_Dispatch *table, int ncid)
{
    return NC_NOERR;
}

static int
plugin_close(int ncid, void *dispatchdata)
{
    return NC_NOERR;
}

static int
plugin_inq_format(int ncid, int *formatp)
{
    if (formatp)
        *formatp = NC_FORMATX_UDF0;
    return NC_NOERR;
}

static int
plugin_inq_format_extended(int ncid, int *formatp, int *modep)
{
    if (formatp)
        *formatp = NC_FORMATX_UDF0;
    if (modep)
        *modep = NC_NOWRITE;
    return NC_NOERR;
}

static int
plugin_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp)
{
    if (ndimsp)
        *ndimsp = 0;
    if (nvarsp)
        *nvarsp = 0;
    if (nattsp)
        *nattsp = 0;
    if (unlimdimidp)
        *unlimdimidp = -1;
    return NC_NOERR;
}

static int
plugin_inq_att(int ncid, int varid, const char *name, nc_type *xtypep,
               size_t *lenp)
{
    return NC_ENOTATT;
}

static int
plugin_inq_typeids(int ncid, int *ntypes, int *typeids)
{
    if (ntypes)
        *ntypes = 0;
    return NC_NOERR;
}

static int
plugin_inq_grps(int ncid, int *numgrps, int *ncids)
{
    if (numgrps)
        *numgrps = 0;
    return NC_NOERR;
}

static int
plugin_inq_grp_parent(int ncid, int *parent_ncid)
{
    return NC_ENOGRP;
}

static int
plugin_inq_ncid(int ncid, const char *name, int *grp_ncid)
{
    if (name != NULL)
        return NC_ENOGRP;
    if (grp_ncid)
        *grp_ncid = ncid;
    return NC_NOERR;
}

static int
plugin_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents)
{
    if (ndims)
        *ndims = 0;
    return NC_NOERR;
}

static int
plugin_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp)
{
    if (nunlimdimsp)
        *nunlimdimsp = 0;
    return NC_NOERR;
}

#if defined(_MSC_VER)
#define PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define PLUGIN_EXPORT
#endif

PLUGIN_EXPORT NC_Dispatch *
tst_udf_format_init(void)
{
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.model = NC_FORMATX_UDF0;
    dispatcher.dispatch_version = NC_DISPATCH_VERSION;
    dispatcher.create = NC_RO_create;
    dispatcher.open = plugin_open;
    dispatcher.redef = NC_RO_redef;
    dispatcher._enddef = NC_RO__enddef;
    dispatcher.sync = NC_RO_sync;
    dispatcher.close = plugin_close;
    dispatcher.set_fill = NC_RO_set_fill;
    dispatcher.inq_format = plugin_inq_format;
    dispatcher.inq_format_extended = plugin_inq_format_extended;
    dispatcher.inq = plugin_inq;
    dispatcher.inq_att = plugin_inq_att;
    dispatcher.inq_grps = plugin_inq_grps;
    dispatcher.inq_ncid = plugin_inq_ncid;
    dispatcher.inq_grp_parent = plugin_inq_grp_parent;
    dispatcher.inq_dimids = plugin_inq_dimids;
    dispatcher.inq_unlimdims = plugin_inq_unlimdims;
    dispatcher.inq_typeids = plugin_inq_typeids;
    dispatcher.rename_att = NC_RO_rename_att;
    dispatcher.del_att = NC_RO_del_att;
    dispatcher.put_att = NC_RO_put_att;
    dispatcher.def_dim = NC_RO_def_dim;
    dispatcher.rename_dim = NC_RO_rename_dim;
    dispatcher.def_var = NC_RO_def_var;
    dispatcher.rename_var = NC_RO_rename_var;
    dispatcher.put_vara = NC_RO_put_vara;
    dispatcher.var_par_access = NC_NOTNC4_var_par_access;
    dispatcher.def_var_fill = NC_RO_def_var_fill;
    dispatcher.inq_var_filter_ids = NC_NOOP_inq_var_filter_ids;
    dispatcher.inq_var_filter_info = NC_NOOP_inq_var_filter_info;
    dispatcher.inq_filter_avail = NC_NOOP_inq_filter_avail;
    return &dispatcher;
}
