/*
Blosc - A blocking, shuffling and lossless compression library

Copyright (C) 2009-2015 Francesc Alted <francesc@blosc.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
Blosc for HDF5 - An HDF5 filter that uses the Blosc compressor.

Copyright (C) 2009-2015 Francesc Alted <francesc@blosc.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
Copyright Notice and Statement for the h5py Project

Copyright (c) 2008 Andrew Collette
http://h5py.alfven.org
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

a. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

b. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

c. Neither the name of the author nor the names of contributors may 
   be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
    Copyright (C) 2010-2016  Francesc Alted
    http://blosc.org
    License: MIT (see LICENSE.txt)

    Filter program that allows the use of the Blosc filter in HDF5.

    This is based on the LZF filter interface (http://h5py.alfven.org)
    by Andrew Collette.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "netcdf_filter_build.h"
#include <netcdf_json.h>

#include "H5Zblosc.h"

#ifdef USE_HDF5
#include <H5Epubgen.h>
#endif

#define BLOSCCTX

#ifdef USE_HDF5
#if defined(__GNUC__)
#define PUSH_ERR(func, minor, str, ...) H5Epush1(__FILE__, func, __LINE__, H5_VERS_MAJOR, minor, str)
#elif defined(_MSC_VER)
#define PUSH_ERR(func, minor, str, ...) H5Epush1(__FILE__, func, __LINE__, H5_VERS_MAJOR, minor, str)
#else
/* This version is portable but it's better to use compiler-supported
   approaches for handling the trailing comma issue when possible. */
#define PUSH_ERR(func, minor, str, ...) H5Epush1(__FILE__, func, __LINE__, H5_VERS_MAJOR, minor, str
)
#endif	/* defined(__GNUC__) */

#define GET_FILTER(a, b, c, d, e, f, g) H5Pget_filter_by_id1(a,b,c,d,e,f,g)

#else /*!USE_HDF5*/
#define PUSH_ERR(f,m,s,...) fprintf(stderr,"%s\n",s)
#endif /*USE_HDF5*/

static int h5z_blosc_initialized = 0;

static size_t blosc_filter(unsigned flags, size_t cd_nelmts,
                    const unsigned cd_values[], size_t nbytes,
                    size_t* buf_size, void** buf);

#ifndef USE_HDF5
#define blosc_set_local NULL
#else
static herr_t blosc_set_local(hid_t dcpl, hid_t type, hid_t space);

/*  Filter setup.  Records the following inside the DCPL:

    1. If version information is not present, set slots 0 and 1 to the filter
       revision and Blosc version, respectively.

    2. Compute the type size in bytes and store it in slot 2.

    3. Compute the chunk size in bytes and store it in slot 3.
*/

static
herr_t blosc_set_local(hid_t dcpl, hid_t type, hid_t space)
{
  int ndims;
  int i;
  herr_t r;

  unsigned int typesize, basetypesize;
  unsigned int bufsize;
  hsize_t chunkdims[32];
  unsigned int flags;
  size_t nelements = 8;
  unsigned int values[] = {0, 0, 0, 0, 0, 0, 0, 0};
  hid_t super_type;
  H5T_class_t classt;

  r = GET_FILTER(dcpl, FILTER_BLOSC, &flags, &nelements, values, 0, NULL);
  if (r < 0) return -1;

  if (nelements < 4) nelements = 4;  /* First 4 slots reserved. */

  /* Set Blosc info in first two slots */
  values[0] = FILTER_BLOSC_VERSION;
  values[1] = BLOSC_VERSION_FORMAT;

  ndims = H5Pget_chunk(dcpl, 32, chunkdims);
  if (ndims < 0) return -1;
  if (ndims > 32) {
    PUSH_ERR("blosc_set_local", H5E_CALLBACK, "Chunk rank exceeds limit");
    return -1;
  }

  typesize = H5Tget_size(type);
  if (typesize == 0) return -1;
  /* Get the size of the base type, even for ARRAY types */
  classt = H5Tget_class(type);
  if (classt == H5T_ARRAY) {
    /* Get the array base component */
    super_type = H5Tget_super(type);
    basetypesize = H5Tget_size(super_type);
    /* Release resources */
    H5Tclose(super_type);
  } else {
    basetypesize = typesize;
  }

  /* Limit large typesizes (they are pretty expensive to shuffle
     and, in addition, Blosc does not handle typesizes larger than
     256 bytes). */
  if (basetypesize > BLOSC_MAX_TYPESIZE) basetypesize = 1;
  values[2] = basetypesize;

  /* Get the size of the chunk */
  bufsize = typesize;
  for (i = 0; i < ndims; i++) {
    bufsize *= chunkdims[i];
  }
  values[3] = bufsize;

#ifdef BLOSC_DEBUG
  fprintf(stderr, "Blosc: Computed buffer size %d\n", bufsize);
#endif

  r = H5Pmodify_filter(dcpl, FILTER_BLOSC, flags, nelements, values);
  if (r < 0) return -1;

  return 1;
}

