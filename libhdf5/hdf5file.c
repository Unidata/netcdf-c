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
#include "ncrc.h"

extern int NC4_extract_file_image(NC_FILE_INFO_T* h5); /* In nc4memcb.c */

static void dumpopenobjects(NC_FILE_INFO_T* h5);

/** @internal When we have open objects at file close, should
    we log them or print to stdout. Default is to log. */
#define LOGOPEN 1

/** @internal Number of reserved attributes. These attributes are
 * hidden from the netcdf user, but exist in the HDF5 file to help
 * netcdf read the file. */
#define NRESERVED 11 /*|NC_reservedatt|*/

/** @internal List of reserved attributes. This list must be in sorted
 * order for binary search. */
static const NC_reservedatt NC_reserved[NRESERVED] = {
   {NC_ATT_CLASS, READONLYFLAG|DIMSCALEFLAG},            /*CLASS*/
   {NC_ATT_DIMENSION_LIST, READONLYFLAG|DIMSCALEFLAG},   /*DIMENSION_LIST*/
   {NC_ATT_NAME, READONLYFLAG|DIMSCALEFLAG},             /*NAME*/
   {NC_ATT_REFERENCE_LIST, READONLYFLAG|DIMSCALEFLAG},   /*REFERENCE_LIST*/
   {NC_ATT_FORMAT, READONLYFLAG},               	 /*_Format*/
   {ISNETCDF4ATT, READONLYFLAG|NAMEONLYFLAG}, 		 /*_IsNetcdf4*/
   {NCPROPS, READONLYFLAG|NAMEONLYFLAG|MATERIALIZEDFLAG},/*_NCProperties*/
   {NC_ATT_COORDINATES, READONLYFLAG|DIMSCALEFLAG|MATERIALIZEDFLAG},/*_Netcdf4Coordinates*/
   {NC_DIMID_ATT_NAME, READONLYFLAG|DIMSCALEFLAG|MATERIALIZEDFLAG},/*_Netcdf4Dimid*/
   {SUPERBLOCKATT, READONLYFLAG|NAMEONLYFLAG},/*_SuperblockVersion*/
   {NC3_STRICT_ATT_NAME, READONLYFLAG|MATERIALIZEDFLAG},  /*_nc3_strict*/
};

/* Forward */
static int NC4_enddef(int ncid);
static void dumpopenobjects(NC_FILE_INFO_T* h5);

/**
 * @internal Define a binary searcher for reserved attributes
 * @param name for which to search
 * @return pointer to the matchig NC_reservedatt structure.
 * @return NULL if not found.
 * @author Dennis Heimbigner
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

/**
 * @internal This function will write all changed metadata and flush
 * HDF5 file to disk.
 *
 * @param h5 Pointer to HDF5 file info struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINDEFINE Classic model file in define mode.
 * @return ::NC_EHDFERR HDF5 error.
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
   log_metadata_nc(h5);
#endif

   /* Write any metadata that has changed. */
   if (!h5->no_write)
   {
      nc_bool_t bad_coord_order = NC_FALSE;

      /* Write any user-defined types. */
      if ((retval = nc4_rec_write_groups_types(h5->root_grp)))
         return retval;

      /* Check to see if the coordinate order is messed up. If
       * detected, propagate to all groups to consistently store
       * dimids. */
      if ((retval = nc4_detect_preserve_dimids(h5->root_grp, &bad_coord_order)))
         return retval;

      /* Write all the metadata. */
      if ((retval = nc4_rec_write_metadata(h5->root_grp, bad_coord_order)))
         return retval;

      /* Write out _NCProperties */
      if((retval = NC4_write_ncproperties(h5)))
	return retval;
   }

   /* Tell HDF5 to flush all changes to the file. */
   hdf5_info = (NC_HDF5_FILE_INFO_T *)h5->format_file_info;
   if (H5Fflush(hdf5_info->hdfid, H5F_SCOPE_GLOBAL) < 0)
      return NC_EHDFERR;

   return NC_NOERR;
}

