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

/* Map the Zarr Format version to a string */
#define ZARR_FORMAT_VERSION_TEMPLATE "%d"

/* Map the NCZarr Format version to a string */
#define NCZARR_FORMAT_VERSION_TEMPLATE "%d.0.0"

/* The name of the env var for changing default zarr format */
#define NCZARRDEFAULTFORMAT "NCZARRFORMAT"

/* The name of the env var for controlling .zmetadata use*/
#define NCZARRDEFAULTNOMETA "NCNOZMETADATA"

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

/* V2 Reserved Objects */#
#define Z2METADATA "/.zmetadata"
#define Z2METAROOT "/.zgroup"
#define Z2ATTSROOT "/.zattrs"
#define Z2GROUP ".zgroup"
#define Z2ATTRS ".zattrs"
#define Z2ARRAY ".zarray"

/* V3 Reserved Objects */
#define Z3METADATA "/zarr.json"
#define Z3METAROOT Z3METADATA
#define Z3OBJECT "zarr.json"
#define Z3GROUP Z3OBJECT
#define Z3ARRAY Z3OBJECT
#define Z3CHUNK "c"

/* Bytes codec name */
#define ZBYTES3 "bytes"

/* V2 Reserved Attributes */
/*
For nczarr versions 2.x.x, the following (key,value)
pairs are stored as if they were standard attributes.
The cost is that lazy attribute reading is no longer possible.

Inserted into /.zattrs in the root group
_nczarr_superblock: {"version": "3.0.0", "format=2"}

Inserted into any .zattrs (at group level)
"_nczarr_group": "{
\"dimensions\": {<dimname>: <integer>, <name>: <integer>,...}
\"arrays\": [\"v1\", \"v2\", ...]
\"groups\": [\"g1\", \"g2\", ...]
}"

Optionally insert into any .zattrs (at root group level)
\"_nczarr_default_maxstrlen\": <integer>
This is needed only when writing a dataset. When reading, it should be redundant
vis-a-vis the actual length (e.g. "|S6") of the dtype of a string variable.

Inserted into any .zattrs (at array level)
"_nczarr_array": "{
\"dimension_references\": [\"/g1/g2/d1\", \"/d2\",...],
\"storage\": \"contiguous\" | \"chunked\",
\"scalar\": 0 | 1
}"
Note that the storage key is probably irrelevant currently because
all data is stored in Zarr in the equivalent of "chunked".
Note also that if scalar is "1", then storage will still be chunked,
and the array will have shape of "[1]" and optionally, a dimension name
of "_scalar_". For external pure zarr datasets, there will not be enough
information to signal scalar, so such an array will be treated as a one
element chunk.

If an array type is a string, the optionally insert into any .zattrs (at array level)
\"_nczarr_maxstrlen\": <integer>
This is needed only when writing a dataset. When reading, it should be redundant
vis-a-vis the length (e.g. "|S6") of the dtype of the variable.

Inserted into any .zattrs
"_nczarr_attrs": "{
\"types\": {\"attr1\": \"<i4\", \"attr2\": \"<i1\",...}
}

*/

/* V3 Reserved Attributes */
/*

For nczarr version 3.x.x, the following (key,value)
pairs are stored as if they were standard attributes.
The cost is that lazy attribute reading is no longer possible.

Inserted into root group zarr.json as an extra attribute.
_nczarr_superblock: {
    "version": 3.0.0,    
    "format": 3
}

Optionally inserted into any group zarr.json as an attribute:
"_nczarr_group": "{
\"dimensions\": [{name: <dimname>, size: <integer>, unlimited: 1|0},...],
   The V2 format is also acceptable: {<dimname>: <integer>, <name>: <integer>,...}
\"arrays\": ["<name>",...],
\"subgroups\": ["<name>",...]
}"

Optionally inserted into any array zarr.json as an attribute:
````
"_nczarr_array": "{
\"dimension_references\": [\"/g1/g2/d1\", \"/d2\",...],
\"nczarr_type\": "<dtype specifing nczarr/netcdf-4 type>" // optional
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

The *nczarr_type* key is used to annotate the type of an array
to allow use of netcdf-4 specific types not represented in the standard Zarr V3 types.
Specifically, there are three current cases:
| dtype | nczarr_type |
| ----- | ----------- |
| uint8 | char        |
| rn    | string      |

If, for example, an array's dtype is specified as *uint8*, then it may be that
it is actually of unsigned 8-bit integer type. But it may actually be of some
netcdf-4 type that is encoded as *uint8* in order to be recognized by other -- pure zarr--
implementations. So, for example, if the netcdf-4 type is *char*, then the array's
dtype is *uint8*, but its nczarr type is *char*.

Optionally Inserted into any group zarr.json or array zarr.json is the extra attribute.
"_nczarr_attrs": {\"attribute_types\": [{\"name\": \"attr1\", \"configuration\": {\"type\": \"<dtype>\"}}, ...]}

*/

