/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/** @file ncd4types.h
 * Type declarations and associated constants for the DAP4 client.
 *
 * Defines all structs, enums, typedefs, and compile-time constants used
 * throughout the libdap4 implementation, including the central NCD4node
 * tree, NCD4meta, NCD4response, NCD4INFO, and supporting types.
 * @author Dennis Heimbigner
 */

#ifndef D4TYPES_H
#define D4TYPES_H 1

#undef COMPILEBYDEFAULT

#include "ncdap.h"
#include "ncrc.h"
#include "ncauth.h"

/*
Control if struct fields can be map targets.
Currently turned off because semantics are unclear.
*/
#undef ALLOWFIELDMAPS

#define long64 long long  /**< Alias for @c long @c long. */
#define ncerror int        /**< Alias for @c int used as an error code. */

/* Misc. code controls */
#define FILLCONSTRAINT TRUE          /**< Always apply fill-value constraint. */
#define DEFAULTSTRINGLENGTH 64       /**< Default maximum string length in bytes. */
/** Default checksum state (1 = enabled). */
#define DEFAULT_CHECKSUM_STATE 1
/** Size in bytes of a DAP4 CRC32 checksum. */
#define CHECKSUMSIZE 4

/**************************************************/
/* sigh, do the forwards */

typedef struct NCD4INFO NCD4INFO;
typedef enum NCD4CSUM NCD4CSUM;
typedef enum NCD4mode NCD4mode;
typedef enum NCD4format NCD4format;
typedef enum NCD4translation NCD4translation;
typedef struct NCD4curl NCD4curl;
typedef struct NCD4meta NCD4meta;
typedef struct NCD4node NCD4node;
typedef struct NCD4params NCD4params;
typedef struct NCD4HDR NCD4HDR;
typedef struct NCD4offset NCD4offset;
typedef struct NCD4vardata NCD4vardata;
typedef struct NCD4response NCD4response;

/** @defgroup ncd4_chunk_flags DAP4 Chunk Header Flags
 * Bit flags stored in the 4-byte DAP4 chunk header.
 * @{
 */
#define NCD4_LAST_CHUNK          (1) /**< This is the final chunk in the message. */
#define NCD4_ERR_CHUNK           (2) /**< This chunk carries an error payload. */
#define NCD4_LITTLE_ENDIAN_CHUNK (4) /**< Data in this chunk is little-endian. */
/** Mask of all defined chunk-header flag bits. */
#define NCD4_ALL_CHUNK_FLAGS (NCD4_LAST_CHUNK|NCD4_ERR_CHUNK|NCD4_LITTLE_ENDIAN_CHUNK)
/** @} */


/**************************************************/
/**
 * Discriminator tag for a DMR tree node.
 *
 * Values are powers of 2 so they can be combined into a bitmask
 * and tested with the ISA() macro.
 */
typedef enum NCD4sort {
    NCD4_NULL=0,     /**< Uninitialized / not-a-node. */
    NCD4_ATTR=1,     /**< DAP4 Attribute node. */
    NCD4_ATTRSET=2,  /**< Attribute container (AttributeSet). */
    NCD4_XML=4,      /**< OtherXML node. */
    NCD4_DIM=8,      /**< Dimension definition node. */
    NCD4_GROUP=16,   /**< Group node (also used for the root Dataset). */
    NCD4_TYPE=32,    /**< Type node: atomic, opaque, enum, struct, or seq. */
    NCD4_VAR=64,     /**< Variable or compound-type field node. */
    NCD4_ECONST=128, /**< Enumeration constant node. */
} NCD4sort;

/** True if @p sort has any of the bits in @p flags set. */
#define ISA(sort,flags) ((sort) & (flags))

#define ISATTR(sort)   ISA((sort),(NCD4_ATTR))   /**< True if @p sort is NCD4_ATTR. */
#define ISDIM(sort)    ISA((sort),(NCD4_DIM))    /**< True if @p sort is NCD4_DIM. */
#define ISGROUP(sort)  ISA((sort),(NCD4_GROUP))  /**< True if @p sort is NCD4_GROUP. */
#define ISTYPE(sort)   ISA((sort),(NCD4_TYPE))   /**< True if @p sort is NCD4_TYPE. */
#define ISVAR(sort)    ISA((sort),(NCD4_VAR))    /**< True if @p sort is NCD4_VAR. */
#define ISECONST(sort) ISA((sort),(NCD4_ECONST)) /**< True if @p sort is NCD4_ECONST. */

