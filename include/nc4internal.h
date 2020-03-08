/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 * @internal This header file contains macros, types and prototypes
 * used to build and manipulate the netCDF metadata model.
 *
 * @author Ed Hartnett, Dennis Heimbigner, Ward Fisher
 */

#ifndef _NC4INTERNAL_
#define _NC4INTERNAL_
#include "netcdf.h"

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "ncdimscale.h"
#include "nc_logging.h"
#include "ncindex.h"
#include "nc_provenance.h"


#include "netcdf_f.h"
#include "netcdf_mem.h"
#include "netcdf_filter.h"
#ifdef USE_PARALLEL
#include "netcdf_par.h"
#endif /* USE_PARALLEL */

/* Always needed */
#include "nc.h"

/** The file ID is stored in the first two bytes of ncid. */
#define FILE_ID_MASK (0xffff0000)

/** The group ID is stored in the last two bytes of ncid. */
#define GRP_ID_MASK (0x0000ffff)

/** File and group IDs are each 16 bits of the ncid. */
#define ID_SHIFT (16)

/* typedef enum {GET, PUT} NC_PG_T; */
/** These are the different objects that can be in our hash-lists. */
typedef enum {NCNAT, NCVAR, NCDIM, NCATT, NCTYP, NCFLD, NCGRP} NC_SORT;

/** The netCDF V2 error code. */
#define NC_V2_ERR (-1)

/** The name of the root group. */
#define NC_GROUP_NAME "/"

/** One mega-byte. */
#define MEGABYTE 1048576

/** The HDF5 ID for the szip filter. */
#define HDF5_FILTER_SZIP 4

#define X_SCHAR_MIN     (-128)          /**< Minimum signed char value. */
#define X_SCHAR_MAX     127             /**< Maximum signed char value. */
#define X_UCHAR_MAX     255U            /**< Maximum unsigned char value. */
#define X_SHORT_MIN     (-32768)        /**< Minimum short value. */
#define X_SHRT_MIN      X_SHORT_MIN     /**< This alias is compatible with limits.h. */
#define X_SHORT_MAX     32767           /**< Maximum short value. */
#define X_SHRT_MAX      X_SHORT_MAX     /**< This alias is compatible with limits.h. */
#define X_USHORT_MAX    65535U          /**< Maximum unsigned short value. */
#define X_USHRT_MAX     X_USHORT_MAX    /**< This alias is compatible with limits.h. */
#define X_INT_MIN       (-2147483647-1) /**< Minimum int value. */
#define X_INT_MAX       2147483647      /**< Maximum int value. */
#define X_LONG_MIN      X_INT_MIN       /**< Minimum long value. */
#define X_LONG_MAX      X_INT_MAX       /**< Maximum long value. */
#define X_UINT_MAX      4294967295U     /**< Maximum unsigned int value. */
#define X_INT64_MIN     (-9223372036854775807LL-1LL)  /**< Minimum int64 value. */
#define X_INT64_MAX     9223372036854775807LL /**< Maximum int64 value. */
#define X_UINT64_MAX    18446744073709551615ULL /**< Maximum unsigned int64 value. */
#ifdef _WIN32 /* Windows, of course, has to be a *little* different. */
#define X_FLOAT_MAX     3.402823466e+38f
#else
#define X_FLOAT_MAX     3.40282347e+38f /**< Maximum float value. */
#endif /* _WIN32 */
#define X_FLOAT_MIN     (-X_FLOAT_MAX)  /**< Minimum float value. */
#define X_DOUBLE_MAX    1.7976931348623157e+308 /**< Maximum double value. */
#define X_DOUBLE_MIN    (-X_DOUBLE_MAX)         /**< Minimum double value. */

/** This is the number of netCDF atomic types. */
#define NUM_ATOMIC_TYPES (NC_MAX_ATOMIC_TYPE + 1)

/** Number of parameters needed for ZLIB filter. */
#define CD_NELEMS_ZLIB 1

/** Get a pointer to the NC_FILE_INFO_T from dispatchdata field. */
#define NC4_DATA(nc) ((NC_FILE_INFO_T *)(nc)->dispatchdata)

/** Set a pointer to the NC_FILE_INFO_T in the dispatchdata field. */
#define NC4_DATA_SET(nc,data) ((nc)->dispatchdata = (void *)(data))