#endif /*USE_HDF5*/

/* The filter function */
static
size_t blosc_filter(unsigned flags, size_t cd_nelmts,
                    const unsigned cd_values[], size_t nbytes,
                    size_t* buf_size, void** buf)
{
  void* outbuf = NULL;
  int status = 0;                /* Return code from Blosc routines */
  size_t typesize;
  size_t outbuf_size;
  int clevel = 5;                /* Compression level default */
  int doshuffle = 1;             /* Shuffle default */
  int compcode;                  /* Blosc compressor */
  int code;
  const char* compname = "blosclz";    /* The compressor by default */
  const char* complist = NULL;

  /* Filter params that are always set */
  typesize = cd_values[2];      /* The datatype size */
  outbuf_size = cd_values[3];   /* Precomputed buffer guess */
  /* Optional params */
  if (cd_nelmts >= 5) {
    clevel = cd_values[4];        /* The compression level */
  }
  if (cd_nelmts >= 6) {
    doshuffle = cd_values[5];  /* BLOSC_SHUFFLE, BLOSC_BITSHUFFLE */
    /* bitshuffle is only meant for production in >= 1.8.0 */
#if ((BLOSC_VERSION_MAJOR <= 1) && (BLOSC_VERSION_MINOR < 8))
    if (doshuffle == BLOSC_BITSHUFFLE) {
      PUSH_ERR("blosc_filter", H5E_CALLBACK,
               "this Blosc library version is not supported.  Please update to >= 1.8");
      goto failed;
    }
#endif
  }
  if (cd_nelmts >= 7) {
    compcode = cd_values[6];     /* The Blosc compressor used */
    /* Check that we actually have support for the compressor code */
    complist = blosc_list_compressors();
    code = blosc_compcode_to_compname(compcode, &compname);
    if (code == -1) {
      char s[4096];
      snprintf(s,sizeof(s),"this Blosc library does not have support for "
                 "the '%s' compressor, but only for: %s",
               compname, complist);
      PUSH_ERR("blosc_filter", H5E_CALLBACK,s);
      goto failed;
    }
  }

  /* We're compressing */
  if (!(flags & H5Z_FLAG_REVERSE)) {

    /* Allocate an output buffer exactly as long as the input data; if
       the result is larger, we simply return 0.  The filter is flagged
       as optional, so HDF5 marks the chunk as uncompressed and
       proceeds.
    */

    outbuf_size = (*buf_size);

#ifdef BLOSC_DEBUG
    fprintf(stderr, "Blosc: Compress %zd chunk w/buffer %zd\n",
    nbytes, outbuf_size);
#endif

    outbuf = malloc(outbuf_size);

    if (outbuf == NULL) {
      PUSH_ERR("blosc_filter", H5E_CALLBACK,
               "Can't allocate compression buffer");
      goto failed;
    }

#ifdef BLOSCCTX
    blosc_set_compressor(compname);
    status = blosc_compress(clevel, doshuffle, typesize, nbytes, *buf, outbuf, nbytes);
#else
    status = blosc_compress_ctx(clevel, doshuffle, typesize, nbytes, *buf, outbut, nbytes,
                                compname, /*blocksize*/0, /*no. thredds*/0);
#endif

    if (status < 0) {
      PUSH_ERR("blosc_filter", H5E_CALLBACK, "Blosc compression error");
      goto failed;
    }

    /* We're decompressing */
  } else {
    /* declare dummy variables */
    size_t cbytes, blocksize;

    if(outbuf) {
        free(outbuf);
	outbuf = NULL;
    }

    /* Extract the exact outbuf_size from the buffer header.
     *
     * NOTE: the guess value got from "cd_values" corresponds to the
     * uncompressed chunk size but it should not be used in a general
     * cases since other filters in the pipeline can modify the buffere
     *  size.
     */
    blosc_cbuffer_sizes(*buf, &outbuf_size, &cbytes, &blocksize);

#ifdef BLOSC_DEBUG
    fprintf(stderr, "Blosc: Decompress %zd chunk w/buffer %zd\n", nbytes, outbuf_size);
#endif

    outbuf = malloc(outbuf_size);

    if (outbuf == NULL) {
      PUSH_ERR("blosc_filter", H5E_CALLBACK, "Can't allocate decompression buffer");
      goto failed;
    }

#ifdef BLOSCCTX
    status = blosc_decompress(*buf, outbuf, outbuf_size);
#else
    status = blosc_decompress_ctx(*buf, outbuf, outbuf_size, /*no. thredds*/0);
#endif

    if (status <= 0) {    /* decompression failed */
      PUSH_ERR("blosc_filter", H5E_CALLBACK, "Blosc decompression error");
      goto failed;
    } /* if !status */

  } /* compressing vs decompressing */

  if (status != 0) {
    free(*buf);
    *buf = outbuf;
    *buf_size = outbuf_size;
    return status;  /* Size of compressed/decompressed data */
  }

  failed:
   free(outbuf);
   return 0;

} /* End filter function */


