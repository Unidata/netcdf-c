/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ztest.h"

typedef struct ProjTest {
    size_t R;
    size_t dimlen[NC_MAX_VAR_DIMS];
    size_t chunklen[NC_MAX_VAR_DIMS];
    NCZSlice slices[NC_MAX_VAR_DIMS];
}  ProjTest;

ProjTest projtests[] = {
{1, {4}, {2}, {{0,4,1}}},
{0, {0}, {0}, {{0}}}
};

static char* printtest(ProjTest* test);
static void printslices(int len, NCZSlice* slices, NCbytes* buf);
static int parseslices(const char* s0, NCZSlice* slices);
static int parsevector(const char* s0, size_t* vector);

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int count,c,r;
    ProjTest test;
    NCZSliceIndex* indices = NULL;

    while ((c = getopt(argc, argv, "r:d:c:s:")) != EOF) {
	switch(c) {
	case 'r': 
	    test.R = atoi(optarg);
	    if(test.R <= 0) {stat = NC_EINVAL; goto done;}
	    break;
	case 'd':
	    count = parsevector(optarg,test.dimlen);
	    if(count <= 0) {stat = NC_EINVAL; goto done;}
	    break;
	case 'c':
	    count = parsevector(optarg,test.chunklen);
	    if(count <= 0) {stat = NC_EINVAL; goto done;}
	    break;
	case 's':
	    count = parseslices(optarg,test.slices);
	    if(count <= 0) {stat = NC_EINVAL; goto done;}
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   stat = NC_EINVAL;
	   goto done;
	}
    }
    printf("%s\n",printtest(&test));
    if((stat = ncz_compute_all_slice_projections(test.R, test.slices, test.dimlen, test.chunklen, &indices)))
	goto done;
    for(r=0;r<test.R;r++) {
	printf("\t%s\n",nczprint_sliceindex(indices[r]));
    }

done:
    if(stat)
	nc_strerror(stat);
    return (stat ? 1 : 0);    
}


static void
printvec(int len, size_t* vec, NCbytes* buf)
{
    int i;
    char value[128];

    ncbytescat(buf,"(");
    for(i=0;i<len;i++) {
        if(i > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)vec[i]);	
	ncbytescat(buf,value);
    }
    ncbytescat(buf,")");
}

static void
printslices(int len, NCZSlice* slices, NCbytes* buf)
{
    int i;
    char value[128];

    for(i=0;i<len;i++) {
        ncbytescat(buf,"[");
        snprintf(value,sizeof(value),"%lu",(unsigned long)slices[i].start);	
	ncbytescat(buf,value);
        ncbytescat(buf,":");
        snprintf(value,sizeof(value),"%lu",(unsigned long)slices[i].stop);	
	ncbytescat(buf,value);
	if(slices[i].stride != 1) {
	    snprintf(value,sizeof(value),"%lu",(unsigned long)slices[i].stride);	
            ncbytescat(buf,":");
	    ncbytescat(buf,value);
	}
        ncbytescat(buf,"]");
    }
}

static char*
printtest(ProjTest* test)
{
    char* result = NULL;
    char value[128];
    NCbytes* buf = ncbytesnew();

    ncbytescat(buf,"Test{");
    snprintf(value,sizeof(value),"R=%lu",(unsigned long)test->R);
    ncbytescat(buf,value);
    ncbytescat(buf," dimlen=");
    printvec(test->R, test->dimlen, buf);
    ncbytescat(buf," chunklen=");
    printvec(test->R, test->chunklen, buf);
    ncbytescat(buf," slices=");
    printslices(test->R, test->slices, buf);
    ncbytescat(buf,"}");    

    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return result;
}

static int
parseslices(const char* s0, NCZSlice* slices)
{
    int count,nchars,nslices;
    NCZSlice* p = NULL;
    const char* s = NULL;
    unsigned long start,stop,stride;

    /* First, compute number of slices */
    for(s=s0,nslices=0;*s;s++) {
	if(*s == '[') nslices++;
    }
    
    if(nslices > NC_MAX_VAR_DIMS) return -1; /* too many */

    /* Extract the slices */
    for(s=s0,p=slices;*s;s+=nchars,p++) {
	/* Try 3-element slice first */
	stride = 1; /* default */
	nchars = -1;
        count = sscanf(s,"[%lu:%lu]%n",&start,&stop,&nchars);
	if(nchars == -1) {
	    nchars = -1;
            count = sscanf(s,"[%lu:%lu:%lu]%n",&start,&stop,&stride,&nchars);
	    if(count != 3) return -1;
	}
        p->start = start;
        p->stop = stop;
        p->stride = stride;
    }
    return nslices;
}

static int
parsevector(const char* s0, size_t* vector)
{
    int count,nchars,nelem;
    size_t* p = NULL;
    const char* s = NULL;

    /* First, compute number of elements */
    for(s=s0,nelem=1;*s;s++) {
	if(*s == ',') nelem++;
    }
    
    if(nelem > NC_MAX_VAR_DIMS) return -1; /* too many */

    /* Extract the elements of the vector */
    for(s=s0,p=vector;*s;p++) {
	unsigned long elem;
	nchars = -1;
        count = sscanf(s,"%lu%n",&elem,&nchars);
	if(nchars == -1 || count != 1) return -1;
	s += nchars;
	if(*s == ',') s++;
	*p = (size_t)elem;
    }
    return nelem;
}
