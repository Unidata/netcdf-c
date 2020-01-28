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

/** Define Filter API Operations */
#define NCZ_FILTER_REG   1
#define NCZ_FILTER_UNREG 2
#define NCZ_FILTER_INQ   3

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

#define NCZMETAROOT "/_nczarr"
#define NCZGROUP "_nczgroup"
#define NCZVAR "_nczvar"
#define NCZATTR "_nczattr"

#define ZGROUP ".zgroup"
#define ZATTRS ".zattrs"
#define ZARRAY ".zarray"

/**************************************************/
/* Forward */

struct NCjson;
struct NCauth;
struct NCZMAP;

/**************************************************/
/* Define annotation data for NCZ objects */

/* Common fields for all annotations */
typedef struct NCZcommon {
    int exists; /* Is this object already materialized? */
} NCZcommon;

/** Struct to hold ZARR-specific info for the file. */
typedef struct NCZ_FILE_INFO {
    NCZcommon common;
    NC_FILE_INFO_T* dataset; /* root of the dataset tree */
    struct NCZMAP* map; /* implementation */
    NClist* controls;
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
    int order; /* 1=>column major, 0=>row major (default); not currently enforced */
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
/* From libsrc4, these are the netcdf-4 cache sizes. */
extern size_t ncz_chunk_cache_size;
extern size_t ncz_chunk_cache_nelems;
extern float ncz_chunk_cache_preemption;

extern int ncz_initialized; /**< True if initialization has happened. */

/* Forward */
struct NC_FILTER_INFO;

/* zinternal.c */
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
int ncz_makeattr(NC_OBJ*, NCindex* attlist, const char* name, nc_type typeid, size_t len, void* values, NC_ATT_INFO_T**);

/* zvar.c */
int ncz_gettype(int xtype, NC_TYPE_INFO_T** typep);

/* Undefined */
/* Find var, doing lazy var metadata read if needed. */
int ncz_find_grp_file_var(int ncid, int varid, NC_FILE_INFO_T** file,
                             NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var);

#endif /* ZINTERNAL_H */