/* HDF5 Plugin Interface */
const H5Z_class2_t blosc_H5Filter[1] = {
    {
        H5Z_CLASS_T_VERS,
        (H5Z_filter_t)(FILTER_BLOSC),
        1,                   /* encoder_present flag (set to true) */
        1,                   /* decoder_present flag (set to true) */
        "blosc",
        /* Filter info  */
        NULL,                           /* The "can apply" callback */
        (H5Z_set_local_func_t)(blosc_set_local), /* The "set local" callback */
        (H5Z_func_t)(blosc_filter),    /* The filter function */
    }
};


H5PL_type_t H5PLget_plugin_type(void) { return H5PL_TYPE_FILTER; }
const void* H5PLget_plugin_info(void) { return blosc_H5Filter; }


/* Provide the codec support for the HDF5 blosc library */

/* NCZarr Filter Objects */

#define DEFAULT_LEVEL		9
#define DEFAULT_BLOCKSIZE	1
#define DEFAULT_TYPESIZE	1
#define DEFAULT_COMPCODE	BLOSC_LZ4

/* Forward */
static void NCZ_blosc_codec_finalize(void);
static int NCZ_blosc_codec_to_hdf5(const char* codec, size_t* nparamsp, unsigned** paramsp);
static int NCZ_blosc_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp);
static int NCZ_blosc_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);

