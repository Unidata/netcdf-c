/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file
 * @internal This file contains functions that return NC_ENOTNC4, for
 * dispatch layers that only implement the classic model.
 *
 * @author Ed Hartnett
 */

#include "nc4internal.h"
#include "nc.h"
#include "hdf4dispatch.h"
#include "ncdispatch.h"

/**
 * @internal Not allowed for classic model.
 *
 * @param ncid Ignored.
 * @param varid Ignored.
 * @param id Ignored.
 * @param nparams Ignored.
 * @param parms Ignored.
 *
 * @return ::NC_ENOTNC4 Not allowed for classic model.
 * @author Ed Hartnett
 */
int
NOTNC4_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams,
                      const unsigned int* parms)
{
   return NC_ENOTNC4;
}


/**
 * @internal Create a group. Its ncid is returned in the new_ncid
 * pointer. 
 *
 * @param parent_ncid Parent group.
 * @param name Name of new group.
 * @param new_ncid Pointer that gets ncid for new group.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ESTRICTNC3 Classic model in use for this file.
 * @return ::NC_ENOTNC4 Not a netCDF-4 file.
 * @author Ed Hartnett
*/
int
NOTNC4_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
   return NC_EPERM;
}

/**
 * @internal Rename a group. 
 *
 * @param grpid Group ID.
 * @param name New name for group.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTNC4 Not a netCDF-4 file.
 * @return ::NC_EPERM File opened read-only.
 * @return ::NC_EBADGRPID Renaming root forbidden.
 * @return ::NC_EHDFERR HDF5 function returned error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
*/
int
NOTNC4_rename_grp(int grpid, const char *name)
{
   return NC_EPERM;   
}

/**
 * @internal Create a compound type.
 *
 * @param ncid File and group ID.
 * @param size Gets size in bytes of one element of type.
 * @param name Name of the type.
 * @param typeidp Gets the type ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @author Ed Hartnett
*/
int
NOTNC4_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp)
{
   return NC_ENOTNC4;
}

/**
 * @internal Insert a named field into a compound type.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param name Name of the type.
 * @param offset Offset of field.
 * @param field_typeid Field type ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @author Ed Hartnett
*/
int
NOTNC4_insert_compound(int ncid, nc_type typeid1, const char *name, size_t offset, 
		   nc_type field_typeid)
{
   return NC_ENOTNC4;
}

/**
 * @internal Insert a named array into a compound type.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param name Name of the array field.
 * @param offset Offset in bytes.
 * @param field_typeid Type of field.
 * @param ndims Number of dims for field.
 * @param dim_sizesp Array of dim sizes.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @author Ed Hartnett
*/
extern int
NOTNC4_insert_array_compound(int ncid, int typeid1, const char *name, 
			 size_t offset, nc_type field_typeid,
			 int ndims, const int *dim_sizesp)
{
   return NC_ENOTNC4;
}

/**
 * @internal Given the ncid, typeid and fieldid, get info about the
 * field.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param fieldid Field ID.
 * @param name Gets name of field.
 * @param offsetp Gets offset of field.
 * @param field_typeidp Gets field type ID.
 * @param ndimsp Gets number of dims for this field.
 * @param dim_sizesp Gets the dim sizes for this field.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett
*/
int
NOTNC4_inq_compound_field(int ncid, nc_type typeid1, int fieldid, char *name, 
		      size_t *offsetp, nc_type *field_typeidp, int *ndimsp, 
		      int *dim_sizesp)
{
   return NC_ENOTNC4;
}

/**
 * @internal Given the typeid and the name, get the fieldid.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param name Name of field.
 * @param fieldidp Pointer that gets new field ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EBADTYPE Type not found.
 * @return ::NC_EBADFIELD Field not found.
 * @author Ed Hartnett
*/
int
NOTNC4_inq_compound_fieldindex(int ncid, nc_type typeid1, const char *name, int *fieldidp)
{
   return NC_ENOTNC4;
}

/* Opaque type. */

/**
 * @internal Create an opaque type. Provide a size and a name.
 *
 * @param ncid File and group ID.
 * @param datum_size Size in bytes of a datum.
 * @param name Name of new vlen type.
 * @param typeidp Pointer that gets new type ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @author Ed Hartnett
*/
int
NOTNC4_def_opaque(int ncid, size_t datum_size, const char *name, 
	      nc_type *typeidp)
{
   return NC_ENOTNC4;
}

/**
 * @internal Define a variable length type.
 *
 * @param ncid File and group ID.
 * @param name Name of new vlen type.
 * @param base_typeid Base type of vlen.
 * @param typeidp Pointer that gets new type ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @author Ed Hartnett
*/
int
NOTNC4_def_vlen(int ncid, const char *name, nc_type base_typeid, 
	    nc_type *typeidp)
{
   return NC_ENOTNC4;
}

/**
 * @internal Create an enum type. Provide a base type and a name. At
 * the moment only ints are accepted as base types.
 *
 * @param ncid File and group ID.
 * @param base_typeid Base type of vlen.
 * @param name Name of new vlen type.
 * @param typeidp Pointer that gets new type ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @author Ed Hartnett
*/
int
NOTNC4_def_enum(int ncid, nc_type base_typeid, const char *name, 
	    nc_type *typeidp)
{
   return NC_ENOTNC4;
}


/**
 * @internal Get enum name from enum value. Name size will be <=
 * NC_MAX_NAME.
 *
 * @param ncid File and group ID.
 * @param xtype Type ID.
 * @param value Value of enum.
 * @param identifier Gets the identifier for this enum value.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EBADTYPE Type not found.
 * @return ::NC_EINVAL Invalid type data.
 * @author Ed Hartnett
*/
int
NOTNC4_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier)
{
   return NC_ENOTNC4;
}

