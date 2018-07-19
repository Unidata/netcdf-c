/* Copyright 2003-2006, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions.*/
/**
 * @file
 * @internal This file is part of netcdf-4, a netCDF-like interface
 * for HDF5, or a HDF5 backend for netCDF, depending on your point of
 * view. This file handles the NetCDF-4 variable functions.
 *
 * @author Ed Hartnett, Dennis Heimbigner, Ward Fisher
 */

#include <nc4internal.h>
#include "nc4dispatch.h"
#include <math.h>

/* Szip options. */
#define NC_SZIP_EC_OPTION_MASK 4  /**< @internal SZIP EC option mask. */
#define NC_SZIP_NN_OPTION_MASK 32 /**< @internal SZIP NN option mask. */
#define NC_SZIP_MAX_PIXELS_PER_BLOCK 32 /**< @internal SZIP max pixels per block. */

/** @internal Default size for unlimited dim chunksize */
#define DEFAULT_1D_UNLIM_SIZE (4096)

#define NC_ARRAY_GROWBY 4 /**< @internal Amount to grow array. */

extern int nc4_get_default_fill_value(const NC_TYPE_INFO_T *type_info,
                                      void *fill_value);

/**
 * @internal Set chunk cache size for a variable. This is the internal
 * function called by nc_set_var_chunk_cache().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param size Size in bytes to set cache.
 * @param nelems Number of elements in cache.
 * @param preemption Controls cache swapping.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
 * @returns ::NC_EINVAL Invalid input.
 * @returns ::NC_EHDFERR HDF5 error.
 * @author Ed Hartnett
 */
int
NC4_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems,
                        float preemption)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   int retval;

   /* Check input for validity. */
   if (preemption < 0 || preemption > 1)
      return NC_EINVAL;

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc && grp && h5);

   /* Find the var. */
   var = (NC_VAR_INFO_T*)ncindexith(grp->vars,varid);
   if(!var)
	return NC_ENOTVAR;
   assert(var && var->hdr.id == varid);

   /* Set the values. */
   var->chunk_cache_size = size;
   var->chunk_cache_nelems = nelems;
   var->chunk_cache_preemption = preemption;

   if ((retval = nc4_reopen_dataset(grp, var)))
      return retval;

   return NC_NOERR;
}

/**
 * @internal A wrapper for NC4_set_var_chunk_cache(), we need this
 * version for fortran. Negative values leave settings as they are.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param size Size in bytes to set cache.
 * @param nelems Number of elements in cache.
 * @param preemption Controls cache swapping.
 *
 * @returns ::NC_NOERR for success
 * @author Ed Hartnett
 */
int
nc_set_var_chunk_cache_ints(int ncid, int varid, int size, int nelems,
                            int preemption)
{
   size_t real_size = H5D_CHUNK_CACHE_NBYTES_DEFAULT;
   size_t real_nelems = H5D_CHUNK_CACHE_NSLOTS_DEFAULT;
   float real_preemption = CHUNK_CACHE_PREEMPTION;

   if (size >= 0)
      real_size = ((size_t) size) * MEGABYTE;

   if (nelems >= 0)
      real_nelems = nelems;

   if (preemption >= 0)
      real_preemption = preemption / 100.;

   return NC4_set_var_chunk_cache(ncid, varid, real_size, real_nelems,
                                  real_preemption);
}

/**
 * @internal This is called by nc_get_var_chunk_cache(). Get chunk
 * cache size for a variable.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param sizep Gets size in bytes of cache.
 * @param nelemsp Gets number of element slots in cache.
 * @param preemptionp Gets cache swapping setting.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Not a netCDF-4 file.
 * @author Ed Hartnett
 */
int
NC4_get_var_chunk_cache(int ncid, int varid, size_t *sizep,
                        size_t *nelemsp, float *preemptionp)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   int retval;

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc && grp && h5);

   /* Find the var. */
   var = (NC_VAR_INFO_T*)ncindexith(grp->vars,varid);
   if(!var)
	return NC_ENOTVAR;
   assert(var && var->hdr.id == varid);

   /* Give the user what they want. */
   if (sizep)
      *sizep = var->chunk_cache_size;
   if (nelemsp)
      *nelemsp = var->chunk_cache_nelems;
   if (preemptionp)
      *preemptionp = var->chunk_cache_preemption;

   return NC_NOERR;
}

