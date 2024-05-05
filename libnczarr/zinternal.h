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

/* This is the version of this NCZarr package */
/* This completely independent of the Zarr specification version */
#define NCZARR_PACKAGE_VERSION "3.0.0"

/* Allowed Zarr Formats */
#define ZARRFORMAT2 2
#define ZARRFORMAT3 3

/* Mode encoded formats */
#define ZARRFORMAT2_STRING "v2"
#define ZARRFORMAT3_STRING "v3"

/* Define the possible NCZarr format versions */
/* These are independent of the Zarr specification version */
#define NCZARRFORMAT0 0 /* if this is a pure zarr dataset */
#define NCZARRFORMAT2 2
#define NCZARRFORMAT3 3

/* Map the NCZarr Format version to a string */
#define NCZARR_FORMAT_VERSION_TEMPLATE "%d.0.0"

/* The name of the env var for changing default zarr format */
#define NCZARRDEFAULTFORMAT "NCZARRFORMAT"

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

/* V2 Reserved Objects */
#define Z2METAROOT "/.zgroup"
#define Z2ATTSROOT "/.zattrs"
#define Z2GROUP ".zgroup"
#define Z2ATTRS ".zattrs"
#define Z2ARRAY ".zarray"

/* V3 Reserved Objects */
#define Z3METAROOT "/zarr.json"
#define Z3OBJECT "zarr.json"
#define Z3GROUP Z3OBJECT
#define Z3ARRAY Z3OBJECT

/* Bytes codec name */
#define ZBYTES3 "bytes"

/* V2 Reserved Attributes */
/*
For nczarr version 2.x.x, the following (key,value)
pairs are stored in .zgroup and/or .zarray.

For nczarr version 3.0.0, the following (key,value)
pairs are stored in .zattrs as if they were standard attributes.
The cost is that lazy attribute reading is no longer possible.

Inserted into /.zgroup || /.zattrs
_nczarr_superblock: {"version": "3.0.0", "format=2"}

Inserted into any .zgroup || .zattrs (at group level)
"_nczarr_group": "{
\"dims\": {\"d1\": \"1\", \"d2\": \"1\",...}
\"vars\": [\"v1\", \"v2\", ...]
\"groups\": [\"g1\", \"g2\", ...]
}"

Inserted into any .zarray || .zattrs (at array level)
"_nczarr_array": "{
\"dimensions\": [\"/g1/g2/d1\", \"/d2\",...]
\"storage\": \"contiguous\" | \"chunked\"
}"
Note that contiguous <=> scalar for Zarr V2.

Inserted into any .zattrs
"_nczarr_attrs": "{
\"types\": {\"attr1\": \"<i4\", \"attr2\": \"json\", \"attr2\": \"char\",...}
}
Note: _nczarr_attrs type include non-standard use of a zarr type ">S1" => NC_CHAR
and "|J0" for json valued attributes.
*/

/* V3 Reserved Attributes */
/*
Inserted into root group zarr.json as an extra attribute.
_nczarr_superblock: {
    "version": 3.0.0,    
    "format": 3
}

Optionally inserted into any group zarr.json as an attribute:
"_nczarr_group": "{
\"dimensions\": [{name: <dimname>, size: <integer>, unlimited: 1|0},...],
\"arrays\": ["<name>",...],
\"subgroups\": ["<name>",...]
}"

Optionally inserted into any array zarr.json as an attribute:
````
"_nczarr_array": "{
\"dimension_references\": [\"/g1/g2/d1\", \"/d2\",...],
\"type_alias\": "<string indicating special type aliasing>" // optional
\"maxstrlen\": "<integer>" // optional
}"
````
The *dimension_references* key is an expansion of the "dimensions" key
found in the *zarr.json* object for an array.
The problem with "dimensions" is that it specifies a simple name for each
dimension, whereas netcdf-4 requires that the array references dimension objects
that may appear in groups anywhere in the file. These references are encoded
as FQNs "pointing" to a specific dimension declaration (see *_nczarr_group* attribute
defined previously).

FQN is an acronym for "Fully Qualified Name".
It is a series of names separated by the "/" character, much
like a file system path.
It identifies the group in which the dimension is ostensibly "defined" in the Netcdf sense.
For example ````/d1```` defines a dimension "d1" defined in the root group.
Similarly ````/g1/g2/d2```` defines a dimension "d2" defined in the
group g2, which in turn is a subgroup of group g1, which is a subgroup
of the root group.

The *type_alias* key is used to annotate the type of an array
to allow discovery of netcdf-4 specific types.
Specifically, there are three current cases:
| dtype | type_alias |
| ----- | ---------- |
| uint8 | char       |
| rn    | string     |
| uint8 | json       |

If, for example, an array's dtype is specified as *uint8*, then it may be that
it is actually of unsigned 8-bit integer type. But it may actually be of some
netcdf-4 type that is encoded as *uint8* in order to be recognized by other -- pure zarr--
implementations. So, for example, if the netcdf-4 type is *char*, then the array's
dtype is *uint8*, but its type alias is *char*.

Optionally Inserted into any group zarr.json or array zarr.json is the extra attribute.
"_nczarr_attrs": {\"attribute_types\": [{\"name\": \"attr1\", \"configuration\": {\"type\": \"<dtype>\"}}, ...]}

*/

