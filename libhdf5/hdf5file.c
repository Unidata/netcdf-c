/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions. */
/**
 * @file
 * @internal The netCDF-4 file functions.
 *
 * This file is part of netcdf-4, a netCDF-like interface for HDF5, or
 * a HDF5 backend for netCDF, depending on your point of view.
 *
 * @author Ed Hartnett
 */

#include "config.h"
#include "hdf5internal.h"

extern int nc4_vararray_add(NC_GRP_INFO_T *grp, NC_VAR_INFO_T *var);

/* From nc4mem.c */
extern int NC4_extract_file_image(NC_FILE_INFO_T* h5);

/** @internal When we have open objects at file close, should
    we log them or print to stdout. Default is to log. */
#define LOGOPEN 1

/* Define the table of names and properties of attributes that are reserved. */

#define NRESERVED 11 /*|NC_reservedatt|*/

/* Must be in sorted order for binary search */
static const NC_reservedatt NC_reserved[NRESERVED] = {
   {NC_ATT_CLASS, READONLYFLAG|DIMSCALEFLAG},            /*CLASS*/
   {NC_ATT_DIMENSION_LIST, READONLYFLAG|DIMSCALEFLAG},   /*DIMENSION_LIST*/
   {NC_ATT_NAME, READONLYFLAG|DIMSCALEFLAG},             /*NAME*/
   {NC_ATT_REFERENCE_LIST, READONLYFLAG|DIMSCALEFLAG},   /*REFERENCE_LIST*/
   {NC_ATT_FORMAT, READONLYFLAG},                /*_Format*/
   {ISNETCDF4ATT, READONLYFLAG|NAMEONLYFLAG}, /*_IsNetcdf4*/
   {NCPROPS, READONLYFLAG|NAMEONLYFLAG},         /*_NCProperties*/
   {NC_ATT_COORDINATES, READONLYFLAG|DIMSCALEFLAG},      /*_Netcdf4Coordinates*/
   {NC_DIMID_ATT_NAME, READONLYFLAG|DIMSCALEFLAG},       /*_Netcdf4Dimid*/
   {SUPERBLOCKATT, READONLYFLAG|NAMEONLYFLAG},/*_SuperblockVersion*/
   {NC3_STRICT_ATT_NAME, READONLYFLAG},  /*_nc3_strict*/
};

/**
 * @internal Define a binary searcher for reserved attributes
 * @param name for which to search
 * @return pointer to the matchig NC_reservedatt structure.
 */
const NC_reservedatt*
NC_findreserved(const char* name)
{
   int n = NRESERVED;
   int L = 0;
   int R = (n - 1);
   for(;;) {
      if(L > R) break;
      int m = (L + R) / 2;
      const NC_reservedatt* p = &NC_reserved[m];
      int cmp = strcmp(p->name,name);
      if(cmp == 0) return p;
      if(cmp < 0)
         L = (m + 1);
      else /*cmp > 0*/
         R = (m - 1);
   }
   return NULL;
}

extern void reportopenobjects(int log, hid_t);

/* Forward */
static int NC4_enddef(int ncid);
static void dumpopenobjects(NC_FILE_INFO_T* h5);

/**
 * @internal This function will write all changed metadata, and
 * (someday) reread all metadata from the file.
 *
 * @param h5 Pointer to HDF5 file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static int
sync_netcdf4_file(NC_FILE_INFO_T *h5)
{
   NC_HDF5_FILE_INFO_T *hdf5_info;
   int retval;

   assert(h5 && h5->format_file_info);
   LOG((3, "%s", __func__));

   /* If we're in define mode, that's an error, for strict nc3 rules,
    * otherwise, end define mode. */
   if (h5->flags & NC_INDEF)
   {
      if (h5->cmode & NC_CLASSIC_MODEL)
         return NC_EINDEFINE;

      /* Turn define mode off. */
      h5->flags ^= NC_INDEF;

      /* Redef mode needs to be tracked separately for nc_abort. */
      h5->redef = NC_FALSE;
   }

#ifdef LOGGING
   /* This will print out the names, types, lens, etc of the vars and
      atts in the file, if the logging level is 2 or greater. */
   log_metadata_nc(h5->root_grp->nc4_info->controller);
