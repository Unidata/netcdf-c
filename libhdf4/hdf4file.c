/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file @internal The HDF4 file functions.
 *
 * @author Ed Hartnett
 */

#include "config.h"
#include <errno.h>  /* netcdf functions sometimes return system errors */
#include "nc.h"
#include "nc4internal.h"
#include "hdf4dispatch.h"
#include <mfhdf.h>

extern int nc4_vararray_add(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);

/** @internal These flags may not be set for open mode. */
static const int ILLEGAL_OPEN_FLAGS = (NC_MMAP|NC_64BIT_OFFSET|NC_MPIIO|NC_MPIPOSIX|NC_DISKLESS);

/**
 * @internal This function will free all allocated metadata memory,
 * and close the HDF4 file.
 *
 * @param h5 Pointer to HDF5 file info struct.
 * @param abort True if this is an abort.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
static int
close_hdf4_file(NC_HDF5_FILE_INFO_T *h5, int abort)
{
   int retval;

   assert(h5 && h5->controller->path && h5->root_grp && h5->no_write);
   LOG((3, "%s: h5->controller->path %s abort %d", __func__,
        h5->controller->path, abort));

   /* According to the docs, always end define mode on close. */
   if (h5->flags & NC_INDEF)
      h5->flags ^= NC_INDEF;

   /* Delete all the list contents for vars, dims, and atts, in each
    * group. */
   if ((retval = nc4_rec_grp_del(&h5->root_grp, h5->root_grp)))
      return retval;

   /* Close hdf file. */
   if (SDend(h5->sdid))
      return NC_EHDFERR;

   /* Free the nc4_info struct; above code should have reclaimed
      everything else */
   free(h5);
   
   return NC_NOERR;
}

#define NUM_TYPES 12 /**< Number of netCDF atomic types. */

/** @internal NetCDF atomic type names. */
static const char nc_type_name_g[NUM_TYPES][NC_MAX_NAME + 1] = {"char", "byte", "short",
                                                                "int", "float", "double", "ubyte",
                                                                "ushort", "uint", "int64",
                                                                "uint64", "string"};

/** @internal NetCDF atomic type sizes. */
static const int nc_type_size_g[NUM_TYPES] = {sizeof(char), sizeof(char), sizeof(short),
                                              sizeof(int), sizeof(float), sizeof(double), sizeof(unsigned char),
                                              sizeof(unsigned short), sizeof(unsigned int), sizeof(long long),
                                              sizeof(unsigned long long), sizeof(char *)};

/**
 * @internal Create a HDF4 file. Since the HDF4 integration provides
 * read-only access to HDF4 files, this returns error NC_ENOTNC4.
 *
 * @param path Ignored.
 * @param cmode Ignored.
 * @param initialsz Ignored.
 * @param basepe Ignored.
 * @param chunksizehintp Ignored.
 * @param use_parallel Ignored.
 * @param parameters Ignored.
 * @param dispatch Ignored.
 * @param nc_file Ignored.
 *
 * @return ::NC_ENOTNC4 Cannot create HDF4 files.
 * @author Ed Hartnett
 */
int
HDF4_create(const char* path, int cmode, size_t initialsz, int basepe,
           size_t *chunksizehintp, int use_parallel, void *parameters,
           NC_Dispatch *dispatch, NC* nc_file)
{
   return NC_ENOTNC4;
}

