/* This is part of the netCDF package.  Copyright 2005-2011,
   University Corporation for Atmospheric Research/Unidata. See
   COPYRIGHT file for conditions of use.

   Test that NetCDF-4 can read HDF4 files.
*/
#include <config.h>
#include <nc_tests.h>
#include <hdf5.h>
#include <H5DSpublic.h>
#include <mfhdf.h>

#define CHUNKEDFILE "chunked.hdf4"
#define CHUNKEDVAR "LandWater"

#define CONTIGFILE "contiguous.hdf4"
#define CONTIGVAR "pres"

#define LAT_LEN 3
#define LON_LEN 2
#define DIMS_2 2

static size_t EXPECTED_CHUNKSIZES[2] = {1,1200};

int
main(int argc, char **argv)
{
   int ncid;
   int varid;
   int rank;
   int d;
   int storage;
   size_t chunksizes[NC_MAX_VAR_DIMS];
   const char* srcdir = ".";
   char* path;
   
   if(argc > 1)
	srcdir = argv[1];

   printf("\n*** Testing HDF4/NetCDF-4 chunking API: chunked...\n");
   {
      path = (char*)malloc(sizeof(srcdir)+sizeof(CHUNKEDFILE)+2);
      strcpy(path,srcdir);
      strcat(path,"/");
      strcat(path,CHUNKEDFILE);

      /* Open with netCDF */
      if (nc_open(path, NC_NOWRITE, &ncid)) ERR; 

      /* Get a variable id */
      if(nc_inq_varid(ncid,CHUNKEDVAR,&varid)) ERR;

      /* get rank */
      if(nc_inq_varndims(ncid,varid,&rank)) ERR;

      /* get chunk info */
      memset(chunksizes,0,sizeof(chunksizes));
      if(nc_inq_var_chunking(ncid,varid,&storage,chunksizes)) ERR;

      if(storage == NC_CONTIGUOUS) {
	fprintf(stderr,"nc_inq_var_chunking did not return CHUNKED\n");
	ERR;
      }

      for(d=0;d<rank;d++) {
	if(EXPECTED_CHUNKSIZES[d] != chunksizes[d]) {
	    fprintf(stderr,"chunk size mismatch: [%d] %ld :: %ld\n",d,chunksizes[d],EXPECTED_CHUNKSIZES[d]);
	    ERR;
	}
      }
      if (nc_close(ncid)) ERR; 
   }
   
   printf("\n*** Testing HDF4/NetCDF-4 chunking API: contiguous...\n");
   {
      path = (char*)malloc(sizeof(srcdir)+sizeof(CONTIGFILE)+2);
      strcpy(path,srcdir);
      strcat(path,"/");
      strcat(path,CONTIGFILE);

      /* Open with netCDF */
      if (nc_open(path, NC_NOWRITE, &ncid)) ERR; 

      /* Get a variable id */
      if(nc_inq_varid(ncid,CONTIGVAR,&varid)) ERR;

      /* get rank */
      if(nc_inq_varndims(ncid,varid,&rank)) ERR;

      /* get chunk info */
      memset(chunksizes,0,sizeof(chunksizes));
      if(nc_inq_var_chunking(ncid,varid,&storage,chunksizes)) ERR;

      if(storage != NC_CONTIGUOUS) {
	fprintf(stderr,"nc_inq_var_chunking did not return CONTIGUOUS\n");
	ERR;
      }

      if (nc_close(ncid)) ERR; 
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

