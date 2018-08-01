/* Copyright 2005-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file This header file contains macros, types, and prototypes for
 * the HDF5 code in libhdf5. This header should not be included in
 * code outside libhdf5.
 *
 * @author Ed Hartnett
*/

#ifndef _HDF5INTERNAL_
#define _HDF5INTERNAL_

#include "config.h"
#include <hdf5.h>
#include <hdf5_hl.h>
#include "nc4internal.h"
#include "ncdimscale.h"
#include "nc4dispatch.h"

#define NC_MAX_HDF5_NAME (NC_MAX_NAME + 10)

/* These have to do with creating chuncked datasets in HDF5. */
#define NC_HDF5_UNLIMITED_DIMSIZE (0)
#define NC_HDF5_CHUNKSIZE_FACTOR (10)
#define NC_HDF5_MIN_CHUNK_SIZE (2)

#define NC_EMPTY_SCALE "NC_EMPTY_SCALE"

/* This is an attribute I had to add to handle multidimensional
 * coordinate variables. */
#define COORDINATES "_Netcdf4Coordinates"
#define COORDINATES_LEN (NC_MAX_NAME * 5)

/* This is used when the user defines a non-coordinate variable with
 * same name as a dimension. */
#define NON_COORD_PREPEND "_nc4_non_coord_"

/* An attribute in the HDF5 root group of this name means that the
 * file must follow strict netCDF classic format rules. */
#define NC3_STRICT_ATT_NAME "_nc3_strict"

/* If this attribute is present on a dimscale variable, use the value
 * as the netCDF dimid. */
#define NC_DIMID_ATT_NAME "_Netcdf4Dimid"

/** This is the name of the class HDF5 dimension scale attribute. */
#define HDF5_DIMSCALE_CLASS_ATT_NAME "CLASS"

/** This is the name of the name HDF5 dimension scale attribute. */
#define HDF5_DIMSCALE_NAME_ATT_NAME "NAME"

/** Strut to hold HDF5-specific info for the file. */
typedef struct  NC_HDF5_FILE_INFO
{
   hid_t hdfid;
} NC_HDF5_FILE_INFO_T;

/* These functions do HDF5 things. */
int rec_detach_scales(NC_GRP_INFO_T *grp, int dimid, hid_t dimscaleid);
int rec_reattach_scales(NC_GRP_INFO_T *grp, int dimid, hid_t dimscaleid);

#endif /* _HDF5INTERNAL_ */
