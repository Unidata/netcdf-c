/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */
/**
 * @file
 * @internal
 * Internal netcdf-4 functions.
 *
 * This file contains functions internal to the netcdf4 library. None of
 * the functions in this file are exposed in the exetnal API. These
 * functions all relate to the manipulation of netcdf-4's in-memory
 * buffer of metadata information, i.e. the linked list of NC
 * structs.
 *
 * @author Ed Hartnett, Dennis Heimbigner, Ward Fisher
 */
#include "config.h"
#include "nc4internal.h"
#include "nc.h" /* from libsrc */
#include "ncdispatch.h" /* from libdispatch */
#include "ncutf8.h"
#include "H5DSpublic.h"

/* These are the default chunk cache sizes for HDF5 files created or
 * opened with netCDF-4. */
extern size_t nc4_chunk_cache_size;
extern size_t nc4_chunk_cache_nelems;
extern float nc4_chunk_cache_preemption;

#ifdef LOGGING
/* This is the severity level of messages which will be logged. Use
   severity 0 for errors, 1 for important log messages, 2 for less
   important, etc. */
int nc_log_level = NC_TURN_OFF_LOGGING;

#endif /* LOGGING */

/**
 * @internal Check and normalize and name.
 *
 * @param name Name to normalize.
 * @param norm_name The normalized name.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EMAXNAME Name too long.
 * @return ::NC_EINVAL NULL given for name.
 * @author Dennis Heimbigner
 */
int
nc4_check_name(const char *name, char *norm_name)
{
   char *temp;
   int retval;

   /* Check for NULL. */
   if (!name)
      return NC_EINVAL;

   assert(norm_name);

   /* Check for NULL. */
   if (!name)
      return NC_EINVAL;

   /* Check the length. */
   if (strlen(name) > NC_MAX_NAME)
      return NC_EMAXNAME;

   /* Make sure this is a valid netcdf name. This should be done
    * before the name is normalized, because it gives better error
    * codes for bad utf8 strings. */
   if ((retval = NC_check_name(name)))
      return retval;

   /* Normalize the name. */
   retval = nc_utf8_normalize((const unsigned char *)name,(unsigned char**)&temp);
   if(retval != NC_NOERR)
      return retval;

   if(strlen(temp) > NC_MAX_NAME) {
      free(temp);
      return NC_EMAXNAME;
   }

   strcpy(norm_name, temp);
   free(temp);

   return NC_NOERR;
}

/**
 * @internal Given an NC pointer, add the necessary stuff for a
 * netcdf-4 file.
 *
 * @param nc Pointer to file's NC struct.
 * @param path The file name of the new file.
 * @param mode The mode flag.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_nc4f_list_add(NC *nc, const char *path, int mode)
{
   NC_FILE_INFO_T *h5;

   assert(nc && !NC4_DATA(nc) && path);

   /* We need to malloc and
      initialize the substructure NC_HDF_FILE_INFO_T. */
   if (!(h5 = calloc(1, sizeof(NC_FILE_INFO_T))))
      return NC_ENOMEM;
   NC4_DATA_SET(nc,h5);
   h5->controller = nc;

   /* Hang on to cmode, and note that we're in define mode. */
   h5->cmode = mode | NC_INDEF;

   /* The next_typeid needs to be set beyond the end of our atomic
    * types. */
   h5->next_typeid = NC_FIRSTUSERTYPEID;

   h5->alldims = nclistnew();
   h5->alltypes = nclistnew();
   h5->allgroups = nclistnew();

   /* There's always at least one open group - the root
    * group. Allocate space for one group's worth of information. Set
    * its hdf id, name, and a pointer to it's file structure. */
   return nc4_build_root_grp(h5);
}

/**
 * @internal Given an ncid, find the relevant group and return a
 * pointer to it, return an error of this is not a netcdf-4 file (or
 * if strict nc3 is turned on for this file.)
 *
 * @param ncid File and group ID.
 * @param grp Pointer that gets pointer to group info struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOTNC4 Not a netCDF-4 file.
 * @return ::NC_ESTRICTNC3 Not allowed for classic model.
 * @author Ed Hartnett
 */
int
nc4_find_nc4_grp(int ncid, NC_GRP_INFO_T **grp)
{
   NC_FILE_INFO_T* h5;
   NC *f = nc4_find_nc_file(ncid,&h5);
   if(f == NULL) return NC_EBADID;

   /* No netcdf-3 files allowed! */
   if (!h5) return NC_ENOTNC4;
   assert(h5->root_grp);

   /* This function demands netcdf-4 files without strict nc3
    * rules.*/
   if (h5->cmode & NC_CLASSIC_MODEL) return NC_ESTRICTNC3;

   /* If we can't find it, the grp id part of ncid is bad. */
   if (!(*grp = nc4_rec_find_grp(h5, (ncid & GRP_ID_MASK))))
      return NC_EBADID;
   return NC_NOERR;
}

/**
 * @internal Given an ncid, find the relevant group and return a
 * pointer to it, also set a pointer to the nc4_info struct of the
 * related file. For netcdf-3 files, *h5 will be set to NULL.
 *
 * @param ncid File and group ID.
 * @param grpp Pointer that gets pointer to group info struct.
 * @param h5p Pointer to HDF5 file struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett
 */
int
nc4_find_grp_h5(int ncid, NC_GRP_INFO_T **grpp, NC_FILE_INFO_T **h5p)
{
   NC_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp;
   NC *f = nc4_find_nc_file(ncid,&h5);
   if(f == NULL) return NC_EBADID;
   if (h5) {
      assert(h5->root_grp);
      /* If we can't find it, the grp id part of ncid is bad. */
      if (!(grp = nc4_rec_find_grp(h5, (ncid & GRP_ID_MASK))))
         return NC_EBADID;
      h5 = (grp)->nc4_info;
      assert(h5);
   } else {
      h5 = NULL;
      grp = NULL;
   }
   if(h5p) *h5p = h5;
   if(grpp) *grpp = grp;
   return NC_NOERR;
}

/**
 * @internal Find info for this file and group, and set pointer to each.
 *
 * @param ncid File and group ID.
 * @param nc Pointer that gets a pointer to the file's NC struct.
 * @param grpp Pointer that gets a pointer to the group struct.
 * @param h5p Pointer that gets HDF5 file struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett
 */
int
nc4_find_nc_grp_h5(int ncid, NC **nc, NC_GRP_INFO_T **grpp,
                   NC_FILE_INFO_T **h5p)
{
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T* h5;
   NC *f = nc4_find_nc_file(ncid,&h5);

   if(f == NULL) return NC_EBADID;
   if(nc) *nc = f;

   if (h5) {
      assert(h5->root_grp);
      /* If we can't find it, the grp id part of ncid is bad. */
      if (!(grp = nc4_rec_find_grp(h5, (ncid & GRP_ID_MASK))))
         return NC_EBADID;

      h5 = (grp)->nc4_info;
      assert(h5);
   } else {
      h5 = NULL;
      grp = NULL;
   }
   if(h5p) *h5p = h5;
   if(grpp) *grpp = grp;
   return NC_NOERR;
}

