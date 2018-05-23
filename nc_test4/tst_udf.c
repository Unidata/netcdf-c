/* This is part of the netCDF package. Copyright 2005-2007 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test user-defined formats.

   Ed Hartnett
*/

#include "config.h"
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "nc4dispatch.h"

#define FILE_NAME "tst_udf.nc"

int
tst_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
             int use_parallel, void *parameters, NC_Dispatch *dispatch,
             NC *nc_file)
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

/* This is the dispatch object that holds pointers to all the
 * functions that make up the HDF4 dispatch interface. */
static NC_Dispatch tst_dispatcher = {

NC_FORMATX_UF0,

NC_RO_create,
tst_open,

NC_RO_redef,
NC_RO__enddef,
NC_RO_sync,
tst_abort,
tst_close,
NC_RO_set_fill,
NC_NOTNC3_inq_base_pe,
NC_NOTNC3_set_base_pe,
tst_inq_format,
tst_inq_format_extended,

NC4_inq,
NC4_inq_type,

NC_RO_def_dim,
NC4_inq_dimid,
NC4_inq_dim,
NC4_inq_unlimdim,
NC_RO_rename_dim,

NC4_inq_att,
NC4_inq_attid,
NC4_inq_attname,
NC_RO_rename_att,
NC_RO_del_att,
NC4_get_att,
NC_RO_put_att,

NC_RO_def_var,
NC4_inq_varid,
NC_RO_rename_var,
tst_get_vara,
NC_RO_put_vara,
NCDEFAULT_get_vars,
NCDEFAULT_put_vars,
NCDEFAULT_get_varm,
NCDEFAULT_put_varm,

NC4_inq_var_all,

NC_NOTNC4_var_par_access,
NC_RO_def_var_fill,

NC4_show_metadata,
NC4_inq_unlimdims,

NC4_inq_ncid,
NC4_inq_grps,
NC4_inq_grpname,
NC4_inq_grpname_full,
NC4_inq_grp_parent,
NC4_inq_grp_full_ncid,
NC4_inq_varids,
NC4_inq_dimids,
NC4_inq_typeids,
NC4_inq_type_equal,
NC_NOTNC4_def_grp,
NC_NOTNC4_rename_grp,
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
NC_NOTNC4_def_var_chunking,
NC_NOTNC4_def_var_endian,
NC_NOTNC4_def_var_filter,
NC_NOTNC4_set_var_chunk_cache,
NC_NOTNC4_get_var_chunk_cache
};

int
main(int argc, char **argv)
{
   printf("\n*** Testing user-defined formats.\n");
   printf("*** testing simple user-defined format...");
   {
      int ncid;
      /* Create an empty file to play with. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Add our test user defined format. */
      if (nc_def_user_format(NC_UF0, &tst_dispatcher, NULL)) ERR;

      /* Open file with our defined functions. */
      if (nc_open(FILE_NAME, NC_UF0, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open file again and abort, which is the same as closing it. */
      if (nc_open(FILE_NAME, NC_UF0, &ncid)) ERR;
      if (nc_inq_format(ncid, NULL) != TEST_VAL_42) ERR;
      if (nc_inq_format_extended(ncid, NULL, NULL) != TEST_VAL_42) ERR;
      if (nc_abort(ncid) != TEST_VAL_42) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