#endif

   /* Write any metadata that has changed. */
   if (!(h5->cmode & NC_NOWRITE))
   {
      nc_bool_t bad_coord_order = NC_FALSE;     /* if detected, propagate to all groups to consistently store dimids */

      if ((retval = nc4_rec_write_groups_types(h5->root_grp)))
         return retval;
      if ((retval = nc4_rec_detect_need_to_preserve_dimids(h5->root_grp, &bad_coord_order)))
         return retval;
      if ((retval = nc4_rec_write_metadata(h5->root_grp, bad_coord_order)))
         return retval;
   }

   /* Flush the HDF5 buffers to disk. */
   hdf5_info = (NC_HDF5_FILE_INFO_T *)h5->format_file_info;
   if (H5Fflush(hdf5_info->hdfid, H5F_SCOPE_GLOBAL) < 0)
      return NC_EHDFERR;

   return retval;
}

/**
 * @internal This function will free all allocated metadata memory,
 * and close the HDF5 file. The group that is passed in must be the
 * root group of the file.
 *
 * @param h5 Pointer to HDF5 file info struct.
 * @param abort True if this is an abort.
 * @param extractmem True if we need to extract and save final inmemory
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_close_netcdf4_file(NC_FILE_INFO_T *h5, int abort, int extractmem)
{
   NC_HDF5_FILE_INFO_T *hdf5_info;
   int retval = NC_NOERR;

   assert(h5 && h5->root_grp && h5->format_file_info);
   LOG((3, "%s: h5->path %s abort %d", __func__, h5->controller->path, abort));
   hdf5_info = (NC_HDF5_FILE_INFO_T *)h5->format_file_info;

   /* According to the docs, always end define mode on close. */
   if (h5->flags & NC_INDEF)
      h5->flags ^= NC_INDEF;

   /* Sync the file, unless we're aborting, or this is a read-only
    * file. */
   if (!h5->no_write && !abort)
      if ((retval = sync_netcdf4_file(h5)))
         goto exit;

   /* Delete all the list contents for vars, dims, and atts, in each
    * group. */
   if ((retval = nc4_rec_grp_del(h5->root_grp)))
      goto exit;

   /* Free lists of dims, groups, and types in the root group. */
   nclistfree(h5->alldims);
   nclistfree(h5->allgroups);
   nclistfree(h5->alltypes);

#ifdef USE_PARALLEL4
   /* Free the MPI Comm & Info objects, if we opened the file in
    * parallel. */
   if (h5->parallel)
   {
      if (h5->comm != MPI_COMM_NULL)
         MPI_Comm_free(&h5->comm);
      if (h5->info != MPI_INFO_NULL)
         MPI_Info_free(&h5->info);
   }
#endif

   /* Free the fileinfo struct, which holds info from the fileinfo
    * hidden attribute. */
   if (h5->fileinfo)
      free(h5->fileinfo);

   /* Check to see if this is an in-memory file and we want to get its
      final content. */
   if(extractmem) {
      /* File must be read/write */
      if(!h5->no_write) {
         retval = NC4_extract_file_image(h5);
      }
   }

   /* Close hdf file. */
   if (H5Fclose(hdf5_info->hdfid) < 0)
   {
      dumpopenobjects(h5);
   }

   /* Free the HDF5-specific info. */
   if (h5->format_file_info)
      free(h5->format_file_info);

exit:
   /* Free the nc4_info struct; above code should have reclaimed
      everything else */
   if(!retval)
      free(h5);
   return retval;
}

