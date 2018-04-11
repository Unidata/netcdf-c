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

#if defined(__cplusplus)
extern "C" {
#endif
   
   /* Set the log level. 0 shows only errors, 1 only major messages,
    * etc., to 5, which shows way too much information. */
   int hdf5_set_log_level(int new_level);

   /* int hdf5_rec_grp_del(NC_GRP_INFO_T **list, NC_GRP_INFO_T *grp); */
   int hdf5_rec_grp_del(NC_GRP_INFO_T *grp);

   int hdf5_att_list_del(NC_ATT_INFO_T **list, NC_ATT_INFO_T *att);

   int hdf5_var_del(NC_VAR_INFO_T *var);

   int hdf5_type_free(NC_TYPE_INFO_T *type);

   void hdf5_initialize(void);

   NC_TYPE_INFO_T *hdf5_rec_find_type(NC_GRP_INFO_T *start_grp, hid_t target_hdf_typeid);
   
#if defined(__cplusplus)
}
#endif

#endif /* _HDF5INTERNAL_ */