/* Reserved attribute flags: must be powers of 2. */
/** Hidden dimscale-related, per-variable attributes; immutable and
 * unreadable thru API. */
#define DIMSCALEFLAG 1

/** Readonly global attributes; readable, but immutable thru the
 * API. */
#define READONLYFLAG 2

/** Subset of readonly flags; readable by name only thru the API. */
#define NAMEONLYFLAG 4

/** Subset of readonly flags; Value is actually in file. */
#define MATERIALIZEDFLAG 8

/* Generic reserved Attributes */
#define NC_ATT_REFERENCE_LIST "REFERENCE_LIST" /**< HDF5 reference list attribute name. */
#define NC_ATT_CLASS "CLASS"                   /**< HDF5 class atttribute name. */
#define NC_ATT_DIMENSION_LIST "DIMENSION_LIST" /**< HDF5 dimension list attribute name. */
#define NC_ATT_NAME "NAME"                     /**< HDF5 name atttribute name. */
#define NC_ATT_COORDINATES COORDINATES         /**< Coordinates atttribute name. */
#define NC_ATT_FORMAT "_Format"                /**< Format atttribute name. */

/** Boolean type, to make the code easier to read. */
typedef enum {NC_FALSE = 0, NC_TRUE = 1} nc_bool_t;

/* Forward declarations. */
struct NC_GRP_INFO;
struct NC_TYPE_INFO;
struct NC_FIlterobject;

/**
 * This struct provides indexed Access to Meta-data objects. See the
 * document docs/indexing.dox for detailed information.
 *
 * Basically it provides a common header and use NCindex instances
 * instead of linked lists.
 *
 * WARNING: ALL OBJECTS THAT CAN BE INSERTED INTO AN NCindex MUST HAVE
 * AN INSTANCE of NC_OBJ AS THE FIRST FIELD.
*/
typedef struct NC_OBJ
{
    NC_SORT sort; /**< Type of object. */
    char* name;   /**< Name, assumed to be null terminated. */
    size_t id;    /**< This objects ID. */
    unsigned int hashkey; /**< The hash key, crc32(name). */
} NC_OBJ;

/**
 * This struct holds information about reserved attributes. These
 * attributes cannot be created or read by the user (through the
 * netCDF API).  */
typedef struct NC_reservedatt
{
    const char *name; /**< Name of the reserved attribute. */
    int flags;        /**< Flags that control handling of reserved attribute. */
} NC_reservedatt;

/** This is a struct to handle the dimension metadata. */
typedef struct NC_DIM_INFO
{
    NC_OBJ hdr;             /**< The hdr contains the name and ID. */
    struct NC_GRP_INFO *container;  /**< Pointer to containing group. */
    size_t len;             /**< Length of this dimension. */
    nc_bool_t unlimited;    /**< True if the dimension is unlimited */
    nc_bool_t extended;     /**< True if the dimension needs to be extended. */
    nc_bool_t too_long;     /**< True if len is too big to fit in local size_t. */
    void *format_dim_info;  /**< Pointer to format-specific dim info. */
    struct NC_VAR_INFO *coord_var; /**< The coord var, if it exists. */
} NC_DIM_INFO_T;

/** This is a struct to handle the attribute metadata. */
typedef struct NC_ATT_INFO
{
    NC_OBJ hdr;             /**< The hdr contains the name and ID. */
    struct NC_OBJ *container;    /**< Pointer to containing group|var. */
    int len;                /**< Length of attribute data. */
    nc_bool_t dirty;        /**< True if attribute modified. */
    nc_bool_t created;      /**< True if attribute already created. */
    nc_type nc_typeid;      /**< NetCDF type of attribute's data. */
    void *format_att_info;  /**< Pointer to format-specific att info. */
    void *data;             /**< The attribute data. */
    nc_vlen_t *vldata;      /**< VLEN data (only used for vlen types). */
    char **stdata;          /**< String data (only for string type). */
} NC_ATT_INFO_T;

