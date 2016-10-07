/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/*
Type declarations and associated constants
are defined here.	
*/

#ifndef D4TYPES_H
#define D4TYPES_H 1

#undef COMPILEBYDEFAULT

#define long64 long long
#define ncerror int

/* Misc. code controls */
#define FILLCONSTRAINT TRUE

#define DEFAULTSTRINGLENGTH 64

/* Max rc file line size */
#define MAXRCLINESIZE 8192

/* Max number of triples from an rc file */
#define MAXRCLINES 8192

/* Size of the checksum */
#define CHECKSUMSIZE 4

/**************************************************/
/*
Collect single bit flags that
affect the operation of the system.
*/

typedef unsigned int NCFLAGS;
#  define SETFLAG(controls,flag) ((controls) |= (flag))
#  define CLRFLAG(controls,flag) ((controls) &= ~(flag))
#  define FLAGSET(controls,flag) (((controls) & (flag)) != 0)

/* Defined flags */
#define NCF_NC3             (0x0001) /* DAP->netcdf-3 */
#define NCF_NC4             (0x0002) /* DAP->netcdf-4 */
#define NCF_CACHE           (0x0008) /* Cache enabled/disabled */
#define NCF_UNCONSTRAINABLE (0x0020) /* Not a constrainable URL */
#define NCF_SHOWFETCH       (0x0040) /* show fetch calls */
#define NCF_ONDISK          (0x0080) /* store data on disk */
#define NCF_WHOLEVAR        (0x0100) /* retrieve only whole variables (as opposed to partial variable) into cache */
#define NCF_PREFETCH        (0x0200) /* Cache prefetch enabled/disabled */
#define NCF_PREFETCH_EAGER  (0x0400) /* Do eager prefetch; 0=>lazy */
#define NCF_PREFETCH_ALL    (0x0800) /* Prefetch all variables */

/* Define all the default on flags */
#define DFALT_ON_FLAGS (NCF_CACHE|NCF_PREFETCH)

/**************************************************/
/* sigh, do the forwards */

typedef struct NCD4INFO NCD4INFO;
typedef enum NCD4mode NCD4mode;
typedef struct NCD4curl NCD4curl;
typedef struct NCD4meta NCD4meta;
typedef struct NCD4globalstate NCD4globalstate;
typedef struct NCD4node NCD4node;
typedef struct NCD4params NCD4params;
typedef struct NCD4triplestore NCD4triplestore;

/**************************************************/
/* DMR Tree node sorts */

/* Concepts from netcdf-4 data model */
typedef enum NCD4sort {
    NCD4_NULL=0,
    NCD4_ATTR=1,
    NCD4_ATTRSET=2,
    NCD4_XML=3, /* OtherXML */
    NCD4_DIM=4,
    NCD4_GROUP=5, /* Including Dataset */
    NCD4_TYPE=6, /* atom types, opaque, enum, struct or seq */
    NCD4_VAR=7, /* Variable or field */
    NCD4_ECONST=8,
#define NSORTS (NCD4_ECONST+1)
} NCD4sort;

#define ISA(sort,flags) ((1<<(sort) & (flags)))

#define ISATTR(sort) ISA((sort),(1<<NCD4_ATTR))
#define ISDIM(sort) ISA((sort),(1<<NCD4_DIM))
#define ISGROUP(sort) ISA((sort),(1<<NCD4_GROUP))
#define ISTYPE(sort) ISA((sort),(1<<NCD4_TYPE))
#define ISVAR(sort) ISA((sort),(1<<NCD4_VAR))
#define ISECONST(sort) ISA((sort),(1<<NCD4_ECONST))

/* Define some nc_type aliases */
#define NC_NULL NC_NAT
#define NC_SEQ NC_VLEN
#define NC_STRUCT NC_COMPOUND

#define ISCMPD(subsort) ((subsort) == NC_SEQ || (subsort) == NC_STRUCT)

/**************************************************/
/* Special attributes; When defining netcdf-4,
   these are suppressed, except for UCARTAGMAPS
*/

#define UCARTAG        "^edu.ucar."
#define UCARTAGNC4     "_edu.ucar."
#define UCARTAGVLEN     "^edu.ucar.isvlen"
#define UCARTAGOPAQUE   "^edu.ucar.opaque.size"
#define UCARTAGUNLIM    "^edu.ucar.isunlim"
#define UCARTAGORIGTYPE "^edu.ucar.orig.type"
#define UCARTAGMAPS     "^edu.ucar.maps"
#define NC4TAGMAPS      "_edu.ucar.maps"

/**************************************************/
/* Misc.*/

/* Define possible checksum modes */


enum NCD4mode {
    NCD4_DMR, /*=> compute checksums for DMR requests only*/
    NCD4_DAP, /*=> compute checksums for DAP requests only*/
    NCD4_ALL  /*=> compute checksums for both kinds of requests */
};

