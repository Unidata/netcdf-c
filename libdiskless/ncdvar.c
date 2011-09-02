/* \file \internal
Variables for the diskless API.

Copyright 2011, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.
*/

#include <nc4internal.h>
#include "ncddispatch.h"
#include <math.h>

/* Min and max deflate levels tolerated by HDF5. */
#define MIN_DEFLATE_LEVEL 0
#define MAX_DEFLATE_LEVEL 9

/* This is to track opened HDF5 objects to make sure they are
 * closed. */
#ifdef EXTRA_TESTS
extern int num_plists;
#endif /* EXTRA_TESTS */

/* One meg is the minimum buffer size. */
#define ONE_MEG 1048576

/* Szip options. */
#define NC_SZIP_EC_OPTION_MASK 4
#define NC_SZIP_NN_OPTION_MASK 32
#define NC_SZIP_MAX_PIXELS_PER_BLOCK 32

int nc4_get_default_fill_value(NC_TYPE_INFO_T *type_info, void *fill_value);

/* Set chunk cache size for a variable. */
int
NCD_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, 
			float preemption)
{
   return NC_EINVAL;
}

/* Get chunk cache size for a variable. */
int
NCD_get_var_chunk_cache(int ncid, int varid, size_t *sizep, 
			size_t *nelemsp, float *preemptionp)
{
   return NC_EINVAL;
}

/* Create a new variable to hold user data. This is what it's all
 * about baby! */
