/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/*
Zarr Metadata Handling

Encapsulates Zarr metadata operations across versions, supporting both 
consolidated access and per-file access. Provides a common interface 
for metadata operations.
 
The dispatcher is defined by the type NCZ_Metadata_Dispatcher.
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
#define Z3METADATA "/zarr.json"

#define ZARR_NOT_CONSOLIDATED 0
#define ZARR_CONSOLIDATED 1

typedef enum {
	NCZMD_NULL,
	NCZMD_GROUP,
	NCZMD_ATTRS,
	NCZMD_ARRAY
} NCZMD_MetadataType;

typedef struct NCZ_Metadata_Dispatcher
{
	int zarr_format;		/* Zarr format version */
	int dispatch_version;   /* Dispatch table version*/
	size64_t flags;			/* Metadata handling flags */
    int (*list_groups)(struct NCZ_FILE_INFO*, NC_GRP_INFO_T *, NClist *subgrpnames);
    int (*list_variables)(struct NCZ_FILE_INFO*, NC_GRP_INFO_T *, NClist *varnames);
    int (*fetch_json_content)(struct NCZ_FILE_INFO*, NCZMD_MetadataType, const char *name, NCjson **jobj);
    int (*update_json_content)(struct NCZ_FILE_INFO*, NCZMD_MetadataType, const char *name, const NCjson *jobj);
	int (*validate_consolidated)(const NCjson *jobj); // Should  validate whole JSON content of .zmetadata
} NCZ_Metadata_Dispatcher;

typedef struct NCZ_Metadata
{
	NCjson *jcsl; // Consolidated JSON view or NULL
	const NCZ_Metadata_Dispatcher *dispatcher;
} NCZ_Metadata;

// regular handler
extern const NCZ_Metadata_Dispatcher *NCZ_metadata_handler2;
// consolidated metadata handler
extern const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler2;


/* Called by nc_initialize and nc_finalize respectively */
extern int NCZMD_initialize(void);
extern int NCZMD_finalize(void);

extern int NCZMD_list_groups(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
extern int NCZMD_list_variables(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, NClist *varnames);

extern int NCZMD_fetch_json_group(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jgroup);
extern int NCZMD_fetch_json_attrs(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jattrs);
extern int NCZMD_fetch_json_array(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jarrays);

/* Write operations */
extern int NCZMD_update_json_group(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, const char *name, const NCjson *jgroup);
extern int NCZMD_update_json_attrs(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, const char *name, const NCjson *jattrs);
extern int NCZMD_update_json_array(struct NCZ_FILE_INFO*zfile, NC_GRP_INFO_T *grp, const char *name, const NCjson *jarrays);
extern int NCZMD_consolidate(struct NCZ_FILE_INFO*zfile);
/**************************************************/

/* Inference for the Metadata handler */
extern int NCZMD_is_metadata_consolidated(struct NCZ_FILE_INFO*zfile);
extern int NCZMD_get_metadata_format(struct NCZ_FILE_INFO*zfile, int *zarrformat); // Only pure Zarr is determined
extern int NCZMD_set_metadata_handler(struct NCZ_FILE_INFO *zfile);
extern void NCZMD_free_metadata_handler(NCZ_Metadata * zmd);

#if defined(__cplusplus)
}
#endif

#endif /* ZMETADATA_H */
