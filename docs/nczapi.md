NetCDF ZARR API
====================================

# Abstract

*This document defines the variant of the netcdf-c library API
that can be used to read/write NCZarr dataset. Additionally,
any special new flags or other parameter values are defined.
It is expected that this document should be consistent with the
NetCDF ZARR Data Model Specification [].*

*Distribution of this document is currently restricted to Unidata.*

# Copyright
_Copyright 2018, UCAR/Unidata<br>
See netcdf/COPYRIGHT file for copying and redistribution conditions._

# Point of Contact

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 12/1/2018<br>
__Last Revised__: 12/1/2018

[TOC]

# Introduction {#nczapi_intro}

This document is a companion document to the 
*NetCDF ZARR Data Model Specification*[].
That document provides a semi-formal and abstract representation of
the NCZarr data model independent of any implementation.

This document describes a variant of the API provided by the netcdf-c
library as shown in its primary definition file *netcdf.h*.
Familiarity with the current netcdf-c library API is assumed.

# The netCDF-Zarr API {#nczapi_netcdf_zarr_api}

This API takes the netcdf-c library API and divides it into sets
of related functions. Any semantic differences are described.
API functions that are disallowed are also described.
Functions are organized according to the netCDF data model.

## NetCDF File Functions
````C
EXTERNL int
nc_create(const char* path, int cmode, int* ncidp);

EXTERNL int
nc__create(const char* path, int cmode, size_t initialsz, size_t* chunksizehintp, int* ncidp);

EXTERNL int
nc_open(const char* path, int mode, int* ncidp);

EXTERNL int
nc__open(const char* path, int mode, size_t* chunksizehintp, int* ncidp);

EXTERNL int
nc_inq_ncid(int ncid, const char* name, int* grp_ncid);

EXTERNL int
nc_redef(int ncid);

EXTERNL int
nc_enddef(int ncid);

EXTERNL int
nc__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align);

EXTERNL int
nc_sync(int ncid);

EXTERNL int
nc_abort(int ncid);

EXTERNL int
nc_close(int ncid);

EXTERNL int
nc_inq_path(int ncid, size_t* pathlen, char* path);

````
With exceptions, all of these functions are implemented with essentially standard semantics.

Notes:

1. The double underscore functions (e.g. *nc__create*) are implemented in terms of the single
underscore versions with the extra parameters ignored.
2. nc_sync, nc_redef, and nc_enddef may be implemented as no-op
   functions depending on the underlying implementation.
3. The syntax and interpretation of the *path* argument are implementation dependent (See Section ?).

## Dimensions
````C
EXTERNL int
nc_def_dim(int ncid, const char* name, size_t len, int* idp);

EXTERNL int
nc_inq_dimid(int ncid, const char* name, int* idp);

EXTERNL int
nc_inq_dim(int ncid, int dimid, char* name, size_t* lenp);

EXTERNL int
nc_inq_dimname(int ncid, int dimid, char* name);

EXTERNL int
nc_inq_dimlen(int ncid, int dimid, size_t* lenp);

EXTERNL int
nc_rename_dim(int ncid, int dimid, const char* name);

````
All of these functions are implemented with essentially standard semantics.

Notes:
1. These APIs all assume named dimensions. The management of named dimensions is still an open
   issue for Zarr. For writing, anonymous dimensions are not allowed, but they are for reading.
   When reading an anonymous dimension, a specially named dimension will be created to represent
   the anonymous dimension.
2. Handling of unlimited dimensions is still T.B.D.

## Types
````C
EXTERNL int
nc_inq_type(int ncid, nc_type xtype, char *name, size_t *size);

/* Get the id of a type from the name. */
EXTERNL int
nc_inq_typeid(int ncid, const char *name, nc_type *typeidp);

EXTERNL int
nc_def_opaque(int ncid, size_t size, const char* name, nc_type* xtypep);

EXTERNL int
nc_inq_opaque(int ncid, nc_type xtype, char* name, size_t* sizep);

EXTERNL int
nc_def_compound(int ncid, size_t size, const char* name, nc_type* typeidp);

