/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/**
 * @file This header file contains macros, types, and prototypes for
 * the ZARR code in libzarr. This header should not be included in
 * code outside libzarr.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#ifndef ZINTERNAL_H
#define ZINTERNAL_H

#define ZARRVERSION 2

/* NCZARRVERSION is ndependent of Zarr version,
   but NCZARRVERSION => ZARRVERSION */
#define NCZARRVERSION "1.0.0"

/* These have to do with creating chuncked datasets in ZARR. */
#define NCZ_CHUNKSIZE_FACTOR (10)
#define NCZ_MIN_CHUNK_SIZE (2)

/* An attribute in the ZARR root group of this name means that the
 * file must follow strict netCDF classic format rules. */
#define NCZ_NC3_STRICT_ATT_NAME "_nc3_strict"

/**************************************************/
/* Constants */

#define RCFILEENV "DAPRCFILE"

/* Figure out a usable max path name max */
#ifdef PATH_MAX /* *nix* */
#define NC_MAX_PATH PATH_MAX
#else
#  ifdef MAX_PATH /*windows*/
#    define NC_MAX_PATH MAX_PATH
#  else
#    define NC_MAX_PATH 4096
#  endif
#endif

#define NCZMETAROOT "/.nczarr"
#define NCZGROUP ".nczgroup"
#define NCZVAR ".nczvar"
#define NCZATTR ".nczattr"

#define ZGROUP ".zgroup"
#define ZATTRS ".zattrs"
#define ZARRAY ".zarray"

#define PUREZARR "zarr"

/**************************************************/
/* Forward */

struct NCjson;
struct NCauth;
struct NCZMAP;
struct NCZChunkCache;

/**************************************************/
/* Define annotation data for NCZ objects */

/* Common fields for all annotations */
typedef struct NCZcommon {
    NC_FILE_INFO_T* file; /* root of the dataset tree */
} NCZcommon;

/** Struct to hold ZARR-specific info for the file. */
typedef struct NCZ_FILE_INFO {
    NCZcommon common;
    struct NCZMAP* map; /* implementation */
    struct NCauth* auth;
    struct nczarr {
	int zarr_version;
	struct {
	    unsigned long major;
	    unsigned long minor;
	    unsigned long release;
	} nczarr_version;
    } zarr;
    int created; /* 1=> created 0=>open */
    int native_endianness; /* NC_ENDIAN_LITTLE | NC_ENDIAN_BIG */
    char** controls; /* Envv format */
    struct Features {
        size64_t flags;
#		define FLAG_PUREZARR  1
#		define FLAG_SHOWFETCH 2
#		define FLAG_LOGGING   4
#		define FLAG_BYTERANGE 8
	NCZM_IMPL mapimpl;
    } features;
} NCZ_FILE_INFO_T;

/* This is a struct to handle the dim metadata. */
typedef struct NCZ_DIM_INFO {
    NCZcommon common;
} NCZ_DIM_INFO_T;

/** Strut to hold ZARR-specific info for attributes. */
typedef struct  NCZ_ATT_INFO {
    NCZcommon common;
} NCZ_ATT_INFO_T;

/* Struct to hold ZARR-specific info for a group. */
typedef struct NCZ_GRP_INFO {
    NCZcommon common;
#if 0
    /* The jcontent field stores the following:
	1. List of (name,length) for dims in the group
	2. List of (name,type) for user-defined types in the group
	3. List of var names in the group
	4. List of subgroups names in the group
    */
    NClist* dims;
    NClist* types; /* currently not used */
    NClist* vars;
    NClist* grps;
#endif
} NCZ_GRP_INFO_T;

/* Struct to hold ZARR-specific info for a variable. */
typedef struct NCZ_VAR_INFO {
    NCZcommon common;
    size64_t chunkproduct; /* product of chunksizes */
    int order; /* 1=>column major, 0=>row major (default); not currently enforced */
    struct NCZChunkCache* cache;
    /* Following are duplicates of NC_VAR_INFO_T equivalents */
    size_t chunk_cache_nelems;   /**< Number of slots in var chunk cache. */
} NCZ_VAR_INFO_T;

/* Struct to hold ZARR-specific info for a field. */
typedef struct NCZ_FIELD_INFO {
    NCZcommon common;
} NCZ_FIELD_INFO_T;

/* Struct to hold ZARR-specific info for a type. */
typedef struct NCZ_TYPE_INFO {
    NCZcommon common;
} NCZ_TYPE_INFO_T;

#if 0
/* Define the contents of the .nczcontent object */
/* The .nczcontent field stores the following:
   1. List of (name,length) for dims in the group
   2. List of (name,type) for user-defined types in the group
   3. List of var names in the group
   4. List of subgroups names in the group
*/
typedef struct NCZCONTENT{
    NClist* dims;
    NClist* types; /* currently not used */
    NClist* vars;
    NClist* grps;
} NCZCONTENT;
#endif

/**************************************************/

extern int ncz_initialized; /**< True if initialization has happened. */

/* Forward */
struct NCZ_Filterspec;

/* zinternal.c */
int NCZ_initialize(void);
int NCZ_finalize(void);
int NCZ_initialize_internal(void);
int NCZ_finalize_internal(void);
int ncz_get_fill_value(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, void **fillp);
int ncz_find_grp_var_att(int ncid, int varid, const char *name, int attnum,
                              int use_name, char *norm_name, NC_FILE_INFO_T** file,
                              NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var,
                              NC_ATT_INFO_T** att);
int NCZ_set_log_level();

/* zcache.c */
int ncz_adjust_var_cache(NC_GRP_INFO_T* grp, NC_VAR_INFO_T* var);
int NCZ_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption);

/* zfile.c */
int ncz_enddef_netcdf4_file(NC_FILE_INFO_T*);
int ncz_closeorabort(NC_FILE_INFO_T*, void* params, int abort);

/* zclose.c */
int ncz_close_ncz_file(NC_FILE_INFO_T* file, int abort);

/* zattr.c */
int ncz_getattlist(NC_GRP_INFO_T *grp, int varid, NC_VAR_INFO_T **varp, NCindex **attlist);
int ncz_create_fillvalue(NC_VAR_INFO_T* var);
int ncz_makeattr(NC_OBJ*, NCindex* attlist, const char* name, nc_type typid, size_t len, void* values, NC_ATT_INFO_T**);

/* zvar.c */
int ncz_gettype(NC_FILE_INFO_T*, NC_GRP_INFO_T*, int xtype, NC_TYPE_INFO_T** typep);
int ncz_find_default_chunksizes2(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);

/* zfilter.c */
/* Dispatch functions are also in zfilter.c */
/* Filterlist management */

/* The NC_VAR_INFO_T->filters field is an NClist of this struct */
struct NCZ_Filter {
    int flags;             /**< Flags describing state of this filter. */
    unsigned int filterid; /**< ID for arbitrary filter. */
    size_t nparams;        /**< nparams for arbitrary filter. */
    unsigned int* params;  /**< Params for arbitrary filter. */
};

int NCZ_filter_lookup(NC_VAR_INFO_T* var, unsigned int id, struct NCZ_Filter** specp);
int NCZ_addfilter(NC_VAR_INFO_T* var, unsigned int id, size_t nparams, const unsigned int* params);
int NCZ_filter_freelist(NC_VAR_INFO_T* var);

/* Undefined */
/* Find var, doing lazy var metadata read if needed. */
int ncz_find_grp_file_var(int ncid, int varid, NC_FILE_INFO_T** file,
                             NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var);

#endif /* ZINTERNAL_H */