/**
 * @internal This function will free all allocated metadata memory,
 * and close the HDF5 file. The group that is passed in must be the
 * root group of the file. If inmemory is used, then save
 * the final memory in mem.memio.
 *
 * @param h5 Pointer to HDF5 file info struct.
 * @param abort True if this is an abort.
 * @param memio the place to return a core image if not NULL
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF5 could not close the file.
 * @return ::NC_EINDEFINE Classic model file is in define mode.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
nc4_close_netcdf4_file(NC_FILE_INFO_T *h5, int abort, NC_memio *memio)
{
   NC_HDF5_FILE_INFO_T *hdf5_info;
   int retval;

   assert(h5 && h5->root_grp && h5->format_file_info);
   LOG((3, "%s: h5->path %s abort %d", __func__, h5->controller->path, abort));

   /* Get HDF5 specific info. */
   hdf5_info = (NC_HDF5_FILE_INFO_T *)h5->format_file_info;

   /* Delete all the list contents for vars, dims, and atts, in each
    * group. */
   if ((retval = nc4_rec_grp_del(h5->root_grp)))
      return retval;

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
   if (h5->provenance)
      NC4_free_provenance(h5->provenance);
   h5->provenance = NULL; /* Avoid double dealloc */

   /* Close hdf file. It may not be open, since this function is also
    * called by NC_create() when a file opening is aborted. */
   if (hdf5_info->hdfid > 0 && H5Fclose(hdf5_info->hdfid) < 0)
   {
      dumpopenobjects(h5);
      return NC_EHDFERR;
   }

   /* If inmemory is used and user wants the final memory block,
      then capture and return the final memory block else free it */
   if(h5->mem.inmemory) {
       /* Pull out the final memory */
       (void)NC4_extract_file_image(h5);
       if(!abort && memio != NULL) {
	    *memio = h5->mem.memio; /* capture it */
	    h5->mem.memio.memory = NULL; /* avoid duplicate free */
       }
       /* If needed, reclaim extraneous memory */
       if(h5->mem.memio.memory != NULL) {
	    /* If the original block of memory is not resizeable, then
               it belongs to the caller and we should not free it. */
	    if(!h5->mem.locked)
	        free(h5->mem.memio.memory);	
       }
       h5->mem.memio.memory = NULL;
       h5->mem.memio.size = 0;
       NC4_image_finalize(h5->mem.udata);
    }

   /* Free the HDF5-specific info. */
   if (h5->format_file_info)
      free(h5->format_file_info);

   /* Free the nc4_info struct; above code should have reclaimed
      everything else */
   free(h5);

   return NC_NOERR;
}

/**
 * @internal This function will recurse through an open HDF5 file and
 * release resources. All open HDF5 objects in the file will be
 * closed.
 *
 * @param h5 Pointer to HDF5 file info struct.
 * @param abort True if this is an abort.
 * @param memio the place to return a core image if not NULL
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EHDFERR HDF5 could not close the file.
 * @author Ed Hartnett
 */
int
nc4_close_hdf5_file(NC_FILE_INFO_T *h5, int abort,  NC_memio *memio)
{
   int retval;

   assert(h5 && h5->root_grp && h5->format_file_info);
   LOG((3, "%s: h5->path %s abort %d", __func__, h5->controller->path, abort));

   /* According to the docs, always end define mode on close. */
   if (h5->flags & NC_INDEF)
      h5->flags ^= NC_INDEF;

   /* Sync the file, unless we're aborting, or this is a read-only
    * file. */
   if (!h5->no_write && !abort)
      if ((retval = sync_netcdf4_file(h5)))
         return retval;

   /* Close all open HDF5 objects within the file. */
   if ((retval = nc4_rec_grp_HDF5_del(h5->root_grp)))
      return retval;

   /* Release all intarnal lists and metadata associated with this
    * file. All HDF5 objects have already been released. */
   if ((retval = nc4_close_netcdf4_file(h5, abort, memio)))
      return retval;

   return NC_NOERR;
}

