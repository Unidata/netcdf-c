
/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Variable fill_values require some special effort.
Specifically, a variable's fill_value occurs in three places.
1. The NC_VAR_INFO_T object's "fill_value" field containing
   the netcdf-4 meta-data for a variable.
2. The _FillValue attribute
3. The "fill_value" key in the (NC)Zarr metadata for an array.

It is necessary to ensure that all three values are properly synchronized.
The fill_value is read/set in the following ways:
1. The nc_def_var_fill API function.
2. The nc_put_att API function when the attibute is set on a variable
   and the attribute name is "_FillValue".
3. Reading the fill_value key when opening an (NC)Zarr file.
4. Writing the fill_value key when creating an (NC)Zarr file.

The rules used here are as follows.
## Using nc_def_var_fill
1. If the no_fill argument is set, then
    1. reclaim the NC_VAR_INFO_T.fill_value_field
    2. set NC_VAR_INFO_T.fill_val_changed field.
2. If no_fill is not set, then
    1. Convert the data value argument to the same type as the variable's type.
    2. store the fill value argument into the NC_VAR_INFO_T.fill_value field (overwriting any existing value)
    3. set NC_VAR_INFO_T.fill_val_changed.
3. Synchronize
    1. Create or overwrite the _FillValue attribute to have same value
as NC_VAR_INFO_T.fill_value.

## Using nc_put_att when att is _FillValue
1. unset NC_VAR_INFO_T.no_fill field
2. create/overwrite the attribute
    1. Convert the attribute data to the same type as the variable's type.
    2. Create or overwrite the _FillValue attribute to the converted value
and type.
3. Synchronize
    1. store the attribute data (and type) into the NC_VAR_INFO_T.fill_value field (overwriting any existing value)
    2. set NC_VAR_INFO_T.fill_val_changed.

## Reading (NC)Zarr metadata
1. If the _FillValue attribute is encountered, then ignore it in favor
   of whatever the "fill_value" key dictates.
2. If the "fill_value" key in the variable's metadata does not exist
or is NULL, then
    1. set the NC_VAR_INFO_T.no_fill field
    2. unset the NC_VAR_INFO_T.fill_val_changed field.
3. If the "fill_value" key is defined, then
    1. compile the key's value to void*
    2. convert the compiled value to the variable's type
    3. store the attribute data (and type) into the NC_VAR_INFO_T.fill_value field (overwriting any existing value)
    4. set NC_VAR_INFO_T.fill_val_changed.
4. Syncronize
    1. Create or overwrite the _FillValue attribute to have same value
as NC_VAR_INFO_T.fill_value.

## Writing (NC)Zarr metadata
1. If the _FillValue attribute is defined, then ignore it in favor
   of whatever the NC_VAR_INFO_T.fill_value field dictates.
2. If NC_VAR_INFO_T.no_fill is set, then write the "fill_value"
key with a NULL value.
3. If NC_VAR_INFO_T.no_fill is not set, then
    1. convert the NC_VAR_INFO_T.fill_value field to the corresponding
JSON value
    2. write fill_value to the (NC)Zarr array metadata.
*/

#ifndef ZFILL_H
#define ZFILL_H

EXTERNL int NCZ_set_fill_value(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, int no_fill, const void* fillvalue);
EXTERNL int NCZ_set_fill_att(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NC_ATT_INFO_T* att, int no_fill, const void* fillvalue);
EXTERNL int NCZ_copy_var_to_fillatt(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NC_ATT_INFO_T* att);
EXTERNL int NCZ_copy_fillatt_to_var(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, NC_VAR_INFO_T* var);
EXTERNL int NCZ_fillvalue_disable(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
EXTERNL int NCZ_copy_value_to_att(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, size_t len, const void* src);
EXTERNL int NCZ_copy_value_to_var_fillvalue(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const void* src);
EXTERNL int NCZ_reclaim_att_data(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att);
EXTERNL int NCZ_reclaim_var_fillvalue(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);

#endif /*ZFILL_H*/