/**
 * @internal A wrapper for NC4_get_var_chunk_cache(), we need this
 * version for fortran.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param sizep Gets size in bytes of cache.
 * @param nelemsp Gets number of element slots in cache.
 * @param preemptionp Gets cache swapping setting.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Not a netCDF-4 file.
 * @author Ed Hartnett
 */
int
nc_get_var_chunk_cache_ints(int ncid, int varid, int *sizep,
                            int *nelemsp, int *preemptionp)
{
   size_t real_size, real_nelems;
   float real_preemption;
   int ret;

   if ((ret = NC4_get_var_chunk_cache(ncid, varid, &real_size,
                                      &real_nelems, &real_preemption)))
      return ret;

   if (sizep)
      *sizep = real_size / MEGABYTE;
   if (nelemsp)
      *nelemsp = (int)real_nelems;
   if(preemptionp)
      *preemptionp = (int)(real_preemption * 100);

   return NC_NOERR;
}

/**
 * @internal Check a set of chunksizes to see if they specify a chunk
 * that is too big.
 *
 * @param grp Pointer to the group info.
 * @param var Pointer to the var info.
 * @param chunksizes Array of chunksizes to check.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_EBADCHUNK Bad chunksize.
 */
static int
check_chunksizes(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var, const size_t *chunksizes)
{
   double dprod;
   size_t type_len;
   int d;
   int retval;

   if ((retval = nc4_get_typelen_mem(grp->nc4_info, var->type_info->hdr.id, &type_len)))
      return retval;
   if (var->type_info->nc_type_class == NC_VLEN)
      dprod = (double)sizeof(hvl_t);
   else
      dprod = (double)type_len;
   for (d = 0; d < var->ndims; d++)
      dprod *= (double)chunksizes[d];

   if (dprod > (double) NC_MAX_UINT)
      return NC_EBADCHUNK;

   return NC_NOERR;
}

/**
 * @internal Determine some default chunksizes for a variable.
 *
 * @param grp Pointer to the group info.
 * @param var Pointer to the var info.
 *
 * @returns ::NC_NOERR for success
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @author Ed Hartnett
 */
