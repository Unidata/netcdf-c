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

/* zarr.c */
extern int ncz_create_dataset(NC_FILE_INFO_T*, NC_GRP_INFO_T*, const char** controls);
extern int ncz_open_dataset(NC_FILE_INFO_T*, const char** controls);
extern int ncz_del_attr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name);

extern int NCZ_isnetcdf4(struct NC_FILE_INFO*);
extern int NCZ_get_libversion(unsigned long* majorp, unsigned long* minorp,unsigned long* releasep);
extern int NCZ_get_superblock(NC_FILE_INFO_T* file, int* superblockp);

extern int ncz_unload_jatts(NCZ_FILE_INFO_T*, NC_OBJ* container, NCjson* jattrs, NCjson* jtypes);

/* zclose.c */
extern int ncz_close_file(NC_FILE_INFO_T* file, int abort);

/* zcvt.c */
extern int NCZ_convert1(NCjson* jsrc, nc_type, char* memory0);
extern int NCZ_stringconvert1(nc_type typid, char* src, char** strp);
extern int NCZ_stringconvert(nc_type typid, size_t len, void* data0, NCjson** jdatap);

/* zsync.c */
extern int ncz_sync_file(NC_FILE_INFO_T* file);
extern int ncz_sync_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
extern int ncz_sync_atts(NC_FILE_INFO_T*, NC_OBJ* container, NCindex* attlist);
extern int ncz_read_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
extern int ncz_read_atts(NC_FILE_INFO_T* file, NC_OBJ* container);
extern int ncz_read_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
extern int ncz_read_file(NC_FILE_INFO_T* file);
extern int ncz_write_var(NC_VAR_INFO_T* var);
extern int ncz_create_superblock(NCZ_FILE_INFO_T* zinfo);

/* zutil.c */
extern int NCZ_grpkey(const NC_GRP_INFO_T* grp, char** pathp);
extern int NCZ_varkey(const NC_VAR_INFO_T* var, char** pathp);
extern int NCZ_dimkey(const NC_DIM_INFO_T* dim, char** pathp);
extern int ncz_splitkey(const char* path, NClist* segments);
extern int NCZ_readdict(NCZMAP* zmap, const char* key, NCjson** jsonp);
extern int NCZ_readarray(NCZMAP* zmap, const char* key, NCjson** jsonp);
extern int ncz_zarr_type_name(nc_type nctype, int little, const char** znamep);
extern int ncz_dtype2typeinfo(const char* dtype, nc_type* nctypep, int* endianness);
extern int ncz_fill_value_sort(nc_type nctype, int*);
extern int NCZ_createobject(NCZMAP* zmap, const char* key, size64_t size);
extern int NCZ_uploadjson(NCZMAP* zmap, const char* key, NCjson* json);
extern int NCZ_downloadjson(NCZMAP* zmap, const char* key, NCjson** jsonp);
extern int NCZ_isLittleEndian(void);
extern int NCZ_subobjects(NCZMAP* map, const char* prefix, const char* tag, NClist* objlist);
extern int NCZ_grpname_full(int gid, char** pathp);
extern int ncz_get_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
extern int NCZ_comma_parse(const char* s, NClist* list);
extern int NCZ_swapatomicdata(size_t datalen, void* data, int typesize);
extern char** NCZ_clonestringvec(size_t len, const char** vec);
extern void NCZ_freestringvec(size_t len, char** vec);
extern int NCZ_create_fill_chunk(size64_t chunksize, size_t typesize, void* fill, void** fillchunkp);

#endif /*ZARR_H*/
