/** \file 
The netCDF diskless API file functions.

Copyright 2011, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.
*/

#include "nc4internal.h"
#include "nc.h"
#include <H5DSpublic.h>
#include "ncddispatch.h"
#include "ncdispatch.h"

/* This is to track opened HDF5 objects to make sure they are
 * closed. */
#ifdef EXTRA_TESTS
extern int num_plists;
extern int num_spaces;
#endif /* EXTRA_TESTS */

#define MIN_DEFLATE_LEVEL 0
#define MAX_DEFLATE_LEVEL 9

/* These are the special attributes added by the HDF5 dimension scale
 * API. They will be ignored by netCDF-4. */
#define REFERENCE_LIST "REFERENCE_LIST"
#define CLASS "CLASS"
#define DIMENSION_LIST "DIMENSION_LIST"
#define NAME "NAME"

static int NCD_enddef(int ncid);

/* This is set by nc_set_default_format in libsrc/nc.c. */
extern int default_create_format;

/* For performance, fill this array only the first time, and keep it
 * in global memory for each further use. */
#define NUM_TYPES 12

int nc4_free_global_hdf_string_typeid();

/** \ingroup diskless
Create a diskless file.

\param path The file name of the new file.
\param cmode The creation mode flag.
\param initialsz Ignored by this function.
\param basepe Ignored by this function.
\param chunksizehintp Ignored by this function.
\param use_parallel Ignored by this function.
\param mpidata Ignored by this function.
\param dispatch Pointer to the dispatch table for this file.
\param ncpp Pointer to start of linked list of open files.
\return ::NC_EINVAL Invalid input (check cmode).
*/
int
NCD_create(const char* path, int cmode, size_t initialsz, int basepe, 
	   size_t *chunksizehintp, int use_parallel, void *mpidata,
	   NC_Dispatch *dispatch, NC **ncpp)
{
   NC_FILE_INFO_T *nc_file = NULL;
   int res;

   assert(ncpp && path);

   LOG((1, "NCD_create_file: path %s cmode 0x%x", path, cmode));
   
   /* Check the cmode for validity. */
   if (cmode & NC_MPIIO || cmode & NC_MPIPOSIX
       || cmode & NC_PNETCDF)
      return NC_EINVAL;

   /* Allocate the storage for this file info struct, and fill it with
      zeros. This adds the file metadata to the global list. */
   if ((res = nc4_file_list_add(&nc_file, dispatch)))
      return res;

   /* Apply default create format. */
   if (default_create_format == NC_FORMAT_64BIT)
      cmode |= NC_64BIT_OFFSET;
   else if (default_create_format == NC_FORMAT_NETCDF4)
      cmode |= NC_NETCDF4;
   else if (default_create_format == NC_FORMAT_NETCDF4_CLASSIC)
   {
      cmode |= NC_NETCDF4;
      cmode |= NC_CLASSIC_MODEL;
   }
   LOG((2, "cmode after applying default format: 0x%x", cmode));

   /* Fill in information we already know. */
   nc_file->int_ncid = nc_file->ext_ncid;

   /* Add necessary structs to hold netcdf-4 file data. */
   if ((res = nc4_nc4f_list_add(nc_file, path, (NC_WRITE | cmode))))
      return res;
   assert(nc_file->nc4_info && nc_file->nc4_info->root_grp);

   *ncpp = (NC *)nc_file;
   return NC_NOERR;
}

/* Given index, get the HDF5 name of an object and the class of the
 * object (group, type, dataset, etc.). This function will try to use
 * creation ordering, but if that fails it will use default
 * (i.e. alphabetical) ordering. (This is necessary to read existing
 * HDF5 archives without creation ordering). */
/* static int */
/* get_name_by_idx(NC_HDF5_FILE_INFO_T *h5, hid_t hdf_grpid, int i, */
/* 		int *obj_class, char *obj_name) */
/* { */
/*    H5O_info_t obj_info; */
/*    H5_index_t idx_field = H5_INDEX_CRT_ORDER; */
/*    ssize_t size; */
/*    herr_t res; */