static int
nc4_find_default_chunksizes2(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var)
{
   int d;
   size_t type_size;
   float num_values = 1, num_unlim = 0;
   int retval;
   size_t suggested_size;
#ifdef LOGGING
   double total_chunk_size;
#endif

   if (var->type_info->nc_type_class == NC_STRING)
      type_size = sizeof(char *);
   else
      type_size = var->type_info->size;

#ifdef LOGGING
   /* Later this will become the total number of bytes in the default
    * chunk. */
   total_chunk_size = (double) type_size;
#endif

   /* How many values in the variable (or one record, if there are
    * unlimited dimensions). */
   for (d = 0; d < var->ndims; d++)
   {
      assert(var->dim[d]);
      if (! var->dim[d]->unlimited)
         num_values *= (float)var->dim[d]->len;
      else {
         num_unlim++;
         var->chunksizes[d] = 1; /* overwritten below, if all dims are unlimited */
      }
   }
   /* Special case to avoid 1D vars with unlim dim taking huge amount
      of space (DEFAULT_CHUNK_SIZE bytes). Instead we limit to about
      4KB */
   if (var->ndims == 1 && num_unlim == 1) {
      if (DEFAULT_CHUNK_SIZE / type_size <= 0)
         suggested_size = 1;
      else if (DEFAULT_CHUNK_SIZE / type_size > DEFAULT_1D_UNLIM_SIZE)
         suggested_size = DEFAULT_1D_UNLIM_SIZE;
      else
         suggested_size = DEFAULT_CHUNK_SIZE / type_size;
      var->chunksizes[0] = suggested_size / type_size;
      LOG((4, "%s: name %s dim %d DEFAULT_CHUNK_SIZE %d num_values %f type_size %d "
           "chunksize %ld", __func__, var->hdr.name, d, DEFAULT_CHUNK_SIZE, num_values, type_size, var->chunksizes[0]));
   }
   if (var->ndims > 1 && var->ndims == num_unlim) { /* all dims unlimited */
      suggested_size = pow((double)DEFAULT_CHUNK_SIZE/type_size, 1.0/(double)(var->ndims));
      for (d = 0; d < var->ndims; d++)
      {
         var->chunksizes[d] = suggested_size ? suggested_size : 1;
         LOG((4, "%s: name %s dim %d DEFAULT_CHUNK_SIZE %d num_values %f type_size %d "
              "chunksize %ld", __func__, var->hdr.name, d, DEFAULT_CHUNK_SIZE, num_values, type_size, var->chunksizes[d]));
      }
   }

   /* Pick a chunk length for each dimension, if one has not already
    * been picked above. */
   for (d = 0; d < var->ndims; d++)
      if (!var->chunksizes[d])
      {
         suggested_size = (pow((double)DEFAULT_CHUNK_SIZE/(num_values * type_size),
                               1.0/(double)(var->ndims - num_unlim)) * var->dim[d]->len - .5);
         if (suggested_size > var->dim[d]->len)
            suggested_size = var->dim[d]->len;
         var->chunksizes[d] = suggested_size ? suggested_size : 1;
         LOG((4, "%s: name %s dim %d DEFAULT_CHUNK_SIZE %d num_values %f type_size %d "
              "chunksize %ld", __func__, var->hdr.name, d, DEFAULT_CHUNK_SIZE, num_values, type_size, var->chunksizes[d]));
      }

#ifdef LOGGING
   /* Find total chunk size. */
   for (d = 0; d < var->ndims; d++)
      total_chunk_size *= (double) var->chunksizes[d];
   LOG((4, "total_chunk_size %f", total_chunk_size));
#endif

   /* But did this result in a chunk that is too big? */
   retval = check_chunksizes(grp, var, var->chunksizes);
   if (retval)
   {
      /* Other error? */
      if (retval != NC_EBADCHUNK)
         return retval;

      /* Chunk is too big! Reduce each dimension by half and try again. */
      for ( ; retval == NC_EBADCHUNK; retval = check_chunksizes(grp, var, var->chunksizes))
         for (d = 0; d < var->ndims; d++)
            var->chunksizes[d] = var->chunksizes[d]/2 ? var->chunksizes[d]/2 : 1;
   }

   /* Do we have any big data overhangs? They can be dangerous to
    * babies, the elderly, or confused campers who have had too much
    * beer. */
   for (d = 0; d < var->ndims; d++)
   {
      size_t num_chunks;
      size_t overhang;
      assert(var->chunksizes[d] > 0);
      num_chunks = (var->dim[d]->len + var->chunksizes[d] - 1) / var->chunksizes[d];
      if(num_chunks > 0) {
         overhang = (num_chunks * var->chunksizes[d]) - var->dim[d]->len;
         var->chunksizes[d] -= overhang / num_chunks;
      }
   }

   return NC_NOERR;
}

