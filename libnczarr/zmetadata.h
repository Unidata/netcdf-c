/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/*
Zarr Metadata Handling

Encapsulates Zarr metadata operations across versions, supporting both 
consolidated access and per-file access. Provides a common interface 
for metadata operations.
 
The dispatcher is defined by the type NCZ_Metadata_Dispatcher.
It offers several types of operations that allow decoupling/abstract
filesystem access, content reading of the JSON metadata files
1. Retrieve JSON representation of (sub)groups, arrays and attributes.
	Directly read from filesystem/objectstore or retrieve the JSON 
	object from the consolidated view respective to the group or variable
2. Wrappers for selected zmap operations that are key based.

Note: This will also be the case of zarr v3
(the elements will be extracted from zarr.json instead)
*/

#ifndef ZMETADATA_H
#define ZMETADATA_H

#if defined(__cplusplus)
extern "C"
{
#endif
/* This is the version of the metadata table. It should be changed
 * when new functions are added to the metadata table. */
#ifndef NCZ_METADATA_VERSION
#define NCZ_METADATA_VERSION 1
#endif /*NCZ_METADATA_VERSION*/

/* The keys in this dict are all stored in root group's container for consolidated metadata */
#define MINIMIM_CSL_REP2_RAW "{\"zarr_consolidated_format\":1, \"metadata\":{}}"
#define MINIMIM_CSL_REP3_RAW "{\"zarr_format\": 3, \"node_type\": \"group\", \"attributes\": {}, \"kind\": \"inline\", \"must_understand\": False, \"metadata\": {}}"

typedef enum NCZMD_MetadataType {
	NCZMD_NULL,
	NCZMD_GROUP,
	NCZMD_ATTRS,
	NCZMD_ARRAY
} NCZMD_MetadataType;

typedef struct NCZ_Metadata_Dispatcher
{
	int zarr_format;	/* Zarr format version */
	int dispatch_version;   /* Dispatch table version*/
	size64_t flags;		/* Metadata handling flags */
#define     ZARR_NOT_CONSOLIDATED 0
#define     ZARR_CONSOLIDATED 1
	int (*open)(NC_FILE_INFO_T* file);
	int (*create)(NC_FILE_INFO_T* file);
	int (*close)(NC_FILE_INFO_T* file);
	int (*consolidate)(NC_FILE_INFO_T* file);
	int (*fetch_json_content)(NC_FILE_INFO_T *, NCZMD_MetadataType, const char* key, NCjson** jobj);
	int (*update_json_content)(NC_FILE_INFO_T *, NCZMD_MetadataType, const char *key, const NCjson *jobj);

	/* zmap wrappers */
	int (*list)(NC_FILE_INFO_T*, const char* prefix, NClist* matches);
	int (*listall)(NC_FILE_INFO_T*, const char* prefix, NClist* matches);
	int (*exists)(NC_FILE_INFO_T* file, const char* prefix);
} NCZ_Metadata_Dispatcher;

typedef struct NCZ_Metadata
{
	NCjson *jcsl; /* Consolidated JSON container: .zmetadata for V2,
			 or root group zarr.json (minus "metadata" dict) for V3 */
	const NCjson *jmeta; /* "metadata" dict from jcsl (or NULL) */
	int dirty; /* The consolidated metadata was modified */
	const NCZ_Metadata_Dispatcher *dispatcher;
} NCZ_Metadata;

/* Handler when not using consolidated metadata */
extern const NCZ_Metadata_Dispatcher *NCZ_metadata_handler;
/* Consolidated metadata handler for Zarr version 2*/
extern const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler2;
/* Consolidated metadata handler for Zarr version 3*/
extern const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler3;

/* Called by nc_initialize and nc_finalize respectively */
extern int NCZMD_initialize(void);
extern int NCZMD_finalize(void);

extern int NCZMD_open(NC_FILE_INFO_T *file);
extern int NCZMD_create(NC_FILE_INFO_T *file);
extern int NCZMD_close(NC_FILE_INFO_T *file);
extern int NCZMD_consolidate(NC_FILE_INFO_T *file);

extern int NCZMD_fetch_json_content(NC_FILE_INFO_T *, NCZMD_MetadataType, const char* key, NCjson** jobjp);
extern int NCZMD_update_json_content(NC_FILE_INFO_T *, NCZMD_MetadataType, const char *key, const NCjson *jobj);

extern int NCZMD_list(NC_FILE_INFO_T*, const char* prefix, struct NClist* matches);
extern int NCZMD_listall(NC_FILE_INFO_T*, const char* prefix, struct NClist* matches);
extern int NCZMD_exists(NC_FILE_INFO_T* file, const char* prefix);

/**************************************************/

/* Inference for the Metadata handler */
extern int NCZMD_is_metadata_consolidated(NC_FILE_INFO_T* file);
extern int NCZMD_get_metadata_format(NC_FILE_INFO_T *zfile, int *zarrformat); /* Only pure Zarr is determined */
extern int NCZMD_set_metadata_handler(NC_FILE_INFO_T *zfile);
extern void NCZMD_clear_metadata_handler(NCZ_Metadata * zmd);

#if defined(__cplusplus)
}
#endif

#endif /* ZMETADATA_H */
