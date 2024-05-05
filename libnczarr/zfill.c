/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "zfill.h"


/**************************************************/
/* Forward */

/**************************************************/

/* (over-) write the NC_VAR_INFO_T.fill_value */
int
NCZ_set_fill_value(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, int no_fill, const void* fillvalue)
{
    int stat = NC_NOERR;
    size_t size;
    nc_type tid = var->type_info->hdr.id;
    int overwrite = (var->fill_value && var->fill_value == fillvalue?1:0);

    if(no_fill) {
	stat = NCZ_fillvalue_disable(file,var);
	goto done;
    }

    if ((stat = nc4_get_typelen_mem(file, tid, &size))) goto done;
    assert(size);

    /* It is possible that the caller is trying to set var->fill_value from var->fill_value,
       so do not reclaim */
    if(!overwrite) {
        if((stat = NCZ_reclaim_var_fillvalue(file,var))) goto done;
    }
    if(fillvalue == NULL) {/* use default fill value */
	/* initialize the fill_value to the default */
	/* Allocate the fill_value space. */
        if((var->fill_value = calloc(1, size))==NULL) {stat = NC_ENOMEM; goto done;}
        if((stat = nc4_get_default_fill_value(var->type_info,var->fill_value))) goto done;
        var->fill_val_changed = 0;
    } else if(!overwrite) {
        /* insert the fill value */
        if((stat = NCZ_copy_value_to_var_fillvalue(file,var,fillvalue))) goto done;
	var->fill_val_changed = 1;
    }
    var->no_fill = 0;
    stat = NCZ_reclaim_fill_chunk(((NCZ_VAR_INFO_T*)var->format_var_info)->cache); /* Reclaim any existing fill_chunk */
    
done:
    return THROW(stat);
}

/* (over-) write/create the _FillValue attribute */
int
NCZ_set_fill_att(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NC_ATT_INFO_T* att, int no_fill, const void* fillvalue)
{
    int stat = NC_NOERR;
    if(no_fill) {
	stat = NCZ_fillvalue_disable(file,var);
    } else {
        if(att == NULL) {
            if((stat = ncz_makeattr(file,(NC_OBJ*)var,NC_FillValue,var->type_info->hdr.id,0,NULL,&att))) goto done;
	}
        assert(att != NULL && strcmp(att->hdr.name,NC_FillValue)==0); /* Verify */
        if((stat = NCZ_copy_value_to_att(file,att,1,fillvalue))) goto done;
	var->no_fill = NC_FALSE;
	var->fill_val_changed = 1;
    }

done:
    return THROW(stat);
}

/* Sync from NC_VAR_INFO_T.fill_value to attribute _FillValue */
int
NCZ_copy_var_to_fillatt(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NC_ATT_INFO_T* att)
{
    int stat = NC_NOERR;

    if(var->no_fill) {
	/* disable fill value */
        stat = NCZ_fillvalue_disable(file,var);
    } else if(att == NULL) {
        stat = ncz_makeattr(file,(NC_OBJ*)var,NC_FillValue,var->type_info->hdr.id,1,var->fill_value,NULL);
    } else { /* presumably already exists */
	assert(strcmp(NC_FillValue,att->hdr.name)==0);
	stat = NCZ_copy_value_to_att(file,att,1,var->fill_value);
    }
    return THROW(stat);
}

/* Sync from Attribute_FillValue to NC_VAR_INFO_T.fill_value */
int
NCZ_copy_fillatt_to_var(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;

    if(att == NULL) {
        /* The att _FillValue must exist */
        att = (NC_ATT_INFO_T*)ncindexlookup(var->att,NC_FillValue);
    }
    assert(var != NULL && att != NULL);
    assert(strcmp(NC_FillValue,att->hdr.name)==0 && att->len == 1);
    if((stat = NCZ_copy_value_to_var_fillvalue(file,var,att->data))) goto done;
    var->fill_val_changed = 1;
    var->no_fill = 0;
    /* Reclaim any existing fill_chunk */
    stat = NCZ_reclaim_fill_chunk(((NCZ_VAR_INFO_T*)var->format_var_info)->cache);
        
done:
    return THROW(stat);
}

/* Turn off FillValue */
int
NCZ_fillvalue_disable(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;

    if(!var->no_fill) var->fill_val_changed = 1;
    var->no_fill = 1;
    /* Reclaim the fill value */
    if((stat = NCZ_reclaim_var_fillvalue(file,var))) return stat;
    stat = NCZ_reclaim_fill_chunk(((NCZ_VAR_INFO_T*)var->format_var_info)->cache); /* Reclaim any existing fill_chunk */
    stat = NCZ_attr_delete(file,var->att,NC_FillValue);
    if (stat && stat != NC_ENOTATT) return stat; else stat = NC_NOERR;
    return stat;    
}

/**************************************************/
/* Basic operations are:
1. copy from a src to att->data
2. copy from a src to var->fill_value.
3. reclaim and clear data in att->data
4. reclaim and clear data in var->fill_value
*/

int
NCZ_copy_value_to_att(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, size_t len, const void* src)
{
    int stat = NC_NOERR;
    assert(att != NULL);
    if((stat = NCZ_reclaim_att_data(file, att))) goto done; /* reclaim old data */
    /* Now fill att->data */
    if((stat = NC_copy_data_all(file->controller,att->nc_typeid,src,len,&att->data))) goto done;
    att->len = len;
done:
    return stat;
}

int
NCZ_copy_value_to_var_fillvalue(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const void* src)
{
    int stat = NC_NOERR;
    nc_type tid = var->type_info->hdr.id;
    assert(var != NULL);
    if((stat = NCZ_reclaim_var_fillvalue(file, var))) goto done; /* reclaim old data */
    /* Now fill var->fill_value */
    if((stat = NC_copy_data_all(file->controller,tid,src,1,&var->fill_value))) goto done;
    
done:
    return stat;
}

int
NCZ_reclaim_att_data(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att)
{
    int stat = NC_NOERR;
    int tid = att->nc_typeid;

    if(att->data != NULL) {
	stat = NC_reclaim_data_all(file->controller,tid,att->data,att->len);
	att->data = NULL;
	att->len = 0;
    }
    return stat;
}

int
NCZ_reclaim_var_fillvalue(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    int tid = var->type_info->hdr.id;

    if(var->fill_value != NULL) {
	stat = NC_reclaim_data_all(file->controller,tid,var->fill_value,1);
	var->fill_value = NULL;
	/* Leave var->no_fill as is */
    }
    return stat;
}

