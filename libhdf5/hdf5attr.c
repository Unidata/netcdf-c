/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions. */
/**
 * @file
 * @internal This file handles HDF5 attributes.
 *
 * @author Ed Hartnett
 */

#include "config.h"
#include "hdf5internal.h"

/**
 * @internal Get the attribute list for either a varid or NC_GLOBAL
 *
 * @param grp Group
 * @param varid Variable ID | NC_BLOGAL
 * @param varp Pointer that gets pointer to NC_VAR_INFO_T
 * instance. Ignored if NULL.
 * @param attlist Pointer that gets pointer to attribute list.
 *
 * @return NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
static int
getattlist(NC_GRP_INFO_T *grp, int varid, NC_VAR_INFO_T **varp,
            NCindex **attlist)
{
   NC_VAR_INFO_T* var;
   int retval;

   if (varid == NC_GLOBAL)
   {
      /* Do we need to read the atts? */
      if (grp->atts_not_read)
         if ((retval = nc4_read_atts(grp, NULL)))
            return retval;

      if (varp)
         *varp = NULL;
      *attlist = grp->att;
   }
   else
   {
      if (!(var = (NC_VAR_INFO_T *)ncindexith(grp->vars, varid)))
         return NC_ENOTVAR;
      assert(var->hdr.id == varid);

      /* Do we need to read the atts? */
      if (var->atts_not_read)
         if ((retval = nc4_read_atts(grp, var)))
            return retval;

      if (varp)
         *varp = var;
      *attlist = var->att;
   }
   return NC_NOERR;
}

/**
 * @internal I think all atts should be named the exact same thing, to
 * avoid confusion!
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param newname New name for attribute.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME New name too long.
 * @return ::NC_EPERM File is read-only.
 * @return ::NC_ENAMEINUSE New name already in use.
 * @return ::NC_ENOTINDEFINE Classic model file not in define mode.
 * @return ::NC_EHDFERR HDF error.
 * @return ::NC_ENOMEM Out of memory.
 * @return ::NC_EINTERNAL Could not rebuild list.
 * @author Ed Hartnett
 */
int
NC4_rename_att(int ncid, int varid, const char *name, const char *newname)
{
   NC *nc;
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var = NULL;
   NC_ATT_INFO_T *att;
   NCindex *list;
   char norm_newname[NC_MAX_NAME + 1], norm_name[NC_MAX_NAME + 1];
   hid_t datasetid = 0;
   int retval = NC_NOERR;

   if (!name || !newname)
      return NC_EINVAL;

   LOG((2, "nc_rename_att: ncid 0x%x varid %d name %s newname %s",
        ncid, varid, name, newname));

   /* If the new name is too long, that's an error. */
   if (strlen(newname) > NC_MAX_NAME)
      return NC_EMAXNAME;

   /* Find info for this file, group, and h5 info. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;
   assert(h5 && grp && h5);

   /* If the file is read-only, return an error. */
   if (h5->no_write)
      return NC_EPERM;

   /* Check and normalize the name. */
   if ((retval = nc4_check_name(newname, norm_newname)))
      return retval;

   /* Get the list of attributes. */
   if ((retval = getattlist(grp, varid, &var, &list)))
      return retval;

   /* Is new name in use? */
   att = (NC_ATT_INFO_T*)ncindexlookup(list,norm_newname);
   if(att != NULL)
      return NC_ENAMEINUSE;

   /* Normalize name and find the attribute. */
   if ((retval = nc4_normalize_name(name, norm_name)))
      return retval;

   att = (NC_ATT_INFO_T*)ncindexlookup(list,norm_name);
   if (!att)
      return NC_ENOTATT;

   /* If we're not in define mode, new name must be of equal or
      less size, if complying with strict NC3 rules. */
   if (!(h5->flags & NC_INDEF) && strlen(norm_newname) > strlen(att->hdr.name) &&
       (h5->cmode & NC_CLASSIC_MODEL))
      return NC_ENOTINDEFINE;

   /* Delete the original attribute, if it exists in the HDF5 file. */
   if (att->created)
   {
      if (varid == NC_GLOBAL)
      {
         if (H5Adelete(((NC_HDF5_GRP_INFO_T *)(grp->format_grp_info))->hdf_grpid,
                       att->hdr.name) < 0)
            return NC_EHDFERR;
      }
      else
      {
         if ((retval = nc4_open_var_grp2(grp, varid, &datasetid)))
            return retval;
         if (H5Adelete(datasetid, att->hdr.name) < 0)
            return NC_EHDFERR;
      }
      att->created = NC_FALSE;
   }

   /* Copy the new name into our metadata. */
   if(att->hdr.name) free(att->hdr.name);
   if (!(att->hdr.name = strdup(norm_newname)))
      return NC_ENOMEM;
   att->hdr.hashkey = NC_hashmapkey(att->hdr.name,strlen(att->hdr.name)); /* Fix hash key */

   att->dirty = NC_TRUE;

   /* Rehash the attribute list so that the new name is used */
   if(!ncindexrebuild(list))
      return NC_EINTERNAL;

   /* Mark attributes on variable dirty, so they get written */
   if(var)
      var->attr_dirty = NC_TRUE;

   return retval;
}