#define NC_NULL   NC_NAT      /**< Alias: no type / null type. */
#define NC_SEQ    NC_VLEN     /**< DAP4 Sequence maps to NC_VLEN. */
#define NC_STRUCT NC_COMPOUND /**< DAP4 Structure maps to NC_COMPOUND. */
/** True if @p subsort is a compound type (Sequence or Structure). */
#define ISCMPD(subsort) ((subsort) == NC_SEQ || (subsort) == NC_STRUCT)

/**************************************************/
/** @defgroup ncd4_special_attrs DAP4 Special Attribute Names
 * Attribute names used for DAP4-to-NetCDF4 translation metadata.
 * Most are suppressed when writing netcdf-4 output, except NC4TAGMAPS.
 * @{
 */
#define RESERVECHAR '_'  /**< Reserved first character for special attribute names. */
#define UCARTAG         "_edu.ucar."            /**< Common prefix for UCAR translation tags. */
#define UCARTAGVLEN     "_edu.ucar.isvlen"      /**< Marks a variable as a VLEN type. */
#define UCARTAGOPAQUE   "_edu.ucar.opaque.size" /**< Records the fixed opaque size. */
#define UCARTAGORIGTYPE "_edu.ucar.orig.type"   /**< Records the original DAP4 type name. */
#define UCARTAGUNLIM    "_edu.ucar.isunlimited" /**< Marks an unlimited dimension. */
/** Map attribute written into the netcdf-4 output file. */
#define NC4TAGMAPS      "_edu.ucar.maps"
/** DAP4 constraint expression query key. */
#define DAP4CE		"dap4.ce"
/** DAP4 checksum query key. */
#define DAP4CSUM	"dap4.checksum"
/** @} */

/**************************************************/
/* Misc.*/

/** Controls how DAP4 data is translated to NetCDF-4. */
enum NCD4translation {
    NCD4_NOTRANS  = 0, /**< Straight DAP4-to-NetCDF4 translation. */
    NCD4_TRANSNC4 = 1, /**< Use @c _edu.ucar flags for improved translation. */
};

#define NCF_DEBUG_NONE  0 /**< No debug action. */
#define NCF_DEBUG_COPY  1 /**< Write data into substrate and close instead of aborting. */

/** Selects which part of a DAP4 response to retrieve. */
enum NCD4mode {
    NCD4_DMR = 1, /**< Retrieve the DMR (metadata) only. */
    NCD4_DAP = 2, /**< Retrieve the full DAP response (DMR + data). */
    NCD4_DSR = 4  /**< Retrieve the DSR (dataset services resource). */
};

/** Wire format of a DAP4 response. */
enum NCD4format {
    NCD4_FORMAT_NONE = 0, /**< No specific format. */
    NCD4_FORMAT_XML  = 1  /**< XML-encoded response. */
};

/**
 * Union providing storage for any DAP4 primitive value.
 *
 * All members share the same 8-byte block, allowing a single value to
 * be interpreted as any atomic NetCDF/DAP4 type without a cast.
 */
union ATOMICS {
    char i8[8];                /**< NC_BYTE (signed 8-bit) values. */
    unsigned char u8[8];       /**< NC_UBYTE (unsigned 8-bit) values. */
    short i16[4];              /**< NC_SHORT (signed 16-bit) values. */
    unsigned short u16[4];     /**< NC_USHORT (unsigned 16-bit) values. */
    int i32[2];                /**< NC_INT (signed 32-bit) values. */
    unsigned int u32[2];       /**< NC_UINT (unsigned 32-bit) values. */
    long long i64[1];          /**< NC_INT64 (signed 64-bit) value. */
    unsigned long long u64[1]; /**< NC_UINT64 (unsigned 64-bit) value. */
    float f32[2];              /**< NC_FLOAT (32-bit IEEE) values. */
    double f64[1];             /**< NC_DOUBLE (64-bit IEEE) value. */
#if SIZEOF_VOIDP == 4
    char* s[2];                /**< NC_STRING pointer(s) on 32-bit platforms. */
#elif SIZEOF_VOIDP == 8
    char* s[1];                /**< NC_STRING pointer on 64-bit platforms. */
#endif
#if (SIZEOF_VOIDP + SIZEOF_SIZE_T) == 8
    nc_vlen_t vl[1];           /**< NC_VLEN (variable-length) value. */
#endif
};

