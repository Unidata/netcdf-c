/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file @internal This file contains the type functions for the HDF4
 * dispatch layer.
 *
 * @author Ed Hartnett
 */
#include "nc4internal.h"
#include "nc4dispatch.h"

#define NUM_ATOMIC_TYPES 13 /**< Number of netCDF atomic types. */

/* The sizes of types may vary from platform to platform, but within
 * netCDF files, type sizes are fixed. */
#define NC_CHAR_LEN sizeof(char)      /**< @internal Size of char. */
#define NC_STRING_LEN sizeof(char *)  /**< @internal Size of char *. */
#define NC_BYTE_LEN 1     /**< @internal Size of byte. */
#define NC_SHORT_LEN 2    /**< @internal Size of short. */
#define NC_INT_LEN 4      /**< @internal Size of int. */
#define NC_FLOAT_LEN 4    /**< @internal Size of float. */
#define NC_DOUBLE_LEN 8   /**< @internal Size of double. */
#define NC_INT64_LEN 8    /**< @internal Size of int64. */

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
HDF4_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp)
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
HDF4_insert_compound(int ncid, nc_type typeid1, const char *name, size_t offset, 
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
HDF4_insert_array_compound(int ncid, int typeid1, const char *name, 
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
HDF4_inq_compound_field(int ncid, nc_type typeid1, int fieldid, char *name, 
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
HDF4_inq_compound_fieldindex(int ncid, nc_type typeid1, const char *name, int *fieldidp)
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
HDF4_def_opaque(int ncid, size_t datum_size, const char *name, 
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
HDF4_def_vlen(int ncid, const char *name, nc_type base_typeid, 
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
HDF4_def_enum(int ncid, nc_type base_typeid, const char *name, 
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
HDF4_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier)
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
HDF4_inq_enum_member(int ncid, nc_type typeid1, int idx, char *identifier, 
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
HDF4_insert_enum(int ncid, nc_type typeid1, const char *identifier, 
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
HDF4_put_vlen_element(int ncid, int typeid1, void *vlen_element, 
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
HDF4_get_vlen_element(int ncid, int typeid1, const void *vlen_element, 
		    size_t *len, void *data)
{
   return NC_ENOTNC4;
}

