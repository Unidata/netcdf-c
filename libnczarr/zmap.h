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

As with Amazon S3, keys are utf8 strings with a specific structure:
that of a path similar to those of a Unix path with '/' as the
separator for the segments of the path.

As with Unix, all keys have this BNF syntax:
<pre>
key: '/' | key segment ;
segment: <sequence of UTF-8 characters except control characters and '/'>
</pre>

Obviously, one can infer a tree structure from this key structure.
A containment relationship is defined by key prefixes.
Thus one key is "contained" (possibly transitively)
by another if one key is a prefix (in the string sense) of the other.
So in this sense the key "/x/y/z" is contained by the key  "/x/y".

As with S3, a key refers to an "object" that can contain content.
An important restriction is placed on the structure of the tree.
Namely, keys are only defined for content-bearing objects.
Further, all the leaves of the tree are these content-bearing objects.
This means that the key for one content-bearing object cannot
be a prefix of any other key.

There several other concepts of note.
1. Dataset - a dataset is the complete tree contained by the key defining
the root of the dataset.
2. Object - equivalent of the S3 object; Each object has a unique key
and "contains" data in the form of an arbitrary sequence of 8-bit bytes.

Notes:
1. The search function is optional. It has two purposes:
   a. Support reading of pure zarr datasets (because they do not explicitly
      track their contents).
   b. Debugging to allow raw examination of the storage. See zdump
      for example.

The zmap API defined here isolates the key-value pair mapping code
from the Zarr-based implementation of NetCDF-4.

It wraps an internal C dispatch table manager
for implementing an abstract data structure
implementing the key/object model.

Issues:
1. S3 limits key lengths to 1024 bytes. Some deeply nested netcdf files
will almost certainly exceed this limit.
2. Besides content, S3 objects can have an associated small set
of what may be called tags, which are themselves of the form of
key-value pairs, but where the key and value are always text. As
far as it is possible to determine, Zarr never uses these tags,
so they are not included in the zmap data structure.

A Note on Error Codes:

This model uses the S3 concepts of keys.  All legal keys "exist"
in that it is possible to write to them, The concept of a key
not-existing has no meaning: all keys exist.  Normally, in S3,
each key specifies an object, but unless that object has
content, it does not exist.  Therefore we distinguish
content-bearing "objects" from non-content-bearing objects.  Our
model only hold content-bearing objects. Note that the length of
that content may be zero.  The important point is that in this
model, only content-bearing objects actually exist.  Note that
this different than, say, a direvtory tree where a key will
always lead to something: a directory or a file.

In any case, the zmap API returns two distinguished error code:
1. NC_NOERR if a content bearing object is created or referenced.
2. NC_EEMPTY is returned when accessing a key that has no content.
This does not preclude other errors being returned such NC_EACCESS or NC_EPERM or NC_EINVAL
if there are permission errors or illegal function arguments, for example.
It also does not preclude the use of other error codes internal to the zmap
implementation. So zmap_nzf, for example, uses NC_ENOTFOUND internally
because it is possible to detect the existence of directories and files.
This does not propagate to the API.

Note that NC_EEMPTY is a new error code to signal to that the
caller asked for non-content-bearing key.

The current set of operations defined for zmaps are define with the
generic nczm_xxx functions below.
*/

#ifndef ZMAP_H
#define ZMAP_H

#include "ncexternl.h"

#define NCZM_SEP "/"

#define NCZM_DOT '.'

/*Mnemonic*/
#define LOCALIZE 1

/* Forward */
typedef struct NCZMAP_API NCZMAP_API;

