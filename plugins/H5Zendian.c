/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */

/*
Author: Dennis Heimbigner
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "netcdf.h"
#include "netcdf_filter_build.h"

/* Local function prototypes */
#ifdef USE_HDF5
static herr_t H5Z_set_local_endian(hid_t dcpl_id, hid_t type_id, hid_t space_id);
#else
#define H5Z_set_local_endian NULL
#endif

static size_t H5Z_filter_endian(unsigned flags, size_t cd_nelmts,
    const unsigned cd_values[], size_t nbytes, size_t *buf_size, void **buf);

static int NCZ_endian_modify_parameters(const NCproplist* env, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);
static int NCZ_endian_codec_to_hdf5(const NCproplist* env, const char* codec_json, int* idp, size_t* vnparamsp, unsigned** vparamsp);
static int NCZ_endian_hdf5_to_codec(const NCproplist* env, int id, size_t vnparams, const unsigned* vparams, char** codecp);

/* This message derives from H5Z */
const H5Z_class2_t H5Z_ENDIAN[1] = {{
    H5Z_CLASS_T_VERS,           /* H5Z_class_t version */
    H5Z_FILTER_BYTES,		/* Filter id number		*/
    1,                          /* encoder_present flag (set to true) */
    1,                          /* decoder_present flag (set to true) */
    "bytes",			/* Filter name for debugging	*/
    NULL,                       /* The "can apply" callback     */
    H5Z_set_local_endian,	/* The "set local" callback     */
    H5Z_filter_endian,		/* The actual filter function	*/
}};

/* External Discovery Functions */
DLLEXPORT
H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

DLLEXPORT
const void*
H5PLget_plugin_info(void)
{
    return H5Z_ENDIAN;
}

/* Local macros */
#define H5Z_ENDIAN_PARM_SIZE      0       /* "Local" parameter for shuffling size */

#ifdef USE_HDF5
/*-------------------------------------------------------------------------
 * Function:	H5Z_set_local_endian
 *
 * Purpose:	Set the "local" dataset parameter for data shuffling to be
 *              the size of the datatype.
 *
 * Return:	Success: Non-negative
 *		Failure: Negative
 *
 * Programmer:	Quincey Koziol
 *              Monday, April  7, 2003
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5Z_set_local_endian(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    herr_t ret_value = SUCCEED; /* Return value */
    unsigned flags = 0;     /* Filter flags */
    size_t cd_nelmts = 1;   /* Number of filter parameters */
    unsigned cd_values[2];  /* Filter parameters */

    NC_UNUSED(space_id);

    FUNC_ENTER_STATIC

    /* Get the filter's current parameters */
    if(H5Pget_filter_by_id1(dcpl_id, H5Z_FILTER_BYTES, &flags, &cd_nelmts, cd_values, 0, NULL) < 0)
        HGOTO_ERROR(H5E_PLINE, H5E_CANTGET, FAIL, "can't get szip parameters")

    /* Get the variable type size */
    if((cd_values[1] = (unsigned)H5Tget_size(type_id)) == 0)
	HGOTO_ERROR(H5E_PLINE, H5E_BADTYPE, FAIL, "bad datatype size")

    /* Is this type a fixed-length string?*/
    {
	H5T_class_t class;
	htri_t is_str;
        hid_t native_typeid, hdf_typeid;
	hdf_typeid = H5Dget_type(type_id);
	native_typeid = H5Tget_native_type(hdf_typeid, H5T_DIR_DEFAULT);
	class = H5Tget_class(native_typeid);
	if(class == H5T_STRING) {
	    is_str = H5Tis_variable_str(native_typeid);
	    if(is_str || H5Tget_size(hdf_typeid) > 1) cd_values[1] = 1; /* avoid swapping */
	}
    }
	
    /* Modify the filter's parameters for this dataset */
    if(H5Pmodify_filter(dcpl_id, H5Z_FILTER_BYTES, flags, (size_t)2, cd_values) < 0)
	HGOTO_ERROR(H5E_PLINE, H5E_CANTSET, FAIL, "can't set local endian parameters")