/* Common constants for both V2 and V3 */
/* Must match values in include/nc4internal.h */
#define NCZ_PREFIX "_nczarr"
#define NCZ_ATTR_OLD_ATTR NC_NCZARR_ATTR_ATTR

#define NCZARRCONTROL "nczarr"
#define PUREZARRCONTROL "zarr"
#define XARRAYCONTROL "xarray"
#define NOXARRAYCONTROL "noxarray"
#define XARRAYSCALAR "_scalar_"
#define DIMSCALAR "/_scalar_"
#define FORMAT2CONTROL "v2"
#define FORMAT3CONTROL "v3"
#define ZMETADATACONTROL "zmetadata"
#define NOZMETADATACONTROL "nozmetadata"

#define LEGAL_DIM_SEPARATORS "/."
#define DFALT_DIM_SEPARATOR_V2 '.'
#define DFALT_DIM_SEPARATOR_V3 '/'

#define islegaldimsep(c) ((c) != '\0' && strchr(LEGAL_DIM_SEPARATORS,(c)) != NULL)

/* Extend the type system */
#define NC_JSON (NC_STRING+1)
#define N_NCZARR_TYPES (NC_JSON+1)
#define NC_JSON_DTYPE_V2 "|J0"
#define NC_JSON_DTYPE_V3 "json"

/* Default max string length for fixed length strings */
#define NCZ_MAXSTR_DFALT 128

/* Mnemonics */
#define ZCLOSE	 1 /* this is closeorabort as opposed to enddef */
#define ZREADING 1 /* this is reading data rather than writing */
#define FIXATT 0
#define FIXOBJ 1

//#define FORVAR 1
//#define FORGRP 2

/* Track the possible cases where a field in some NC_XXX_INFO_T*
   must be sync'd with corresponding Attribute
*/
typedef enum DualAtt {DA_NOT, DA_FILLVALUE, DA_MAXSTRLEN, DA_DFALTSTRLEN, DA_SEP, DA_DFALTSEP, DA_QUANTIZE, DA_ALL} DualAtt;

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
    struct NCZ_Metadata metadata_handler;
    struct Zarrformat {
	int zarr_format;
	int nczarr_format;
	char dimension_separator;
    } zarr;
    int creating; /* 1=> created 0=>open */
    int native_endianness; /* NC_ENDIAN_LITTLE | NC_ENDIAN_BIG */
    size_t default_maxstrlen; /* default max str size for variables of type string */
    NClist* urlcontrols; /* controls specified by the file url fragment */
    size64_t flags;
#		define FLAG_PUREZARR         1	/* NCZarr metadata, if present, should be suppressed */
#		define FLAG_SHOWFETCH        2	/* [show=fetch] debug output requested */
#		define FLAG_LOGGING          4	/* [log] debug output requested */
#		define FLAG_XARRAYDIMS       8	/* Utilize the xarray _ARRAY_DIMENSIONS attribute */
#		define FLAG_NCZARR_KEY	    16	/* _nczarr_xxx keys are stored in object and not in _nczarr_attrs */
#		define FLAG_NOCONSOLIDATED  32	/* Suppress consolidated metadata */
#		define FLAG_ZOH		    32	/* Usind ZoH Protocol */
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
    struct NCZ_META_HDR* metastate; /* Hold per-format state */
    /* Read json group and json attrs once */
#if 0
    struct ZARROBJ {
	char* prefix; /* prefix of group and attrs */
	NCjson* obj; /* V2->.zgroup|.zarray, V3->zarr.json */
	NCjson* atts; /* V2->.zattrs, V3->attributes */
	int constatts; /* 1=>do not reclaim atts field */
        int nczkey;   /* 1 => _nczarr_xxx are in obj as keys and not attributes */
    } zgroup;
