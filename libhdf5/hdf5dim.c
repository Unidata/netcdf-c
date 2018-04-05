/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions. */
/**
 * @file
 * @internal This file is part of netcdf-4, a netCDF-like interface
 * for HDF5, or a HDF5 backend for netCDF, depending on your point of
 * view.
 *
 * This file handles the nc4 dimension functions.
 *
 * @author Ed Hartnett
 */

#include "nc4internal.h"
#include "nc4dispatch.h"

/**
 * @internal Netcdf-4 files might have more than one unlimited
 * dimension, but return the first one anyway.
 *
 * @note that this code is inconsistent with nc_inq
 *
 * @param ncid File and group ID.
 * @param unlimdimidp Pointer that gets ID of first unlimited
 * dimension, or -1.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett
 */
int
NC4_inq_unlimdim(int ncid, int *unlimdimidp)
{
   NC *nc;
   NC_GRP_INFO_T *grp, *g;
   NC_HDF5_FILE_INFO_T *h5;
   NC_DIM_INFO_T *dim;
   int found = 0;
   int retval;
   int i;

   LOG((2, "%s: called", __func__));

   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(h5 && nc && grp);

   if (unlimdimidp)
   {
      /* According to netcdf-3 manual, return -1 if there is no unlimited
         dimension. */
      *unlimdimidp = -1;
      for (g = grp; g && !found; g = g->parent)
      {
	 for(i=0;i<ncindexsize(grp->dim);i++)
         {
	    dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
	    if(dim == NULL) continue;
            if (dim->unlimited)
            {
               *unlimdimidp = dim->hdr.id;
               found++;
               break;
            }
         }
      }
   }

   return NC_NOERR;
}

/**
 * @internal Dimensions are defined in attributes attached to the
 * appropriate group in the data file.
 *
 * @param ncid File and group ID.
 * @param name Name of the new dimension.
 * @param len Length of the new dimension.
 * @param idp Pointer that gets the ID of the new dimension. Ignored
 * if NULL.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @return ::NC_EINVAL Invalid input.
 * @return ::NC_EPERM Read-only file.
 * @return ::NC_EUNLIMIT Only one unlimited dim for classic model.
 * @return ::NC_ENOTINDEFINE Not in define mode.
 * @return ::NC_EDIMSIZE Dim length too large.
 * @return ::NC_ENAMEINUSE Name already in use in group.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
NC4_def_dim(int ncid, const char *name, size_t len, int *idp)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_HDF5_FILE_INFO_T *h5;
   NC_DIM_INFO_T *dim;
   char norm_name[NC_MAX_NAME + 1];
   int retval = NC_NOERR;
   int i;

   LOG((2, "%s: ncid 0x%x name %s len %d", __func__, ncid, name,
        (int)len));

   /* Find our global metadata structure. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(h5 && nc && grp);

   /* If the file is read-only, return an error. */
   if (h5->no_write)
      return NC_EPERM;

   /* Check some stuff if strict nc3 rules are in effect. */
   if (h5->cmode & NC_CLASSIC_MODEL)
   {
      /* Only one limited dimenson for strict nc3. */
      if (len == NC_UNLIMITED) {
	 for(i=0;i<ncindexsize(grp->dim);i++) {
	    dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
	    if(dim == NULL) continue;
            if (dim->unlimited)
               return NC_EUNLIMIT;
	}
      }
      /* Must be in define mode for stict nc3. */
      if (!(h5->flags & NC_INDEF))
         return NC_ENOTINDEFINE;
   }

   /* Make sure this is a valid netcdf name. */
   if ((retval = nc4_check_name(name, norm_name)))
      return retval;

   /* For classic model: dim length has to fit in a 32-bit unsigned
    * int, as permitted for 64-bit offset format. */
   if (h5->cmode & NC_CLASSIC_MODEL)
      if(len > X_UINT_MAX) /* Backward compat */
         return NC_EDIMSIZE;

   /* Make sure the name is not already in use. */
   dim = (NC_DIM_INFO_T*)ncindexlookup(grp->dim,norm_name);
   if(dim != NULL)
         return NC_ENAMEINUSE;

   /* If it's not in define mode, enter define mode. Do this only
    * after checking all input data, so we only enter define mode if
    * input is good. */
   if (!(h5->flags & NC_INDEF))
      if ((retval = NC4_redef(ncid)))
         return retval;

   /* Add a dimension to the list. The ID must come from the file
    * information, since dimids are visible in more than one group. */
   if ((retval = nc4_dim_list_add(grp, norm_name, len, -1, &dim)))
      return retval;

   /* Pass back the dimid. */
   if (idp)
      *idp = dim->hdr.id;

   return retval;
}