/* Structure for NCZ_PLUGIN_CODEC */
static NCZ_codec_t NCZ_blosc_codec = {/* NCZ_codec_t  codec fields */ 
  NCZ_CODEC_CLASS_VER,	/* Struct version number */
  NCZ_CODEC_HDF5,	/* Struct sort */
  "blosc",	        /* Standard name/id of the codec */
  FILTER_BLOSC,	        /* HDF5 alias for blosc */
  NULL, /*NCZ_blosc_codec_initialize*/
  NCZ_blosc_codec_finalize,
  NCZ_blosc_codec_to_hdf5,
  NCZ_blosc_hdf5_to_codec,
  NCZ_blosc_modify_parameters,
};

/* External Export API */
const void*
NCZ_get_codec_info(void)
{
    if(!h5z_blosc_initialized) {
	h5z_blosc_initialized = 1;
	blosc_init();
    }
    return (void*)&NCZ_blosc_codec;
}

/* NCZarr Interface Functions */

/* Create the true parameter set:
Visible parameters:
param[0] -- reserved
param[1] -- reserved
param[2] -- reserved
param[3] -- variable chunksize in bytes | 0 (=>default)
param[4] -- compression level
param[5] -- BLOSC_SHUFFLE|BLOSC_BITSHUFFLE
param[6] -- compressor index

Working parameters:
param[0] -- filter revision
param[1] -- blosc version
param[2] -- variable type size in bytes
param[3] -- variable chunksize in bytes
param[4] -- compression level
param[5] -- BLOSC_SHUFFLE|BLOSC_BITSHUFFLE
param[6] -- compressor index
*/

static void
NCZ_blosc_codec_finalize(void)
{
    if(h5z_blosc_initialized) {
        blosc_destroy();
	h5z_blosc_initialized = 0;
    }
}

