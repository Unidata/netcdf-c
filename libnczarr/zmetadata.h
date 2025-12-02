/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/*
Zarr Metadata Handling

Encapsulates Zarr metadata operations across versions, supporting both 
consolidated access and per-file access. Provides a common interface 
for metadata operations.
 
The dispatcher is defined by the type NCZ_Metadata.
It offers 2 types of operations that allow decoupling/abstract
filesystem access, content reading of the JSON metadata files
1. Listings: (involves either listing or parsing consolidated view)
 - variables within a group
 - groups withing a group
2. Retrieve JSON representation of (sub)groups, arrays and attributes.
	Directly read from filesystem/objectstore or retrieve the JSON 
	object from the consolidated view respective to the group or variable

Note: This will also be the case of zarr v3
(the elements will be extracted from zarr.json instead)
*/

#ifndef ZMETADATA_H
#define ZMETADATA_H
struct NCZ_FILE_INFO;

#if defined(__cplusplus)
extern "C"
{
#endif
/* This is the version of the metadata table. It should be changed
 * when new functions are added to the metadata table. */
#ifndef NCZ_METADATA_VERSION
#define NCZ_METADATA_VERSION 1
#endif /*NCZ_METADATA_VERSION*/

#define Z2METADATA "/.zmetadata"

#define ZARR_NOT_CONSOLIDATED 0
#define ZARR_CONSOLIDATED 1

typedef enum {
	NCZMD_NULL,
	NCZMD_GROUP,
	NCZMD_ATTRS,
	NCZMD_ARRAY
} NCZMD_MetadataType;

typedef struct NCZ_Metadata
{
	int zarr_format;		/* Zarr format version */
	int dispatch_version;   /* Dispatch table version*/
	size64_t flags;			/* Metadata handling flags */
	NCjson *jcsl; // Consolidated JSON view or NULL
    int (*list_groups)(struct NCZ_FILE_INFO*, const char * key, NClist *subgrpnames);
    int (*list_variables)(struct NCZ_FILE_INFO*, const char * key, NClist *varnames);
    int (*fetch_json_content)(struct NCZ_FILE_INFO*, NCZMD_MetadataType, const char *name, NCjson **jobj);
    int (*update_json_content)(struct NCZ_FILE_INFO*, NCZMD_MetadataType, const char *name, const NCjson *jobj);
	int (*validate_consolidated)(const NCjson *jobj); // Should  validate whole JSON content of .zmetadata
} NCZ_Metadata;

// regular handler
extern const NCZ_Metadata *NCZ_metadata_handler2;
// consolidated metadata handler
extern const NCZ_Metadata *NCZ_csl_metadata_handler2;

/* Inference for the Metadata handler */
extern int NCZMD_is_metadata_consolidated(struct NCZ_FILE_INFO*zfile);
extern int NCZMD_get_metadata_format(struct NCZ_FILE_INFO*zfile, int *zarrformat); // Only pure Zarr is determined
extern int NCZMD_set_metadata_handler(struct NCZ_FILE_INFO *zfile);
extern void NCZMD_free_metadata_handler(NCZ_Metadata * zmd);

extern int NCZMD_list_groups(struct NCZ_FILE_INFO*zfile, const char * key, NClist *subgrpnames);
extern int NCZMD_list_variables(struct NCZ_FILE_INFO*zfile, const char * key, NClist *varnames);

extern int NCZMD_fetch_json_group(struct NCZ_FILE_INFO*zfile, const char *key, NCjson **jgroup);
extern int NCZMD_fetch_json_attrs(struct NCZ_FILE_INFO*zfile, const char *key, NCjson **jattrs);
extern int NCZMD_fetch_json_array(struct NCZ_FILE_INFO*zfile, const char *key, NCjson **jarrays);

/* Write operations */
extern int NCZMD_update_json_group(struct NCZ_FILE_INFO*zfile, const char *key, const NCjson *jgroup);
extern int NCZMD_update_json_attrs(struct NCZ_FILE_INFO*zfile, const char *key, const NCjson *jattrs);
extern int NCZMD_update_json_array(struct NCZ_FILE_INFO*zfile, const char *key, const NCjson *jarrays);
extern int NCZMD_consolidate(struct NCZ_FILE_INFO*zfile);


#if defined(__cplusplus)
}
#endif

#endif /* ZMETADATA_H */
