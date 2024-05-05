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
ncz4_create_var(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent,
				const char* name,
				nc_type nctype,
				int storage,
				int scalar,
				int endianness,
				size_t maxstrlen,
				int dimsep,
				char order,
			        size_t ndims,
				size64_t* shape,
				size64_t* chunksizes,
				int* dimids,
			        NClist* filters,
				int no_fill,
				void* fill_value,
			        NC_VAR_INFO_T** varp)
{
    int stat = NC_NOERR;
    char norm_name[NC_MAX_NAME];
    NC_VAR_INFO_T* var = NULL;
    NCZ_VAR_INFO_T* zvar = NULL;
    NC_TYPE_INFO_T* typ = NULL;
    char* dimbasename = NULL;

    NC_UNUSED(order);
    
    /* Check and normalize the name. */
    if((stat = nc4_check_name(name, norm_name))) goto done;

    if(nctype <= NC_NAT || nctype > NC_MAX_ATOMIC_TYPE) {stat = NC_EBADTYPE; goto done;}
    /* Locate the NC_TYPE_INFO_T object */
    if((stat = ncz_gettype(file,parent,nctype,&typ))) goto done;

    if((stat = nc4_var_list_add2(parent, norm_name, &var))) goto done;

    if((stat = NCZ_fillin_var(file, var, typ, ndims, dimids, shape, chunksizes, endianness))) goto done;
    zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    
    /* Overrides */
    zvar->dimension_separator = (char)dimsep;
    zmaxstrlen(&zvar->maxstrlen,maxstrlen);

    /* Set fill value */
    if((stat=NCZ_set_fill_value(file,var,no_fill,fill_value))) goto done;

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Set the filter list */
    {
        size_t k, nfilters;
	NClist* varfilters = (NClist*)var->filters;
	nfilters = nclistlength(filters);
	for(k=0;k<nfilters;k++) {
	    NCZ_Filter* fi = (NCZ_Filter*)nclistremove(filters,0);
	    assert(fi != NULL);
	    nclistpush(varfilters,fi);
	}
        /* At this point, we can finalize the filters */
        if((stat = NCZ_filter_setup(var))) goto done;
    }
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/

    if(varp) {*varp = var; var = NULL;}

done:
     nullfree(dimbasename);
     return THROW(stat);
}

int
ncz4_create_dim(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, size64_t len, int isunlimited,
			   NC_DIM_INFO_T** dimp)
{
    int stat = NC_NOERR;
    NC_DIM_INFO_T* dim = NULL;
    NCZ_DIM_INFO_T* zdim = NULL;
    if((stat = nc4_dim_list_add(parent, name, (size_t)len, -1, &dim))) goto done;
    dim->unlimited = (isunlimited ? 1 : 0);
    if((zdim = calloc(1,sizeof(NCZ_DIM_INFO_T))) == NULL) {stat = NC_ENOMEM; goto done;}
    zdim->common.file = file;
    dim->format_dim_info = zdim;
    if(dimp) *dimp = dim;
done:
    return THROW(stat);
}

/* This is an abbreviated form of ncz_put_att */
int
ncz4_create_attr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name,
		 nc_type typeid, size_t len, void* values,
		 NC_ATT_INFO_T** attp)
{
    /* Defer to zattr.c */
    return ncz_makeattr(file,container,name,typeid,len,values,attp);
}

#ifdef NETCDF_ENABLE_NCZARR_FILTERS
int
ncz4_create_filter(NC_FILE_INFO_T* file,
				NCZ_HDF5* hdf5,
				NCZ_Codec* codec,
				NClist* filterlist,
				NCZ_Filter** filterp)
{
    int stat = NC_NOERR;
    NCZ_Filter* filter = NULL;

    if((filter=(NCZ_Filter*)calloc(1,sizeof(NCZ_Filter)))==NULL) {stat = NC_ENOMEM; goto done;}
    if((stat = NCZ_insert_filter(file,filterlist,hdf5,codec,filter,0/*!exists*/))) goto done;

    if(filterp) {*filterp = filter; filter = NULL;}

done:
    NCZ_filter_free(filter);
    return THROW(stat);
}
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/