/**
 * @internal Get all the information about a variable. Pass NULL for
 * whatever you don't care about. This is the internal function called
 * by nc_inq_var(), nc_inq_var_deflate(), nc_inq_var_fletcher32(),
 * nc_inq_var_chunking(), nc_inq_var_chunking_ints(),
 * nc_inq_var_fill(), nc_inq_var_endian(), nc_inq_var_filter(), and
 * nc_inq_var_szip().
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param name Gets name.
 * @param xtypep Gets type.
 * @param ndimsp Gets number of dims.
 * @param dimidsp Gets array of dim IDs.
 * @param nattsp Gets number of attributes.
 * @param shufflep Gets shuffle setting.
 * @param deflatep Gets deflate setting.
 * @param deflate_levelp Gets deflate level.
 * @param fletcher32p Gets fletcher32 setting.
 * @param contiguousp Gets contiguous setting.
 * @param chunksizesp Gets chunksizes.
 * @param no_fill Gets fill mode.
 * @param fill_valuep Gets fill value.
 * @param endiannessp Gets one of ::NC_ENDIAN_BIG ::NC_ENDIAN_LITTLE
 * ::NC_ENDIAN_NATIVE
 * @param idp Pointer to memory to store filter id.
 * @param nparamsp Pointer to memory to store filter parameter count.
 * @param params Pointer to vector of unsigned integers into which
 * to store filter parameters.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Bad varid.
 * @returns ::NC_ENOMEM Out of memory.
 * @returns ::NC_EINVAL Invalid input.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC4_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep,
                int *ndimsp, int *dimidsp, int *nattsp,
                int *shufflep, int *deflatep, int *deflate_levelp,
                int *fletcher32p, int *contiguousp, size_t *chunksizesp,
                int *no_fill, void *fill_valuep, int *endiannessp,
                unsigned int* idp, size_t* nparamsp, unsigned int* params
   )
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   int d;
   int retval;

   LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   assert(nc);
   assert(grp && h5);

   /* Walk through the list of vars, and return the info about the one
      with a matching varid. If the varid is -1, find the global
      atts and call it a day. */
   if (varid == NC_GLOBAL)
   {
      if (nattsp)
      {
         /* Do we need to read the atts? */
         if (grp->atts_not_read)
            if ((retval = nc4_read_grp_atts(grp)))
               return retval;

	 *nattsp = ncindexcount(grp->att);
      }
      return NC_NOERR;
   }

   /* Find the var. */
   var = (NC_VAR_INFO_T*)ncindexith(grp->vars,varid);
   if(!var)
      return NC_ENOTVAR;
   assert(var && var->hdr.id == varid);

   /* Copy the data to the user's data buffers. */
   if (name)
      strcpy(name, var->hdr.name);
   if (xtypep)
      *xtypep = var->type_info->hdr.id;
   if (ndimsp)
      *ndimsp = var->ndims;
   if (dimidsp)
      for (d = 0; d < var->ndims; d++)
         dimidsp[d] = var->dimids[d];
   if (nattsp)
   {
      if (var->atts_not_read)
         if ((retval = nc4_read_var_atts(grp, var)))
            return retval;
      *nattsp = ncindexcount(var->att);
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
      *deflatep = (int)var->deflate;
   if (deflate_levelp)
      *deflate_levelp = var->deflate_level;
   if (shufflep)
      *shufflep = (int)var->shuffle;
   if (fletcher32p)
      *fletcher32p = (int)var->fletcher32;

   if (idp)
      *idp = var->filterid;
   if (nparamsp)
      *nparamsp = (var->params == NULL ? 0 : var->nparams);
   if (params && var->params != NULL)
      memcpy(params,var->params,var->nparams*sizeof(unsigned int));

   /* Fill value stuff. */
   if (no_fill)
      *no_fill = (int)var->no_fill;

   /* Don't do a thing with fill_valuep if no_fill mode is set for
    * this var, or if fill_valuep is NULL. */
   if (!var->no_fill && fill_valuep)
   {
      /* Do we have a fill value for this var? */
      if (var->fill_value)
      {
         if (var->type_info->nc_type_class == NC_STRING)
         {
            assert(*(char **)var->fill_value);
            /* This will allocate memeory and copy the string. */
            if (!(*(char **)fill_valuep = strdup(*(char **)var->fill_value)))
            {
               free(*(char **)fill_valuep);
               return NC_ENOMEM;
            }
         }
         else
         {
            assert(var->type_info->size);
            memcpy(fill_valuep, var->fill_value, var->type_info->size);
         }
      }
      else
      {
         if (var->type_info->nc_type_class == NC_STRING)
         {
            if (!(*(char **)fill_valuep = calloc(1, sizeof(char *))))
               return NC_ENOMEM;

            if ((retval = nc4_get_default_fill_value(var->type_info, (char **)fill_valuep)))
            {
               free(*(char **)fill_valuep);
               return retval;
            }
         }
         else
         {
            if ((retval = nc4_get_default_fill_value(var->type_info, fill_valuep)))
               return retval;
         }
      }
   }

   /* Does the user want the endianness of this variable? */
   if (endiannessp)
      *endiannessp = var->type_info->endianness;

   return NC_NOERR;
}

/**
 * @internal Inquire about chunking settings for a var. This is used
 * by the fortran API.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param contiguousp Gets contiguous setting.
 * @param chunksizesp Gets chunksizes.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_EINVAL Invalid input
 * @returns ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
nc_inq_var_chunking_ints(int ncid, int varid, int *contiguousp, int *chunksizesp)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   NC_FILE_INFO_T *h5;

   size_t *cs = NULL;
   int i, retval;

   /* Find this ncid's file info. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc);

   /* Find var cause I need the number of dims. */
   if ((retval = nc4_find_g_var_nc(nc, ncid, varid, &grp, &var)))
      return retval;

   /* Allocate space for the size_t copy of the chunksizes array. */
   if (var->ndims)
      if (!(cs = malloc(var->ndims * sizeof(size_t))))
         return NC_ENOMEM;

   retval = NC4_inq_var_all(ncid, varid, NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, NULL, contiguousp, cs, NULL,
                            NULL, NULL, NULL, NULL, NULL);

   /* Copy from size_t array. */
   if (chunksizesp && var->contiguous == NC_CHUNKED)
      for (i = 0; i < var->ndims; i++)
      {
         chunksizesp[i] = (int)cs[i];
         if (cs[i] > NC_MAX_INT)
            retval = NC_ERANGE;
      }

   if (var->ndims)
      free(cs);
   return retval;
}

