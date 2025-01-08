/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file
 * @internal This file contains functions that 
 * can be used in dispatch tables to handle
 * such things as unimplemented functions.
 * As a rule, these functions will return NC_ENOTNC4.
 * In some cases, the function may succeed and set
 * appropriate output parameter values. These are indicated
 * by the NC_NOOP prefix to the function name.
 *
 * @author Ed Hartnett, Dennis Heimbigner
 */

#include "netcdf.h"
#include "netcdf_filter.h"
#include "netcdf_aux.h"
#include "ncdispatch.h"
#include "nc4internal.h"

/**
 * @internal Not implemented in some dispatch tables
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param quantize_mode Ignored.
 * @param nsd Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_var_quantize(int ncid, int varid, int quantize_mode, int nsd)
{
   NC_UNUSED(ncid);
   NC_UNUSED(varid);
   NC_UNUSED(quantize_mode);
   NC_UNUSED(nsd);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented in some dispatch tables
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param quantize_modep Ignored.
 * @param nsdp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_var_quantize(int ncid, int varid,  int *quantize_modep,
			   int *nsdp)
{
   NC_UNUSED(ncid);
   NC_UNUSED(varid);
   NC_UNUSED(quantize_modep);
   NC_UNUSED(nsdp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented in some dispatch tables
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param id Ignored.
 * @param nparams Ignored.
 * @param parms Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_var_filter(int ncid, int varid, unsigned int  id, size_t nparams, const unsigned int* parms)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(id);
    NC_UNUSED(nparams);
    NC_UNUSED(parms);
    return NC_ENOTNC4;
}

int
NC_NOTNC4_inq_var_filter_ids(int ncid, int varid, size_t* nfilters, unsigned int* filterids)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(nfilters);
    NC_UNUSED(filterids);
    return NC_ENOTNC4;
}

int
NC_NOTNC4_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparams, unsigned int* params)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(id);
    NC_UNUSED(nparams);
    NC_UNUSED(params);
    return NC_ENOTNC4;
}

int
NC_NOOP_inq_var_filter_ids(int ncid, int varid, size_t* nfilters, unsigned int* filterids)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(filterids);
    if(nfilters) *nfilters = 0;
    return NC_NOERR;
}

int
NC_NOOP_inq_var_filter_info(int ncid, int varid, unsigned int id, size_t* nparams, unsigned int* params)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(id);
    NC_UNUSED(nparams);
    NC_UNUSED(params);
    return NC_ENOFILTER;
}

int
NC_NOOP_inq_filter_avail(int ncid, unsigned id)
{
    NC_UNUSED(ncid);
    NC_UNUSED(id);
    return NC_ENOFILTER;
}

/**
 * @internal Not allowed for classic model.
 *
 * @param parent_ncid Ignored.
 * @param name Ignored.
 * @param new_ncid Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
    NC_UNUSED(parent_ncid);
    NC_UNUSED(name);
    NC_UNUSED(new_ncid);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param grpid Ignored.
 * @param name Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_rename_grp(int grpid, const char *name)
{
    NC_UNUSED(grpid);
    NC_UNUSED(name);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param size Ignored.
 * @param name Ignored.
 * @param typeidp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(size);
    NC_UNUSED(name);
    NC_UNUSED(typeidp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param name Ignored.
 * @param offset Ignored.
 * @param field Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_insert_compound(int ncid, nc_type typeid1, const char *name, size_t offset,
                          nc_type field_typeid)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(name);
    NC_UNUSED(offset);
    NC_UNUSED(field_typeid);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param name Ignored.
 * @param offset Ignored.
 * @param field Ignored.
 * @param ndims Ignored.
 * @param dim Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
extern int
NC_NOTNC4_insert_array_compound(int ncid, int typeid1, const char *name,
                                size_t offset, nc_type field_typeid,
                                int ndims, const int *dim_sizesp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(name);
    NC_UNUSED(offset);
    NC_UNUSED(field_typeid);
    NC_UNUSED(ndims);
    NC_UNUSED(dim_sizesp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param fieldid Ignored.
 * @param name Ignored.
 * @param offsetp Ignored.
 * @param field Ignored.
 * @param ndimsp Ignored.
 * @param dim Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_compound_field(int ncid, nc_type typeid1, int fieldid, char *name,
                             size_t *offsetp, nc_type *field_typeidp, int *ndimsp,
                             int *dim_sizesp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(fieldid);
    NC_UNUSED(name);
    NC_UNUSED(offsetp);
    NC_UNUSED(field_typeidp);
    NC_UNUSED(ndimsp);
    NC_UNUSED(dim_sizesp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param name Ignored.
 * @param fieldidp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_compound_fieldindex(int ncid, nc_type typeid1, const char *name, int *fieldidp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(name);
    NC_UNUSED(fieldidp);
    return NC_ENOTNC4;
}

/* Opaque type. */

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param datum Ignored.
 * @param name Ignored.
 * @param typeidp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_opaque(int ncid, size_t datum_size, const char *name,
                     nc_type *typeidp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(datum_size);
    NC_UNUSED(name);
    NC_UNUSED(typeidp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param name Ignored.
 * @param base_typeid Ignored.
 * @param typeidp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_vlen(int ncid, const char *name, nc_type base_typeid,
                   nc_type *typeidp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(name);
    NC_UNUSED(base_typeid);
    NC_UNUSED(typeidp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param base_typeid Ignored.
 * @param name Ignored.
 * @param typeidp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_enum(int ncid, nc_type base_typeid, const char *name,
                   nc_type *typeidp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(base_typeid);
    NC_UNUSED(name);
    NC_UNUSED(typeidp);
    return NC_ENOTNC4;
}


/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param xtype Ignored.
 * @param value Ignored.
 * @param identifier Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier)
{
    NC_UNUSED(ncid);
    NC_UNUSED(xtype);
    NC_UNUSED(value);
    NC_UNUSED(identifier);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param idx Ignored.
 * @param identifier Ignored.
 * @param value Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_enum_member(int ncid, nc_type typeid1, int idx, char *identifier,
                          void *value)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(idx);
    NC_UNUSED(identifier);
    NC_UNUSED(value);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param identifier Ignored.
 * @param value Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_insert_enum(int ncid, nc_type typeid1, const char *identifier,
                      const void *value)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(identifier);
    NC_UNUSED(value);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param vlen_element Ignored.
 * @param len Ignored.
 * @param data Ignored.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_put_vlen_element(int ncid, int typeid1, void *vlen_element,
                           size_t len, const void *data)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(vlen_element);
    NC_UNUSED(len);
    NC_UNUSED(data);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param vlen_element Ignored.
 * @param len Ignored.
 * @param data Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_get_vlen_element(int ncid, int typeid1, const void *vlen_element,
                           size_t *len, void *data)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(vlen_element);
    NC_UNUSED(len);
    NC_UNUSED(data);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param size Ignored.
 * @param nelems Ignored.
 * @param preemption Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems,
                              float preemption)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(size);
    NC_UNUSED(nelems);
    NC_UNUSED(preemption);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param sizep Ignored.
 * @param nelemsp Ignored.
 * @param preemptionp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_get_var_chunk_cache(int ncid, int varid, size_t *sizep,
                              size_t *nelemsp, float *preemptionp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(sizep);
    NC_UNUSED(nelemsp);
    NC_UNUSED(preemptionp);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param shuffle Ignored.
 * @param deflate Ignored.
 * @param deflate_level Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC_NOTNC4_def_var_deflate(int ncid, int varid, int shuffle, int deflate,
                          int deflate_level)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(shuffle);
    NC_UNUSED(deflate);
    NC_UNUSED(deflate_level);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param fletcher32 Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC_NOTNC4_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(fletcher32);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param contiguous Ignored.
 * @param chunksizesp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC_NOTNC4_def_var_chunking(int ncid, int varid, int contiguous, const size_t *chunksizesp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(contiguous);
    NC_UNUSED(chunksizesp);
    return NC_EPERM;
}


/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param endianness Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_def_var_endian(int ncid, int varid, int endianness)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(endianness);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param par_access Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC_NOTNC4_var_par_access(int ncid, int varid, int par_access)
{
    NC_UNUSED(ncid);
    NC_UNUSED(varid);
    NC_UNUSED(par_access);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param name Ignored.
 * @param grp_ncid Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_ncid(int ncid, const char *name, int *grp_ncid)
{
    NC_UNUSED(ncid);
    NC_UNUSED(name);
    NC_UNUSED(grp_ncid);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param numgrps Ignored.
 * @param ncids Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_grps(int ncid, int *numgrps, int *ncids)
{
    NC_UNUSED(ncid);
    NC_UNUSED(numgrps);
    NC_UNUSED(ncids);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param name Ignored.

 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_grpname(int ncid, char *name)
{
    NC_UNUSED(ncid);
    NC_UNUSED(name);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param lenp Ignored.
 * @param full_name Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_grpname_full(int ncid, size_t *lenp, char *full_name)
{
    NC_UNUSED(ncid);
    NC_UNUSED(lenp);
    NC_UNUSED(full_name);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param parent_ncid Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_grp_parent(int ncid, int *parent_ncid)
{
    NC_UNUSED(ncid);
    NC_UNUSED(parent_ncid);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param full_name Ignored.
 * @param grp_ncid Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_grp_full_ncid(int ncid, const char *full_name, int *grp_ncid)
{
    NC_UNUSED(ncid);
    NC_UNUSED(full_name);
    NC_UNUSED(grp_ncid);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param nvars Ignored.
 * @param varids Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_varids(int ncid, int *nvars, int *varids)
{
    NC_UNUSED(ncid);
    NC_UNUSED(nvars);
    NC_UNUSED(varids);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param ndims Ignored.
 * @param dimids Ignored.
 * @param include_parents Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents)
{
    NC_UNUSED(ncid);
    NC_UNUSED(ndims);
    NC_UNUSED(dimids);
    NC_UNUSED(include_parents);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param ntypes Ignored.
 * @param typeids Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_typeids(int ncid, int *ntypes, int *typeids)
{
    NC_UNUSED(ncid);
    NC_UNUSED(ntypes);
    NC_UNUSED(typeids);
    return NC_ENOTNC4;
}

/**
 * @internal Not implemented for a dispatch table.
 *
 * @param ncid Ignored.
 * @param typeid1 Ignored.
 * @param name Ignored.
 * @param size Ignored.
 * @param base_nc_typep Ignored.
 * @param nfieldsp Ignored.
 * @param classp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_user_type(int ncid, nc_type typeid1, char *name, size_t *size,
                        nc_type *base_nc_typep, size_t *nfieldsp, int *classp)
{
    NC_UNUSED(ncid);
    NC_UNUSED(typeid1);
    NC_UNUSED(name);
    NC_UNUSED(size);
    NC_UNUSED(base_nc_typep);
    NC_UNUSED(nfieldsp);
    NC_UNUSED(classp);
    return NC_ENOTNC4;
}

/**
 * @internal Get the id of a type from the name.
 *
 * @param ncid Ignored.
 * @param name Ignored.
 * @param typeidp Ignored.
 *
 * @return ::NC_ENOTNC4 Not implemented for a dispatch table.
 * @author Ed Hartnett
 */
int
NC_NOTNC4_inq_typeid(int ncid, const char *name, nc_type *typeidp)
{
    /* Note that this should actually work for atomic types */
    NC_UNUSED(ncid);
    NC_UNUSED(name);
    NC_UNUSED(typeidp);
    return NC_ENOTNC4;
}
