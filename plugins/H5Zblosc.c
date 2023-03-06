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

/*
HDF5 Blosc filter takes 7 parameters as follows:
param[0] -- FILTER_BLOSC_VERSION (ignored)
param[1] -- BLOSC_VERSION_FORMAT (ignored)
param[2] -- typesize in bytes
param[3] -- chunksize in bytes (ignored)
param[4] -- compression level
param[5] -- shuffle enabled
param[6] -- compressor to use
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

#undef BLOSC_DEBUG

#undef BLOSCCTX

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
  int i,ndims;
  unsigned int typesize, basetypesize;
  unsigned int bufsize;
  unsigned int flags = 0;
  size_t nelements = 7;
  unsigned int values[] = {0, 0, 0, 0, 0, 0, 0};
  hid_t super_type;
  H5T_class_t classt;
  hsize_t chunkdims[32];

  assert(sizeof(hid_t) == 8);
  
  if(H5Pget_filter_by_id1(dcpl, H5Z_FILTER_BLOSC, &flags, &nelements, values, 0, NULL) < 0) goto failed;

  assert(nelements == 7);

#ifdef BLOSC_DEBUG
  fprintf(stderr, "Blosc: initial parameters =(%u) [%u %u %u %u %u %u %u]\n",
		(unsigned)nelements,values[0],values[1],values[2],values[3],values[4],values[5],values[6]); fflush(stderr);
#endif

  /* Ignore first two slots */
  values[0] = 0; /* FILTER_BLOSC_VERSION*/
  values[1] = 0; /*BLOSC_VERSION_FORMAT*/

  ndims = H5Pget_chunk(dcpl, 32, chunkdims);
  if (ndims < 0) goto failed;
  if (ndims > 32) {
    fprintf(stderr,"Blosc Filter Error: blosc_set_local: Chunk rank exceeds limit\n");
    goto failed;
  }

  typesize = H5Tget_size(type);
  if (typesize == 0) goto failed;
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

  {
      /* Get the size of the chunk */
      bufsize = typesize;
#ifdef BLOSC_DEBUG
      fprintf(stderr,">>> set_local: |dims|=%d dims=(",(int)ndims);
      for (i = 0; i < ndims; i++) {
	fprintf(stderr,"%s%u",(i==0?"":","),(unsigned)chunkdims[i]);
      }
      fprintf(stderr,")\n");
#endif
      for (i = 0; i < ndims; i++) {
        bufsize *= chunkdims[i];
      }
      values[3] = bufsize;
  }
#ifdef BLOSC_DEBUG
  fprintf(stderr, "Blosc: working parameters =(%u) [%u %u %u %u %u %u %u]\n",
		(unsigned)nelements,values[0],values[1],values[2],values[3],values[4],values[5],values[6]); fflush(stderr);
#endif

  if(H5Pmodify_filter(dcpl, H5Z_FILTER_BLOSC, flags, nelements, values) < 0) goto failed;

  return 1;

failed:
  return -1;
}

#endif /*USE_HDF5*/

/* The filter function */
static
size_t blosc_filter(unsigned flags, size_t cd_nelmts,
                    const unsigned cd_values[], size_t nbytes,
                    size_t* buf_size, void** buf)
{
  void* outbuf = NULL;
  size_t typesize;
  size_t outbuf_size;
  int clevel = 5;                /* Compression level default */
  int doshuffle = 1;             /* Shuffle default */
  int compcode;                  /* Blosc compressor */
  int code;
  const char* compname = "blosclz";    /* The compressor by default */
  int bloscsize = 0;

#ifdef BLOSC_DEBUG
  fprintf(stderr, "Blosc: compressor =(%u) [%u %u %u %u %u %u %u]\n",
		(unsigned)cd_nelmts,cd_values[0],cd_values[1],cd_values[2],cd_values[3],cd_values[4],cd_values[5],cd_values[6]); fflush(stderr);
#endif

  /* Filter params that are always set */
  typesize = cd_values[2];      /* The datatype size */
  /* Optional params */
  if (cd_nelmts >= 5) {
    clevel = cd_values[4];        /* The compression level */
  }
  if (cd_nelmts >= 6) {
    doshuffle = cd_values[5];  /* BLOSC_SHUFFLE, BLOSC_BITSHUFFLE */
  }
  if (cd_nelmts >= 7) {
    compcode = cd_values[6];     /* The Blosc compressor used */
    code = blosc_compcode_to_compname(compcode, &compname);
    if (code == -1) {
      fprintf(stderr,"Blosc Filter Error: this Blosc library does not have support for the '%s' compressor\n",compname);
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
    fprintf(stderr, "Blosc: Compress %zd chunk w/chunksize %zd\n",
		nbytes, outbuf_size);
#endif

    outbuf = malloc(outbuf_size);

    if (outbuf == NULL) {
      fprintf(stderr,"Blosc Filter Error: blosc_filter: Can't allocate compression buffer\n");
      goto failed;
    }

#ifdef BLOSCCTX
    bloscsize = blosc_compress_ctx(clevel, doshuffle, typesize, nbytes, *buf, outbut, nbytes,
                                compname, /*blocksize*/0, /*no. thredds*/0);
#else
    blosc_set_compressor(compname);
    bloscsize = blosc_compress(clevel, doshuffle, typesize, nbytes, *buf, outbuf, nbytes);
#endif
    if(bloscsize == 0) {
        fprintf(stderr,"Blosc_FIlter Error: blosc_filter: Buffer is uncompressible.\n");
	goto failed;
    } else if(bloscsize < 0) {
      fprintf(stderr,"Blosc Filter Error: blosc_filter: blosc compression error\n");
      goto failed;
    }

  } else { /* We're decompressing */
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
      fprintf(stderr,"Blosc Filter Error: blosc_filter: can't allocate decompression buffer\n");
      goto failed;
    }

#ifdef BLOSCCTX
    bloscsize = blosc_decompress_ctx(*buf, outbuf, outbuf_size, /*no. thredds*/0);
#else
    bloscsize = blosc_decompress(*buf, outbuf, outbuf_size);
#endif

    if(bloscsize <= 0) {    /* decompression failed */
      fprintf(stderr,"Blosc Filter Error: blosc_filter: blosc decompression error\n");
      goto failed;
    } /* if !status */

  } /* compressing vs decompressing */

  {
    free(*buf);
    *buf = outbuf;
    *buf_size = outbuf_size;
    return bloscsize;  /* Size of compressed/decompressed data */
  }

failed:
   free(outbuf);
   return 0;

} /* End filter function */


/* HDF5 Plugin Interface */
const H5Z_class2_t blosc_H5Filter[1] = {
    {
        H5Z_CLASS_T_VERS,
        (H5Z_filter_t)(H5Z_FILTER_BLOSC),
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