/**
 * @internal Delete an att. Rub it out. Push the button on
 * it. Liquidate it. Bump it off. Take it for a one-way
 * ride. Terminate it.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute to delete.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTATT Attribute not found.
 * @return ::NC_EINVAL No name provided.
 * @return ::NC_EPERM File is read only.
 * @return ::NC_ENOTINDEFINE Classic model not in define mode.
 * @return ::NC_EINTERNAL Could not rebuild list.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC4_del_att(int ncid, int varid, const char *name)
{
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   NC_FILE_INFO_T *h5;
   NC_ATT_INFO_T *att;
   NCindex* attlist = NULL;
   hid_t locid = 0;
   int i;
   size_t deletedid;
   int retval;

   /* Name must be provided. */
   if (!name)
      return NC_EINVAL;

   LOG((2, "nc_del_att: ncid 0x%x varid %d name %s", ncid, varid, name));

   /* Find info for this file, group, and h5 info. */
   if ((retval = nc4_find_grp_h5(ncid, &grp, &h5)))
      return retval;
   assert(h5 && grp);

   /* If the file is read-only, return an error. */
   if (h5->no_write)
      return NC_EPERM;

   /* If file is not in define mode, return error for classic model
    * files, otherwise switch to define mode. */
   if (!(h5->flags & NC_INDEF))
   {
      if (h5->cmode & NC_CLASSIC_MODEL)
         return NC_ENOTINDEFINE;
      if ((retval = NC4_redef(ncid)))
         return retval;
   }

   /* Get either the global or a variable attribute list. */
   if ((retval = getattlist(grp, varid, &var, &attlist)))
      return retval;

   /* Determine the location id in the HDF5 file. */
   if (varid == NC_GLOBAL)
      locid = ((NC_HDF5_GRP_INFO_T *)(grp->format_grp_info))->hdf_grpid;
   else if (var->created)
      locid = var->hdf_datasetid;

   /* Now find the attribute by name. */
   if (!(att = (NC_ATT_INFO_T*)ncindexlookup(attlist, name)))
      return NC_ENOTATT;

   /* Delete it from the HDF5 file, if it's been created. */
   if (att->created)
   {
      assert(locid);
      if (H5Adelete(locid, att->hdr.name) < 0)
         return NC_EATTMETA;
   }

   deletedid = att->hdr.id;

   /* Remove this attribute in this list */
   if ((retval = nc4_att_list_del(attlist, att)))
      return retval;

   /* Renumber all attributes with higher indices. */
   for (i = 0; i < ncindexsize(attlist); i++)
   {
      NC_ATT_INFO_T *a;
      if (!(a = (NC_ATT_INFO_T *)ncindexith(attlist, i)))
         continue;
      if (a->hdr.id > deletedid)
         a->hdr.id--;
   }

   /* Rebuild the index. */
   if (!ncindexrebuild(attlist))
      return NC_EINTERNAL;

   return NC_NOERR;
}

/**
 * @internal This will return the length of a netcdf atomic data type
 * in bytes.
 *
 * @param type A netcdf atomic type.
 *
 * @return Type size in bytes, or -1 if type not found.
 * @author Ed Hartnett
 */
static int
nc4typelen(nc_type type)
{
   switch(type){
   case NC_BYTE:
   case NC_CHAR:
   case NC_UBYTE:
      return 1;
   case NC_USHORT:
   case NC_SHORT:
      return 2;
   case NC_FLOAT:
   case NC_INT:
   case NC_UINT:
      return 4;
   case NC_DOUBLE:
   case NC_INT64:
   case NC_UINT64:
      return 8;
   }
   return -1;
}

