/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/hdf5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This include file is used if one wished to build a filter plugin
   independent of HDF5. See examples in the plugins directory
*/

#ifndef NETCDF_FILTER_BUILD_H
#define NETCDF_FILTER_BUILD_H 1

#include "netcdf_filter_hdf5_build.h"

/* Avoid including netcdf_json.h and ncjson.h */
#ifndef NCJSON_H
#include "netcdf_json.h"
#endif /*NCJSON_H*/

/* Ditto */
#ifndef NCPROPLIST_H
#include "netcdf_proplist.h"
#endif

/**************************************************/
/* Build To a NumCodecs-style C-API for Filters */

/* Version of the NCZ_codec_t structure */
#define NCZ_CODEC_CLASS_VER 1

/* List of the kinds of NCZ_codec_t formats */
#define NCZ_CODEC_HDF5 1 /* HDF5 <-> Codec converter */

/* Defined flags for filter invocation (not stored); powers of two */
#define NCZ_FILTER_DECODE 0x00000001

/* External Discovery Functions */

/*
Obtain a pointer to an instance of NCZ_codec_class_t.

NCZ_get_codec_info(void) --  returns pointer to instance of NCZ_codec_class_t.
			      Instance an be recast based on version+sort to the plugin type specific info.
So the void* return value is typically actually of type NCZ_codec_class_t*.

Signature: typedef const void* (*NCZ_get_codec_info_proto)(void);

The current object returned by NCZ_get_codec_info is a
 pointer to an instance of NCZ_codec_t.

The key to this struct are the several function pointers that do
initialize/finalize and conversion between codec JSON and HDF5
parameters.  The function pointers defined in NCZ_codec_t
manipulate HDF5 parameters and NumCodec JSON.

Obtain a pointer to an instance of NCZ_codec_class_t.

NCZ_get_codec_info(void) --  returns pointer to instance of NCZ_codec_class_t.
			      Instance an be recast based on version+sort to the plugin type specific info.
So the void* return value is typically actually of type NCZ_codec_class_t*.
*/
typedef const void* (*NCZ_get_codec_info_proto)(void);

/*
Obtain a pointer to a NULL terminated vector of NCZ_codec_class_t*.

NCZ_codec_info_defaults(void) --  returns pointer to a vector of pointers to instances of NCZ_codec_class_t. The vector is NULL terminated.
So the void* return value is typically actually of type NCZ_codec_class_t**.

Signature: typedef const void* (*NCZ_codec_info_defaults_proto)(void);

This entry point is used to return the codec information for
multiple filters that otherwise do not have codec information defined.
*/
typedef const void* (*NCZ_codec_info_defaults_proto)(void);

/* The current object returned by NCZ_get_plugin_info is a
   pointer to an instance of NCZ_codec_t.

The key to this struct are the several function pointers that do initialize/finalize
and conversion between codec JSON and HDF5 parameters.

The function pointers defined in NCZ_codec_t manipulate HDF5 parameters and NumCodec JSON.

* Initialize use of the filter. This is invoked when a filter is loaded.

void (*NCZ_codec_initialize)(struct NCproplist*);

* Finalize use of the filter. Since HDF5 does not provide this functionality, the codec may need to do it.
See H5Zblosc.c for an example. This function is invoked when a filter is unloaded.

void (*NCZ_codec_finalize)(struct NCproplist*);

* Convert a JSON representation to an HDF5 representation. Invoked when a NumCodec JSON Codec is extracted
from Zarr metadata.

int (*NCZ_codec_to_hdf5)(struct NCproplist* env, const char* codec, unsigned int* idp, size_t* nparamsp, unsigned** paramsp);

@param env -- (in) extra environmental information
@param codec   -- (in) ptr to JSON string representing the codec.
@param idp -- the hdf5 filter id number;
@param nparamsp -- (out) store the length of the converted HDF5 unsigned vector
@param paramsp -- (out) store a pointer to the converted HDF5 unsigned vector;
                  caller frees. Note the double indirection.
@return -- a netcdf-c error code.


* Convert an HDF5 vector of visible parameters to a JSON representation.

int (*NCZ_hdf5_to_codec)(struct NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp);

@param env -- (in) extra environmental information
@param id -- the hdf5 filter id number;
@param nparams -- (in) the length of the HDF5 unsigned vector
@param params -- (in) pointer to the HDF5 unsigned vector.
@param codecp -- (out) store the string representation of the codec; caller must free.
@return -- a netcdf-c error code.

* Convert a set of visible parameters to a set of working parameters using extra environmental information.
Also allows for changes to the visible parameters. Invoked before filter is actually used.

int (*NCZ_modify_parameters)(const struct NCproplist* env, unsigned* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

@param env -- (in) properties, including file ncid and the variable varid
@param idp -- (in/out) the hdf5 filter id number;
@params vnparamsp -- (in/out) number of visible parameters
@params vparamsp -- (in/out) vector of visible parameters
@params wnparamsp -- (out) number of working parameters
@params wparamsp -- (out) vector of working parameters
@return -- a netcdf-c error code.

* Convert an HDF5 vector of visible parameters to a JSON representation.

-int (*NCZ_hdf5_to_codec)(const struct NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp);

@param env -- (in) extra environmental information
@param id -- (in) the hdf5 filter id number;
@param nparams -- (in) the length of the HDF5 unsigned vector
@param params -- (in) pointer to the HDF5 unsigned vector.
@param codecp -- (out) store the string representation of the codec; caller must free.
@return -- a netcdf-c error code.

*/

/* Opaque */
struct NCproplist;
struct NCjson;

/* Test if JSON dict is in raw format.
@param jraw to test
@return NCJ_OK if in raw format; NCJ_ERR/NC_ERR otherwise.
*/
#ifndef NCraw_test
#define NC_RAWTAG "hdf5raw"
#define NC_RAWVERSION "1"
#define NCraw_test(jraw) (jraw == NULL || NCJsort(jraw) != NCJ_DICT \
		? NCJ_ERR \
		: (strcmp(NCJstring(NCJdictlookup(jraw,NC_RAWTAG)),NC_RAWVERSION)!=0 ? NCJ_ERR : NCJ_OK))
#endif /*NCraw_test*/

/*
The struct that provides the necessary filter info.
The combination of version + sort uniquely determines
the format of the remainder of the struct
*/
typedef struct NCZ_codec_t {
    int version; /* Version number of the struct */
    int sort; /* Format of remainder of the struct;
                 Currently always NCZ_CODEC_HDF5 */
    const char* codecid;            /* The name/id of the codec */
    unsigned hdf5id; /* corresponding hdf5 id */
    void (*NCZ_codec_initialize)(const struct NCproplist* env);
    void (*NCZ_codec_finalize)(const struct NCproplist* env);
    int (*NCZ_codec_to_hdf5)(const struct NCproplist* env, const char* codec, unsigned* idp, size_t* nparamsp, unsigned** paramsp);
    int (*NCZ_hdf5_to_codec)(const struct NCproplist* env, unsigned id, size_t nparams, const unsigned* params, char** codecp);
    int (*NCZ_modify_parameters)(const struct NCproplist* env, unsigned* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);
} NCZ_codec_t;

#ifndef NC_UNUSED
#define NC_UNUSED(var) (void)var
#endif

#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif
#endif

#endif /*NETCDF_FILTER_BUILD_H*/