/**
 * @internal Use NC_FILE_INFO_T->allgroups to locate a group id.
 *
 * @param h5 Pointer to file info
 * @param target_nc_grpid Group ID to be found.
 *
 * @return Pointer to group info struct, or NULL if not found.
 * @author Ed Hartnett
 */
NC_GRP_INFO_T *
nc4_rec_find_grp(NC_FILE_INFO_T *h5, int target_nc_grpid)
{
   NC_GRP_INFO_T *g;

   assert(h5);

   /* Is this the group we are searching for? */
   g = nclistget(h5->allgroups,target_nc_grpid);
   return g;
}

/**
 * @internal Given an ncid and varid, get pointers to the group and var
 * metadata.
 *
 * @param nc Pointer to file's NC struct.
 * @param ncid File ID.
 * @param varid Variable ID.
 * @param grp Pointer that gets pointer to group info.
 * @param var Pointer that gets pointer to var info.
 *
 * @return ::NC_NOERR No error.
 */
int
nc4_find_g_var_nc(NC *nc, int ncid, int varid,
                  NC_GRP_INFO_T **grp, NC_VAR_INFO_T **var)
{
   NC_FILE_INFO_T* h5 = NC4_DATA(nc);

   /* Find the group info. */
   assert(grp && var && h5 && h5->root_grp);
   *grp = nc4_rec_find_grp(h5, (ncid & GRP_ID_MASK));

   /* It is possible for *grp to be NULL. If it is,
      return an error. */
   if(*grp == NULL)
      return NC_EBADID;

   /* Find the var info. */
   (*var) = (NC_VAR_INFO_T*)ncindexith((*grp)->vars,varid);
   if((*var) == NULL)
	return NC_ENOTVAR;

   return NC_NOERR;
}

/**
 * @internal Find a dim in a grp (or its parents).
 *
 * @param grp Pointer to group info struct.
 * @param dimid Dimension ID to find.
 * @param dim Pointer that gets pointer to dim info if found.
 * @param dim_grp Pointer that gets pointer to group info of group that contains dimension.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADDIM Dimension not found.
 * @author Ed Hartnett
 */
int
nc4_find_dim(NC_GRP_INFO_T *grp, int dimid, NC_DIM_INFO_T **dim,
             NC_GRP_INFO_T **dim_grp)
{
   NC_GRP_INFO_T *g;
   int found = 0;
   NC_FILE_INFO_T* h5 = grp->nc4_info;

   assert(h5 && grp && dim);

   /* Find the dim info. */
   (*dim) = nclistget(h5->alldims,dimid);
   if((*dim) == NULL)
	return NC_EBADDIM;

   /* Redundant: Verify that this dim is in fact in the group or its parent */
   for (found=0, g = grp; g ; g = g->parent) {
	if(g == (*dim)->container) {found = 1; break;}
   }
   /* If we didn't find it, return an error. */
   assert(found);

   /* Give the caller the group the dimension is in. */
   if (dim_grp)
      *dim_grp = (*dim)->container;

   return NC_NOERR;
}

/**
 * @internal Find a var (by name) in a grp.
 *
 * @param grp Pointer to group info.
 * @param name Name of var to find.
 * @param var Pointer that gets pointer to var info struct, if found.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_find_var(NC_GRP_INFO_T *grp, const char *name, NC_VAR_INFO_T **var)
{
   assert(grp && var && name);

   /* Find the var info. */
   *var = (NC_VAR_INFO_T*)ncindexlookup(grp->vars,name);
   return NC_NOERR;
}

/**
 * @internal Locate netCDF type by name.
 *
 * @param start_grp Pointer to starting group info.
 * @param name Name of type to find.
 *
 * @return Pointer to type info, or NULL if not found.
 * @author Ed Hartnett
 */
NC_TYPE_INFO_T *
nc4_rec_find_named_type(NC_GRP_INFO_T *start_grp, char *name)
{
   NC_GRP_INFO_T *g;
   NC_TYPE_INFO_T *type, *res;
   int i;

   assert(start_grp);

   /* Does this group have the type we are searching for? */
   type  = (NC_TYPE_INFO_T*)ncindexlookup(start_grp->type,name);
   if(type != NULL)
	return type;

   /* Search subgroups. */
   for(i=0;i<ncindexsize(start_grp->children);i++) {
      g = (NC_GRP_INFO_T*)ncindexith(start_grp->children,i);
      if(g == NULL) continue;
      if ((res = nc4_rec_find_named_type(g, name)))
          return res;
   }
   /* Can't find it. Oh, woe is me! */
   return NULL;
}

/**
 * @internal Recursively hunt for a netCDF type id.
 * Note using h5->alltypes, we no longer do recursion
 *
 * @param h5 the root file
 * @param target_nc_typeid NetCDF type ID to find.
 *
 * @return Pointer to type info, or NULL if not found.
 * @author Ed Hartnett
 */
NC_TYPE_INFO_T *
nc4_rec_find_nc_type(NC_FILE_INFO_T *h5, nc_type target_nc_typeid)
{
   assert(h5);
   return nclistget(h5->alltypes, target_nc_typeid);
}

/**
 * @internal Use a netCDF typeid to find a type in a type_list.
 *
 * @param h5 Pointer to HDF5 file info struct.
 * @param typeid The netCDF type ID.
 * @param type Pointer to pointer to the list of type info structs.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Invalid input.
 * @author Ed Hartnett
 */
int
nc4_find_type(const NC_FILE_INFO_T *h5, nc_type typeid, NC_TYPE_INFO_T **type)
{
   if (typeid < 0 || !type)
      return NC_EINVAL;
   *type = NULL;

   /* Atomic types don't have associated NC_TYPE_INFO_T struct, just
    * return NOERR. */
   if (typeid <= NC_STRING)
      return NC_NOERR;

   /* Find the type. */
   *type = (NC_TYPE_INFO_T*)nclistget(h5->alltypes,typeid);
   if((*type) == NULL)
      return NC_EBADTYPID;
   return NC_NOERR;
}

/**
 * @internal Given a group, find an att. If name is provided, use that,
 * otherwise use the attnum.
 *
 * @param grp Pointer to group info struct.
 * @param varid Variable ID.
 * @param name Name to of attribute.
 * @param attnum Number of attribute.
 * @param att Pointer to pointer that gets attribute info struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_ENOTATT Attribute not found.
 * @author Ed Hartnett
 */