EXTERNL int
nc_insert_compound(int ncid, nc_type xtype, const char* name, size_t offset, nc_type field_typeid);

EXTERNL int
nc_insert_array_compound(int ncid, nc_type xtype, const char* name, size_t offset, nc_type field_typeid, int ndims, const int* dim_sizes);

EXTERNL int
nc_inq_compound(int ncid, nc_type xtype, char* name, size_t* sizep, size_t* nfieldsp);

EXTERNL int
nc_inq_compound_name(int ncid, nc_type xtype, char* name);

EXTERNL int
nc_inq_compound_size(int ncid, nc_type xtype, size_t* sizep);

EXTERNL int
nc_inq_compound_nfields(int ncid, nc_type xtype, size_t* nfieldsp);

EXTERNL int
nc_inq_compound_field(int ncid, nc_type xtype, int fieldid, char* name, size_t* offsetp, nc_type* field_typeidp, int* ndimsp, int* dim_sizesp);

EXTERNL int
nc_inq_compound_fieldname(int ncid, nc_type xtype, int fieldid, char* name);

EXTERNL int
nc_inq_compound_fieldindex(int ncid, nc_type xtype, const char* name, int* fieldidp);

EXTERNL int
nc_inq_compound_fieldoffset(int ncid, nc_type xtype, int fieldid, size_t* offsetp);

EXTERNL int
nc_inq_compound_fieldtype(int ncid, nc_type xtype, int fieldid, nc_type* field_typeidp);

EXTERNL int
nc_inq_compound_fieldndims(int ncid, nc_type xtype, int fieldid, int* ndimsp);

EXTERNL int
nc_inq_compound_fielddim_sizes(int ncid, nc_type xtype, int fieldid, int* dim_sizes);

````
As of version 2, Zarr supports the equivalent of compound types.
Fixed length byte string (Opaques) can also be represented.

## Variables 
````C

EXTERNL int
nc_def_var(int ncid, const char* name, nc_type xtype, int ndims, const int* dimidsp, int* varidp);

EXTERNL int
nc_inq_var(int ncid, int varid, char* name, nc_type* xtypep, int* ndimsp, int* dimidsp, int* nattsp);

EXTERNL int
nc_inq_varid(int ncid, const char* name, int* varidp);

EXTERNL int
nc_inq_varname(int ncid, int varid, char* name);

EXTERNL int
nc_inq_vartype(int ncid, int varid, nc_type* xtypep);

EXTERNL int
nc_inq_varndims(int ncid, int varid, int* ndimsp);

EXTERNL int
nc_inq_vardimid(int ncid, int varid, int* dimidsp);

EXTERNL int
nc_inq_varnatts(int ncid, int varid, int* nattsp);

EXTERNL int
nc_rename_var(int ncid, int varid, const char* name);
````
The basic variable definition/inquiry functions have the standard
netCDF-4 semantics.

## Variable Representation Functions
````C
EXTERNL int
nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms);

EXTERNL int
nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparams, unsigned int* params);

EXTERNL int
nc_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int deflate_level);

EXTERNL int
nc_inq_var_deflate(int ncid, int varid, int* shufflep, int* deflatep, int* deflate_levelp);

EXTERNL int
nc_inq_var_szip(int ncid, int varid, int* options_maskp, int* pixels_per_blockp);

EXTERNL int
nc_def_var_fletcher32(int ncid, int varid, int fletcher32);

EXTERNL int
nc_inq_var_fletcher32(int ncid, int varid, int* fletcher32p);

EXTERNL int
nc_def_var_chunking(int ncid, int varid, int storage, const size_t* chunksizesp);

EXTERNL int
nc_inq_var_chunking(int ncid, int varid, int* storagep, size_t* chunksizesp);

EXTERNL int
nc_def_var_fill(int ncid, int varid, int no_fill, const void* fill_value);

EXTERNL int
nc_inq_var_fill(int ncid, int varid, int* no_fill, void* fill_valuep);

````
These function specify information about the layout and storage of variables.
The deflate and szip functions are all implemented as calls to the def/inq filter
functions. It appears that the semantics of the chunking functions
match that of Zarr so that they can be directly implemented.
Handling of the fill functions is still T.B.D.


