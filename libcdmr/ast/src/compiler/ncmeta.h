/*
  This file is part of netcdf-4, a netCDF-like interface for HDF5, or a
  HDF5 backend for netCDF, depending on your point of view.

  This header file contains the definitions of structs used to hold
  netCDF file metadata in memory.

  Copyright 2005 University Corporation for Atmospheric Research/Unidata.

  $Id: nc4internal.h,v 1.137 2010/06/01 15:34:51 ed Exp $ */

#ifndef _NCMETA_
#define _NCMETA_

/* Define structure to hold netcdf-4 and netcdf-3 (as special case)
   meta-data; note that all non-meta information is elided
   and assumed to be in the annotation.
*/

/* Define a field to indicate the kind of structure */
typedef enum NC_META_SORT {
   NC_META_FILE,
   NC_META_GRP,
   NC_META_DIM,
   NC_META_ATT,
   NC_META_VAR,
   NC_META_ENUM,
   NC_META_ENUMVAL,
   NC_META_TYPE,
} NC_META_SORT ;

/* Define a struct to hold common information */
typedef struct NC_META_COMMON
{
   NC_META_SORT sort;
   struct NC_META_COMMON *next;
   struct NC_META_COMMON *prev;
   char *name;
   void* annotation;
} NC_META_COMMON_T;

/* This is a struct to handle the dim metadata. */
typedef struct NC_DIM_INFO
{
   NC_META_COMMON common;
   size_t len;
   int dimid;
   int unlimited;
   struct NC_VAR_INFO *coord_var; /* The coord var, if it exists. */
} NC_DIM_INFO_T;

typedef struct NC_ATT_INFO
{
   NC_META_COMMON common;
   int len;
   nc_type xtype;
   int attnum;
   int class;
} NC_ATT_INFO_T;

/* This is a struct to handle the var metadata. */
typedef struct NC_VAR_INFO
{
   NC_META_COMMON common;
   int ndims;
   NC_DIM_INFO_T **dim;
   int varid;
   int natts;
   struct NC_TYPE_INFO *type_info;
   nc_type xtype;
   NC_ATT_INFO_T *att;
   int no_fill;
   void *fill_value;
} NC_VAR_INFO_T;

typedef struct NC_FIELD_INFO
{
   NC_META_COMMON common;
   struct NC_TYPE_INFO* typ;
   nc_type nctype;
   size_t offset;
   int fieldid;
   int ndims;
   int *dim_size;
} NC_FIELD_INFO_T;

typedef struct NC_ECONST_INFO
{
   NC_META_COMMON common;
   void *value;
} NC_ECONST_INFO_T;

typedef struct NC_TYPE_INFO
{
   NC_META_COMMON common;
   nc_type nc_typeid;
   int class; /* NC_VLEN, NC_COMPOUND, NC_OPAQUE, or NC_ENUM */
   int num_enum_members;
   NC_ECONST_INFO_T *enum_member;
   NC_FIELD_INFO_T *field; /* Used for compound types. */
   int num_fields;
   nc_type base_nc_type; /* for VLEN and ENUM only */
} NC_TYPE_INFO_T;

/* This holds information for one group. Groups reproduce with
 * parthenogenesis. */
typedef struct NC_GROUP_INFO
{
   NC_META_COMMON common;
   int nc_grpid;
   struct NC_GROUP_INFO *parent;
   struct NC_GROUP_INFO *children;
   int nvars;
   NC_VAR_INFO_T *var;
   int ndims;
   NC_DIM_INFO_T *dim;
   int natts;
   NC_ATT_INFO_T *att;
   struct NC_FILE_INFO *file; /* parent group/file */
   NC_TYPE_INFO_T *type;
} NC_GROUP_INFO_T;


/* This is the metadata we need to keep track of for each
   netcdf-4 file. */
typedef struct NC_FILE_INFO
{
   NC_META_COMMON common;
   NC_GROUP_INFO_T *root_grp;
   NC_TYPE_INFO_T *type;
   short next_nc_grpid;
   int next_typeid;
   int next_dimid;
   int fill_mode;
} NC_FILE_INFO_T;

