/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */
/**
 * @file @internal Internal netcdf-4 functions.
 *
 * This file contains functions internal to the netcdf4 library. None of
 * the functions in this file are exposed in the exetnal API. These
 * functions all relate to the manipulation of netcdf-4's in-memory
 * buffer of metadata information, i.e. the linked list of NC
 * structs.
 *
 * @author Ed Hartnett
 */

#include "config.h"
#include "hdf5internal.h"

#undef DEBUGH5

#ifdef DEBUGH5
/**
 * @internal Provide a catchable error reporting function
 *
 * @param ignored Ignored.
 *
 * @return 0 for success.
 */
static herr_t
h5catch(void* ignored)
{
   H5Eprint(NULL);
   return 0;
}
#endif

/* These are the default chunk cache sizes for HDF5 files created or
 * opened with netCDF-4. */
extern size_t nc4_chunk_cache_size;
extern size_t nc4_chunk_cache_nelems;
extern float nc4_chunk_cache_preemption;

#ifdef LOGGING
/* This is the severity level of messages which will be logged. Use
   severity 0 for errors, 1 for important log messages, 2 for less
   important, etc. */
extern int nc_log_level;

#endif /* LOGGING */

int nc4_hdf5_initialized = 0; /**< True if initialization has happened. */

/**
 * @internal Provide a wrapper for H5Eset_auto
 * @param func Pointer to func.
 * @param client_data Client data.
 *
 * @return 0 for success
 */
static herr_t
set_auto(void* func, void *client_data)
{
#ifdef DEBUGH5
   return H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)h5catch,client_data);
#else
   return H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)func,client_data);
#endif
}

/**
 * @internal Provide a function to do any necessary initialization of
 * the HDF5 library.
 */
void
nc4_hdf5_initialize(void)
{
   if (set_auto(NULL, NULL) < 0)
      LOG((0, "Couldn't turn off HDF5 error messages!"));
   LOG((1, "HDF5 error messages have been turned off."));
   nc4_hdf5_initialized = 1;
}

/**
 * @internal Given a varid, return the maximum length of a dimension
 * using dimid.
 *
 * @param grp Pointer to group info struct.
 * @param varid Variable ID.
 * @param dimid Dimension ID.
 * @param maxlen Pointer that gets the max length.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static int
find_var_dim_max_length(NC_GRP_INFO_T *grp, int varid, int dimid, size_t *maxlen)
{
   hid_t datasetid = 0, spaceid = 0;
   NC_VAR_INFO_T *var;
   hsize_t *h5dimlen = NULL, *h5dimlenmax = NULL;
   int d, dataset_ndims = 0;
   int retval = NC_NOERR;

   *maxlen = 0;

   /* Find this var. */
   var = (NC_VAR_INFO_T*)ncindexith(grp->vars,varid);
   if (!var) return NC_ENOTVAR;
   assert(var->hdr.id == varid);

   /* If the var hasn't been created yet, its size is 0. */
   if (!var->created)
   {
      *maxlen = 0;
   }
   else
   {
      /* Get the number of records in the dataset. */
      if ((retval = nc4_open_var_grp2(grp, var->hdr.id, &datasetid)))
         BAIL(retval);
      if ((spaceid = H5Dget_space(datasetid)) < 0)
         BAIL(NC_EHDFERR);

      /* If it's a scalar dataset, it has length one. */
      if (H5Sget_simple_extent_type(spaceid) == H5S_SCALAR)
      {
         *maxlen = (var->dimids && var->dimids[0] == dimid) ? 1 : 0;
      }
      else
      {
         /* Check to make sure ndims is right, then get the len of each
            dim in the space. */
         if ((dataset_ndims = H5Sget_simple_extent_ndims(spaceid)) < 0)
            BAIL(NC_EHDFERR);
         if (dataset_ndims != var->ndims)
            BAIL(NC_EHDFERR);
         if (!(h5dimlen = malloc(dataset_ndims * sizeof(hsize_t))))
            BAIL(NC_ENOMEM);
         if (!(h5dimlenmax = malloc(dataset_ndims * sizeof(hsize_t))))
            BAIL(NC_ENOMEM);
         if ((dataset_ndims = H5Sget_simple_extent_dims(spaceid,
                                                        h5dimlen, h5dimlenmax)) < 0)
            BAIL(NC_EHDFERR);
         LOG((5, "find_var_dim_max_length: varid %d len %d max: %d",
              varid, (int)h5dimlen[0], (int)h5dimlenmax[0]));
         for (d=0; d<dataset_ndims; d++) {
            if (var->dimids[d] == dimid) {
               *maxlen = *maxlen > h5dimlen[d] ? *maxlen : h5dimlen[d];
            }
         }
      }
   }

