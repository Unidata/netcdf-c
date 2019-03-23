/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 dataset code, even more. These are not intended to be
   exhaustive tests, but they use HDF5 the same way that netCDF-4
   does, so if these tests don't work, than netCDF-4 won't work
   either.

   This test does some renames in HDF5 to see how it works.

   Ed Hartnett 2/1/19
*/

#include "h5_err_macros.h"
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_rename.h5"
#define STR_LEN 255

int
main()
{
   printf("\n*** Checking HDF5 variable renaming.\n");
   printf("*** Checking HDF5 variable ordering after renames...\n");

#define NUM_ELEMENTS 6
#define MAX_SYMBOL_LEN 2
#define ELEMENTS_NAME "Elements"
   {
      hid_t did[NUM_ELEMENTS], fapl_id, fcpl_id, gcpl_id;
      hsize_t num_obj;
      hid_t fileid, grpid, spaceid;
      int i;
      H5O_info_t obj_info;
      char names[NUM_ELEMENTS][MAX_SYMBOL_LEN + 1] = {"H", "He", "Li", "Be", "B", "C"};
      char names2[NUM_ELEMENTS][MAX_SYMBOL_LEN + 1] = {"h", "He", "Li", "Be", "B", "C"};
      char name[MAX_SYMBOL_LEN + 1];
      ssize_t size;

      /* Create file, setting latest_format in access propertly list
       * and H5P_CRT_ORDER_TRACKED in the creation property list. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Create group, with link_creation_order set in the group
       * creation property list. */
      if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((grpid = H5Gcreate_anon(fileid, gcpl_id, H5P_DEFAULT)) < 0) ERR;
      if ((H5Olink(grpid, fileid, ELEMENTS_NAME, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Create a scalar space. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;

      /* Create the variables, one per element. */
      for (i = 0; i < NUM_ELEMENTS; i++)
      {
	 if ((did[i] = H5Dcreate(grpid, names[i], H5T_NATIVE_INT,
				 spaceid, H5P_DEFAULT)) < 0) ERR;
	 if (H5Dclose(did[i]) < 0) ERR;
      }

      if (H5Pclose(fapl_id) < 0 ||
	  H5Pclose(gcpl_id) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check the order. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, ELEMENTS_NAME)) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_ELEMENTS) ERR;
      printf("Original order:\n");
      for (i = 0; i < num_obj; i++)
      {
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
				i, &obj_info, H5P_DEFAULT) < 0) ERR;
	 if (obj_info.type != H5O_TYPE_DATASET) ERR;
	 if ((size = H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
					NULL, 0, H5P_DEFAULT)) < 0) ERR;
	 H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
				name, size+1, H5P_DEFAULT);
	 if (strcmp(name, names[i])) ERR;
         printf("name %s\n", name);
      }

      /* Rename the first dataset. */
      if (H5Lmove(grpid, names[0], grpid, names2[0], H5P_DEFAULT, H5P_DEFAULT) < 0)
          ERR;

      if (H5Pclose(fapl_id) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check the order again. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, ELEMENTS_NAME)) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_ELEMENTS) ERR;
      printf("New order:\n");
      for (i = 0; i < num_obj; i++)
      {
         if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
        			i, &obj_info, H5P_DEFAULT) < 0) ERR;
         if (obj_info.type != H5O_TYPE_DATASET) ERR;
         if ((size = H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
        				NULL, 0, H5P_DEFAULT)) < 0) ERR;
         H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
        			name, size+1, H5P_DEFAULT);
         printf("name %s\n", name);
         /* if (strcmp(name, names2[i])) ERR; */
      }
      if (H5Pclose(fapl_id) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0)
         ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
