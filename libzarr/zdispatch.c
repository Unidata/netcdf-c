/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"


/**************************************************/

static int nczinitialized = 0;

static NC_Dispatch NCZ_dispatch_base;

NC_Dispatch* NCZ_dispatch_table = NULL;

/**************************************************/

static int
globalinit(void)
{
    int stat = NC_NOERR;
    {
	CURLcode cstat = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(cstat != CURLE_OK)
	    fprintf(stderr,"curl_global_init failed!\n");
    }
    return stat;
}

/**************************************************/
int
NCZ_initialize(void)
{
    NCZ_dispatch_table = &NCZ_dispatch_base;
    nczinitialized = 1;
    ncloginit();
#ifdef ZDEBUG
    /* force logging to go to stderr */
    nclogclose();
    if(nclogopen(NULL))
        ncsetlogging(1); /* turn it on */
#endif
    /* Init global state */
    globalinit();
    /* Load rc file */
    NC_rcload();    

    return THROW(NC_NOERR);
}

int
NCZ_finalize(void)
{
    curl_global_cleanup();
    return THROW(NC_NOERR);
}

int
NCZ_create(const char *path, int cmode,
	  size_t initialsz, int basepe, size_t *chunksizehintp,
	  int use_parallel, void* parameters,
	  struct NC_Dispatch* table, NC* ncp)
{return NC_NOERR;}

int
NCZ_open(const char *path, int mode,
	    int basepe, size_t *chunksizehintp,
	    int use_parallel, void* parameters,
	    struct NC_Dispatch* table, NC* ncp)
{return NC_NOERR;}

int
NCZ_redef(int ncid)
{return NC_NOERR;}

int
nc__enddef(int ncid, size_t h_minfree, size_t v_align,
        size_t v_minfree, size_t r_align)
{return NC_NOERR;}

int
NCZ_sync(int ncid)
{return NC_NOERR;}

int
NCZ_abort(int ncid)
{return NC_NOERR;}

int
NCZ_close(int ncid, void* params)
{return NC_NOERR;}

int
NCZ_set_fill(int ncid ,int fillmode, int* old_modep)
{return NC_NOERR;}

int
NCZ_inq_base_pe(int ncid, int* pe)
{
    return NC_NOTNC3_inq_base_pe(ncid,pe);
}

int
NCZ_set_base_pe(int ncid, int pe)
{
    return NC_NOTNC3_set_base_pe(ncid,pe);
}

int
NCZ_inq_format(int ncid, int* formatp)
{return NC4_inq_format(ncid,formatp);}

int
NCZ_inq_format_extended(int ncid, int* formatp, int* modep)
{return NC4_inq_format_extended(ncid,formatp,modep);}

int
NCZ_inq(int ncid, int* ndimsp, int* nvarsp, int* nattsp, int* unlimdimidp)
{return NC_NOERR;}

int
NCZ_inq_type(int ncid, nc_type xtype, char* name, size_t* size)
{return NC4_inq_type(ncid,xtype,name,size);}

int
NCZ_def_dim(int ncid, const char* name, size_t len, int* idp)
{return NC_NOERR;}

int
NCZ_inq_dimid(int ncid, const char* name, int* idp)
{return NC4_inq_dimid(ncid,name,idp);}

int
NCZ_inq_dim(int ncid, int dimid, char* name, size_t* lenp)
{return NC_NOERR;}

int
NCZ_inq_unlimdim(int ncid, int *unlimdimidp)
{return NC4_inq_unlimdim(ncid,unlimdimidp);}

int
NCZ_rename_dim(int ncid, int dimid, const char* name)
{return NC_NOERR;}

int
NCZ_inq_att(int ncid, int varid, const char* name, nc_type* xtypep, size_t* lenp)
{return NC_NOERR;}

int
NCZ_inq_attid(int ncid, int varid, const char* name, int* idp)
{return NC_NOERR;}

int
NCZ_inq_attname(int ncid, int varid, int attnum, char* name)
{return NC_NOERR;}

int
NCZ_rename_att(int ncid, int varid, const char* oldname, const char* newname)
{return NC_NOERR;}

int
NCZ_del_att(int ncid, int varid, const char* name)
{return NC_NOERR;}

int
NCZ_get_att(int ncid, int varid, const char* name, void* ip, nc_type memtype)
{return NC_NOERR;}

int
NCZ_put_att(int ncid, int varid, const char* name, nc_type xtype, size_t len, const void* op, nc_type memtype)
{return NC_NOERR;}