static void
dumpopenobjects(NC_FILE_INFO_T* h5)
{
   NC_HDF5_FILE_INFO_T *hdf5_info;
   int nobjs;

   assert(h5 && h5->format_file_info);
   hdf5_info = (NC_HDF5_FILE_INFO_T *)h5->format_file_info;

   nobjs = H5Fget_obj_count(hdf5_info->hdfid, H5F_OBJ_ALL);

   /* Apparently we can get an error even when nobjs == 0 */
   if(nobjs < 0) {
      return;
   } else if(nobjs > 0) {
      char msg[1024];
      int logit = 0;
      /* If the close doesn't work, probably there are still some HDF5
       * objects open, which means there's a bug in the library. So
       * print out some info on to help the poor programmer figure it
       * out. */
      snprintf(msg,sizeof(msg),"There are %d HDF5 objects open!", nobjs);
#ifdef LOGGING
#ifdef LOGOPEN
      LOG((0, msg));
      logit = 1;
#endif
#else
      fprintf(stdout,"%s\n",msg);
      logit = 0;
#endif
      reportopenobjects(logit,hdf5_info->hdfid);
      fflush(stderr);
   }

   return;
}

size_t nc4_chunk_cache_size = CHUNK_CACHE_SIZE;            /**< Default chunk cache size. */
size_t nc4_chunk_cache_nelems = CHUNK_CACHE_NELEMS;        /**< Default chunk cache number of elements. */
float nc4_chunk_cache_preemption = CHUNK_CACHE_PREEMPTION; /**< Default chunk cache preemption. */


/**
 * Set chunk cache size. Only affects files opened/created *after* it
 * is called.
 *
 * @param size Size in bytes to set cache.
 * @param nelems Number of elements to hold in cache.
 * @param preemption Premption stragety (between 0 and 1).
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Bad preemption.
 * @author Ed Hartnett
 */
int
nc_set_chunk_cache(size_t size, size_t nelems, float preemption)
{
   if (preemption < 0 || preemption > 1)
      return NC_EINVAL;
   nc4_chunk_cache_size = size;
   nc4_chunk_cache_nelems = nelems;
   nc4_chunk_cache_preemption = preemption;
   return NC_NOERR;
}

/**
 * Get chunk cache size. Only affects files opened/created *after* it
 * is called.
 *
 * @param sizep Pointer that gets size in bytes to set cache.
 * @param nelemsp Pointer that gets number of elements to hold in cache.
 * @param preemptionp Pointer that gets premption stragety (between 0 and 1).
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc_get_chunk_cache(size_t *sizep, size_t *nelemsp, float *preemptionp)
{
   if (sizep)
      *sizep = nc4_chunk_cache_size;

   if (nelemsp)
      *nelemsp = nc4_chunk_cache_nelems;

   if (preemptionp)
      *preemptionp = nc4_chunk_cache_preemption;
   return NC_NOERR;
}

/**
 * @internal Set the chunk cache. Required for fortran to avoid size_t
 * issues.
 *
 * @param size Cache size.
 * @param nelems Number of elements.
 * @param preemption Preemption * 100.
 *
 * @return NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc_set_chunk_cache_ints(int size, int nelems, int preemption)
{
   if (size <= 0 || nelems <= 0 || preemption < 0 || preemption > 100)
      return NC_EINVAL;
   nc4_chunk_cache_size = size;
   nc4_chunk_cache_nelems = nelems;
   nc4_chunk_cache_preemption = (float)preemption / 100;
   return NC_NOERR;
}

/**
 * @internal Get the chunk cache settings. Required for fortran to
 * avoid size_t issues.
 *
 * @param sizep Pointer that gets cache size.
 * @param nelemsp Pointer that gets number of elements.
 * @param preemptionp Pointer that gets preemption * 100.
 *
 * @return NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc_get_chunk_cache_ints(int *sizep, int *nelemsp, int *preemptionp)
{
   if (sizep)
      *sizep = (int)nc4_chunk_cache_size;
   if (nelemsp)
      *nelemsp = (int)nc4_chunk_cache_nelems;
   if (preemptionp)
      *preemptionp = (int)(nc4_chunk_cache_preemption * 100);

   return NC_NOERR;
}

/**
 * @internal This will return the length of a netcdf data type in bytes.
 *
 * @param type A netcdf atomic type.
 *
 * @return Type size in bytes, or -1 if type not found.
 * @author Ed Hartnett
 */
