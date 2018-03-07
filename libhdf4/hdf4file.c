/* Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for copying
 * and redistribution conditions.*/
/**
 * @file @internal The HDF4 file functions.
 *
 * @author Ed Hartnett
 */

#include "config.h"
#include "nc4internal.h"
#include "hdf4dispatch.h"
#include <mfhdf.h>

extern int nc4_vararray_add(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);

/** @internal These flags may not be set for open mode. */
static const int ILLEGAL_OPEN_FLAGS = (NC_MMAP|NC_64BIT_OFFSET|NC_MPIIO|NC_MPIPOSIX|NC_DISKLESS);

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
 * @return ::NC_EBADTYPEID Type not found.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
static int
get_netcdf_type_from_hdf4(NC_HDF5_FILE_INFO_T *h5, int32 hdf4_typeid,
                          nc_type *xtype, NC_TYPE_INFO_T *type_info)
{
   int t = 0;

   /* Added this variable in the course of fixing NCF-332. Prior to
    * the fix, all data types were assigned NC_ENDIAN_BIG, so I am
    * preserving that. */
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
 * @internal Read an attribute from a HDF4 file.
 *
 * @param h5 Pointer to the file metadata struct.
 * @param var Pointer to variable metadata struct or NULL for global
 * attributes.
 * @param a Index of attribute to read.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF4 error.
 * @return ::NC_EATTMETA Error reading HDF4 attribute.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
static int
hdf4_read_att(NC_HDF5_FILE_INFO_T *h5, NC_VAR_INFO_T *var, int a)
{
   NC_HDF4_FILE_INFO_T *hdf4_file;
   NC_ATT_INFO_T *att;   
   NC_ATT_INFO_T **att_list;   
   int32 att_data_type, att_count;
   size_t att_type_size;
   int hdf4_sd_id;
   int retval;
   
   LOG((3, "%s: a %d var %s", __func__, a, var ? var->name : "global"));

   /* Check inputs. */
   assert(h5 && h5->format_file_info);

   /* Get the HDF4 file info. */
   hdf4_file = h5->format_file_info;

   /* Decide what att list to use, global or from a var. */
   if (var) {
      NC_VAR_HDF4_INFO_T *hdf4_var;
      assert(var->format_var_info);
      att_list = &var->att;
      hdf4_var = var->format_var_info;
      hdf4_sd_id = hdf4_var->sdsid;
   } else {
      att_list = &h5->root_grp->att;
      hdf4_sd_id = hdf4_file->sdid;
   }
   
   /* Add to the end of the list of atts. */
   if ((retval = nc4_att_list_add(att_list, &att)))
      return retval;
   att->attnum = !var ? h5->root_grp->natts++ : var->natts++;
   att->created = NC_TRUE;
   
   /* Learn about this attribute. */
   if (!(att->name = malloc((NC_MAX_HDF4_NAME + 1) * sizeof(char))))
      return NC_ENOMEM;
   if (SDattrinfo(hdf4_sd_id, a, att->name, &att_data_type, &att_count))
      return NC_EATTMETA;
   if ((retval = get_netcdf_type_from_hdf4(h5, att_data_type, &att->nc_typeid,
                                           NULL)))
      return retval;
   att->len = att_count;
   LOG((4, "att->name %s att->nc_typeid %d att->len %d", att->name,
        att->nc_typeid, att->len));
   
   /* Allocate memory to hold the data. */
   if ((retval = nc4_get_typelen_mem(h5, att->nc_typeid, 0, &att_type_size)))
      return retval;
   if (!(att->data = malloc(att_type_size * att->len)))
      return NC_ENOMEM;
   
   /* Read the data. */
   if (SDreadattr(hdf4_sd_id, a, att->data))
      return NC_EHDFERR;
   
   return NC_NOERR;
}

/**
 * @internal Read a HDF4 dimension. As new dimensions are found, add
 * them to the metadata list of dimensions.
 *
 * @param h5 Pointer to the file metadata struct.
 * @param var Pointer to variable metadata struct or NULL for global
 * attributes.
 * 
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF4 error.
 * @return ::NC_EDIMMETA Error reading HDF4 dimension info.
 * @return ::NC_ENOMEM Out of memory.
 * @return ::NC_EMAXNAME Name too long.
 * @author Ed Hartnett
 */
static int
hdf4_read_dim(NC_HDF5_FILE_INFO_T *h5, NC_VAR_INFO_T *var, int rec_dim_size, int d)
{
   NC_VAR_HDF4_INFO_T *hdf4_var;
   NC_DIM_INFO_T *dim;
   int32 dimid, dim_len, dim_data_type, dim_num_attrs;
   char dim_name[NC_MAX_NAME + 1];
   int retval;
   
   assert(h5 && h5->format_file_info && var && var->format_var_info);
   hdf4_var = var->format_var_info;         
   if ((dimid = SDgetdimid(hdf4_var->sdsid, d)) == FAIL) 
      return NC_EDIMMETA;

   if (SDdiminfo(dimid, dim_name, &dim_len, &dim_data_type,
                 &dim_num_attrs))
      return NC_EDIMMETA;

   /* Do we already have this dimension? HDF4 explicitly uses
    * the name to tell. */
   for (dim = h5->root_grp->dim; dim; dim = dim->l.next)
      if (!strcmp(dim->name, dim_name))
         break;

   /* If we didn't find this dimension, add one. */
   if (!dim)
   {
      LOG((4, "adding dimension %s for HDF4 dataset %s",
           dim_name, var->name));
      if ((retval = nc4_dim_list_add(&h5->root_grp->dim, &dim)))
         return retval;
      dim->dimid = h5->root_grp->nc4_info->next_dimid++;
      if (strlen(dim_name) > NC_MAX_HDF4_NAME)
         return NC_EMAXNAME;
      if (!(dim->name = strdup(dim_name)))
         return NC_ENOMEM;
      if (dim_len)
         dim->len = dim_len;
      else
         dim->len = rec_dim_size;
      dim->hash = hash_fast(dim_name, strlen(dim_name));
   }

   /* Tell the variable the id of this dimension. */
   var->dimids[d] = dim->dimid;
   var->dim[d] = dim;

   return NC_NOERR;
}

/**
 * @internal Read a HDF4 variable, including its associated dimensions
 * and attributes.
 *
 * @param h5 Pointer to the file metadata struct.
 * @param v Index of variable to read.
 * 
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF4 error.
 * @return ::NC_EDIMMETA Error reading HDF4 dimension info.
 * @return ::NC_EVARMETA Error reading HDF4 dataset or att.
 * @return ::NC_EATTMETA Error reading HDF4 attribute.
 * @return ::NC_ENOMEM Out of memory.
 * @return ::NC_EMAXNAME Name too long.
 * @author Ed Hartnett
 */
static int
hdf4_read_var(NC_HDF5_FILE_INFO_T *h5, int v)
{
   NC_HDF4_FILE_INFO_T *hdf4_file;
   NC_VAR_INFO_T *var;
   NC_VAR_HDF4_INFO_T *hdf4_var;
   HDF_CHUNK_DEF chunkdefs;
   int flag;
   int32 data_type, num_atts;
   int32 *dimsize;
   int32 rec_dim_size;
   int32 rank;
   int d, a;
   int retval;

   assert(h5 && h5->format_file_info);
   hdf4_file = h5->format_file_info;

   /* Create and init a variable metadata struct. */
   if ((retval = nc4_var_add(&var)))
      return retval;
   var->varid = h5->root_grp->nvars++;
   var->created = NC_TRUE;
   var->written_to = NC_TRUE;

   /* Add the var to the variable array, growing it as needed. */
   if ((retval = nc4_vararray_add(h5->root_grp, var)))
      return retval;

   /* Malloc a struct to hold HDF4-specific variable
    * information. */
   if (!(hdf4_var = malloc(sizeof(NC_VAR_HDF4_INFO_T))))
      return NC_ENOMEM;

   /* Point the format_info pointer at the HDF4 var info. */
   var->format_var_info = hdf4_var;      

   /* Open this dataset in HDF4 file. */
   if ((hdf4_var->sdsid = SDselect(hdf4_file->sdid, v)) == FAIL)
      return NC_EVARMETA;

   /* Get shape, name, type, and attribute info about this dataset. */
   if (!(var->name = malloc(NC_MAX_HDF4_NAME + 1)))
      return NC_ENOMEM;

   /* Invoke SDgetInfo with null dimsize to get rank. */
   if (SDgetinfo(hdf4_var->sdsid, var->name, &rank, NULL, &data_type,
                 &num_atts))
      return NC_EVARMETA;

   /* Create hash for names for quick lookups. */
   var->hash = hash_fast(var->name, strlen(var->name));

   /* Allocate space for dim sizes. */
   if (!(dimsize = malloc(sizeof(int32) * rank)))
      return NC_ENOMEM;

   /* Get the length of the dimensions. */
   if (SDgetinfo(hdf4_var->sdsid, var->name, &rank, dimsize,
                 &data_type, &num_atts)) {
      free(dimsize);
      return NC_EVARMETA;
   }
   rec_dim_size = rank ? dimsize[0] : 0;
   free(dimsize);      
   var->ndims = rank;
   hdf4_var->hdf4_data_type = data_type;

   /* Fill special type_info struct for variable type information. */
   if (!(var->type_info = calloc(1, sizeof(NC_TYPE_INFO_T)))) 
      return NC_ENOMEM;

   if ((retval = get_netcdf_type_from_hdf4(h5, data_type,
                                           &var->type_info->nc_typeid,
                                           var->type_info))) 
      return retval;
   LOG((3, "var->name %s var->ndims %d var->type_info->nc_typeid %d "
        "num_atts %d", var->name, var->ndims, var->type_info->nc_typeid,
        num_atts));

   /* Indicate that the variable has a pointer to the type */
   var->type_info->rc++;

   /* Get the size of the type. */
   if ((retval = nc4_get_typelen_mem(h5, var->type_info->nc_typeid, 0,
                                     &var->type_info->size))) 
      return retval;

   LOG((3, "reading HDF4 dataset %s, rank %d netCDF type %d", var->name,
        rank, var->type_info->nc_typeid));

   /* Get the fill value. */
   if (!(var->fill_value = malloc(var->type_info->size))) 
      return NC_ENOMEM;

   if (SDgetfillvalue(hdf4_var->sdsid, var->fill_value))
   {
      /* Whoops! No fill value! */
      free(var->fill_value);
      var->fill_value = NULL;
   }

   /* Allocate storage for dimension info in this variable. */
   if (var->ndims)
   {
      if (!(var->dim = malloc(sizeof(NC_DIM_INFO_T *) * var->ndims))) 
         return NC_ENOMEM;
      if (!(var->dimids = malloc(sizeof(int) * var->ndims))) 
         return NC_ENOMEM;
   }

   /* Determine whether chunking is in use. */
   if (!SDgetchunkinfo(hdf4_var->sdsid, &chunkdefs, &flag))
   {
      if (flag == HDF_NONE)
         var->contiguous = NC_TRUE;
      else if (flag & HDF_CHUNK)
      {
         /* Chunking is in use. Find the chunk sizes. */
         var->contiguous = NC_FALSE;
         if (!(var->chunksizes = malloc(var->ndims * sizeof(size_t))))
            return NC_ENOMEM;
         for (d = 0; d < var->ndims; d++) 
            var->chunksizes[d] = chunkdefs.chunk_lengths[d];
      }
   }

   /* Read the dimensions for this var. */
   for (d = 0; d < var->ndims; d++)
      if ((retval = hdf4_read_dim(h5, var, rec_dim_size, d)))
         return retval;

   /* Read the variable atts. */
   for (a = 0; a < num_atts; a++)
      if ((retval = hdf4_read_att(h5, var, a)))
         return retval;
      
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
 * parallel I/O) layer. Not supported for HDF4.
 * @param dispatch Pointer to the dispatch table for this file.
 * @param nc Pointer to an instance of NC.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF4 error.
 * @return ::NC_EINVAL Access must be read-only.
 * @return ::NC_EVARMETA Error reading HDF4 dataset or att.
 * @return ::NC_EDIMMETA Error reading HDF4 dimension info.
 * @return ::NC_EATTMETA Error reading HDF4 attribute.
 * @return ::NC_EMAXNAME Name too long.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
NC_HDF4_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
             int use_parallel, void *parameters, NC_Dispatch *dispatch,
             NC *nc)
{
   NC_HDF5_FILE_INFO_T *h5;
   NC_HDF4_FILE_INFO_T *hdf4_file;   
   int32 num_datasets, num_gatts;
   int v, a;
   int retval;

   /* Check inputs. */
   assert(nc && path && !use_parallel && !parameters);

   LOG((1, "%s: path %s mode %d params %x", __func__, path, mode,
        parameters));

   /* Check the mode for validity */
   if (mode & ILLEGAL_OPEN_FLAGS)
      return NC_EINVAL;

   /* Open the file. */
   nc->int_ncid = nc->ext_ncid;

   /* Add necessary structs to hold netcdf-4 file data. */
   if ((retval = nc4_nc4f_list_add(nc, path, mode)))
      return retval;
   h5 = NC4_DATA(nc);
   assert(h5 && h5->root_grp);

   /* Allocate data to hold HDF4 specific file data. */
   if (!(hdf4_file = malloc(sizeof(NC_HDF4_FILE_INFO_T))))
      return NC_ENOMEM;
   h5->format_file_info = hdf4_file;
   h5->no_write = NC_TRUE;

   /* Open the file and initialize SD interface. */
   if ((hdf4_file->sdid = SDstart(path, DFACC_READ)) == FAIL)
      return NC_EHDFERR;

   /* Learn how many datasets and global atts we have. */
   if (SDfileinfo(hdf4_file->sdid, &num_datasets, &num_gatts))
      return NC_EHDFERR;

   /* Read the global atts. */
   for (a = 0; a < num_gatts; a++)
      if ((retval = hdf4_read_att(h5, NULL, a)))
         return retval;

   /* Read each dataset, including dimensions and attributes. */
   for (v = 0; v < num_datasets; v++)
      if ((retval = hdf4_read_var(h5, v)))
         return retval;

#ifdef LOGGING
   /* This will print out the names, types, lens, etc of the vars and
      atts in the file, if the logging level is 2 or greater. */
   log_metadata_nc(nc);
#endif
   return NC_NOERR;
}

