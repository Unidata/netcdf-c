/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 *
 * @Dennis Heimbigner
 */

#ifndef ZFORMAT_H
#define ZFORMAT_H

/*
Notes on internal architecture.

Zarr version 2 vs Zarr version 3 is handled by using
a dispatch table mechanism similar to the dispatch
mechanism used in netcdf_dispatch.h to choose the
netcdf file format.

The dispatcher is defined by the type NCZ_Formatter.
That dispatcher allows the Zarr format independent code
to be isolated from the Zarr format specific code.
The table has the following groups of entries:
1. open/create/close
2. reading metadata -- use the JSON metadata of a file to a fill in the tree of an instance of NC_FILE_INFO_T.
3. writing metadata -- use an NC_FILE_INFO_T tree to build and write the JSON metadata of a file.
4. misc. actions -- e.g. building chunk keys.
*/

/* This is the version of the formatter table. It should be changed
 * when new functions are added to the formatter table. */
#ifndef NCZ_FORMATTER_VERSION
#define NCZ_FORMATTER_VERSION 1
#endif /*NCZ_FORMATTER_VERSION*/

/* struct Fill Values */
#define NCZ_CODEC_ENV_EMPTY_V2 {NCZ_CODEC_ENV_VER, 2}
#define NCZ_CODEC_ENV_EMPTY_V3 {NCZ_CODEC_ENV_VER, 3}

/* Opaque */
struct NCZ_Plugin;

/* Hold a collection of related json objects */
struct ZOBJ {
    NCjson* jobj;	/* group|var json */
    NCjson* jatts;	/* Attributes json associated with jobj field */
    int constjatts;  /* 1 => need to reclaim jatts field */
};

extern struct ZOBJ NCZ_emptyzobj(void);

/* Hold attribute info */
struct Ainfo {
    nc_type type;
    int endianness;
    size_t typelen;
};

/* Capture arguments for ncz4_create_var */
struct CVARGS {
	const char* varname;
	nc_type vtype;
	int storage;
	int scalar;
	int endianness;
	size_t maxstrlen;
	char dimension_separator;
	char order;
	size_t rank;
	size64_t shapes[NC_MAX_VAR_DIMS];
	size64_t chunks[NC_MAX_VAR_DIMS];
	int dimids[NC_MAX_VAR_DIMS];
	NClist* filterlist;
	int no_fill;
	void* fill_value;
};


/* This is the dispatch table, with a pointer to each netCDF
 * function. */
typedef struct NCZ_Formatter {
    int nczarr_format;
    int zarr_format;
    int dispatch_version; /* Version of the dispatch table */

/**************************************************/

/*File-Level Operations*/
int (*create)(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
int (*open)(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
int (*close)(NC_FILE_INFO_T* file);

/*Read JSON Metadata*/
int (*download_grp)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj);
int (*download_var)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj);

int (*decode_group)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj, NCjson** jzgrpp, NCjson** jzsuperp);
int (*decode_superblock)(NC_FILE_INFO_T* file, const NCjson* jsuper, int* zarrformat, int* nczarrformat);
int (*decode_nczarr_group)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jnczgrp, NClist* vars, NClist* subgrps, NClist* dimdefs);
int (*decode_var)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj, NClist* jfilters, size64_t** shapesp, size64_t** chunksp, NClist* dimrefs);
int (*decode_attributes)(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts);

/*Write JSON Metadata*/
int (*upload_grp)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj);
int (*upload_var)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj);

int (*encode_superblock)(NC_FILE_INFO_T* file, NCjson** jsuperp);
int (*encode_nczarr_group)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jzgroupp);
int (*encode_group)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jgroupp);

int (*encode_nczarr_array)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson** jzvarp);
int (*encode_var)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj, NCjson** jvarp);

int (*encode_attributes)(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson** jnczconp, NCjson** jsuperp, NCjson** jattsp);

/*Filter Processing*/
int (*encode_filter)(NC_FILE_INFO_T* file, NCZ_Filter* filter, NCjson** jfilterp);
int (*decode_filter)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson* jfilter, NCZ_Filter* filter);

/*Search*/
int (*searchobjects)(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrpnames);

/*Chunkkeys*/
int (*encode_chunkkey)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);
int (*decode_chunkkey)(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const char* chunkname, size_t* rankp, size64_t** chunkindicesp);

