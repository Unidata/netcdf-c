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
#include <hdf5.h>
#include <nc_tests.h>

#define FILE_NAME_NC "tst_endian_float.nc"
#define FILE_NAME_H5 "tst_endian_float.h5"


#define NDIM 10
#define NLON 20
#define DIM_NAME "x"
#define DIM_LEN 3
#define GRP_NAME "grp"
#define VAR_NAME2 "fv"
#define VAR_NAME3 "iv"
#define VAR_NAME "jv"
int main() {

  int ncid, dimid, varid, varid2, varid3, retval;
    int ed, ed2, ed3;
    int failures = 0;
    int oldfill = 0;

#ifdef LOGGING1
    printf("Setting log level 10\n");
    nc_set_log_level(10);
    LOG((2,"setting Log_level, logging with level 2."));
#endif
    printf("* Checking that endianness is properly read from file.\n");
    printf("** Generating test files.\n");
   /*
   * 1. Create a netcdf file with endianness as desired.
   */
  {

    printf("*** Creating a file via netcdf API: %s.\n",FILE_NAME_NC);
    retval = nc_create(FILE_NAME_NC, NC_NETCDF4 | NC_CLOBBER, &ncid);

    retval = nc_def_dim(ncid, DIM_NAME, NDIM, &dimid);

    /* First, Float */
    retval = nc_def_var(ncid, VAR_NAME, NC_FLOAT, 1, &dimid, &varid);
    retval = nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG);

    /* Second, Double */
    retval = nc_def_var(ncid, VAR_NAME2, NC_DOUBLE, 1, &dimid, &varid2);
    retval = nc_def_var_endian(ncid, varid2, NC_ENDIAN_BIG);

    /* Third, Int */
    retval = nc_def_var(ncid, VAR_NAME3, NC_INT, 1, &dimid, &varid3);
    retval = nc_def_var_endian(ncid, varid3, NC_ENDIAN_BIG);

    retval = nc_close(ncid);
  }

    /*
     * Create a file with the hdf5 api
     */
    {
        hid_t fileid, grpid, spaceid;
        hid_t native_did, le_did, be_did;
        hsize_t dims[1];
        printf("*** Creating a file via hdf5 API: %s\n",FILE_NAME_H5);

        if ((fileid = H5Fcreate(FILE_NAME_H5, H5F_ACC_TRUNC, H5P_DEFAULT,
                                    H5P_DEFAULT)) < 0) ERR;
        if ((grpid = H5Gcreate1(fileid, GRP_NAME, 0)) < 0) ERR;

        /* Create a dataset of native endian. */
           dims[0] = DIM_LEN;
           if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
           if ((native_did = H5Dcreate1(grpid, VAR_NAME3, H5T_STD_I32BE,
                                       spaceid, H5P_DEFAULT)) < 0) ERR;
           if ((le_did = H5Dcreate1(grpid, VAR_NAME, H5T_IEEE_F32BE,
                                   spaceid, H5P_DEFAULT)) < 0) ERR;
           if ((be_did = H5Dcreate1(grpid, VAR_NAME2, H5T_IEEE_F32BE,
                                   spaceid, H5P_DEFAULT)) < 0) ERR;

           if (H5Dclose(native_did) < 0 ||
               H5Dclose(le_did) < 0 ||
               H5Dclose(be_did) < 0 ||
               H5Sclose(spaceid) < 0 ||
               H5Gclose(grpid) < 0 ||
               H5Fclose(fileid) < 0)
             {
               printf("Error closing hdf5 file.\n");
               failures++;
           }


     }


    /*
     * 3. Reopen netcdf-generated file, check to see if the endianness attribute
     *    exists.
     */
    printf("** Checking test files.\n");
    {
    ncid = 0;
    printf("*** %s\n",FILE_NAME_NC);
    retval = nc_open(FILE_NAME_NC, NC_NETCDF4 | NC_NOWRITE, &ncid);
    retval = nc_set_fill(ncid, NC_NOFILL, &oldfill);
    retval = nc_inq_varid(ncid,VAR_NAME,&varid);
    retval = nc_inq_varid(ncid,VAR_NAME2,&varid2);
    retval = nc_inq_varid(ncid,VAR_NAME3,&varid3);

    retval = nc_inq_var_endian(ncid,varid,&ed);
    if(ed != NC_ENDIAN_BIG) {
      printf("\tTest 1: Error for float variable endianness: [%d] not NC_ENDIAN_BIG\n",ed);
      failures++;
    } else {
      printf("\tTest 1: [%d] is NC_ENDIAN_BIG, Success.\n",ed);
    }

    retval = nc_inq_var_endian(ncid,varid2,&ed2);
    if(ed2 != NC_ENDIAN_BIG) {
      printf("\tTest 2: Error for double variable endianness: [%d] not NC_ENDIAN_BIG\n",ed);
      failures++;
    } else {
      printf("\tTest 2: [%d] is NC_ENDIAN_BIG, Success.\n",ed2);
    }

    retval = nc_inq_var_endian(ncid,varid3,&ed3);
    if(ed3 != NC_ENDIAN_BIG) {
      printf("\tTest 3: Error for integer variable endianness: [%d] not NC_ENDIAN_BIG\n",ed2);
      failures++;
    } else {
      printf("\tTest 3: [%d] is NC_ENDIAN_BIG, Success.\n",ed3);
    }

  retval = nc_close(ncid);
  }


    /*
     * 4. Reopen hdf5-generated file, check to see if the endianness attribute
     *    exists.
     */
    {
    ncid = 0;
    printf("*** %s\n",FILE_NAME_H5);
    retval = nc_open(FILE_NAME_H5, NC_NETCDF4 | NC_NOWRITE, &ncid);
    retval = nc_inq_varid(ncid,VAR_NAME,&varid);
    retval = nc_inq_varid(ncid,VAR_NAME2,&varid2);
    retval = nc_inq_varid(ncid,VAR_NAME3,&varid3);

    retval = nc_inq_var_endian(ncid,varid,&ed);
    if(ed != NC_ENDIAN_BIG) {
      printf("\tTest 1: Error for float variable endianness: [%d] not NC_ENDIAN_BIG\n",ed);
      failures++;
    } else {
      printf("\tTest 1: [%d] is NC_ENDIAN_BIG, Success.\n",ed);
    }

    retval = nc_inq_var_endian(ncid,varid2,&ed2);
    if(ed2 != NC_ENDIAN_BIG) {
      printf("\tTest 2: Error for double variable endianness: [%d] not NC_ENDIAN_BIG\n",ed);
      failures++;
    } else {
      printf("\tTest 2: [%d] is NC_ENDIAN_BIG, Success.\n",ed2);
    }

    retval = nc_inq_var_endian(ncid,varid3,&ed3);
    if(ed3 != NC_ENDIAN_BIG) {
      printf("\tTest 3: Error for integer variable endianness: [%d] not NC_ENDIAN_BIG\n",ed2);
      failures++;
    } else {
      printf("\tTest 3: [%d] is NC_ENDIAN_BIG, Success.\n",ed3);
    }

  retval = nc_close(ncid);
  }
  printf("** Failures Returned: [%d]\n",failures);
  return failures;
}
