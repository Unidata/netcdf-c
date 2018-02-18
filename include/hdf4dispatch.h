/* Copyright 2018, UCAR/Unidata. See netcdf/COPYRIGHT file for copying
 * and redistribution conditions. */
/**
 * @file
 * This header file contains the prototypes for the HDF4 versions
 * of the netCDF functions.
 *
 * Ed Hartnett
 */
#ifndef _HDF4DISPATCH_H
#define _HDF4DISPATCH_H

#include "config.h"
#include "ncdispatch.h"
#include <mfhdf.h>  /* The HDF4 header file. */

/* Stuff below is for hdf4 files. */
typedef struct NC_VAR_HDF4_INFO
{
   int sdsid;
   int hdf4_data_type;
} NC_VAR_HDF4_INFO_T;

typedef struct NC_HDF4_FILE_INFO
{
   int sdid;
} NC_HDF4_FILE_INFO_T;

#if defined(__cplusplus)
extern "C" {
#endif

   extern int
   HDF4_open(const char *path, int mode, int basepe, size_t *chunksizehintp, 
             int use_parallel, void* parameters, NC_Dispatch*, NC*);

   extern int
   HDF4_abort(int ncid);

   extern int
   HDF4_close(int ncid);

   extern int
   HDF4_inq_format(int ncid, int *formatp);

   extern int
   HDF4_inq_format_extended(int ncid, int *formatp, int *modep);

   extern int
   HDF4_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp);

   extern int
   HDF4_inq_type(int, nc_type, char *, size_t *);

/* Begin _dim */

   extern int
   HDF4_inq_dimid(int ncid, const char *name, int *idp);

   extern int
   HDF4_inq_dim(int ncid, int dimid, char *name, size_t *lenp);

   extern int
   HDF4_inq_unlimdim(int ncid, int *unlimdimidp);

/* End _dim */
/* Begin _att */

   extern int
   HDF4_inq_att(int ncid, int varid, const char *name,
                nc_type *xtypep, size_t *lenp);

   extern int 
   HDF4_inq_attid(int ncid, int varid, const char *name, int *idp);

   extern int
   HDF4_inq_attname(int ncid, int varid, int attnum, char *name);

/* End _att */
/* Begin {put,get}_att */

   extern int
   HDF4_get_att(int ncid, int varid, const char *name, void *value, nc_type);

/* End {put,get}_att */
/* Begin _var */

   extern int
   HDF4_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep, 
                    int *ndimsp, int *dimidsp, int *nattsp, 
                    int *shufflep, int *deflatep, int *deflate_levelp,
                    int *fletcher32p, int *contiguousp, size_t *chunksizesp, 
                    int *no_fill, void *fill_valuep, int *endiannessp, 
                    unsigned int* idp, size_t* nparamsp, unsigned int* params);

   extern int
   HDF4_inq_varid(int ncid, const char *name, int *varidp);

   extern int
   HDF4_get_vara(int ncid, int varid, const size_t *start, const size_t *count,
                 void *value, nc_type);

/* End _var */

/* netCDF4 API only */

   extern int
   HDF4_inq_ncid(int, const char *, int *);

   extern int
   HDF4_inq_grps(int, int *, int *);

   extern int
   HDF4_inq_grpname(int, char *);

   extern int
   HDF4_inq_grpname_full(int, size_t *, char *);

   extern int
   HDF4_inq_grp_parent(int, int *);

   extern int
   HDF4_inq_grp_full_ncid(int, const char *, int *);

   extern int
   HDF4_inq_varids(int, int * nvars, int *);

   extern int
   HDF4_inq_dimids(int, int * ndims, int *, int);

   extern int
   HDF4_inq_typeids(int, int * ntypes, int *);
   
   extern int
   HDF4_inq_type_equal(int, nc_type, int, nc_type, int *);

   extern int
   HDF4_inq_user_type(int, nc_type, char *, size_t *, nc_type *, 
                      size_t *, int *);

   extern int
   HDF4_inq_unlimdims(int, int *, int *);

   extern int
   HDF4_show_metadata(int);

   extern int 
   HDF4_initialize(void);

#if defined(__cplusplus)
}
#endif

#endif /*_HDF4DISPATCH_H */
