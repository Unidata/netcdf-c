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
#include <assert.h>

#include <szlib.h>
#include "H5Zszip.h"

#include "netcdf.h"
#include "netcdf_filter.h"
#include "netcdf_filter_build.h"
#include "netcdf_json.h"

#define H5Z_FILTER_SZIP         4       /*szip compression              */

/**************************************************/
/* NCZarr Filter Objects */

static int NCZ_szip_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_szip_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);
static int NCZ_szip_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

static NCZ_codec_t NCZ_szip_codec = {
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "szip",	        /* Standard name/id of the codec */
  H5Z_FILTER_SZIP,   /* HDF5 alias for szip */
  NULL, /*NCZ_szip_codec_initialize*/
  NULL, /*NCZ_szip_codec_finalize*/
  NCZ_szip_codec_to_hdf5,
  NCZ_szip_hdf5_to_codec,
  NCZ_szip_modify_parameters,
};

static int
NCZ_szip_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    unsigned* params = NULL;
    size_t nparams = 2; /* No. of visible parameters */
    NCjson* json = NULL;
    NCjson* jtmp = NULL;
    struct NCJconst jc = {0,0,0,NULL};
    
    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EINTERNAL; goto done;}

    if((params = (unsigned*)calloc(nparams,sizeof(unsigned)))== NULL)
        {stat = NC_ENOMEM; goto done;}

    if(NCJparse(codec_json,0,&json))
        {stat = NC_EFILTER; goto done;}

    if(NCJdictget(json,"mask",&jtmp) || jtmp == NULL)
        {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc))
        {stat = NC_EFILTER;  goto done;}
    params[H5Z_SZIP_PARM_MASK] = (unsigned)jc.ival;

    jtmp = NULL;
    if(NCJdictget(json,"pixels-per-block",&jtmp) || jtmp == NULL)
        {stat = NC_EFILTER; goto done;}
    if(NCJcvt(jtmp,NCJ_INT,&jc))
        {stat = NC_EFILTER;  goto done;}
    params[H5Z_SZIP_PARM_PPB] = (unsigned)jc.ival;

    *nparamsp = nparams;
    *paramsp = params; params = NULL;
    
done:
    NCJreclaim(json);
    nullfree(params);
    return stat;
}

static int
NCZ_szip_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[2048];

    snprintf(json,sizeof(json),"{\"id\": \"%s\", \"mask\": %u, \"pixels-per-block\": %u}",
    		NCZ_szip_codec.codecid,
		params[H5Z_SZIP_PARM_MASK],
		params[H5Z_SZIP_PARM_PPB]);
    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}