/**
 * @internal Given dim name, find its id.
 *
 * @param ncid File and group ID.
 * @param name Name of the dimension to find.
 * @param idp Pointer that gets dimension ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EBADDIM Dimension not found.
 * @return ::NC_EINVAL Invalid input. Name must be provided.
 * @author Ed Hartnett
 */
int
NC4_inq_dimid(int ncid, const char *name, int *idp)
{
   NC *nc;
   NC_GRP_INFO_T *grp, *g;
   NC_HDF5_FILE_INFO_T *h5;
   NC_DIM_INFO_T *dim;
   char norm_name[NC_MAX_NAME + 1];
   int retval;
   int found;

   LOG((2, "%s: ncid 0x%x name %s", __func__, ncid, name));

   /* Check input. */
   if (!name)
      return NC_EINVAL;

   /* Find metadata for this file. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(h5 && nc && grp);

   /* Normalize name. */
   if ((retval = nc4_normalize_name(name, norm_name)))
      return retval;

   /* check for a name match in this group and its parents */
   found = 0;
   for (g = grp; g ; g = g->parent) {
       dim = (NC_DIM_INFO_T*)ncindexlookup(g->dim,norm_name);
       if(dim != NULL) {found = 1; break;}
   }
   if(!found)
      return NC_EBADDIM;
   assert(dim != NULL);
   if (idp)
	*idp = dim->hdr.id;
   return NC_NOERR;
}

/**
 * @internal Find out name and len of a dim. For an unlimited
 * dimension, the length is the largest length so far written. If the
 * name of lenp pointers are NULL, they will be ignored.
 *
 * @param ncid File and group ID.
 * @param dimid Dimension ID.
 * @param name Pointer that gets name of the dimension.
 * @param lenp Pointer that gets length of dimension.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EDIMSIZE Dimension length too large.
 * @return ::NC_EBADDIM Dimension not found.
 * @author Ed Hartnett
 */