/* Define the space of implemented (eventually) map implementations */
typedef enum NCZM_IMPL {
NCZM_UNDEF=0, /* In-memory implementation */
NCZM_S3=1,	/* Amazon S3 implementation */
NCZM_NC4=2,	/* Netcdf-4 file based implementation */
NCZM_FILE=3,	/* File system directory-based implementation */
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

/* Define the object-level API */

struct NCZMAP_API {
    int version;

    /* Map Operations */
        int (*close)(NCZMAP* map, int deleteit);
    /* Object Operations */
	int (*exists)(NCZMAP* map, const char* key);
	int (*len)(NCZMAP* map, const char* key, size64_t* sizep);
	int (*defineobj)(NCZMAP* map, const char* key);
	int (*read)(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content);
	int (*write)(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content);
        int (*search)(NCZMAP* map, const char* prefix, NClist* matches);
};

/* Define the Dataset level API */
typedef struct NCZMAP_DS_API {
    int version;
    int (*create)(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);
    int (*open)(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);
} NCZMAP_DS_API;

#ifdef __cplusplus
extern "C" {
#endif

/* Object API Wrappers; note that there are no group operations
   because group keys do not map to directories.
   */

/**
Check if a specified content-bearing object exists or not.
@param map -- the containing map
@param key -- the key specifying the content-bearing object
@return NC_NOERR if the object exists
@return NC_ENOTFOUND if the object does not exist
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_exists(NCZMAP* map, const char* key);

/**
Return the current size of a specified content-bearing object exists or not.
@param map -- the containing map
@param key -- the key specifying the content-bearing object
@param sizep -- the object's size is returned thru this pointer.
@return NC_NOERR if the object exists
@return NC_ENOTFOUND if the object does not exist
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_len(NCZMAP* map, const char* key, size64_t* sizep);

/**
Create a specified content-bearing object.
@param map -- the containing map
@param key -- the key specifying the content-bearing object
@return NC_NOERR if the object is created
@return NC_ENOTFOUND if the object does not exist
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_defineobj(NCZMAP* map, const char* key);

/**
Read the content of a specified content-bearing object.
@param map -- the containing map
@param key -- the key specifying the content-bearing object
@param start -- offset into the content to start reading
@param count -- number of bytes to read
@param content -- read the data into this memory
@return NC_NOERR if the operation succeeded
@return NC_ENOTFOUND if the object does not exist
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_read(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content);

/**
Write the content of a specified content-bearing object.
@param map -- the containing map
@param key -- the key specifying the content-bearing object
@param start -- offset into the content to start writing
@param count -- number of bytes to write
@param content -- write the data from this memory
@return NC_NOERR if the operation succeeded
@return NC_ENOTFOUND if the object does not exist
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_write(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content);

/**
Return a vector of keys representing the content-bearing
objects that are immediately contained by the prefix key.
@param map -- the containing map
@param prefix -- the key into the tree where the search is to occur
@param matches -- return the set of keys in this list
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_search(NCZMAP* map, const char* prefix, NClist* matches);

/**
Close a map
@param map -- the map to close
@param deleteit-- if true, then delete the corresponding dataset
@return NC_NOERR if the operation succeeded
@return NC_ENOTFOUND if the object does not exist
@return NC_EXXX if the operation failed for one of several possible reasons
*/
EXTERNL int nczmap_close(NCZMAP* map, int deleteit);

/* Create/open and control a dataset using a specific implementation */
EXTERNL int nczmap_create(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);
EXTERNL int nczmap_open(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp);

/* Utility functions */

/** Split a path into pieces along '/' character; elide any leading '/' */
EXTERNL int nczm_split(const char* path, NClist* segments);

/* Split a path into pieces along some character; elide any leading char */
EXTERNL int nczm_split_delim(const char* path, char delim, NClist* segments);

/* Convenience: Join all segments into a path using '/' character */
EXTERNL int nczm_join(NClist* segments, char** pathp);

/* Convenience: Join all segments into a path using '/' character
   but taking possible lead windows drive letter into account
*/
EXTERNL int nczm_joinpath(NClist* segments, char** pathp);

/* Convenience: concat two strings; caller frees */
EXTERNL int nczm_concat(const char* prefix, const char* suffix, char** pathp);

/* Break a key into prefix and suffix, where prefix is the first nsegs segments;
   nsegs can be negative to specify that suffix is |nsegs| long
*/
EXTERNL int nczm_divide_at(const char* key, int nsegs, char** prefixp, char** suffixp);

/* Reclaim the content of a map but not the map itself */
EXTERNL int nczm_clear(NCZMAP* map);

/* Return 1 if path is absolute; takes Windows drive letters into account */
EXTERNL int nczm_isabsolutepath(const char* path);

/* Convert forward to back slash if needed */
EXTERNL int nczm_localize(const char* path, char** newpathp, int local);
EXTERNL int nczm_canonicalpath(const char* path, char** cpathp);

#ifdef __cplusplus
}
#endif

#endif /*ZMAP_H*/