/** This is a struct to handle the var metadata. */
typedef struct NC_VAR_INFO
{
    NC_OBJ hdr;                  /**< The hdr contains the name and ID. */
    char *hdf5_name;             /**< Used if name in HDF5 must be different from name. */
    struct NC_GRP_INFO *container; /**< Pointer to containing group. */
    size_t ndims;                /**< Number of dims. */
    int *dimids;                 /**< Dim IDs. */
    NC_DIM_INFO_T **dim;         /**< Pointer to array of NC_DIM_INFO_T. */
    nc_bool_t is_new_var;        /**< True if variable is newly created. */
    nc_bool_t was_coord_var;     /**< True if variable was a coordinate var, but either the dim or var has been renamed. */
    nc_bool_t became_coord_var;  /**< True if variable _became_ a coordinate var, because either the dim or var has been renamed. */
    nc_bool_t fill_val_changed;  /**< True if variable's fill value changes after it has been created. */
    nc_bool_t attr_dirty;        /**< True if variable's attributes are dirty and should be rewritten. */
    nc_bool_t created;           /**< Variable has already been created (_not_ that it was just created). */
    nc_bool_t written_to;        /**< True if variable has data written to it. */
    struct NC_TYPE_INFO *type_info; /**< Contains info about the variable type. */
    int atts_read;               /**< If true, the atts have been read. */
    nc_bool_t meta_read;         /**< True if this vars metadata has been completely read. */
    nc_bool_t coords_read;       /**< True if this var has hidden coordinates att, and it has been read. */
    NCindex *att;                /**< List of NC_ATT_INFO_T. */
    nc_bool_t no_fill;           /**< True if no fill value is defined for var. */
    void *fill_value;            /**< Pointer to fill value, or NULL. */
    size_t *chunksizes;          /**< For chunked storage, an array (size ndims) of chunksizes. */
    int storage;                 /**< Storage of this var, compact, contiguous, or chunked. */
    int parallel_access;         /**< Type of parallel access for I/O on variable (collective or independent). */
    nc_bool_t dimscale;          /**< True if var is a dimscale. */
    nc_bool_t *dimscale_attached;  /**< Array of flags that are true if dimscale is attached for that dim index. */
    nc_bool_t shuffle;           /**< True if var has shuffle filter applied. */
    nc_bool_t fletcher32;        /**< True if var has fletcher32 filter applied. */
    size_t chunk_cache_size;     /**< Size in bytes of the var chunk chache. */
    size_t chunk_cache_nelems;   /**< Number of slots in var chunk cache. */
    float chunk_cache_preemption; /**< Chunk cache preemtion policy. */
    void *format_var_info;       /**< Pointer to any binary format info. */
    NClist* filters;             /**< List of filters to be applied to var data.  */
} NC_VAR_INFO_T;

/** This is a struct to handle the field metadata from a user-defined
 * type. */
typedef struct NC_FIELD_INFO
{
    NC_OBJ hdr; /**< The hdr contains the name and ID. */
    nc_type nc_typeid; /**< The type of this field. */
    size_t offset;     /**< Offset in bytes of field. */
    int ndims;         /**< Number of dims. */
    int *dim_size;     /**< Dim sizes. */
    void *format_field_info;  /**< Pointer to any binary format info for field. */
} NC_FIELD_INFO_T;

/** This is a struct to handle metadata for a user-defined enum
 * type. */
typedef struct NC_ENUM_MEMBER_INFO
{
    char *name;  /**< Name of member. */
    void *value; /**< Value of member. */
} NC_ENUM_MEMBER_INFO_T;

/** This is a struct to handle metadata for a user-defined type. */
typedef struct NC_TYPE_INFO
{
    NC_OBJ hdr;            /**< The hdr contains the name and ID. */
    struct NC_GRP_INFO *container; /**< Containing group */
    unsigned rc;                 /**< Ref. count of objects using this type */
    int endianness;              /**< What endianness for the type? */
    size_t size;                 /**< Size of the type in memory, in bytes */
    nc_bool_t committed;         /**< True when datatype is committed in the file */
    nc_type nc_type_class;       /**< NC_VLEN, NC_COMPOUND, NC_OPAQUE, NC_ENUM, NC_INT, NC_FLOAT, or NC_STRING. */
    void *format_type_info;      /**< HDF5-specific type info. */

    /** Information for each type or class */
    union {
        struct {
            NClist* enum_member;    /**< <! NClist<NC_ENUM_MEMBER_INFO_T*> */
            nc_type base_nc_typeid; /**< Typeid of the base type. */
        } e;                        /**< Enum */
        struct Fields {
            NClist* field;        /**< <! NClist<NC_FIELD_INFO_T*> */
        } c;                      /**< Compound */
        struct {
            nc_type base_nc_typeid; /**< Typeid of the base type. */
        } v;                     /**< Variable-length. */
    } u;                         /**< Union of structs, for each type/class. */
} NC_TYPE_INFO_T;