int
nc4_find_grp_att(NC_GRP_INFO_T *grp, int varid, const char *name, int attnum,
                 NC_ATT_INFO_T **att)
{
   NC_VAR_INFO_T *var;
   NCindex* attlist = NULL;
   int retval;

   assert(grp && grp->hdr.name);
   LOG((4, "nc4_find_grp_att: grp->name %s varid %d name %s attnum %d",
        grp->hdr.name, varid, name, attnum));

   /* Get either the global or a variable attribute list. */
   if (varid == NC_GLOBAL)
   {
      attlist = grp->att;

      /* Do we need to read the atts? */
      if (grp->atts_not_read)
         if ((retval = nc4_read_atts(grp, NULL)))
            return retval;
   }
   else
   {
      var = (NC_VAR_INFO_T*)ncindexith(grp->vars,varid);
      if (!var) return NC_ENOTVAR;

      /* Do we need to read the var attributes? */
      if (var->atts_not_read)
         if ((retval = nc4_read_atts(grp, var)))
            return retval;

      attlist = var->att;
      assert(var->hdr.id == varid);
   }

   /* Now find the attribute by name or number. If a name is provided,
    * ignore the attnum. */
   if(attlist) {
      NC_ATT_INFO_T* a;
      if(name != NULL)
          a = (NC_ATT_INFO_T*)ncindexlookup(attlist,name);
      else
          a = (NC_ATT_INFO_T*)ncindexith(attlist,attnum);
      if(a != NULL) {
	*att = a;
	return NC_NOERR;
      }
   }

   /* If we get here, we couldn't find the attribute. */
   return NC_ENOTATT;
}

/**
 * @internal Given an ncid, varid, and name or attnum, find and return
 * pointer to NC_ATT_INFO_T metadata.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name to of attribute.
 * @param attnum Number of attribute.
 * @param att Pointer to pointer that gets attribute info struct.

 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_ENOTATT Attribute not found.
 * @author Ed Hartnett
 */
int
nc4_find_nc_att(int ncid, int varid, const char *name, int attnum,
                NC_ATT_INFO_T **att)
{
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *h5;
   int retval;

   LOG((4, "nc4_find_nc_att: ncid 0x%x varid %d name %s attnum %d",
        ncid, varid, name, attnum));

   /* Find info for this file and group, and set pointer to each. */
   if ((retval = nc4_find_grp_h5(ncid, &grp, &h5)))
      return retval;
   assert(grp && h5);

   return nc4_find_grp_att(grp,varid,name,attnum,att);
}


/**
 * @internal Given an id, walk the list and find the appropriate NC.
 *
 * @param ext_ncid File/group ID to find.
 * @param h5p Pointer to pointer that gets the HDF5 file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett, Dennis Heimbigner
 */
NC*
nc4_find_nc_file(int ext_ncid, NC_FILE_INFO_T** h5p)
{
   NC* nc;
   int stat;

   stat = NC_check_id(ext_ncid,&nc);
   if(stat != NC_NOERR)
      nc = NULL;

   if(nc)
      if(h5p) *h5p = (NC_FILE_INFO_T*)nc->dispatchdata;

   return nc;
}

/**
 * @internal Add NC_OBJ to appropriate indices.
 *
 * @param index NCindex into which to insert obj
 * @param obj Pointer to object to add.
 *
 * @author Ed Hartnett, Dennis Heimbigner
 */
static void
obj_list_add(NCindex* index, NC_OBJ* obj)
{
    /* Insert object into the index */
//    ncindexset(index,obj->id,obj);
    ncindexadd(index,obj);
}

/**
 * @internal Add NC_OBJ to allXXX lists in a file
 *
 * @param file Pointer to the containing file
 * @param obj Pointer to object to add.
 *
 * @author Dennis Heimbigner
 */
static void
obj_track(NC_FILE_INFO_T* file, NC_OBJ* obj)
{
    NClist* list = NULL;
    /* record the object in the file  */
    switch (obj->sort) {
    case NCDIM: list = file->alldims; break;
    case NCTYP: list = file->alltypes; break;
    case NCGRP: list = file->allgroups; break;
    default:
	assert(NC_FALSE);
    }
    /* Insert at the appropriate point in the list */
    nclistset(list,obj->id,obj);
}

/**
 * @internal Remove object from a list
 *
 * @param index NCindex from which to delete
 * @param obj Pointer to object to delete.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static void
obj_list_del(NCindex* index, NC_OBJ *obj)
{
   ncindexidel(index,obj->id);
}

/**
 * @internal Create a new variable and insert int relevant lists
 *
 * @param grp the containing group
 * @param name the name for the new variable
 * @param ndims the rank of the new variable
 * @param var Pointer in which to return a pointer to the new var.
 *
 * @param var Pointer to pointer that gets variable info struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
nc4_var_list_add(NC_GRP_INFO_T* grp, const char* name, int ndims, NC_VAR_INFO_T **var)
{
   NC_VAR_INFO_T *new_var = NULL;
   int retval = NC_NOERR;

   /* Allocate storage for new variable. */
   if (!(new_var = calloc(1, sizeof(NC_VAR_INFO_T))))
      BAIL(NC_ENOMEM);
   new_var->hdr.sort = NCVAR;

   /* These are the HDF5-1.8.4 defaults. */
   new_var->chunk_cache_size = nc4_chunk_cache_size;
   new_var->chunk_cache_nelems = nc4_chunk_cache_nelems;
   new_var->chunk_cache_preemption = nc4_chunk_cache_preemption;

   /* Now fill in the values in the var info structure. */
   new_var->hdr.id = ncindexsize(grp->vars);

   if (!(new_var->hdr.name = strdup(name)))
      BAIL(NC_ENOMEM);

   new_var->hdr.hashkey = NC_hashmapkey(new_var->hdr.name,strlen(new_var->hdr.name));
   new_var->ndims = ndims;

   /* Allocate space for dimension information. */
   if (ndims)
   {
      if (!(new_var->dim = calloc(ndims, sizeof(NC_DIM_INFO_T *))))
         BAIL(NC_ENOMEM);
      if (!(new_var->dimids = calloc(ndims, sizeof(int))))
         BAIL(NC_ENOMEM);
      /* Initialize dimids to illegal values (-1). See the comment
         in nc4hdf.c#nc4_rec_match_dimscales. */
      memset(new_var->dimids, -1, ndims*sizeof(int));
   }

   new_var->att = ncindexnew(0);

   /* Officially track it */
   ncindexadd(grp->vars,(NC_OBJ*)new_var);

   /* Set the var pointer, if one was given */
   if (var)
      *var = new_var;

exit:
   if(retval != NC_NOERR) {
	nc4_var_list_del(grp,new_var);
   }
   return retval;


}

