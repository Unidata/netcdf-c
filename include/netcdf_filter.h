/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/*
 * In order to use any of the netcdf_XXX.h files, it is necessary
 * to include netcdf.h followed by any netcdf_XXX.h files.
 * Various things (like EXTERNL) are defined in netcdf.h
 * to make them available for use by the netcdf_XXX.h files.
*/

#ifndef NETCDF_FILTER_H
#define NETCDF_FILTER_H 1

/* API for libdispatch/dfilter.c
   Note that since this filter_actions() is visible
   thru the dispatch table (via netcdf_dispatch.h)
   and that can be seen by clients using user-defined
   formats, then all argument types need
   to be user visible as well.
*/

/* Must match values in <H5Zpublic.h> */
#ifndef H5Z_FILTER_DEFLATE
#define H5Z_FILTER_DEFLATE 1
#endif
#ifndef H5Z_FILTER_SZIP
#define H5Z_FILTER_SZIP 4
#define H5_SZIP_ALLOW_K13_OPTION_MASK   1
#define H5_SZIP_CHIP_OPTION_MASK        2
#define H5_SZIP_EC_OPTION_MASK          4
#define H5_SZIP_NN_OPTION_MASK          32
#define H5_SZIP_MAX_PIXELS_PER_BLOCK    32

#define NC_SZIP_EC 4  /**< Selects entropy coding method for szip. */
#define NC_SZIP_NN 32 /**< Selects nearest neighbor coding method for szip. */
#endif

#define H5_SZIP_ALL_MASKS (H5_SZIP_CHIP_OPTION_MASK|H5_SZIP_EC_OPTION_MASK|H5_SZIP_NN_OPTION_MASK)

/** The maximum allowed setting for pixels_per_block when calling nc_def_var_szip(). */
#define NC_MAX_PIXELS_PER_BLOCK 32

#if defined(__cplusplus)
extern "C" {
#endif

/* Define the formats for NC_FILTER classes as aliases for NC_FORMATX_XXX*/
#define NC_FILTER_FORMAT_HDF5 (NC_FORMATX_NC_HDF5)
#define NCX_FILTER_FORMAT (NC_FORMATX_NCZARR)

/**************************************************/
/* HDF5 Specific filter functions (Deprecated) */

/*Define a filter for a variable */
EXTERNL int
nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms);

/* Learn about the first defined filter filter on a variable */
EXTERNL int
nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparams, unsigned int* params);

/* Support inquiry about all the filters associated with a variable */
/* As is usual, it is expected that this will be called twice: 
   once to get the number of filters, and then a second time to read the ids */
EXTERNL int nc_inq_var_filterids(int ncid, int varid, size_t* nfilters, unsigned int* filterids);

/* Learn about the filter with specified id wrt a variable */
EXTERNL int
nc_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparams, unsigned int* params);

/* Remove filter from variable*/
EXTERNL int nc_var_filter_remove(int ncid, int varid, unsigned int id);

/* Support direct user defined filters if enabled during configure;
   last arg is void*, but is actually H5Z_class2_t*.
   It is void* to avoid having to reference hdf.h.
*/
EXTERNL int nc_filter_client_register(unsigned int id, void*);
EXTERNL int nc_filter_client_unregister(unsigned int id);
EXTERNL int nc_filter_client_inq(unsigned int id, void*);

/* End HDF5 Specific Declarations */

/**************************************************/
/* X (String-based extension) Declarations */

/*Define a filter for a variable */
EXTERNL int
nc_def_var_filterx(int ncid, int varid, const char* id, size_t nparams, const char** params);

/* Support inquiry about all the filters associated with a variable */
/* As is usual, it is expected that this will be called twice: 
   once to get the number of filters, and then a second time to read the ids */
EXTERNL int
nc_inq_var_filterx_ids(int ncid, int varid, size_t* nfilters, char** filteridsp);

/* Learn about the filter with specified id wrt a variable */
EXTERNL int
nc_inq_var_filterx_info(int ncid, int varid, const char* id, size_t* nparamsp, char** paramsp);

/* Remove filter from variable*/
EXTERNL int
nc_var_filterx_remove(int ncid, int varid, const char* id);

/* Support direct user defined filters if enabled during configure;
   last arg is void*, but is actually H5Z_class2_t*.
   It is void* to avoid having to reference hdf.h.
*/
EXTERNL int nc_filterx_client_register(const char* id, void*);
EXTERNL int nc_filterx_client_unregister(const char* id);
EXTERNL int nc_filterx_client_inq(const char* id, void*);

/* End X (String-based extension) Declarations */

/**************************************************/

/* Set szip compression for a variable. */
EXTERNL int nc_def_var_szip(int ncid, int varid, int options_mask, int pixels_per_block);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_FILTER_H */