#endif
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
    struct NCZ_META_HDR* metastate; /* Hold per-format state */
    int nonstdchunkkey; /* 1 => use a non-standard chunk key encoding; always 0 for V2. For V2, 1 => use V3 encoding */
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
    char norm_name[NC_MAX_NAME+1];
    size64_t shape;
    int unlimited;
} NCZ_DimInfo;

/* Dimension declaration info */
typedef struct NCZ_DimDecl {
    char* fqn;
    size64_t shape;
} NCZ_DimDecl;

/* Parsed Attribute info */
struct NCZ_AttrInfo {
    const char* name;
    nc_type nctype;
    size_t typelen;
    int endianness;
    size_t datalen;
    void* data;
};

/* Intermediate JSON results */
struct ZCVT {
    signed long long int64v;
    unsigned long long uint64v;
    double float64v;
    char* strv; /* null terminated utf-8 */
};
#define zcvt_empty {0,0,0.0,NULL}

/* In a number of places, it is desirable
   to -- in effect -- pass a pointer variable by reference
   so that the called function can manage the memory
   contained in that variable (i.e. reclaim it
   and ensure the variable is empty.
   To mark this situation, we define a struct
   containing a pointer to the pointer variable
   and pass an instance of that struct to the called
   function. Using this method is more or less
   equivalent to making an argument be in+out
   as opposed to out only.
*/
typedef struct NCREF {void* *ref;} NCREF;

/* Invoke a function with a single ref argument */
#define callref(var,expr) { \
	NCREF ref = {(void**)&var}; \
	stat = (expr);      \
        assert(var == NULL);\
	if(stat) goto done;}

/* Take control of the contents of the ref'd variable */
#define xferref(var,r,type) {var = *((type**)(r).ref); *((r).ref) = NULL;}

/**************************************************/
/* Constants */

/* Common property lists */
EXTERNL const struct NCproplist* NCplistzarrv2;
EXTERNL const struct NCproplist* NCplistzarrv3;

EXTERNL struct NCZ_AttrInfo NC_emptyAttrInfo();

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
void zsetmaxstrlen(size_t maxstrlen, NC_VAR_INFO_T* var);
void zsetdfaltstrlen(size_t maxstrlen, NC_FILE_INFO_T* file);
void zsetdimsep(char dimsep, NC_VAR_INFO_T* var);
void zsetdfaltdimsep(char dimsep, NC_FILE_INFO_T* file);

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
int NCZ_read_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts, const NCjson* jatypes);
int NCZ_attr_convert(const NCjson* src, nc_type typeid, size_t* countp, NCbytes* dst);
int ncz_makeattr(NC_FILE_INFO_T* file, NC_OBJ* container, struct NCZ_AttrInfo* ainfo, NC_ATT_INFO_T** attp);
int NCZ_attr_delete(NC_FILE_INFO_T* file, NCindex* attlist, const char* name);
int NCZ_getattr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* aname, nc_type nctype, NC_ATT_INFO_T** attp, int* isnewp);;
int NCZ_reclaim_att_data(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att);
int NCZ_set_dual_obj_data(NC_FILE_INFO_T* file, NC_OBJ* object, const char* name, DualAtt which, size_t len, const void* data);
int NCZ_set_att_data(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, size_t len, const void* data);
int NCZ_sync_dual_att(NC_FILE_INFO_T* file, NC_OBJ* container, const char* aname, DualAtt which, int direction);
int NCZ_ensure_dual_attributes(NC_FILE_INFO_T* file, NC_OBJ* container);
DualAtt NCZ_is_dual_att(const char* aname);
NC_SORT NCZ_dual_att_container(DualAtt da);

/* zvar.c */
int ncz_gettype(NC_FILE_INFO_T*, NC_GRP_INFO_T*, int xtype, NC_TYPE_INFO_T** typep);
int ncz_find_default_chunksizes2(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);
int NCZ_ensure_quantizer(int ncid, NC_VAR_INFO_T* var);
int NCZ_write_var_data(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
int NCZ_reclaim_dim(NC_DIM_INFO_T* dim);

/* Undefined */
/* Find var, doing lazy var metadata read if needed. */
int ncz_find_file_grp_var(int ncid, int varid, NC_FILE_INFO_T** file, NC_GRP_INFO_T** grp, NC_VAR_INFO_T** var);

#endif /* ZINTERNAL_H */