int
NCD_def_var(int ncid, const char *name, nc_type xtype, int ndims, 
           const int *dimidsp, int *varidp)
{
   NC_FILE_INFO_T *nc;

   LOG((2, "NCD_def_var: ncid 0x%x name %s xtype %d ndims %d",
        ncid, name, xtype, ndims));

   /* If there are dimensions, I need their ids. */
   if (ndims && !dimidsp)
      return NC_EINVAL;

   /* Find metadata for this file. */
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Netcdf-3 cases handled by dispatch layer. */
   assert(nc->nc4_info);

   /* Handle netcdf-4 cases. */
   {
      NC_GRP_INFO_T *grp;
      NC_VAR_INFO_T *var;
      NC_DIM_INFO_T *dim;
      NC_HDF5_FILE_INFO_T *h5;
      NC_TYPE_INFO_T *type_info;
      char norm_name[NC_MAX_NAME + 1];
      int new_varid = 0;
      int num_unlim = 0;
      int d;
      size_t num_values = 1;
      int retval;

      /* Find info for this file and group, and set pointer to each. */
      if ((retval = nc4_find_grp_h5(ncid, &grp, &h5)))
	 return retval;
      assert(grp && h5);

      /* If it's not in define mode, strict nc3 files error out,
       * otherwise switch to define mode. */
      if (!(h5->flags & NC_INDEF))
      {
	 if (h5->cmode & NC_CLASSIC_MODEL)
	    return NC_ENOTINDEFINE;
	 if ((retval = NCD_redef(ncid)))
	    return retval;
      }

      /* Check and normalize the name. */
      if ((retval = nc4_check_name(name, norm_name)))
	 return retval;

      /* Not a Type is, well, not a type.*/
      if (xtype == NC_NAT)
	 return NC_EBADTYPE;
   
      /* For classic files, only classic types are allowed. */
      if (h5->cmode & NC_CLASSIC_MODEL && xtype > NC_DOUBLE)
	 return NC_ESTRICTNC3;

      /* If this is a user defined type, find it. */
      if (xtype > NC_STRING)
	 if ((retval = nc4_find_type(grp->file->nc4_info, xtype, &type_info)))
	    return NC_EBADTYPE;

      /* cast needed for braindead systems with signed size_t */
      if((unsigned long) ndims > X_INT_MAX) /* Backward compat */
	 return NC_EINVAL;

      /* Classic model files have a limit on number of vars. */
      if(h5->cmode & NC_CLASSIC_MODEL && h5->nvars >= NC_MAX_VARS)
	 return NC_EMAXVARS;

      /* Check that this name is not in use as a var, grp, or type. */
      if ((retval = nc4_check_dup_name(grp, norm_name)))
	 return retval;

      /* Get the new varid. */
      for (var = grp->var; var; var = var->next)
	 new_varid++;

      /* Check all the dimids to make sure they exist. */
      for (d = 0; d < ndims; d++)
      {
	 if ((retval = nc4_find_dim(grp, dimidsp[d], &dim, NULL)))
	    return retval;
	 if (dim->unlimited)
	    num_unlim++;
	 else
	    num_values *= dim->len;
      }

      /* These degrubbing messages sure are handy! */
      LOG((3, "nc_def_var_nc4: name %s type %d ndims %d", norm_name, xtype, ndims));
#ifdef LOGGING
      {
	 int dd;
	 for (dd = 0; dd < ndims; dd++)
	    LOG((4, "dimid[%d] %d", dd, dimidsp[dd]));
      }
#endif

      /* Add the var to the end of the list. */
      if ((retval = nc4_var_list_add(&grp->var, &var)))
	 return retval;

      /* Now fill in the values in the var info structure. */
      if (!(var->name = malloc((strlen(norm_name) + 1) * sizeof(char))))
	 return NC_ENOMEM;
      strcpy(var->name, norm_name);
      var->varid = grp->nvars++;
      var->xtype = xtype;
      var->ndims = ndims;
      var->dirty++;
   
      /* If this is a user-defined type, there is a type_info stuct with
       * all the type information. For atomic types, fake up a type_info
       * struct. */
      if (xtype > NC_STRING)
	 var->type_info = type_info;
      else
      {
	 if (!(var->type_info = calloc(1, sizeof(NC_TYPE_INFO_T))))
	    return NC_ENOMEM;
	 var->type_info->nc_typeid = xtype;
	 if ((retval = nc4_get_hdf_typeid(h5, var->xtype, &var->type_info->hdf_typeid, 
					  var->type_info->endianness)))
	    return retval;
	 if ((var->type_info->native_typeid = H5Tget_native_type(var->type_info->hdf_typeid, 
								 H5T_DIR_DEFAULT)) < 0)
	    return NC_EHDFERR;
	 if ((retval = nc4_get_typelen_mem(h5, var->type_info->nc_typeid, 0, 
					   &var->type_info->size)))
	    return retval;
      }
      if (!num_unlim)
	 var->contiguous = 1;

      /* Allocate space for dimension information. */
      if (ndims)
      {
	 if (!(var->dim = calloc(ndims, sizeof(NC_DIM_INFO_T *))))
	    return NC_ENOMEM;
	 if (!(var->dimids = calloc(ndims, sizeof(int))))
	    return NC_ENOMEM;
      }

      /* Allocate space for the data. */
      if (!(var->diskless_data = malloc(num_values * var->type_info->size)))
	 return NC_ENOMEM;

      /* At the same time, check to see if this is a coordinate
       * variable. If so, it will have the same name as one of its
       * dimensions. If it is a coordinate var, is it a coordinate var in
       * the same group as the dim? */
      for (d = 0; d < ndims; d++)
      {
	 NC_GRP_INFO_T *dim_grp;
	 if ((retval = nc4_find_dim(grp, dimidsp[d], &dim, &dim_grp)))
	    return retval;
	 if (strcmp(dim->name, norm_name) == 0 && dim_grp == grp && d == 0)
	 {
	    var->dimscale++;
	    dim->coord_var = var;
	    dim->coord_var_in_grp++;
	 }
	 var->dimids[d] = dimidsp[d];
	 var->dim[d] = dim;
      }

      /* If the user names this variable the same as a dimension, but
       * doesn't use that dimension first in its list of dimension ids,
       * is not a coordinate variable. I need to change its HDF5 name,
       * because the dimension will cause a HDF5 dataset to be created,
       * and this var has the same name. */
      for (dim = grp->dim; dim; dim = dim->next)
	 if (!strcmp(dim->name, norm_name) && 
	     (!var->ndims || dimidsp[0] != dim->dimid))
	 {
	    /* Set a different hdf5 name for this variable to avoid name
	     * clash. */
	    if (strlen(norm_name) + strlen(NON_COORD_PREPEND) > NC_MAX_NAME)
	       return NC_EMAXNAME;
	    if (!(var->hdf5_name = malloc((strlen(NON_COORD_PREPEND) + 
					   strlen(norm_name) + 1) * sizeof(char))))
	       return NC_ENOMEM;
	 
	    sprintf(var->hdf5_name, "%s%s", NON_COORD_PREPEND, norm_name);
	 }

      /* If this is a coordinate var, it is marked as a HDF5 dimension
       * scale. (We found dim above.) Otherwise, allocate space to
       * remember whether dimension scales have been attached to each
       * dimension. */
      if (!var->dimscale && ndims)
	 if (ndims && !(var->dimscale_attached = calloc(ndims, sizeof(int))))
	    return NC_ENOMEM;
      
      /* Return the varid. */
      if (varidp)
	 *varidp = var->varid;
      LOG((4, "new varid %d", var->varid));
   }
   return NC_NOERR;
}

