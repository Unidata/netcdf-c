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

static int NCZTR_create(const char *path, int cmode, size_t initialsz, int basepe, size_t *chunksizehintp, void *parameters, const struct NC_Dispatch *table, int ncid)
{
    int stat = NC_NOERR;
    stat = NCZ_create(path,cmode,initialsz,basepe,chunksizehintp,parameters,table,ncid);
    return stat;
}

static int NCZTR_open(const char *path, int mode, int basepe, size_t *chunksizehintp,void *parameters, const struct NC_Dispatch *table, int ncid)
{
    int stat = NC_NOERR;
    stat = NCZ_open(path,mode,basepe,chunksizehintp,parameters,table,ncid);
    return stat;
}

static int NCZTR_redef(int ncid)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_redef(ncid)");
    stat = NCZ_redef(ncid);
    return ZUNTRACE(stat);
}

static int NCZTR__enddef(int ncid,size_t h_minfree,size_t v_align,size_t v_minfree,size_t r_align)
{
    int stat = NC_NOERR;
    stat = NCZ__enddef(ncid,h_minfree,v_align,v_minfree,r_align);
    return stat;
}

static int NCZTR_sync(int ncid)
{
    int stat = NC_NOERR;
    stat = NCZ_sync(ncid);
    return stat;
}

static int NCZTR_abort(int ncid)
{
    int stat = NC_NOERR;
    stat = NCZ_abort(ncid);
    return stat;
}

static int NCZTR_close(int ncid, void* params)
{
    int stat = NC_NOERR;
    stat = NCZ_close(ncid,params);
    return stat;
}

