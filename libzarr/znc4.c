/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "csincludes.h"

/**************************************************/
/*
Following functions basically return the netcdf-4 value WRT to the nc4id.
However, it is necessary to modify the grpid(ncid) to point to the substrate.
*/

int
NCCS_inq_base_pe(int ncid, int* pe)
{
    NC* ncp;
    int ret = NC_NOERR;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_base_pe(substrateid, pe);
    return (ret);
}

int
NCCS_inq_format(int ncid, int* formatp)
{
    NC* ncp;
    int ret = NC_NOERR;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_format(substrateid, formatp);
    return (ret);
}

int
NCCS_inq(int ncid, int* ndimsp, int* nvarsp, int* nattsp, int* unlimdimidp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq(substrateid, ndimsp, nvarsp, nattsp, unlimdimidp);
    return (ret);
}

int
NCCS_inq_type(int ncid, nc_type p2, char* p3, size_t* p4)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_type(substrateid, p2, p3, p4);
    return (ret);
}

int
NCCS_inq_dimid(int ncid, const char* name, int* idp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_dimid(substrateid, name, idp);
    return (ret);
}

int
NCCS_inq_unlimdim(int ncid, int* unlimdimidp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_unlimdim(substrateid, unlimdimidp);
    return (ret);
}

int
NCCS_inq_dim(int ncid, int dimid, char* name, size_t* lenp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_dim(substrateid, dimid, name, lenp);
    return (ret);
}

int
NCCS_rename_dim(int ncid, int dimid, const char* name)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_rename_dim(substrateid, dimid, name);
    return (ret);
}

int
NCCS_inq_att(int ncid, int varid, const char* name,
	    nc_type* xtypep, size_t* lenp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_att(substrateid, varid, name, xtypep, lenp);
    return (ret);
}

int
NCCS_inq_attid(int ncid, int varid, const char *name, int *idp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_attid(substrateid, varid, name, idp);
    return (ret);
}

int
NCCS_inq_attname(int ncid, int varid, int attnum, char* name)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_attname(substrateid, varid, attnum, name);
    return (ret);
}

int
NCCS_rename_att(int ncid, int varid, const char* name, const char* newname)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_rename_att(substrateid, varid, name, newname);
    return (ret);
}

int
NCCS_del_att(int ncid, int varid, const char* p3)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_del_att(substrateid, varid, p3);
    return (ret);
}

int
NCCS_get_att(int ncid, int varid, const char* name, void* value, nc_type t)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = NCDISPATCH_get_att(substrateid, varid, name, value, t);
    return (ret);
}

int
NCCS_inq_var_all(int ncid, int varid, char *name, nc_type* xtypep,
               int* ndimsp, int* dimidsp, int* nattsp,
               int* shufflep, int* deflatep, int* deflate_levelp,
               int* fletcher32p, int* contiguousp, size_t* chunksizesp,
               int* no_fill, void* fill_valuep, int* endiannessp,
	       unsigned int* idp, size_t* nparamsp, unsigned int* params
               )
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = NCDISPATCH_inq_var_all(substrateid, varid, name, xtypep,
               ndimsp, dimidsp, nattsp,
               shufflep, deflatep, deflate_levelp,
               fletcher32p, contiguousp, chunksizesp,
               no_fill, fill_valuep, endiannessp,
               idp, nparamsp, params);
    return (ret);
}

int
NCCS_inq_varid(int ncid, const char *name, int *varidp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_varid(substrateid,name,varidp);
    return (ret);
}

int
NCCS_rename_var(int ncid, int varid, const char* name)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_rename_var(substrateid, varid, name);
    return (ret);
}

int
NCCS_var_par_access(int ncid, int p2, int p3)
{
    return (NC_ENOPAR);
}

int
NCCS_inq_ncid(int ncid, const char* name, int* grp_ncid)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_ncid(substrateid, name, grp_ncid);
    return (ret);
}

int
NCCS_show_metadata(int ncid)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_show_metadata(substrateid);
    return (ret);
}

int
NCCS_inq_grps(int ncid, int* ngrpsp, int* grpids)
{
    NC* ncp;
    int ret;
    int substrateid;
    int ngrps;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    /* We always need to know |grpids| */
    ret = nc_inq_grps(substrateid, &ngrps, grpids);
    if(ret == NC_NOERR) {
	if(ngrpsp != NULL)
	    *ngrpsp = ngrps; /* return if caller want it */
	if(grpids != NULL) {
	    int i;
	    /* We need to convert the substrate group ids to dap4 group ids */
	    for(i=0;i<ngrps;i++)
		grpids[i] = makenc4id(ncp,grpids[i]);
	}
    }
    return (ret);
}