/* _ARRAY_DIMENSIONS encode/decode */
int (*encode_xarray) (NC_FILE_INFO_T* file, size_t rank, NC_DIM_INFO_T** dims, char** xarraydims, size_t* zarr_rankp);

/* Misc. */
char (*default_dimension_separator) (NC_FILE_INFO_T* file);

} NCZ_Formatter;

#if defined(__cplusplus)
extern "C" {
#endif

/* Called by nc_initialize and nc_finalize respectively */
extern int NCZF_initialize(void);
extern int NCZF_finalize(void);

/* Wrappers for the formatter functions */

/*File-Level Operations*/
extern int NCZF_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
extern int NCZF_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map);
extern int NCZF_close(NC_FILE_INFO_T* file);

/*Read JSON Metadata*/
extern int NCZF_download_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj);
extern int NCZF_download_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj);

extern int NCZF_decode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* jgroup, NCjson** jzgrpp, NCjson** jzsuperp);
extern int NCZF_decode_superblock(NC_FILE_INFO_T* file, const NCjson* jsuper, int* zarrformat, int* nczarrformat);
extern int NCZF_decode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jnczgrp, NClist* vars, NClist* subgrps, NClist* dimdefs);
extern int NCZF_decode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj, NClist* jfilters, size64_t** shapesp, size64_t** chunksp, NClist* dimrefs);
extern int NCZF_decode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts);

/*Write JSON Metadata*/
extern int NCZF_upload_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj);
extern int NCZF_upload_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj);

extern int NCZF_encode_superblock(NC_FILE_INFO_T* file, NCjson** jsuperp);
extern int NCZF_encode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jzgroupp);
extern int NCZF_encode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jgroupp);
extern int NCZF_encode_nczarr_array(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson** jzvarp);
extern int NCZF_encode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj, NCjson** jvarp);
extern int NCZF_encode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson** jnczvar, NCjson** jsuperp, NCjson** jattsp);

/*Filter Processing*/
extern int NCZF_encode_filter(NC_FILE_INFO_T* file, NCZ_Filter* filter, NCjson** jfilterp);
extern int NCZF_decode_filter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson* jfilter, NCZ_Filter* filter);

/*Search*/
extern int NCZF_searchobjects(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrpnames);

/*Chunkkeys*/
extern int NCZF_encode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, size_t rank, const size64_t* chunkindices, char dimsep, char** keyp);
extern int NCZF_decode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const char* chunkname, size_t* rankp, size64_t** chunkindicesp);

/* _ARRAY_DIMENSIONS (xarray) encode/decode */
extern int NCZF_encode_xarray(NC_FILE_INFO_T* file, size_t rank, NC_DIM_INFO_T** dims, char** xarraydims, size_t* zarr_rankp);

/* Format specific default dimension separator */
extern char NCZF_default_dimension_separator(NC_FILE_INFO_T* file);

/* Define known dispatch tables and initializers */
/* Each handles a specific NCZarr format + Pure Zarr */
/* WARNING: there is a lot of similar code in the dispatchers,
   so fixes to one may need to be propagated to the other dispatchers.
*/

extern const NCZ_Formatter* NCZ_formatter2; /* NCZarr V2 dispatch table => Zarr V2 */
extern const NCZ_Formatter* NCZ_formatter3; /* NCZarr V3 dispatch table => Zarr V3*/

/**************************************************/

/* Use inference to get map and the formatter */
extern int NCZ_get_map(NC_FILE_INFO_T* file, NCURI* url, mode_t mode, size64_t constraints, void* params, NCZMAP** mapp);
extern int NCZ_get_formatter(NC_FILE_INFO_T* file, const NCZ_Formatter** formatterp);

/**************************************************/
/* Misc. */
extern void NCZ_clear_zobj(struct ZOBJ* zobj);
extern void NCZ_reclaim_zobj(struct ZOBJ* zobj);
extern void NCZ_reclaim_json(NCjson* json);
extern void NCZ_reclaim_json_list(NClist* listj);

/**************************************************/

#if defined(__cplusplus)
}
#endif

/* Misc. Macros */
#define TESTPUREZARR if(zinfo->flags & FLAG_PUREZARR) purezarr = 1;
#define TESTXARRAY if(zinfo->flags & FLAG_XARRAYDIMS) xarray = 1;


#endif /* ZFORMAT_H */
