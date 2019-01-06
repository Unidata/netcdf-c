/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**************************************************/
/* These functions go directly to the substrate */

extern int NCCS_inq_base_pe(int ncid, int* pe);

extern int NCCS_inq_format(int ncid, int* formatp);

extern int NCCS_inq(int ncid, int* ndimsp, int* nvarsp, int* nattsp, int* unlimdimidp);

extern int NCCS_inq_type(int ncid, nc_type p2, char* p3, size_t* p4);

extern int NCCS_inq_dimid(int ncid, const char* name, int* idp);

extern int NCCS_inq_unlimdim(int ncid, int* unlimdimidp);

extern int NCCS_inq_dim(int ncid, int dimid, char* name, size_t* lenp);

extern int NCCS_rename_dim(int ncid, int dimid, const char* name);

extern int NCCS_inq_att(int ncid, int varid, const char* name,
	    nc_type* xtypep, size_t* lenp);

extern int NCCS_inq_attid(int ncid, int varid, const char *name, int *idp);

extern int NCCS_inq_attname(int ncid, int varid, int attnum, char* name);

extern int NCCS_rename_att(int ncid, int varid, const char* name, const char* newname);

extern int NCCS_del_att(int ncid, int varid, const char* p3);

extern int NCCS_get_att(int ncid, int varid, const char* name, void* value, nc_type t);

extern int NCCS_inq_var_all(int ncid, int varid, char *name, nc_type* xtypep,
               int* ndimsp, int* dimidsp, int* nattsp,
               int* shufflep, int* deflatep, int* deflate_levelp,
               int* fletcher32p, int* contiguousp, size_t* chunksizesp,
               int* no_fill, void* fill_valuep, int* endiannessp,
	       unsigned int* idp, size_t* nparamsp, unsigned int* params
               );

extern int NCCS_inq_varid(int ncid, const char *name, int *varidp);

extern int NCCS_rename_var(int ncid, int varid, const char* name);

extern int NCCS_var_par_access(int ncid, int p2, int p3);

extern int NCCS_inq_ncid(int ncid, const char* name, int* grp_ncid);

extern int NCCS_show_metadata(int ncid);

extern int NCCS_inq_grps(int ncid, int* ngrpsp, int* grpids);

extern int NCCS_inq_grpname(int ncid, char* p);

extern int NCCS_inq_unlimdims(int ncid, int* p2, int* p3);

extern int NCCS_inq_grpname_full(int ncid, size_t* p2, char* p3);

extern int NCCS_inq_grp_parent(int ncid, int* p);

extern int NCCS_inq_grp_full_ncid(int ncid, const char* fullname, int* grpidp);

extern int NCCS_inq_varids(int ncid, int* nvars, int* p);

extern int NCCS_inq_dimids(int ncid, int* ndims, int* p3, int p4);

extern int NCCS_inq_typeids(int ncid, int*  ntypes, int* p);

extern int NCCS_inq_type_equal(int ncid, nc_type t1, int p3, nc_type t2, int* p5);

extern int NCCS_inq_user_type(int ncid, nc_type t, char* p3, size_t* p4, nc_type* p5,
                   size_t* p6, int* p7);

extern int NCCS_inq_typeid(int ncid, const char* name, nc_type* t);

extern int NCCS_inq_compound_field(int ncid, nc_type xtype, int fieldid, char *name,
		      size_t *offsetp, nc_type* field_typeidp, int *ndimsp,
		      int *dim_sizesp);

extern int NCCS_inq_compound_fieldindex(int ncid, nc_type xtype, const char *name,
			   int *fieldidp);

extern int NCCS_get_vlen_element(int ncid, int p2, const void* p3, size_t* p4, void* p5);

extern int NCCS_inq_enum_member(int ncid, nc_type t1, int p3, char* p4, void* p5);

extern int NCCS_inq_enum_ident(int ncid, nc_type t1, long long p3, char* p4);

extern int NCCS_get_var_chunk_cache(int ncid, int p2, size_t* p3, size_t* p4, float* p5);

/**************************************************/
/* These functions have cloud specific implementations */

extern int NCCS_create(const char *path, int cmode,
	  size_t initialsz, int basepe, size_t *chunksizehintp,
	  int use_parallel, void* parameters,
	  struct NC_Dispatch* table, NC* ncp);
extern int NCCS_open(const char *path, int mode,
	    int basepe, size_t *chunksizehintp,
	    int use_parallel, void* parameters,
	    struct NC_Dispatch* table, NC* ncp);

extern int NCCS_redef(int);
extern int NCCS__enddef(int,size_t,size_t,size_t,size_t);
extern int NCCS_sync(int);
extern int NCCS_abort(int);
extern int NCCS_close(int,void*);
extern int NCCS_set_fill(int,int,int*);
extern int NCCS_inq_base_pe(int,int*);
extern int NCCS_set_base_pe(int,int);
extern int NCCS_inq_format(int,int*);
extern int NCCS_inq_format_extended(int,int*,int*);

