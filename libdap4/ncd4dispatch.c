/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include "ncd4dispatch.h"
#include "d4includes.h"
#include "d4curlfunctions.h"

#ifdef HAVE_GETRLIMIT
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/time.h>
#  endif
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/resource.h>
#  endif
#endif

#if 0
/* Define the set of protocols known to be constrainable */
static char* constrainableprotocols[] = {"http", "https",NULL};
#endif

static int ncd4initialized = 0;

static NC_Dispatch NCD4_dispatch_base;

NC_Dispatch* NCD4_dispatch_table = NULL; /* moved here from ddispatch.c */

/* Collect global state info in one place */
NCD4globalstate* NCD4_globalstate = NULL;

/* Forward */
static int globalinit(void);

/**************************************************/
int
NCD4_initialize(void)
{
    NCD4_dispatch_table = &NCD4_dispatch_base;
    ncd4initialized = 1;
    ncloginit();
#ifdef D4DEBUG
    /* force logging to go to stderr */
    nclogclose();
    if(nclogopen(NULL))
        ncsetlogging(1); /* turn it on */
#endif
    /* Init global state */
    globalinit();
    /* Load rc file */
    NCD4_rcload();    
    return THROW(NC_NOERR);
}

int
NCD4_finalize(void)
{
    if(NCD4_globalstate != NULL) {
        nullfree(NCD4_globalstate->tempdir);
        nullfree(NCD4_globalstate->home);
	nclistfree(NCD4_globalstate->rc.rc);
	nullfree(NCD4_globalstate->rc.rcfile);
	free(NCD4_globalstate);
	NCD4_globalstate = NULL;
    }
    return THROW(NC_NOERR);
}

static int
NCD4_redef(int ncid)
{
    return (NC_EPERM);
}

static int
NCD4__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align)
{
    return (NC_EPERM);
}

static int
NCD4_sync(int ncid)
{
    return (NC_EINVAL);
}

static int
NCD4_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch* dispatch, NC* ncp)
{
   return THROW(NC_EPERM);
}

static int
NCD4_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            const void *value,
	    nc_type memtype)
{
    return THROW(NC_EPERM);
}

static int
NCD4_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value,
	    nc_type memtype)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return ret;
    ret = nc_get_vara(getnc4id(d4info), varid, start, edges, value);
    return ret;
}

static int
NCD4_put_vars(int ncid, int varid,
	    const size_t *start, const size_t *edges, const ptrdiff_t* stride,
            const void *value0, nc_type memtype)
{
    return THROW(NC_EPERM);
}

static int
NCD4_get_vars(int ncid, int varid,
	    const size_t *start, const size_t *edges, const ptrdiff_t* stride,
            void *value, nc_type memtype)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_get_vars(getnc4id(d4info), varid, start, edges, stride, value);
    return (ret);
}

/**************************************************/

static int
NCD4_inq_format_extended(int ncid, int* formatp, int* modep)
{
    NC* nc;
    int ncstatus = NC_check_id(ncid, (NC**)&nc);
    if(ncstatus != NC_NOERR) return (ncstatus);
    if(modep) *modep = nc->mode;
    if(formatp) *formatp = NC_FORMATX_DAP2;
    return THROW(NC_NOERR);
}

/*
Force dap4 access to be read-only
*/
static int
NCD4_set_fill(int ncid, int fillmode, int* old_modep)
{
    return (NC_EPERM);
}

static int
NCD4_set_base_pe(int ncid, int pe)
{
    return (NC_EPERM);
}

static int
NCD4_def_dim(int ncid, const char* name, size_t len, int* idp)
{
    return (NC_EPERM);
}

static int
NCD4_put_att(int ncid, int varid, const char* name, nc_type datatype,
	   size_t len, const void* value, nc_type t)
{
    return (NC_EPERM);
}

static int
NCD4_def_var(int ncid, const char *name,
  	     nc_type xtype, int ndims, const int *dimidsp, int *varidp)
{
    return (NC_EPERM);
}

/*
Following functions basically return the netcdf-4 value WRT to the nc4id.
*/

static int
NCD4_inq_base_pe(int ncid, int* pe)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_base_pe(getnc4id(d4info), pe);
    return (ret);
}

