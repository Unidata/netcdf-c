/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/**
 * @file This header file contains types (and type-related macros)
 * for the libzarr code.
 *
 *
 * @author Dennis Heimbigner
*/

/*
This API essentially implements a simplified variant
of the Amazon S3 API. Specifically, we have the following
kinds of things.

1. Dataset - equivalent of the S3 bucket; root of all the data
and metadata for a dataset. Each dataset is associated with an arbitrary
number of "objects", where each object has a unique key (i.e. name).
2. Object - equivalent of the S3 object; Each object has a unique key
and "contains" data in the form of an arbitrary sequence of 8-bit bytes.

As with Amazon S3, keys are utf8 strings with a specific structure:
that of a path similar to those of a Unix path with '/' as the
separator for the segments of the path.

As with Unix, all paths have this BNF syntax:
<pre>
path: '/' | path segment ;
segment: <sequence of UTF-8 characters except control characters and '/'>
</pre>

Obviously, this path structure imposes a tree structure on the set
of objects where one object is "contained" (possibly transitively)
by another if one path is a prefix (in the string sense) of the other.

Issues:
1. S3 limits key lengths to 1024 bytes. Some deeply nested netcdf files
will almost certainly exceed this limit.

*/

#ifndef ZMAP_H
#define ZMAP_H

#define NCZM_SEP "/"

/* Mnemonic */
#define NCZ_ISMETA 0

/* Forward */
struct NCZMAP_API;

/* Define the space of implemented (eventually) map implementations */
typedef enum NCZM_IMPL {
NCZM_UNDEF=0, /* In-memory implementation */
NCZM_S3=1,    /* Amazon S3 implementation */
NCZM_FILE=2,  /* File system directory-based implementation */
NCZM_NC4=3,   /* Netcdf-4 file based implementation */
} NCZM_IMPL;

/* Define the default map implementation */
#define NCZM_DEFAULT NCZM_NC4

/*
For each dataset, we create what amounts to a class
defining data and the API function implementations.
All datasets are subclasses of NCZMAP.
In the usual C approach, subclassing is performed by
casting.

So all Dataset structs have this as their first field
so we can cast to this form; avoids need for
a separate per-implementation malloc piece.

*/
typedef struct NCZMAP {
    NCZM_IMPL format;
    char* url;
    int mode;
    size64_t flags;
    struct NCZMAP_API* api;
} NCZMAP;

/* Forward */
typedef struct NCZMAP_API NCZMAP_API;

/* Define the object-level API */

struct NCZMAP_API {
    int version;

    /* Object Operations */
	int (*exists)(NCZMAP* map, const char* key);
	int (*len)(NCZMAP* map, const char* key, ssize64_t* lenp);
	/* Define an object; do nothing if already exists */
	int (*def)(NCZMAP* map, const char* key, ssize64_t len);
	/* Read/write data */
	int (*read)(NCZMAP* map, const char* key, ssize64_t start, ssize64_t count, void* content);
	int (*write)(NCZMAP* map, const char* key, ssize64_t start, ssize64_t count, const void* content);
	/* Read/write metadata (e.g. Json)*/
	int (*readmeta)(NCZMAP* map, const char* key, ssize64_t count, char* content);
	int (*writemeta)(NCZMAP* map, const char* key, ssize64_t count, const char* content);
        int (*close)(NCZMAP* map, int delete);
	/* Search for keys with specified prefix */
        int (*search)(NCZMAP* map, const char* prefix, NClist* matches);
};

/* Define the Dataset level API */
typedef struct NCZMAP_DS_API {
    int version;
    int (*verify)(const char *path, int mode, size64_t flags, void* parameters);
    int (*create)(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);
    int (*open)(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);
} NCZMAP_DS_API;

/* Object API Wrappers */
extern int nczmap_exists(NCZMAP* map, const char* key);
extern int nczmap_len(NCZMAP* map, const char* key, ssize64_t* lenp);
extern int nczmap_def(NCZMAP* map, const char* key, ssize64_t lenp);
extern int nczmap_read(NCZMAP* map, const char* key, ssize64_t start, ssize64_t count, void* content);
extern int nczmap_write(NCZMAP* map, const char* key, ssize64_t start, ssize64_t count, const void* content);
extern int nczmap_readmeta(NCZMAP* map, const char* key, ssize64_t count, char* content);
extern int nczmap_writemeta(NCZMAP* map, const char* key, ssize64_t count, const char* content);
extern int nczmap_close(NCZMAP* map, int delete);
extern int nczmap_search(NCZMAP* map, const char* prefix, NClist* matches);

/* Create/open and control a dataset using a specific implementation */
extern int nczmap_create(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);
extern int nczmap_open(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);

/* Utility functions */
extern int nczm_split(const char* path, NClist* segments);
extern int nczm_joinprefix(NClist* segments, int nsegs, char** pathp);
extern int nczm_join(NClist* segments, char** pathp);
extern int nczm_suffix(const char* prefix, const char* suffix, char** pathp);
extern int nczm_clear(NCZMAP* map);

#endif /*ZMAP_H*/