int
NC4_inq_dim(int ncid, int dimid, char *name, size_t *lenp)
{
   NC *nc;
   NC_HDF5_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp, *dim_grp;
   NC_DIM_INFO_T *dim;
   int ret = NC_NOERR;

   LOG((2, "%s: ncid 0x%x dimid %d", __func__, ncid, dimid));

   /* Find our global metadata structure. */
   if ((ret = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return ret;
   assert(h5 && nc && grp);

   /* Find the dimension and its home group. */
   if ((ret = nc4_find_dim(grp, dimid, &dim, &dim_grp)))
      return ret;
   assert(dim);

   /* Return the dimension name, if the caller wants it. */
   if (name && dim->hdr.name)
      strcpy(name, dim->hdr.name);

   /* Return the dimension length, if the caller wants it. */
   if (lenp)
   {
      if (dim->unlimited)
      {
         /* Since this is an unlimited dimension, go to the file
            and see how many records there are. Take the max number
            of records from all the vars that share this
            dimension. */
         *lenp = 0;
         if ((ret = nc4_find_dim_len(dim_grp, dimid, &lenp)))
            return ret;
      }
      else
      {
         if (dim->too_long)
         {
            ret = NC_EDIMSIZE;
            *lenp = NC_MAX_UINT;
         }
         else
            *lenp = dim->len;
      }
   }

   return ret;
}

/**
 * @internal Rename a dimension, for those who like to prevaricate.
 *
 * @param ncid File and group ID.
 * @param dimid Dimension ID.
 * @param name New dimension name.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EHDFERR HDF5 returned error.
 * @return ::NC_ENOMEM Out of memory.
 * @return ::NC_EINVAL Name must be provided.
 * @return ::NC_ENAMEINUSE Name is already in use in group.
 * @return ::NC_EMAXNAME Name is too long.
 * @return ::NC_EBADDIM Dimension not found.
 * @return ::NC_EBADNAME Name breaks netCDF name rules.
 * @return ::NC_EDIMMETA Unable to delete HDF5 dataset.
 * @author Ed Hartnett
 */
int
NC4_rename_dim(int ncid, int dimid, const char *name)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_DIM_INFO_T *dim, *tmpdim;
   NC_HDF5_FILE_INFO_T *h5;
   char norm_name[NC_MAX_NAME + 1];
   int retval;

   /* Note: name is new name */
   if (!name)
      return NC_EINVAL;

   LOG((2, "%s: ncid 0x%x dimid %d name %s", __func__, ncid,
        dimid, name));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc && h5 && grp);

   /* Trying to write to a read-only file? No way, Jose! */
   if (h5->no_write)
      return NC_EPERM;

   /* Make sure this is a valid netcdf name. */
   if ((retval = nc4_check_name(name, norm_name)))
      return retval;

   /* Get the original dim */
   if((retval=nc4_find_dim(grp,dimid,&dim,NULL)) != NC_NOERR)
	return retval;
   if(dim == NULL) /* No such dim */
	return NC_EBADDIM;

   /* Check if new name is in use */
   tmpdim = (NC_DIM_INFO_T*)ncindexlookup(grp->dim,norm_name);
   if(tmpdim != NULL)
	return NC_ENAMEINUSE;

   /* Check for renaming dimension w/o variable */
   if (dim->hdf_dimscaleid)
   {
      /* Sanity check */
      assert(!dim->coord_var);
      LOG((3, "dim %s is a dim without variable", dim->hdr.name));

      /* Delete the dimscale-only dataset. */
      if ((retval = delete_existing_dimscale_dataset(grp, dimid, dim)))
         return retval;
   }

   /* Give the dimension its new name in metadata. UTF8 normalization
    * has been done. */
   assert(dim->hdr.name);
   free(dim->hdr.name);
   if (!(dim->hdr.name = strdup(norm_name)))
      return NC_ENOMEM;
   LOG((3, "dim is now named %s", dim->hdr.name));
   dim->hdr.hashkey = NC_hashmapkey(dim->hdr.name,strlen(dim->hdr.name)); /* Fix hash key */

   if(!ncindexrebuild(grp->dim))
	return NC_EINTERNAL;

   /* Check if dimension was a coordinate variable, but names are
    * different now */
   if (dim->coord_var && strcmp(dim->hdr.name, dim->coord_var->hdr.name))
   {
      /* Break up the coordinate variable */
      if ((retval = nc4_break_coord_var(grp, dim->coord_var, dim)))
         return retval;
   }

   /* Check if dimension should become a coordinate variable. */
   if (!dim->coord_var)
   {
      NC_VAR_INFO_T *var;

      /* Attempt to find a variable with the same name as the
       * dimension in the current group. */
      if ((retval = nc4_find_var(grp, dim->hdr.name, &var)))
         return retval;

      /* Check if we found a variable and the variable has the
       * dimension in index 0. */
      if (var && var->dim[0] == dim)
      {
         /* Sanity check */
         assert(var->dimids[0] == dim->hdr.id);

         /* Reform the coordinate variable. */
         if ((retval = nc4_reform_coord_var(grp, var, dim)))
            return retval;
      }
   }

   return NC_NOERR;
}

/**
 * @internal Returns an array of unlimited dimension ids.The user can
 * get the number of unlimited dimensions by first calling this with
 * NULL for the second pointer.
 *
 * @param ncid File and group ID.
 * @param nunlimdimsp Pointer that gets the number of unlimited
 * dimensions. Ignored if NULL.
 * @param unlimdimidsp Pointer that gets arrray of unlimited dimension
 * ID. Ignored if NULL.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC4_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp)
{
   NC_DIM_INFO_T *dim;
   NC_GRP_INFO_T *grp;
   NC *nc;
   NC_HDF5_FILE_INFO_T *h5;
   int num_unlim = 0;
   int retval;
   int i;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(h5 && nc && grp);

   /* Get our dim info. */
   assert(h5);
   {
      for(i=0;i<ncindexsize(grp->dim);i++) 
      {
	 dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
	 if(dim == NULL) continue;
         if (dim->unlimited)
         {
            if (unlimdimidsp)
               unlimdimidsp[num_unlim] = dim->hdr.id;
            num_unlim++;
         }
      }
   }

   /* Give the number if the user wants it. */
   if (nunlimdimsp)
      *nunlimdimsp = num_unlim;

   return NC_NOERR;
}