exit:
   if (spaceid > 0 && H5Sclose(spaceid) < 0)
      BAIL2(NC_EHDFERR);
   if (h5dimlen) free(h5dimlen);
   if (h5dimlenmax) free(h5dimlenmax);
   return retval;
}

/**
 * @internal Search for type with a given HDF type id.
 *
 * @param h5 File
 * @param target_hdf_typeid HDF5 type ID to find.
 *
 * @return Pointer to type info struct, or NULL if not found.
 * @author Ed Hartnett
 */
NC_TYPE_INFO_T *
nc4_rec_find_hdf_type(NC_FILE_INFO_T *h5, hid_t target_hdf_typeid)
{
   NC_TYPE_INFO_T *type;
   htri_t equal;
   int i;

   assert(h5);

   for(i=0;i<nclistlength(h5->alltypes);i++) {
      type = (NC_TYPE_INFO_T*)nclistget(h5->alltypes,i);
      if(type == NULL) continue;
      /* Is this the type we are searching for? */
      if ((equal = H5Tequal(type->native_hdf_typeid ? type->native_hdf_typeid :
                            type->hdf_typeid, target_hdf_typeid)) < 0)
         return NULL;
      if (equal)
         return type;
   }
   /* Can't find it. Fate, why do you mock me? */
   return NULL;
}

/**
 * @internal Find the actual length of a dim by checking the length of
 * that dim in all variables that use it, in grp or children. **len
 * must be initialized to zero before this function is called.
 *
 * @param grp Pointer to group info struct.
 * @param dimid Dimension ID.
 * @param len Pointer to pointer that gets length.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_find_dim_len(NC_GRP_INFO_T *grp, int dimid, size_t **len)
{
   NC_GRP_INFO_T *g;
   NC_VAR_INFO_T *var;
   int retval;
   int i;

   assert(grp && len);
   LOG((3, "nc4_find_dim_len: grp->name %s dimid %d", grp->hdr.name, dimid));

   /* If there are any groups, call this function recursively on
    * them. */
   for(i=0;i<ncindexsize(grp->children);i++) {
      g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
      if(g == NULL) continue;
      if ((retval = nc4_find_dim_len(g, dimid, len)))
         return retval;
   }

   /* For all variables in this group, find the ones that use this
    * dimension, and remember the max length. */
   for (i=0; i < ncindexsize(grp->vars); i++)
   {
      size_t mylen;
      var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
      if (var == NULL) continue;

      /* Find max length of dim in this variable... */
      if ((retval = find_var_dim_max_length(grp, var->hdr.id, dimid, &mylen)))
         return retval;

      **len = **len > mylen ? **len : mylen;
   }

   return NC_NOERR;
}

/**
 * @internal Break a coordinate variable to separate the dimension and
 * the variable.
 *
 * This is called from nc_rename_dim() and nc_rename_var(). In some
 * renames, the coord variable must stay, but it is no longer a coord
 * variable. This function changes a coord var into an ordinary
 * variable.
 *
 * @param grp Pointer to group info struct.
 * @param coord_var Pointer to variable info struct.
 * @param dim Pointer to dimension info struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Quincey Koziol, Ed Hartnett
 */