/*    /\* These HDF5 macros prevent an HDF5 error message when a */
/*     * non-creation-ordered HDF5 file is opened. *\/ */
/*    H5E_BEGIN_TRY { */
/*       res = H5Oget_info_by_idx(hdf_grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, */
/* 			       i, &obj_info, H5P_DEFAULT); */
/*    } H5E_END_TRY; */
   
/*    /\* Creation ordering not available, so make sure this file is */
/*     * opened for read-only access. This is a plain old HDF5 file being */
/*     * read by netCDF-4. *\/ */
/*    if (res < 0) */
/*    { */
/*       if (H5Oget_info_by_idx(hdf_grpid, ".", H5_INDEX_NAME, H5_ITER_INC, */
/* 			     i, &obj_info, H5P_DEFAULT) < 0) */
/* 	 return NC_EHDFERR; */
/*       if (!h5->no_write) */
/* 	 return NC_ECANTWRITE; */
/*       h5->ignore_creationorder = 1; */
/*       idx_field = H5_INDEX_NAME; */
/*    } */

/*    *obj_class = obj_info.type; */
/*    if ((size = H5Lget_name_by_idx(hdf_grpid, ".", idx_field, H5_ITER_INC, i, */
/* 				  NULL, 0, H5P_DEFAULT)) < 0) */
/*       return NC_EHDFERR; */
/*    if (size > NC_MAX_NAME) */
/*       return NC_EMAXNAME; */
/*    if (H5Lget_name_by_idx(hdf_grpid, ".", idx_field, H5_ITER_INC, i, */
/* 			  obj_name, size+1, H5P_DEFAULT) < 0) */
/*       return NC_EHDFERR; */

/*    LOG((4, "get_name_by_idx: encountered HDF5 object obj_name %s", obj_name)); */

/*    return NC_NOERR; */
/* } */

/** \internal 
This struct is used to pass information back from the callback
function used with H5Literate. 
*/
struct nc_hdf5_link_info 
{
   char name[NC_MAX_NAME + 1];
   H5I_type_t obj_type;   
};   

int
NCD_open(const char *path, int mode, int basepe, size_t *chunksizehintp, 
	 int use_parallel, void *mpidata, NC_Dispatch *dispatch, NC **ncpp)
{
   return NC_EINVAL;
}

/* Unfortunately HDF only allows specification of fill value only when
   a dataset is created. Whereas in netcdf, you first create the
   variable and then (optionally) specify the fill value. To
   accomplish this in HDF5 I have to delete the dataset, and recreate
   it, with the fill value specified. */
int 
NCD_set_fill(int ncid, int fillmode, int *old_modep)
{
   NC_FILE_INFO_T *nc;
 
   LOG((2, "nc_set_fill: ncid 0x%x fillmode %d", ncid, fillmode));

   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Is this a netcdf-3 file? */
   assert(nc->nc4_info);

   /* Trying to set fill on a read-only file? You sicken me! */
   if (nc->nc4_info->no_write)
      return NC_EPERM;

   /* Did you pass me some weird fillmode? */
   if (fillmode != NC_FILL && fillmode != NC_NOFILL)
      return NC_EINVAL;

   /* If the user wants to know, tell him what the old mode was. */
   if (old_modep)
      *old_modep = nc->nc4_info->fill_mode;

   nc->nc4_info->fill_mode = fillmode;

   return NC_NOERR;
}

/* Put the file back in redef mode. This is done automatically for
 * netcdf-4 files, if the user forgets. */
int
NCD_redef(int ncid)
{
   NC_FILE_INFO_T *nc;

   LOG((1, "nc_redef: ncid 0x%x", ncid));

   /* Find this file's metadata. */
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Handle netcdf-3 files. */
   assert(nc->nc4_info);

   /* If we're already in define mode, return an error. */
   if (nc->nc4_info->flags & NC_INDEF)
      return NC_EINDEFINE;

   /* If the file is read-only, return an error. */
   if (nc->nc4_info->no_write)
      return NC_EPERM;

   /* Set define mode. */
   nc->nc4_info->flags |= NC_INDEF;

   /* For nc_abort, we need to remember if we're in define mode as a
      redef. */
   nc->nc4_info->redef++;

   return NC_NOERR;
}

