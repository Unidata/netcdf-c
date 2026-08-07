/* This is part of the netCDF package. Copyright 2005-2026 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test that all 10 UDF (User-Defined Format) slots can be used
   simultaneously. This test verifies the fix for GitHub issue #3372,
   where UDF code only allowed 2 UDF layers instead of 10.

   Ed Hartnett
*/

#include "config.h"
#include <string.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "nc4dispatch.h"
#include "hdf5dispatch.h"
#include "netcdf_dispatch.h"

#define FILE_NAME "tst_udf_multi.nc"

/* Each UDF slot gets a unique return code for identification */
#define UDF0_RETURN 100
#define UDF1_RETURN 101
#define UDF2_RETURN 102
#define UDF3_RETURN 103
#define UDF4_RETURN 104
#define UDF5_RETURN 105
#define UDF6_RETURN 106
#define UDF7_RETURN 107
#define UDF8_RETURN 108
#define UDF9_RETURN 109

/* Mode flags for each UDF */
int udf_modes[NC_MAX_UDF_FORMATS] = {
    NC_UDF0, NC_UDF1, NC_UDF2, NC_UDF3, NC_UDF4,
    NC_UDF5, NC_UDF6, NC_UDF7, NC_UDF8, NC_UDF9
};

/* Expected return codes from each UDF's inq_format function */
int udf_returns[NC_MAX_UDF_FORMATS] = {
    UDF0_RETURN, UDF1_RETURN, UDF2_RETURN, UDF3_RETURN, UDF4_RETURN,
    UDF5_RETURN, UDF6_RETURN, UDF7_RETURN, UDF8_RETURN, UDF9_RETURN
};

/* 10 dispatch tables, one for each UDF slot */
static NC_Dispatch udf_dispatchers[NC_MAX_UDF_FORMATS];

/* Simple open function that always succeeds */
static int
tst_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
         void *parameters, const NC_Dispatch *dispatch, int ncid)
{
    return NC_NOERR;
}

/* Close function */
static int
tst_close(int ncid, void *v)
{
    return NC_NOERR;
}

/* Dummy get_vara function */
static int
tst_get_vara(int ncid, int varid, const size_t *start, const size_t *count,
             void *value, nc_type t)
{
    return NC_NOERR;
}

/* inq_format returns unique code per UDF for identification */
static int
tst_inq_format_udf0(int ncid, int *formatp)
{
    return UDF0_RETURN;
}
static int
tst_inq_format_udf1(int ncid, int *formatp)
{
    return UDF1_RETURN;
}
static int
tst_inq_format_udf2(int ncid, int *formatp)
{
    return UDF2_RETURN;
}
static int
tst_inq_format_udf3(int ncid, int *formatp)
{
    return UDF3_RETURN;
}
static int
tst_inq_format_udf4(int ncid, int *formatp)
{
    return UDF4_RETURN;
}
static int
tst_inq_format_udf5(int ncid, int *formatp)
{
    return UDF5_RETURN;
}
static int
tst_inq_format_udf6(int ncid, int *formatp)
{
    return UDF6_RETURN;
}
static int
tst_inq_format_udf7(int ncid, int *formatp)
{
    return UDF7_RETURN;
}
static int
tst_inq_format_udf8(int ncid, int *formatp)
{
    return UDF8_RETURN;
}
static int
tst_inq_format_udf9(int ncid, int *formatp)
{
    return UDF9_RETURN;
}

/* Array of function pointers for each UDF's inq_format */
typedef int (*inq_format_func)(int, int*);
static inq_format_func udf_inq_formats[NC_MAX_UDF_FORMATS] = {
    tst_inq_format_udf0, tst_inq_format_udf1, tst_inq_format_udf2,
    tst_inq_format_udf3, tst_inq_format_udf4, tst_inq_format_udf5,
    tst_inq_format_udf6, tst_inq_format_udf7, tst_inq_format_udf8,
    tst_inq_format_udf9
};

#ifdef _MSC_VER
static int
NC4_no_show_metadata(int ncid)
{
    return NC_NOERR;
}
#define NC4_show_metadata NC4_no_show_metadata
#endif


