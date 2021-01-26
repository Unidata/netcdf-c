/*
  Copyright 2018, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.
*/

/*! \file
Test support for multiple filters per variable
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <hdf5.h>
#include "netcdf.h"
#include "netcdf_filter.h"

/* The HDF assigned id for bzip compression */
#define BZIP2_ID 307
/* The compression level used in this example */
#define BZIP2_LEVEL 9

#define DEFLATE_LEVEL 2

#define NOOP_ID 40000

#define NFILTERS 3

#define TESTFILE "multifilter.nc"

/* Point at which we give up */
#define MAXERRS 8

#define NDIMS 4
#define DIMSIZE 4
#define CHUNKSIZE 4 /* Note: not the total size of the chunk, but size wrt a dim*/

static size_t dimsize = DIMSIZE;
static size_t chunksize = CHUNKSIZE;
static size_t actualdims = NDIMS;

static size_t actualproduct = 1; /* x-product over dim sizes */
static size_t chunkproduct = 1; /* x-product over chunksizes */

static size_t dims[NDIMS];
static size_t chunks[NDIMS];

static int nerrs = 0;

static int ncid, varid;
static int dimids[NDIMS];
static float* array = NULL;
static float* expected = NULL;

/* Forward */
static void init(int argc, char** argv);
static int test_multi(void);
static int verifychunks(void);

#define ERRR do { \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n", \
	__FILE__, __LINE__);				    \
nerrs++;\
} while (0)

static int
check(int err,int line)
{
    if(err != NC_NOERR) {
	fprintf(stderr,"fail (%d): %s\n",line,nc_strerror(err));
	fflush(stderr);
	exit(1);
    }
    return NC_NOERR;
}

#define CHECK(x) check(x,__LINE__)

/*
Read the chunking information about the variable
and verify that it is as expected.
*/

static int
verifychunks(void)
{
    int i;
    int store = -1;
    size_t chunksizes[NDIMS];
    memset(chunksizes,0,sizeof(chunksizes));
    CHECK(nc_inq_var_chunking(ncid, varid, &store, chunksizes));
    /* Storate must be chunked, not contiguous */
    if(store != NC_CHUNKED) {
	fprintf(stderr,"bad chunk store\n");
	return NC_ESTORAGE;
    }
    /* Chunk sizes must match our predefined set */
    for(i=0;i<actualdims;i++) {
        if(chunksizes[i] != chunks[i]) {
	    fprintf(stderr,"bad chunk size: %d\n",i);
	    return NC_EBADCHUNK;
	}
    }
    return 1;
}

/*
Compare the data we wrote against the data we read.
*/

static int
compare(void)
{
    int errs = 0;
    int i;
    printf("data comparison: |array|=%lu\n",(unsigned long)actualproduct);
    for(i=0;i<actualproduct;i++) {
	if(expected[i] != array[i]) {
	    printf("mismatch: array[%d]=%f expected[%d]=%f\n",
                            i,array[i],i,expected[i]);
            errs++;
            if(errs >= MAXERRS)
                break;
	}
   }
   if(errs == 0)
        printf("no data errors\n");
   if(actualproduct <= 1)
	return NC_EBADDIM;
   return (errs == 0 ? NC_NOERR: NC_EINVAL);
}

int
verifyfilters(int ncid, int varid)
{
    size_t nparams;
    size_t nfilters;
    unsigned int filterids[NFILTERS];
    unsigned int params[2];
    
    /* Read back the compression info and verify it */
    CHECK(nc_inq_var_filter_ids(ncid,varid,&nfilters,filterids));
    if(nfilters != NFILTERS) {
	fprintf(stderr,"Fail: nfilters mismatch: expected=%d actual=%u\n",NFILTERS,(unsigned)nfilters);
        return NC_EINVAL;
    }    
    if(filterids[0] != BZIP2_ID
       || filterids[1] != H5Z_FILTER_DEFLATE
       || filterids[2] != NOOP_ID
       ) {
	fprintf(stderr,"Fail: filter id mismatch: actual/expected={%u/%u,%u/%u,%u/%u}\n",
	        filterids[0],BZIP2_ID,
		filterids[1],H5Z_FILTER_DEFLATE,
		filterids[2],NOOP_ID);
        return NC_EINVAL;
    }    
    /* Get level for each filter */
    CHECK(nc_inq_var_filter_info(ncid,varid,BZIP2_ID,&nparams,params));
    if(nparams != 1) {
	fprintf(stderr,"Fail: nparams mismatch: id=%u expected=1 actual=%u\n",filterids[0],(unsigned)nparams);
        return NC_EINVAL;
    }    
    if(params[0] != BZIP2_LEVEL) {
	fprintf(stderr,"Fail: parameter mismatch: expected=%u actual=%u\n",BZIP2_LEVEL,params[0]);
        return NC_EINVAL;
    }    
    CHECK(nc_inq_var_filter_info(ncid,varid,H5Z_FILTER_DEFLATE,&nparams,params));
    if(nparams != 1) {
	fprintf(stderr,"Fail: nparams mismatch: id=%u expected=1 actual=%u\n",filterids[1],(unsigned)nparams);
        return NC_EINVAL;
    }    
    if(params[0] != DEFLATE_LEVEL) {
	fprintf(stderr,"Fail: parameter mismatch: expected=%u actual=%u\n",BZIP2_LEVEL,params[0]);
        return NC_EINVAL;
    }    
    CHECK(nc_inq_var_filter_info(ncid,varid,NOOP_ID,&nparams,params));
    if(nparams != 0) {
	fprintf(stderr,"Fail: parameter mismatch: id=%u nparams: expected=0 actual=%u\n",NOOP_ID,(unsigned)nparams);
        return NC_EINVAL;
    }    
    return NC_NOERR;
}

