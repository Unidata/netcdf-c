/*
 * Copyright 2018, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"
#include "netcdf_filter_build.h"
#include "h5misc.h"

/*
Common utilities related to filters.
Taken from libdispatch/dfilters.c.
*/

#ifdef WORDS_BIGENDIAN
/* Byte swap an 8-byte integer in place */
static void
byteswap8(unsigned char* mem)
{
    unsigned char c;
    c = mem[0];
    mem[0] = mem[7];
    mem[7] = c;
    c = mem[1];
    mem[1] = mem[6];
    mem[6] = c;
    c = mem[2];
    mem[2] = mem[5];
    mem[5] = c;
    c = mem[3];
    mem[3] = mem[4];
    mem[4] = c;
}

/* Byte swap an 8-byte integer in place */
static void
byteswap4(unsigned char* mem)
{
    unsigned char c;
    c = mem[0];
    mem[0] = mem[3];
    mem[3] = c;
    c = mem[1];
    mem[1] = mem[2];
    mem[2] = c;
}
#endif /*WORDS_BIGENDIAN*/

void
NC_h5filterspec_fix8(void* mem0, int decode)
{
    NC_UNUSED(mem0);
    NC_UNUSED(decode);    
#ifdef WORDS_BIGENDIAN
    unsigned char* mem = mem0;
    if(decode) { /* Apply inverse of the encode case */
	byteswap4(mem); /* step 1: byte-swap each piece */
	byteswap4(mem+4);
	byteswap8(mem); /* step 2: convert to little endian format */
    } else { /* encode */
	byteswap8(mem); /* step 1: convert to little endian format */
	byteswap4(mem); /* step 2: byte-swap each piece */
	byteswap4(mem+4);
    }
#else /* Little endian */
    /* No action is necessary */
#endif
}

/**************************************************/

/**
Encode/decode the storage of the raw HDF5 unsigned parameters.

Codec Format
{
"hdf5raw": "<NC_RAWVERSION>",
"nparams": "n",
"0": "<unsigned int>",
"1": "<unsigned int>",
"2": "<unsigned int>",
...
"n": "<unsigned int>",
}
*/

/* Convert from HDF5 unsigned to a JSON dict.
@param nparams
@params params
@return the JSON dict or NULL if failed
*/
int
NCraw_encode(size_t nparams, const unsigned* params, NCjson** jparamsp)
{
    int stat = 0;
    unsigned i;
    NCjson* jparams = NULL;
    NCjson* jnparams = NULL;
    NCjson* jnum = NULL;
    char digits[64];

    NCJnew(NCJ_DICT,&jparams);
    NCJcheck(NCJinsertstring(jparams,NC_RAWTAG,NC_RAWVERSION));
    snprintf(digits,sizeof(digits),"%zu",nparams);
    NCJcheck(NCJnewstring(NCJ_INT,digits,&jnparams));
    NCJcheck(NCJinsert(jparams,"nparams",jnparams)); jnparams = NULL;
    for(i=0;i<nparams;i++) {
	char num[64];
	snprintf(digits,sizeof(digits),"%u",i);
	snprintf(num,sizeof(num),"%u",params[i]);
	NCJcheck(NCJnewstring(NCJ_INT,num,&jnum));
        NCJcheck(NCJinsert(jparams,digits,jnum)); jnum = NULL;
    }
    if(jparamsp) {*jparamsp = jparams; jparams = NULL;}
done:
    NCJreclaim(jnparams);
    NCJreclaim(jnum);
    NCJreclaim(jparams);
    return stat;
}

/* Convert JSON formatted string to HDF5 nparams + params.
@param jcodec from which to extract params
@param nparamsp return number of params
@params paramsp return params
@return NCJ_OK if success, NCJ_ERR if fail.
*/
int
NCraw_decode(const NCjson* jcodec, size_t* nparamsp, unsigned** paramsp)
{
    int stat = 0;
    unsigned i;
    const NCjson* jvalue = NULL;
    size_t nparams = 0;
    unsigned* params = NULL;
    char digits[64];

    NCJcheck(NCJdictget(jcodec,NC_RAWTAG,(NCjson**)&jvalue));
    if(jvalue == NULL) {stat = NCJ_ERR; goto done;}
    if(strcmp(NCJstring(jvalue),NC_RAWVERSION) != 0) {stat = NCJ_ERR; goto done;}
    NCJcheck(NCJdictget(jcodec,"nparams",(NCjson**)&jvalue));
    if(jvalue != NULL && NCJsort(jvalue) != NCJ_INT) {stat = NCJ_ERR; goto done;}
    if(jvalue == NULL) {
        nparams = 0;
    } else {
	if(1 != sscanf(NCJstring(jvalue),"%zu",&nparams)) {stat = NCJ_ERR; goto done;}
    }
    if(nparams > 0) {
	if((params = (unsigned*)malloc(sizeof(unsigned)*nparams))==NULL) {stat = NCJ_ERR; goto done;}
    }
    for(i=0;i<nparams;i++) {
	snprintf(digits,sizeof(digits),"%u",i);
        NCJcheck(NCJdictget(jcodec,digits,(NCjson**)&jvalue));
        if(jvalue == NULL) {stat = NCJ_ERR; goto done;} /* nparams mismatch */
        if(NCJsort(jvalue) != NCJ_INT) {stat = NCJ_ERR; goto done;}
	sscanf(NCJstring(jvalue),"%u",&params[i]);
    }
    if(nparamsp) *nparamsp = nparams;
    if(paramsp) {*paramsp = params; params = NULL;}
done:
    if(params) free(params);
    return stat;
}