/* For netcdf-4 files, this just calls nc_enddef, ignoring the extra
 * parameters. */
int
NCD__enddef(int ncid, size_t h_minfree, size_t v_align,
	    size_t v_minfree, size_t r_align)
{
   NC_FILE_INFO_T *nc;

   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   return NCD_enddef(ncid);
}

/* Take the file out of define mode. This is called automatically for
 * netcdf-4 files, if the user forgets. */
static int NCD_enddef(int ncid)
{
   NC_FILE_INFO_T *nc;

   LOG((1, "nc_enddef: ncid 0x%x", ncid));

   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Take care of netcdf-3 files. */
   assert(nc->nc4_info);

   return nc4_enddef_netcdf4_file(nc->nc4_info);
}

/* This function will write all changed metadata, and (someday) reread
 * all metadata from the file. */
static int
sync_netcdf4_file(NC_HDF5_FILE_INFO_T *h5)
{
   int retval;

   assert(h5);
   LOG((3, "sync_netcdf4_file"));

   /* If we're in define mode, that's an error, for strict nc3 rules,
    * otherwise, end define mode. */
   if (h5->flags & NC_INDEF)
   {
      if (h5->cmode & NC_CLASSIC_MODEL)
	 return NC_EINDEFINE;

      /* Turn define mode off. */
      h5->flags ^= NC_INDEF;
      
      /* Redef mode needs to be tracked seperately for nc_abort. */
      h5->redef = 0;
   }

#ifdef LOGGING
   /* This will print out the names, types, lens, etc of the vars and
      atts in the file, if the logging level is 2 or greater. */ 
   log_metadata_nc(h5->root_grp->file);
#endif

   /* Write any metadata that has changed. */
   if (!(h5->cmode & NC_NOWRITE))
   {
      if ((retval = nc4_rec_write_types(h5->root_grp)))
	 return retval;
      if ((retval = nc4_rec_write_metadata(h5->root_grp)))
	 return retval;
   }

   H5Fflush(h5->hdfid, H5F_SCOPE_GLOBAL);

   /* Reread all the metadata. */
   /*if ((retval = nc4_rec_read_metadata(grp)))
     return retval;*/

   return retval;
}

/* Flushes all buffers associated with the file, after writing all
   changed metadata. This may only be called in data mode. */
int
NCD_sync(int ncid)
{
   NC_FILE_INFO_T *nc;
   int retval;

   LOG((2, "nc_sync: ncid 0x%x", ncid));

   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* Take care of netcdf-3 files. */
   assert(nc->nc4_info);

   /* If we're in define mode, we can't sync. */
   if (nc->nc4_info && nc->nc4_info->flags & NC_INDEF)
   {
      if (nc->nc4_info->cmode & NC_CLASSIC_MODEL)
	 return NC_EINDEFINE;
      if ((retval = nc_enddef(ncid)))
	 return retval;
   }

   return sync_netcdf4_file(nc->nc4_info);
}

/* This function will free all allocated metadata memory, and close
   the HDF5 file. The group that is passed in must be the root group
   of the file. */
static int
close_netcdf4_file(NC_HDF5_FILE_INFO_T *h5, int abort)
{
   int retval;

   assert(h5 && h5->root_grp);
   LOG((3, "close_netcdf4_file: h5->path %s abort %d", 
	h5->path, abort));

   /* According to the docs, always end define mode on close. */
   if (h5->flags & NC_INDEF)
      h5->flags ^= NC_INDEF;

   /* Delete all the list contents for vars, dims, and atts, in each
    * group. */
   if ((retval = nc4_rec_grp_del(&h5->root_grp, h5->root_grp)))
      return retval;

   /* Delete the memory for the path, if it's been allocated. */
   if (h5->path)
      free(h5->path);

   /* Free the nc4_info struct. */
   free(h5);
   return NC_NOERR;
}

/* From the netcdf-3 docs: The function nc_abort just closes the
   netCDF dataset, if not in define mode. If the dataset is being
   created and is still in define mode, the dataset is deleted. If
   define mode was entered by a call to nc_redef, the netCDF dataset
   is restored to its state before definition mode was entered and the
   dataset is closed. */
