/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
 * Provide the zarr specific code to implement the netcdf-4 code.
 *
 * @author Dennis Heimbigner
 */

#ifndef ZARR_H
#define ZARR_H

struct ChunkKey;
struct S3credentials;

/* zarr.c */
EXTERNL int ncz_create_dataset(NC_FILE_INFO_T*, NC_GRP_INFO_T*, const char** controls);
EXTERNL int ncz_open_dataset(NC_FILE_INFO_T*, const char** controls);
EXTERNL int ncz_del_attr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name);

/* HDF5 Mimics */
EXTERNL int NCZ_isnetcdf4(struct NC_FILE_INFO*);
EXTERNL int NCZ_get_libversion(unsigned long* majorp, unsigned long* minorp,unsigned long* releasep);
EXTERNL int NCZ_get_superblock(NC_FILE_INFO_T* file, int* superblockp);

EXTERNL int ncz_unload_jatts(NCZ_FILE_INFO_T*, NC_OBJ* container, NCjson* jattrs, NCjson* jtypes);

/* zclose.c */
EXTERNL int ncz_close_file(NC_FILE_INFO_T* file, int abort);

/* zcvt.c */
EXTERNL int NCZ_convert1(NCjson* jsrc, nc_type, unsigned char* memory0);
EXTERNL int NCZ_stringconvert1(nc_type typid, size_t len, char* src, NCjson* jvalue);
EXTERNL int NCZ_stringconvert(nc_type typid, size_t len, void* data0, NCjson** jdatap);

/* zsync.c */
EXTERNL int ncz_sync_file(NC_FILE_INFO_T* file, int isclose);
EXTERNL int ncz_sync_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, int isclose);
EXTERNL int ncz_sync_atts(NC_FILE_INFO_T*, NC_OBJ* container, NCindex* attlist, int isclose);
EXTERNL int ncz_read_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
EXTERNL int ncz_read_atts(NC_FILE_INFO_T* file, NC_OBJ* container);
EXTERNL int ncz_read_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
EXTERNL int ncz_read_file(NC_FILE_INFO_T* file);
EXTERNL int ncz_write_var(NC_VAR_INFO_T* var);
EXTERNL int ncz_read_superblock(NC_FILE_INFO_T* zinfo, char** nczarrvp, char** zarrfp);

/* zutil.c */
EXTERNL int NCZ_grpkey(const NC_GRP_INFO_T* grp, char** pathp);
EXTERNL int NCZ_varkey(const NC_VAR_INFO_T* var, char** pathp);
EXTERNL int NCZ_dimkey(const NC_DIM_INFO_T* dim, char** pathp);
EXTERNL int ncz_splitkey(const char* path, NClist* segments);
EXTERNL int NCZ_readdict(NCZMAP* zmap, const char* key, NCjson** jsonp);
EXTERNL int NCZ_readarray(NCZMAP* zmap, const char* key, NCjson** jsonp);
EXTERNL int ncz_zarr_type_name(nc_type nctype, int little, const char** znamep);
EXTERNL int ncz_nctype2typeinfo(const char* snctype, nc_type* nctypep);
EXTERNL int ncz_dtype2typeinfo(const char* dtype, nc_type* nctypep, int* endianness);
EXTERNL int ncz_fill_value_sort(nc_type nctype, int*);
EXTERNL int NCZ_createobject(NCZMAP* zmap, const char* key, size64_t size);
EXTERNL int NCZ_uploadjson(NCZMAP* zmap, const char* key, NCjson* json);
EXTERNL int NCZ_downloadjson(NCZMAP* zmap, const char* key, NCjson** jsonp);
EXTERNL int NCZ_isLittleEndian(void);
EXTERNL int NCZ_subobjects(NCZMAP* map, const char* prefix, const char* tag, char dimsep, NClist* objlist);
EXTERNL int NCZ_grpname_full(int gid, char** pathp);
EXTERNL int ncz_get_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
EXTERNL int NCZ_comma_parse(const char* s, NClist* list);
EXTERNL int NCZ_swapatomicdata(size_t datalen, void* data, int typesize);
EXTERNL char** NCZ_clonestringvec(size_t len, const char** vec);
EXTERNL void NCZ_freestringvec(size_t len, char** vec);
EXTERNL int NCZ_s3clear(NCS3INFO* s3map);
EXTERNL int NCZ_ischunkname(const char* name,char dimsep);
EXTERNL char* NCZ_chunkpath(struct ChunkKey key);
EXTERNL int NCZ_reclaim_fill_value(NC_VAR_INFO_T* var);
EXTERNL int NCZ_copy_fill_value(NC_VAR_INFO_T* var, void** dstp);

/* zwalk.c */
EXTERNL int NCZ_read_chunk(int ncid, int varid, size64_t* zindices, void* chunkdata);

#endif /*ZARR_H*/
