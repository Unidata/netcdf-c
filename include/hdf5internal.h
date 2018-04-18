/* Copyright 2005-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file This header file contains the definitions of structs used to
 * hold netCDF file metadata in memory.
 *
 * @author Ed Hartnett
*/

#ifndef _HDF5INTERNAL_
#define _HDF5INTERNAL_

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <hdf5.h>
#include "ncdimscale.h"
#include "nc_logging.h"
#include "nc4internal.h"

#ifdef USE_PARALLEL
#include "netcdf_par.h"
#endif /* USE_PARALLEL */
#include "netcdf.h"
#include "netcdf_f.h"

/* Always needed */
#include "nc.h"

#ifdef LOGGING

/* To log something... */
void nc_log_hdf5(void);

/* To log based on error code, and set retval. */
#define BAIL2(e) \
   do { \
      retval = e; \
      LOG((0, "file %s, line %d.\n%s", __FILE__, __LINE__, nc_strerror(e))); \
      nc_log_hdf5(); \
   } while (0) 

#else /* LOGGING */

#define BAIL2(e) \
   do { \
      retval = e; \
   } while (0)
#endif /* LOGGING */

/* This is a struct to handle the dim metadata. */
typedef struct NC_HDF5_DIM_INFO
{
   hid_t hdf_dimscaleid;        /* Non-zero if a DIM_WITHOUT_VARIABLE dataset is in use (no coord var). */
   HDF5_OBJID_T hdf5_objid;
} NC_HDF5_DIM_INFO_T;

/* This is the metadata we need to keep track of for each
   netcdf-4/HDF5 file. */
typedef struct  NC_HDF5_FILE_INFO_2
{
   hid_t hdfid;
} NC_HDF5_FILE_INFO_2_T;

/* This is a struct to handle the var metadata. */
typedef struct NC_HDF5_VAR_INFO
{
   /* char *hdf5_name; /\* used if different from name *\/ */
   hid_t hdf_datasetid;
   /* HDF5_OBJID_T *dimscale_hdf5_objids; */
} NC_HDF5_VAR_INFO_T;

/* This is a struct to handle the var metadata. */
typedef struct NC_HDF5_ATT_INFO
{
   hid_t native_hdf_typeid;     /* Native HDF5 datatype for attribute's data */
} NC_HDF5_ATT_INFO_T;

/* This holds information for one group. */
typedef struct NC_HDF5_GRP_INFO
{
   hid_t hdf_grpid;
} NC_HDF5_GRP_INFO_T;

typedef struct NC_HDF5_TYPE_INFO
{
   hid_t hdf_typeid;            /* HDF5 type ID, in the file */
   hid_t native_hdf_typeid;     /* HDF5 type ID, in memory */

   /* Information for each type or class */
   union {
      struct {
         hid_t base_hdf_typeid;
      } e;                      /* Enum */
      struct {
         hid_t base_hdf_typeid;
      } v;                      /* Variable-length */
   } u;                         /* Union of structs, for each type/class */
} NC_HDF5_TYPE_INFO_T;

#if defined(__cplusplus)
extern "C" {
#endif
   
   /* Set the log level. 0 shows only errors, 1 only major messages,
    * etc., to 5, which shows way too much information. */
   int hdf5_set_log_level(int new_level);

   int hdf5_rec_grp_del(NC_GRP_INFO_T *grp);

   int hdf5_att_list_del(NC_ATT_INFO_T **list, NC_ATT_INFO_T *att);

   int hdf5_var_del(NC_VAR_INFO_T *var);

   int hdf5_type_free(NC_TYPE_INFO_T *type);

   void hdf5_initialize(void);

   NC_TYPE_INFO_T *hdf5_rec_find_type(NC_GRP_INFO_T *start_grp, hid_t target_hdf_typeid);

   int rec_detach_scales(NC_GRP_INFO_T *grp, int dimid, hid_t dimscaleid);
   int rec_reattach_scales(NC_GRP_INFO_T *grp, int dimid, hid_t dimscaleid);
   int delete_existing_dimscale_dataset(NC_GRP_INFO_T *grp, int dimid, NC_DIM_INFO_T *dim);
   int nc4_open_var_grp2(NC_GRP_INFO_T *grp, int varid, hid_t *dataset);
   int nc4_put_vara(NC *nc, int ncid, int varid, const size_t *startp,
                    const size_t *countp, nc_type xtype, int is_long, void *op);
   int nc4_get_vara(NC *nc, int ncid, int varid, const size_t *startp,
                    const size_t *countp, nc_type xtype, int is_long, void *op);
   int nc4_rec_match_dimscales(NC_GRP_INFO_T *grp);
   int nc4_rec_detect_need_to_preserve_dimids(NC_GRP_INFO_T *grp, nc_bool_t *bad_coord_orderp);
   int nc4_rec_write_metadata(NC_GRP_INFO_T *grp, nc_bool_t bad_coord_order);
   int nc4_rec_write_groups_types(NC_GRP_INFO_T *grp);
   int nc4_enddef_netcdf4_file(NC_HDF5_FILE_INFO_T *h5);
   int nc4_reopen_dataset(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);
   int nc4_adjust_var_cache(NC_GRP_INFO_T *grp, NC_VAR_INFO_T * var);
   NC_TYPE_INFO_T *nc4_rec_find_hdf_type(NC_HDF5_FILE_INFO_T* h5, hid_t target_hdf_typeid);
   int nc4_get_hdf_typeid(NC_HDF5_FILE_INFO_T *h5, nc_type xtype,
                          hid_t *hdf_typeid, int endianness);

#if defined(__cplusplus)
}
#endif

#endif /* _HDF5INTERNAL_ */