/**************************************************/
/**
 * DAP4 chunk header (4 bytes on the wire).
 *
 * Each chunk in a DAP4 serialization is preceded by this header.
 * @c flags carries NCD4_LAST_CHUNK, NCD4_ERR_CHUNK, and/or
 * NCD4_LITTLE_ENDIAN_CHUNK.  @c count is the byte length of the
 * chunk payload that follows.
 */
struct NCD4HDR {
    unsigned int flags; /**< Chunk-type flags (see ncd4_chunk_flags). */
    unsigned int count; /**< Byte length of the chunk payload. */
};

/**************************************************/
/**
 * Bounded cursor for walking a serialized DAP4 byte stream.
 *
 * Invariant: @c base <= @c offset <= @c limit.
 * Use the INCR/DECR macros in ncd4.h for all pointer arithmetic.
 */
struct NCD4offset {
    char* offset; /**< Current read position (char* enables pointer arithmetic). */
    char* base;   /**< Start of the buffer. */
    char* limit;  /**< One past the last valid byte. */
};

/** Saved cursor position within an NCD4offset stream. */
typedef char* NCD4mark;

/**************************************************/
/**
 * Universal node type for the DMR metadata tree.
 *
 * A single node type is used for every kind of DMR object (groups,
 * variables, dimensions, types, attributes, enum constants).  The
 * @c sort field is the gross discriminator; @c subsort refines it for
 * type nodes.  Sort-specific data lives in the anonymous sub-structs
 * (@c attr, @c opaque, @c dim, @c en, @c group, @c meta, @c data, @c nc4).
 */
struct NCD4node {
    NCD4sort sort;       /**< Gross discriminator (NCD4_VAR, NCD4_DIM, etc.). */
    nc_type subsort;     /**< Sub-type discriminator (NC_INT, NC_STRUCT, etc.). */
    char* name;          /**< Raw, unescaped node name. */
    NCD4node* container; /**< Parent node (group, enum type, compound type, etc.). */
    int visited;         /**< Scratch flag used during recursive tree walks. */
    NClist* groups;        /**< NClist<NCD4node*>: child groups (NCD4_GROUP nodes). */
    NClist* vars;          /**< NClist<NCD4node*>: variables in a group, or fields in a struct/seq. */
    NClist* types;         /**< NClist<NCD4node*>: type definitions in a group. */
    NClist* dims;          /**< NClist<NCD4node*>: dim defs in a group, or dim refs in a var. */
    NClist* attributes;    /**< NClist<NCD4node*>: attribute child nodes. */
    NClist* mapnames;      /**< NClist<char*>: raw map target names from the DMR. */
    NClist* maps;          /**< NClist<NCD4node*>: resolved map target variable nodes. */
    NClist* xmlattributes; /**< NClist<char*>: OtherXML attribute strings. */
    NCD4node* basetype;    /**< Base type node for variables and enum constants. */
    struct { /**< Valid when sort == NCD4_ATTR. */
        NClist* values; /**< NClist<char*>: string-encoded attribute values. */
    } attr;
    struct { /**< Valid when subsort == NC_OPAQUE. */
        size_t size; /**< Fixed opaque size in bytes; 0 means variable-length. */
    } opaque;
    struct { /**< Valid when sort == NCD4_DIM. */
        size_t size;     /**< Declared dimension size. */
        int isunlimited; /**< Non-zero if this is an unlimited dimension. */
        int isanonymous; /**< Non-zero if this is an anonymous (inline) dimension. */
    } dim;
    struct { /**< Valid when sort == NCD4_ECONST or subsort == NC_ENUM. */
        union ATOMICS ecvalue; /**< Numeric value of an enumeration constant. */
        NClist* econsts;       /**< NClist<NCD4node*>: constants belonging to an enum type. */
    } en;
    struct { /**< Valid when sort == NCD4_GROUP. */
        NClist* elements;  /**< NClist<NCD4node*>: all top-level objects in the group. */
        int isdataset;     /**< Non-zero if this group is the root Dataset element. */
        char* dapversion;  /**< DAP protocol version string from the DMR. */
        char* dmrversion;  /**< DMR schema version string from the DMR. */
        char* datasetname; /**< Dataset name from the DMR root element. */
        NClist* varbyid;   /**< NClist<NCD4node*>: variables indexed by varid. */
    } group;
    struct { /**< NetCDF-4 metadata computed during NCD4_metabuild(). */
        int id;           /**< NetCDF-4 id; meaning depends on sort (ncid, dimid, varid, typeid). */
        int isfixedsize;  /**< Non-zero if this type has a fixed in-memory size (recursively). */
        d4size_t dapsize; /**< Size of the type in the DAP4 serialization (bytes);
                           *   equals memsize only for types <= NC_UINT64. */
        nc_type cmpdid;   /**< NetCDF-4 compound type id created for a Sequence type. */
        size_t memsize;   /**< In-memory size of one instance, accounting for alignment
                           *   but not dimproduct. */
        d4size_t offset;  /**< Byte offset of this field within its parent compound type. */
        size_t alignment; /**< Required memory alignment of this field. */
    } meta;
    struct NCD4vardata { /**< Data-pipeline state, populated during data processing. */
        int valid;               /**< Non-zero when this struct contains valid data. */
        D4blob dap4data;         /**< Location and size of this variable's data in the serialization. */
        unsigned remotechecksum; /**< CRC32 checksum embedded in the server's data stream. */
        unsigned localchecksum;  /**< CRC32 checksum computed by the client for verification. */
        int checksumattr;        /**< Non-zero if _DAP4_Checksum_CRC32 attribute is defined. */
        unsigned attrchecksum;   /**< Checksum value from the _DAP4_Checksum_CRC32 attribute. */
        NCD4response* response;  /**< Response object from which this variable's data was taken. */
    } data;
    struct { /**< NetCDF-4 translation metadata (from _edu.ucar.* attributes). */
        int isvlen; /**< Non-zero if _edu.ucar.isvlen is set. */
        struct {
            NCD4node* group; /**< Group part of the original type's FQN. */
            char* name;      /**< Name part of the original type's FQN. */
        } orig; /**< Original DAP4 type, split from UCARTAGORIGTYPE. */
    } nc4;
};

