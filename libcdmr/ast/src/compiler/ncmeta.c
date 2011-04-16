#include "ncmeta.h"


int
nc_dim_new(char* name, NC_VAR_INFO_T* var, NC_DIM_INFO_T** memp)
{
    NC_DIM_INFO_T* mem = calloc(1,sizeof(NC_DIM_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(var != NULL) nc_var_add_dim(var,mem);
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_dim_set(NC_DIM_INFO_T* dim, size_t len, int dimid, int unlimited)
{
    dim->len = len;
    dim->dimid = dimid;
    dim->unlimited = unlimited;
    return NC_NOERR;
}

int
nc_dim_get(NC_DIM_INFO_T* dim, size_t* lenp, int* dimidp, int* unlimitedp)
{
    if(lenp) *lenp = dim->len;
    if(dimidp) *dimidp = dim->dimid;
    if(unlimitedp) *unlimitedp = dim->unlimited;
    return NC_NOERR;
}

int
nc_att_new(char* name, NC_META_COMMON_T* list, NC_ATT_INFO_T** memp)
{
    NC_ATT_INFO_T* mem = calloc(1,sizeof(NC_ATT_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(list != NULL) nc_list_add(list,(NC_META_COMMON_T*)mem);
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_att_set(NC_ATT_INFO_T*, nc_type xtype, int attnum, int klass)
{
    return NC_NOERR;
}

int
nc_att_get(NC_ATT_INFO_T*, nc_type* xtype, int* attnum, int* klass)
{
    return NC_NOERR;
}


int
nc_var_new(char* name, NC_META_COMMON_T* list, NC_VAR_INFO_T** memp)
{
    NC_VAR_INFO_T* mem = calloc(1,sizeof(NC_VAR_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(list != NULL) nc_list_add(list,(NC_META_COMMON_T*)mem);
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_var_set(NC_VAR_INFO_T*, NC_TYPE_INFO_T*, int nofill, void* fill_value)
{
    return NC_NOERR;
}

int
nc_var_get(NC_VAR_INFO_T*, NC_TYPE_INFO_T*, int* nofill, void** fill_value)
{
    return NC_NOERR;
}

    extern int nc_var_add_dim(NC_VAR_INFO_T*, NC_DIM_INFO_T* dim);
    extern int nc_var_add_att(NC_VAR_INFO_T*, NC_ATT_INFO_T* att);

int
nc_field_new(char* name, NC_TYPE_INFO_T* cmpd, NC_FIELD_INFO_T** memp)
{
    NC_FIELD_INFO_T* mem = calloc(1,sizeof(NC_FIELD_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(cmpd != NULL) nc_type_add_field(cmpd,mem);
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_field_get(NC_FIELD_INFO_T*, size_t* offset, int* fieldid)
{
    return NC_NOERR;
}

int
nc_field_set(NC_FIELD_INFO_T*, size_t offset, int fieldid)
{
    return NC_NOERR;
}

    extern int nc_field_add_dim(NC_VAR_INFO_T*, int size);

int
nc_econst_new(char* name, NC_TYPE_INFO_T* enumtyp, NC_ECONST_INFO_T** econstp)
{
    NC_ECONST_INFO_T* mem = calloc(1,sizeof(NC_ECONST_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(enumtyp != NULL) nc_type_add_econst(enumtyp,mem);
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_econst_set(NC_ECONST_INFO_T*, void* value)
{
    return NC_NOERR;
}

int
nc_econst_get(NC_ECONST_INFO_T*, void** value)
{
    return NC_NOERR;
}


int
nc_type_new(char* name, NC_COMMON_INFO_T* list, NC_TYPE_INFO_T** typep)
{
    NC_TYPE_INFO_T* mem = calloc(1,sizeof(NC_TYPE_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(list != NULL) nc_list_add(list,(NC_META_COMMON_T*)mem);
    if( memp) * memp = mem;
    return NC_NOERR;
}

int
nc_type_set(NC_TYPE_INFO_T*, nc_type typeid, nc_type klass, NC_TYPE_INFO_T* basetype)
{
    return NC_NOERR;
}

int
nc_type_get(NC_TYPE_INFO_T*, nc_type* typeid, nc_type* klass, NC_TYPE_INFO_T* basetype)
{
    return NC_NOERR;
}

    extern int nc_type_add_field(NC_TYPE_INFO_T*, NC_FIELD_INFO_T*);
    extern int nc_type_add_econst(NC_TYPE_INFO_T*, NC_ECONST_INFO_T*);

int
nc_group_new(char* name, NC_GROUP_INFO_T* parent, NC_GROUP_INFO_T** memp)
{
    NC_GROUP_INFO_T* mem = calloc(1,sizeof(NC_GROUP_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(parent != NULL) {
??	nc_list_add(var,(NC_META_COMMON_T*)mem);
    {
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_group_set(NC_GROUP_INFO_T*, int groupid, NC_FILE_INFO_T* file)
{
    return NC_NOERR;
}

int
nc_group_get(NC_GROUP_INFO_T*, int* groupid, NC_FILE_INFO_T* file)
{
    return NC_NOERR;
}

    extern int nc_type_add_var(NC_TYPE_INFO_T*, NC_VAR_INFO_T*);
    extern int nc_type_add_dim(NC_TYPE_INFO_T*, NC_DIM_INFO_T*);
    extern int nc_type_add_att(NC_TYPE_INFO_T*, NC_ATT_INFO_T*);

int
nc_file_new(char* name, NC_META_COMMON_T* list, NC_FILE_INFO_T** filep)
{
    NC_FILE_INFO_T* mem = calloc(1,sizeof(NC_FILE_INFO_T));
    if(mem == NULL) return NC_ENOMEM;
    if(list != NULL) nc_list_add(list,(NC_META_COMMON_T*)mem);
    if(memp) *memp = mem;
    return NC_NOERR;
}

int
nc_file_set(NC_FILE_INFO_T*, NC_GROUP_INFO_T* root int fillmode)
{
    return NC_NOERR;
}

int
nc_file_get(NC_FILE_INFO_T*, NC_GROUP_INFO_T** root int* fillmode)
{
    return NC_NOERR;
}

int
nc_type_add_group(NC_TYPE_INFO_T*, NC_GROUP_INFO_T*)
{
    return NC_NOERR;
}