extern int NCCS_inq(int,int*,int*,int*,int*);
extern int NCCS_inq_type(int, nc_type, char*, size_t*);

extern int NCCS_def_dim(int, const char*, size_t, int*);
extern int NCCS_inq_dimid(int, const char*, int*);
extern int NCCS_inq_dim(int, int, char*, size_t*);
extern int NCCS_inq_unlimdim(int ncid,  int *unlimdimidp);
extern int NCCS_rename_dim(int, int, const char*);

extern int NCCS_inq_att(int, int, const char*, nc_type*, size_t*);
extern int NCCS_inq_attid(int, int, const char*, int*);
extern int NCCS_inq_attname(int, int, int, char*);
extern int NCCS_rename_att(int, int, const char*, const char*);
extern int NCCS_del_att(int, int, const char*);
extern int NCCS_get_att(int, int, const char*, void*, nc_type);
extern int NCCS_put_att(int, int, const char*, nc_type, size_t, const void*, nc_type);

extern int NCCS_def_var(int, const char*, nc_type, int, const int*, int*);
extern int NCCS_inq_varid(int, const char*, int*);
extern int NCCS_rename_var(int, int, const char*);

extern int NCCS_get_vara(int, int, const size_t*, const size_t*, void*, nc_type);
extern int NCCS_put_vara(int, int, const size_t*, const size_t*, const void*, nc_type);

extern int NCCS_get_vars(int, int, const size_t*, const size_t*, const ptrdiff_t*, void*, nc_type);
extern int NCCS_put_vars(int, int, const size_t*, const size_t*, const ptrdiff_t*, const void*, nc_type);

extern int NCCS_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep,
               int *ndimsp, int *dimidsp, int *nattsp,
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp,
               int *no_fill, void *fill_valuep, int *endiannessp,
	       unsigned int* idp, size_t* nparamsp, unsigned int* params
              );

extern int NCCS_var_par_access(int, int, int);
extern int NCCS_def_var_fill(int, int, int, const void*);

extern int NCCS_show_metadata(int);
extern int NCCS_inq_unlimdims(int, int*, int*);
extern int NCCS_inq_ncid(int, const char*, int*);
extern int NCCS_inq_grps(int, int*, int*);
extern int NCCS_inq_grpname(int, char*);
extern int NCCS_inq_grpname_full(int, size_t*, char*);
extern int NCCS_inq_grp_parent(int, int*);
extern int NCCS_inq_grp_full_ncid(int, const char*, int*);
extern int NCCS_inq_varids(int, int* nvars, int*);
extern int NCCS_inq_dimids(int, int* ndims, int*, int);
extern int NCCS_inq_typeids(int, int* ntypes, int*);
extern int NCCS_inq_type_equal(int, nc_type, int, nc_type, int*);
extern int NCCS_def_grp(int, const char*, int*);
extern int NCCS_rename_grp(int, const char*);
extern int NCCS_inq_user_type(int, nc_type, char*, size_t*, nc_type*, size_t*, int*);
extern int NCCS_inq_typeid(int, const char*, nc_type*);

extern int NCCS_def_compound(int, size_t, const char*, nc_type*);
extern int NCCS_insert_compound(int, nc_type, const char*, size_t, nc_type);
extern int NCCS_insert_array_compound(int, nc_type, const char*, size_t, nc_type, int, const int*);
extern int NCCS_inq_compound_field(int, nc_type, int, char*, size_t*, nc_type*, int*, int*);
extern int NCCS_inq_compound_fieldindex(int, nc_type, const char*, int*);
extern int NCCS_def_vlen(int, const char*, nc_type base_typeid, nc_type*);
extern int NCCS_put_vlen_element(int, int, void*, size_t, const void*);
extern int NCCS_get_vlen_element(int, int, const void*, size_t*, void*);
extern int NCCS_def_enum(int, nc_type, const char*, nc_type*);
extern int NCCS_insert_enum(int, nc_type, const char*, const void*);
extern int NCCS_inq_enum_member(int, nc_type, int, char*, void*);
extern int NCCS_inq_enum_ident(int, nc_type, long long, char*);
extern int NCCS_def_opaque(int, size_t, const char*, nc_type*);
extern int NCCS_def_var_deflate(int, int, int, int, int);
extern int NCCS_def_var_fletcher32(int, int, int);
extern int NCCS_def_var_chunking(int, int, int, const size_t*);
extern int NCCS_def_var_endian(int, int, int);
extern int NCCS_def_var_filter(int, int, unsigned int, size_t, const unsigned int*);
extern int NCCS_set_var_chunk_cache(int, int, size_t, size_t, float);
extern int NCCS_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, float *preemptionp);

