/* This is part of the netCDF package. Copyright 2005-2018 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test user-defined formats..

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

#define FILE_NAME "tst_udf.nc"

#ifdef _MSC_VER
static int
NC4_no_show_metadata(int ncid)
{
    return NC_NOERR;
}

#define NC4_show_metadata NC4_no_show_metadata
#endif

int
tst_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
         void *parameters, const NC_Dispatch *dispatch, int ncid)
{
    return NC_NOERR;
}

int
tst_abort(int ncid)
{
    return TEST_VAL_42;
}

int
tst_close(int ncid, void *v)
{
    return NC_NOERR;
}

int
tst_inq_format(int ncid, int *formatp)
{
    return TEST_VAL_42;
}

int
tst_inq_format_extended(int ncid, int *formatp, int *modep)
{
    return TEST_VAL_42;
}

int
tst_get_vara(int ncid, int varid, const size_t *start, const size_t *count,
             void *value, nc_type t)
{
    return TEST_VAL_42;
}

/* Dispatch tables initialized at runtime in init_dispatchers() because
 * MSVC cannot use dllimport function addresses as static initializers
 * (error C2099: "initializer is not a constant"). */
static NC_Dispatch tst_dispatcher;
static NC_Dispatch tst_dispatcher_bad_version;

/* Populate both dispatch tables at runtime. Must be called once
 * before any test code references tst_dispatcher or
 * tst_dispatcher_bad_version. */
static void
init_dispatchers(void)
{
    /* --- tst_dispatcher --- */
    memset(&tst_dispatcher, 0, sizeof(tst_dispatcher));

    tst_dispatcher.model = NC_FORMATX_UDF0;
    tst_dispatcher.dispatch_version = NC_DISPATCH_VERSION;

    tst_dispatcher.create = NC_RO_create;
    tst_dispatcher.open = tst_open;

    tst_dispatcher.redef = NC_RO_redef;
    tst_dispatcher._enddef = NC_RO__enddef;
    tst_dispatcher.sync = NC_RO_sync;
    tst_dispatcher.abort = tst_abort;
    tst_dispatcher.close = tst_close;
    tst_dispatcher.set_fill = NC_RO_set_fill;
    tst_dispatcher.inq_format = tst_inq_format;
    tst_dispatcher.inq_format_extended = tst_inq_format_extended;

    tst_dispatcher.inq = NC4_inq;
    tst_dispatcher.inq_type = NC4_inq_type;

    tst_dispatcher.def_dim = NC_RO_def_dim;
    tst_dispatcher.inq_dimid = NC4_inq_dimid;
    tst_dispatcher.inq_dim = HDF5_inq_dim;
    tst_dispatcher.inq_unlimdim = NC4_inq_unlimdim;
    tst_dispatcher.rename_dim = NC_RO_rename_dim;

    tst_dispatcher.inq_att = NC4_inq_att;
    tst_dispatcher.inq_attid = NC4_inq_attid;
    tst_dispatcher.inq_attname = NC4_inq_attname;
    tst_dispatcher.rename_att = NC_RO_rename_att;
    tst_dispatcher.del_att = NC_RO_del_att;
    tst_dispatcher.get_att = NC4_get_att;
    tst_dispatcher.put_att = NC_RO_put_att;

    tst_dispatcher.def_var = NC_RO_def_var;
    tst_dispatcher.inq_varid = NC4_inq_varid;
    tst_dispatcher.rename_var = NC_RO_rename_var;
    tst_dispatcher.get_vara = tst_get_vara;
    tst_dispatcher.put_vara = NC_RO_put_vara;
    tst_dispatcher.get_vars = NCDEFAULT_get_vars;
    tst_dispatcher.put_vars = NCDEFAULT_put_vars;
    tst_dispatcher.get_varm = NCDEFAULT_get_varm;
    tst_dispatcher.put_varm = NCDEFAULT_put_varm;

    tst_dispatcher.inq_var_all = NC4_inq_var_all;

    tst_dispatcher.var_par_access = NC_NOTNC4_var_par_access;
    tst_dispatcher.def_var_fill = NC_RO_def_var_fill;

    tst_dispatcher.show_metadata = NC4_show_metadata;
    tst_dispatcher.inq_unlimdims = NC4_inq_unlimdims;

    tst_dispatcher.inq_ncid = NC4_inq_ncid;
    tst_dispatcher.inq_grps = NC4_inq_grps;
    tst_dispatcher.inq_grpname = NC4_inq_grpname;
    tst_dispatcher.inq_grpname_full = NC4_inq_grpname_full;
    tst_dispatcher.inq_grp_parent = NC4_inq_grp_parent;
    tst_dispatcher.inq_grp_full_ncid = NC4_inq_grp_full_ncid;
    tst_dispatcher.inq_varids = NC4_inq_varids;
    tst_dispatcher.inq_dimids = NC4_inq_dimids;
    tst_dispatcher.inq_typeids = NC4_inq_typeids;
    tst_dispatcher.inq_type_equal = NC4_inq_type_equal;
    tst_dispatcher.def_grp = NC_NOTNC4_def_grp;
    tst_dispatcher.rename_grp = NC_NOTNC4_rename_grp;
    tst_dispatcher.inq_user_type = NC4_inq_user_type;
    tst_dispatcher.inq_typeid = NC4_inq_typeid;

    tst_dispatcher.def_compound = NC_NOTNC4_def_compound;
    tst_dispatcher.insert_compound = NC_NOTNC4_insert_compound;
    tst_dispatcher.insert_array_compound = NC_NOTNC4_insert_array_compound;
    tst_dispatcher.inq_compound_field = NC_NOTNC4_inq_compound_field;
    tst_dispatcher.inq_compound_fieldindex = NC_NOTNC4_inq_compound_fieldindex;
    tst_dispatcher.def_vlen = NC_NOTNC4_def_vlen;
    tst_dispatcher.put_vlen_element = NC_NOTNC4_put_vlen_element;
    tst_dispatcher.get_vlen_element = NC_NOTNC4_get_vlen_element;
    tst_dispatcher.def_enum = NC_NOTNC4_def_enum;
    tst_dispatcher.insert_enum = NC_NOTNC4_insert_enum;
    tst_dispatcher.inq_enum_member = NC_NOTNC4_inq_enum_member;
    tst_dispatcher.inq_enum_ident = NC_NOTNC4_inq_enum_ident;
    tst_dispatcher.def_opaque = NC_NOTNC4_def_opaque;
    tst_dispatcher.def_var_deflate = NC_NOTNC4_def_var_deflate;
    tst_dispatcher.def_var_fletcher32 = NC_NOTNC4_def_var_fletcher32;
    tst_dispatcher.def_var_chunking = NC_NOTNC4_def_var_chunking;
    tst_dispatcher.def_var_endian = NC_NOTNC4_def_var_endian;
    tst_dispatcher.def_var_filter = NC_NOTNC4_def_var_filter;
    tst_dispatcher.set_var_chunk_cache = NC_NOTNC4_set_var_chunk_cache;
    tst_dispatcher.get_var_chunk_cache = NC_NOTNC4_get_var_chunk_cache;
#if NC_DISPATCH_VERSION >= 3
    tst_dispatcher.inq_var_filter_ids = NC_NOOP_inq_var_filter_ids;
    tst_dispatcher.inq_var_filter_info = NC_NOOP_inq_var_filter_info;
#endif
#if NC_DISPATCH_VERSION >= 4
    tst_dispatcher.def_var_quantize = NC_NOTNC4_def_var_quantize;
    tst_dispatcher.inq_var_quantize = NC_NOTNC4_inq_var_quantize;
#endif
#if NC_DISPATCH_VERSION >= 5
    tst_dispatcher.inq_filter_avail = NC_NOOP_inq_filter_avail;
#endif

    /* --- tst_dispatcher_bad_version (same but wrong ABI version) --- */
    memcpy(&tst_dispatcher_bad_version, &tst_dispatcher,
           sizeof(tst_dispatcher));
    tst_dispatcher_bad_version.dispatch_version = NC_DISPATCH_VERSION - 1;
}