/**
 * @internal Given an HDF4 type, set a pointer to netcdf type.
 *
 * See http://www.hdfgroup.org/training/HDFtraining/UsersGuide/Fundmtls.fm3.html
 * for more information re: HDF4 types.
 *
 * @param h5 Pointer to HDF5 file info struct.
 * @param hdf4_typeid Type ID for hdf4 datatype.
 * @param xtype Pointer to netcdf type, where result will be stored.
 * @param type_info Pointer to type info for the variable.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static int
get_netcdf_type_from_hdf4(NC_HDF5_FILE_INFO_T *h5, int32 hdf4_typeid,
                          nc_type *xtype, NC_TYPE_INFO_T *type_info)
{
   int t = 0;

   /* Added this variable in the course of fixing NCF-332.
    * Prior to the fix, all data types were assigned
    * NC_ENDIAN_BIG, so I am preserving that here for now.
    * Not sure why it wouldn't be NC_ENDIAN_NATIVE, although
    * I can hazard a guess or two.
    */
   int endianness = NC_ENDIAN_BIG;
   assert(h5 && xtype);

   switch(hdf4_typeid)
   {
   case DFNT_CHAR:
      *xtype = NC_CHAR;
      t = 0;
      break;
   case DFNT_UCHAR:
   case DFNT_UINT8:
      *xtype = NC_UBYTE;
      t = 6;
      break;
   case DFNT_LUINT8:
      *xtype = NC_UBYTE;
      t = 6;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_INT8:
      *xtype = NC_BYTE;
      t = 1;
      break;
   case DFNT_LINT8:
      *xtype = NC_BYTE;
      t = 1;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_INT16:
      *xtype = NC_SHORT;
      t = 2;
      break;
   case DFNT_LINT16:
      *xtype = NC_SHORT;
      t = 2;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_UINT16:
      *xtype = NC_USHORT;
      t = 7;
      break;
   case DFNT_LUINT16:
      *xtype = NC_USHORT;
      t = 7;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_INT32:
      *xtype = NC_INT;
      t = 3;
      break;
   case DFNT_LINT32:
      *xtype = NC_INT;
      t = 3;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_UINT32:
      *xtype = NC_UINT;
      t = 8;
      break;
   case DFNT_LUINT32:
      *xtype = NC_UINT;
      t = 8;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_FLOAT32:
      *xtype = NC_FLOAT;
      t = 4;
      break;
   case DFNT_LFLOAT32:
      *xtype = NC_FLOAT;
      t = 4;
      endianness = NC_ENDIAN_LITTLE;
      break;
   case DFNT_FLOAT64:
      *xtype = NC_DOUBLE;
      t = 5;
      break;
   case DFNT_LFLOAT64:
      *xtype = NC_DOUBLE;
      t = 5;
      endianness = NC_ENDIAN_LITTLE;
      break;
   default:
      *xtype = NC_NAT;
      return NC_EBADTYPID;
   }

   if (type_info)
   {
      if (hdf4_typeid == DFNT_FLOAT32)
         type_info->nc_type_class = NC_FLOAT;
      else if (hdf4_typeid == DFNT_FLOAT64)
         type_info->nc_type_class = NC_DOUBLE;
      else if (hdf4_typeid == DFNT_CHAR)
         type_info->nc_type_class = NC_STRING;
      else
         type_info->nc_type_class = NC_INT;
      type_info->endianness = endianness;
      type_info->nc_typeid = *xtype;
      type_info->size = nc_type_size_g[t];
      if (!(type_info->name = strdup(nc_type_name_g[t])))
         return NC_ENOMEM;
   }

   return NC_NOERR;
}

