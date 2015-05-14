/*! Test for NCF-331. Added May 11, 2015.
 *  See the following links for more information:
 *
 *  o Issue on GitHub: https://github.com/Unidata/netcdf-c/issues/112
 *  o Issue in JIRA:   https://bugtracking.unidata.ucar.edu/browse/NCF-331
 *
 * Test contributed by Jeff Whitaker
 */

#include <string.h>
#include <netcdf.h>
#include <stdio.h>
#include "nc_logging.h"

#define FILE_NAME "tst_endian_float.nc"
#define NDIM 10
#define NLON 20
#define DIM_NAME "x"
#define VAR_NAME2 "fv"
#define VAR_NAME3 "iv"
#define VAR_NAME "jv"
int main() {

  /*
   * 1. Create a file with endianness as desired.
   */

  int ncid, dimid, varid, varid2, varid3, retval;
  int ed, ed2, ed3;
  int failures = 0;
  int oldfill = 0;

#ifdef LOGGING
  printf("Setting log level 10\n");
  nc_set_log_level(10);
  LOG((2,"setting Log_level, logging with level 2."));
#endif

  retval = nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid);

  retval = nc_def_dim(ncid, DIM_NAME, NDIM, &dimid);

  /* First, Float */
  retval = nc_def_var(ncid, VAR_NAME, NC_FLOAT, 1, &dimid, &varid);
    retval = nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG);

  /* Second, Double */
  retval = nc_def_var(ncid, VAR_NAME2, NC_DOUBLE, 1, &dimid, &varid2);
  retval = nc_def_var_endian(ncid, varid2, NC_ENDIAN_BIG);

  /* Third, Int */
  retval = nc_def_var(ncid, VAR_NAME3, NC_INT, 1, &dimid, &varid3);
  retval = nc_def_var_endian(ncid, varid3, NC_ENDIAN_LITTLE);

  retval = nc_close(ncid);

  /*
   * 2. Reopen file, check to see if the endianness attribute
   *    exists.
   */
  ncid = 0;
  retval = nc_open(FILE_NAME, NC_NETCDF4 | NC_NOWRITE, &ncid);
  retval = nc_set_fill(ncid, NC_NOFILL, &oldfill);
  retval = nc_inq_varid(ncid,VAR_NAME,&varid);
  retval = nc_inq_varid(ncid,VAR_NAME2,&varid2);
  retval = nc_inq_varid(ncid,VAR_NAME3,&varid3);

  retval = nc_inq_var_endian(ncid,varid,&ed);
  if(ed != NC_ENDIAN_BIG) {
    printf("Test 1: Error for float variable endianness: [%d] not NC_ENDIAN_BIG\n",ed);
    failures++;
  } else {
    printf("Test 1: [%d] is NC_ENDIAN_BIG, Success.\n",ed);
  }

  retval = nc_inq_var_endian(ncid,varid2,&ed2);
  if(ed2 != NC_ENDIAN_BIG) {
    printf("Test 2: Error for double variable endianness: [%d] not NC_ENDIAN_BIG\n",ed);
    failures++;
  } else {
    printf("Test 2: [%d] is NC_ENDIAN_BIG, Success.\n",ed2);
  }

  retval = nc_inq_var_endian(ncid,varid3,&ed3);
  if(ed3 != NC_ENDIAN_LITTLE) {
    printf("Test 3: Error for integer variable endianness: [%d] not NC_ENDIAN_BIG\n",ed2);
    failures++;
  } else {
    printf("Test 3: [%d] is NC_ENDIAN_BIG, Success.\n",ed3);
  }

  retval = nc_close(ncid);


  /* Force a failure for now, until I can automate the check
     programatically that the endianess attribute is properly
     stored. */

  return failures;
}