/* Get all the information about a variable. Pass NULL for whatever
 * you don't care about. This is an internal function, not exposed to
 * the user. */
int 
NCD_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep, 
               int *ndimsp, int *dimidsp, int *nattsp, 
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp, 
               int *no_fill, void *fill_valuep, int *endiannessp, 
	       int *options_maskp, int *pixels_per_blockp)
{
   NC_FILE_INFO_T *nc;
   NC_GRP_INFO_T *grp; 
   NC_HDF5_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   NC_ATT_INFO_T *att;
   int natts=0;
   size_t type_size;
   int d;
   int retval;

   LOG((2, "nc_inq_var_all: ncid 0x%x varid %d", ncid, varid));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc && grp && h5);

   /* Walk through the list of vars, and return the info about the one
      with a matching varid. If the varid is -1, find the global
      atts and call it a day. */
   if (varid == NC_GLOBAL)
   {
      if (nattsp)
      {
         for (att = grp->att; att; att = att->next)
            natts++;
         *nattsp = natts;
      }
      return NC_NOERR;
   }

   /* Find the var. */
   for (var = grp->var; var; var = var->next)
      if (var->varid == varid)
         break;
   
   /* Oh no! Maybe we couldn't find it (*sob*)! */
   if (!var)
      return NC_ENOTVAR;
   
   /* Copy the data to the user's data buffers. */
   if (name)
      strcpy(name, var->name);
   if (xtypep)
      *xtypep = var->xtype;
   if (ndimsp)
      *ndimsp = var->ndims;
   if (dimidsp)
      for (d = 0; d < var->ndims; d++)
         dimidsp[d] = var->dimids[d];
   if (nattsp)
   {
      for (att = var->att; att; att = att->next)
         natts++;
      *nattsp = natts;
   }
   
   /* Chunking stuff. */
   if (!var->contiguous && chunksizesp)
      for (d = 0; d < var->ndims; d++)
      {
         chunksizesp[d] = var->chunksizes[d];
         LOG((4, "chunksizesp[%d]=%d", d, chunksizesp[d]));
      }

   if (contiguousp)
      *contiguousp = var->contiguous ? NC_CONTIGUOUS : NC_CHUNKED;

   /* Filter stuff. */
   if (deflatep)
      *deflatep = var->deflate;
   if (deflate_levelp)
      *deflate_levelp = var->deflate_level;
   if (shufflep)
      *shufflep = var->shuffle;
   if (fletcher32p)
      *fletcher32p = var->fletcher32;
   if (options_maskp)
      *options_maskp = var->options_mask;
   if (pixels_per_blockp)
      *pixels_per_blockp = var->pixels_per_block;

   /* Fill value stuff. */
   if (no_fill)
      *no_fill = var->no_fill;

   /* Don't do a thing with fill_valuep if no_fill mode is set for
    * this var, or if fill_valuep is NULL. */
   if (!var->no_fill && fill_valuep)
   {
      /* Do we have a fill value for this var? */
      if (var->fill_value)
      {
         if ((retval = nc4_get_typelen_mem(grp->file->nc4_info, var->xtype, 0, &type_size)))
            return retval;
         memcpy(fill_valuep, var->fill_value, type_size);
      }
      else
      {
         if ((retval = nc4_get_default_fill_value(var->type_info, fill_valuep)))
            return retval;
      }
   }

   /* Does the user want the endianness of this variable? */
   if (endiannessp)
      *endiannessp = var->type_info->endianness;

   return NC_NOERR;
}

/* This functions sets extra stuff about a netCDF-4 variable which
   must be set before the enddef but after the def_var. This is an
   internal function, deliberately hidden from the user so that we can
   change the prototype of this functions without changing the API. */