/**
 * @internal Add to the beginning of a dim list.
 *
 * @param grp container for the dim
 * @param name for the dim
 * @param len for the dim
 * @param assignedid override dimid if >= 0
 * @param dim Pointer to pointer that gets the new dim info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_dim_list_add(NC_GRP_INFO_T* grp, const char* name, size_t len, int assignedid, NC_DIM_INFO_T **dim)
{
   NC_DIM_INFO_T *new_dim = NULL;
   NC_FILE_INFO_T *h5 = grp->nc4_info;
   int retval = NC_NOERR;

   if (!(new_dim = calloc(1, sizeof(NC_DIM_INFO_T))))
      BAIL(NC_ENOMEM);
   new_dim->hdr.sort = NCDIM;

   if(assignedid >= 0)
       new_dim->hdr.id = assignedid;
   else
       new_dim->hdr.id = h5->next_dimid++;

   /* Initialize the metadata for this dimension. */
   if (!(new_dim->hdr.name = strdup(name)))
      BAIL(NC_ENOMEM);
   /* Create a hash of the name. */
   new_dim->hdr.hashkey = NC_hashmapkey(new_dim->hdr.name,strlen(new_dim->hdr.name));
   new_dim->len = len;
   if (len == NC_UNLIMITED)
      new_dim->unlimited = NC_TRUE;
   new_dim->container = grp;

   /* Add object to lists */
   obj_list_add(grp->dim, (NC_OBJ*)new_dim);
   obj_track(h5,(NC_OBJ*)new_dim);

   /* Set the dim pointer, if one was given */
   if (dim)
      *dim = new_dim;

exit:
   if(retval != NC_NOERR)
	nc4_dim_free(new_dim); /* would not be in list yet*/
   return retval;
}

/**
 * @internal Add to the end of an att list.
 *
 * @param list NCindex of att info structs.
 * @param name name of the new attribute
 * @param att Pointer to pointer that gets the new att info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_att_list_add(NCindex* list, const char* name, NC_ATT_INFO_T **att)
{
   int retval = NC_NOERR;
   NC_ATT_INFO_T *new_att = NULL;

   LOG((3, "%s: name %s ", __func__, name));

   if (!(new_att = calloc(1, sizeof(NC_ATT_INFO_T))))
      return NC_ENOMEM;
   new_att->hdr.sort = NCATT;

   /* Fill in the information we know. */
   new_att->hdr.id = ncindexsize(list);
   if (!(new_att->hdr.name = strdup(name)))
      BAIL(NC_ENOMEM);

   /* Create a hash of the name. */
   new_att->hdr.hashkey = NC_hashmapkey(name, strlen(name));

   /* Add object to list as specified by its number */
   obj_list_add(list,(NC_OBJ*)new_att);

   /* Set the attribute pointer, if one was given */
   if (att)
      *att = new_att;

exit:
   if(retval) {
	nc4_att_list_del(list,new_att);
   }
   return retval;
}

/**
 * @internal Add to the end of a group list. Can't use 0 as a
 * new_nc_grpid - it's reserved for the root group.
 *
 * @param parent The parent group.
 * @param name Name of the group.
 * @param grp Pointer to pointer that gets new group info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_grp_list_add(NC_GRP_INFO_T * parent, char *name, NC_GRP_INFO_T **grp)
{
   NC_GRP_INFO_T *new_grp;
   NC_FILE_INFO_T* h5;
   NC* nc;

   h5 = parent->nc4_info;
   nc = h5->controller;

   LOG((3, "%s: name %s ", __func__, name));

   /* Get the memory to store this groups info. */
   if (!(new_grp = calloc(1, sizeof(NC_GRP_INFO_T))))
      return NC_ENOMEM;
   new_grp->hdr.sort = NCGRP;

   /* Fill in this group's information. */
   new_grp->hdr.id = h5->next_nc_grpid++;
   new_grp->parent = parent;
   if (!(new_grp->hdr.name = strdup(name)))
   {
      free(new_grp);
      return NC_ENOMEM;
   }
   new_grp->hdr.hashkey = NC_hashmapkey(new_grp->hdr.name,strlen(new_grp->hdr.name));

   new_grp->nc4_info = NC4_DATA(nc);

   new_grp->children = ncindexnew(0);
   new_grp->dim = ncindexnew(0);
   new_grp->att = ncindexnew(0);
   new_grp->type = ncindexnew(0);
   new_grp->vars = ncindexnew(0);

   /* Add object to lists */
   obj_list_add(parent->children, (NC_OBJ*)new_grp);
   obj_track(h5,(NC_OBJ*)new_grp);

   /* Set the group pointer, if one was given */
   if (grp)
      *grp = new_grp;

   return NC_NOERR;
}

/**
 * @internal Build the root group with id 0.
 *
 * @param h5 root file
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_build_root_grp(NC_FILE_INFO_T* h5)
{
   NC_GRP_INFO_T *new_grp;
   NC* nc;

   assert(h5);

   nc = h5->controller;

   LOG((3, "%s: name %s ", __func__, NC_GROUP_NAME));

   /* Get the memory to store this groups info. */
   if (!(new_grp = calloc(1, sizeof(NC_GRP_INFO_T))))
      return NC_ENOMEM;
   new_grp->hdr.sort = NCGRP;

   /* Fill in this group's information. */
   new_grp->hdr.id = h5->next_nc_grpid++; /* should be 0 */
   assert(new_grp->hdr.id == 0);
   new_grp->parent = NULL;
   if (!(new_grp->hdr.name = strdup(NC_GROUP_NAME)))
   {
      free(new_grp);
      return NC_ENOMEM;
   }
   new_grp->hdr.hashkey = NC_hashmapkey(new_grp->hdr.name,strlen(new_grp->hdr.name));

   new_grp->nc4_info = NC4_DATA(nc);

   new_grp->children = ncindexnew(0);
   new_grp->dim = ncindexnew(0);
   new_grp->att = ncindexnew(0);
   new_grp->type = ncindexnew(0);
   new_grp->vars = ncindexnew(0);

   /* Add object to lists */
   obj_track(h5,(NC_OBJ*)new_grp);

   /* Set the group pointer, if one was given */
   h5->root_grp = new_grp;

   return NC_NOERR;
}

/**
 * @internal Names for groups, variables, and types must not be the
 * same. This function checks that a proposed name is not already in
 * use. Normalzation of UTF8 strings should happen before this
 * function is called.
 *
 * @param grp Pointer to group info struct.
 * @param name Name to check.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENAMEINUSE Name is in use.
 * @author Ed Hartnett
 */
