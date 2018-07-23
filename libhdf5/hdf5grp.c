/* Copyright 2005-2018, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions. */
/**
 * @file @internal This file is part of netcdf-4, a netCDF-like
 * interface for HDF5, or a HDF5 backend for netCDF, depending on your
 * point of view.
 *
 * This file handles HDF5 groups.
 *
 * @author Ed Hartnett
 */

#include "config.h"
#include "hdf5internal.h"

/**
 * @internal Create a group. Its ncid is returned in the new_ncid
 * pointer.
 *
 * @param parent_ncid Parent group.
 * @param name Name of new group.
 * @param new_ncid Pointer that gets ncid for new group.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ESTRICTNC3 Classic model in use for this file.
 * @return ::NC_ENOTNC4 Not a netCDF-4 file.
 * @author Ed Hartnett
 */
int
NC4_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
   NC_GRP_INFO_T *grp, *g;
   NC_FILE_INFO_T *h5;
   char norm_name[NC_MAX_NAME + 1];
   int retval;

   LOG((2, "%s: parent_ncid 0x%x name %s", __func__, parent_ncid, name));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_grp_h5(parent_ncid, &grp, &h5)))
      return retval;
   assert(h5);

   /* Check and normalize the name. */
   if ((retval = nc4_check_name(name, norm_name)))
      return retval;

   /* Check that this name is not in use as a var, grp, or type. */
   if ((retval = nc4_check_dup_name(grp, norm_name)))
      return retval;

   /* No groups in netcdf-3! */
   if (h5->cmode & NC_CLASSIC_MODEL)
      return NC_ESTRICTNC3;

   /* If it's not in define mode, switch to define mode. */
   if (!(h5->flags & NC_INDEF))
      if ((retval = NC4_redef(parent_ncid)))
         return retval;

   /* Update internal lists to reflect new group. The actual HDF5
    * group creation will be done when metadata is written by a
    * sync. */
   if ((retval = nc4_grp_list_add(grp, norm_name, &g)))
      return retval;
   if (new_ncid)
      *new_ncid = grp->nc4_info->controller->ext_ncid | g->hdr.id;

   return NC_NOERR;
}

/**
 * @internal Rename a group.
 *
 * @param grpid Group ID.
 * @param name New name for group.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTNC4 Not a netCDF-4 file.
 * @return ::NC_EPERM File opened read-only.
 * @return ::NC_EBADGRPID Renaming root forbidden.
 * @return ::NC_EHDFERR HDF5 function returned error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
NC4_rename_grp(int grpid, const char *name)
{
   NC_GRP_INFO_T *grp, *parent;
   NC_FILE_INFO_T *h5;
   char norm_name[NC_MAX_NAME + 1];
   int retval;

   LOG((2, "nc_rename_grp: grpid 0x%x name %s", grpid, name));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_grp_h5(grpid, &grp, &h5)))
      return retval;
   assert(h5);

   if (h5->no_write)
      return NC_EPERM; /* attempt to write to a read-only file */

   /* Do not allow renaming the root group */
   if (grp->parent == NULL)
      return NC_EBADGRPID;
   parent = grp->parent;

   /* Check and normalize the name. */
   if ((retval = nc4_check_name(name, norm_name)))
      return retval;

   /* Check that this name is not in use as a var, grp, or type in the
    * parent group (i.e. the group that grp is in). */
   if ((retval = nc4_check_dup_name(parent, norm_name)))
      return retval;

   /* If it's not in define mode, switch to define mode. */
   if (!(h5->flags & NC_INDEF))
      if ((retval = NC4_redef(grpid)))
         return retval;

   /* Rename the group, if it exists in the file */
   if (grp->hdf_grpid)
   {
      /* Close the group */
      if (H5Gclose(grp->hdf_grpid) < 0)
         return NC_EHDFERR;
      grp->hdf_grpid = 0;

      /* Attempt to rename & re-open the group, if the parent group is open */
      if (grp->parent->hdf_grpid)
      {
         /* Rename the group */
         if (H5Gmove(parent->hdf_grpid, grp->hdr.name, name) < 0)
            return NC_EHDFERR;

         /* Reopen the group, with the new name */
         if ((grp->hdf_grpid = H5Gopen2(parent->hdf_grpid, name, H5P_DEFAULT)) < 0)
            return NC_EHDFERR;
      }
   }

   /* Give the group its new name in metadata. UTF8 normalization
    * has been done. */
   free(grp->hdr.name);
   if (!(grp->hdr.name = strdup(norm_name)))
      return NC_ENOMEM;
   grp->hdr.hashkey = NC_hashmapkey(grp->hdr.name,strlen(grp->hdr.name)); /* Fix hash key */

   if(!ncindexrebuild(parent->children))
      return NC_EINTERNAL;

   return NC_NOERR;
}