static int
NCD4_inq_format(int ncid, int* formatp)
{
    NC* d4info;
    int ret = NC_NOERR;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_format(getnc4id(d4info), formatp);
    return (ret);
}

static int
NCD4_inq(int ncid, int* ndimsp, int* nvarsp, int* nattsp, int* unlimdimidp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq(getnc4id(d4info), ndimsp, nvarsp, nattsp, unlimdimidp);
    return (ret);
}

static int
NCD4_inq_type(int ncid, nc_type p2, char* p3, size_t* p4)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_type(getnc4id(d4info), p2, p3, p4);
    return (ret);
}

static int
NCD4_inq_dimid(int ncid, const char* name, int* idp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_dimid(getnc4id(d4info), name, idp);
    return (ret);
}

static int
NCD4_inq_dim(int ncid, int dimid, char* name, size_t* lenp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_dim(getnc4id(d4info), dimid, name, lenp);
    return (ret);
}

static int
NCD4_inq_unlimdim(int ncid, int* unlimdimidp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_unlimdim(getnc4id(d4info), unlimdimidp);
    return (ret);
}

static int
NCD4_rename_dim(int ncid, int dimid, const char* name)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_rename_dim(getnc4id(d4info), dimid, name);
    return (ret);
}

static int
NCD4_inq_att(int ncid, int varid, const char* name,
	    nc_type* xtypep, size_t* lenp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_att(getnc4id(d4info), varid, name, xtypep, lenp);
    return (ret);
}

static int
NCD4_inq_attid(int ncid, int varid, const char *name, int *idp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_attid(getnc4id(d4info), varid, name, idp);
    return (ret);
}

static int
NCD4_inq_attname(int ncid, int varid, int attnum, char* name)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_attname(getnc4id(d4info), varid, attnum, name);
    return (ret);
}

static int
NCD4_rename_att(int ncid, int varid, const char* name, const char* newname)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_rename_att(getnc4id(d4info), varid, name, newname);
    return (ret);
}

static int
NCD4_del_att(int ncid, int varid, const char* p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_del_att(getnc4id(d4info), varid, p3);
    return (ret);
}

static int
NCD4_get_att(int ncid, int varid, const char* name, void* value, nc_type t)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = NCDISPATCH_get_att(getnc4id(d4info), varid, name, value, t);
    return (ret);
}

static int
NCD4_inq_var_all(int ncid, int varid, char *name, nc_type* xtypep,
               int* ndimsp, int* dimidsp, int* nattsp,
               int* shufflep, int* deflatep, int* deflate_levelp,
               int* fletcher32p, int* contiguousp, size_t* chunksizesp,
               int* no_fill, void* fill_valuep, int* endiannessp,
	       int* options_maskp, int* pixels_per_blockp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = NCDISPATCH_inq_var_all(getnc4id(d4info), varid, name, xtypep,
               ndimsp, dimidsp, nattsp,
               shufflep, deflatep, deflate_levelp,
               fletcher32p, contiguousp, chunksizesp,
               no_fill, fill_valuep, endiannessp,
	       options_maskp, pixels_per_blockp);
    return (ret);
}

static int
NCD4_inq_varid(int ncid, const char *name, int *varidp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_varid(getnc4id(d4info),name,varidp);
    return (ret);
}

static int
NCD4_rename_var(int ncid, int varid, const char* name)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_rename_var(getnc4id(d4info), varid, name);
    return (ret);
}

static int
NCD4_var_par_access(int ncid, int p2, int p3)
{
    return (NC_ENOPAR);
}


#ifdef USE_NETCDF4

static int
NCD4_inq_ncid(int ncid, const char* name, int* grp_ncid)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_ncid(getnc4id(d4info), name, grp_ncid);
    return (ret);
}

static int
NCD4_show_metadata(int ncid)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_show_metadata(getnc4id(d4info));
    return (ret);
}