int
nc4_check_dup_name(NC_GRP_INFO_T *grp, char *name)
{
   NC_TYPE_INFO_T *type;
   NC_GRP_INFO_T *g;
   NC_VAR_INFO_T *var;

   /* Any types of this name? */
   type = (NC_TYPE_INFO_T*)ncindexlookup(grp->type,name);
   if(type != NULL)
         return NC_ENAMEINUSE;

   /* Any child groups of this name? */
   g = (NC_GRP_INFO_T*)ncindexlookup(grp->children,name);
   if(g != NULL)
         return NC_ENAMEINUSE;

   /* Any variables of this name? */
   var = (NC_VAR_INFO_T*)ncindexlookup(grp->vars,name);
   if(var != NULL)
         return NC_ENAMEINUSE;

   return NC_NOERR;
}

/**
 * @internal Create a type, but do not add to various lists nor increment its ref count
 *
 * @param grp Pointer to group info struct.
 * @param size Size of type in bytes.
 * @param name Name of type.
 * @param assignedid if >= 0 then override the default type id
 * @param type Pointer that gets pointer to new type info
 *             struct. Ignored if NULL.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett, Ward Fisher
 */
int
nc4_type_new(NC_GRP_INFO_T *grp, size_t size, const char *name, int assignedid, NC_TYPE_INFO_T **type)
{
  NC_TYPE_INFO_T *new_type = NULL;

  /* Allocate memory for the type */
  if (!(new_type = calloc(1, sizeof(NC_TYPE_INFO_T))))
    return NC_ENOMEM;
  new_type->hdr.sort = NCTYP;

  /* Remember info about this type. */
  new_type->hdr.id = assignedid;
  new_type->size = size;
  if (!(new_type->hdr.name = strdup(name))) {
    free(new_type);
    return NC_ENOMEM;
  }

  new_type->hdr.hashkey = NC_hashmapkey(name,strlen(name));

  /* Return a pointer to the new type, if requested */
  if (type)
    *type = new_type;

  return NC_NOERR;
}

/**
 * @internal Add to the end of a type list.
 *
 * @param grp Pointer to group info struct.
 * @param size Size of type in bytes.
 * @param name Name of type.
 * @param type Pointer that gets pointer to new type info
 * struct. Ignored if NULL.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
nc4_type_list_add(NC_GRP_INFO_T *grp, size_t size, const char *name,
                  NC_TYPE_INFO_T **type)
{
   int retval = NC_NOERR;
   NC_TYPE_INFO_T *new_type = NULL;

   retval = nc4_type_new(grp,size,name, grp->nc4_info->next_typeid,&new_type);
   if(retval)
	return retval;
   grp->nc4_info->next_typeid++;

   /* Increment the ref. count on the type */
   new_type->rc++;

   /* Add object to lists */
   obj_list_add(grp->type, (NC_OBJ*)new_type);
   obj_track(grp->nc4_info,(NC_OBJ*)new_type);

   /* Return a pointer to the new type, if requested */
   if (type)
      *type = new_type;

   return NC_NOERR;
}

/**
 * @internal Add to the end of a compound field list.
 *
 * @param parent parent type
 * @param name Name of the field.
 * @param offset Offset in bytes.
 * @param field_hdf_typeid The HDF5 type ID of the field.
 * @param native_typeid The HDF5 native type ID of the field.
 * @param xtype The netCDF type of the field.
 * @param ndims The number of dimensions of the field.
 * @param dim_sizesp An array of dim sizes for the field.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_field_list_add(NC_TYPE_INFO_T *parent, const char *name,
                   size_t offset, hid_t field_hdf_typeid, hid_t native_typeid,
                   nc_type xtype, int ndims, const int *dim_sizesp)
{
   NC_FIELD_INFO_T *field;

   /* Name has already been checked and UTF8 normalized. */
   if (!name)
      return NC_EINVAL;

   /* Allocate storage for this field information. */
   if (!(field = calloc(1, sizeof(NC_FIELD_INFO_T))))
      return NC_ENOMEM;
   field->hdr.sort = NCFLD;

   /* Store the information about this field. */
   if (!(field->hdr.name = strdup(name)))
   {
      free(field);
      return NC_ENOMEM;
   }
   field->hdr.hashkey = NC_hashmapkey(field->hdr.name,strlen(field->hdr.name));
   field->hdf_typeid = field_hdf_typeid;
   field->native_hdf_typeid = native_typeid;
   field->nc_typeid = xtype;
   field->offset = offset;
   field->ndims = ndims;
   if (ndims)
   {
      int i;
      if (!(field->dim_size = malloc(ndims * sizeof(int))))
      {
         free(field->hdr.name);
         free(field);
         return NC_ENOMEM;
      }
      for (i = 0; i < ndims; i++)
         field->dim_size[i] = dim_sizesp[i];
   }

   /* Add object to lists */
   field->hdr.id = nclistlength(parent->u.c.field);
   nclistpush(parent->u.c.field,field);

   return NC_NOERR;
}

/**
 * @internal Add a member to an enum type.
 *
 * @param parent Containing NC_TYPE_INFO_T object
 * @param size Size in bytes of new member.
 * @param name Name of the member.
 * @param value Value to associate with member.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Ed Hartnett
 */
int
nc4_enum_member_add(NC_TYPE_INFO_T *parent, size_t size,
                    const char *name, const void *value)
{
   NC_ENUM_MEMBER_INFO_T *member;

   /* Name has already been checked. */
   assert(name && size > 0 && value);
   LOG((4, "%s: size %d name %s", __func__, size, name));

   /* Allocate storage for this field information. */
   if (!(member = calloc(1, sizeof(NC_ENUM_MEMBER_INFO_T))))
      return NC_ENOMEM;
   if (!(member->value = malloc(size))) {
      free(member);
      return NC_ENOMEM;
   }
   if (!(member->name = strdup(name))) {
      free(member->value);
      free(member);
      return NC_ENOMEM;
   }

   /* Store the value for this member. */
   memcpy(member->value, value, size);

   /* Add object to list */
   nclistpush(parent->u.e.enum_member,member);

   return NC_NOERR;
}

/**
 * @internal Free up a field
 *
 * @param field Pointer to field info of field to delete.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static void
field_free(NC_FIELD_INFO_T *field)
{
   /* Free some stuff. */
   if (field->hdr.name)
      free(field->hdr.name);
   if (field->dim_size)
      free(field->dim_size);

   /* Nc_Free the memory. */
   free(field);
}

