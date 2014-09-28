/*
  Copyright 2008, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This program tests the large file bug in netCDF 3.6.2,
  creating byte and short variables larger than 4 GiB.
*/

#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef SZIP_COMPRESSION
#include <szlib.h>
#endif

#define ZIP_FILE_NAME "zip.nc"
#define BZIP2_FILE_NAME "bzip2.nc"
#define SZIP_FILE_NAME "szip.nc"

#define DEFLATE_LEVEL 9

        unsigned int options_mask;
	unsigned int bits_per_pixel;
        unsigned int pixels_per_block;


#define NUMDIMS 2		/* rank of each variable in tests */
#if 0
#define DIM1 2048
#define DIM2 2097153		/* DIM1*DIM2*sizeof(char)   > 2**32 */
#else
#define DIM1 64
#define DIM2 32
#endif

/* Created Meta-data
netcdf zip {
dimensions:
	dim1 = 2048 ;
	dim2 = 2097152 ;
variables:
	int var(dim1, dim2) ;
}
*/

static int
test_bzip2(const char *testfile) 
{
    int i,j;
    int ncid, varid, dimids[NUMDIMS];
    size_t index[NUMDIMS];
    nc_compression_t parms;
    char* algorithm;

    /* Create a file with one big variable. */
    if (nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;
    if (nc_def_dim(ncid, "dim1", DIM1, &dimids[0])) ERR;
    if (nc_def_dim(ncid, "dim2", DIM2, &dimids[1])) ERR;
    if (nc_def_var(ncid, "var", NC_INT, NUMDIMS, dimids, &varid)) ERR;
    /* Use bzip2 compression */
    parms.level = DEFLATE_LEVEL;
    if (nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "bzip2", &parms)) ERR;
    if (nc_enddef(ncid)) ERR;
    /* Fill in the array */
    for(i=0;i<DIM1;i++) {
        for(j=0;j<DIM2;j++) {
	    int value = (i*DIM2) + j;
	    index[0] = i;
            index[1] = j;
            if (nc_put_var1_int(ncid, varid, index, &value)) {
		ERR;
	    }
	}
    }
    if (nc_close(ncid)) ERR;

    /* Open the file and check it. */
    if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_varid(ncid, "var", &varid)) ERR;
    /* Check the compression algorithm */
    if (nc_inq_var_compress(ncid,varid,NULL,&algorithm,&parms)) ERR;
    if (strcmp(algorithm,"bzip2") != 0) {
	printf("Compression algorithm mismatch: %s\n",algorithm);
	exit(1);
    } else {
	printf("Compression algorithm verified: %s\n",algorithm);
    }
    for(i=0;i<DIM1;i++) {
        for(j=0;j<DIM2;j++) {
	    int expected = (i*DIM2) + j;
	    int value = (i*DIM2) + j;
	    index[0] = i;
            index[1] = j;
            if (nc_get_var1_int(ncid, varid, index, &value)) ERR;
	    if(value != expected) ERR;
	}
    }
    if (nc_close(ncid)) ERR;
    return 0;
}