#define NCZ_V2_PREFIX "_nczarr"
#define NCZ_V2_SUPERBLOCK "_nczarr_superblock"
/* Must match values in include/nc4internal.h */
#define NCZ_V2_GROUP   "_nczarr_group"
#define NCZ_V2_ARRAY   "_nczarr_array"
#define NCZ_V2_ATTR    "_nczarr_attrs"

#define NCZ_V3_PREFIX NCZ_V2_PREFIX
#define NCZ_V3_SUPERBLOCK "_nczarr_superblock"
/* Must match values in include/nc4internal.h */
#define NCZ_V3_GROUP   "_nczarr_group"
#define NCZ_V3_ARRAY   "_nczarr_array"
#define NCZ_V3_ATTR    "_nczarr_attrs"

#define NCZARRCONTROL "nczarr"
#define PUREZARRCONTROL "zarr"
#define XARRAYCONTROL "xarray"
#define NOXARRAYCONTROL "noxarray"
#define XARRAYSCALAR "_scalar_"
#define DIMSCALAR "/_scalar_"
#define NCZARR_MAXSTRLEN_ATTR "_nczarr_maxstrlen"
#define NCZARR_DEFAULT_MAXSTRLEN_ATTR "_nczarr_default_maxstrlen"
#define FORMAT2CONTROL "v2"
#define FORMAT3CONTROL "v3"

#define LEGAL_DIM_SEPARATORS "./"
#define DFALT_DIM_SEPARATOR_V2 '.'
#define DFALT_DIM_SEPARATOR_V3 '/'

#define islegaldimsep(c) ((c) != '\0' && strchr(LEGAL_DIM_SEPARATORS,(c)) != NULL)

/* Extend the type system */
#define NC_JSON (NC_STRING+1)
#define N_NCZARR_TYPES (NC_JSON+1)

/* Default max string length for fixed length strings */
#define NCZ_MAXSTR_DEFAULT 128

/* Mnemonics */
#define ZCLOSE	 1 /* this is closeorabort as opposed to enddef */
#define ZREADING 1 /* this is reading data rather than writing */

/* Useful macro */
#define ncidforx(file,grpid) ((file)->controller->ext_ncid | (grpid))
#define ncidfor(var) ncidforx((var)->container->nc4_info,(var)->container->hdr.id)

/**************************************************/
/* Opaque */

struct NClist;
struct NCjson;
struct NCauth;
struct NCZMAP;
struct NCZChunkCache;
struct NCZ_Formatter;
struct NCproplist;
struct NCZ_META_HDR;

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
    struct Zarrformat {
	int zarr_format;
	int nczarr_format;
    } zarr;
    int creating; /* 1=> created 0=>open */
    int native_endianness; /* NC_ENDIAN_LITTLE | NC_ENDIAN_BIG */
    size_t default_maxstrlen; /* default max str size for variables of type string */
    NClist* urlcontrols; /* controls specified by the file url fragment */
    size64_t flags;
#		define FLAG_PUREZARR    1
#		define FLAG_SHOWFETCH   2
#		define FLAG_LOGGING     4
#		define FLAG_XARRAYDIMS  8
    NCZM_IMPL mapimpl;
    struct NCZ_Formatter* dispatcher;
    struct NCZ_META_HDR* metastate; /* Hold per-format state */
} NCZ_FILE_INFO_T;

/* This is a struct to handle the dim metadata. */
typedef struct NCZ_DIM_INFO {
    NCZcommon common;
    struct NCZ_META_HDR* metastate; /* Hold per-format state */
} NCZ_DIM_INFO_T;