/**
 * @internal Free allocated space for type information.
 *
 * @param type Pointer to type info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_type_free(NC_TYPE_INFO_T *type)
{
   int i;

   /* Decrement the ref. count on the type */
   assert(type->rc);
   type->rc--;

   /* Release the type, if the ref. count drops to zero */
   if (0 == type->rc)
   {
      /* Close any open user-defined HDF5 typeids. */
      if (type->hdf_typeid && H5Tclose(type->hdf_typeid) < 0)
         return NC_EHDFERR;
      if (type->native_hdf_typeid && H5Tclose(type->native_hdf_typeid) < 0)
         return NC_EHDFERR;

      /* Free the name. */
      if (type->hdr.name)
         free(type->hdr.name);

      /* Class-specific cleanup */
      switch (type->nc_type_class)
      {
      case NC_COMPOUND:
      {
         NC_FIELD_INFO_T *field;

         /* Delete all the fields in this type (there will be some if its a
          * compound). */
        for(i=0;i<nclistlength(type->u.c.field);i++) {
            field = nclistget(type->u.c.field,i);
            field_free(field);
         }
	 nclistfree(type->u.c.field);
 	 type->u.c.field = NULL; /* belt and suspenders */
      }
      break;

      case NC_ENUM:
      {
         NC_ENUM_MEMBER_INFO_T *enum_member;

         /* Delete all the enum_members, if any. */
	 for(i=0;i<nclistlength(type->u.e.enum_member);i++) {
            enum_member = nclistget(type->u.e.enum_member,i);
            free(enum_member->value);
            free(enum_member->name);
            free(enum_member);
         }
	 nclistfree(type->u.e.enum_member);
 	 type->u.e.enum_member = NULL; /* belt and suspenders */

         if (H5Tclose(type->u.e.base_hdf_typeid) < 0)
            return NC_EHDFERR;
      }
      break;

      case NC_VLEN:
         if (H5Tclose(type->u.v.base_hdf_typeid) < 0)
            return NC_EHDFERR;

      default:
         break;
      }

      /* Release the memory. */
      free(type);
   }

   return NC_NOERR;
}

/**
 * @internal  Delete a var, and free the memory.
 *
 * @param var Pointer to the var info struct of var to delete.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_var_free(NC_VAR_INFO_T *var)
{
   NC_ATT_INFO_T *att;
   int ret = NC_NOERR;
   int i;

   if(var == NULL)
      return NC_NOERR;

   /* First delete all the attributes attached to this var. */
   for(i=0;i<ncindexsize(var->att);i++) {
	att = (NC_ATT_INFO_T*)ncindexith(var->att,i);
	if(att == NULL) continue; /* might be a gap */
        if ((ret = nc4_att_free(att)))
         return ret;
   }
   ncindexfree(var->att);
   var->att = NULL;

   /* Free some things that may be allocated. */
   if (var->chunksizes)
   {free(var->chunksizes);var->chunksizes = NULL;}

   if (var->hdf5_name)
   {free(var->hdf5_name); var->hdf5_name = NULL;}

   if (var->hdr.name)
   {free(var->hdr.name); var->hdr.name = NULL;}

   if (var->dimids)
   {free(var->dimids); var->dimids = NULL;}

   if (var->dim)
   {free(var->dim); var->dim = NULL;}

   /* Delete any fill value allocation. This must be done before the
    * type_info is freed. */
   if (var->fill_value)
   {
      if (var->hdf_datasetid)
      {
         if (var->type_info)
         {
            if (var->type_info->nc_type_class == NC_VLEN)
               nc_free_vlen((nc_vlen_t *)var->fill_value);
            else if (var->type_info->nc_type_class == NC_STRING && *(char **)var->fill_value)
               free(*(char **)var->fill_value);
         }
      }
      free(var->fill_value);
      var->fill_value = NULL;
   }

   /* Release type information */
   if (var->type_info)
   {
      int retval;

      if ((retval = nc4_type_free(var->type_info)))
         return retval;
      var->type_info = NULL;
   }

   /* Delete any HDF5 dimscale objid information. */
   if (var->dimscale_hdf5_objids)
      free(var->dimscale_hdf5_objids);

   /* Delete information about the attachment status of dimscales. */
   if (var->dimscale_attached)
      free(var->dimscale_attached);

   /* Release parameter information. */
   if (var->params)
      free(var->params);

   /* Delete the var. */
   free(var);

   return NC_NOERR;
}

/**
 * @internal  Delete a var, and free the memory.
 *
 * @param grp the containing group
 * @param var Pointer to the var info struct of var to delete.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_var_list_del(NC_GRP_INFO_T* grp, NC_VAR_INFO_T *var)
{
   int i;

   if(var == NULL)
      return NC_NOERR;

   /* Remove from lists */
   if(grp) {
       i = ncindexfind(grp->vars,(NC_OBJ*)var);
       if(i >= 0)
           ncindexidel(grp->vars,i);
   }
   return nc4_var_free(var);
}

/**
 * @internal Free a dim
 *
 * @param dim Pointer to dim info struct of type to delete.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett, Ward Fisher
 */
int
nc4_dim_free(NC_DIM_INFO_T *dim)
{
   /* Free memory allocated for names. */
  if(dim) {
    if (dim->hdr.name)
      free(dim->hdr.name);

    free(dim);
  }
  return NC_NOERR;
}