int
NCCS_inq_grpname(int ncid, char* p)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_grpname(substrateid, p);
    return (ret);
}


int
NCCS_inq_unlimdims(int ncid, int* p2, int* p3)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_unlimdims(substrateid, p2, p3);
    return (ret);
}

int
NCCS_inq_grpname_full(int ncid, size_t* p2, char* p3)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_grpname_full(substrateid, p2, p3);
    return (ret);
}

int
NCCS_inq_grp_parent(int ncid, int* p)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_grp_parent(substrateid, p);
    if(p != NULL)
	*p = makenc4id(ncp,*p);
    return (ret);
}

int
NCCS_inq_grp_full_ncid(int ncid, const char* fullname, int* grpidp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_grp_full_ncid(substrateid, fullname, grpidp);
    if(grpidp != NULL)
	*grpidp = makenc4id(ncp,*grpidp);
    return (ret);
}

int
NCCS_inq_varids(int ncid, int* nvars, int* p)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_varids(substrateid, nvars, p);
    return (ret);
}

int
NCCS_inq_dimids(int ncid, int* ndims, int* p3, int p4)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_dimids(substrateid, ndims, p3, p4);
    return (ret);
}

int
NCCS_inq_typeids(int ncid, int*  ntypes, int* p)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_typeids(substrateid, ntypes, p);
    return (ret);
}

int
NCCS_inq_type_equal(int ncid, nc_type t1, int p3, nc_type t2, int* p5)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_type_equal(substrateid, t1, p3, t2, p5);
    return (ret);
}

int
NCCS_inq_user_type(int ncid, nc_type t, char* p3, size_t* p4, nc_type* p5,
                   size_t* p6, int* p7)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_user_type(substrateid, t, p3, p4, p5, p6, p7);
    return (ret);
}

int
NCCS_inq_typeid(int ncid, const char* name, nc_type* t)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_typeid(substrateid, name, t);
    return (ret);
}

int
NCCS_inq_compound_field(int ncid, nc_type xtype, int fieldid, char *name,
		      size_t *offsetp, nc_type* field_typeidp, int *ndimsp,
		      int *dim_sizesp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_compound_field(substrateid, xtype, fieldid, name, offsetp, field_typeidp, ndimsp, dim_sizesp);
    return (ret);
}

int
NCCS_inq_compound_fieldindex(int ncid, nc_type xtype, const char *name,
			   int *fieldidp)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_compound_fieldindex(substrateid, xtype, name, fieldidp);
    return (ret);
}

int
NCCS_get_vlen_element(int ncid, int p2, const void* p3, size_t* p4, void* p5)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_get_vlen_element(substrateid, p2, p3, p4, p5);
    return (ret);
}

int
NCCS_inq_enum_member(int ncid, nc_type t1, int p3, char* p4, void* p5)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_enum_member(substrateid, t1, p3, p4, p5);
    return (ret);
}

int
NCCS_inq_enum_ident(int ncid, nc_type t1, long long p3, char* p4)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_inq_enum_ident(substrateid, t1, p3, p4);
    return (ret);
}

int
NCCS_get_var_chunk_cache(int ncid, int p2, size_t* p3, size_t* p4, float* p5)
{
    NC* ncp;
    int ret;
    int substrateid;
    if((ret = NC_check_id(ncid, (NC**)&ncp)) != NC_NOERR) return (ret);
    substrateid = makenc4id(ncp,ncid);
    ret = nc_get_var_chunk_cache(substrateid, p2, p3, p4, p5);
    return (ret);
}

/**************************************************/
/*
Following functions are overridden to handle 
nc cloud implementation specific issues.
*/

/* Force specific format */
int
NCCS_inq_format_extended(int ncid, int* formatp, int* modep)
{
    NC* nc;
    int ncstatus = NC_check_id(ncid, (NC**)&nc);
    if(ncstatus != NC_NOERR) return (ncstatus);
    if(modep) *modep = nc->mode;
    if(formatp) *formatp = NC_FORMATX_CLOUD;
    return THROW(NC_NOERR);
}