static int
NCD4_inq_grps(int ncid, int* p2, int* p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_grps(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_inq_grpname(int ncid, char* p)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_grpname(getnc4id(d4info), p);
    return (ret);
}


static int
NCD4_inq_unlimdims(int ncid, int* p2, int* p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_unlimdims(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_inq_grpname_full(int ncid, size_t* p2, char* p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_grpname_full(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_inq_grp_parent(int ncid, int* p)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_grp_parent(getnc4id(d4info), p);
    return (ret);
}

static int
NCD4_inq_grp_full_ncid(int ncid, const char* p2, int* p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_grp_full_ncid(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_inq_varids(int ncid, int* nvars, int* p)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_varids(getnc4id(d4info), nvars, p);
    return (ret);
}

static int
NCD4_inq_dimids(int ncid, int* ndims, int* p3, int p4)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_dimids(getnc4id(d4info), ndims, p3, p4);
    return (ret);
}

static int
NCD4_inq_typeids(int ncid, int*  ntypes, int* p)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_typeids(getnc4id(d4info), ntypes, p);
    return (ret);
}

static int
NCD4_inq_type_equal(int ncid, nc_type t1, int p3, nc_type t2, int* p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_type_equal(getnc4id(d4info), t1, p3, t2, p5);
    return (ret);
}

static int
NCD4_inq_user_type(int ncid, nc_type t, char* p3, size_t* p4, nc_type* p5,
                   size_t* p6, int* p7)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_user_type(getnc4id(d4info), t, p3, p4, p5, p6, p7);
    return (ret);
}

static int
NCD4_inq_typeid(int ncid, const char* name, nc_type* t)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_typeid(getnc4id(d4info), name, t);
    return (ret);
}

static int
NCD4_def_grp(int ncid, const char* p2, int* p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_grp(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_rename_grp(int ncid, const char* p)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_rename_grp(getnc4id(d4info), p);
    return (ret);
}

static int
NCD4_def_compound(int ncid, size_t p2, const char* p3, nc_type* t)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_compound(getnc4id(d4info), p2, p3, t);
    return (ret);
}

static int
NCD4_insert_compound(int ncid, nc_type t1, const char* p3, size_t p4, nc_type t2)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_insert_compound(getnc4id(d4info), t1, p3, p4, t2);
    return (ret);
}

static int
NCD4_insert_array_compound(int ncid, nc_type t1, const char* p3, size_t p4,
			  nc_type t2, int p6, const int* p7)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_insert_array_compound(getnc4id(d4info), t1, p3, p4,  t2, p6, p7);
    return (ret);
}

static int
NCD4_inq_compound_field(int ncid, nc_type xtype, int fieldid, char *name,
		      size_t *offsetp, nc_type* field_typeidp, int *ndimsp,
		      int *dim_sizesp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_compound_field(getnc4id(d4info), xtype, fieldid, name, offsetp, field_typeidp, ndimsp, dim_sizesp);
    return (ret);
}

static int
NCD4_inq_compound_fieldindex(int ncid, nc_type xtype, const char *name,
			   int *fieldidp)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_compound_fieldindex(getnc4id(d4info), xtype, name, fieldidp);
    return (ret);
}

static int
NCD4_def_vlen(int ncid, const char* p2, nc_type base_typeid, nc_type* t)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_vlen(getnc4id(d4info), p2, base_typeid, t);
    return (ret);
}

static int
NCD4_put_vlen_element(int ncid, int p2, void* p3, size_t p4, const void* p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_put_vlen_element(getnc4id(d4info), p2, p3, p4, p5);
    return (ret);
}

static int
NCD4_get_vlen_element(int ncid, int p2, const void* p3, size_t* p4, void* p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_get_vlen_element(getnc4id(d4info), p2, p3, p4, p5);
    return (ret);
}

static int
NCD4_def_enum(int ncid, nc_type t1, const char* p3, nc_type* t)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_enum(getnc4id(d4info), t1, p3, t);
    return (ret);
}

static int
NCD4_insert_enum(int ncid, nc_type t1, const char* p3, const void* p4)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_insert_enum(getnc4id(d4info), t1, p3, p4);
    return (ret);
}

static int
NCD4_inq_enum_member(int ncid, nc_type t1, int p3, char* p4, void* p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_enum_member(getnc4id(d4info), t1, p3, p4, p5);
    return (ret);
}