/**
 * @internal Free a dim and unlist it
 *
 * @param grp Pointer to dim's containing group
 * @param dim Pointer to dim info struct of type to delete.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
nc4_dim_list_del(NC_GRP_INFO_T* grp, NC_DIM_INFO_T *dim)
{
   if(grp && dim) {
	int pos = ncindexfind(grp->dim,(NC_OBJ*)dim);
	if(pos >= 0)
            ncindexidel(grp->dim,pos);
   }
   return nc4_dim_free(dim);
}

/**
 * @internal Recursively delete the data for a group (and everything
 * it contains) in our internal metadata store.
 *
 * @param grp Pointer to group info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_rec_grp_del(NC_GRP_INFO_T *grp)
{
   NC_GRP_INFO_T *g;
   NC_VAR_INFO_T *var;
   NC_ATT_INFO_T *att;
   NC_DIM_INFO_T *dim;
   int retval;
   int i;

   assert(grp);
   LOG((3, "%s: grp->name %s", __func__, grp->hdr.name));

   /* WARNING: for all these deletes, the nc4_xxx_del
      functions will modify the index, so we need to
      not assume any state about them.
   */

   /* Recursively call this function for each child, if any, stopping
    * if there is an error. */
   for(i=0;i<ncindexsize(grp->children);i++) {
	g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if(g == NULL) continue;
        if ((retval = nc4_rec_grp_del(g)))
         return retval;
   }
   ncindexfree(grp->children);
   grp->children = NULL;

   /* Delete all the list contents for vars, dims, and atts, in this
    * group. */
   for(i=0;i<ncindexsize(grp->att);i++) {
	att = (NC_ATT_INFO_T*)ncindexith(grp->att,i);
	if(att == NULL) continue;
        LOG((4, "%s: deleting att %s", __func__, att->hdr.name));
        if ((retval = nc4_att_free(att)))  /* free but leave in parent list */
         return retval;
   }
   ncindexfree(grp->att);
   grp->att = NULL;

   /* Delete all vars. */
   for(i=0;i<ncindexsize(grp->vars);i++) {
      var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
      if(var == NULL) continue;
      LOG((4, "%s: deleting var %s", __func__, var->hdr.name));
      /* Close HDF5 dataset associated with this var, unless it's a
       * scale. */
      if (var->hdf_datasetid && H5Dclose(var->hdf_datasetid) < 0)
         return NC_EHDFERR;
      if ((retval = nc4_var_free(var)))  /* free but leave in parent list */
         return retval;
   }
   ncindexfree(grp->vars);
   grp->vars = NULL;

   /* Delete all dims. */
   for(i=0;i<ncindexsize(grp->dim);i++) {
      dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
      if(dim == NULL) continue;
      LOG((4, "%s: deleting dim %s", __func__, dim->hdr.name));
      /* If this is a dim without a coordinate variable, then close
       * the HDF5 DIM_WITHOUT_VARIABLE dataset associated with this
       * dim. */
      if (dim->hdf_dimscaleid && H5Dclose(dim->hdf_dimscaleid) < 0)
         return NC_EHDFERR;
      if ((retval = nc4_dim_free(dim))) /* free but leave in parent list */
         return retval;
   }
   ncindexfree(grp->dim);
   grp->dim = NULL;

   /* Delete all types. */
   /* Is this code correct? I think it should do repeated passes
      over h5->alltypes using the ref count to decide what to delete */
   for(i=0;i<ncindexsize(grp->type);i++) {
      NC_TYPE_INFO_T* type = (NC_TYPE_INFO_T*)ncindexith(grp->type,i);
      if(type == NULL) continue;
      LOG((4, "%s: deleting type %s", __func__, type->hdr.name));
      if ((retval = nc4_type_free(type))) /* free but leave in parent list */
         return retval;
   }
   ncindexfree(grp->type);
   grp->type = NULL;

   /* Tell HDF5 we're closing this group. */
   LOG((4, "%s: closing group %s", __func__, grp->hdr.name));
   if (grp->hdf_grpid && H5Gclose(grp->hdf_grpid) < 0)
      return NC_EHDFERR;

   /* Free up this group */
   /* Free the name. */
   free(grp->hdr.name);
   free(grp);

   return NC_NOERR;
}

/**
 * @internal Remove a NC_ATT_INFO_T from an index.
 * This will nc_free the memory too.
 *
 * @param list Pointer to pointer of list.
 * @param att Pointer to attribute info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
nc4_att_list_del(NCindex* list, NC_ATT_INFO_T *att)
{
    assert(att != NULL);
    if(list != NULL) obj_list_del(list, (NC_OBJ *)att);
    return nc4_att_free(att);
}

/**
 * @internal Free memory of an attribute object
 *
 * @param att Pointer to attribute info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_att_free(NC_ATT_INFO_T *att)
{
   int i;

   LOG((3, "%s: name %s ", __func__, att->hdr.name));

   /* Free memory that was malloced to hold data for this
    * attribute. */
   if (att->data)
      free(att->data);

   /* Free the name. */
   if (att->hdr.name) {
      free(att->hdr.name);
      att->hdr.name = NULL;
   }

   /* Close the HDF5 typeid. */
   if (att->native_hdf_typeid && H5Tclose(att->native_hdf_typeid) < 0)
      return NC_EHDFERR;

   /* If this is a string array attribute, delete all members of the
    * string array, then delete the array of pointers to strings. (The
    * array was filled with pointers by HDF5 when the att was read,
    * and memory for each string was allocated by HDF5. That's why I
    * use free and not nc_free, because the netCDF library didn't
    * allocate the memory that is being freed.) */
   if (att->stdata)
   {
      for (i = 0; i < att->len; i++)
         if(att->stdata[i])
            free(att->stdata[i]);
      free(att->stdata);
   }

   /* If this att has vlen data, release it. */
   if (att->vldata)
   {
      for (i = 0; i < att->len; i++)
         nc_free_vlen(&att->vldata[i]);
      free(att->vldata);
   }

   free(att);
   return NC_NOERR;
}

/**
 * @internal Normalize a UTF8 name. Put the result in norm_name, which
 * can be NC_MAX_NAME + 1 in size. This function makes sure the free()
 * gets called on the return from utf8proc_NFC, and also ensures that
 * the name is not too long.
 *
 * @param name Name to normalize.
 * @param norm_name The normalized name.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EMAXNAME Name too long.
 * @author Dennis Heimbigner
 */
int
nc4_normalize_name(const char *name, char *norm_name)
{
   char *temp_name;
   int stat = nc_utf8_normalize((const unsigned char *)name,(unsigned char **)&temp_name);
   if(stat != NC_NOERR)
      return stat;
   if (strlen(temp_name) > NC_MAX_NAME)
   {
      free(temp_name);
      return NC_EMAXNAME;
   }
   strcpy(norm_name, temp_name);
   free(temp_name);
   return NC_NOERR;
}

#ifdef ENABLE_SET_LOG_LEVEL

/**
 * @internal Use this to set the global log level. Set it to
 * NC_TURN_OFF_LOGGING (-1) to turn off all logging. Set it to 0 to
 * show only errors, and to higher numbers to show more and more
 * logging details. If logging is not enabled with --enable-logging at
 * configure when building netCDF, this function will do nothing.
 * Note that it is possible to set the log level using the environment
 * variable named _NETCDF_LOG_LEVEL_ (e.g. _export NETCDF_LOG_LEVEL=4_).
 *
 * @param new_level The new logging level.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc_set_log_level(int new_level)
{
#ifdef LOGGING
   if(!nc4_hdf5_initialized)
      nc4_hdf5_initialize();

   /* /\* If the user wants to completely turn off logging, turn off HDF5 */
   /*    logging too. Now I truly can't think of what to do if this */
   /*    fails, so just ignore the return code. *\/ */
   /* if (new_level == NC_TURN_OFF_LOGGING) */
   /* { */
   /*    set_auto(NULL,NULL); */
   /*    LOG((1, "HDF5 error messages turned off!")); */
   /* } */

   /* /\* Do we need to turn HDF5 logging back on? *\/ */
   /* if (new_level > NC_TURN_OFF_LOGGING && */
   /*     nc_log_level <= NC_TURN_OFF_LOGGING) */
   /* { */
   /*    if (set_auto((H5E_auto_t)&H5Eprint, stderr) < 0) */
   /*       LOG((0, "H5Eset_auto failed!")); */
   /*    LOG((1, "HDF5 error messages turned on.")); */
   /* } */

   /* Now remember the new level. */
   nc_log_level = new_level;
   LOG((4, "log_level changed to %d", nc_log_level));