/**
 * @internal Output a list of still-open objects in the HDF5
 * file. This is only called if the file fails to close cleanly.
 *
 * @param h5 Pointer to file info.
 *
 * @author Dennis Heimbigner
 */
static void
dumpopenobjects(NC_FILE_INFO_T* h5)
{
   NC_HDF5_FILE_INFO_T *hdf5_info;
   int nobjs;

   assert(h5 && h5->format_file_info);
   hdf5_info = (NC_HDF5_FILE_INFO_T *)h5->format_file_info;

   if(hdf5_info->hdfid <= 0)
	return; /* File was never opened */

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
   NC_FILE_INFO_T *nc4_info;
   int retval;

   LOG((2, "%s: ncid 0x%x fillmode %d", __func__, ncid, fillmode));

   /* Get pointer to file info. */
   if ((retval = nc4_find_grp_h5(ncid, NULL, &nc4_info)))
      return retval;
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
   NC_FILE_INFO_T *nc4_info;
   int retval;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find this file's metadata. */
   if ((retval = nc4_find_grp_h5(ncid, NULL, &nc4_info)))
      return retval;
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
   return NC4_enddef(ncid);
}

/**
 * @internal Take the file out of define mode. This is called
 * automatically for netcdf-4 files, if the user forgets.
 *
 * @param ncid File and group ID.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EBADGRPID Bad group ID.
 * @author Ed Hartnett
 */
static int
NC4_enddef(int ncid)
{
   NC_FILE_INFO_T *nc4_info;
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
   int i;
   int retval;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find pointer to group and nc4_info. */
   if ((retval = nc4_find_nc_grp_h5(ncid, NULL, &grp, &nc4_info)))
      return retval;

   /* When exiting define mode, mark all variable written. */
   for (i = 0; i < ncindexsize(grp->vars); i++)
   {
      var = (NC_VAR_INFO_T *)ncindexith(grp->vars, i);
      assert(var);
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
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EINDEFINE Classic model file is in define mode.
 * @author Ed Hartnett
 */
int
NC4_sync(int ncid)
{
   NC_FILE_INFO_T *nc4_info;
   int retval;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   if ((retval = nc4_find_grp_h5(ncid, NULL, &nc4_info)))
      return retval;
   assert(nc4_info);

   /* If we're in define mode, we can't sync. */
   if (nc4_info->flags & NC_INDEF)
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
   NC_FILE_INFO_T *nc4_info;
   int delete_file = 0;
   char path[NC_MAX_NAME + 1];
   int retval;

   LOG((2, "%s: ncid 0x%x", __func__, ncid));

   /* Find metadata for this file. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, NULL, &nc4_info)))
      return retval;
   assert(nc4_info);

   /* If we're in define mode, but not redefing the file, delete it. */
   if (nc4_info->flags & NC_INDEF && !nc4_info->redef)
   {
      delete_file++;
      strncpy(path, nc->path, NC_MAX_NAME);
   }

   /* Free any resources the netcdf-4 library has for this file's
    * metadata. */
   if ((retval = nc4_close_hdf5_file(nc4_info, 1, NULL)))
      return retval;

   /* Delete the file, if we should. */
   if (delete_file)
      if (remove(path) < 0)
         return NC_ECANTREMOVE;

   return NC_NOERR;
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
   NC_memio* memio = NULL;

   LOG((1, "%s: ncid 0x%x", __func__, ncid));

   /* Find our metadata for this file. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   assert(nc && h5 && grp);

   /* This must be the root group. */
   if (grp->parent)
      return NC_EBADGRPID;

   inmemory = ((h5->cmode & NC_INMEMORY) == NC_INMEMORY);

   if(inmemory && params != NULL) {
      memio = (NC_memio*)params;
   }

   /* Call the nc4 close. */
   if ((retval = nc4_close_hdf5_file(grp->nc4_info, 0, memio)))
      return retval;

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

   /* Count the number of dims, vars, and global atts; need to iterate
    * because of possible nulls. */
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
 * @return ::NC_ENOTINDEFINE Not in define mode.
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