int
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
NC4_set_fill(int ncid, int fillmode, int *old_modep)
{
   NC *nc;
   NC_FILE_INFO_T* nc4_info;

   LOG((2, "%s: ncid 0x%x fillmode %d", __func__, ncid, fillmode));

   if (!(nc = nc4_find_nc_file(ncid,&nc4_info)))
      return NC_EBADID;
   assert(nc4_info);

   /* Trying to set fill on a read-only file? You sicken me! */
   if (nc4_info->no_write)
      return NC_EPERM;

   /* Did you pass me some weird fillmode? */
   if (fillmode != NC_FILL && fillmode != NC_NOFILL)
      return NC_EINVAL;

   /* If the user wants to know, tell him what the old mode was. */
   if (old_modep)
      *old_modep = nc4_info->fill_mode;

   nc4_info->fill_mode = fillmode;


   return NC_NOERR;
}

/**
 * @internal Put the file back in redef mode. This is done
 * automatically for netcdf-4 files, if the user forgets.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC4_redef(int ncid)
{
   NC_FILE_INFO_T* nc4_info;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find this file's metadata. */
   if (!(nc4_find_nc_file(ncid,&nc4_info)))
      return NC_EBADID;
   assert(nc4_info);

   /* If we're already in define mode, return an error. */
   if (nc4_info->flags & NC_INDEF)
      return NC_EINDEFINE;

   /* If the file is read-only, return an error. */
   if (nc4_info->no_write)
      return NC_EPERM;

   /* Set define mode. */
   nc4_info->flags |= NC_INDEF;

   /* For nc_abort, we need to remember if we're in define mode as a
      redef. */
   nc4_info->redef = NC_TRUE;

   return NC_NOERR;
}

/**
 * @internal For netcdf-4 files, this just calls nc_enddef, ignoring
 * the extra parameters.
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
NC4__enddef(int ncid, size_t h_minfree, size_t v_align,
            size_t v_minfree, size_t r_align)
{
   if (nc4_find_nc_file(ncid,NULL) == NULL)
      return NC_EBADID;

   return NC4_enddef(ncid);
}

/**
 * @internal Take the file out of define mode. This is called
 * automatically for netcdf-4 files, if the user forgets.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
static int NC4_enddef(int ncid)
{
   NC *nc;
   NC_FILE_INFO_T* nc4_info;
   NC_GRP_INFO_T *grp;
   int i;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   if (!(nc = nc4_find_nc_file(ncid,&nc4_info)))
      return NC_EBADID;
   assert(nc4_info);

   /* Find info for this file and group */
   if (!(grp = nc4_rec_find_grp(nc4_info, (ncid & GRP_ID_MASK))))
      return NC_EBADGRPID;

   /* when exiting define mode, mark all variable written */
   for (i=0; i<ncindexsize(grp->vars); i++) {
      NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
      if(var != NULL) continue;
      var->written_to = NC_TRUE;
   }

   return nc4_enddef_netcdf4_file(nc4_info);
}

/**
 * @internal Flushes all buffers associated with the file, after
 * writing all changed metadata. This may only be called in data mode.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC4_sync(int ncid)
{
   NC *nc;
   int retval;
   NC_FILE_INFO_T* nc4_info;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   if (!(nc = nc4_find_nc_file(ncid,&nc4_info)))
      return NC_EBADID;
   assert(nc4_info);

   /* If we're in define mode, we can't sync. */
   if (nc4_info && nc4_info->flags & NC_INDEF)
   {
      if (nc4_info->cmode & NC_CLASSIC_MODEL)
         return NC_EINDEFINE;
      if ((retval = NC4_enddef(ncid)))
         return retval;
   }

   return sync_netcdf4_file(nc4_info);
}

