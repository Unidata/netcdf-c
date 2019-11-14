/* This is part of the netCDF package.  Copyright 2019 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of user Kai Mühlbauer.

   Test handling of anonymous dimensions when netCDF reads a HDF5
   file. This test partially contributed by user

   Ed Hartnett, 11/13/2019
*/
#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_interops_dims.h5"
#define DIM_LEN 100

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF5/NetCDF-4 interoperability handling of HDF5 dimensions.\n");
   printf("*** Checking handling of a HDF5 file with anon dims...");
   {
       hid_t       file_id, dataset_id, dataspace_id;  /* identifiers */
       hsize_t     dims[2];

       /* Create a new file using default properties. */
       if ((file_id = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0)
           ERR;

       /* Create the data space for the dataset. */
       dims[0] = DIM_LEN;
       dims[1] = DIM_LEN;
       if ((dataspace_id = H5Screate_simple(2, dims, NULL)) < 0)
           ERR;

       /* Create the dataset. */
       dataset_id = H5Dcreate2(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
                               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

       /* End access to the dataset and release resources used by it. */
       if (H5Dclose(dataset_id) < 0)
           ERR;

       /* Terminate access to the data space. */
       if (H5Sclose(dataspace_id) < 0)
           ERR;

       /* Close the file. */
       if (H5Fclose(file_id) < 0)
           ERR;

       /* Now open the file with netCDF. */
       {
           int ndims, nvars, ngatts, unlimdimid;
           int ncid;
           char dim_name[NC_MAX_NAME + 1];
           size_t dim_len;

           if (nc_open(FILE_NAME, NC_NOWRITE, &ncid))
               ERR;
           if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid))
               ERR;
           if (ndims != 2 || nvars != 1 || ngatts != 0 || unlimdimid != -1)
               ERR;
           if (nc_inq_dim(ncid, 0, dim_name, &dim_len))
               ERR;
           if (strcmp(dim_name, "phony_dim_0") || dim_len != DIM_LEN)
               ERR;
           if (nc_inq_dim(ncid, 1, dim_name, &dim_len))
               ERR;
           if (strcmp(dim_name, "phony_dim_1") || dim_len != DIM_LEN)
               ERR;
           if (nc_close(ncid))
               ERR;
       }
   }
   SUMMARIZE_ERR;
   printf("*** Checking handling of a HDF5 file with anon dims and two datasets...");
   {
       hid_t       file_id, dataset_id, dataset_id_2, dataspace_id;  /* identifiers */
       hsize_t     dims[2];

       /* Create a new file using default properties. */
       if ((file_id = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
                                H5P_DEFAULT)) < 0) ERR;

       /* Create the data space for the dataset. */
       dims[0] = DIM_LEN;
       dims[1] = DIM_LEN;
       if ((dataspace_id = H5Screate_simple(2, dims, NULL)) < 0) ERR;

       /* Create the datasets. */
       if ((dataset_id = H5Dcreate2(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
       if ((dataset_id_2 = H5Dcreate2(file_id, "/dset2", H5T_STD_I32BE, dataspace_id,
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

       if (H5Dclose(dataset_id) < 0) ERR;
       if (H5Dclose(dataset_id_2) < 0) ERR;
       if (H5Sclose(dataspace_id) < 0) ERR;
       if (H5Fclose(file_id) < 0) ERR;

       /* Now open the file with netCDF. */
       {
           int ndims, nvars, ngatts, unlimdimid;
           int ncid;
           char dim_name[NC_MAX_NAME + 1];
           size_t dim_len;

           if (nc_open(FILE_NAME, NC_NOWRITE, &ncid))
               ERR;
           if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid))
               ERR;
           if (ndims != 2 || nvars != 2 || ngatts != 0 || unlimdimid != -1)
               ERR;
           if (nc_inq_dim(ncid, 0, dim_name, &dim_len))
               ERR;
           if (strcmp(dim_name, "phony_dim_0") || dim_len != DIM_LEN)
               ERR;
           if (nc_inq_dim(ncid, 1, dim_name, &dim_len))
               ERR;
           if (strcmp(dim_name, "phony_dim_1") || dim_len != DIM_LEN)
               ERR;
           if (nc_close(ncid))
               ERR;
       }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