/* Simulate a self-registration init function that returns NC_Dispatch*. */
static NC_Dispatch*
tst_self_reg_init(void)
{
    return &tst_dispatcher;
}

/* Simulate a self-registration init function that fails (returns NULL). */
static NC_Dispatch*
tst_self_reg_init_null(void)
{
    return NULL;
}

/* Simulate a self-registration init function returning a bad ABI version. */
static NC_Dispatch*
tst_self_reg_init_bad_ver(void)
{
    return &tst_dispatcher_bad_version;
}

#define NUM_UDFS 2

int mode[NUM_UDFS] = {NC_UDF0, NC_UDF1};

int
main(int argc, char **argv)
{
    init_dispatchers();
    printf("\n*** Testing user-defined formats.\n");
    printf("*** testing simple user-defined format...");
    {
        int ncid;
        NC_Dispatch *disp_in;
        int i;

        /* Create an empty file to play with. */
        if (nc_create(FILE_NAME, 0, &ncid)) ERR;
        if (nc_close(ncid)) ERR;

        /* Test all available user-defined format slots. */
        for (i = 0; i < NUM_UDFS; i++)
        {
            /* Add our user defined format. */
            if (nc_def_user_format(mode[i], &tst_dispatcher, NULL)) ERR;

            /* Check that our user-defined format has been added. */
            if (nc_inq_user_format(mode[i], &disp_in, NULL)) ERR;
            if (disp_in != &tst_dispatcher) ERR;

            /* Open file with our defined functions. */
            if (nc_open(FILE_NAME, mode[i], &ncid)) ERR;
            if (nc_close(ncid)) ERR;

            /* Open file again and abort, which is the same as closing
             * it. This also tests that the UDF flags are given
             * priority. If NC_NETCDF4 flag were given priority, then
             * nc_abort() will not return TEST_VAL_42, but instead will
             * return 0. */
            if (nc_open(FILE_NAME, mode[i], &ncid)) ERR;
            if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
            if (nc_inq_format_extended(ncid, NULL, NULL) != TEST_VAL_42) ERR;
            if (nc_abort(ncid) != TEST_VAL_42) ERR;
        }
    }
    SUMMARIZE_ERR;
    printf("*** testing user-defined format with magic number...");
    {
        int ncid;
        NC_Dispatch *disp_in;
        char magic_number[5] = "1111";
        char dummy_data[11] = "0123456789";
        char magic_number_in[NC_MAX_MAGIC_NUMBER_LEN];
        FILE *FP;
        int i;

        /* Create a file with magic number at start. */
        if (!(FP = NCfopen(FILE_NAME, "w"))) ERR;
        if (fwrite(magic_number, sizeof(char), strlen(magic_number), FP)
            != strlen(magic_number)) ERR;
        if (fwrite(dummy_data, sizeof(char), strlen(dummy_data), FP)
            != strlen(dummy_data)) ERR;
        if (fclose(FP)) ERR;

        /* Test all available user-defined format slots. */
        for (i = 0; i < NUM_UDFS; i++)
        {
            /* Add our test user defined format. */
            mode[i] = mode[i]|NC_NETCDF4;
            if (nc_def_user_format(mode[i], &tst_dispatcher, magic_number)) ERR;

            /* Check that our user-defined format has been added. */
            if (nc_inq_user_format(mode[i], &disp_in, magic_number_in)) ERR;
            if (disp_in != &tst_dispatcher) ERR;
            if (strncmp(magic_number, magic_number_in, strlen(magic_number))) ERR;

            /* Open file with our defined functions. */
            if (nc_open(FILE_NAME, mode[i], &ncid)) ERR;
            if (nc_close(ncid)) ERR;

            /* Open file again and abort, which is the same as closing
             * it. This time we don't specify a mode, because the magic
             * number is used to identify the file. */
            if (nc_open(FILE_NAME, 0, &ncid)) ERR;
            if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
            if (nc_inq_format_extended(ncid, NULL, NULL) != TEST_VAL_42) ERR;
            if (nc_abort(ncid) != TEST_VAL_42) ERR;
        }
    }
    SUMMARIZE_ERR;
    printf("*** testing bad version causes dispatch table to be rejected...");
    {
        int i;
        char magic_number[5] = "1111";

        /* Test all available user-defined format slots. */
        for (i = 0; i < NUM_UDFS; i++)
        {
            /* Make sure our bad version format is rejected. */
            if (nc_def_user_format(mode[i], &tst_dispatcher_bad_version,
                                   NULL) != NC_EINVAL) ERR;
            /* Make sure defining a magic number with netcdf3 is rejected. */
            if (nc_def_user_format(NC_CLASSIC_MODEL, &tst_dispatcher, 
                                   magic_number) != NC_EINVAL) ERR;
        }
    }
    SUMMARIZE_ERR;
    printf("*** testing self-registration init pattern (returns NC_Dispatch*)...");
    {
        /* Simulate the self-registration pattern where the plugin init
         * function returns NC_Dispatch* instead of int. This is the
         * pattern used when HAVE_NETCDF_UDF_SELF_REGISTRATION is
         * defined. The loader must call through an NC_Dispatch*-returning
         * function pointer and register the table itself. */
        NC_Dispatch* (*init_returning_dispatch)(void);
        NC_Dispatch* table;
        NC_Dispatch* disp_in;
        int i;

        /* Point our function-pointer at a helper that returns the
         * dispatch table pointer (simulating a plugin init). */
        init_returning_dispatch = (NC_Dispatch* (*)(void))(void*)tst_self_reg_init;
        table = init_returning_dispatch();

        /* The returned pointer must not be NULL. */
        if (!table) ERR;

        /* The returned table must have the correct ABI version. */
        if (table->dispatch_version != NC_DISPATCH_VERSION) ERR;

        /* Register it via nc_def_user_format, as the fixed loader does. */
        for (i = 0; i < NUM_UDFS; i++)
        {
            if (nc_def_user_format(mode[i], table, NULL)) ERR;

            /* Verify it was registered. */
            if (nc_inq_user_format(mode[i], &disp_in, NULL)) ERR;
            if (disp_in != table) ERR;
        }

        /* Test NULL-returning init (simulates plugin failure). */
        {
            NC_Dispatch* (*null_init)(void);
            NC_Dispatch* bad_table;

            null_init = (NC_Dispatch* (*)(void))(void*)tst_self_reg_init_null;
            bad_table = null_init();
            if (bad_table != NULL) ERR;
        }

        /* Test bad ABI version via self-registration path. */
        {
            NC_Dispatch* (*bad_init)(void);
            NC_Dispatch* bad_table;

            bad_init = (NC_Dispatch* (*)(void))(void*)tst_self_reg_init_bad_ver;
            bad_table = bad_init();
            if (!bad_table) ERR;
            if (bad_table->dispatch_version == NC_DISPATCH_VERSION) ERR;

            /* nc_def_user_format should reject a bad version table. */
            if (nc_def_user_format(NC_UDF0, bad_table, NULL) != NC_EINVAL) ERR;
        }
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