static int
NCD4_inq_enum_ident(int ncid, nc_type t1, long long p3, char* p4)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_inq_enum_ident(getnc4id(d4info), t1, p3, p4);
    return (ret);
}

static int
NCD4_def_opaque(int ncid, size_t p2, const char* p3, nc_type* t)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_opaque(getnc4id(d4info), p2, p3, t);
    return (ret);
}

static int
NCD4_def_var_deflate(int ncid, int p2, int p3, int p4, int p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_var_deflate(getnc4id(d4info), p2, p3, p4, p5);
    return (ret);
}

static int
NCD4_def_var_fletcher32(int ncid, int p2, int p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_var_fletcher32(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_def_var_chunking(int ncid, int p2, int p3, const size_t* p4)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_var_chunking(getnc4id(d4info), p2, p3, p4);
    return (ret);
}

static int
NCD4_def_var_fill(int ncid, int p2, int p3, const void* p4)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_var_fill(getnc4id(d4info), p2, p3, p4);
    return (ret);
}

static int
NCD4_def_var_endian(int ncid, int p2, int p3)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_def_var_endian(getnc4id(d4info), p2, p3);
    return (ret);
}

static int
NCD4_set_var_chunk_cache(int ncid, int p2, size_t p3, size_t p4, float p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_set_var_chunk_cache(getnc4id(d4info), p2, p3, p4, p5);
    return (ret);
}

static int
NCD4_get_var_chunk_cache(int ncid, int p2, size_t* p3, size_t* p4, float* p5)
{
    NC* d4info;
    int ret;
    if((ret = NC_check_id(ncid, (NC**)&d4info)) != NC_NOERR) return (ret);
    ret = nc_get_var_chunk_cache(getnc4id(d4info), p2, p3, p4, p5);
    return (ret);
}

#endif // USE_NETCDF4

int
NCDAP4_ping(const char* url)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    CURL* curl = NULL;
    NCbytes* buf = NULL;

    /* Create a CURL instance */
    stat = NCD4_curlopen(&curl);
    if(stat != NC_NOERR) return stat;    

    /* Use redirects */
    cstat = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    if (cstat != CURLE_OK)
        goto done;
    cstat = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (cstat != CURLE_OK)
        goto done;

    /* use a very short timeout: 5 seconds */
    cstat = curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)5);
    if (cstat != CURLE_OK)
        goto done;

    /* fail on HTTP 400 code errors */
    cstat = curl_easy_setopt(curl, CURLOPT_FAILONERROR, (long)1);
    if (cstat != CURLE_OK)
        goto done;

    /* Try to get the file */
    buf = ncbytesnew();
    stat = NCD4_fetchurl(curl,url,buf,NULL,NULL);
    if(stat == NC_NOERR) {
	/* Don't trust curl to return an error when request gets 404 */
	long http_code = 0;
	cstat = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &http_code);
        if (cstat != CURLE_OK)
            goto done;
	if(http_code >= 400) {
	    cstat = CURLE_HTTP_RETURNED_ERROR;
	    goto done;
	}
    } else
        goto done;

done:
    ncbytesfree(buf);
    NCD4_curlclose(curl);
    if(cstat != CURLE_OK) {
        nclog(NCLOGERR, "curl error: %s", curl_easy_strerror(cstat));
        stat = NC_EDAPSVC;
    }
    return THROW(stat);
}

