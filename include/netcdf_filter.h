/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */
/*! @file */

/*
 * In order to use any of the netcdf_XXX.h files, it is necessary
 * to include netcdf.h followed by any netcdf_XXX.h files.
 * Various things (like EXTERNL) are defined in netcdf.h
 * to make them available for use by the netcdf_XXX.h files.
*/

#ifndef NETCDF_FILTER_H
#define NETCDF_FILTER_H 1

/**************************************************/
/* API for libdispatch/dfilter.c */

/* Must match values in <H5Zpublic.h> */
#ifndef H5Z_FILTER_DEFLATE
#define H5Z_FILTER_DEFLATE 1
#define H5Z_FILTER_SHUFFLE 2
#define H5Z_FILTER_FLETCHER32 3
#endif

#ifndef H5Z_FILTER_SZIP
#define H5Z_FILTER_SZIP 4
#define H5_SZIP_ALLOW_K13_OPTION_MASK   1
#define H5_SZIP_CHIP_OPTION_MASK        2
#define H5_SZIP_EC_OPTION_MASK          4
#define H5_SZIP_LSB_OPTION_MASK		8
#define H5_SZIP_MSB_OPTION_MASK		16
#define H5_SZIP_NN_OPTION_MASK          32
#define H5_SZIP_MAX_PIXELS_PER_BLOCK    32
#define H5_SZIP_RAW_OPTION_MASK		128

#define NC_SZIP_EC 4  /**< Selects entropy coding method for szip. */
#define NC_SZIP_NN 32 /**< Selects nearest neighbor coding method for szip. */
#endif

#define H5_SZIP_ALL_MASKS (H5_SZIP_CHIP_OPTION_MASK|H5_SZIP_EC_OPTION_MASK|H5_SZIP_NN_OPTION_MASK)

/** The maximum allowed setting for pixels_per_block when calling nc_def_var_szip(). */
#define NC_MAX_PIXELS_PER_BLOCK 32

/* Other Standard Filters */
#ifndef H5Z_FILTER_ZSTD
#define H5Z_FILTER_ZSTD 32015
#endif
#ifndef H5Z_FILTER_BZIP2
#define H5Z_FILTER_BZIP2 307
#endif
#ifndef H5Z_FILTER_BLOSC
#define H5Z_FILTER_BLOSC 32001
#endif
#ifndef BLOSC_SHUFFLE
enum BLOSC_SHUFFLE {
BLOSC_NOSHUFFLE=0,  /* no shuffle */
BLOSC_SHUFFLE=1,  /* byte-wise shuffle */
BLOSC_BITSHUFFLE=2  /* bit-wise shuffle */
};
#endif

/*Assumes use of c-blosc library */
#ifndef BLOSC_BLOSCLZ
enum BLOSC_SUBCOMPRESSORS {BLOSC_LZ=0, BLOSC_LZ4=1, BLOSC_LZ4HC=2, BLOSC_SNAPPY=3, BLOSC_ZLIB=4, BLOSC_ZSTD=5};
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**************************************************/
/* HDF5 Format filter functions */

/*Define a filter for a variable */
EXTERNL int
nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms);

/* Learn about the first defined filter filter on a variable */
EXTERNL int
nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparams, unsigned int* params);

/* Support inquiry about all the filters associated with a variable */
/* As is usual, it is expected that this will be called twice: 
   once to get the number of filters, and then a second time to read the ids */
EXTERNL int nc_inq_var_filter_ids(int ncid, int varid, size_t* nfilters, unsigned int* filterids);

/* Learn about the filter with specified id wrt a variable */
EXTERNL int nc_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparams, unsigned int* params);

/* End HDF5 Format Declarations */

/**************************************************/
/* Allow user to interrogate if a filter is available*/
/* Would prefer to get list of all known filters, but HDF5 does not support that */

/* See if filter is available */
EXTERNL int nc_inq_filter_avail(int ncid, unsigned id);

/**************************************************/
/* Functions for accessing standardized filters */

EXTERNL int nc_def_var_bzip2(int ncid, int varid, int level);
EXTERNL int nc_inq_var_bzip2(int ncid, int varid, int* hasfilterp, int *levelp);

/* Note use of 'zstandard' instead of 'zstd' */
EXTERNL int nc_def_var_zstandard(int ncid, int varid, int level);
EXTERNL int nc_inq_var_zstandard(int ncid, int varid, int* hasfilterp, int *levelp);

EXTERNL int nc_def_var_blosc(int ncid, int varid, unsigned subcompressor, unsigned level, unsigned blocksize, unsigned addshuffle);
EXTERNL int nc_inq_var_blosc(int ncid, int varid, int* hasfilterp, unsigned* subcompressorp, unsigned* levelp, unsigned* blocksizep, unsigned* addshufflep);

/* Filter path query/set */
EXTERNL int nc_filter_path_query(int id);

/**************************************************/
/* API for libdispatch/dplugin.c */

/* Plugin path functions */

/**
 * This function is called as part of nc_initialize.
 * Its purpose is to initialize the plugin paths state.
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/

EXTERNL int nc_plugin_path_initialize(void);

/**
 * This function is called as part of nc_finalize()
 * Its purpose is to clean-up plugin path state.
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/

EXTERNL int nc_plugin_path_finalize(void);

/**
 * Return the current sequence of directories in the internal plugin path list.
 * Since this function does not modify the plugin path, it can be called at any time.
 * @param formatx specify which dispatch implementatio to read: currently NC_FORMATX_NC_HDF5 or NC_FORMATX_NCZARR.
 * @param ndirsp return the number of dirs in the internal path list
 * @param dirs memory for storing the sequence of directies in the internal path list.
 * @return NC_NOERR
 * @author Dennis Heimbigner
 *
 * As a rule, this function needs to be called twice.
 * The first time with npaths not NULL and pathlist set to NULL
 *     to get the size of the path list.
 * The second time with pathlist not NULL to get the actual sequence of paths.
*/

EXTERNL int nc_plugin_path_read(int formatx, size_t* ndirsp, char** dirs);

/**
 * Empty the current internal path sequence
 * and replace with the sequence of directories argument.
 *
 * Using a paths argument of NULL or npaths argument of 0 will clear the set of plugin paths.
 * @param formatx specify which dispatch implementation to write: currently NC_FORMATX_NC_HDF5 or NC_FORMATX_NCZARR.
 * @param ndirs length of the dirs argument
 * @param dirs to overwrite the current internal path list
 * @return NC_NOERR
 * @author Dennis Heimbigner
*/

EXTERNL int nc_plugin_path_write(int formatx, size_t ndirs, char** const dirs);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_FILTER_H */