static int
nc_def_var_extra(int ncid, int varid, int *shuffle, int *deflate, 
		 int *deflate_level, int *fletcher32, int *contiguous, 
		 const size_t *chunksizes, int *no_fill, 
                 const void *fill_value, int *endianness, 
		 int *options_mask, int *pixels_per_block)
{
   NC_FILE_INFO_T *nc;
   NC_GRP_INFO_T *grp; 
   NC_HDF5_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   NC_DIM_INFO_T *dim;
   size_t type_size;
   int d;
   int retval;

   LOG((2, "nc_def_var_extra: ncid 0x%x varid %d", ncid, varid));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   /* Attempting to do any of these things on a netCDF-3 file produces
    * an error. */
   if (!h5)
      return NC_ENOTNC4;

   assert(nc && grp && h5);

   /* Find the var. */
   for (var = grp->var; var; var = var->next)
      if (var->varid == varid)
         break;
   
   /* Oh no! Maybe we couldn't find it (*sob*)! */
   if (!var)
      return NC_ENOTVAR;

   /* Can't turn on contiguous and deflate/fletcher32/szip. */
   if (contiguous)
      if ((*contiguous != NC_CHUNKED && deflate) || 
	  (*contiguous != NC_CHUNKED && fletcher32) ||
	  (*contiguous != NC_CHUNKED && options_mask))
	 return NC_EINVAL;

   /* If the HDF5 dataset has already been created, then it is too
    * late to set all the extra stuff. */
   if (var->created)
      return NC_ELATEDEF;

   /* Check compression options. */
   if ((deflate && options_mask) ||
       (deflate && !deflate_level) ||
       (options_mask && !pixels_per_block))
      return NC_EINVAL;      
       
   /* Valid deflate level? */
   if (deflate && deflate_level)
   {
      if (*deflate)
         if (*deflate_level < MIN_DEFLATE_LEVEL ||
             *deflate_level > MAX_DEFLATE_LEVEL)
            return NC_EINVAL;
      if (var->options_mask)
            return NC_EINVAL;

      /* For scalars, just ignore attempt to deflate. */
      if (!var->ndims)
            return NC_NOERR;

      /* Well, if we couldn't find any errors, I guess we have to take
       * the users settings. Darn! */
      var->contiguous = 0;
      var->deflate = *deflate;
      if (*deflate)
         var->deflate_level = *deflate_level;
      LOG((3, "nc_def_var_extra: *deflate_level %d", *deflate_level));      
   }

   /* Szip in use? */
   if (options_mask)
   {
#ifndef USE_SZIP
      return NC_EINVAL;
#endif
      if (var->deflate)
	 return NC_EINVAL;
      if ((*options_mask != NC_SZIP_EC_OPTION_MASK) &&
	  (*options_mask != NC_SZIP_NN_OPTION_MASK))
	 return NC_EINVAL;
      if ((*pixels_per_block > NC_SZIP_MAX_PIXELS_PER_BLOCK) ||
	  (var->type_info->nc_typeid >= NC_STRING))
	 return NC_EINVAL;
      var->options_mask = *options_mask;
      var->pixels_per_block = *pixels_per_block;
      var->contiguous = 0;
   }

   /* Shuffle filter? */
   if (shuffle)
   {
      var->shuffle = *shuffle;
      var->contiguous = 0;
   }

   /* Fltcher32 checksum error protection? */
   if (fletcher32)
   {
      var->fletcher32 = *fletcher32;
      var->contiguous = 0;
   }
   
   /* Does the user want a contiguous dataset? Not so fast! Make sure
    * that there are no unlimited dimensions, and no filters in use
    * for this data. */
   if (contiguous && *contiguous)
   {
      if (var->deflate || var->fletcher32 || var->shuffle || var->options_mask)
	 return NC_EINVAL;
      
      for (d = 0; d < var->ndims; d++)
      {
	 if ((retval = nc4_find_dim(grp, var->dimids[d], &dim, NULL)))
	    return retval;
	 if (dim->unlimited)
	    return NC_EINVAL;
      }

      var->contiguous = NC_CONTIGUOUS;
   }

   /* Chunksizes anyone? */
   if (contiguous && *contiguous == NC_CHUNKED)
   {
      var->contiguous = 0;
   }

   /* Are we setting a fill modes? */
   if (no_fill)
   {
      if (*no_fill)
         var->no_fill = 1;
      else
         var->no_fill = 0;
   }

   /* Are we setting a fill value? */
   if (fill_value && !var->no_fill)
   {
      /* If fill value hasn't been set, allocate space. */
      if ((retval = nc4_get_typelen_mem(h5, var->xtype, 0, &type_size)))
         return retval;
      if (!var->fill_value)
         if (!(var->fill_value = malloc(type_size)))
            return NC_ENOMEM;

      /* Copy the fill_value. */
      LOG((4, "Copying fill value into metadata for variable %s", 
           var->name));
      memcpy(var->fill_value, fill_value, type_size);

      /* If there's a _FillValue attribute, delete it. */
      retval = nc_del_att(ncid, varid, _FillValue);
      if (retval && retval != NC_ENOTATT)
         return retval;

      /* Create a _FillValue attribute. */
      if ((retval = nc_put_att(ncid, varid, _FillValue, var->xtype, 1, fill_value)))
         return retval;
   }

   /* Is the user setting the endianness? */
   if (endianness)
      var->type_info->endianness = *endianness;

   return NC_NOERR;
}

