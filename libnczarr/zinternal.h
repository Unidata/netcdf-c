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

#define ZARRVERSION "2"

/* NCZARRVERSION is independent of Zarr version,
   but NCZARRVERSION => ZARRVERSION */
#define NCZARRVERSION "2.0.0"

/* These have to do with creating chunked datasets in ZARR. */
#define NCZ_CHUNKSIZE_FACTOR (10)
#define NCZ_MIN_CHUNK_SIZE (2)


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

/* V1 reserved objects */
#define NCZMETAROOT "/.nczarr"
#define NCZGROUP ".nczgroup"
#define NCZARRAY ".nczarray"
#define NCZATTRS ".nczattrs"
/* Deprecated */
#define NCZVARDEP ".nczvar"
#define NCZATTRDEP ".nczattr"

#define ZMETAROOT "/.zgroup"
#define ZGROUP ".zgroup"
#define ZATTRS ".zattrs"
#define ZARRAY ".zarray"

/* Pure Zarr pseudo names */
#define ZDIMANON "_zdim"

/* V2 Reserved Attributes */
/*
Inserted into /.zgroup
_nczarr_superblock: {"version": "2.0.0"}
Inserted into any .zgroup
"_nczarr_group": "{
\"dimensions\": {\"d1\": \"1\", \"d2\": \"1\",...}
\"variables\": [\"v1\", \"v2\", ...]
\"groups\": [\"g1\", \"g2\", ...]
}"
Inserted into any .zarray
"_nczarr_array": "{
\"dimensions\": [\"/g1/g2/d1\", \"/d2\",...]
\"storage\": \"scalar\"|\"contiguous\"|\"compact\"|\"chunked\"
}"
Inserted into any .zattrs ? or should it go into the container?
"_nczarr_attrs": "{
\"types\": {\"attr1\": \"<i4\", \"attr2\": \"<i1\",...}
}
+
+Note: _nczarr_attrs type include non-standard use of a zarr type "|U1" => NC_CHAR.
+
*/

#define NCZ_V2_SUPERBLOCK "_nczarr_superblock"
#define NCZ_V2_GROUP   "_nczarr_group"
#define NCZ_V2_ARRAY   "_nczarr_array"
#define NCZ_V2_ATTR    NC_NCZARR_ATTR

#define NCZ_V2_SUPERBLOCK_UC "_NCZARR_SUPERBLOCK"
#define NCZ_V2_GROUP_UC   "_NCZARR_GROUP"
#define NCZ_V2_ARRAY_UC   "_NCZARR_ARRAY"
#define NCZ_V2_ATTR_UC    NC_NCZARR_ATTR_UC

#define NCZARRCONTROL "nczarr"
#define PUREZARRCONTROL "zarr"
#define XARRAYCONTROL "xarray"
#define NOXARRAYCONTROL "noxarray"
#define XARRAYSCALAR "_scalar_"

#define LEGAL_DIM_SEPARATORS "./"
#define DFALT_DIM_SEPARATOR '.'

/* Default max string length for fixed length strings */
#define NCZ_MAXSTR_DEFAULT 128

#define islegaldimsep(c) ((c) != '\0' && strchr(LEGAL_DIM_SEPARATORS,(c)) != NULL)

/* Mnemonics */
#define ZCLEAR	0 /* For NCZ_copy_data */
#define ZCLOSE	1 /* this is closeorabort as opposed to enddef */

/* Useful macro */
#define ncidforx(file,grpid) ((file)->controller->ext_ncid | (grpid))
#define ncidfor(var) ncidforx((var)->container->nc4_info,(var)->container->hdr.id)

/**************************************************/
/* Forward */

struct NClist;
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
    char** envv_controls; /* Envv format */
    struct Controls {
        size64_t flags;
#		define FLAG_PUREZARR    1
#		define FLAG_SHOWFETCH   2
#		define FLAG_LOGGING     4
#		define FLAG_XARRAYDIMS  8
#		define FLAG_NCZARR_V1   16
	NCZM_IMPL mapimpl;
    } controls;
    int default_maxstrlen; /* default max str size for variables of type string */
} NCZ_FILE_INFO_T;

/* This is a struct to handle the dim metadata. */
typedef struct NCZ_DIM_INFO {
    NCZcommon common;
} NCZ_DIM_INFO_T;

/** Struct to hold ZARR-specific info for attributes. */
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
    size64_t chunksize; /* chunkproduct * typesize */
    int order; /* 1=>column major, 0=>row major (default); not currently enforced */
    size_t scalar;
    struct NCZChunkCache* cache;
    struct NClist* xarray; /* names from _ARRAY_DIMENSIONS */
    char dimension_separator; /* '.' | '/' */
    NClist* incompletefilters;
    int maxstrlen; /* max length of strings for this variable */
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
int NCZ_ensure_fill_value(NC_VAR_INFO_T* var);
int ncz_find_grp_var_att(int ncid, int varid, const char *name, int attnum,
                              int use_name, char *norm_name, NC_FILE_INFO_T** file,
                              NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var,
                              NC_ATT_INFO_T** att);
int NCZ_set_log_level(void);

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
int NCZ_ensure_quantizer(int ncid, NC_VAR_INFO_T* var);

/* Undefined */
/* Find var, doing lazy var metadata read if needed. */
int ncz_find_grp_file_var(int ncid, int varid, NC_FILE_INFO_T** file,
                             NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var);

#endif /* ZINTERNAL_H */