#endif /*LOGGING */
   return 0;
}
#endif /* ENABLE_SET_LOG_LEVEL */

#ifdef LOGGING
#define MAX_NESTS 10
/**
 * @internal Recursively print the metadata of a group.
 *
 * @param grp Pointer to group info struct.
 * @param tab_count Number of tabs.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static int
rec_print_metadata(NC_GRP_INFO_T *grp, int tab_count)
{
   NC_GRP_INFO_T *g;
   NC_ATT_INFO_T *att;
   NC_VAR_INFO_T *var;
   NC_DIM_INFO_T *dim;
   NC_TYPE_INFO_T *type;
   NC_FIELD_INFO_T *field;
   char tabs[MAX_NESTS+1] = "";
   char *dims_string = NULL;
   char temp_string[10];
   int t, retval, d, i;

   /* Come up with a number of tabs relative to the group. */
   for (t = 0; t < tab_count && t < MAX_NESTS; t++)
      tabs[t] = '\t';
   tabs[t] = '\0';

   LOG((2, "%s GROUP - %s nc_grpid: %d nvars: %d natts: %d",
        tabs, grp->hdr.name, grp->hdr.id, ncindexsize(grp->vars), ncindexsize(grp->att)));

   for(i=0;i<ncindexsize(grp->att);i++) {
      att = (NC_ATT_INFO_T*)ncindexith(grp->att,i);
      if(att == NULL) continue;
      LOG((2, "%s GROUP ATTRIBUTE - attnum: %d name: %s type: %d len: %d",
           tabs, att->hdr.id, att->hdr.name, att->nc_typeid, att->len));
   }

   for(i=0;i<ncindexsize(grp->dim);i++) {
      dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
      if(dim == NULL) continue;
      LOG((2, "%s DIMENSION - dimid: %d name: %s len: %d unlimited: %d",
           tabs, dim->hdr.id, dim->hdr.name, dim->len, dim->unlimited));
   }

   for(i=0;i<ncindexsize(grp->vars);i++)
   {
      int j;
      var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
      if (var == NULL) continue;
      if(var->ndims > 0)
      {
         dims_string = (char*)malloc(sizeof(char)*(var->ndims*4));
         strcpy(dims_string, "");
         for (d = 0; d < var->ndims; d++)
         {
            sprintf(temp_string, " %d", var->dimids[d]);
            strcat(dims_string, temp_string);
         }
      }
      LOG((2, "%s VARIABLE - varid: %d name: %s type: %d ndims: %d dimscale: %d dimids:%s endianness: %d, hdf_typeid: %d",
           tabs, var->hdr.id, var->hdr.name, var->type_info->hdr.id, var->ndims, (int)var->dimscale,
           (dims_string ? dims_string : " -"),var->type_info->endianness, var->type_info->native_hdf_typeid));
      for(j=0;j<ncindexsize(var->att);j++) {
         att = (NC_ATT_INFO_T*)ncindexith(var->att,j);
	 if(att == NULL) continue;
         LOG((2, "%s VAR ATTRIBUTE - attnum: %d name: %s type: %d len: %d",
              tabs, att->hdr.id, att->hdr.name, att->nc_typeid, att->len));
      }
      if(dims_string)
      {
         free(dims_string);
         dims_string = NULL;
      }
   }

   for(i=0;i<ncindexsize(grp->type);i++)
   {
      if((type = (NC_TYPE_INFO_T*)ncindexith(grp->type,i)) == NULL) continue;
      LOG((2, "%s TYPE - nc_typeid: %d hdf_typeid: 0x%x committed: %d "
           "name: %s num_fields: %d", tabs, type->hdr.id,
           type->hdf_typeid, type->size, (int)type->committed, type->hdr.name));
      /* Is this a compound type? */
      if (type->nc_type_class == NC_COMPOUND)
      {
	 int j;
         LOG((3, "compound type"));
	 for(j=0;j<nclistlength(type->u.c.field);j++) {
	    field = (NC_FIELD_INFO_T*)nclistget(type->u.c.field,j);
            LOG((4, "field %s offset %d nctype %d ndims %d", field->hdr.name,
                 field->offset, field->nc_typeid, field->ndims));
	 }
      }
      else if (type->nc_type_class == NC_VLEN)
      {
         LOG((3, "VLEN type"));
         LOG((4, "base_nc_type: %d", type->u.v.base_nc_typeid));
      }
      else if (type->nc_type_class == NC_OPAQUE)
         LOG((3, "Opaque type"));
      else if (type->nc_type_class == NC_ENUM)
      {
         LOG((3, "Enum type"));
         LOG((4, "base_nc_type: %d", type->u.e.base_nc_typeid));
      }
      else
      {
         LOG((0, "Unknown class: %d", type->nc_type_class));
         return NC_EBADTYPE;
      }
   }

   /* Call self for each child of this group. */
   for(i=0;i<ncindexsize(grp->children);i++)
   {
      if((g = (NC_GRP_INFO_T*)ncindexith(grp->children,i)) == NULL) continue;
      if ((retval = rec_print_metadata(g, tab_count + 1)))
            return retval;
   }
   return NC_NOERR;
}

/**
 * @internal Print out the internal metadata for a file. This is
 * useful to check that netCDF is working! Nonetheless, this function
 * will print nothing if logging is not set to at least two.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
log_metadata_nc(NC *nc)
{
   NC_FILE_INFO_T *h5 = NC4_DATA(nc);

   LOG((2, "*** NetCDF-4 Internal Metadata: int_ncid 0x%x ext_ncid 0x%x",
        nc->int_ncid, nc->ext_ncid));
   if (!h5)
   {
      LOG((2, "This is a netCDF-3 file."));
      return NC_NOERR;
   }
   LOG((2, "FILE - path: %s cmode: 0x%x parallel: %d redef: %d "
        "fill_mode: %d no_write: %d next_nc_grpid: %d", nc->path,
        h5->cmode, (int)h5->parallel, (int)h5->redef, h5->fill_mode, (int)h5->no_write,
        h5->next_nc_grpid));
   if(nc_log_level >= 2)
       return rec_print_metadata(h5->root_grp, 0);
   return NC_NOERR;
}

#endif /*LOGGING */

/**
 * @internal Show the in-memory metadata for a netcdf file.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC4_show_metadata(int ncid)
{
   int retval = NC_NOERR;
#ifdef LOGGING
   NC *nc;
   int old_log_level = nc_log_level;

   /* Find file metadata. */
   if (!(nc = nc4_find_nc_file(ncid,NULL)))
      return NC_EBADID;

   /* Log level must be 2 to see metadata. */
   nc_log_level = 2;
   retval = log_metadata_nc(nc);
   nc_log_level = old_log_level;
#endif /*LOGGING*/
   return retval;
}
