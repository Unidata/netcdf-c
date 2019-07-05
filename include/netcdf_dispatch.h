/* Copyright 2019 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 * This header contains the definition of the dispatch table. This
 * table contains a pointer to every netcdf function. When a file is
 * opened or created, the dispatch code in libdispatch decides which
 * dispatch table to use, and all subsequent netCDF calls for that
 * file will use the selected dispatch table. There are dispatch
 * tables for HDF5, HDF4, pnetcdf, etc.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#ifndef NETCDF_DISPATCH_H
#define NETCDF_DISPATCH_H

struct NC_Dispatch
{
    int model; /* one of the NC_FORMATX #'s */

    int (*create)(const char *path, int cmode, size_t initialsz,
                  int basepe, size_t *chunksizehintp, void *parameters,
                  const struct NC_Dispatch *table, NC *ncp);
    int (*open)(const char *path, int mode, int basepe, size_t *chunksizehintp,
                void *parameters, const struct NC_Dispatch *table, NC *ncp);

    int (*redef)(int);
    int (*_enddef)(int,size_t,size_t,size_t,size_t);
    int (*sync)(int);
    int (*abort)(int);
    int (*close)(int, void *);
    int (*set_fill)(int, int, int *);
    int (*inq_base_pe)(int, int *);
    int (*set_base_pe)(int, int);
    int (*inq_format)(int, int*);
    int (*inq_format_extended)(int, int *, int *);

    int (*inq)(int, int *, int *, int *, int *);
    int (*inq_type)(int, nc_type, char *, size_t *);

    int (*def_dim)(int, const char *, size_t, int *);
    int (*inq_dimid)(int, const char *, int *);
    int (*inq_dim)(int, int, char *, size_t *);
    int (*inq_unlimdim)(int ncid,  int *unlimdimidp);
    int (*rename_dim)(int, int, const char *);

    int (*inq_att)(int, int, const char *, nc_type *, size_t *);
    int (*inq_attid)(int, int, const char *, int*);
    int (*inq_attname)(int, int, int, char *);
    int (*rename_att)(int, int, const char *, const char *);
    int (*del_att)(int, int, const char *);
    int (*get_att)(int, int, const char *, void *, nc_type);
    int (*put_att)(int, int, const char *, nc_type, size_t, const void *, nc_type);

    int (*def_var)(int, const char *, nc_type, int, const int *, int *);
    int (*inq_varid)(int, const char *, int *);
    int (*rename_var)(int, int, const char *);

    int (*get_vara)(int, int, const size_t *, const size_t *, void *, nc_type);
    int (*put_vara)(int, int, const size_t *, const size_t *,
                    const void *, nc_type);

    int (*get_vars)(int, int, const size_t *, const size_t *, const ptrdiff_t *,
                    void *, nc_type);
    int (*put_vars)(int, int, const size_t *, const size_t *, const ptrdiff_t *,
                    const void *, nc_type);

    int (*get_varm)(int, int, const size_t *, const size_t *, const ptrdiff_t *,
                    const ptrdiff_t *, void *, nc_type);
    int (*put_varm)(int, int, const size_t *, const size_t *, const ptrdiff_t *,
                    const ptrdiff_t *, const void *, nc_type);

    int (*inq_var_all)(int ncid, int varid, char *name, nc_type *xtypep,
                       int *ndimsp, int *dimidsp, int *nattsp,
                       int *shufflep, int *deflatep, int *deflate_levelp,
                       int *fletcher32p, int *contiguousp, size_t *chunksizesp,
                       int *no_fill, void *fill_valuep, int *endiannessp,
                       unsigned int *idp, size_t *nparamsp, unsigned int *params);

    int (*var_par_access)(int, int, int);
    int (*def_var_fill)(int, int, int, const void *);

/* Note the following may still be invoked by netcdf client code
   even when the file is a classic file; they will just return an error or
   be ignored.
*/
#ifdef USE_NETCDF4
    int (*show_metadata)(int);
    int (*inq_unlimdims)(int, int*, int*);
    int (*inq_ncid)(int, const char*, int*);
    int (*inq_grps)(int, int*, int*);
    int (*inq_grpname)(int, char*);
    int (*inq_grpname_full)(int, size_t*, char*);
    int (*inq_grp_parent)(int, int*);
    int (*inq_grp_full_ncid)(int, const char*, int*);
    int (*inq_varids)(int, int* nvars, int*);
    int (*inq_dimids)(int, int* ndims, int*, int);
    int (*inq_typeids)(int, int* ntypes, int*);
    int (*inq_type_equal)(int, nc_type, int, nc_type, int*);
    int (*def_grp)(int, const char*, int*);
    int (*rename_grp)(int, const char*);
    int (*inq_user_type)(int, nc_type, char*, size_t*, nc_type*, size_t*, int*);
    int (*inq_typeid)(int, const char*, nc_type*);

    int (*def_compound)(int, size_t, const char *, nc_type *);
    int (*insert_compound)(int, nc_type, const char *, size_t, nc_type);
    int (*insert_array_compound)(int, nc_type, const char *, size_t, nc_type,
                                 int, const int *);
    int (*inq_compound_field)(int, nc_type, int, char *, size_t *, nc_type *,
                              int *, int *);
    int (*inq_compound_fieldindex)(int, nc_type, const char *, int *);
    int (*def_vlen)(int, const char *, nc_type base_typeid, nc_type *);
    int (*put_vlen_element)(int, int, void *, size_t, const void *);
    int (*get_vlen_element)(int, int, const void *, size_t *, void *);
    int (*def_enum)(int, nc_type, const char *, nc_type *);
    int (*insert_enum)(int, nc_type, const char *, const void *);
    int (*inq_enum_member)(int, nc_type, int, char *, void *);
    int (*inq_enum_ident)(int, nc_type, long long, char *);
    int (*def_opaque)(int, size_t, const char *, nc_type*);
    int (*def_var_deflate)(int, int, int, int, int);
    int (*def_var_fletcher32)(int, int, int);
    int (*def_var_chunking)(int, int, int, const size_t *);
    int (*def_var_endian)(int, int, int);
    int (*def_var_filter)(int, int, unsigned int, size_t, const unsigned int *);
    int (*set_var_chunk_cache)(int, int, size_t, size_t, float);
    int (*get_var_chunk_cache)(int ncid, int varid, size_t *sizep,
                               size_t *nelemsp, float *preemptionp);
#endif /*USE_NETCDF4*/
};

#endif /* NETCDF_DISPATCH_H */