/* Initialize all 10 dispatch tables */
static void
init_dispatchers(void)
{
    int i;

    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        NC_Dispatch *dsp = &udf_dispatchers[i];

        memset(dsp, 0, sizeof(NC_Dispatch));

        /* UDF0 and UDF1 use NC_FORMATX_UDF0 and NC_FORMATX_UDF1
         * UDF2-UDF9 start at NC_FORMATX_UDF2 */
        if (i <= 1)
            dsp->model = NC_FORMATX_UDF0 + i;
        else
            dsp->model = NC_FORMATX_UDF2 + i - 2;

        dsp->dispatch_version = NC_DISPATCH_VERSION;
        dsp->create = NC_RO_create;
        dsp->open = tst_open;
        dsp->redef = NC_RO_redef;
        dsp->_enddef = NC_RO__enddef;
        dsp->sync = NC_RO_sync;
        dsp->abort = NC4_abort;
        dsp->close = tst_close;
        dsp->set_fill = NC_RO_set_fill;
        dsp->inq_format = udf_inq_formats[i];
        dsp->inq_format_extended = NC4_inq_format_extended;
        dsp->inq = NC4_inq;
        dsp->inq_type = NC4_inq_type;
        dsp->def_dim = NC_RO_def_dim;
        dsp->inq_dimid = NC4_inq_dimid;
        dsp->inq_dim = HDF5_inq_dim;
        dsp->inq_unlimdim = NC4_inq_unlimdim;
        dsp->rename_dim = NC_RO_rename_dim;
        dsp->inq_att = NC4_inq_att;
        dsp->inq_attid = NC4_inq_attid;
        dsp->inq_attname = NC4_inq_attname;
        dsp->rename_att = NC_RO_rename_att;
        dsp->del_att = NC_RO_del_att;
        dsp->get_att = NC4_get_att;
        dsp->put_att = NC_RO_put_att;
        dsp->def_var = NC_RO_def_var;
        dsp->inq_varid = NC4_inq_varid;
        dsp->rename_var = NC_RO_rename_var;
        dsp->get_vara = tst_get_vara;
        dsp->put_vara = NC_RO_put_vara;
        dsp->get_vars = NCDEFAULT_get_vars;
        dsp->put_vars = NCDEFAULT_put_vars;
        dsp->get_varm = NCDEFAULT_get_varm;
        dsp->put_varm = NCDEFAULT_put_varm;
        dsp->inq_var_all = NC4_inq_var_all;
        dsp->var_par_access = NC_NOTNC4_var_par_access;
        dsp->def_var_fill = NC_RO_def_var_fill;
        dsp->show_metadata = NC4_show_metadata;
        dsp->inq_unlimdims = NC4_inq_unlimdims;
        dsp->inq_ncid = NC4_inq_ncid;
        dsp->inq_grps = NC4_inq_grps;
        dsp->inq_grpname = NC4_inq_grpname;
        dsp->inq_grpname_full = NC4_inq_grpname_full;
        dsp->inq_grp_parent = NC4_inq_grp_parent;
        dsp->inq_grp_full_ncid = NC4_inq_grp_full_ncid;
        dsp->inq_varids = NC4_inq_varids;
        dsp->inq_dimids = NC4_inq_dimids;
        dsp->inq_typeids = NC4_inq_typeids;
        dsp->inq_type_equal = NC4_inq_type_equal;
        dsp->def_grp = NC_NOTNC4_def_grp;
        dsp->rename_grp = NC_NOTNC4_rename_grp;
        dsp->inq_user_type = NC4_inq_user_type;
        dsp->inq_typeid = NC4_inq_typeid;
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

    init_dispatchers();

    printf("\n*** Testing all 10 UDF slots can be used simultaneously.\n");

    printf("*** testing all 10 UDF slots can be registered...");
    {
        /* Create an empty file to play with */
        if (nc_create(FILE_NAME, 0, &ncid)) ERR;
        if (nc_close(ncid)) ERR;

        /* Register all 10 UDF formats */
        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            if (nc_def_user_format(udf_modes[i], &udf_dispatchers[i], NULL)) ERR;
        }

        /* Verify all 10 were registered correctly */
        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            NC_Dispatch *disp_in;
            if (nc_inq_user_format(udf_modes[i], &disp_in, NULL)) ERR;
            if (disp_in != &udf_dispatchers[i]) ERR;
        }
    }
    SUMMARIZE_ERR;

    printf("*** testing each UDF slot dispatches correctly...");
    {
        /* Test that each UDF slot dispatches to the correct handler.
         * We open the file with each UDF mode and check that
         * nc_inq_format returns the expected unique code. */
        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            int ret;

            if (nc_open(FILE_NAME, udf_modes[i], &ncid)) ERR;

            /* The inq_format function should return the unique code
             * for this UDF slot */
            ret = nc_inq_format(ncid, NULL);
            if (ret != udf_returns[i]) {
                fprintf(stderr, "UDF%d: expected %d, got %d\n", i, udf_returns[i], ret);
                ERR;
            }

            if (nc_close(ncid)) ERR;
        }
    }
    SUMMARIZE_ERR;

    printf("*** testing UDF slots are independent...");
    {
        /* Re-verify that all dispatch tables are still correctly registered
         * after using them */
        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            NC_Dispatch *disp_in;
            if (nc_inq_user_format(udf_modes[i], &disp_in, NULL)) ERR;
            if (disp_in != &udf_dispatchers[i]) ERR;
            if (disp_in->dispatch_version != NC_DISPATCH_VERSION) ERR;
        }
    }
    SUMMARIZE_ERR;

    printf("*** testing UDF mode flags don't collide with other flags...");
    {
        /* Verify that NC_UDF2 through NC_UDF9 don't share bits with
         * important mode flags like NC_NOATTCREORD (0x20000) and
         * NC_NODIMSCALE_ATTACH (0x40000). This was the root cause of
         * GitHub issue #3372. */

        /* These flags should not overlap with UDF flags */
        int critical_flags = NC_NOATTCREORD | NC_NODIMSCALE_ATTACH;

        for (i = 2; i < NC_MAX_UDF_FORMATS; i++) {
            /* UDF2+ should not share bits with critical flags */
            if (udf_modes[i] & critical_flags) {
                fprintf(stderr, "UDF%d (0x%x) collides with critical flags\n",
                        i, udf_modes[i]);
                ERR;
            }
        }

        /* Also verify no two UDF flags share bits */
        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            int j;
            for (j = i + 1; j < NC_MAX_UDF_FORMATS; j++) {
                if (udf_modes[i] & udf_modes[j]) {
                    fprintf(stderr, "UDF%d (0x%x) and UDF%d (0x%x) share bits\n",
                            i, udf_modes[i], j, udf_modes[j]);
                    ERR;
                }
            }
        }
    }
    SUMMARIZE_ERR;

    FINAL_RESULTS;
}
