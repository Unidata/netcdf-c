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
#include <errno.h>  /* netcdf functions sometimes return system errors */
#include "nc.h"
#include "nc4internal.h"
#include "nc4dispatch.h"
#include <H5DSpublic.h> /* must be after nc4internal.h */
#include <H5Fpublic.h>
#include <hdf5_hl.h>


/** @internal When we have open objects at file close, should
    we log them or print to stdout. Default is to log. */
#define LOGOPEN 1

#define CD_NELEMS_ZLIB 1 /**< Number of parameters needed for ZLIB filter. */

/* Custom iteration callback data */
typedef struct {
   NC_GRP_INFO_T *grp;
   NC_VAR_INFO_T *var;
} att_iter_info;

/* Define the table of names and properties of attributes that are reserved. */

#define NRESERVED 11 /*|NC_reservedatt|*/

/* Must be in sorted order for binary search */
static const NC_reservedatt NC_reserved[NRESERVED] = {
{NC_ATT_CLASS, READONLYFLAG|DIMSCALEFLAG},		 /*CLASS*/
{NC_ATT_DIMENSION_LIST, READONLYFLAG|DIMSCALEFLAG},	 /*DIMENSION_LIST*/
{NC_ATT_NAME, READONLYFLAG|DIMSCALEFLAG},		 /*NAME*/
{NC_ATT_REFERENCE_LIST, READONLYFLAG|DIMSCALEFLAG},	 /*REFERENCE_LIST*/
{NC_ATT_FORMAT, READONLYFLAG},		 /*_Format*/
{ISNETCDF4ATT, READONLYFLAG|NAMEONLYFLAG}, /*_IsNetcdf4*/
{NCPROPS, READONLYFLAG|NAMEONLYFLAG},	 /*_NCProperties*/
{NC_ATT_COORDINATES, READONLYFLAG|DIMSCALEFLAG},	 /*_Netcdf4Coordinates*/
{NC_DIMID_ATT_NAME, READONLYFLAG|DIMSCALEFLAG},	 /*_Netcdf4Dimid*/
{SUPERBLOCKATT, READONLYFLAG|NAMEONLYFLAG},/*_SuperblockVersion*/
{NC3_STRICT_ATT_NAME, READONLYFLAG},	 /*_nc3_strict*/
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

#ifdef NC4NOTUSED
/**
 * @internal Define the names of attributes to ignore added by the
 * HDF5 dimension scale; these attached to variables. They cannot be
 * modified thru the netcdf-4 API.
 */
static const char* NC_RESERVED_VARATT_LIST[] = {
   NC_ATT_REFERENCE_LIST,
   NC_ATT_CLASS,
   NC_ATT_DIMENSION_LIST,
   NC_ATT_NAME,
   NC_ATT_COORDINATES,
   NC_DIMID_ATT_NAME,
   NULL
};

/**
 * @internal Define the names of attributes to ignore because they are
 * "hidden" global attributes. They can be read, but not modified thru
 * the netcdf-4 API.
 */
static const char* NC_RESERVED_ATT_LIST[] = {
   NC_ATT_FORMAT,
   NC3_STRICT_ATT_NAME,
   NCPROPS,
   ISNETCDF4ATT,
   SUPERBLOCKATT,
   NULL
};

/** 
 * @internal Define the subset of the reserved list that is readable
 * by name only 
*/
static const char* NC_RESERVED_SPECIAL_LIST[] = {
   ISNETCDF4ATT,
   SUPERBLOCKATT,
   NCPROPS,
   NULL
};
#endif

size_t nc4_chunk_cache_size = CHUNK_CACHE_SIZE;            /**< Default chunk cache size. */
size_t nc4_chunk_cache_nelems = CHUNK_CACHE_NELEMS;        /**< Default chunk cache number of elements. */
float nc4_chunk_cache_preemption = CHUNK_CACHE_PREEMPTION; /**< Default chunk cache preemption. */

#define NUM_TYPES 12 /**< Number of netCDF atomic types. */

/** @internal Native HDF5 constants for atomic types. For performance,
 * fill this array only the first time, and keep it in global memory
 * for each further use. */
static hid_t h5_native_type_constant_g[NUM_TYPES];

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
   NC_HDF5_FILE_INFO_T* nc4_info;

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
   NC_HDF5_FILE_INFO_T* nc4_info;

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
   NC_HDF5_FILE_INFO_T *h5;
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