static int NCZTR_set_fill(int ncid, int fillmode, int *old)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_set_fill(ncid,fillmode,old)");
    stat = NCZ_set_fill(ncid,fillmode,old);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_format(int ncid, int* formatp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_format(ncid,formatp)");
    stat = NC4_inq_format(ncid,formatp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_format_extended(int ncid, int *formatp, int *modep)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_format_extended(ncid,formatp,modep)");
    stat = NCZ_inq_format_extended(ncid,formatp,modep);
    return ZUNTRACE(stat);
}

static int NCZTR_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *udimp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq(ncid,ndimsp,nvarsp,nattsp,udimp)");
    stat = NCZ_inq(ncid,ndimsp,nvarsp,nattsp,udimp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_type(int ncid, nc_type xtype, char *name, size_t *size)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_type(ncid,xtype,name,size)");
    stat = NC4_inq_type(ncid,xtype,name,size);
    return ZUNTRACE(stat);
}

static int NCZTR_def_dim(int ncid, const char *name, size_t len, int *idp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_dim(ncid,name,len,idp)");
    stat = NCZ_def_dim(ncid,name,len,idp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_dimid(int ncid, const char *name, int *idp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_dimid(ncid,name,idp)");
    stat = NC4_inq_dimid(ncid,name,idp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_dim(int ncid, int dimid, char *name, size_t *lenp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_dim(ncid,dimid,name,lenp)");
    stat = NCZ_inq_dim(ncid,dimid,name,lenp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_unlimdim(int ncid,  int *unlimdimidp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_unlimdim(ncid,unlimdimidp)");
    stat = NC4_inq_unlimdim(ncid,unlimdimidp);
    return ZUNTRACE(stat);
}

static int NCZTR_rename_dim(int ncid, int dimid, const char *name)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_rename_dim(ncid,dimid,name)");
    stat = NCZ_rename_dim(ncid,dimid,name);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_att(int ncid, int varid, const char *name, nc_type *xtypep, size_t *lenp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_att(ncid,varid,name,xtypep,lenp)");
    stat = NCZ_inq_att(ncid,varid,name,xtypep,lenp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_attid(int ncid, int varid, const char* name, int *idp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_attid(ncid,varid,name,idp)");
    stat = NCZ_inq_attid(ncid,varid,name,idp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_attname(int ncid, int varid, int attnum, char *name)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_attname(ncid,varid,attnum,name)");
    stat = NCZ_inq_attname(ncid,varid,attnum,name);
    return ZUNTRACE(stat);
}

static int NCZTR_rename_att(int ncid, int varid, const char* name, const char *newname)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_rename_att(ncid,varid,name,newname)");
    stat = NCZ_rename_att(ncid,varid,name,newname);
    return ZUNTRACE(stat);
}

static int NCZTR_del_att(int ncid, int varid, const char *name)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_del_att(ncid,varid,name)");
    stat = NCZ_del_att(ncid,varid,name);
    return ZUNTRACE(stat);
}

static int NCZTR_get_att(int ncid, int varid, const char* name, void *data, nc_type memtype)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_get_att(ncid,varid,name,data,memtype)");
    stat = NCZ_get_att(ncid,varid,name,data,memtype);
    return ZUNTRACE(stat);
}

static int NCZTR_put_att(int ncid, int varid, const char* name, nc_type filetype, size_t len, const void *data, nc_type memtype)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_put_att(ncid,varid,name,filetype,len,data,memtype)");
    stat = NCZ_put_att(ncid,varid,name,filetype,len,data,memtype);
    return ZUNTRACE(stat);
}

static int NCZTR_def_var(int ncid, const char* name, nc_type xtype, int ndims, const int *dimidsp, int *varidp)
{
    int stat = NC_NOERR;
    stat = NCZ_def_var(ncid,name,xtype,ndims,dimidsp,varidp);
    return stat;
}

static int NCZTR_inq_varid(int ncid, const char* name, int *varidp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_varid(ncid,name,varidp)");
    stat = NC4_inq_varid(ncid,name,varidp);
    return ZUNTRACE(stat);
}

static int NCZTR_rename_var(int ncid, int varid, const char *name)
{
    int stat = NC_NOERR;
    stat = NCZ_rename_var(ncid,varid,name);
    return stat;
}

static int NCZTR_get_vara(int ncid, int varid, const size_t *startp, const size_t *countp, void *ip, nc_type memtype)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_get_vara(ncid,varid,startp,countp,ip,memtype)");
    stat = NCZ_get_vara(ncid,varid,startp,countp,ip,memtype);
    return ZUNTRACE(stat);
}

static int NCZTR_put_vara(int ncid, int varid, const size_t *startp, const size_t *countp, const void *ip, nc_type memtype)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_put_vara(ncid,varid,startp,countp,ip,memtype)");
    stat = NCZ_put_vara(ncid,varid,startp,countp,ip,memtype);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep, int *ndimsp, int *dimidsp, int *nattsp, int *shufflep, int *deflatep, int *deflate_levelp, int *fletcher32p, int *contiguousp, size_t *chunksizesp, int *no_fill, void *fill_valuep, int *endiannessp, unsigned int *idp, size_t *nparamsp, unsigned int *params)
{
    int stat = NC_NOERR;
    stat = NCZ_inq_var_all(ncid,varid,name,xtypep,ndimsp,dimidsp,nattsp,shufflep,deflatep,deflate_levelp,fletcher32p,contiguousp,chunksizesp,no_fill,fill_valuep,endiannessp,idp,nparamsp,params);
    return stat;
}

static int NCZTR_var_par_access(int ncid, int varid, int par_access)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_var_par_access(ncid,varid,par_access)");
    stat = NCZ_var_par_access(ncid,varid,par_access);
    return ZUNTRACE(stat);
}

static int NCZTR_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_var_fill(ncid,varid,no_fill,fill_value)");
    stat = NCZ_def_var_fill(ncid,varid,no_fill,fill_value);
    return ZUNTRACE(stat);
}

static int NCZTR_show_metadata(int ncid)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_show_metadata(ncid)");
    stat = NCZ_show_metadata(ncid);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_unlimdims(int ncid, int* n, int* uidsp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_unlimdims(ncid,n,uidsp)");
    stat = NCZ_inq_unlimdims(ncid,n,uidsp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_ncid(int ncid, const char* name, int* grpidp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_ncid(ncid,name,grpidp)");
    stat = NC4_inq_ncid(ncid,name,grpidp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_grps(int ncid, int* n, int* ncids)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_grps(ncid,n,ncids)");
    stat = NC4_inq_grps(ncid,n,ncids);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_grpname(int ncid, char* name)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_grpname(ncid,name)");
    stat = NC4_inq_grpname(ncid,name);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_grpname_full(int ncid, size_t* lenp, char* fullname)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_grpname_full(ncid,lenp,fullname)");
    stat = NC4_inq_grpname_full(ncid,lenp,fullname);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_grp_parent(int ncid, int* parentidp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_grp_parent(ncid,parentidp)");
    stat = NC4_inq_grp_parent(ncid,parentidp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_grp_full_ncid(int ncid, const char* fullname, int* grpidp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_grp_full_ncid(ncid,fullname,grpidp)");
    stat = NC4_inq_grp_full_ncid(ncid,fullname,grpidp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_varids(int ncid, int* nvars, int* varids)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_varids(ncid,nvars,varids)");
    stat = NC4_inq_varids(ncid,nvars,varids);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_dimids(int ncid, int* ndims, int* dimids, int inclparents)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_dimids(ncid,ndims,dimids,inclparents)");
    stat = NC4_inq_dimids(ncid,ndims,dimids,inclparents);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_typeids(int ncid, int* ntypes, int* typeids)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_typeids(ncid,ntypes,typeids)");
    stat = NCZ_inq_typeids(ncid,ntypes,typeids);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_type_equal(int ncid1, nc_type tid1, int ncid2, nc_type tid2, int* eq)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_type_equal(ncid1,tid1,ncid2,tid2,eq)");
    stat = NCZ_inq_type_equal(ncid1,tid1,ncid2,tid2,eq);
    return ZUNTRACE(stat);
}

static int NCZTR_def_grp(int parent, const char* name, int* grpid)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_grp(parent,name,grpid)");
    stat = NCZ_def_grp(parent,name,grpid);
    return ZUNTRACE(stat);
}

static int NCZTR_rename_grp(int ncid, const char* name)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_rename_grp(ncid,name)");
    stat = NCZ_rename_grp(ncid,name);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_user_type(int ncid, nc_type xtype, char* name, size_t* size, nc_type* basetid, size_t* nfields, int* classp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_inq_user_type(ncid,xtype,name,size,basetid,nfields,classp)");
    stat = NC4_inq_user_type(ncid,xtype,name,size,basetid,nfields,classp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_typeid(int ncid, const char* name, nc_type* tidp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_typeid(ncid,name,tidp)");
    stat = NCZ_inq_typeid(ncid,name,tidp);
    return ZUNTRACE(stat);
}

static int NCZTR_def_var_chunking(int ncid, int varid, int storage, const size_t *chunksizes)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_var_chunking(ncid,varid,storage,chunksizes)");
    stat = NCZ_def_var_chunking(ncid,varid,storage,chunksizes);
    return ZUNTRACE(stat);
}

static int NCZTR_def_var_endian(int ncid, int varid, int endian)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_var_endian(ncid,varid,endian)");
    stat = NCZ_def_var_endian(ncid,varid,endian);
    return ZUNTRACE(stat);
}

static int NCZTR_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_set_var_chunk_cache(ncid,varid,size,nelems,preemption)");
    stat = NCZ_set_var_chunk_cache(ncid,varid,size,nelems,preemption);
    return ZUNTRACE(stat);
}

static int NCZTR_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, float *preemptionp)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NC4_get_var_chunk_cache(ncid,varid,sizep,nelemsp,preemptionp)");
    stat = NC4_get_var_chunk_cache(ncid,varid,sizep,nelemsp,preemptionp);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_var_filter_ids(int ncid, int varid, size_t* nfilters, unsigned int* filterids)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_var_filter_ids(ncid,varid,nfilters,filterids)");
    stat = NCZ_inq_var_filter_ids(ncid,varid,nfilters,filterids);
    return ZUNTRACE(stat);
}

static int NCZTR_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparams, unsigned int* params)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_inq_var_filter_info(ncid,varid,id,nparams,params)");
    stat = NCZ_inq_var_filter_info(ncid,varid,id,nparams,params);
    return ZUNTRACE(stat);
}


static int NCZTR_get_vars(int ncid, int varid, const size_t *startp, const size_t *countp, const ptrdiff_t *stridep, void *ip, nc_type memtype)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_get_vars(ncid,varid,startp,countp,stridep,ip,memtype)");
    stat = NCZ_get_vars(ncid,varid,startp,countp,stridep,ip,memtype);
    return ZUNTRACE(stat);
}

static int NCZTR_put_vars(int ncid, int varid, const size_t *startp, const size_t *countp, const ptrdiff_t *stridep, const void *ip, nc_type memtype)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_put_vars(ncid,varid,startp,countp,stridep,ip,memtype)");
    stat = NCZ_put_vars(ncid,varid,startp,countp,stridep,ip,memtype);
    return ZUNTRACE(stat);
}

#if 0
static int NCZTR_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int level)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_var_deflate(ncid,varid,shuffle,deflate,level)");
    stat = NCZ_def_var_deflate(ncid,varid,shuffle,deflate,level);
    return ZUNTRACE(stat);
}