static int
NCZ_blosc_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp)
{
    int i,stat = NC_NOERR;
    nc_type vtype;
    int storage, ndims;
    size_t* chunklens = NULL;
    size_t typesize, chunksize;
    char vname[NC_MAX_NAME+1];
    unsigned* params = NULL;
    size_t nparams;
    size_t vnparams = *vnparamsp;
    unsigned* vparams = *vparamsp;
    
    if(vnparams < 7)
        {stat = NC_EFILTER; goto done;}
    nparams = 7;

    if(vnparams > 0 && vparams == NULL)
        {stat = NC_EFILTER; goto done;}

    if(wnparamsp == NULL || wparamsp == NULL)
        {stat = NC_EFILTER; goto done;}

    vnparams = *vnparamsp;
    vparams = *vparamsp;

    /* Get variable info */
    if((stat = nc_inq_var(ncid,varid,vname,&vtype,&ndims,NULL,NULL))) goto done;

    if(ndims == 0) {stat = NC_EFILTER; goto done;}

    /* Get the typesize */
    if((stat = nc_inq_type(ncid,vtype,NULL,&typesize))) goto done;

    /* Compute chunksize */
    if((chunklens = (size_t*)calloc(ndims,sizeof(size_t)))==NULL) goto done;
    if((stat = nc_inq_var_chunking(ncid,varid,&storage,chunklens))) goto done;
    if(storage != NC_CHUNKED) {stat = NC_EFILTER; goto done;}
    chunksize = typesize;
    for(i=0;i<ndims;i++) chunksize *= chunklens[i];

    if((params = (unsigned*)malloc(vnparams*sizeof(unsigned)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    memcpy(params,vparams,vnparams*sizeof(unsigned));

    params[0] = FILTER_BLOSC_VERSION;
    params[1] = BLOSC_VERSION_FORMAT;
    params[2] = (unsigned)typesize;

    params[3] = chunksize;
    params[4] = params[4];
    params[5] = params[5];
    params[6] = params[6];

    *wnparamsp = nparams;
    nullfree(*wparamsp);
    *wparamsp = params; params = NULL;
    
done:
    nullfree(chunklens);
    nullfree(params);
    FUNC_LEAVE_NOAPI(stat)
}

static int
NCZ_blosc_codec_to_hdf5(const char* codec_json, size_t* nparamsp, unsigned** paramsp)
{
    int stat = NC_NOERR;
    NCjson* jcodec = NULL;
    NCjson* jtmp = NULL;
    unsigned* params = NULL;
    struct NCJconst jc = {0,0,0,NULL};
    int compcode;

    /* parse the JSON */
    if(NCJparse(codec_json,0,&jcodec)) {stat = NC_EFILTER; goto done;}
    if(NCJsort(jcodec) != NCJ_DICT) {stat = NC_EPLUGIN; goto done;}
    /* Verify the codec ID */
    if(NCJdictget(jcodec,"id",&jtmp))
        {stat = NC_EFILTER; goto done;}
    if(jtmp == NULL || !NCJisatomic(jtmp)) {stat = NC_EINVAL; goto done;}
    if(strcmp(NCJstring(jtmp),NCZ_blosc_codec.codecid)!=0) {stat = NC_EINVAL; goto done;}

    if((params = (unsigned*)calloc(7,sizeof(unsigned)))==NULL) {stat = NC_ENOMEM; goto done;}

    /* Get compression level*/
    if(NCJdictget(jcodec,"clevel",&jtmp)) {stat = NC_EFILTER; goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_INT,&jc)) {stat = NC_EFILTER;  goto done;}
    } else
        jc.ival = DEFAULT_LEVEL;
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EFILTER; goto done;}
    params[4] = (unsigned)jc.ival;

    /* Get blocksize */
    if(NCJdictget(jcodec,"blocksize",&jtmp)) {stat = NC_EFILTER;  goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_INT,&jc)) {stat = NC_EFILTER; goto done;}
    } else
        jc.ival = DEFAULT_BLOCKSIZE;
    if(jc.ival < 0 || jc.ival > NC_MAX_UINT) {stat = NC_EFILTER; goto done;}
    params[3] = (unsigned)jc.ival;

    /* Get shuffle */
    if(NCJdictget(jcodec,"shuffle",&jtmp)) {stat = NC_EFILTER; goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_INT,&jc)) {stat = NC_EFILTER; goto done;}
    } else
        jc.ival = BLOSC_NOSHUFFLE;
    params[5] = (unsigned)jc.ival;

    /* Get compname */
    if(NCJdictget(jcodec,"cname",&jtmp)) {stat = NC_EFILTER;  goto done;}
    if(jtmp) {
        if(NCJcvt(jtmp,NCJ_STRING,&jc)) {stat = NC_EFILTER; goto done;}
        if(jc.sval == NULL || strlen(jc.sval) == 0) {stat = NC_EFILTER; goto done;}
        if((compcode = blosc_compname_to_compcode(jc.sval)) < 0) {stat = NC_EFILTER; goto done;}
    } else
        compcode = DEFAULT_COMPCODE;
    params[6] = (unsigned)compcode;

    if(nparamsp) *nparamsp = 7;
    if(paramsp) {*paramsp = params; params = NULL;}
    
done:
    if(jc.sval) {
	free(jc.sval);
    }
    if(params) {
        free(params);
    }
    NCJreclaim(jcodec);
    return stat;
}

static int
NCZ_blosc_hdf5_to_codec(size_t nparams, const unsigned* params, char** codecp)
{
    int stat = NC_NOERR;
    char json[1024];
    const char* compname = NULL;

    if(nparams == 0 || params == NULL)
        {stat = NC_EINVAL; goto done;}

    /* Get the sub-compressor name */
    if(blosc_compcode_to_compname((int)params[6],&compname) < 0) {stat = NC_EFILTER; goto done;}

    snprintf(json,sizeof(json),
	"{\"id\": \"blosc\",\"clevel\": %u,\"blocksize\": %u,\"cname\": \"%s\",\"shuffle\": %d}",
	params[4],params[3],compname,params[5]);

    if(codecp) {
        if((*codecp = strdup(json))==NULL) {stat = NC_ENOMEM; goto done;}
    }
    
done:
    return stat;
}