/**
 * @internal Open a HDF4 file.
 *
 * @param path The file name of the new file.
 * @param mode The open mode flag.
 * @param nc Pointer that gets the NC file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
static int
nc4_open_hdf4_file(const char *path, int mode, NC *nc)
{
   NC_HDF5_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp;
   NC_ATT_INFO_T *att;
   int32 num_datasets, num_gatts;
   int32 rank;
   int v, d, a;
   int retval;
   NC_HDF5_FILE_INFO_T* nc4_info = NULL;

   LOG((3, "%s: path %s mode %d", __func__, path, mode));
   assert(path && nc);

   /* Must be read-only access to hdf4 files. */
   if (mode & NC_WRITE)
      return NC_EINVAL;

   /* Add necessary structs to hold netcdf-4 file data. */
   if ((retval = nc4_nc4f_list_add(nc, path, mode)))
      return retval;
   nc4_info = NC4_DATA(nc);
   assert(nc4_info && nc4_info->root_grp);
   h5 = nc4_info;
   h5->hdf4 = NC_TRUE;
   grp = h5->root_grp;
   h5->no_write = NC_TRUE;

   /* Open the file and initialize SD interface. */
   if ((h5->sdid = SDstart(path, DFACC_READ)) == FAIL)
      return NC_EHDFERR;

   /* Learn how many datasets and global atts we have. */
   if (SDfileinfo(h5->sdid, &num_datasets, &num_gatts))
      return NC_EHDFERR;

   /* Read the atts. */
   for (a = 0; a < num_gatts; a++)
   {
      int32 att_data_type, att_count;
      size_t att_type_size;

      /* Add to the end of the list of atts for this var. */
      if ((retval = nc4_att_list_add(&h5->root_grp->att, &att)))
         return retval;
      att->attnum = grp->natts++;
      att->created = NC_TRUE;

      /* Learn about this attribute. */
      if (!(att->name = malloc(NC_MAX_HDF4_NAME * sizeof(char))))
         return NC_ENOMEM;
      if (SDattrinfo(h5->sdid, a, att->name, &att_data_type, &att_count))
         return NC_EATTMETA;
      if ((retval = get_netcdf_type_from_hdf4(h5, att_data_type,
                                              &att->nc_typeid, NULL)))
         return retval;
      att->len = att_count;

      /* Allocate memory to hold the data. */
      if ((retval = nc4_get_typelen_mem(h5, att->nc_typeid, 0, &att_type_size)))
         return retval;
      if (!(att->data = malloc(att_type_size * att->len)))
         return NC_ENOMEM;

      /* Read the data. */
      if (SDreadattr(h5->sdid, a, att->data))
         return NC_EHDFERR;
   }

   /* Read each dataset. */
   for (v = 0; v < num_datasets; v++)
   {
      NC_VAR_INFO_T *var;
      int32 data_type, num_atts;
      /* Problem: Number of dims is returned by the call that requires
         a pre-allocated array, 'dimsize'.
         From SDS_SD website:
         http://www.hdfgroup.org/training/HDFtraining/UsersGuide/SDS_SD.fm3.html
         The maximum rank is 32, or MAX_VAR_DIMS (as defined in netcdf.h).

         int32 dimsize[MAX_VAR_DIMS];
      */
      int32 *dimsize = NULL;
      size_t var_type_size;
      int a;

      /* Add a variable. */
      if ((retval = nc4_var_add(&var)))
         return retval;

      var->varid = grp->nvars++;
      var->created = NC_TRUE;
      var->written_to = NC_TRUE;

      /* Add a var to the variable array, growing it as needed. */
      if ((retval = nc4_vararray_add(grp, var)))
         return retval;

      /* Open this dataset in HDF4 file. */
      if ((var->sdsid = SDselect(h5->sdid, v)) == FAIL)
         return NC_EVARMETA;

      /* Get shape, name, type, and attribute info about this dataset. */
      if (!(var->name = malloc(NC_MAX_HDF4_NAME + 1)))
         return NC_ENOMEM;

      /* Invoke SDgetInfo with null dimsize to get rank. */
      if (SDgetinfo(var->sdsid, var->name, &rank, NULL, &data_type, &num_atts))
         return NC_EVARMETA;

      var->hash = hash_fast(var->name, strlen(var->name));

      if(!(dimsize = (int32*)malloc(sizeof(int32)*rank)))
         return NC_ENOMEM;

      if (SDgetinfo(var->sdsid, var->name, &rank, dimsize, &data_type, &num_atts)) {
         if(dimsize) free(dimsize);
         return NC_EVARMETA;
      }

      var->ndims = rank;
      var->hdf4_data_type = data_type;

      /* Fill special type_info struct for variable type information. */
      if (!(var->type_info = calloc(1, sizeof(NC_TYPE_INFO_T)))) {
         if(dimsize) free(dimsize);
         return NC_ENOMEM;
      }

      if ((retval = get_netcdf_type_from_hdf4(h5, data_type, &var->type_info->nc_typeid, var->type_info))) {
         if(dimsize) free(dimsize);
         return retval;
      }

      /* Indicate that the variable has a pointer to the type */
      var->type_info->rc++;

      if ((retval = nc4_get_typelen_mem(h5, var->type_info->nc_typeid, 0, &var_type_size))) {
         if(dimsize) free(dimsize);
         return retval;
      }

      var->type_info->size = var_type_size;
      LOG((3, "reading HDF4 dataset %s, rank %d netCDF type %d", var->name,
           rank, var->type_info->nc_typeid));

      /* Get the fill value. */
      if (!(var->fill_value = malloc(var_type_size))) {
         if(dimsize) free(dimsize);
         return NC_ENOMEM;
      }

      if (SDgetfillvalue(var->sdsid, var->fill_value))
      {
         /* Whoops! No fill value! */
         free(var->fill_value);
         var->fill_value = NULL;
      }

      /* Allocate storage for dimension info in this variable. */
      if (var->ndims)
      {
         if (!(var->dim = malloc(sizeof(NC_DIM_INFO_T *) * var->ndims))) {
            if(dimsize) free(dimsize);
            return NC_ENOMEM;
         }

         if (!(var->dimids = malloc(sizeof(int) * var->ndims))) {
            if(dimsize) free(dimsize);
            return NC_ENOMEM;
         }
      }


      /* Find its dimensions. */
      for (d = 0; d < var->ndims; d++)
      {
         int32 dimid, dim_len, dim_data_type, dim_num_attrs;
         char dim_name[NC_MAX_NAME + 1];
         NC_DIM_INFO_T *dim;

         if ((dimid = SDgetdimid(var->sdsid, d)) == FAIL) {
            if(dimsize) free(dimsize);
            return NC_EDIMMETA;
         }
         if (SDdiminfo(dimid, dim_name, &dim_len, &dim_data_type,
                       &dim_num_attrs))
         {
            if(dimsize) free(dimsize);
            return NC_EDIMMETA;
         }

         /* Do we already have this dimension? HDF4 explicitly uses
          * the name to tell. */
         for (dim = grp->dim; dim; dim = dim->l.next)
            if (!strcmp(dim->name, dim_name))
               break;

         /* If we didn't find this dimension, add one. */
         if (!dim)
         {
            LOG((4, "adding dimension %s for HDF4 dataset %s",
                 dim_name, var->name));
            if ((retval = nc4_dim_list_add(&grp->dim, &dim)))
               return retval;
            dim->dimid = grp->nc4_info->next_dimid++;
            if (strlen(dim_name) > NC_MAX_HDF4_NAME)
               return NC_EMAXNAME;
            if (!(dim->name = strdup(dim_name)))
               return NC_ENOMEM;
            if (dim_len)
               dim->len = dim_len;
            else
               dim->len = *dimsize;
            dim->hash = hash_fast(dim_name, strlen(dim_name));
         }

         /* Tell the variable the id of this dimension. */
         var->dimids[d] = dim->dimid;
         var->dim[d] = dim;
      }

      /* Read the atts. */
      for (a = 0; a < num_atts; a++)
      {
         int32 att_data_type, att_count;
         size_t att_type_size;

         /* Add to the end of the list of atts for this var. */
         if ((retval = nc4_att_list_add(&var->att, &att))) {
            if(dimsize) free(dimsize);
            return retval;
         }
         att->attnum = var->natts++;
         att->created = NC_TRUE;

         /* Learn about this attribute. */
         if (!(att->name = malloc(NC_MAX_HDF4_NAME * sizeof(char)))) {
            if(dimsize) free(dimsize);
            return NC_ENOMEM;
         }
         if (SDattrinfo(var->sdsid, a, att->name, &att_data_type, &att_count)) {
            if(dimsize) free(dimsize);
            return NC_EATTMETA;
         }
         if ((retval = get_netcdf_type_from_hdf4(h5, att_data_type,
                                                 &att->nc_typeid, NULL))) {
            if(dimsize) free(dimsize);
            return retval;
         }

         att->len = att_count;

         /* Allocate memory to hold the data. */
         if ((retval = nc4_get_typelen_mem(h5, att->nc_typeid, 0, &att_type_size))) {
            if(dimsize) free(dimsize);
            return retval;
         }
         if (!(att->data = malloc(att_type_size * att->len))) {
            if(dimsize) free(dimsize);
            return NC_ENOMEM;
         }

         /* Read the data. */
         if (SDreadattr(var->sdsid, a, att->data)) {
            if(dimsize) free(dimsize);
            return NC_EHDFERR;
         }
      }
      if(dimsize) free(dimsize);

      {
         /* HDF4 files can be chunked */
         HDF_CHUNK_DEF chunkdefs;
         int flag;
         if(!SDgetchunkinfo(var->sdsid, &chunkdefs, &flag)) {
            if(flag == HDF_NONE)
               var->contiguous = NC_TRUE;
            else if((flag & HDF_CHUNK) != 0) {
               var->contiguous = NC_FALSE;
               if (!(var->chunksizes = malloc(var->ndims * sizeof(size_t))))
                  return NC_ENOMEM;
               for (d = 0; d < var->ndims; d++) {
                  var->chunksizes[d] = chunkdefs.chunk_lengths[d];
               }
            }
         }
      }

   } /* next var */