/* Define the new and get/put functions for the above structures */

extern int nc_dim_new(char* name, NC_VAR_INFO_T* var, NC_DIM_INFO_T** dimp);
extern int nc_dim_set(NC_DIM_INFO_T*, size_t len, int dimid, int unlimited);
extern int* nc_dim_get(NC_DIM_INFO_T*, size_t* len, int* dimid, int* unlimited);

extern int nc_att_new(char* name, NC_META_COMMON_T* list, NC_ATT_INFO_T** attp);
extern int nc_att_set(NC_ATT_INFO_T*, nc_type xtype, int attnum, int klass);
extern int* nc_att_get(NC_ATT_INFO_T*, nc_type* xtype, int* attnum, int* klass);

extern int nc_var_new(char* name, NC_META_COMMON_T* list, NC_VAR_INFO_T** varp);
extern int nc_var_set(NC_VAR_INFO_T*, NC_TYPE_INFO_T*, int nofill, void* fill_value);
extern int* nc_var_get(NC_VAR_INFO_T*, NC_TYPE_INFO_T*, int* nofill, void** fill_value);
    extern int nc_var_add_dim(NC_VAR_INFO_T*, NC_DIM_INFO_T* dim);
    extern int nc_var_add_att(NC_VAR_INFO_T*, NC_ATT_INFO_T* att);

extern int nc_field_new(char* name, NC_TYPE_INFO_T* cmpd, NC_FIELD_INFO_T** fieldp);
extern int* nc_field_get(NC_FIELD_INFO_T*, size_t* offset, int* fieldid);
extern int nc_field_set(NC_FIELD_INFO_T*, size_t offset, int fieldid);
    extern int nc_field_add_dim(NC_VAR_INFO_T*, int size);

extern int nc_econst_new(char* name, NC_TYPE_INFO_T* enumtyp, NC_ECONST_INFO_T** econstp);
extern int nc_econst_set(NC_ECONST_INFO_T*, void* value);
extern int* nc_econst_get(NC_ECONST_INFO_T*, void** value);

extern int nc_type_new(char* name, NC_COMMON_INFO_T* list, NC_TYPE_INFO_T** typep);
extern int nc_type_set(NC_TYPE_INFO_T*, nc_type typeid, nc_type klass, NC_TYPE_INFO_T* basetype);
extern int* nc_type_get(NC_TYPE_INFO_T*, nc_type* typeid, nc_type* klass, NC_TYPE_INFO_T* basetype);
    extern int nc_type_add_field(NC_TYPE_INFO_T*, NC_FIELD_INFO_T*);
    extern int nc_type_add_econst(NC_TYPE_INFO_T*, NC_ECONST_INFO_T*);

extern int nc_group_new(char* name, NC_GROUP_INFO_T* parent, NC_GROUP_INFO_T** groupp);
extern int nc_group_set(NC_GROUP_INFO_T*, int groupid, NC_FILE_INFO_T* file);
extern int* nc_group_get(NC_GROUP_INFO_T*, int* groupid, NC_FILE_INFO_T* file);
    extern int nc_type_add_var(NC_TYPE_INFO_T*, NC_VAR_INFO_T*);
    extern int nc_type_add_dim(NC_TYPE_INFO_T*, NC_DIM_INFO_T*);
    extern int nc_type_add_att(NC_TYPE_INFO_T*, NC_ATT_INFO_T*);

extern int nc_file_new(char* name, NC_META_COMMON_T* list, NC_FILE_INFO_T** filep);
extern int nc_file_set(NC_FILE_INFO_T*, NC_GROUP_INFO_T* root int fillmode);
extern int nc_file_set(NC_FILE_INFO_T*, NC_GROUP_INFO_T** root int* fillmode);
    extern int nc_type_add_group(NC_TYPE_INFO_T*, NC_GROUP_INFO_T*);

#endif /*_NCMETA_*/