int
nc4_break_coord_var(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *coord_var, NC_DIM_INFO_T *dim)
{
   int retval = NC_NOERR;

   /* Sanity checks */
   assert(grp && coord_var && dim && dim->coord_var == coord_var &&
          coord_var->dim[0] == dim && coord_var->dimids[0] == dim->hdr.id &&
          !dim->hdf_dimscaleid);
   LOG((3, "%s dim %s was associated with var %s, but now has different name",
        __func__, dim->hdr.name, coord_var->hdr.name));

   /* If we're replacing an existing dimscale dataset, go to
    * every var in the file and detach this dimension scale. */
   if ((retval = rec_detach_scales(grp->nc4_info->root_grp,
                                   dim->hdr.id, coord_var->hdf_datasetid)))
      return retval;

   /* Allow attached dimscales to be tracked on the [former]
    * coordinate variable */
   if (coord_var->ndims)
   {
      /* Coordinate variables shouldn't have dimscales attached. */
      assert(!coord_var->dimscale_attached);

      /* Allocate space for tracking them */
      if (!(coord_var->dimscale_attached = calloc(coord_var->ndims,
                                                  sizeof(nc_bool_t))))
         return NC_ENOMEM;
   }

   /* Remove the atts that go with being a coordinate var. */
   /* if ((retval = remove_coord_atts(coord_var->hdf_datasetid))) */
   /*    return retval; */

   /* Detach dimension from variable */
   coord_var->dimscale = NC_FALSE;
   dim->coord_var = NULL;

   /* Set state transition indicators */
   coord_var->was_coord_var = NC_TRUE;
   coord_var->became_coord_var = NC_FALSE;

   return NC_NOERR;
}

/**
 * @internal Delete an existing dimscale-only dataset.
 *
 * A dimscale-only HDF5 dataset is created when a dim is defined
 * without an accompanying coordinate variable.
 *
 * Sometimes, during renames, or late creation of variables, an
 * existing, dimscale-only dataset must be removed. This means
 * detaching all variables that use the dataset, then closing and
 * unlinking it.
 *
 * @param grp The grp of the dimscale-only dataset to be deleted, or a
 * higher group in the hierarchy (ex. root group).
 * @param dimid id of the dimension
 * @param dim Pointer to the dim with the dimscale-only dataset to be
 * deleted.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF5 error.
 * @author Ed Hartnett
 */
int
delete_existing_dimscale_dataset(NC_GRP_INFO_T *grp, int dimid, NC_DIM_INFO_T *dim)
{
   int retval;

   assert(grp && dim);
   LOG((2, "%s: deleting dimscale dataset %s dimid %d", __func__, dim->hdr.name,
        dimid));

   /* Detach dimscale from any variables using it */
   if ((retval = rec_detach_scales(grp, dimid, dim->hdf_dimscaleid)) < 0)
      return retval;

   /* Close the HDF5 dataset */
   if (H5Dclose(dim->hdf_dimscaleid) < 0)
      return NC_EHDFERR;
   dim->hdf_dimscaleid = 0;

   /* Now delete the dataset. */
   if (H5Gunlink(grp->hdf_grpid, dim->hdr.name) < 0)
      return NC_EHDFERR;

   return NC_NOERR;
}

/**
 * @internal Reform a coordinate variable from a dimension and a
 * variable.
 *
 * @param grp Pointer to group info struct.
 * @param var Pointer to variable info struct.
 * @param dim Pointer to dimension info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Quincey Koziol
 */
