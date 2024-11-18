/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */


#ifndef ZMETADATA_H
#define ZMETADATA_H
#include "zincludes.h"
#include "ncjson.h"
#include "zinternal.h"

/*
Notes on internal architecture.

Encapsulating Zarr metadata operations across versions.
Such allows to use the same interface for both consolidated
access as well as fetching each and every object on the filesystem

The dispatcher is defined by the type NCZ_Metadata_Dispatcher.
That dispatcher allows the Zarr independent code to be 
isolated from the code handling the json files.
The table has the following groups of entries:
1. List variables within a group
2. List subgroups withing a group
3. Retrieve JSON representation of (sub)groups, arrays and attributes
    depending on the implementation it might require fetch the content
    of the json file or process the consolidated json to retrive
    the corrent part
   Note: This is also the case of v3, the elements will be extracted from zarr.json

*/

/* This is the version of the metadata table. It should be changed
 * when new functions are added to the metadata table. */
#ifndef NCZ_METADATA_VERSION
#define NCZ_METADATA_VERSION 1
#endif /*NCZ_METADATA_VERSION*/

#define Z2METADATA "/.zmetadata"
#define Z3METADATA "/zarr.json"

typedef enum
{
	NCZMD_NULL,
	NCZMD_GROUP,
	NCZMD_ATTRS,
	NCZMD_ARRAY
} NCZMD_MetadataType;

typedef struct NCZ_Metadata_Dispatcher
{
	int zarr_format;
	int dispatch_version; /* Version of the dispatch table */
	size64_t flags;
#define ZARR_NOT_CONSOLIDATED 0
#define ZARR_CONSOLIDATED 1

	int (*list_groups)(NCZ_FILE_INFO_T *, NC_GRP_INFO_T *, NClist *subgrpnames);
	int (*list_variables)(NCZ_FILE_INFO_T *, NC_GRP_INFO_T *, NClist *varnames);

	int (*fetch_json_content)(NCZ_FILE_INFO_T *, NCZMD_MetadataType, const char *name, NCjson **jobj);
} NCZ_Metadata_Dispatcher;

typedef struct NCZ_Metadata
{
	NCjson *jcsl; // Internal JSON configuration
	const NCZ_Metadata_Dispatcher *dispatcher;
} NCZ_Metadata;

// "normal" handlers
extern const NCZ_Metadata_Dispatcher *NCZ_metadata_handler2;
extern const NCZ_Metadata_Dispatcher *NCZ_metadata_handler3;

// "consolidated" metadata handlers
extern const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler2;
extern const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler3;

#if defined(__cplusplus)
extern "C"
{
#endif

/* Called by nc_initialize and nc_finalize respectively */
extern int NCZMD_initialize(void);
extern int NCZMD_finalize(void);

extern int NCZMD_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
extern int NCZMD_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames);

extern int NCZMD_fetch_json_group(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jgroup);
extern int NCZMD_fetch_json_attrs(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jattrs);
extern int NCZMD_fetch_json_array(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jarrays);

/**************************************************/

/* Inference for the Metadata handler */
extern int NCZMD_is_metadata_consolidated(NCZ_FILE_INFO_T *zfile);
extern int NCZMD_get_metadata_format(NCZ_FILE_INFO_T *zfile, int *zarrformat); // Only pure Zarr is determined
extern int NCZMD_set_metadata_handler(NCZ_FILE_INFO_T *zfile, const NCZ_Metadata **mdhandlerp);

#if defined(__cplusplus)
}
#endif

#endif /* ZMETADATA_H */