int
NCD_abort(int ncid)
{
   NC_FILE_INFO_T *nc;
   int delete_file = 0;
   char path[NC_MAX_NAME + 1];
   int retval = NC_NOERR;

   LOG((2, "nc_abort: ncid 0x%x", ncid));

   /* Find metadata for this file. */
   if (!(nc = nc4_find_nc_file(ncid)))
      return NC_EBADID;

   /* If this is a netcdf-3 file, let the netcdf-3 library handle it. */
   assert(nc->nc4_info);

   /* If we're in define mode, but not redefing the file, delete it. */
   if (nc->nc4_info->flags & NC_INDEF && !nc->nc4_info->redef)
   {
      delete_file++;
      strcpy(path, nc->nc4_info->path);
      /*strcpy(path, nc->path);*/
   }

   /* Free any resources the netcdf-4 library has for this file's
    * metadata. */
   if ((retval = close_netcdf4_file(nc->nc4_info, 1)))
      return retval;
   
   /* Delete the file, if we should. */
   if (delete_file)
      remove(path);

   /* Delete this entry from our list of open files. */
   nc4_file_list_del(nc);

   return retval;
}

/* Close the netcdf file, writing any changes first. */
int
NCD_close(int ncid)
{
   NC_GRP_INFO_T *grp;
   NC_FILE_INFO_T *nc;
   NC_HDF5_FILE_INFO_T *h5;
   int retval;

   LOG((1, "nc_close: ncid 0x%x", ncid));

   /* Find our metadata for this file. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   assert(h5 && nc);

   /* This must be the root group. */
   if (grp->parent)
      return NC_EBADGRPID;

   /* Call the nc4 close. */
   if ((retval = close_netcdf4_file(grp->file->nc4_info, 0)))
      return retval;

   /* Delete this entry from our list of open files. */
   if (nc->path)
      free(nc->path);
   nc4_file_list_del(nc);

   /* Reset the ncid numbers if there are no more files open. */
   if(count_NCList() == 0)
      nc4_file_list_free();

   return NC_NOERR;
}

/* It's possible for any of these pointers to be NULL, in which case
   don't try to figure out that value. */
int
NCD_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp)
{
   NC_FILE_INFO_T *nc;
   NC_HDF5_FILE_INFO_T *h5;
   NC_GRP_INFO_T *grp;
   NC_DIM_INFO_T *dim;
   NC_ATT_INFO_T *att;
   NC_VAR_INFO_T *var;
   int retval;

   LOG((2, "nc_inq: ncid 0x%x", ncid)); 

   /* Find file metadata. */
   if ((retval = nc4_find_nc_grp_h5(ncid, &nc, &grp, &h5)))
      return retval;

   /* Netcdf-3 files are already taken care of. */
   assert(h5 && grp && nc);

   /* Count the number of dims, vars, and global atts. */
   if (ndimsp)
   {
      *ndimsp = 0;
      for (dim = grp->dim; dim; dim = dim->next)
	 (*ndimsp)++;
   }
   if (nvarsp)
   {
      *nvarsp = 0;
      for (var = grp->var; var; var= var->next)
	 (*nvarsp)++;
   }
   if (nattsp)
   {
      *nattsp = 0;
      for (att = grp->att; att; att = att->next)
	 (*nattsp)++;
   }

   if (unlimdimidp)
   {
      /* Default, no unlimited dimension */
      int found = 0;
      *unlimdimidp = -1;

      /* If there's more than one unlimited dim, which was not possible
	 with netcdf-3, then only the last unlimited one will be reported
	 back in xtendimp. */
      /* Note that this code is inconsistent with nc_inq_unlimid() */
      for (dim = grp->dim; dim; dim = dim->next)
	 if (dim->unlimited)
	 {
	    *unlimdimidp = dim->dimid;
	    found++;
	    break;
	 }
   }

   return NC_NOERR;   
}

#ifdef EXTRA_TESTS
int
nc_exit()
{
   if (num_plists || num_spaces)
      return NC_EHDFERR;
      
   return NC_NOERR;
}
#endif /* EXTRA_TESTS */



