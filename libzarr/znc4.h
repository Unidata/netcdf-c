
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

extern int NCCS_inq_format_extended(int ncid, int* formatp, int* modep);

