/* This is part of the netCDF package. Copyright 2005-2026 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test that all NC_MAX_UDF_FORMATS UDF (User-Defined Format) slots
   can be used simultaneously. This test originally verified the fix
   for GitHub issue #3372, where UDF code only allowed 2 UDF layers;
   it now also verifies the flag-plus-number mode encoding that
   provides 64 slots.

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

/* Each UDF slot gets a unique return code for identification:
 * UDF_RETURN_BASE + slot number. */
#define UDF_RETURN_BASE 100

/* Mode flags for each UDF; filled in by init_dispatchers() */
int udf_modes[NC_MAX_UDF_FORMATS];

/* Expected return codes from each UDF's inq_format function;
 * filled in by init_dispatchers() */
int udf_returns[NC_MAX_UDF_FORMATS];

/* One dispatch table for each UDF slot */
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

/* inq_format returns unique code per UDF for identification.
 * One function is generated per slot with a macro. */
#define DEFINE_INQ_FORMAT(N)                        \
    static int                                      \
    tst_inq_format_udf##N(int ncid, int *formatp)   \
    {                                               \
        return UDF_RETURN_BASE + N;                 \
    }

#define UDF_SLOT_LIST(X)                                        \
    X(0)  X(1)  X(2)  X(3)  X(4)  X(5)  X(6)  X(7)              \
    X(8)  X(9)  X(10) X(11) X(12) X(13) X(14) X(15)             \
    X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23)             \
    X(24) X(25) X(26) X(27) X(28) X(29) X(30) X(31)             \
    X(32) X(33) X(34) X(35) X(36) X(37) X(38) X(39)             \
    X(40) X(41) X(42) X(43) X(44) X(45) X(46) X(47)             \
    X(48) X(49) X(50) X(51) X(52) X(53) X(54) X(55)             \
    X(56) X(57) X(58) X(59) X(60) X(61) X(62) X(63)

UDF_SLOT_LIST(DEFINE_INQ_FORMAT)

/* Array of function pointers for each UDF's inq_format */
typedef int (*inq_format_func)(int, int*);
#define INQ_FORMAT_ENTRY(N) tst_inq_format_udf##N,
static inq_format_func udf_inq_formats[NC_MAX_UDF_FORMATS] = {
    UDF_SLOT_LIST(INQ_FORMAT_ENTRY)
};

#ifdef _MSC_VER
static int
NC4_no_show_metadata(int ncid)
{
    return NC_NOERR;
}
#define NC4_show_metadata NC4_no_show_metadata
#endif


/* Initialize all dispatch tables */
static void
init_dispatchers(void)
{
    int i;

    for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
        NC_Dispatch *dsp = &udf_dispatchers[i];

        memset(dsp, 0, sizeof(NC_Dispatch));

        udf_modes[i] = NC_UDF(i);
        udf_returns[i] = UDF_RETURN_BASE + i;
        dsp->model = NC_FORMATX_UDF(i);

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

    printf("\n*** Testing all %d UDF slots can be used simultaneously.\n",
           NC_MAX_UDF_FORMATS);

    printf("*** testing all %d UDF slots can be registered...", NC_MAX_UDF_FORMATS);
    {
        /* Create an empty file to play with */
        if (nc_create(FILE_NAME, 0, &ncid)) ERR;
        if (nc_close(ncid)) ERR;

        /* Register all UDF formats */
        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            if (nc_def_user_format(udf_modes[i], &udf_dispatchers[i], NULL)) ERR;
        }

        /* Verify all were registered correctly */
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

    printf("*** testing UDF mode encoding...");
    {
        /* Verify the flag-plus-number encoding. The slot number field
         * must not collide with mode flags that may legally combine
         * with a UDF open, such as NC_NOATTCREORD (0x20000) and
         * NC_NODIMSCALE_ATTACH (0x40000). Bit collisions with these
         * flags were the root cause of GitHub issue #3372. */
        int critical_flags = NC_NOATTCREORD | NC_NODIMSCALE_ATTACH | NC_NETCDF4;

        /* NC_UDF0 must keep its historic value. */
        if (NC_UDF0 != 0x0040) ERR;

        for (i = 0; i < NC_MAX_UDF_FORMATS; i++) {
            /* Every UDF mode must have the UDF flag set. */
            if (!(udf_modes[i] & NC_UDF_FLAG)) ERR;

            /* The slot number must round-trip through the field. */
            if (((udf_modes[i] >> NC_UDF_NUM_SHIFT) & NC_UDF_NUM_MASK) != i) {
                fprintf(stderr, "UDF%d (0x%x): slot number does not round-trip\n",
                        i, udf_modes[i]);
                ERR;
            }

            /* The encoding must not collide with combinable flags. */
            if (udf_modes[i] & critical_flags) {
                fprintf(stderr, "UDF%d (0x%x) collides with critical flags\n",
                        i, udf_modes[i]);
                ERR;
            }
        }
    }
    SUMMARIZE_ERR;

    FINAL_RESULTS;
}