## Variable IO
````C
EXTERNL int
nc_put_var(int ncid, int varid,  const void* op);

EXTERNL int
nc_get_var(int ncid, int varid,  void* ip);

EXTERNL int
nc_put_var1(int ncid, int varid,  const size_t* indexp, const void* op);

EXTERNL int
nc_get_var1(int ncid, int varid,  const size_t* indexp, void* ip);

EXTERNL int
nc_put_vara(int ncid, int varid,  const size_t* startp, const size_t* countp, const void* op);

EXTERNL int
nc_get_vara(int ncid, int varid,  const size_t* startp, const size_t* countp, void* ip);

EXTERNL int
nc_put_vars(int ncid, int varid,  const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, const void* op);

EXTERNL int
nc_get_vars(int ncid, int varid,  const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, void* ip);

EXTERNL int
nc_put_varm(int ncid, int varid,  const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, const ptrdiff_t* imapp, const void* op);

EXTERNL int
nc_put_var_T(int ncid, int varid, const T* op);

EXTERNL int
nc_get_var_T(int ncid, int varid, T* ip);

EXTERNL int
nc_put_var1_T(int ncid, int varid, const size_t* indexp, const T* op);

EXTERNL int
nc_get_var1_T(int ncid, int varid, const size_t* indexp, T* ip);

EXTERNL int
nc_put_vara_T(int ncid, int varid, const size_t* startp, const size_t* countp, const T* op);

EXTERNL int
nc_get_vara_short(int ncid, int varid, const size_t* startp, const size_t* countp, T* ip);

EXTERNL int
nc_put_vars_T(int ncid, int varid, const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, const T* op);

EXTERNL int
nc_get_vars_T(int ncid, int varid, const size_t* startp, const size_t* countp, ptrdiff_t* stridep, T* ip);

EXTERNL int
nc_put_varm_T(int ncid, int varid, const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, const ptrdiff_t* imapp, const T* op);

EXTERNL int
nc_get_varm_T(int ncid, int varid, const size_t* startp, const size_t* countp, const ptrdiff_t* stridep, const ptrdiff_t* imapp, T* ip);

````
The primary variable I/O functions are defined by the first eight functions in this list,
as is the case in the existing netcdf library code.
The put/get varm functions are all implemented in terms of calls to put/get vars functions,
again as in the existing code.

The get/put var T functions primarily exist to support library implemented type conversion.
If the actual variable type is different than the function type (the T), then automatic
conversion is performed from the actual type to the desired type. With some judicious refactoring,
it should be possible to reuse the existing conversion code in the netcdf-c library.

## Attributes
````C
EXTERNL int
nc_put_att(int ncid, int varid, const char* name, nc_type xtype, size_t len, const void* op);

EXTERNL int
nc_get_att(int ncid, int varid, const char* name, void* ip);

EXTERNL int
nc_inq_att(int ncid, int varid, const char* name, nc_type* xtypep, size_t* lenp);

EXTERNL int
nc_inq_attid(int ncid, int varid, const char* name, int* idp);

EXTERNL int
nc_inq_atttype(int ncid, int varid, const char* name, nc_type* xtypep);

EXTERNL int
nc_inq_attlen(int ncid, int varid, const char* name, size_t* lenp);

EXTERNL int
nc_inq_attname(int ncid, int varid, int attnum, char* name);

EXTERNL int
nc_copy_att(int ncid_in, int varid_in, const char* name, int ncid_out, int varid_out);

EXTERNL int
nc_rename_att(int ncid, int varid, const char* name, const char* newname);

EXTERNL int
nc_del_att(int ncid, int varid, const char* name);

EXTERNL int
nc_put_att_T(int ncid, int varid, const char* name, size_t len, const T* op);

EXTERNL int
nc_get_att_T(int ncid, int varid, const char* name, T* op);
````
The primary attribute put/get functions are defined by the first two functions in this list.
The get/put T functions are implemented in terms of these two more generic functions.

