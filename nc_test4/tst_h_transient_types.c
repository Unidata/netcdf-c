/* This is part of the netCDF package.  Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program tests fixes for reading netCDF-4 files that contain
   datasets with reference datatypes.  The netCDF-4 library should ignore
   the datasets & attributes that have reference datatypes and allow the 
   rest of the file to be accessed.
*/

#include "netcdf.h"
#include <config.h>
#include <nc_tests.h>
#include <err_macros.h>
#include <hdf5.h>
#include <complex.h>

#define FILE_NAME "tst_h_transient.h5"
#define VAR_NAME "var"
#define INT_ATT_NAME "intatt"
#define INT_VAR_NAME "intvar"

int
main()
{
  double complex expected_z = 1 + 2*I;

  printf("\n*** Creating file with datasets that have transient datatypes.\n");
  {
	hid_t fileid, scalar_spaceid;
	hid_t dsetid, complex_dtype;

	/* Create new file, using default properties. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

    /* Create compound datatype, but don't commit to file */
    if ((complex_dtype = H5Tcreate(H5T_COMPOUND, sizeof(double complex))) < 0) ERR;
    if (H5Tinsert(complex_dtype, "r", 0, H5T_NATIVE_DOUBLE) < 0) ERR;
    if (H5Tinsert(complex_dtype, "i", sizeof(double), H5T_NATIVE_DOUBLE) < 0) ERR;

    /* Create dataset with transient datatype */
	if ((scalar_spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
    if ((dsetid = H5Dcreate2(fileid, VAR_NAME, complex_dtype, scalar_spaceid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

    /* Write complex number to file */
    if ((H5Dwrite(dsetid, complex_dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &expected_z)) <0) ERR;

    /* Close dataset */
    if (H5Dclose(dsetid) < 0) ERR;

    /* Close rest */
	if (H5Sclose(scalar_spaceid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;
  }

  printf("*** Checking accessing file through netCDF-4 API...");
  {
    int ncid, varid;
    double complex read_z;
    int num_types, class;
    int *typeids;
    nc_type base_nc_type;
    char name[NC_MAX_NAME];
    size_t size, nfields;


    nc_set_log_level(4);
    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

    if (nc_inq_typeids(ncid, &num_types, NULL)) ERR;
    typeids = (int*)malloc((size_t)num_types * sizeof(int));
    if (nc_inq_typeids(ncid, NULL, typeids)) ERR;

    if (nc_inq_user_type(ncid, typeids[0], name, &size, &base_nc_type, &nfields, &class)) ERR;

    /* Verify that the dataset is present */
    if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;

    /* Read complex variable */
    if (nc_get_var(ncid, varid, &read_z)) ERR;

    if (read_z != expected_z) ERR;

    if (nc_close(ncid)) ERR;
  }
  SUMMARIZE_ERR;

  FINAL_RESULTS;
}