#ifdef LOGGING
   /* This will print out the names, types, lens, etc of the vars and
      atts in the file, if the logging level is 2 or greater. */
   log_metadata_nc(h5->root_grp->nc4_info->controller);
#endif
   return NC_NOERR;
}

/**
 * @internal Open a HDF4 file.
 *
 * @param path The file name of the file.
 * @param mode The open mode flag.
 * @param basepe Ignored by this function.
 * @param chunksizehintp Ignored by this function.
 * @param use_parallel Must be 0 for sequential, access. Parallel
 * access not supported for HDF4.
 * @param parameters pointer to struct holding extra data (e.g. for
 * parallel I/O) layer. Ignored if NULL.
 * @param dispatch Pointer to the dispatch table for this file.
 * @param nc_file Pointer to an instance of NC.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Invalid input.
 * @author Ed Hartnett
 */
int
HDF4_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
          int use_parallel, void *parameters, NC_Dispatch *dispatch,
          NC *nc_file)
{
   assert(nc_file && path);

   LOG((1, "%s: path %s mode %d params %x", __func__, path, mode,
        parameters));

   /* Check inputs. */
   assert(path && !use_parallel);

   /* Check the mode for validity */
   if (mode & ILLEGAL_OPEN_FLAGS)
      return NC_EINVAL;

   /* Open the file. */
   nc_file->int_ncid = nc_file->ext_ncid;
   return nc4_open_hdf4_file(path, mode, nc_file);
}