/** This holds information for one group. Groups reproduce with
 * parthenogenesis. */
typedef struct NC_GRP_INFO
{
    NC_OBJ hdr;                   /**< The hdr contains the name and ID. */
    void *format_grp_info;        /**< Pointer to binary format info for group. */
    struct NC_FILE_INFO *nc4_info; /**< Pointer containing NC_FILE_INFO_T. */
    struct NC_GRP_INFO *parent;  /**< Pointer tp parent group. */
    int atts_read;               /**< True if atts have been read for this group. */
    NCindex* children;           /**< NCindex<struct NC_GRP_INFO*> */
    NCindex* dim;                /**< NCindex<NC_DIM_INFO_T> * */
    NCindex* att;                /**< NCindex<NC_ATT_INFO_T> * */
    NCindex* type;               /**< NCindex<NC_TYPE_INFO_T> * */
    /* Note that this is the list of vars with position == varid */
    NCindex* vars;               /**< NCindex<NC_VAR_INFO_T> * */
} NC_GRP_INFO_T;

/* These constants apply to the cmode parameter in the
 * HDF5_FILE_INFO_T defined below. */
#define NC_CREAT 2      /**< in create phase, cleared by ncendef */
#define NC_INDEF 8      /**< in define mode, cleared by ncendef */
#define NC_NSYNC 0x10   /**< synchronise numrecs on change */
#define NC_HSYNC 0x20   /**< synchronise whole header on change */
#define NC_NDIRTY 0x40  /**< numrecs has changed */
#define NC_HDIRTY 0x80  /**< header info has changed */

/** This is the metadata we need to keep track of for each
 * netcdf-4/HDF5 file. */
typedef struct  NC_FILE_INFO
{
    NC *controller; /**< Pointer to containing NC. */
#ifdef USE_PARALLEL4
    MPI_Comm comm;  /**< Copy of MPI Communicator used to open the file. */
    MPI_Info info;  /**< Copy of MPI Information Object used to open the file. */
#endif
    int flags;      /**< Flags used to open the file. */
    int cmode;      /**< Create mode used to create the file. */
    nc_bool_t parallel;   /**< True if file is open for parallel access */
    nc_bool_t redef;      /**< True if redefining an existing file */
    int fill_mode;        /**< Fill mode for vars - Unused internally currently */
    nc_bool_t no_write;   /**< true if nc_open has mode NC_NOWRITE. */
    NC_GRP_INFO_T *root_grp; /**< Pointer to root group. */
    short next_nc_grpid;  /**< Next available group ID. */
    int next_typeid;      /**< Next available type ID. */
    int next_dimid;       /**< Next available dim ID. */
    /* Provide convenience vectors indexed by the object id.  This
       allows for direct conversion of e.g. an nc_type to the
       corresponding NC_TYPE_INFO_T object. */
    NClist *alldims;   /**< List of all dims. */
    NClist *alltypes;  /**< List of all types. */
    NClist *allgroups; /**< List of all groups, including root group. */
    void *format_file_info; /**< Pointer to binary format info for file. */
    NC4_Provenance provenance; /**< File provenence info. */
    struct NC4_Memio
    {
        NC_memio memio; /**< What we sent to image_init and what comes back. */
        int locked;     /**< Do not copy and do not free. */
        int persist;    /**< Should file be persisted out on close? */
        int inmemory;   /**< NC_INMEMORY flag was set. */
        int diskless;   /**< NC_DISKLESS flag was set => inmemory. */
        int created;    /**< 1 => create, 0 => open. */
        unsigned int imageflags; /**< for H5LTopen_file_image. */
        size_t initialsize;      /**< Initial size. */
        void *udata;    /**< Extra memory allocated in NC4_image_init. */
    } mem;
} NC_FILE_INFO_T;

/** Variable Length Datatype struct in memory. Must be identical to
 * HDF5 hvl_t. (This is only used for VL sequences, not VL strings,
 * which are stored in char *'s) */
