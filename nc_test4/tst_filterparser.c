/*
  Copyright 2008, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netcdf.h"
#include "ncfilter.h"

#undef USE_INTERNAL

#define PARAMS_ID 32768

static const unsigned int baseline[] = {
-17, /* 0 signed int*/
23,  /* 1 unsigned int*/
-25, /* 2 signed int*/
27,  /* 3 unsigned int*/
77,  /* 4 signed int*/
93,  /* 5 unsigned int*/
1145389056U, /* 6 float*/
3287505826, 1097305129,   /* 7-8 double*/
1, 2147483648,            /* 9-10 signed long long*/
4294967295, 4294967295,   /* 11-12 unsigned long long*/
/* Edge cases */
2147483647,   /* 13 max signed int*/
-2147483648,  /* 14 min signed int*/
4294967295    /* 15 max unsigned int with no trailing U*/
};

static const char* spec = 
"32768, -17b, 23ub, -25S, 27US, 77, 93U, 789f, 12345678.12345678d, -9223372036854775807L, 18446744073709551615UL, 2147483647, -2147483648, 4294967295";

/* Test support for the conversions */
/* Not sure if this kind of casting via union is legal C99 */
static union {
    unsigned int ui;
    float f;
} uf;

static union {
    unsigned int ui[2];
    double d;
} ud;

static union {
    unsigned int ui[2];
    unsigned long long ull;
    long long ll;
} ul;

static int nerrs = 0;

#ifdef USE_INTERNAL
static int parsefilterspec(const char* spec, unsigned int* idp, size_t* nparamsp, unsigned int** paramsp);
#endif

#ifdef WORD_BIGENDIAN
static void byteswap8(unsigned char* mem);
#endif

static void
report(const char* which)
{
    fprintf(stderr,"mismatch: %s\n",which);
    fflush(stderr);
    nerrs++;
}

static void
mismatch(size_t i, unsigned int baseline, unsigned int params)
{
   fprintf(stderr,"mismatch: [%d] baseline=%ud spec=%ud\n",(int)i,baseline,params);
    fflush(stderr);
    nerrs++;
}

/**************************************************/
int
main(int argc, char **argv)
{
    int stat = 0;
    unsigned int id = 0;
    size_t i,nparams = 0;
    unsigned int* params = NULL;

    printf("\nTesting filter parser.\n");

#ifdef USE_INTERNAL
    stat = parsefilterspec(spec,&id,&nparams,&params);
#else
    stat = NC_parsefilterspec(spec,&id,&nparams,&params);
#endif
    if(!stat) {
	report("NC_parsefilterspec failed");
	exit(1);
    }
    for(i=0;i<nparams;i++) {
	if(baseline[i] != params[i])
	    mismatch(i,baseline[i],params[i]);
    }
    /* Now some specialized tests */
    uf.ui = params[6];
    if(uf.f != (float)789.0)
	report("uf.f");
    ud.ui[0] = params[7];
    ud.ui[1] = params[8];
#ifdef WORD_BIGENDIAN
    byteswap8((unsigned char*)&ud.d);
#endif
    if(ud.d != (double)12345678.12345678)
	report("ud.d");
    ul.ui[0] = params[9];
    ul.ui[1] = params[10];
#ifdef WORD_BIGENDIAN
    byteswap8((unsigned char*)&ul.ll);
#endif
    if(ul.ll != -9223372036854775807LL)
	report("ul.ll");
    ul.ui[0] = params[11];
    ul.ui[1] = params[12];
#ifdef WORD_BIGENDIAN
    byteswap8((unsigned char*)&ul.ull);
#endif
    if(ul.ull != 18446744073709551615ULL)
	report("ul.ull");

    if (params)
       free(params);

    if (!nerrs)
       printf("SUCCESS!!\n");

    return (nerrs > 0 ? 1 : 0);
}

#ifdef WORD_BIGENDIAN
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
#endif

#ifdef USE_INTERNAL
static int
parsefilterspec(const char* spec, unsigned int* idp, size_t* nparamsp, unsigned int** paramsp)
{
    char* p;
    char* sdata = NULL;
    int stat;
    unsigned int uval, id;
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
#ifdef WORDS_BIGENDIAN	    
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
#ifdef WORDS_BIGENDIAN	    
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
#endif /*USE_INTERNAL*/