#ifdef SZIP_COMPRESSION
static int
test_szip(const char *testfile) 
{
    int i,j;
    int ncid, varid, dimids[NUMDIMS];
    size_t index[NUMDIMS];
    nc_compression_t parms;
    char* algorithm;

    /* Create a file with one big variable. */
    if (nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;
    if (nc_def_dim(ncid, "dim1", DIM1, &dimids[0])) ERR;
    if (nc_def_dim(ncid, "dim2", DIM2, &dimids[1])) ERR;
    if (nc_def_var(ncid, "var", NC_INT, NUMDIMS, dimids, &varid)) ERR;
    /* Use szip compression */
    parms.szip.bits_per_pixel = 0;
    parms.szip.pixels_per_scanline = 0;
#if 0
    parms.szip.options_mask = SZ_NN_OPTION_MASK;
    parms.szip.pixels_per_block = 16;
#else
    parms.szip.options_mask = SZ_EC_OPTION_MASK;
    parms.szip.pixels_per_block = 32;
#endif
    if (nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "szip", &parms)) ERR;
    if (nc_enddef(ncid)) ERR;
    /* Fill in the array */
    for(i=0;i<DIM1;i++) {
        for(j=0;j<DIM2;j++) {
	    int value = (i*DIM2) + j;
	    index[0] = i;
            index[1] = j;
            if (nc_put_var1_int(ncid, varid, index, &value)) {
		ERR;
	    }
	}
    }
    if (nc_close(ncid)) ERR;

    /* Open the file and check it. */
    if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_varid(ncid, "var", &varid)) ERR;
    /* Check the compression algorithm */
    if (nc_inq_var_compress(ncid,varid,NULL,&algorithm,&parms)) ERR;
    if (strcmp(algorithm,"szip") != 0) {
	printf("Compression algorithm mismatch: %s\n",algorithm);
	exit(1);
    } else {
	printf("Compression algorithm verified: %s\n",algorithm);
    }
    for(i=0;i<DIM1;i++) {
        for(j=0;j<DIM2;j++) {
	    int expected = (i*DIM2) + j;
	    int value = (i*DIM2) + j;
	    index[0] = i;
            index[1] = j;
            if (nc_get_var1_int(ncid, varid, index, &value)) ERR;
	    if(value != expected) ERR;
	}
    }
    if (nc_close(ncid)) ERR;
    return 0;
}
#endif

static int
test_zip(const char *testfile) 
{
    int i,j;
    int ncid, varid, dimids[NUMDIMS];
    size_t index[NUMDIMS];
    nc_compression_t parms;
    char* algorithm;

    /* Create a file with one big variable. */
    if (nc_create(testfile, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;
    if (nc_def_dim(ncid, "dim1", DIM1, &dimids[0])) ERR;
    if (nc_def_dim(ncid, "dim2", DIM2, &dimids[1])) ERR;
    if (nc_def_var(ncid, "var", NC_INT, NUMDIMS, dimids, &varid)) ERR;
    /* Use zip compression */
    parms.level = DEFLATE_LEVEL;
    if (nc_def_var_compress(ncid, varid, NC_NOSHUFFLE, "zip", &parms)) ERR;
    if (nc_enddef(ncid)) ERR;

    /* Fill in the array */
    for(i=0;i<DIM1;i++) {
        for(j=0;j<DIM2;j++) {
	    int value = (i*DIM2) + j;
	    index[0] = i;
            index[1] = j;
            if (nc_put_var1_int(ncid, varid, index, &value)) ERR;
	}
    }
    if (nc_close(ncid)) ERR;

    /* Open the file and check it. */
    if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_varid(ncid, "var", &varid)) ERR;
    /* Check the compression algorithm */
    if (nc_inq_var_compress(ncid,varid,NULL,&algorithm,&parms)) ERR;
    if (strcmp(algorithm,"zip") != 0) {
	printf("Compression algorithm mismatch: %s\n",algorithm);
	exit(1);
    } else {
	printf("Compression algorithm verified: %s\n",algorithm);
    }

    for(i=0;i<DIM1;i++) {
        for(j=0;j<DIM2;j++) {
	    int expected = (i*DIM2) + j;
	    int value = (i*DIM2) + j;
	    index[0] = i;
            index[1] = j;
            if (nc_get_var1_int(ncid, varid, index, &value)) ERR;
	    if(value != expected) ERR;
	}
    }
    if (nc_close(ncid)) ERR;
    return 0;
}

int
main(int argc, char **argv)
{
    char testfile[NC_MAX_NAME + 1];

#ifdef BZIP2_COMPRESSION
    {
        printf("\n*** Testing bzip2 compression.\n");
	sprintf(testfile, "%s", BZIP2_FILE_NAME);
	test_bzip2(testfile);
	SUMMARIZE_ERR;
    }
#endif
    
#ifdef SZIP_COMPRESSION
    {
        printf("\n*** Testing szip compression.\n");
	sprintf(testfile, "%s", SZIP_FILE_NAME);
	test_szip(testfile);
	SUMMARIZE_ERR;
    }
#endif

    {
        printf("\n*** Testing zip compression.\n");
	sprintf(testfile, "%s", ZIP_FILE_NAME);
	test_zip(testfile);
	SUMMARIZE_ERR;
    }

    FINAL_RESULTS;
}
