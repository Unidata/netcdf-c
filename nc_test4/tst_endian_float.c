/*! Test for NCF-331.
 *  See the following links for more information:
 *
 *  o Issue on GitHub: https://github.com/Unidata/netcdf-c/issues/112
 *  o Issue in JIRA:   https://bugtracking.unidata.ucar.edu/browse/NCF-331
 *
 * Test contributed by Jeff Whitaker
 */

#include <string.h>
#include <netcdf.h>
#define FILE_NAME "test.nc"
#define NDIM 10
#define NLON 20
#define DIM_NAME "x"
#define VAR_NAME "fv"
#define VAR_NAME2 "iv"
int
main()
{
  int ncid, dimid, varid, varid2, retval;
  retval = nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid);
  retval = nc_def_dim(ncid, DIM_NAME, NDIM, &dimid);
  retval = nc_def_var(ncid, VAR_NAME, NC_FLOAT, 1, &dimid, &varid);
  retval = nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG);
  retval = nc_def_var(ncid, VAR_NAME2, NC_INT, 1, &dimid, &varid2);
  retval = nc_def_var_endian(ncid, varid2, NC_ENDIAN_BIG);
  retval = nc_close(ncid);
  return 0;
}