/**
 * @internal Define chunking stuff for a var. This is called by
 * the fortran API.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param contiguous Pointer to contiguous setting.
 * @param chunksizesp Array of chunksizes.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Invalid variable ID.
 * @returns ::NC_ENOTNC4 Attempting netcdf-4 operation on file that is
 * not netCDF-4/HDF5.
 * @returns ::NC_ELATEDEF Too late to change settings for this variable.
 * @returns ::NC_ENOTINDEFINE Not in define mode.
 * @returns ::NC_EINVAL Invalid input
 * @returns ::NC_EBADCHUNK Bad chunksize.
 * @author Ed Hartnett
 */
int
nc_def_var_chunking_ints(int ncid, int varid, int contiguous, int *chunksizesp)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   NC_FILE_INFO_T *h5;
   size_t *cs = NULL;
   int i, retval;

   /* Find this ncid's file info. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc);

   /* Find var cause I need the number of dims. */
   if ((retval = nc4_find_g_var_nc(nc, ncid, varid, &grp, &var)))
      return retval;

   /* Allocate space for the size_t copy of the chunksizes array. */
   if (var->ndims)
      if (!(cs = malloc(var->ndims * sizeof(size_t))))
         return NC_ENOMEM;

   /* Copy to size_t array. */
   for (i = 0; i < var->ndims; i++)
      cs[i] = chunksizesp[i];

   retval = nc_def_var_extra(ncid, varid, NULL, NULL, NULL, NULL,
                             &contiguous, cs, NULL, NULL, NULL);

   if (var->ndims)
      free(cs);
   return retval;
}

/**
 * @internal Find the ID of a variable, from the name. This function
 * is called by nc_inq_varid().
 *
 * @param ncid File ID.
 * @param name Name of the variable.
 * @param varidp Gets variable ID.

 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Bad ncid.
 * @returns ::NC_ENOTVAR Bad variable ID.
 */
int
NC4_inq_varid(int ncid, const char *name, int *varidp)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   char norm_name[NC_MAX_NAME + 1];
   int retval;

   if (!name)
      return NC_EINVAL;
   if (!varidp)
      return NC_NOERR;

   LOG((2, "%s: ncid 0x%x name %s", __func__, ncid, name));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, NULL)))
      return retval;

   /* Normalize name. */
   if ((retval = nc4_normalize_name(name, norm_name)))
      return retval;

   /* Find var of this name. */
   var = (NC_VAR_INFO_T*)ncindexlookup(grp->vars,norm_name);
   if(var)
      {
         *varidp = var->hdr.id;
         return NC_NOERR;
      }
   return NC_ENOTVAR;
}

/**
 * @internal
 *
 * This function will change the parallel access of a variable from
 * independent to collective.
 *
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param par_access NC_COLLECTIVE or NC_INDEPENDENT.
 *
 * @returns ::NC_NOERR No error.
 * @returns ::NC_EBADID Invalid ncid passed.
 * @returns ::NC_ENOTVAR Invalid varid passed.
 * @returns ::NC_ENOPAR LFile was not opened with nc_open_par/nc_create_var.
 * @returns ::NC_EINVAL Invalid par_access specified.
 * @returns ::NC_NOERR for success
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC4_var_par_access(int ncid, int varid, int par_access)
{
#ifndef USE_PARALLEL4
   return NC_ENOPAR;
#else
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var;
   int retval;

   LOG((1, "%s: ncid 0x%x varid %d par_access %d", __func__, ncid,
        varid, par_access));

   if (par_access != NC_INDEPENDENT && par_access != NC_COLLECTIVE)
      return NC_EINVAL;

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   /* This function only for files opened with nc_open_par or nc_create_par. */
   if (!h5->parallel)
      return NC_ENOPAR;

   /* Find the var, and set its preference. */
   var = (NC_VAR_INFO_T*)ncindexith(grp->vars,varid);
   if (!var) return NC_ENOTVAR;
   assert(var->hdr.id == varid);

   if (par_access)
      var->parallel_access = NC_COLLECTIVE;
   else
      var->parallel_access = NC_INDEPENDENT;
   return NC_NOERR;
#endif /* USE_PARALLEL4 */
}