/**
 * @internal Close the HDF4 file. 
 *
 * @param ncid File ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EHDFERR HDF4 error.
 * @author Ed Hartnett
*/
int
NC_HDF4_close(int ncid)
{
   NC_HDF5_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp;
   NC_HDF4_FILE_INFO_T *hdf4_file;   
   int varid;
   int retval;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find our metadata for this file. */
   if ((retval = nc4_find_grp_h5(ncid, &grp, &h5)))
      return retval;
   assert(h5 && grp && !grp->parent);

   /* Delete the HDF4-specific var info. */
   for (varid = 0; varid < h5->root_grp->vars.nelems; varid++)
      free(h5->root_grp->vars.value[varid]->format_var_info);

   /* Delete all the list contents for vars, dims, and atts, in each
    * group. */
   if ((retval = nc4_rec_grp_del(&h5->root_grp, h5->root_grp)))
      return retval;

   /* Close hdf4 file and free HDF4 file info. */
   hdf4_file = (NC_HDF4_FILE_INFO_T *)h5->format_file_info;
   if (SDend(hdf4_file->sdid))
      return NC_EHDFERR;
   free(hdf4_file);

   /* Free the nc4_info struct; above code should have reclaimed
      everything else */
   free(h5);

   return NC_NOERR;
}