static int NCZTR_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_var_fletcher32(ncid,varid,fletcher32)");
    stat = NCZ_def_var_fletcher32(ncid,varid,fletcher32);
    return ZUNTRACE(stat);
}

static int NCZTR_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int *params)
{
    int stat = NC_NOERR;
    ZTRACE(0,"NCZ_def_var_filter(ncid,varid,id,nparams,params)");
    stat = NCZ_def_var_filter(ncid,varid,id,nparams,params);
    return ZUNTRACE(stat);
}

#endif

static const NC_Dispatch NCZ_dispatcher_trace = {

    NC_FORMATX_NCZARR,
    NC_DISPATCH_VERSION,

    NCZTR_create,
    NCZTR_open,

    NCZTR_redef,
    NCZTR__enddef,
    NCZTR_sync,
    NCZTR_abort,
    NCZTR_close,
    NCZTR_set_fill,
    NCZTR_inq_format,
    NCZTR_inq_format_extended,

    NCZTR_inq,
    NCZTR_inq_type,

    NCZTR_def_dim,
    NCZTR_inq_dimid,
    NCZTR_inq_dim,
    NCZTR_inq_unlimdim,
    NCZTR_rename_dim,

    NCZTR_inq_att,
    NCZTR_inq_attid,
    NCZTR_inq_attname,
    NCZTR_rename_att,
    NCZTR_del_att,
    NCZTR_get_att,
    NCZTR_put_att,

    NCZTR_def_var,
    NCZTR_inq_varid,
    NCZTR_rename_var,
    NCZTR_get_vara,
    NCZTR_put_vara,
    NCZTR_get_vars,
    NCZTR_put_vars,
    NCDEFAULT_get_varm,
    NCDEFAULT_put_varm,

    NCZTR_inq_var_all,

    NCZTR_var_par_access,
    NCZTR_def_var_fill,

    NCZTR_show_metadata,
    NCZTR_inq_unlimdims,

    NCZTR_inq_ncid,
    NCZTR_inq_grps,
    NCZTR_inq_grpname,
    NCZTR_inq_grpname_full,
    NCZTR_inq_grp_parent,
    NCZTR_inq_grp_full_ncid,
    NCZTR_inq_varids,
    NCZTR_inq_dimids,
    NCZTR_inq_typeids,
    NCZTR_inq_type_equal,
    NCZTR_def_grp,
    NCZTR_rename_grp,
    NCZTR_inq_user_type,
    NCZTR_inq_typeid,

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
    NCZTR_def_var_chunking,
    NCZTR_def_var_endian,
    NC_NOTNC4_def_var_filter,
    NCZTR_set_var_chunk_cache,
    NCZTR_get_var_chunk_cache,
    NCZTR_inq_var_filter_ids,
    NCZTR_inq_var_filter_info,
};