done:
    return ret_value;
} /* end H5Z_set_local_endian() */
#endif

#define swapinline16(ip) \
{ \
    char b[2]; \
    char* src = (char*)(ip); \
    b[0] = src[1]; \
    b[1] = src[0]; \
    memcpy(ip, b, 2); \
}

#define swapinline32(ip) \
{ \
    char b[4]; \
    char* src = (char*)(ip); \
    b[0] = src[3]; \
    b[1] = src[2]; \
    b[2] = src[1]; \
    b[3] = src[0]; \
    memcpy(ip, b, 4); \
}

#define swapinline64(ip) \
{ \
    char b[8]; \
    char* src = (char*)(ip); \
    b[0] = src[7]; \
    b[1] = src[6]; \
    b[2] = src[5]; \
    b[3] = src[4]; \
    b[4] = src[3]; \
    b[5] = src[2]; \
    b[6] = src[1]; \
    b[7] = src[0]; \
    memcpy(ip, b, 8); \
}

static size_t
H5Z_filter_endian(unsigned flags, size_t cd_nelmts, const unsigned cd_values[],
                   size_t nbytes, size_t *buf_size, void **buf)
{
    herr_t ret_value = 0;       /* Return value */
    unsigned typesize;       /* Number of bytes per element */
    union { unsigned char bytes[SIZEOF_INT]; int i; } u; /* to test local endianness */
    int fileendian;    
    int nativeendian;
    int swap;

    NC_UNUSED(flags);

    /* Check arguments */
    if (cd_nelmts!=2)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "invalid bytes parameters")

    /* Determine native endianness */
    u.i = 1;
    if(u.bytes[0] == 1) nativeendian = NC_ENDIAN_LITTLE; else nativeendian = NC_ENDIAN_BIG;

    /* Determine file endianness */
    if(cd_values[0] == 1) fileendian = NC_ENDIAN_LITTLE; else fileendian = NC_ENDIAN_BIG;

    /* Decide if we need to swap */
    swap = (nativeendian != fileendian);

    /* Get the number of bytes per element from the parameter block */
    typesize=cd_values[1];

    /* Check that the buffer is a multiple of the type size */
    if(nbytes % typesize)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "invalid bytes parameters")

    /* Don't do anything for 1-byte types or if !swap */
    if(swap && typesize > 1) {
	size_t i;
	char* src = (char*)(*buf);
	for(i=0;i<nbytes;i+=typesize) {
	    char* p = src + i;
	    switch (typesize) {
	    case 2: swapinline16(p); break;
	    case 4: swapinline32(p); break;
	    case 8: swapinline64(p); break;
	    default: break;
	    }
	}
	if(*buf_size != nbytes)
	    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, 0, "internal error")
    } /* swap && typesize > 1 */

    /* Set the return value */
    ret_value = (int)nbytes;

done:
    return (size_t)ret_value;
}

/**************************************************/
/* Provide the codec support for endian filter */


static NCZ_codec_t NCZ_endian_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,		/* Struct version number */
  NCZ_CODEC_HDF5,		/* Struct sort */
  H5Z_CODEC_BYTES,		/* Standard name/id of the codec */
  H5Z_FILTER_BYTES,		/* HDF5 alias for endian */
  NULL,				/* NCZ_endian_codec_initialize*/
  NULL,				/* NCZ_endian_codec_finalize*/
  NCZ_endian_codec_to_hdf5,
  NCZ_endian_hdf5_to_codec,
  NCZ_endian_modify_parameters, /*NCZ_endian_modify_parameters*/
};

/* External Export API */
DLLEXPORT
const void*
NCZ_get_codec_info(void)
{
    return (void*)&NCZ_endian_codec;
}

/* NCZarr Interface Functions */

/* Create the true parameter set:
Visible parameters:
param[0] -- 1 => little endian, 2 => big endian
param[1] -- type size: 2|4|8

Working parameters:
param[0] -- 1 => little endian, 2 => big endian
param[1] -- type size: 2|4|8
*/