typedef struct
{
    size_t len; /**< Length of VL data (in base type units) */
    void *p;    /**< Pointer to VL data */
} nc_hvl_t;

/** The names of the atomic data types. */
extern const char *nc4_atomic_name[NC_MAX_ATOMIC_TYPE + 1];

/* These functions convert between netcdf and HDF5 types. */
int nc4_get_typelen_mem(NC_FILE_INFO_T *h5, nc_type xtype, size_t *len);
int nc4_convert_type(const void *src, void *dest, const nc_type src_type,
                     const nc_type dest_type, const size_t len, int *range_error,
                     const void *fill_value, int strict_nc3);

/* These functions do HDF5 things. */
int nc4_reopen_dataset(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);
int nc4_read_atts(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);

/* Find items in the in-memory lists of metadata. */
int nc4_find_nc_grp_h5(int ncid, NC **nc, NC_GRP_INFO_T **grp,
                       NC_FILE_INFO_T **h5);
int nc4_find_grp_h5(int ncid, NC_GRP_INFO_T **grp, NC_FILE_INFO_T **h5);
int nc4_find_nc4_grp(int ncid, NC_GRP_INFO_T **grp);
int nc4_find_dim(NC_GRP_INFO_T *grp, int dimid, NC_DIM_INFO_T **dim,
                 NC_GRP_INFO_T **dim_grp);
int nc4_find_var(NC_GRP_INFO_T *grp, const char *name, NC_VAR_INFO_T **var);
int nc4_find_dim_len(NC_GRP_INFO_T *grp, int dimid, size_t **len);
int nc4_find_type(const NC_FILE_INFO_T *h5, int typeid1, NC_TYPE_INFO_T **type);
NC_TYPE_INFO_T *nc4_rec_find_named_type(NC_GRP_INFO_T *start_grp, char *name);
NC_TYPE_INFO_T *nc4_rec_find_equal_type(NC_GRP_INFO_T *start_grp, int ncid1,
                                        NC_TYPE_INFO_T *type);
int nc4_find_nc_att(int ncid, int varid, const char *name, int attnum,
                    NC_ATT_INFO_T **att);
int nc4_find_grp_h5_var(int ncid, int varid, NC_FILE_INFO_T **h5,
                        NC_GRP_INFO_T **grp, NC_VAR_INFO_T **var);
int nc4_find_grp_att(NC_GRP_INFO_T *grp, int varid, const char *name,
                     int attnum, NC_ATT_INFO_T **att);
int nc4_get_typeclass(const NC_FILE_INFO_T *h5, nc_type xtype,
                      int *type_class);

/* Free various types */
int nc4_type_free(NC_TYPE_INFO_T *type);

/* These list functions add and delete vars, atts. */
int nc4_nc4f_list_add(NC *nc, const char *path, int mode);
int nc4_nc4f_list_del(NC_FILE_INFO_T *h5);
int nc4_file_list_add(int ncid, const char *path, int mode,
                      void **dispatchdata);
int nc4_file_list_get(int ncid, char **path, int *mode,
                      void **dispatchdata);
int nc4_file_list_del(int ncid);
int nc4_file_change_ncid(int ncid, unsigned short new_ncid_index);
int nc4_var_list_add(NC_GRP_INFO_T* grp, const char* name, int ndims,
                     NC_VAR_INFO_T **var);
int nc4_var_list_add2(NC_GRP_INFO_T* grp, const char* name,
                      NC_VAR_INFO_T **var);
int nc4_var_set_ndims(NC_VAR_INFO_T *var, int ndims);
int nc4_var_list_del(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);
int nc4_dim_list_add(NC_GRP_INFO_T *grp, const char *name, size_t len,
                     int assignedid, NC_DIM_INFO_T **dim);
int nc4_dim_list_del(NC_GRP_INFO_T *grp, NC_DIM_INFO_T *dim);
int nc4_type_new(size_t size, const char *name, int assignedid,
                 NC_TYPE_INFO_T **type);
int nc4_type_list_add(NC_GRP_INFO_T *grp, size_t size, const char *name,
                      NC_TYPE_INFO_T **type);
int nc4_type_list_del(NC_GRP_INFO_T *grp, NC_TYPE_INFO_T *type);
int nc4_type_free(NC_TYPE_INFO_T *type);
int nc4_field_list_add(NC_TYPE_INFO_T* parent, const char *name,
                       size_t offset, nc_type xtype, int ndims,
                       const int *dim_sizesp);