int
nc4_reform_coord_var(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var, NC_DIM_INFO_T *dim)
{
   int need_to_reattach_scales = 0;
   int retval = NC_NOERR;

   assert(grp && var && dim);
   LOG((3, "%s: dim->hdr.name %s var->hdr.name %s", __func__, dim->hdr.name, var->hdr.name));

   /* Detach dimscales from the [new] coordinate variable */
   if(var->dimscale_attached)
   {
      int dims_detached = 0;
      int finished = 0;
      int d;

      /* Loop over all dimensions for variable */
      for (d = 0; d < var->ndims && !finished; d++)
      {
         /* Is there a dimscale attached to this axis? */
         if(var->dimscale_attached[d])
         {
            NC_GRP_INFO_T *g;
            int k;

            for (g = grp; g && !finished; g = g->parent)
            {
               NC_DIM_INFO_T *dim1;
               for(k=0;k<ncindexsize(g->dim);k++)
               {
                  if((dim1 = (NC_DIM_INFO_T*)ncindexith(g->dim,k)) == NULL) continue;
                  if (var->dimids[d] == dim1->hdr.id)
                  {
                     hid_t dim_datasetid;  /* Dataset ID for dimension */

                     /* Find dataset ID for dimension */
                     if (dim1->coord_var)
                        dim_datasetid = dim1->coord_var->hdf_datasetid;
                     else
                        dim_datasetid = dim1->hdf_dimscaleid;

                     /* dim_datasetid may be 0 in some cases when
                      * renames of dims and vars are happening. In
                      * this case, the scale has already been
                      * detached. */
                     if (dim_datasetid > 0)
                     {
                        LOG((3, "detaching scale from %s", var->hdr.name));
                        if (H5DSdetach_scale(var->hdf_datasetid, dim_datasetid, d) < 0)
                           BAIL(NC_EHDFERR);
                     }
                     var->dimscale_attached[d] = NC_FALSE;
                     if (dims_detached++ == var->ndims)
                        finished++;
                  }
               }
            }
         }
      } /* next variable dimension */

      /* Release & reset the array tracking attached dimscales */
      free(var->dimscale_attached);
      var->dimscale_attached = NULL;
      need_to_reattach_scales++;
   }

   /* Use variable's dataset ID for the dimscale ID. */
   if (dim->hdf_dimscaleid && grp != NULL)
   {
      LOG((3, "closing and unlinking dimscale dataset %s", dim->hdr.name));
      if (H5Dclose(dim->hdf_dimscaleid) < 0)
         BAIL(NC_EHDFERR);
      dim->hdf_dimscaleid = 0;

      /* Now delete the dimscale's dataset
         (it will be recreated later, if necessary) */
      if (H5Gunlink(grp->hdf_grpid, dim->hdr.name) < 0)
         return NC_EDIMMETA;
   }

   /* Attach variable to dimension */
   var->dimscale = NC_TRUE;
   dim->coord_var = var;

   /* Check if this variable used to be a coord. var */
   if (need_to_reattach_scales || (var->was_coord_var && grp != NULL))
   {
      /* Reattach the scale everywhere it is used. */
      /* (Recall that netCDF dimscales are always 1-D) */
      if ((retval = rec_reattach_scales(grp->nc4_info->root_grp,
                                        var->dimids[0], var->hdf_datasetid)))
         return retval;

      /* Set state transition indicator (cancels earlier transition) */
      var->was_coord_var = NC_FALSE;
   }
   else
      /* Set state transition indicator */
      var->became_coord_var = NC_TRUE;

exit:
   return retval;
}

#ifdef LOGGING
/**
 * This is the same as nc_set_log_level(), but will also turn on HDF5
 * internal logging, in addition to netCDF logging.
 *
 * @param new_level The new logging level.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
hdf5_set_log_level(int new_level)
{
   if (!nc4_hdf5_initialized)
      nc4_hdf5_initialize();

   /* If the user wants to completely turn off logging, turn off HDF5
      logging too. Now I truely can't think of what to do if this
      fails, so just ignore the return code. */
   if (new_level == NC_TURN_OFF_LOGGING)
   {
      set_auto(NULL, NULL);
      LOG((1, "HDF5 error messages turned off!"));
   }

   /* Do we need to turn HDF5 logging back on? */
   if (new_level > NC_TURN_OFF_LOGGING &&
       nc_log_level <= NC_TURN_OFF_LOGGING)
   {
      if (set_auto((H5E_auto_t)&H5Eprint, stderr) < 0)
         LOG((0, "H5Eset_auto failed!"));
      LOG((1, "HDF5 error messages turned on."));
   }

   /* Now remember the new level. */
   nc_set_log_level(new_level);

   return NC_NOERR;
}
#endif /* LOGGING */