/* Define storage for all the primitive types (plus vlen) */
union ATOMICS {
    char i8[8];
    unsigned char u8[8];
    short i16[4];
    unsigned short u16[4];
    int i32[2];
    unsigned int u32[2];
    long long i64[1];
    unsigned long long u64[1];
    float f32[2];
    double f64[1];
#if SIZEOF_VOIDP == 4
    char* s[2];
#elif SIZEOF_VOIDP == 8
    char* s[1];
#endif
#if (SIZEOF_VOIDP + SIZEOF_SIZE_T) == 8
    nc_vlen_t vl[1];
#endif
};

/**************************************************/
/* !Node type for the NetCDF-4 metadata produced from
   parsing the DMR tree.
   We only use a single node type tagged with the sort.
   Some information is not kept (e.g. attributes).
*/
struct NCD4node {
    NCD4sort sort; /* gross discriminator */
    nc_type subsort; /* subcases of sort */
    char* name; /* Raw unescaped */
    NCD4node*  container; /* parent object: e.g. group, enum, compound...*/
    int visited; /* for recursive walks of all nodes */
    /* Sort specific fields */
    NClist* groups;   /* NClist<NCD4node*> groups in group */
    NClist* vars;   /* NClist<NCD4node*> vars in group, fields in struct/seq */
    NClist* types;   /* NClist<NCD4node*> types in group */
    NClist* dims;    /* NClist<NCD4node*>; dimdefs in group, dimrefs in vars */
    NClist* attributes; /* NClist<NCD4node*> */
    NClist* maps;       /* ^edu.ucar.maps ; NClist<NCD4node*> */
    NCD4node* basetype;
    struct { /* sort == NCD4_ATTRIBUTE */
        NClist* values;
    } attr;
    struct { /* sort == NCD4_OPAQUE */
	long long size; /* 0 => var length */
    } opaque;
    struct { /* sort == NCD4_DIMENSION */
	long long size;
    } dim;
    struct { /* sort == NCD4_ECONST || sort == NCD4_ENUM */    
        union ATOMICS ecvalue;
        NClist* econsts; /* NClist<NCD4node*> */
    } en;
    struct { /* sort == NCD4_GROUP */
	NClist* elements;   /* NClist<NCD4node*> defines the fqn path */
        int isdataset;
        char* dapversion;
        char* dmrversion;
        char* datasetname;
    } group;
    struct { /* Meta Info */
        int id; /* Relevant netcdf id; interpretation depends on sort */
        nc_type cmpdid; /*netcdf id for the compound type created for seq type */
        unsigned long long offset; /* computed structure field offsets */
    } meta;
    struct { /* Data compilation info */
        int flags; /* See d4data for actual flags */
	D4blob vardata; /* cover the memory of this variable/field */
        unsigned char checksum[CHECKSUMSIZE]; /* toplevel variable checksum */    
    } data;
    struct { /* Track netcdf-4 conversion info */
	int isvlen;	/*  ^edu.ucar.isvlen */
	int isunlim;	/* ^edu.ucar.isunlim */
	/* Split UCARTAGORIGTYPE into group plus name */
	struct {
  	    NCD4node* group;
	    char* name;
	} orig;
	/* Represented elsewhare: ^edu.ucar.opaque.size */ 
    } nc4;
};

/* Tracking info about the serialized input before and after de-chunking */
typedef struct NCD4serial {
    size_t rawsize; /* |rawdata| */ 
    void* rawdata;
    size_t dapsize; /* |dapdata|; this is transient */
    void* dap; /* pointer into rawdata where dap data starts */ 
    size_t dmrsize; /* |dmrdata| */
    char* dmr;/* pointer into rawdata where dmr starts */ 
    char* errdata; /* null || error chunk (null terminated) */
    int hostbigendian;
    int remotebigendian; /* 1 if the packet says data is bigendian */
} NCD4serial;

/* This will be passed out of the parse */
struct NCD4meta {
    int ncid; /* root ncid of the netcdf-4 file; copy of NCD4parse argument*/
    NCD4node* root;
    NClist* allnodes; /*list<NCD4node>*/
    struct Error { /* Content of any error response */
	char* parseerror;
	int   httpcode;
	char* message;
	char* context;
	char* otherinfo;
    } error;
    int debuglevel;
    NCD4mode checksummode;
    NCD4serial serial;
    NClist* blobs;  /* various malloc'd chunks will be remembered here */
};