/**
 * Parsed DMR metadata for one server response.
 *
 * Created by NCD4_parse() and consumed by NCD4_metabuild().  Owns the
 * NCD4node tree rooted at @c root and the flat @c allnodes list.
 */
struct NCD4meta {
    NCD4INFO* controller;  /**< Owning NCD4INFO connection object. */
    int ncid;              /**< Root ncid of the substrate NetCDF-4 file
                            *   (copy of NCD4INFO::substrate::nc4id). */
    NCD4node* root;        /**< Root Dataset node of the DMR tree. */
    NClist* allnodes;      /**< NClist<NCD4node*>: flat list of every node in the tree. */
    int swap;              /**< Non-zero if byte-swapping is required for this response. */
    NClist* groupbyid;     /**< NClist<NCD4node*>: groups indexed by (groupid >> 16). */
    NCD4node* _bytestring; /**< Synthetic bytestring type node, created on demand. */
    NClist* atomictypes;   /**< NClist<NCD4node*>: all known atomic type nodes. */
};

/**
 * Internal state for the DMR XML parser.
 *
 * Allocated on entry to NCD4_parse() and freed on return.  Accumulates
 * node lists that are later transferred into the NCD4meta structure.
 */
typedef struct NCD4parser {
    NCD4INFO* controller;   /**< Owning NCD4INFO connection object. */
    char* input;            /**< Raw DMR XML text being parsed. */
    int debuglevel;         /**< Verbosity level for parser debug output. */
    int dapparse;           /**< Non-zero when parsing the DMR from a full DAP response. */
    NCD4meta* metadata;     /**< Metadata object being populated. */
    NCD4response* response; /**< Response object associated with this parse. */
    NClist* types;          /**< NClist<NCD4node*>: user-defined types encountered so far. */
    NClist* dims;           /**< NClist<NCD4node*>: dimension nodes encountered so far. */
    NClist* vars;           /**< NClist<NCD4node*>: variable nodes encountered so far. */
    NClist* groups;         /**< NClist<NCD4node*>: group nodes encountered so far. */
    NCD4node* dapopaque;    /**< Single variable-length opaque type node. */
} NCD4parser;

/**
 * Holds a complete (possibly processed) response from a DAP4 server.
 *
 * Created by NCD4_newResponse() and freed by NCD4_reclaimResponse().
 * The @c raw blob holds the unmodified bytes from the wire; after
 * NCD4_dechunk() the @c serial sub-struct points into that buffer.
 */
