/* This is part of the netCDF package.  Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program does HDF5 string stuff.

   Here's a HDF5 sample programs:
   http://hdf.ncsa.uiuc.edu/training/other-ex5/sample-programs/strings.c
*/
#include <string.h>
#include "err_macros.h"
#include <hdf5.h>

#define FILE_NAME "ref_fixedstring.h5"
#define DIM1_LEN 4
#define ATT_NAME1 "att1"
#define ATT_NAMEN "attn"
#define GRP_NAME "group"

int
main()
{
   hid_t fileid, typeid;
   hid_t spaceid1, attid1;
   hid_t spaceidn, attidn;
   hid_t dataspaceid1, dataset1;
   hid_t dataspaceidn, datasetn;
   size_t type_size;

   const char data1[4] = "abcd";
   const char datan[16] = "abcdefghijklmnop";
   const hsize_t dims[1] =  {4};

   printf("\n*** Checking HDF5 fixed length string types.\n");

   printf("*** Create HDF5 Dataset ...");   

   /* Open file. */
   if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      
   /* Create fixed string type. */
   if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
   type_size = 4 * sizeof(char);
   if (H5Tset_size (typeid, type_size) < 0) ERR;
   if (H5Tset_strpad (typeid, H5T_STR_NULLPAD) < 0) ERR;
      
   /* Write a scalar attribute of this type. */
   if ((spaceid1 = H5Screate(H5S_SCALAR)) < 0) ERR;
   if ((attid1 = H5Acreate1(fileid, ATT_NAME1, typeid, spaceid1, H5P_DEFAULT)) < 0) ERR;
   if (H5Awrite(attid1, typeid, &data1) < 0) ERR;

   /* Write a vector attribute of this type. */
   if ((spaceidn = H5Screate_simple(1,dims,NULL)) < 0) ERR;
   if ((attidn = H5Acreate1(fileid, ATT_NAMEN, typeid, spaceidn, H5P_DEFAULT)) < 0) ERR;
   if (H5Awrite(attidn, typeid, &datan) < 0) ERR;

   /* Write a scalar variable of this type. */
   if ((dataspaceid1 = H5Screate(H5S_SCALAR)) < 0) ERR;
   dataset1 = H5Dcreate1(fileid, "v1", typeid, dataspaceid1, H5P_DEFAULT);
   if (H5Dwrite (dataset1, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data1) < 0) ERR;

   /* Write a vector variable of this type. */
   if ((dataspaceidn = H5Screate_simple(1,dims,NULL)) < 0) ERR;
   datasetn = H5Dcreate1(fileid, "vn", typeid, dataspaceidn, H5P_DEFAULT);
   if (H5Dwrite (datasetn, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, datan) < 0) ERR;

   /* Close everything up. */
   if (H5Dclose(datasetn) < 0) ERR;
   if (H5Sclose(dataspaceidn) < 0) ERR;
   if (H5Dclose(dataset1) < 0) ERR;
   if (H5Sclose(dataspaceid1) < 0) ERR;
   if (H5Aclose(attidn) < 0) ERR;
   if (H5Sclose(spaceidn) < 0) ERR;
   if (H5Aclose(attid1) < 0) ERR;
   if (H5Sclose(spaceid1) < 0) ERR;
   if (H5Tclose(typeid) < 0) ERR;
   if (H5Fclose(fileid) < 0) ERR;

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