/**
 * @internal From the netcdf-3 docs: The function nc_abort just closes
 * the netCDF dataset, if not in define mode. If the dataset is being
 * created and is still in define mode, the dataset is deleted. If
 * define mode was entered by a call to nc_redef, the netCDF dataset
 * is restored to its state before definition mode was entered and the
 * dataset is closed.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC4_abort(int ncid)
{
   NC *nc;
   int delete_file = 0;
   char path[NC_MAX_NAME + 1];
   int retval = NC_NOERR;
   NC_FILE_INFO_T* nc4_info;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   /* Find metadata for this file. */
   if (!(nc = nc4_find_nc_file(ncid,&nc4_info)))
      return NC_EBADID;

   assert(nc4_info);

   /* If we're in define mode, but not redefing the file, delete it. */
   if (nc4_info->flags & NC_INDEF && !nc4_info->redef)
   {
      delete_file++;
      strncpy(path, nc->path,NC_MAX_NAME);
   }

   /* Free any resources the netcdf-4 library has for this file's
    * metadata. */
   if ((retval = nc4_close_netcdf4_file(nc4_info, 1, 0)))
      return retval;

   /* Delete the file, if we should. */
   if (delete_file)
      if (remove(path) < 0)
         return NC_ECANTREMOVE;

   return retval;
}

/**
 * @internal Close the netcdf file, writing any changes first.
 *
 * @param ncid File and group ID.
 * @param params any extra parameters in/out of close
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC4_close(int ncid, void* params)
{
   NC_GRP_INFO_T *grp;
   NC *nc;
   NC_FILE_INFO_T *h5;
   int retval;
   int inmemory;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find our metadata for this file. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   assert(nc && h5 && grp);

   /* This must be the root group. */
   if (grp->parent)
      return NC_EBADGRPID;

   inmemory = ((h5->cmode & NC_INMEMORY) == NC_INMEMORY);

   /* Call the nc4 close. */
   if ((retval = nc4_close_netcdf4_file(grp->nc4_info, 0, (inmemory?1:0))))
      return retval;
   if(inmemory && params != NULL) {
      NC_memio* memio = (NC_memio*)params;
      *memio = h5->mem.memio;
   }

   return NC_NOERR;
}

/**
 * @internal Learn number of dimensions, variables, global attributes,
 * and the ID of the first unlimited dimension (if any).
 *
 * @note It's possible for any of these pointers to be NULL, in which
 * case don't try to figure out that value.
 *
 * @param ncid File and group ID.
 * @param ndimsp Pointer that gets number of dimensions.
 * @param nvarsp Pointer that gets number of variables.
 * @param nattsp Pointer that gets number of global attributes.
 * @param unlimdimidp Pointer that gets first unlimited dimension ID,
 * or -1 if there are no unlimied dimensions.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
NC4_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp)
{
   NC *nc;
   NC_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp;
   int retval;
   int i;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   /* Find file metadata. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   assert(h5 && grp && nc);

   /* Count the number of dims, vars, and global atts; need to iterate because of possible nulls */
   if (ndimsp)
   {
      *ndimsp = ncindexcount(grp->dim);
   }
   if (nvarsp)
   {
      *nvarsp = ncindexcount(grp->vars);
   }
   if (nattsp)
   {
      /* Do we need to read the atts? */
      if (grp->atts_not_read)
         if ((retval = nc4_read_atts(grp, NULL)))
            return retval;

      *nattsp = ncindexcount(grp->att);
   }

   if (unlimdimidp)
   {
      /* Default, no unlimited dimension */
      *unlimdimidp = -1;

      /* If there's more than one unlimited dim, which was not possible
         with netcdf-3, then only the last unlimited one will be reported
         back in xtendimp. */
      /* Note that this code is inconsistent with nc_inq_unlimid() */
      for(i=0;i<ncindexsize(grp->dim);i++) {
         NC_DIM_INFO_T* d = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
         if(d == NULL) continue;
         if(d->unlimited) {
            *unlimdimidp = d->hdr.id;
            break;
         }
      }
   }

   return NC_NOERR;
}

/**
 * @internal This function will do the enddef stuff for a netcdf-4 file.
 *
 * @param h5 Pointer to HDF5 file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc4_enddef_netcdf4_file(NC_FILE_INFO_T *h5)
{
   assert(h5);
   LOG((3, "%s", __func__));

   /* If we're not in define mode, return an error. */
   if (!(h5->flags & NC_INDEF))
      return NC_ENOTINDEFINE;

   /* Turn define mode off. */
   h5->flags ^= NC_INDEF;

   /* Redef mode needs to be tracked separately for nc_abort. */
   h5->redef = NC_FALSE;

   return sync_netcdf4_file(h5);
}