static int
NCZ_endian_modify_parameters(const NCproplist* env, int* idp, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int stat = NC_NOERR;
    unsigned* wparams = NULL;
    size_t wnparams;
    size_t vnparams = *vnparamsp;
    unsigned* vparams = *vparamsp;
    uintptr_t ncid, varid;
    nc_type xtype;
    size_t typesize;
    
    NC_UNUSED(idp);

    if(vnparams < 0)
        {stat = NC_EFILTER; goto done;}

    wnparams = 2;

    if(vparams == NULL)
        {stat = NC_EFILTER; goto done;}

    if(wnparamsp == NULL || wparamsp == NULL)
        {stat = NC_EFILTER; goto done;}

    ncproplistget(env,"ncid",&ncid,NULL);
    ncproplistget(env,"varid",&varid,NULL);

    vnparams = *vnparamsp;
    vparams = *vparamsp;

    /* Get variable type and then type size */
    if((stat = nc_inq_vartype((int)ncid,(int)varid,&xtype))) goto done;
    if(xtype >= NC_FIRSTUSERTYPEID) {stat = NC_EINVAL; goto done;}
    if((stat = nc_inq_type((int)ncid,xtype,NULL,&typesize))) goto done;

    /* oops, NC_STRING is special */
    if(xtype == NC_STRING) typesize = 1; /* to avoid swapping */

    /* Create the working parameters */
    if((wparams = (unsigned*)malloc(wnparams*sizeof(unsigned)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    memcpy(wparams,vparams,vnparams*sizeof(unsigned));

    /* Add the type size */
    wparams[1] = (unsigned)typesize;

    *wnparamsp = wnparams;
    nullfree(*wparamsp);
    *wparamsp = wparams; wparams = NULL;
    
done:
    nullfree(wparams);
    FUNC_LEAVE_NOAPI(stat)
}

static int
NCZ_endian_codec_to_hdf5(const NCproplist* env, const char* codec_json, int* idp, size_t* vnparamsp, unsigned** vparamsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    const NCjson* jdict = NULL;
    const NCjson* jtmp = NULL;
    unsigned* vparams = NULL;
    uintptr_t zarrformat = 0;

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec)<0) {stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}

    if(zarrformat == 3) {
        if(NCJdictget(jcodec,"name",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
	if(NCJdictget(jcodec,"configuration",(NCjson**)&jdict)<0) {stat = NC_EFILTER; goto done;}
    } else {
	jdict = jcodec;
        if(NCJdictget(jdict,"id",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
    }

    /* Verify the codec ID */
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_endian_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    if((vparams = (unsigned*)calloc(1,sizeof(unsigned)))==NULL) {stat = NC_ENOMEM; goto done;}

    /* Get endianess */
    if(NCJdictget(jdict,"endian",(NCjson**)&jtmp)<0) {stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || NCJsort(jtmp)!=NCJ_STRING) {stat = NC_EFILTER; goto done;}
    vparams[0] = (strcmp(NCJstring(jtmp),"little")==0?NC_ENDIAN_LITTLE:NC_ENDIAN_BIG);

    if(vnparamsp) *vnparamsp = 1;
    if(vparamsp) {*vparamsp = vparams; vparams = NULL;}
    if(idp) *idp = H5Z_FILTER_BYTES;
    
done:
    if(vparams) free(vparams);
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_endian_hdf5_to_codec(const NCproplist* env, int id, size_t vnparams, const unsigned* vparams, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];
    uintptr_t zarrformat;

    NC_UNUSED(id);

    if(vnparams == 0 || vparams == NULL) {stat = NC_EINVAL; goto done;}

    ncproplistget(env,"zarrformat",&zarrformat,NULL);

    if(zarrformat == 3)
        snprintf(json,sizeof(json),
	    "{\"name\": \"bytes\", \"configuration\": {\"endian\": \"%s\"}}",
	    (vparams[0]==NC_ENDIAN_LITTLE?"little":"big"));
    else
	snprintf(json,sizeof(json),
	    "{\"id\": \"bytes\",\"endian\": \"%s\"}",
	    (vparams[0]==NC_ENDIAN_LITTLE?"little":"big"));
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
