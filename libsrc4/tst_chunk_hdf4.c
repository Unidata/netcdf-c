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

#define FILE_NAME "DEM_SN_A.h00v08.006_0.hdf"
#define VAR_NAME "LandWater"

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
   
   printf("\n*** Testing HDF4/NetCDF-4 chunking API...\n");
   {
      /* Now open with netCDF */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; 

      /* Get a variable id */
      if(nc_inq_varid(ncid,VAR_NAME,&varid)) ERR;

      /* get rank */
      if(nc_inq_varndims(ncid,varid,&rank)) ERR;

      /* get chunk info */
      if(nc_inq_var_chunking(ncid,varid,&storage,chunksizes)) ERR;

      if(storage == NC_CONTIGUOUS) {
	fprintf(stderr,"nc_inq_var_chunking did not CHUNKED\n");
	ERR;
      }

      for(d=0;d<rank;d++) {
	if(EXPECTED_CHUNKSIZES[d] != chunksizes[d]) {
	    fprintf(stderr,"chunk size mismatch: [%d] %d :: %d\n",d,chunksizes[d],EXPECTED_CHUNKSIZES[d]);
	    ERR;
	}
      }
      if (nc_close(ncid)) ERR; 
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

