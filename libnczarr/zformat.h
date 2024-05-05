/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 *
 * @author Dennis Heimbigner
 */

#ifndef ZFORMAT_H
#define ZFORMAT_H

/*
Notes on internal architecture.

Zarr version 2 vs Zarr version 3 is handled by using
a dispatch table mechanism similar to the dispatch
mechanism used in netcdf_dispatch.h to choose the
netcdf file format.

The dispatcher is defined by the type NCZ_Formatter.
That dispatcher allows the Zarr format independent code
to be isolated from the Zarr format specific code.
The table has the following groups of entries:
1. open/create/close
2. misc. actions -- e.g. building chunk keys and converting between the Zarr codec and an HDF5 filter.
3. reading metadata -- use the JSON metadata of a file to a fill in the tree of an instance of NC_FILE_INFO_T.
4. writing metadata -- use an NC_FILE_INFO_T tree to build and write the JSON metadata of a file.
*/


/* This is the version of the formatter table. It should be changed
 * when new functions are added to the formatter table. */
#ifndef NCZ_FORMATTER_VERSION
#define NCZ_FORMATTER_VERSION 1
#endif /*NCZ_FORMATTER_VERSION*/

/* struct Fill Values */
#define NCZ_CODEC_ENV_EMPTY_V2 {NCZ_CODEC_ENV_VER, 2}
#define NCZ_CODEC_ENV_EMPTY_V3 {NCZ_CODEC_ENV_VER, 3}

/* Opaque */
struct NCZ_Plugin;

/* This is the dispatch table, with a pointer to each netCDF
 * function. */
typedef struct NCZ_Formatter {
    int nczarr_format;
    int zarr_format;
    int dispatch_version; /* Version of the dispatch table */
    int (*create)    (NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
    int (*open)      (NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
    int (*close)     (NC_FILE_INFO_T* file);

    /* Read meta-data elements */
    int (*readmeta)  (NC_FILE_INFO_T* file);

    /* write meta-data elements */
    int (*writemeta)  (NC_FILE_INFO_T* file);

    /* Codec converters */
    int (*hdf2codec) (const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter);

    /* Misc. */
    int (*buildchunkkey)(size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);
} NCZ_Formatter;

#if defined(__cplusplus)
extern "C" {
#endif

/* Called by nc_initialize and nc_finalize respectively */
extern int NCZF_initialize(void);
extern int NCZF_finalize(void);

/* Wrappers for the formatter functions */

extern int NCZF_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
extern int NCZF_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
extern int NCZF_close(NC_FILE_INFO_T* file);
    
extern int NCZF_readmeta(NC_FILE_INFO_T* file);
extern int NCZF_writemeta(NC_FILE_INFO_T* file);

extern int NCZF_nctype2dtype(NC_FILE_INFO_T* file, nc_type nctype, int endianness, int purezarr, size_t len, char** dnamep, const char** tagp);
extern int NCZF_dtype2nctype(NC_FILE_INFO_T* file, const char* dtype , const char* dalias, nc_type* nctypep, size_t* typelenp, int* endianp);

extern int NCZF_hdf2codec(const NC_FILE_INFO_T* file, const NC_VAR_INFO_T* var, NCZ_Filter* filter);

extern int NCZF_buildchunkkey(const NC_FILE_INFO_T* file, size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);

/* Define known dispatch tables and initializers */
/* Each handles a specific NCZarr format + Pure Zarr */
/* WARNING: there is a lot of similar code in the dispatchers,
   so fixes to one may need to be propagated to the other dispatchers.
*/

extern const NCZ_Formatter* NCZ_formatter2; /* NCZarr V2 dispatch table => Zarr V2 */
extern const NCZ_Formatter* NCZ_formatter3; /* NCZarr V3 dispatch table => Zarr V3*/
/**************************************************/

/* Use inference to get map and the formatter */
extern int NCZ_get_map(NC_FILE_INFO_T* file, NCURI* url, mode_t mode, size64_t constraints, void* params, NCZMAP** mapp);
extern int NCZ_get_formatter(NC_FILE_INFO_T* file, const NCZ_Formatter** formatterp);

#if defined(__cplusplus)
}
#endif

#endif /* ZFORMAT_H */