/*
Create the file, write it, then re-read for comparison.
*/
static int
test_multi(void)
{
    int i;
    unsigned int params[2];

    printf("\n*** Testing Multi-filter application: filter set = bzip2 deflate noop");
    printf("\n");
    
    /* Clear the data array */
    memset(array,0,sizeof(float)*actualproduct);

    /* Create a file */
    CHECK(nc_create(TESTFILE, NC_NETCDF4|NC_CLOBBER, &ncid));

    /* Do not use fill for this file */
    CHECK(nc_set_fill(ncid, NC_NOFILL, NULL));

    /* Define the dimensions */
    for(i=0;i<actualdims;i++) {
	char dimname[1024];
	snprintf(dimname,sizeof(dimname),"dim%d",i);
        CHECK(nc_def_dim(ncid, dimname, dims[i], &dimids[i]));
    }

    /* Define the variable */
    CHECK(nc_def_var(ncid, "var", NC_FLOAT, actualdims, dimids, &varid));

    /* Set chunking on the variable */
    CHECK(nc_def_var_chunking(ncid,varid,NC_CHUNKED,chunks));

    /* Verify that chunking succeeded */
    if(!verifychunks())
	return NC_EINVAL;

    /* Set bzip2 compression for the variable: takes one parameter == level */
    params[0] = BZIP2_LEVEL;
    CHECK(nc_def_var_filter(ncid,varid,BZIP2_ID,1,params));

    /* Set deflate (zip) compression for the variable: takes one parameter == level */
    params[0] = DEFLATE_LEVEL;
    CHECK(nc_def_var_filter(ncid,varid,H5Z_FILTER_DEFLATE,1,params));

    /* Set noop) compression for the variable */
    CHECK(nc_def_var_filter(ncid,varid,NOOP_ID,0,NULL));

    /* Read back the compression info and verify it */
    CHECK(verifyfilters(ncid,varid));
    printf("filters verified\n");
    
    /* Show chunking */ 
    printf("show chunks:");
    for(i=0;i<actualdims;i++)
	printf("%s%u",(i==0?" chunks=":","),(unsigned)chunks[i]);
    printf("\n");

    /* prepare to write */
    CHECK(nc_enddef(ncid));

    /* Fill in the array */
    for(i=0;i<actualproduct;i++)
	expected[i] = (float)i;

    /* write array */
    CHECK(nc_put_var(ncid,varid,expected));

    /* Close file */
    CHECK(nc_close(ncid));

    /* Now re-open and verify */
    printf("\n*** Testing Multi-filters.\n");

    /* Clear the data array */
    memset(array,0,sizeof(float)*actualproduct);

    /* Open the file */
    CHECK(nc_open(TESTFILE, NC_NOWRITE, &ncid));

    /* Get the variable id */
    CHECK(nc_inq_varid(ncid, "var", &varid));

    CHECK(verifyfilters(ncid,varid));
    printf("filters verified\n");

    /* Verify chunking */
    if(!verifychunks())
	return 0;

    /* Read the data */
    CHECK(nc_get_var_float(ncid, varid, array));

    /* Close the file */
    CHECK(nc_close(ncid));
    return (compare() == NC_NOERR ? 0 : 1);
}

/**************************************************/
/* Utilities */

static void
init(int argc, char** argv)
{
    int i;
    /* Setup various variables */
    actualproduct = 1;
    chunkproduct = 1;
    for(i=0;i<NDIMS;i++) {
	dims[i] = dimsize;
	chunks[i] = chunksize;
	if(i < actualdims) {
	    actualproduct *= dims[i];
	    chunkproduct *= chunks[i];
	}
    }
    /* Allocate max size */
    array = (float*)calloc(1,sizeof(float)*actualproduct);
    expected = (float*)calloc(1,sizeof(float)*actualproduct);
}

/**************************************************/
int
main(int argc, char **argv)
{
    H5Eprint(stderr);
    init(argc,argv);
    if(test_multi() != NC_NOERR) ERRR;
    exit(nerrs > 0?1:0);
}