The get/put T functions primarily exist to support library implemented type conversion.
If the actual attribute type is different than the function type (the T), then automatic
conversion is performed from the actual type to the desired type. With some judicious refactoring,
it should be possible to reuse the existing conversion code in the netcdf-c library.

The put T functions specify the actual type of the attribute, so there is no conversion
implied. 

## Groups
````C
EXTERNL int
nc_def_grp(int parent_ncid, const char* name, int* new_ncid);

EXTERNL int
nc_rename_grp(int grpid, const char* name);
````
The semantics of the group functions appear to be completely consistent with the
existing Zarr semantics. It is assumed that the graph of groups is a tree,
which implies no cycles and no shared subgroups.


## NetCDF Error Handling
````C
EXTERNL const char*
nc_strerror(int ncerr);

EXTERNL int
nc_set_log_level(int new_level);
````
Error reporting and event logging is not defined for Zarr, so these are the
same as for the netcdf-c library.

## Miscellaneous Functions
````C
EXTERNL const char*
nc_inq_libvers(void);

EXTERNL int
nc_initialize(void);

EXTERNL int
nc_finalize(void);

EXTERNL int
nc_set_fill(int ncid, int fillmode, int* old_modep);

EXTERNL int
nc_set_default_format(int format, int* old_formatp);

EXTERNL int
nc_inq_format(int ncid, int* formatp);

EXTERNL int
nc_inq_format_extended(int ncid, int* formatp, int* modep);

EXTERNL int
nc_set_chunk_cache(size_t size, size_t nelems, float preemption);

EXTERNL int
nc_get_chunk_cache(size_t* sizep, size_t* nelemsp, float* preemptionp);

EXTERNL int
nc_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption);

EXTERNL int
nc_get_var_chunk_cache(int ncid, int varid, size_t* sizep, size_t* nelemsp, float* preemptionp);

EXTERNL int
nc_inq(int ncid, int* ndimsp, int* nvarsp, int* nattsp, int* unlimdimidp);

EXTERNL int
nc_inq_ndims(int ncid, int* ndimsp);

EXTERNL int
nc_inq_nvars(int ncid, int* nvarsp);

EXTERNL int
nc_inq_natts(int ncid, int* nattsp);

EXTERNL int
nc_delete(const char* path);

````
Notes:

1. It is unclear if the format related functions are sufficient for specifying cloud
   format information. There may be significant implementation-dependent information
   that these functions cannot provide as currently defined.
2. Use of the chunk caching functions may be completely implementation dependent.
   The idea of using a chunk cache seems to be an obvious requirement for good
   performance.
3. All the inq functions should be able to have standard netcdf semantics.
4. The *nc_delete* function has always been something of an outlier, but it is useful
   to have a way to completely remove a dataset in a way that is implementation dependent.

## To Be Determined Functions
````C
EXTERNL int
nc_inq_unlimdim(int ncid, int* unlimdimidp);

EXTERNL int
nc_inq_unlimdims(int ncid, int* nunlimdimsp, int* unlimdimidsp);
````
The nature of the implementation of these functions is yet to be determined.

## Parallelism Functions
````C
EXTERNL int
nc__create_mp(const char* path, int cmode, size_t initialsz, int basepe, size_t* chunksizehintp, int* ncidp);

EXTERNL int
nc__open_mp(const char* path, int mode, int basepe, size_t* chunksizehintp, int* ncidp);

EXTERNL int
nc_delete_mp(const char* path, int basepe);

EXTERNL int
nc_set_base_pe(int ncid, int pe);

EXTERNL int
nc_inq_base_pe(int ncid, int* pe);

````
The netcdf library parallelism-related functions are all heavily MPI oriented.
It is unclear what is to be done with these functions.

## Unimplemented Functions
````C
EXTERNL int
nc_show_metadata(int ncid);

EXTERNL int
nc_copy_var(int ncid_in, int varid, int ncid_out);

EXTERNL int
nc_def_enum(int ncid, nc_type base_typeid, const char* name, nc_type* typeidp);

EXTERNL int
nc_insert_enum(int ncid, nc_type xtype, const char* name, const void* value);