struct NCD4response {
    NCD4INFO* controller;      /**< Owning NCD4INFO connection object. */
    D4blob raw;                /**< Complete raw response bytes as received from the server. */
    int querychecksumming;     /**< Non-zero if the user specified a dap4.ce checksum option. */
    int attrchecksumming;      /**< Non-zero if _DAP4_Checksum_CRC32 is defined for any variable. */
    int inferredchecksumming;  /**< Non-zero if either querychecksumming or attrchecksumming is set. */
    int checksumignore;        /**< Non-zero to assume checksums are present but skip validation. */
    int remotelittleendian;    /**< Non-zero if the server reports data as little-endian. */
    NCD4mode mode;             /**< Retrieval mode: DMR-only or full DAP (DMR + data). */
    struct NCD4serial {
        size_t dapsize; /**< Byte length of the dechunked DAP data (transient). */
        void* dap;      /**< Pointer into @c raw where dechunked DAP data begins. */
        char* dmr;      /**< Null-terminated copy of the DMR XML text. */
        char* errdata;  /**< Null-terminated error chunk text, or NULL if no error. */
        int httpcode;   /**< HTTP status code returned by the last request. */
    } serial; /**< Dechunked and processed DAP portion of the response. */
    struct Error {
        char* parseerror; /**< XML parse error message, or NULL. */
        int   httpcode;   /**< HTTP status code from the error response. */
        char* message;    /**< Human-readable error message from the server. */
        char* context;    /**< Error context string from the server. */
        char* otherinfo;  /**< Additional error information from the server. */
    } error; /**< Content of any DAP4 error response received from the server. */
};

/**************************************************/
/**
 * libcurl state for one DAP4 connection.
 *
 * Wraps the CURL easy handle and associated buffers and options.
 */
struct NCD4curl {
    CURL* curl;      /**< libcurl easy handle. */
    NCbytes* packet; /**< Receive buffer for HTTP response body. */
    struct errdata {
        char* code;                    /**< Server-supplied error code string. */
        char* message;                 /**< Server-supplied error message string. */
        long  httpcode;                /**< HTTP status code of the error response. */
        char  errorbuf[CURL_ERROR_SIZE]; /**< libcurl CURLOPT_ERRORBUFFER scratch space. */
    } errdata; /**< Error information returned by the server. */
    struct {
        int  active;   /**< Non-zero to enable TCP keepalive. */
        long idle;     /**< CURLOPT_TCP_KEEPIDLE value (seconds). */
        long interval; /**< CURLOPT_TCP_KEEPINTVL value (seconds). */
    } keepalive; /**< TCP keepalive settings. */
    long buffersize; /**< libcurl receive buffer size (CURLOPT_BUFFERSIZE). */
};

/**************************************************/
/**
 * Top-level connection and session state for one DAP4 open file.
 *
 * One NCD4INFO is allocated per nc_open() call on a DAP4 URL and is
 * stored as NC::dispatchdata.  It owns the curl handle, the substrate
 * NetCDF-4 file, and all response objects.
 */
struct NCD4INFO {
    NC* controller;       /**< Parent NC object that owns this NCD4INFO. */
    char* rawdmrurltext;  /**< URL string exactly as passed to ncd4_open(). */
    char* dmrurltext;     /**< URL string after modification by ncd4_open(). */
    NCURI* dmruri;        /**< Parsed form of rawdmrurltext. */
    NCD4curl* curl;       /**< libcurl state for this connection. */
    int inmemory;         /**< Non-zero to store fetched data entirely in memory. */
    NCD4meta* dmrmetadata;/**< DMR metadata, independent of individual responses. */
    NClist* responses;    /**< NClist<NCD4response*>: all responses from this handle. */
    struct {
        int hostlittleendian; /**< Non-zero if the host machine is little-endian. */
    } platform; /**< Per-platform properties detected at open time. */
    struct {
        int realfile;    /**< Non-zero if a real temporary file was created. */
        char* filename;  /**< Path of the substrate temporary file. */
        int nc4id;       /**< ncid of the substrate NetCDF-4 file (not the external id). */
    } substrate; /**< Substrate NetCDF-4 file used to hold translated metadata. */
    struct {
        NCCONTROLS flags;                /**< Feature-control flags. */
        NCCONTROLS debugflags;           /**< Debug-control flags. */
        NCD4translation translation;     /**< Active translation mode. */
        char substratename[NC_MAX_NAME]; /**< Name used for the substrate file. */
        size_t opaquesize;               /**< Default fixed opaque size in bytes. */
    } controls; /**< Configurable controls parsed from URL fragment parameters. */
    NCauth* auth;         /**< Authentication credentials for this connection. */
    struct {
        char* filename; /**< Path used when the URL scheme is "file:". */
    } fileproto; /**< State for file-protocol (non-HTTP) access. */
    int debuglevel;   /**< Verbosity level for debug output. */
    NClist* blobs;    /**< NClist<void*>: memory blobs to free when the file is closed. */
};

#endif /*D4TYPES_H*/