/** Struct to hold ZARR-specific info for attributes. */
typedef struct  NCZ_ATT_INFO {
    NCZcommon common;
} NCZ_ATT_INFO_T;

/* Struct to hold ZARR-specific info for a group. */
typedef struct NCZ_GRP_INFO {
    NCZcommon common;
    NCjson* jatts; /* JSON encoding of the attributes; do not reclaim */
    struct NCZ_META_HDR* metastate; /* Hold per-format state */
} NCZ_GRP_INFO_T;

/* Struct to hold ZARR-specific info for a variable. */
typedef struct NCZ_VAR_INFO {
    NCZcommon common;
    size64_t chunkproduct; /* product of chunksizes */
    size64_t chunksize; /* chunkproduct * typesize */
    int order; /* 1=>column major, 0=>row major (default); not currently enforced */
    int scalar;
    struct NCZChunkCache* cache;
    struct NClist* dimension_names; /* names from _ARRAY_DIMENSIONS or dimension_names key */
    char dimension_separator; /* '.' | '/' */
    size_t maxstrlen; /* max length of strings for this variable */
    NCjson* jarray; /* Zarr.json; reclaim */
    const NCjson* jzarray; /* _nczarr_array: contains dimensions, attribute types, and storage type; do not reclaim */
    NCjson* jatts; /* JSON encoding of the attributes; do not reclaim */
    struct NCZ_META_HDR* metastate; /* Hold per-format state */
} NCZ_VAR_INFO_T;

/* Struct to hold ZARR-specific info for a field. */
typedef struct NCZ_FIELD_INFO {
    NCZcommon common;
} NCZ_FIELD_INFO_T;

/* Struct to hold ZARR-specific info for a type. */
typedef struct NCZ_TYPE_INFO {
    NCZcommon common;
} NCZ_TYPE_INFO_T;

/* Parsed dimension info */
typedef struct NCZ_DimInfo {
    char* name;
    char* fqn;
    size64_t shape;
    int unlimited;
} NCZ_DimInfo;

/* Parsed Attribute info */
struct NCZ_AttrInfo {
    char* name;
    nc_type nctype;
    NCjson* values;
};


/**************************************************/

/* Common property lists */
EXTERNL const struct NCproplist* NCplistzarrv2;
EXTERNL const struct NCproplist* NCplistzarrv3;

/**************************************************/

extern int ncz_initialized; /**< True if initialization has happened. */

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
int NCZ_zclose_var1(NC_VAR_INFO_T* var);

/* zattr.c */
int ncz_getattlist(NC_GRP_INFO_T *grp, int varid, NC_VAR_INFO_T **varp, NCindex **attlist);
int ncz_create_fillvalue(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
int NCZ_computeattrinfo(const char* name, nc_type atype, nc_type typehint, int purezarr, const NCjson* values, nc_type* typeidp, size_t* typelenp, size_t* lenp, void** datap);
int NCZ_computeattrdata(nc_type* typeidp, const NCjson* values, size_t* typelenp, size_t* countp, void** datap);
int NCZ_read_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts, const NCjson* jatypes);
int NCZ_attr_convert(const NCjson* src, nc_type typeid, size_t typelen, size_t* countp, NCbytes* dst);
int ncz_makeattr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name, nc_type typeid, size_t len, void* values, NC_ATT_INFO_T** attp);
int NCZ_attr_delete(NC_FILE_INFO_T* file, NCindex* attlist, const char* name);

/* zvar.c */
int ncz_gettype(NC_FILE_INFO_T*, NC_GRP_INFO_T*, int xtype, NC_TYPE_INFO_T** typep);
int ncz_find_default_chunksizes2(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);
int NCZ_ensure_quantizer(int ncid, NC_VAR_INFO_T* var);
int NCZ_write_var_data(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
int NCZ_fillin_var(NC_FILE_INFO_T* h5, NC_VAR_INFO_T* var, NC_TYPE_INFO_T* type, size_t ndims, const int* dimids, size64_t* shape, size64_t* chunksizes, int endianness);

/* zvar.c */
int NCZ_reclaim_dim(NC_DIM_INFO_T* dim);

/* Undefined */
/* Find var, doing lazy var metadata read if needed. */
int ncz_find_file_grp_var(int ncid, int varid, NC_FILE_INFO_T** file,
                             NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var);

#endif /* ZINTERNAL_H */