/**
 * @internal Get information about an enum member: an identifier and
 * value. Identifier size will be <= NC_MAX_NAME.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param idx Enum member index.
 * @param identifier Gets the identifier.
 * @param value Gets the enum value.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EBADTYPE Type not found.
 * @return ::NC_EINVAL Bad idx.
 * @author Ed Hartnett
*/
int
NOTNC4_inq_enum_member(int ncid, nc_type typeid1, int idx, char *identifier, 
		   void *value)
{
   return NC_ENOTNC4;
}

/**
 * @internal Insert a identifier value into an enum type. The value
 * must fit within the size of the enum type, the identifier size must
 * be <= NC_MAX_NAME.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param identifier Name of this enum value.
 * @param value Value of enum.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EBADTYPE Type not found.
 * @return ::NC_ETYPDEFINED Type already defined.
 * @author Ed Hartnett
*/
int
NOTNC4_insert_enum(int ncid, nc_type typeid1, const char *identifier, 
	       const void *value)
{
   return NC_ENOTNC4;
}

/**
 * @internal Insert one element into an already allocated vlen array
 * element.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param vlen_element The VLEN element to insert.
 * @param len Length of element in bytes.
 * @param data Element data.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
int
NOTNC4_put_vlen_element(int ncid, int typeid1, void *vlen_element, 
		    size_t len, const void *data)
{
   return NC_ENOTNC4;
}

/**
 * @internal Insert one element into an already allocated vlen array
 * element.
 *
 * @param ncid File and group ID.
 * @param typeid1 Type ID.
 * @param vlen_element The VLEN element to insert.
 * @param len Length of element in bytes.
 * @param data Element data.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
int
NOTNC4_get_vlen_element(int ncid, int typeid1, const void *vlen_element, 
		    size_t *len, void *data)
{
   return NC_ENOTNC4;
}

/**
 * @internal Set chunk cache size for a variable. This is the internal
 * function called by nc_set_var_chunk_cache().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param size Size in bytes to set cache.
 * @param nelems Number of elements in cache.
 * @param preemption Controls cache swapping.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
 * @returns ::NC_EINVAL Invalid input.
 * @returns ::NC_EHDFERR HDF5 error.
 * @author Ed Hartnett
 */
int
NOTNC4_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems,
                         float preemption)
{
   return NC_ENOTNC4;
}

/**
 * @internal This is called by nc_get_var_chunk_cache(). Get chunk
 * cache size for a variable.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param sizep Gets size in bytes of cache.
 * @param nelemsp Gets number of element slots in cache.
 * @param preemptionp Gets cache swapping setting.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Not a netCDF-4 file.
 * @author Ed Hartnett
 */
int
NOTNC4_get_var_chunk_cache(int ncid, int varid, size_t *sizep,
                        size_t *nelemsp, float *preemptionp)
{
   return NC_ENOTNC4;
}

/**
 * @internal Set compression settings on a variable. This is called by
 * nc_def_var_deflate().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param shuffle True to turn on the shuffle filter.
 * @param deflate True to turn on deflation.
 * @param deflate_level A number between 0 (no compression) and 9
 * (maximum compression).
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_ENOTINDEFINE Not in define mode.
 * @returns ::NC_EINVAL Invalid input
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NOTNC4_def_var_deflate(int ncid, int varid, int shuffle, int deflate,
                     int deflate_level)
{
   return NC_EPERM;
}

/**
 * @internal Set checksum on a variable. This is called by
 * nc_def_var_fletcher32().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param fletcher32 Pointer to fletcher32 setting.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_ENOTINDEFINE Not in define mode.
 * @returns ::NC_EINVAL Invalid input
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NOTNC4_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
   return NC_ENOTNC4;
}

/**
 * @internal Define chunking stuff for a var. This is called by
 * nc_def_var_chunking(). Chunking is required in any dataset with one
 * or more unlimited dimensions in HDF5, or any dataset using a
 * filter.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param contiguous Pointer to contiguous setting.
 * @param chunksizesp Array of chunksizes.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_ENOTINDEFINE Not in define mode.
 * @returns ::NC_EINVAL Invalid input
 * @returns ::NC_EBADCHUNK Bad chunksize.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NOTNC4_def_var_chunking(int ncid, int varid, int contiguous, const size_t *chunksizesp)
{
   return NC_EPERM;
}


/**
 * @internal This functions sets endianness for a netCDF-4
 * variable. Called by nc_def_var_endian().
 *
 * @note All pointer parameters may be NULL, in which case they are ignored.
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param endianness Endianness setting.
 *
 * @returns ::NC_NOERR for success
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3
 * netcdf-4 file.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_ENOTINDEFINE Not in define mode.
 * @returns ::NC_EPERM File is read only.
 * @returns ::NC_EINVAL Invalid input
 * @author Ed Hartnett
 */
int
NOTNC4_def_var_endian(int ncid, int varid, int endianness)
{
   return NC_ENOTNC4;
}

/**
 * @internal
 *
 * This function will change the parallel access of a variable from
 * independent to collective.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param par_access NC_COLLECTIVE or NC_INDEPENDENT.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Invalid ncid passed.
 * @returns ::NC_ENOTVAR Invalid varid passed.
 * @returns ::NC_ENOPAR LFile was not opened with nc_open_par/nc_create_var.
 * @returns ::NC_EINVAL Invalid par_access specified.
 * @returns ::NC_NOERR for success
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NOTNC4_var_par_access(int ncid, int varid, int par_access)
{
   return NC_ENOTNC4;
}

