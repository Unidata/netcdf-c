/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
 * Provide the zarr specific code to implement the netcdf-4 code.
 *
 * @author Dennis Heimbigner
 */

#ifndef ZUTIL_H
#define ZUTIL_H

EXTERNL int NCZ_grpkey(const NC_GRP_INFO_T* grp, char** pathp);
EXTERNL int NCZ_varkey(const NC_VAR_INFO_T* var, char** pathp);
EXTERNL int NCZ_dimkey(const NC_DIM_INFO_T* dim, char** pathp);
EXTERNL int ncz_splitkey(const char* path, NClist* segments);
EXTERNL int ncz_nctypedecode(const char* snctype, nc_type* nctypep);
EXTERNL int NCZ_inferattrtype(const char* aname, nc_type typehint, const NCjson* value, nc_type* typeidp);
EXTERNL int ncz_fill_value_sort(nc_type nctype, int*);
EXTERNL int NCZ_createobject(NCZMAP* zmap, const char* key, size64_t size);
EXTERNL int NCZ_uploadjson(NCZMAP* zmap, const char* key, const NCjson* json);
EXTERNL int NCZ_downloadjson(NCZMAP* zmap, const char* key, NCjson** jsonp);
EXTERNL int NCZ_isLittleEndian(void);
EXTERNL int NCZ_subobjects(NCZMAP* map, const char* prefix, const char* tag, char dimsep, NClist* objlist);
EXTERNL int NCZ_grpname_full(int gid, char** pathp);
EXTERNL int NCZ_comma_parse(const char* s, NClist* list);
EXTERNL int NCZ_swapatomicdata(size_t datalen, void* data, int typesize);
EXTERNL char** NCZ_clonestringvec(size_t len, const char** vec);
EXTERNL void NCZ_freestringvec(size_t len, char** vec);
EXTERNL void NCZ_clearstringvec(size_t len, char** vec);
EXTERNL void NCZ_reclaim_diminfo_list(NClist* diminfo);
EXTERNL void NCZ_reclaim_diminfo(struct NCZ_DimInfo* diminfo);
EXTERNL void NCZ_reclaim_dimdecl_list(NClist* dimdecl);
EXTERNL void NCZ_reclaim_dimdecl(struct NCZ_DimDecl* dimdecl);
EXTERNL int NCZ_ischunkname(const char* name,char dimsep);
EXTERNL int NCZ_fixed2char(const void* fixed, char** charp, size_t count, size_t maxstrlen);
EXTERNL int NCZ_char2fixed(const char** charp, void* fixed, size_t count, size_t maxstrlen);
EXTERNL int NCZ_copy_data(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const void* memory, size_t count, int reading, void* copy);
EXTERNL int NCZ_iscomplexjson(const char* aname, const NCjson* value);
EXTERNL int NCZ_iscomplexjsonstring(const char* aname, size_t textlen, const char* text, NCjson** jsonp);
EXTERNL int NCZ_makeFQN(NC_OBJ* obj, NCbytes* fqn);
EXTERNL int NCZ_locateFQN(NC_GRP_INFO_T* parent, const char* fqn, NC_SORT sort, NC_OBJ** objectp, char** basenamep);
EXTERNL int NCZ_search_name(NC_GRP_INFO_T* startgrp, const char* name, NC_SORT sort, NC_OBJ** objectp);
EXTERNL char* NCZ_deescape(const char* s);
EXTERNL char* NCZ_backslashescape(const char* s);
EXTERNL int NCZ_sortstringlist(void* vec, size_t count);
EXTERNL int NCZ_sortpairlist(void* vec, size_t count);
EXTERNL struct NCZ_AttrInfo NCZ_emptyAttrInfo(void);
EXTERNL void NCZ_clearAttrInfo(NC_FILE_INFO_T* file, struct NCZ_AttrInfo* ainfo);
EXTERNL void NCZ_setatts_read(NC_OBJ* container);
EXTERNL int NCZ_decodesizet64vec(const NCjson* jshape, size_t* rankp, size64_t* shapes);
EXTERNL int NCZ_decodesizetvec(const NCjson* jshape, size_t* rankp, size_t* shapes);
EXTERNL int NCZ_uniquedimname(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NCZ_DimInfo* dimdata, NC_DIM_INFO_T** dimp, NCbytes* dimname);
EXTERNL int NCZ_computeattrdata(NC_FILE_INFO_T* file, const NCjson* jdata, struct NCZ_AttrInfo* att);
EXTERNL int NCZ_dictgetalt(const NCjson* jdict, const NCjson** jvaluep, ...);
EXTERNL int NCZ_dictgetalt2(const NCjson* jdict, const NCjson** jvaluep, const char* name1, const char* name2); /* common case */
EXTERNL int NCZ_getnczarrkey(NC_FILE_INFO_T* file, struct ZOBJ* jsonz, const char* name, const NCjson** jncxxxp);
EXTERNL char* NCZ_chunkpath(struct ChunkKey key);
EXTERNL size_t NCZ_get_maxstrlen(NC_OBJ* obj);
EXTERNL char NCZ_get_dimsep(NC_VAR_INFO_T* var);
EXTERNL double* NCZ_isnaninfstring(const char* val);
EXTERNL int NCZ_zarrproplist(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCproplist** clonep);

#endif /*ZUTIL_H*/