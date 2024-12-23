/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "znc4.h"
#include "zfill.h"
#include "zformat.h"
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
#include "zfilter.h"
#endif

/**************************************************/

/* Forward */

/**************************************************/

int
ncz4_create_file(NC_FILE_INFO_T** filep)
{
    int stat = NC_NOERR;
    NC_UNUSED(filep);
    return THROW(stat);
}

int
ncz4_create_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, NC_GRP_INFO_T** grpp)
{
    int stat = NC_NOERR;
    char norm_name[NC_MAX_NAME];
    NC_GRP_INFO_T* grp = NULL;
    NCZ_GRP_INFO_T* zgrp = NULL;

    /* Check and normalize the name. */
    if((stat = nc4_check_name(name, norm_name))) goto done;
    if((stat = nc4_grp_list_add(file, parent, norm_name, &grp))) goto done;
    if((zgrp = calloc(1, sizeof(NCZ_GRP_INFO_T)))==NULL) {stat = NC_ENOMEM; goto done;}
    zgrp->common.file = file;
    grp->format_grp_info = zgrp;
    zgrp = NULL;
    grp->nc4_info = file;
    if(grpp) *grpp = grp;
done:
    return THROW(stat);
}

int
ncz4_create_var(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, NC_VAR_INFO_T** varp)
{
    int stat = NC_NOERR;
    char norm_name[NC_MAX_NAME];
    NC_VAR_INFO_T* var = NULL;
    NCZ_VAR_INFO_T* zvar = NULL;

    /* Check and normalize the name. */
    if((stat = nc4_check_name(name, norm_name))) goto done;
    if((stat = nc4_var_list_add2(parent, norm_name, &var))) goto done;
    if((zvar = calloc(1, sizeof(NCZ_VAR_INFO_T)))==NULL) {stat = NC_ENOMEM; goto done;}
    zvar->common.file = file;
    var->format_var_info = zvar;
    zvar = NULL;
    if(varp) *varp = var;
done:
    return THROW(stat);
}


int
ncz4_create_dim(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const struct NCZ_DimInfo* dimdef, NC_DIM_INFO_T** dimp)
{
    int stat = NC_NOERR;
    NC_DIM_INFO_T* dim = NULL;
    NCZ_DIM_INFO_T* zdim = NULL;
    if((stat = nc4_dim_list_add(parent, dimdef->norm_name, (size_t)dimdef->shape, -1, &dim))) goto done;
    dim->unlimited = (dimdef->unlimited ? 1 : 0);
    if((zdim = calloc(1,sizeof(NCZ_DIM_INFO_T))) == NULL) {stat = NC_ENOMEM; goto done;}
    zdim->common.file = file;
    dim->format_dim_info = zdim;
    if(dimp) *dimp = dim;
done:
    return THROW(stat);
}

#if 0
/* This is an abbreviated form of ncz_put_att */
int
ncz4_create_attr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name,
		 nc_type typeid, size_t len, void* values,
		 NC_ATT_INFO_T** attp)
{
    /* Defer to zattr.c */
    return ncz_makeattr(file,container,name,typeid,len,values,attp);
???    return ncz_makeattr(file,container,name,typeid,len,values,attp);
}
#endif /*0*/

#if 0
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
int
ncz4_create_filter(NC_FILE_INFO_T* file,
				NCZ_HDF5* hdf5,
				NCZ_Codec* codec,
				NCZ_Filter** filterp)
{
    int stat = NC_NOERR;
    NCZ_Filter* filter = NULL;

    if((filter=(NCZ_Filter*)calloc(1,sizeof(NCZ_Filter)))==NULL) {stat = NC_ENOMEM; goto done;}
??????

    if((stat = NCZ_addfilter(file,fvar,&filter))) goto done;

    if(filterp) {*filterp = filter; filter = NULL;}

done:
    NCZ_filter_free(filter);
    return THROW(stat);
}
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
#endif /*0*/
