#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "netcdf.h"

#include "test_utils.h"

/* This is the name of the data file we will read. */
#define FILE_NAME "https://s3.us-east-1.amazonaws.com/unidata-zarr-test-data/"TESTSUBTREE"/testfile.zarr#mode=zarr,s3"
#define GROUPNAME "g1"
#define VARNAME "v"

#define NX 1
#define NY 1

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

int
main()
{
   /* There will be netCDF IDs for the file, each group, and each
    * variable. */
   int ncid, varid1, grp1id;

   uint8_t data_in[3 * NX * NY];

   /* Loop indexes, and error handling. */
   int retval;

   /* Open the file. NC_NOWRITE tells netCDF we want read-only access
    * to the file.*/
   if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
      ERR(retval);

   /* Get the group ids of our group. */
   if ((retval = nc_inq_ncid(ncid, GROUPNAME, &grp1id)))
      ERR(retval);

   /* Get the varid of the uint8 data variable, based on its name, in
    * grp1. */
   if ((retval = nc_inq_varid(grp1id, VARNAME, &varid1)))
      ERR(retval);

   /* Read the data. */
   if ((retval = nc_get_var_ubyte(grp1id, varid1, &data_in[0])))
      ERR(retval);

   /* Close the file, freeing all resources. */
   if ((retval = nc_close(ncid)))
      ERR(retval);

   printf("*** SUCCESS reading example file %s!\n", FILE_NAME);
   return 0;
}
