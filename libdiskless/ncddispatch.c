/*! \file \internal
Contains the dispatch functions for diskless interface.

Copyright 2011, UCAR/Unidata. See COPYRIGHT file for copying and
redistribution conditions.
*/

#include <stdlib.h>
#include "nc.h"
#include "ncdispatch.h"
#include "ncddispatch.h"

NC_Dispatch NCD_dispatcher = 
{
   NC_DISPATCH_NCD,

   NCD_new_nc,

   NCD_create,
   NCD_open,

   NCD_redef,
   NCD__enddef,
   NCD_sync,
   NCD_abort,
   NCD_close,
   NCD_set_fill,
   NCD_inq_base_pe,
   NCD_set_base_pe,
   NCD_inq_format,

   NCD_inq,
   NCD_inq_type,

   NCD_def_dim,
   NCD_inq_dimid,
   NCD_inq_dim,
   NCD_inq_unlimdim,
   NCD_rename_dim,

   NCD_inq_att,
   NCD_inq_attid,
   NCD_inq_attname,
   NCD_rename_att,
   NCD_del_att,
   NCD_get_att,
   NCD_put_att,

   NCD_def_var,
   NCD_inq_varid,
   NCD_rename_var,
   NCD_get_vara,
   NCD_put_vara,
   NCDEFAULT_get_vars,
   NCDEFAULT_put_vars,
   NCDEFAULT_get_varm,
   NCDEFAULT_put_varm,

   NCD_inq_var_all,

   NCD_show_metadata,
   NCD_inq_unlimdims,

   NCD_var_par_access,

   NCD_inq_ncid,
   NCD_inq_grps,
   NCD_inq_grpname,
   NCD_inq_grpname_full,
   NCD_inq_grp_parent,
   NCD_inq_grp_full_ncid,
   NCD_inq_varids,
   NCD_inq_dimids,
   NCD_inq_typeids,
   NCD_inq_type_equal,
   NCD_def_grp,
   NCD_inq_user_type,
   NCD_inq_typeid,

   NCD_def_compound,
   NCD_insert_compound,
   NCD_insert_array_compound,
   NCD_inq_compound_field,
   NCD_inq_compound_fieldindex,
   NCD_def_vlen,
   NCD_put_vlen_element,
   NCD_get_vlen_element,
   NCD_def_enum,
   NCD_insert_enum,
   NCD_inq_enum_member,
   NCD_inq_enum_ident,
   NCD_def_opaque,
   NCD_def_var_deflate,
   NCD_def_var_fletcher32,
   NCD_def_var_chunking,
   NCD_def_var_fill,
   NCD_def_var_endian,
   NCD_set_var_chunk_cache,
   NCD_get_var_chunk_cache,

};

int
NCD_initialize(void)
{
   NCD_dispatch_table = &NCD_dispatcher;
   return NC_NOERR;
}
