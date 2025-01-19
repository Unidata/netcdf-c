/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */

/**
 * @file This header file containsfilter related  macros, types, and prototypes for
 * the filter code in libnczarr. This header should not be included in
 * code outside libnczarr.
 *
 * @author Dennis Heimbigner
 */

#ifndef ZFILTER_H
#define ZFILTER_H

/* zfilter.c */
/* Dispatch functions are also in zfilter.c */
/* Filterlist management */

/*Mnemonic*/
#define ENCODING 1

/* The NC_VAR_INFO_T->filters field is an NClist of this struct.
Each filter can have two parts: HDF5 and Codec.
The NC_VAR_INFO_T.filters list only holds entries where both the HDF5 info
and the codec info are defined.
The NCZ_VAR_INFO_T.codecs list holds the codec info when reading a Zarr file.
Note that it is not possible to have an entry on the filters list that does not
have both HDF5 and codec. This is because nc_def_var_filter will fail if the codec
part is not available. If a codec is read from a file and there is no available
corresponding HDF5 implementation, then that codec will not appear in the filters list.
It is possible that some subset of the codecs do have a corresponding HDF5, but we
enforce the rule that no entries go into the filters list unless all are defined.
It is still desirable for a user to be able to see what filters and codecs are defined
for a variable. This is accommodated by providing two special attributes:
1, "_Filters" attribute shows the HDF5 filters defined on the variable, if any.
2, "_Codecs" attribute shows the codecs defined on the variable; for zarr, this list
   should always be defined.

For Zarr V3, we add a notion of "pseudo" filters. These are filters
that place-holders to satisfy the Zarr V3 spec, but whose action is
actually handled elsewhere in the nczarr code.
There is currently one one such pseudo filter: "bytes".
This filter is expected to be the first filter in the filter chain.
Semantically, "bytes" handles data type endianness.

Such filters do not appear in the _Codecs attribute or the _Filters attribute,
but are in the Zarr metadata.

When reading a V3 Zarr dataset, the "bytes" codec must occur first in the filter chain,
else the array is marked as unreadable.
If it the first codec, then it is parsed to find out the endianness of the array.
*/

/* Opaque */
struct H5Z_class2_t;
struct NCZ_codec_t;
struct NCPSharedLib;
struct NCZ_Plugin;
struct NCZ_Codec;
struct NCZ_HDF5;

typedef struct NCZ_Params {
    size_t nparams;
    unsigned* params;
} NCZ_Params;

/* HDF5 Info */
typedef struct NCZ_HDF5 {
    int id;           /**< HDF5 id corresponding to filterid. */
    NCZ_Params visible;
    NCZ_Params working;
} NCZ_HDF5;
extern struct NCZ_HDF5 NCZ_hdf5_empty(void);

/* Codec Info */
typedef struct NCZ_Codec {
    char* id;		/**< The NumCodecs ID */
    char* codec;	/**< The codec string from the file; NULL if creating */
} NCZ_Codec;
extern struct NCZ_Codec NCZ_codec_empty();

typedef struct NCZ_Filter {
    NCZ_HDF5 hdf5;
    NCZ_Codec codec;
    struct NCZ_Plugin* plugin;  /**< Implementation of this filter. */
    int incomplete;		/* If set, => filter has no complete matching plugin */
    int suppress;		/* if set, then this filter should not be executed */
    int invisible;		/* Do not report existence to user */
    int flags;             	/**< Flags describing state of this filter. */
#	define FLAG_VISIBLE	1 /* If set, then visible parameters are defined */
#	define FLAG_WORKING	2 /* If set, then WORKING parameters are defined */
#	define FLAG_NEWVISIBLE	16 /* If set, => visible parameters  were modified */
} NCZ_Filter;

int NCZ_filter_initialize(void);
int NCZ_filter_finalize(void);

int NCZ_addfilter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCREF filter);
int NCZ_plugin_lookup(const char* codecid, struct NCZ_Plugin** pluginp);

int NCZ_filter_verify(NCZ_Filter* filter, int varsized);
int NCZ_filter_setup(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
int NCZ_filter_freelists(NC_VAR_INFO_T* var);
int NCZ_codec_freelist(NCZ_VAR_INFO_T* zvar);
int NCZ_filter_free(NCZ_Filter* spec);
int NCZ_applyfilterchain(NC_FILE_INFO_T*, NC_VAR_INFO_T*, NClist* chain, size_t insize, void* indata, size_t* outlen, void** outdata, int encode);
int NCZ_filter_jsonize(NC_FILE_INFO_T*, NC_VAR_INFO_T*, struct NCZ_Filter* filter, struct NCjson**);
int NCZ_filter_build(NC_FILE_INFO_T*, NC_VAR_INFO_T* var, const NCjson* jfilter, int chainindex);
int NCZ_complete_filter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCZ_Filter* filter);
int NCZ_codec_attr(NC_VAR_INFO_T* var, size_t* lenp, void* data);
void ncz_hdf5_clear(NCZ_HDF5* h);
void ncz_codec_clear(NCZ_Codec* c);

#endif /*ZFILTER_H*/