/* Set the deflate level for a var, lower is faster, higher is
 * better. Must be called after nc_def_var and before nc_enddef or any
 * functions which writes data to the file. */
int
NCD_def_var_deflate(int ncid, int varid, int shuffle, int deflate, 
                   int deflate_level)
{
   return nc_def_var_extra(ncid, varid, &shuffle, &deflate, 
                           &deflate_level, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

/* Set checksum for a var. This must be called after the nc_def_var
 * but before the nc_enddef. */
int
NCD_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
   return nc_def_var_extra(ncid, varid, NULL, NULL, NULL, &fletcher32, 
                           NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
   
/* Define chunking stuff for a var. This must be done after nc_def_var
   and before nc_enddef. 

   Chunking is required in any dataset with one or more unlimited
   dimension in HDF5, or any dataset using a filter.

   Where chunksize is a pointer to an array of size ndims, with the
   chunksize in each dimension. 
*/
int
NCD_def_var_chunking(int ncid, int varid, int contiguous, const size_t *chunksizesp)
{
   return nc_def_var_extra(ncid, varid, NULL, NULL, NULL, NULL, 
                           &contiguous, chunksizesp, NULL, NULL, NULL, NULL, NULL);
}

/* Define fill value behavior for a variable. This must be done after
   nc_def_var and before nc_enddef. */
int
NCD_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value)
{
   return nc_def_var_extra(ncid, varid, NULL, NULL, NULL, NULL, NULL, 
                           NULL, &no_fill, fill_value, NULL, NULL, NULL);
}


/* Define the endianness of a variable. */
int
NCD_def_var_endian(int ncid, int varid, int endianness)
{
   return nc_def_var_extra(ncid, varid, NULL, NULL, NULL, NULL, NULL,
                           NULL, NULL, NULL, &endianness, NULL, NULL);
}

/* Get var id from name. */
int
NCD_inq_varid(int ncid, const char *name, int *varidp)
{
   NC_FILE_INFO_T *nc;
   NC_GRP_INFO_T *grp; 
   NC_HDF5_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   char norm_name[NC_MAX_NAME + 1];
   int retval;
   
   if (!name)
      return NC_EINVAL;
   if (!varidp)
      return NC_NOERR;

   LOG((2, "nc_inq_varid: ncid 0x%x name %s", ncid, name));
   
   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   
   /* Handle netcdf-3. */
   assert(h5);
   
   /* Normalize name. */
   if ((retval = nc4_normalize_name(name, norm_name)))
      return retval;

   /* Find var of this name. */
   for (var = grp->var; var; var = var->next)
      if (!(strcmp(var->name, norm_name)))
      {
         *varidp = var->varid;
         return NC_NOERR;
      }

   return NC_ENOTVAR;
}

/* Rename a var to "bubba," for example.
   
   According to the netcdf-3.5 docs: If the new name is longer than
   the old name, the netCDF dataset must be in define mode.  */
int
NCD_rename_var(int ncid, int varid, const char *name)
{
   NC_FILE_INFO_T *nc;
   NC_GRP_INFO_T *grp; 
   NC_HDF5_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   int retval = NC_NOERR;

   LOG((2, "nc_rename_var: ncid 0x%x varid %d name %s", 
        ncid, varid, name));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   
   /* Take care of netcdf-3 files. */
   assert(h5);

   /* Is the new name too long? */
   if (strlen(name) > NC_MAX_NAME)
      return NC_EMAXNAME;

   /* Trying to write to a read-only file? No way, Jose! */
   if (h5->no_write)
      return NC_EPERM;

   /* Check name validity, if strict nc3 rules are in effect for this
    * file. */
   if ((retval = NC_check_name(name)))
      return retval;

   /* Is name in use? */
   for (var = grp->var; var; var = var->next)
      if (!strncmp(var->name, name, NC_MAX_NAME))
         return NC_ENAMEINUSE;   

   /* Find the var. */
   for (var = grp->var; var; var = var->next)
      if (var->varid == varid)
         break;
   if (!var)
      return NC_ENOTVAR;

   /* If we're not in define mode, new name must be of equal or
      less size, if strict nc3 rules are in effect for this . */
   if (!(h5->flags & NC_INDEF) && strlen(name) > strlen(var->name) &&
       (h5->cmode & NC_CLASSIC_MODEL))
      return NC_ENOTINDEFINE;

   /* Change the HDF5 file, if this var has already been created
      there. */
   if (var->created)
   {
      if (H5Gmove(grp->hdf_grpid, var->name, name) < 0)
         BAIL(NC_EHDFERR);
   }

   /* Now change the name in our metadata. */
   free(var->name);
   if (!(var->name = malloc((strlen(name) + 1) * sizeof(char))))
      return NC_ENOMEM;
   strcpy(var->name, name);

  exit:
   return retval;
}

int
NCD_var_par_access(int ncid, int varid, int par_access) 
{
   return NC_ENOPAR;
}

int
NCD_put_vara(int ncid, int varid, const size_t *startp, 
            const size_t *countp, const void *op, int memtype)
{
   NC_FILE_INFO_T *nc;
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   void *copy_point = NULL;
   size_t num_values = 1, num_values_to_start = 0;
   int d, retval;

   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;
   assert(nc->nc4_info);

   /* Find our metadata for this file, group, and var. */
   assert(nc);
   if ((retval = nc4_find_g_var_nc(nc, ncid, varid, &grp, &var)))
      return retval;
   assert(grp && nc->nc4_info && var && var->name);
   
   /* Where do I start? */
   if (var->ndims)
   {
      num_values_to_start = 1;
      for (d = 0; d < var->ndims; d++)
	 num_values_to_start *= (startp[d] * var->dim[d]->len);
   }
   copy_point = (void *)((char *)var->diskless_data + 
			 num_values_to_start * var->type_info->size);

   /* How many values do I copy? */
   if (var->ndims)
      for (d = 0; d < var->ndims; d++)
	 num_values *= countp[d];

   /* Copy the data to memory. */
   memcpy(copy_point, op, 
	  num_values * var->type_info->size);
   
   return NC_NOERR;
}

/** \internal Read an array of values from a diskless file. */
int
NCD_get_vara(int ncid, int varid, const size_t *startp, 
            const size_t *countp, void *ip, int memtype)
{
   NC_FILE_INFO_T *nc;
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   void *copy_point = NULL;
   size_t num_values = 1, num_values_to_start = 0;
   int d, retval;

   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Find our metadata for this file, group, and var. */
   assert(nc);
   if ((retval = nc4_find_g_var_nc(nc, ncid, varid, &grp, &var)))
      return retval;
   assert(grp && nc->nc4_info && var && var->name);
   
   /* Where do I start reading this data? */
   if (var->ndims)
   {
      num_values_to_start = 1;
      for (d = 0; d < var->ndims; d++)
	 num_values_to_start *= (startp[d] * var->dim[d]->len);
   }
   copy_point = (void *)((char *)var->diskless_data + 
			 num_values_to_start * var->type_info->size);

   /* How many values do I copy? */
   if (var->ndims)
      for (d = 0; d < var->ndims; d++)
	 num_values *= countp[d];

   /* Copy the data to memory. */
   memcpy(ip, copy_point, 
	  num_values * var->type_info->size);
   
   return NC_NOERR;
}
