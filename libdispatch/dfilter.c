/*
 * Copyright 1996, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include "netcdf.h"

/*
Common utilities related to filters.
*/


/* Forward */
static void byteswap8(unsigned char* mem);

/**************************************************/
/*
Parse a filter spec string into a filter id + a vector
of unsigned ints.

@param spec0 - a string containing the spec as a sequence of
              constants separated by commas.
@param idp - store the parsed filter id here
@param nparamsp - store number of parsed filter params here
@param paramsp - store the vector of parsed filter params here
@return 1 if parse succeeded, 0 otherwise.
*/

EXTERNL int
NC_parsefilterspec(const char* spec, unsigned int* idp, size_t* nparamsp, unsigned int** paramsp)
{
    char* p;
    char* sdata = NULL;
    int stat;
    unsigned int id;
    size_t count; /* no. of comma delimited params */
    size_t nparams; /* final no. of unsigned ints */
    size_t i;
    unsigned int* ulist = NULL;
    unsigned char mem[8]; /* to convert to network byte order */

    if(spec == NULL || strlen(spec) == 0) goto fail;
    sdata = strdup(spec);

    /* Count number of parameters + id and delimit */
    p=sdata;
    for(count=0;;count++) {
        char* q = strchr(p,',');
	if(q == NULL) break;
	*q++ = '\0';
	p = q;
    }
    count++; /* for final piece */

    if(count == 0)
	goto fail; /* no id and no parameters */

    /* Extract the filter id */
    p = sdata;
    stat = sscanf(p,"%u",&id);
    if(stat != 1) goto fail;
    p = p + strlen(p) + 1; /* skip the filter id */
    count--;

    /* Allocate the max needed space; *2 in case the params are all doubles */
    ulist = (unsigned int*)malloc(sizeof(unsigned int)*(count)*2);
    if(ulist == NULL) goto fail;

    /* walk and convert */
    nparams = 0;
    for(i=0;i<count;i++) {
	char* q;
	unsigned long long val64u;
	unsigned int val32u;
	double vald;
	float valf;
	unsigned int *vector;
	int isunsigned;
	int isnegative;
	int type;

	/* Get trailing discrimination characters */
	isunsigned = 0;
	isnegative = 0;
        type = 0;
	if(strchr(p,'-') != NULL) isnegative = 1;
        q = p+strlen(p)-2;
	if(*q == 'U' || *q == 'u') isunsigned = 1;
	q++;
	switch (*q) {
	case 'f': case 'F': type = 'f'; break; /* short */
	case 'd': case 'D': type = 'd'; break; /* double */
	case 'b': case 'B': type = 'b'; break; /* byte */
	case 's': case 'S': type = 's'; break; /* short */
	case 'l': case 'L': type = 'l'; break; /* long long */
	case 'u': case 'U': type = 'i'; isunsigned = 1; break; /* integer */
	case '.':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': type = 'i'; break;
	default:
	    if(*q == '\0')
		type = 'i';
	    else goto fail;
	}
	/* Now parse */
	switch (type) {
	case 'b':
	case 's':
	case 'i':
 	    /* special case for a positive integer;for back compatibility.*/
	    if(!isnegative)
	        stat = sscanf(p,"%u",&val32u);
	    else
                stat = sscanf(p,"%d",(int*)&val32u);
	    if(stat != 1) goto fail;
	    ulist[nparams++] = val32u;
	    break;
	case 'f':
	    stat = sscanf(p,"%lf",&vald);
	    if(stat != 1) goto fail;
	    valf = (float)vald;
	    ulist[nparams++] = *(unsigned int*)&valf;
	    break;
	case 'd':
	    stat = sscanf(p,"%lf",&vald);
	    if(stat != 1) goto fail;
	    /* convert to network byte order */
	    memcpy(mem,&vald,sizeof(mem));
#ifndef WORDS_BIGENDIAN	    
	    byteswap8(mem);  /* convert little endian to big endian */
#endif
	    vector = (unsigned int*)mem;
	    ulist[nparams++] = vector[0];
	    ulist[nparams++] = vector[1];
	    break;
	case 'l': /* long long */
	    if(isunsigned)
	        stat = sscanf(p,"%llu",&val64u);
	    else
                stat = sscanf(p,"%lld",(long long*)&val64u);
	    if(stat != 1) goto fail;
	    /* convert to network byte order */
	    memcpy(mem,&val64u,sizeof(mem));
#ifndef WORDS_BIGENDIAN	    
	    byteswap8(mem);  /* convert little endian to big endian */
#endif
	    vector = (unsigned int*)mem;
	    ulist[nparams++] = vector[0];
	    ulist[nparams++] = vector[1];
	    break;
	default:
	    goto fail;
	}
        p = p + strlen(p) + 1; /* move to next param */
    }
    /* Now return results */
    if(idp) *idp = id;
    if(nparamsp) *nparamsp = nparams;
    if(paramsp) *paramsp = ulist;
    ulist = NULL; /* avoid duplicate free */
    if(sdata) free(sdata);
    if(ulist) free(ulist);
    return 1;
fail:
    if(sdata) free(sdata);
    if(ulist) free(ulist);
    return 0;
}

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
