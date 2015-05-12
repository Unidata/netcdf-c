/* Test introduced May 12, 2015 as part of debugging NCF-331,
   found in the Undiata JIRA system.  We are investigating
   how endianness is stored to a file.
*/

#include "h5_err_macros.h"
#include "hdf5.h"

#include <stdio.h>

#define FILE_NAME "tst_h_endian_float.h5"
#define NDIM 10
#define NLON 20

#define DIM_NAME "x"
#define VAR_NAME "jv"
#define VAR_NAME2 "fv"
#define VAR_NAME3 "iv"

#define NUM_STR 1
#define NDIMS 1
#define NFLOAT 1
#define NINT 1

#define GNAME "FloatBigEnds"

int main() {

  hid_t       fileid, grpid, plistid, spaceid, datasetid;
  hsize_t     dims[NDIMS] = {NUM_STR}, max_dims[NDIMS] = {H5S_UNLIMITED};
  float empty = -42.0;

  printf("\n*** Checking if HDF5 Endianness is properly stored.\n");

  /* Create file, open root group. */
  printf("\t* Creating File.\n");
  if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
                          H5P_DEFAULT)) < 0) ERR;

    printf("\t* Opening Group.\n");
  if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

  /* Create space for the dataset. */
  printf("\t* Creating Space.\n");
  if ((spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;

  /* Create dataset. */
  printf("\t* Creating Dataset.\n");
  if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;

  //printf("\t* Setting fill value..\n");
  //if (H5Pset_fill_value(plistid, H5T_NATIVE_FLOAT, &empty) < 0) ERR;
  printf("\t* Getting dataset id.\n");
  if ((datasetid = H5Dcreate1(grpid,GNAME,H5T_IEEE_F32BE,
                              spaceid,plistid)) < 0) ERR;

  printf("\t* Closing datasetid.\n");
  if (H5Dclose(datasetid) < 0) ERR;
    printf("\t* Closing plistid.\n");
  if (H5Pclose(plistid) < 0) ERR;
  printf("\t* Closing spaceid.\n");
  if (H5Sclose(spaceid) < 0) ERR;
    printf("\t* Closing grpid.\n");
  if (H5Gclose(grpid) < 0) ERR;
  printf("\t* Closing fileid.\n\n\n");
  if (H5Fclose(fileid) < 0) ERR;

  SUMMARIZE_ERR;
  FINAL_RESULTS;

}