int
NCZ_def_var(int ncid, const char* name, nc_type xtype, int ndims, const int* dimidsp, int* varidp)
{return NC_NOERR;}

int
NCZ_inq_varid(int ncid, const char* name, int* varidp)
{return NC4_inq_varid(ncid,name,varidp);}

int
NCZ_rename_var(int ncid, int varid, const char* newname)
{return NC_NOERR;}

int
NCZ_get_vara(int ncid, int varid, const size_t* startp, const size_t* countp, void* ip, nc_type memtype)
{return NC_NOERR;}

int
NCZ_put_vara(int ncid, int varid, const size_t* startp, const size_t* countp, const void* op, nc_type memtype)
{return NC_NOERR;}

int
NCZ_get_vars(int ncid, int varid, const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, void* ip, nc_type memtype)
{return NC_NOERR;}

int
NCZ_put_vars(int ncid, int varid, const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, const void* op, nc_type memtype)
{return NC_NOERR;}

int
NCZ_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep,
               int *ndimsp, int *dimidsp, int *nattsp,
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp,
               int *no_fill, void *fill_valuep, int *endiannessp,
	       unsigned int* idp, size_t* nparamsp, unsigned int* params
              )
{return NC_NOERR;}

int
NCZ_var_par_access(int ncid, int varid, int par_access)
{return NC4_var_par_access(ncid,varid,par_access);}

int
NCZ_def_var_fill(int ncid, int varid, int no_fill, const void* fill_value)
{return NC_NOERR;}

int
NCZ_show_metadata(int ncid)
{return NC4_show_metadata(ncid);}

int
NCZ_inq_unlimdims(int ncid, int* nunlimdimsp, int* unlimdimidsp)
{return NC4_inq_unlimdims(ncid,nunlimdimsp,unlimdimidsp);}

int
NCZ_inq_ncid(int ncid, const char* name, int* grp_ncid)
{return NC4_inq_ncid(ncid,name,grp_ncid);}

int
NCZ_inq_grps(int ncid, int* ngrpsp, int* ncidsp)
{return NC4_inq_grps(ncid,ngrpsp,ncidsp);}

int
NCZ_inq_grpname(int ncid, char* name)
{return NC_NOERR;}

int
NCZ_inq_grpname_full(int ncid, size_t* lenp, char* full_name)
{return NC4_inq_grpname_full(ncid,lenp,full_name);}

int
NCZ_inq_grp_parent(int ncid, int* parent_ncidp)
{return NC4_inq_grp_parent(ncid,parent_ncidp);}

int
NCZ_inq_grp_full_ncid(int ncid, const char* full_name, int* grp_ncid)
{return NC4_inq_grp_full_ncid(ncid,full_name,grp_ncid);}

int
NCZ_inq_varids(int ncid, int* nvars, int* varids)
{return NC4_inq_varids(ncid,nvars,varids);}

int
NCZ_inq_dimids(int ncid, int* ndims, int* dimids, int include_parent)
{return NC4_inq_dimids(ncid,ndims,dimids,include_parent);}

int
NCZ_inq_typeids(int ncid, int* ntypes, int* typeids)
{return NC4_inq_typeids(ncid,ntypes,typeids);}

int
NCZ_inq_type_equal(int ncid1, nc_type typeid1, int ncid2, nc_type typeid2, int*equal)
{return NC_NOERR;}

int
NCZ_def_grp(int ncid, const char* name, int* new_ncidp)
{return NC_NOERR;}

int
NCZ_rename_grp(int ncid, const char* name)
{return NC_NOERR;}

int
NCZ_inq_user_type(int ncid, nc_type xtype, char* name, size_t* size, nc_type* base_type, size_t* nfieldsp, int* classp)
{return NC4_inq_user_type(ncid,xtype,name,size,base_type,nfieldsp,classp);}

int
NCZ_inq_typeid(int ncid, const char* name, nc_type* typeidp)
{return NC_NOERR;}

int
NCZ_def_compound(int ncid, size_t size, const char* name, nc_type* typeidp)
{return NC_NOERR;}

int
NCZ_insert_compound(int ncid, nc_type xtype, const char* name, size_t offset, nc_type field_typeid)
{return NC_NOERR;}

int
NCZ_insert_array_compound(int ncid, nc_type xtype, const char* name, size_t offset, nc_type field_typeid, int ndims, const int* dim_sizes)
{return NC_NOERR;}