static int
NCZ_szip_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int i,ret_value = NC_NOERR;
    nc_type vtype;
    size_t typesize, scanline, dtype_precision, npoints;
    int ndims, storage, dtype_order;
    int dimids[NC_MAX_VAR_DIMS];
    char vname[NC_MAX_NAME+1];
    size_t chunklens[NC_MAX_VAR_DIMS];
    unsigned* params = NULL;
    unsigned* vparams = NULL;
    size_t wnparams = 4;
    
    if(wnparamsp == NULL || wparamsp == NULL)
        {ret_value = NC_EFILTER; goto done;}
    if(vnparamsp == NULL || vparamsp == NULL)
        {ret_value = NC_EFILTER; goto done;}
    if(*vnparamsp > 0 && *vparamsp == NULL)
        {ret_value = NC_EFILTER; goto done;}

    vparams = *vparamsp;

    /* Get variable info */
    if((ret_value = nc_inq_var(ncid,varid,vname,&vtype,&ndims,dimids,NULL))) goto done;

    /* Get the typesize */
    if((ret_value = nc_inq_type(ncid,vtype,NULL,&typesize))) goto done;

    /* Get datatype's precision, in case is less than full bits  */
    dtype_precision = typesize*8;

    if(dtype_precision > 24) {
        if(dtype_precision <= 32)
            dtype_precision = 32;
        else if(dtype_precision <= 64)
            dtype_precision = 64;
    } /* end if */

    if(ndims == 0) {ret_value = NC_EFILTER; goto done;}

    /* Set "local" parameter for this dataset's "pixels-per-scanline" */
    if((ret_value = nc_inq_dimlen(ncid,dimids[ndims-1],&scanline))) goto done;

    /* Get number of elements for the dataspace;  use
       total number of elements in the chunk to define the new 'scanline' size */
    /* Compute chunksize */
    if((ret_value = nc_inq_var_chunking(ncid,varid,&storage,chunklens))) goto done;
    if(storage != NC_CHUNKED) {ret_value = NC_EFILTER; goto done;}
    npoints = 1;
    for(i=0;i<ndims;i++) npoints *= chunklens[i];

    /* Get datatype's endianness order */
    if((ret_value = nc_inq_var_endian(ncid,varid,&dtype_order))) goto done;

    if((params = (unsigned*)malloc(wnparams*sizeof(unsigned)))==NULL)
        {ret_value = NC_ENOMEM; goto done;}
    params[H5Z_SZIP_PARM_MASK] = vparams[H5Z_SZIP_PARM_MASK];
    params[H5Z_SZIP_PARM_PPB] = vparams[H5Z_SZIP_PARM_PPB];

    /* Set "local" parameter for this dataset's "bits-per-pixel" */
    params[H5Z_SZIP_PARM_BPP] = dtype_precision;

    /* Adjust scanline if it is smaller than number of pixels per block or
       if it is bigger than maximum pixels per scanline, or there are more than
       SZ_MAX_BLOCKS_PER_SCANLINE blocks per scanline  */
    if(scanline < vparams[H5Z_SZIP_PARM_PPB]) {
        if(npoints < vparams[H5Z_SZIP_PARM_PPB])
	    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "pixels per block greater than total number of elements in the chunk")
	scanline = MIN((vparams[H5Z_SZIP_PARM_PPB] * SZ_MAX_BLOCKS_PER_SCANLINE), npoints);
    } else {
        if(scanline <= SZ_MAX_PIXELS_PER_SCANLINE)
            scanline = MIN((vparams[H5Z_SZIP_PARM_PPB] * SZ_MAX_BLOCKS_PER_SCANLINE), scanline);
        else
            scanline = vparams[H5Z_SZIP_PARM_PPB] * SZ_MAX_BLOCKS_PER_SCANLINE;
    } /* end else */
    /* Assign the final value to the scanline */
    params[H5Z_SZIP_PARM_PPS] = (unsigned)scanline;

    /* Set the correct mask flags */

    /* From H5Pdcpl.c#H5Pset_szip */
    params[H5Z_SZIP_PARM_MASK] &= (unsigned)(~H5_SZIP_CHIP_OPTION_MASK);
    params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_ALLOW_K13_OPTION_MASK;
    params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_RAW_OPTION_MASK;
    params[H5Z_SZIP_PARM_MASK] &= (unsigned)(~(H5_SZIP_LSB_OPTION_MASK | H5_SZIP_MSB_OPTION_MASK));

    /* From H5Zszip.c#H5Z__set_local_szip */
    params[H5Z_SZIP_PARM_MASK] &= (unsigned)(~(H5_SZIP_LSB_OPTION_MASK | H5_SZIP_MSB_OPTION_MASK));
    switch(dtype_order) {
    case NC_ENDIAN_LITTLE:      /* Little-endian byte order */
        params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_LSB_OPTION_MASK;
        break;
    case NC_ENDIAN_BIG:      /* Big-endian byte order */
        params[H5Z_SZIP_PARM_MASK] |= H5_SZIP_MSB_OPTION_MASK;
        break;
    default:
        HGOTO_ERROR(H5E_PLINE, H5E_BADTYPE, FAIL, "bad datatype endianness order")
    } /* end switch */

    *wnparamsp = wnparams;
    nullfree(*wparamsp);
    *wparamsp = params; params = NULL;
    
done:
    nullfree(params);
    FUNC_LEAVE_NOAPI(ret_value)
}

#if 0
static int
NCZ_szip_visible_parameters(int ncid, int varid, size_t nparamsin, const unsigned int* paramsin, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    unsigned* params = NULL;

    if(nparamsp == NULL || paramsp == NULL)
        {stat = NC_EFILTER; goto done;}
    
    if((params = (unsigned*)malloc(2*sizeof(unsigned)))==NULL)
        {stat = NC_ENOMEM; goto done;}

    params[H5Z_SZIP_PARM_MASK] = paramsin[H5Z_SZIP_PARM_MASK];
    params[H5Z_SZIP_PARM_PPB] = paramsin[H5Z_SZIP_PARM_PPB];

    nullfree(*paramsp);
    *paramsp = params; params = NULL;

done:
    nullfree(params);
    return stat;
}
#endif

/**************************************************/

NCZ_codec_t* NCZ_szip_codecs[] = {
&NCZ_szip_codec,
NULL
};

/* External Export API */
DLLEXPORT
const void*
NCZ_codec_info_defaults(void)
{
    return (void*)&NCZ_szip_codecs;
}