/**
 * @internal Unfortunately HDF only allows specification of fill value
 * only when a dataset is created. Whereas in netcdf, you first create
 * the variable and then (optionally) specify the fill value. To
 * accomplish this in HDF5 I have to delete the dataset, and recreate
 * it, with the fill value specified. 
 *
 * @param ncid File and group ID.
 * @param fillmode File mode.
 * @param old_modep Pointer that gets old mode. Ignored if NULL.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
int
HDF4_set_fill(int ncid, int fillmode, int *old_modep)
{
   return NC_EPERM;
}

/**
 * @internal Not allowed for HDF4 files.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_ENOTNC4 No error.
 * @author Ed Hartnett
*/
int
HDF4_redef(int ncid)
{
   return NC_EPERM;
}

/**
 * @internal This does nothing for HDF4 files. Since they are
 * read-only, then can never be put into define mode.
 *
 * @param ncid File and group ID.
 * @param h_minfree Ignored for netCDF-4 files.
 * @param v_align Ignored for netCDF-4 files.
 * @param v_minfree Ignored for netCDF-4 files.
 * @param r_align Ignored for netCDF-4 files.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
int
HDF4__enddef(int ncid, size_t h_minfree, size_t v_align,
             size_t v_minfree, size_t r_align)
{
   return NC_NOERR;   
}

/**
 * @internal Does nothing, since HDF4 files are read only. There can
 * be no changes to sync to disk.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
*/
int
HDF4_sync(int ncid)
{
   return NC_NOERR;
}

/**
 * @internal Closes the file. There can be no changes to abort since
 * HDF4 files are read only.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
HDF4_abort(int ncid)
{
   NC *nc;
   NC_HDF5_FILE_INFO_T* nc4_info;
   int retval;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   /* Find metadata for this file. */
   if (!(nc = nc4_find_nc_file(ncid, &nc4_info)))
      return NC_EBADID;
   assert(nc4_info);

   /* Free any resources the netcdf-4 library has for this file's
    * metadata. */
   if ((retval = close_hdf4_file(nc4_info, 1)))
      return retval;

   return NC_NOERR;
}

/**
 * @internal Close the HDF4 file. 
 *
 * @param ncid File ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett
*/
int
HDF4_close(int ncid)
{
   NC_GRP_INFO_T *grp;
   NC *nc;
   NC_HDF5_FILE_INFO_T *h5;
   int retval;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find our metadata for this file. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(nc && h5 && grp && !grp->parent);

   /* Call the nc4 close. */
   if ((retval = close_hdf4_file(grp->nc4_info, 0)))
      return retval;

   return NC_NOERR;
}