int nc4_att_list_add(NCindex *list, const char *name, NC_ATT_INFO_T **att);
int nc4_att_list_del(NCindex *list, NC_ATT_INFO_T *att);
int nc4_grp_list_add(NC_FILE_INFO_T *h5, NC_GRP_INFO_T *parent, char *name,
                     NC_GRP_INFO_T **grp);
int nc4_build_root_grp(NC_FILE_INFO_T *h5);
int nc4_rec_grp_del(NC_GRP_INFO_T *grp);
int nc4_enum_member_add(NC_TYPE_INFO_T *type, size_t size, const char *name,
                        const void *value);

/* Check and normalize names. */
int NC_check_name(const char *name);
int nc4_check_name(const char *name, char *norm_name);
int nc4_normalize_name(const char *name, char *norm_name);
int nc4_check_dup_name(NC_GRP_INFO_T *grp, char *norm_name);

/* Find default fill value. */
int nc4_get_default_fill_value(const NC_TYPE_INFO_T *type_info, void *fill_value);

/* Get an att given pointers to file, group, and perhaps ver info. */
int nc4_get_att_ptrs(NC_FILE_INFO_T *h5, NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var,
                     const char *name, nc_type *xtype, nc_type mem_type,
                     size_t *lenp, int *attnum, void *data);

/* Close the file. */
int nc4_close_netcdf4_file(NC_FILE_INFO_T *h5, int abort, NC_memio *memio);

/* HDF5 initialization/finalization */
extern int nc4_hdf5_initialized;
extern void nc4_hdf5_initialize(void);
extern void nc4_hdf5_finalize(void);

/* This is only included if --enable-logging is used for configure; it
   prints info about the metadata to stderr. */
#ifdef LOGGING
int log_metadata_nc(NC_FILE_INFO_T *h5);
#endif

/* Binary searcher for reserved attributes */
extern const NC_reservedatt *NC_findreserved(const char *name);

/**************************************************/
/* Internal filter related structures */

/* Internal filter actions */
#define NCFILTER_DEF		1
#define NCFILTER_REMOVE  	2
#define NCFILTER_INQ	    	3
#define NCFILTER_FILTERIDS      4
#define NCFILTER_INFO		5
#define NCFILTER_FREESPEC	6
#define NCFILTER_CLIENT_REG	10
#define NCFILTER_CLIENT_UNREG	11
#define NCFILTER_CLIENT_INQ	12

typedef enum NC_FILTER_SORT {
	NC_FILTER_SORT_SPEC=((int)1),
	NC_FILTER_SORT_IDS=((int)2),
	NC_FILTER_SORT_CLIENT=((int)3),
} NC_FILTER_SORT;

/* Provide structs to pass args to filter_actions function for HDF5*/

typedef struct NC_FILTER_SPEC_HDF5 {
    int active;            /**< true iff HDF5 library was told to activate filter */
    unsigned int filterid; /**< ID for arbitrary filter. */
    size_t nparams;        /**< nparams for arbitrary filter. */
    unsigned int* params;  /**< Params for arbitrary filter. */
} NC_FILTER_SPEC_HDF5;

typedef struct NC_FILTERIDS_HDF5 {
    size_t nfilters;          /**< number of filters */
    unsigned int* filterids;  /**< Filter ids. */
} NC_FILTERIDS_HDF5;

typedef struct NC_FILTER_CLIENT_HDF5 {
    unsigned int id;
    /* The filter info for hdf5 */
    /* Avoid needing hdf.h by using void* */
    void* info;
} NC_FILTER_CLIENT_HDF5;

typedef struct NC_FILTER_OBJ_HDF5 {
    NC_Filterobject hdr; /* So we can cast it */
    NC_FILTER_SORT sort; /* discriminate union */
    union {
        NC_FILTER_SPEC_HDF5 spec;
        NC_FILTERIDS_HDF5 ids;
        NC_FILTER_CLIENT_HDF5 client;
    } u;
} NC_FILTER_OBJ_HDF5;

extern void NC4_freefilterspec(NC_FILTER_SPEC_HDF5* f);

#endif /* _NC4INTERNAL_ */