EXTERNL int
nc_inq_enum(int ncid, nc_type xtype, char* name, nc_type* base_nc_typep, size_t* base_sizep, size_t* num_membersp);

EXTERNL int
nc_inq_enum_member(int ncid, nc_type xtype, int idx, char* name, void* value);

EXTERNL int
nc_inq_enum_ident(int ncid, nc_type xtype, long long value, char* identifier);

EXTERNL int
nc_def_vlen(int ncid, const char* name, nc_type base_typeid, nc_type* xtypep);

EXTERNL int
nc_inq_vlen(int ncid, nc_type xtype, char* name, size_t* datum_sizep, nc_type* base_nc_typep);

EXTERNL int
nc_free_vlen(nc_vlen_t* vl);

EXTERNL int
nc_free_vlens(size_t len, nc_vlen_t vlens[]);

EXTERNL int
nc_put_vlen_element(int ncid, int typeid1, void* vlen_element, size_t len, const void* data);

EXTERNL int
nc_get_vlen_element(int ncid, int typeid1, const void* vlen_element, size_t* len, void* data);

EXTERNL int
nc_def_var_endian(int ncid, int varid, int endian);

EXTERNL int
nc_inq_var_endian(int ncid, int varid, int* endianp);

````
These functions are currently "unimplemented" in the sense that they will return 
the error code  *NC_ENOTBUILT*.

# Issues

## Path URLS

It is assumed that the format of a Zarr file will look like a netcdf Enhanced file
with some restrictions such as no enhanced types. However, the path for specifying
a cloud-based dataset will be more complicated than a simple file path. As with DAP2
and DAP4, it will be some kind of URL annotated with extra information relevant
to its interpretation.


## Architecture

It is expected that the netcdf-c library code that is constructed to provide
cloud storage access will have a multi-level architecture with a component
to manage the netCDF data model, a component to manage the Zarr data model,
and a component to manage access to the underlying key-value based cloud storage.
It is also probable that the first two components will be combined to produce
a single component to simultaneouly manage both data models. This is the pattern
used by the DAP4 code. However, the implementation of the key-value pair
storage is inherently controlled by the cloud in which the dataset is to be stored.
So, for example, we may need to have an Amazon S3 library or a JetStream Swift library
that mediates communication between the netcdf library and the actual cloud storage.

## Anonymous dimensions

## Unlimited Dimensions
The idea of an unlimited (expandable) dimension is extremely useful.
The current Zarr specification does not appear to allow for this,
so it is unclear how this will be resolved.

The problem is that the implementation of unlimited dimensins is non-trivial,
especially in the face of chunking.

## Variable Fill
Setting fill values for a variable can be implemented independently
of the underlying implementation, but at a significant cost because
the whole content of the variable must be initialized with the fill value.

## Endian-ness, Fletcher, Shuffle, and other Filters

The basic compression filters can be handled pretty well
through the API that exists in the netcdf-4 API. However,
the issue of *fletcher* checksumming and bit shuffle
is still to be determined.

The handling of *endianness* is still open. Currently, it
appears that a fixed endianness is assumed for Zarr, but netcdf-4
allows per-variable constrol of the endianness of the data storage.
It is probable that we will all the endian inquiry function but
will not implement the endian definition function to avoid an
unnecessary inconsistency with the existing Zarr.

## User Defined Types
The enumeration user-defined type is probably the most important
type constructor missing from Zarr. So it is probable that at some point
we will propose and implement the addition of enumerations. In principle,
it is not all that difficult. Effectively an enumeration is a named typed
that consists of a set of integers and a 1-1 mapping of those integers to
a set of names (the enumeration constants). So, what needs to be specified
is the type name, the size of the base integer type (e.g. short, int, int64, etc)
and the map between specific integers and their names.

The second most important user defined type will be what netcdf-4 calls "compound" types.
These are effectively similar to a C struct where the struct type consists of a set of
named, typed *fields*. It should be noted that the Zarr python implementation does
provide something like this by treating it as a serialized object plus the associated
(de-)serializers. Something less language dependent would be preferable.