/**
 * @internal
 * Write an attribute to a netCDF-4/HDF5 file, converting
 * data type if necessary.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param file_type Type of the attribute data in file.
 * @param len Number of elements in attribute array.
 * @param data Attribute data.
 * @param mem_type Type of data in memory.
 * @param force write even if the attribute is special
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Invalid parameters.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_EBADNAME Name contains illegal characters.
 * @return ::NC_ENAMEINUSE Name already in use.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
nc4_put_att(NC_GRP_INFO_T* grp, int varid, const char *name, nc_type file_type,
            size_t len, const void *data, nc_type mem_type, int force)
{
   NC* nc;
   NC_FILE_INFO_T *h5;
   NC_VAR_INFO_T *var = NULL;
   NCindex* attlist = NULL;
   NC_ATT_INFO_T* att;
   char norm_name[NC_MAX_NAME + 1];
   nc_bool_t new_att = NC_FALSE;
   int retval = NC_NOERR, range_error = 0;
   size_t type_size;
   int i;
   int ret;
   int ncid;

   h5 = grp->nc4_info;
   nc = h5->controller;
   assert(nc && grp && h5);

   ncid = nc->ext_ncid | grp->hdr.id;

   /* Find att, if it exists. (Must check varid first or nc_test will
    * break.) */
   if ((ret = getattlist(grp, varid, &var, &attlist)))
      return ret;

   /* The length needs to be positive (cast needed for braindead
      systems with signed size_t). */
   if((unsigned long) len > X_INT_MAX)
      return NC_EINVAL;

   /* Check name before LOG statement. */
   if (!name || strlen(name) > NC_MAX_NAME)
      return NC_EBADNAME;

   LOG((1, "%s: ncid 0x%x varid %d name %s file_type %d mem_type %d len %d",
        __func__,ncid, varid, name, file_type, mem_type, len));

   /* If len is not zero, then there must be some data. */
   if (len && !data)
      return NC_EINVAL;

   /* If the file is read-only, return an error. */
   if (h5->no_write)
      return NC_EPERM;

   /* Check and normalize the name. */
   if ((retval = nc4_check_name(name, norm_name)))
      return retval;

   /* Check that a reserved att name is not being used improperly */
   const NC_reservedatt* ra = NC_findreserved(name);
   if(ra != NULL && !force) {
      /* case 1: grp=root, varid==NC_GLOBAL, flags & READONLYFLAG */
      if (nc->ext_ncid == ncid && varid == NC_GLOBAL && grp->parent == NULL
          && (ra->flags & READONLYFLAG))
         return NC_ENAMEINUSE;
      /* case 2: grp=NA, varid!=NC_GLOBAL, flags & DIMSCALEFLAG */
      if (varid != NC_GLOBAL && (ra->flags & DIMSCALEFLAG))
         return NC_ENAMEINUSE;
   }

   /* See if there is already an attribute with this name. */
   att = (NC_ATT_INFO_T*)ncindexlookup(attlist,norm_name);

   LOG((1, "%s: ncid 0x%x varid %d name %s file_type %d mem_type %d len %d",
        __func__, ncid, varid, name, file_type, mem_type, len));

   if (!att)
   {
      /* If this is a new att, require define mode. */
      if (!(h5->flags & NC_INDEF))
      {
         if (h5->cmode & NC_CLASSIC_MODEL)
            return NC_ENOTINDEFINE;
         if ((retval = NC4_redef(ncid)))
            BAIL(retval);
      }
      new_att = NC_TRUE;
   }
   else
   {
      /* For an existing att, if we're not in define mode, the len
         must not be greater than the existing len for classic model. */
      if (!(h5->flags & NC_INDEF) &&
          len * nc4typelen(file_type) > (size_t)att->len * nc4typelen(att->nc_typeid))
      {
         if (h5->cmode & NC_CLASSIC_MODEL)
            return NC_ENOTINDEFINE;
         if ((retval = NC4_redef(ncid)))
            BAIL(retval);
      }
   }

   /* We must have two valid types to continue. */
   if (file_type == NC_NAT || mem_type == NC_NAT)
      return NC_EBADTYPE;

   /* Get information about this type. */
   if ((retval = nc4_get_typelen_mem(h5, file_type, &type_size)))
      return retval;

   /* No character conversions are allowed. */
   if (file_type != mem_type &&
       (file_type == NC_CHAR || mem_type == NC_CHAR ||
        file_type == NC_STRING || mem_type == NC_STRING))
      return NC_ECHAR;

   /* For classic mode file, only allow atts with classic types to be
    * created. */
   if (h5->cmode & NC_CLASSIC_MODEL && file_type > NC_DOUBLE)
      return NC_ESTRICTNC3;

   /* Add to the end of the attribute list, if this att doesn't
      already exist. */
   if (new_att)
   {
      LOG((3, "adding attribute %s to the list...", norm_name));
      if ((ret = nc4_att_list_add(attlist, norm_name, &att)))
         BAIL(ret);

      /* Allocate storage for the HDF5 specific att info. */
      if (!(att->format_att_info = calloc(1, sizeof(NC_HDF5_ATT_INFO_T))))
         BAIL(NC_ENOMEM);
   }

   /* Now fill in the metadata. */
   att->dirty = NC_TRUE;
   att->nc_typeid = file_type;

   /* If this att has vlen or string data, release it before we lose the length value. */
   if (att->stdata)
   {
      for (i = 0; i < att->len; i++)
         if(att->stdata[i])
            free(att->stdata[i]);
      free(att->stdata);
      att->stdata = NULL;
   }
   if (att->vldata)
   {
      for (i = 0; i < att->len; i++)
         nc_free_vlen(&att->vldata[i]); /* FIX: see warning of nc_free_vlen */
      free(att->vldata);
      att->vldata = NULL;
   }

   att->len = len;

   /* If this is the _FillValue attribute, then we will also have to
    * copy the value to the fill_vlue pointer of the NC_VAR_INFO_T
    * struct for this var. (But ignore a global _FillValue
    * attribute). */
   if (!strcmp(att->hdr.name, _FillValue) && varid != NC_GLOBAL)
   {
      int size;

      /* Fill value must be same type and have exactly one value */
      if (att->nc_typeid != var->type_info->hdr.id)
         return NC_EBADTYPE;
      if (att->len != 1)
         return NC_EINVAL;

      /* If we already wrote to the dataset, then return an error. */
      if (var->written_to)
         return NC_ELATEFILL;

      /* Get the length of the veriable data type. */
      if ((retval = nc4_get_typelen_mem(grp->nc4_info, var->type_info->hdr.id,
                                        &type_size)))
         return retval;

      /* Already set a fill value? Now I'll have to free the old
       * one. Make up your damn mind, would you? */
      if (var->fill_value)
      {
         if (var->type_info->nc_type_class == NC_VLEN)
         {
            if ((retval = nc_free_vlen(var->fill_value)))
               return retval;
         }
         else if (var->type_info->nc_type_class == NC_STRING)
         {
            if (*(char **)var->fill_value)
               free(*(char **)var->fill_value);
         }
         free(var->fill_value);
      }

      /* Determine the size of the fill value in bytes. */
      if (var->type_info->nc_type_class == NC_VLEN)
         size = sizeof(hvl_t);
      else if (var->type_info->nc_type_class == NC_STRING)
         size = sizeof(char *);
      else
         size = type_size;

      /* Allocate space for the fill value. */
      if (!(var->fill_value = calloc(1, size)))
         return NC_ENOMEM;

      /* Copy the fill_value. */
      LOG((4, "Copying fill value into metadata for variable %s", var->hdr.name));
      if (var->type_info->nc_type_class == NC_VLEN)
      {
         nc_vlen_t *in_vlen = (nc_vlen_t *)data, *fv_vlen = (nc_vlen_t *)(var->fill_value);
         NC_TYPE_INFO_T* basetype;
	 size_t basetypesize = 0;

	 /* get the basetype and its size */
	 basetype = var->type_info;
         if ((retval = nc4_get_typelen_mem(grp->nc4_info, basetype->hdr.id, &basetypesize)))
             return retval;
	 /* shallow clone the content of the vlen; shallow because it has only a temporary existence */
         fv_vlen->len = in_vlen->len;
         if (!(fv_vlen->p = malloc(basetypesize * in_vlen->len)))
            return NC_ENOMEM;
         memcpy(fv_vlen->p, in_vlen->p, in_vlen->len * basetypesize);
      }
      else if (var->type_info->nc_type_class == NC_STRING)
      {
         if (*(char **)data)
         {
            if (!(*(char **)(var->fill_value) = malloc(strlen(*(char **)data) + 1)))
               return NC_ENOMEM;
            strcpy(*(char **)var->fill_value, *(char **)data);
         }
         else
            *(char **)var->fill_value = NULL;
      }
      else
         memcpy(var->fill_value, data, type_size);

      /* Indicate that the fill value was changed, if the variable has already
       * been created in the file, so the dataset gets deleted and re-created. */
      if (var->created)
         var->fill_val_changed = NC_TRUE;
   }

   /* Copy the attribute data, if there is any. VLENs and string
    * arrays have to be handled specially. */
   if (att->len)
   {
      nc_type type_class;    /* Class of attribute's type */

      /* Get class for this type. */
      if ((retval = nc4_get_typeclass(h5, file_type, &type_class)))
         return retval;

      assert(data);
      if (type_class == NC_VLEN)
      {
         const hvl_t *vldata1;
         NC_TYPE_INFO_T *vltype;
         size_t base_typelen;

         /* Get the type object for the attribute's type */
         if ((retval = nc4_find_type(h5, file_type, &vltype)))
            BAIL(retval);

         /* Retrieve the size of the base type */
         if ((retval = nc4_get_typelen_mem(h5, vltype->u.v.base_nc_typeid, &base_typelen)))
            BAIL(retval);

         vldata1 = data;
         if (!(att->vldata = (nc_vlen_t*)malloc(att->len * sizeof(hvl_t))))
            BAIL(NC_ENOMEM);
         for (i = 0; i < att->len; i++)
         {
            att->vldata[i].len = vldata1[i].len;
	    /* Warning, this only works for cases described for nc_free_vlen() */
            if (!(att->vldata[i].p = malloc(base_typelen * att->vldata[i].len)))
               BAIL(NC_ENOMEM);
            memcpy(att->vldata[i].p, vldata1[i].p, base_typelen * att->vldata[i].len);
         }
      }
      else if (type_class == NC_STRING)
      {
         LOG((4, "copying array of NC_STRING"));
         if (!(att->stdata = malloc(sizeof(char *) * att->len))) {
            BAIL(NC_ENOMEM);
         }

         /* If we are overwriting an existing attribute,
            specifically an NC_CHAR, we need to clean up
            the pre-existing att->data. */
         if (!new_att && att->data) {
            free(att->data);
            att->data = NULL;
         }

         for (i = 0; i < att->len; i++)
         {
            if(NULL != ((char **)data)[i]) {
               LOG((5, "copying string %d of size %d", i, strlen(((char **)data)[i]) + 1));
               if (!(att->stdata[i] = strdup(((char **)data)[i])))
                  BAIL(NC_ENOMEM);
            }
            else
               att->stdata[i] = ((char **)data)[i];
         }
      }
      else
      {
         /* [Re]allocate memory for the attribute data */
         if (!new_att)
            free (att->data);
         if (!(att->data = malloc(att->len * type_size)))
            BAIL(NC_ENOMEM);

         /* Just copy the data, for non-atomic types */
         if (type_class == NC_OPAQUE || type_class == NC_COMPOUND || type_class == NC_ENUM)
            memcpy(att->data, data, len * type_size);
         else
         {
            /* Data types are like religions, in that one can convert.  */
            if ((retval = nc4_convert_type(data, att->data, mem_type, file_type,
                                           len, &range_error, NULL,
                                           (h5->cmode & NC_CLASSIC_MODEL))))
               BAIL(retval);
         }
      }
   }
   att->dirty = NC_TRUE;
   att->created = NC_FALSE;

   /* Mark attributes on variable dirty, so they get written */
   if(var)
      var->attr_dirty = NC_TRUE;

exit:
   /* If there was an error return it, otherwise return any potential
      range error value. If none, return NC_NOERR as usual.*/
   if (retval)
      return retval;
   if (range_error)
      return NC_ERANGE;
   return NC_NOERR;
}

/**
 * @internal
 * Write an attribute to a netCDF-4/HDF5 file, converting
 * data type if necessary.
 * Wrapper around nc4_put_att
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param file_type Type of the attribute data in file.
 * @param len Number of elements in attribute array.
 * @param data Attribute data.
 * @param mem_type Type of data in memory.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Invalid parameters.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_EBADNAME Name contains illegal characters.
 * @return ::NC_ENAMEINUSE Name already in use.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NC4_put_att(int ncid, int varid, const char *name, nc_type file_type,
            size_t len, const void *data, nc_type mem_type)
{
   int ret = NC_NOERR;
   NC *nc;
   NC_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp;
    
   /* Find info for this file, group, and h5 info. */
   if ((ret = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return ret;
   assert(nc && grp && h5);

   return nc4_put_att(grp,varid,name,file_type,len,data,mem_type,0);
}