static int
globalinit(void)
{
    int stat = NC_NOERR;
    if(NCD4_globalstate != NULL) return stat;
    NCD4_globalstate = (NCD4globalstate*)calloc(1,sizeof(NCD4globalstate));
    if(NCD4_globalstate == NULL) {
	nclog(NCLOGERR, "Out of memory");
	return stat;
    }

    /* Capture temp dir*/
    {
	char* tempdir;
	char* p;
	char* q;
	char cwd[NC_MAX_PATH];
#if defined(_WIN32) || defined(_WIN64)
        tempdir = getenv("TEMP");
#else
	tempdir = "/tmp";
#endif
        if(tempdir == NULL) {
	    fprintf(stderr,"Cannot find a temp dir; using ./\n");
	    tempdir = getcwd(cwd,sizeof(cwd));
	    if(tempdir == NULL || *tempdir == '\0') tempdir = ".";
	}
        NCD4_globalstate->tempdir= (char*)malloc(strlen(tempdir) + 1);
	for(p=tempdir,q=NCD4_globalstate->tempdir;*p;p++,q++) {
	    if((*p == '/' && *(p+1) == '/')
	       || (*p == '\\' && *(p+1) == '\\')) {p++;}
	    *q = *p;
	}
	*q = '\0';
#if defined(_WIN32) || defined(_WIN64)
#else
        /* Canonicalize */
	for(p=NCD4_globalstate->tempdir;*p;p++) {
	    if(*p == '\\') {*p = '/'; };
	}
	*q = '\0';
#endif
    }

    /* Capture $HOME */
    {
	char* p;
	char* q;
        char* home = getenv("HOME");

        if(home == NULL) {
	    /* use tempdir */
	    home = NCD4_globalstate->tempdir;
	}
        NCD4_globalstate->home = (char*)malloc(strlen(home) + 1);
	for(p=home,q=NCD4_globalstate->home;*p;p++,q++) {
	    if((*p == '/' && *(p+1) == '/')
	       || (*p == '\\' && *(p+1) == '\\')) {p++;}
	    *q = *p;
	}
	*q = '\0';
#if defined(_WIN32) || defined(_WIN64)
#else
        /* Canonicalize */
	for(p=home;*p;p++) {
	    if(*p == '\\') {*p = '/'; };
	}
#endif
    }

    {
	CURLcode cstat = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(cstat != CURLE_OK)
	    fprintf(stderr,"curl_global_init failed!\n");
    }
    NCD4_curl_protocols(NCD4_globalstate); /* see what protocols are supported */
    return stat;
}

/**************************************************/

static NC_Dispatch NCD4_dispatch_base = {

NC_FORMATX_DAP4,

NCD4_create,
NCD4_open,

NCD4_redef,
NCD4__enddef,
NCD4_sync,
NCD4_abort,
NCD4_close,
NCD4_set_fill,
NCD4_inq_base_pe,
NCD4_set_base_pe,
NCD4_inq_format,
NCD4_inq_format_extended, /*inq_format_extended*/

NCD4_inq,
NCD4_inq_type,

NCD4_def_dim,
NCD4_inq_dimid,
NCD4_inq_dim,
NCD4_inq_unlimdim,
NCD4_rename_dim,

NCD4_inq_att,
NCD4_inq_attid,
NCD4_inq_attname,
NCD4_rename_att,
NCD4_del_att,
NCD4_get_att,
NCD4_put_att,

NCD4_def_var,
NCD4_inq_varid,
NCD4_rename_var,
NCD4_get_vara,
NCD4_put_vara,
NCD4_get_vars,
NCD4_put_vars,
NCDEFAULT_get_varm,
NCDEFAULT_put_varm,

NCD4_inq_var_all,

NCD4_var_par_access,

#ifdef USE_NETCDF4
NCD4_show_metadata,
NCD4_inq_unlimdims,
NCD4_inq_ncid,
NCD4_inq_grps,
NCD4_inq_grpname,
NCD4_inq_grpname_full,
NCD4_inq_grp_parent,
NCD4_inq_grp_full_ncid,
NCD4_inq_varids,
NCD4_inq_dimids,
NCD4_inq_typeids,
NCD4_inq_type_equal,
NCD4_def_grp,
NCD4_rename_grp,
NCD4_inq_user_type,
NCD4_inq_typeid,

NCD4_def_compound,
NCD4_insert_compound,
NCD4_insert_array_compound,
NCD4_inq_compound_field,
NCD4_inq_compound_fieldindex,
NCD4_def_vlen,
NCD4_put_vlen_element,
NCD4_get_vlen_element,
NCD4_def_enum,
NCD4_insert_enum,
NCD4_inq_enum_member,
NCD4_inq_enum_ident,
NCD4_def_opaque,
NCD4_def_var_deflate,
NCD4_def_var_fletcher32,
NCD4_def_var_chunking,
NCD4_def_var_fill,
NCD4_def_var_endian,
NCD4_set_var_chunk_cache,
NCD4_get_var_chunk_cache,

#endif /*USE_NETCDF4*/

};