typedef struct NCD4parser {
    char* input;
    int debuglevel;
    NCD4meta* metadata;
    NCD4INFO* parent;
    /* Capture useful subsets of dataset->allnodes */
    NClist* types; /*list<NCD4node>*/
    NClist* dims; /*list<NCD4node>*/
    NClist* vars; /*list<NCD4node>*/
    NClist* groups; /*list<NCD4node>*/
    /* Convenience for short cut fqn detection */
    NClist* atomictypes; /*list<NCD4node>*/
    NCD4node* dapopaque; /* Single non-fixed-size opaque type */
} NCD4parser;

/**************************************************/

struct NCD4triplestore {
    int ntriples;
    struct NCD4triple {
        char host[MAXRCLINESIZE]; /* includes port if specified */
        char key[MAXRCLINESIZE];
        char value[MAXRCLINESIZE];
   } triples[MAXRCLINES];
};

/**************************************************/

/* Collect global state info in one place */
struct NCD4globalstate {
    int initialized;
    struct {
        int proto_file;
        int proto_https;
    } curl;
    char* tempdir; /* track a usable temp dir */
    char* home; /* track $HOME for use in creating $HOME/.oc dir */
    struct {
	int ignore; /* if 1, then do not use any rc file */
	int loaded;
        NCD4triplestore daprc; /* the rc file triple store fields*/
        char* rcfile; /* specified rcfile; overrides anything else */
    } rc;
};

/* Curl info */
struct NCD4curl {
    CURL* curl; /* curl handle*/
    NCbytes* packet; 
    struct errdata {/* Hold info for an error return from server */
	char* code;
	char* message;
	long  httpcode;
	char  errorbuf[CURL_ERROR_SIZE]; /* CURLOPT_ERRORBUFFER*/
    } errdata;
    struct curlflags {
        int proto_file; /* Is file: supported? */
        int proto_https; /* is https: supported? */
	int compress; /*CURLOPT_ENCODING*/
	int verbose; /*CURLOPT_ENCODING*/
	int timeout; /*CURLOPT_TIMEOUT*/
	int maxredirs; /*CURLOPT_MAXREDIRS*/
	char* useragent; /*CURLOPT_USERAGENT*/
	/* track which of these are created by oc */
#define COOKIECREATED 1
#define NETRCCREATED 2
	int createdflags;
	char* cookiejar; /*CURLOPT_COOKIEJAR,CURLOPT_COOKIEFILE*/
	char* netrc; /*CURLOPT_NETRC,CURLOPT_NETRC_FILE*/
    } curlflags;
    struct ssl {
	int   verifypeer; /* CURLOPT_SSL_VERIFYPEER;
                             do not do this when cert might be self-signed
                             or temporarily incorrect */
	int   verifyhost; /* CURLOPT_SSL_VERIFYHOST; for client-side verification */
        char* certificate; /*CURLOPT_SSLCERT*/
	char* key; /*CURLOPT_SSLKEY*/
	char* keypasswd; /*CURLOPT_SSLKEYPASSWD*/
        char* cainfo; /* CURLOPT_CAINFO; certificate authority */
	char* capath;  /*CURLOPT_CAPATH*/
    } ssl;
    struct proxy {
	char *host; /*CURLOPT_PROXY*/
	int port; /*CURLOPT_PROXYPORT*/
	char* userpwd; /*CURLOPT_PROXYUSERPWD*/
    } proxy;
    struct credentials {
	char *userpwd; /*CURLOPT_USERPWD*/
    } creds;
    void* usercurldata;
};

/**************************************************/
struct NCD4params {
#if 0
    NClist*  projectedvars; /* vars appearing in nc_open url projections */
    unsigned int defaultstringlength;
    unsigned int defaultsequencelimit; /* global sequence limit;0=>no limit */
    size_t fetchlimit;
    size_t smallsizelimit; /* what constitutes a small object? */
    size_t totalestimatedsize;
    const char* separator; /* constant; do not free */
    /* global string dimension */
    struct CDFnode* globalstringdim;
    NClist*  usertypes; /* nodes which will represent netcdf types */
#endif
};

/* Define a structure holding common info */

struct NCD4INFO {
    NC*   controller; /* Parent instance of NCD4INFO */
    NCFLAGS  flags;
    int nc4id; /* nc4 file ncid used to hold metadata */
    int debug;
    char* rawurltext; /* as given to ncd4_open */
    char* urltext;    /* as modified by ncd4_open */
    NCURI* uri;      /* parse of rawuritext */
    /*DC4Constraint* dapconstraint;  from url */
    NCD4curl* curl;
    int inmemory; /* store fetched data in memory? */
    struct {
	char*   memory;   /* allocated memory if ONDISK is not set */
        char*   filename; /* If ONDISK is set */
        FILE*   file;     /* ditto */
        off_t   datasize; /* size on disk or in memory */
        long dmrlastmodified;
        long daplastmodified;
    } data;
};

#endif /*D4TYPES_H*/