int
NCZ_inq_compound_field(int ncid, nc_type xtype, int fieldid, char* name, size_t* offsetp, nc_type* field_typeidp, int* ndimsp, int* dim_sizesp)
{return NC4_inq_compound_field(ncid,xtype,fieldid,name,offsetp,field_typeidp,ndimsp,dim_sizesp);}

int
NCZ_inq_compound_fieldindex(int ncid, nc_type xtype, const char* name, int* fieldidp)
{return NC4_inq_compound_fieldindex(ncid,xtype,name.,fieldidp);}

int
NCZ_def_vlen(int ncid, const char* name, nc_type base_typeid, nc_type* xtypep)
{return NC_NOERR;}

int
NCZ_put_vlen_element(int ncid, int typeid1, void* vlen_element, size_t len, const void* data)
{return NC_NOERR;}

int
NCZ_get_vlen_element(int ncid, int typeid1, const void* vlen_element, size_t* len, void* data)
{return NC_NOERR;}

int
NCZ_def_enum(int ncid, nc_type base_typeid, const char* name, nc_type* typeidp)
{return NC_NOERR;}

int
NCZ_insert_enum(int ncid, nc_type xtype, const char* name, const void* value)
{return NC_NOERR;}

int
NCZ_inq_enum_member(int ncid, nc_type xtype, int idx, char* name, void* value)
{return NC4_inq_enum_member(ncid,xtype,idx,name,value);}

int
NCZ_inq_enum_ident(int ncid, nc_type xtype, long long value, char* name)
{return NC4_inq_enum_ident(ncid,xtype,value,name);}

int
NCZ_def_opaque(int ncid, size_t size, const char* name, nc_type* sizep)
{return NC_NOERR;}

int
NCZ_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int deflate_level)
{return NC_NOERR;}

int
NCZ_def_var_fletcher32(int ncid, int varid, int fletcher32)
{return NC_NOERR;}

int
NCZ_def_var_chunking(int ncid, int varid, int storage, const size_t* chunksizep)
{return NC_NOERR;}

int
NCZ_def_var_endian(int ncid, int varid, int endian)
{return NC_NOERR;}

int
NCZ_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms)
{return NC_NOERR;}

int
NCZ_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption)
{return NC_NOERR;}

int
NCZ_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, float *preemptionp)
{return NC4_get_var_chunk_cache(ncid,varid,sizep,nelemsp,preemptionp);}

static NC_Dispatch NCZ_dispatcher = {

NC_FORMATX_ZARR,

NCZ_create,
NCZ_open,

NCZ_redef,
NCZ__enddef,
NCZ_sync,
NCZ_abort,
NCZ_close,
NCZ_set_fill,
NC_NOTNC3_inq_base_pe,
NC_NOTNC3_set_base_pe,
NCZ_inq_format,
NCZ_inq_format_extended,

NCZ_inq,
NC4_inq_type,

NCZ_def_dim,
NC4_inq_dimid,
NCz_inq_dim,
NC4_inq_unlimdim,
NCZ_rename_dim,

NC4_inq_att,
NC4_inq_attid,
NC4_inq_attname,
NCZ_rename_att,
NCZ_del_att,
NC4_get_att,
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

NC4_inq_var_all,

NC4_var_par_access,
NCZ_def_var_fill,

#ifdef USE_NETCDF4
NC4_show_metadata,
NC4_inq_unlimdims,

NC4_inq_ncid,
NC4_inq_grps,
NCZ_inq_grpname,
NCZ_inq_grpname_full,
NCZ_inq_grp_parent,
NCZ_inq_grp_full_ncid,
NC4_inq_varids,
NC4_inq_dimids,
NC4_inq_typeids,
NCZ_inq_type_equal,
NCZ_def_grp,
NCZ_rename_grp,
NC4_inq_user_type,
NCZ_inq_typeid,

NCZ_def_compound,
NCZ_insert_compound,
NCZ_insert_array_compound,
NC4_inq_compound_field,
NC4_inq_compound_fieldindex,
NCZ_def_vlen,
NCZ_put_vlen_element,
NCZ_get_vlen_element,
NCZ_def_enum,
NCZ_insert_enum,
NCZ_inq_enum_member,
NC4_inq_enum_ident,
NCZ_def_opaque,
NCZ_def_var_deflate,
NCZ_def_var_fletcher32,
NCZ_def_var_chunking,
NCZ_def_var_endian,
NCZ_def_var_filter,
NCZ_set_var_chunk_cache,
NC4_get_var_chunk_cache,
#endif

};

NC_Dispatch* HDF5_dispatch_table = NULL; /* moved here from ddispatch.c */



