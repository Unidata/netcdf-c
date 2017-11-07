/*********************************************************************
   Copyright 2010, UCAR/Unidata See netcdf/COPYRIGHT file for
   copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nc.h"
#include <ncdispatch.h>
#ifdef USE_PIO
#include <pio_internal.h>
#endif /* USE_PIO */

#define ID_SHIFT (16)
#define NCFILELISTLENGTH 0x10000

/* Version one just allocates the max space (sizeof(NC*)*2^16)*/
static NC** nc_filelist = NULL;

static int numfiles = 0;

#ifdef USE_PIO
/* This is used to look up the IO system ID info to check that a new
 * ncid is acceptable to all tasks. */
extern int current_iosysid;
#endif

/* Common */
int
count_NCList(void)
{
    return numfiles;
}


void
free_NCList(void)
{
    if(numfiles > 0) return; /* not empty */
    if(nc_filelist != NULL) free(nc_filelist);
    nc_filelist = NULL;
}

/** 
 * Add a file to the array of open files. 
 * 
 * @param ncp pointer of the NC struct to add to the list iof files.
 *
 * @returns 0 for success
 * @returns NC_ENOMEM out of memory or no more open file slots.
 * @author Dennis Heimbigner.
 * @internal
 */
int
add_to_NCList(NC* ncp)
{
    int i;
    int new_id;
    if(nc_filelist == NULL) {
	if (!(nc_filelist = calloc(1, sizeof(NC*)*NCFILELISTLENGTH)))
	    return NC_ENOMEM;
	numfiles = 0;
    }
#ifdef USE_REFCOUNT
    /* Check the refcount */
    if(ncp->refcount > 0)
	return NC_NOERR;
#endif

    new_id = 0; /* id's begin at 1 */
    for(i=1; i < NCFILELISTLENGTH; i++) {
       if(nc_filelist[i] == NULL) {new_id = i; break;}
    }
    if(new_id == 0) return NC_ENOMEM; /* no more slots */
       
    nc_filelist[new_id] = ncp;
    numfiles++;
    new_id = (new_id << ID_SHIFT);
    ncp->ext_ncid = new_id;
    return NC_NOERR;
}

#ifdef USE_PIO

/**
 * Check that a proposed ncid is acceptable to all tasks in the
 * communicator.
 *
 * This function is for PIO. It's possible for different processors on
 * the communicator to have different slots used up in their
 * nc_filelist[] array. This function takes a proposed ncid and checks
 * that it is free with all tasks on the communicator.
 *
 * @param new_id the proposed ncid to be checked.
 * @param id_ok pointer to an int that will get 0 if the proposed ncid
 * is already in use, or one if it is available everywhere.
 *
 * @return 0 for success, error code otherwise.
 * @author Ed Hartnett
 * @internal
 */
static int
pio_check_ncid(int new_id, int *id_ok)
{
    iosystem_desc_t *ios;
    int id_available = 1;
    int mpierr;
    int ret;
    
   if (id_ok)
      *id_ok = 1;

   /* Use the current ID to find the IO system info. */
   if (!(ios = pio_get_iosystem_from_id(current_iosysid)))
      return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);
   
   /* Broadcast proposed ncid to all tasks from io root, necessary
    * because files may be opened on mutilple iosystems, causing the
    * underlying library to reuse ncids. Hilarious confusion
    * ensues. */
   if (ios->async)
   {
      int iorank;
      int id_ok_for_me = 0;
      int my_new_id = new_id;
      int *ok_array = NULL;
      
      if ((mpierr = MPI_Bcast(&my_new_id, 1, MPI_INT, ios->ioroot, ios->my_comm)))
	 return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

      /* Find this tasks rank in IO communicator. */
      if ((ret = PIOc_iotask_rank(current_iosysid, &iorank)))
	 return ret;
      
      if (iorank == 0)
      {
	 /* We know this is ncid good for iorank 0. */
	 id_ok_for_me = 1;

	 /* Allocate array to hold answer from other tasks. */
	 if (!(ok_array = malloc(sizeof(int) * ios->num_uniontasks)))
	    return NC_ENOMEM;
      }
      else
      {
	 /* Every task but ioroot needs to check if this ID is free. */
	 if (!nc_filelist[my_new_id])
	    id_ok_for_me = 1;	    
      }

      /* Get answer from other tasks. */
      if ((mpierr = MPI_Gather(&id_ok_for_me, 1, MPI_INT, ok_array, ios->num_uniontasks, MPI_INT,
			       ios->ioroot, ios->my_comm)))
	 return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

      /* Did they all approve of this proposed ID? */
      for (int i = 0; i < ios->num_uniontasks; i++)
      {
	 if (!ok_array[i])
	 {
	    id_available = 0;
	    break;
	 }
      }

      /* Report result to caller. */
      if (id_ok)
	 *id_ok = id_available;
      
      /* Free resources on ioroot task. */
      if (ok_array)
	 free(ok_array);
   }
   
   return NC_NOERR;
}

/**
 * Add to the NC list of open files, for PIO. This function is only
 * called if PIO is built, and the file is opened or created with
 * NC_PIO in the mode flag.
 *
 * @param ncp pointer to the NC struct.
 *
 * @returns 0 on success, error code otherwise.
 * @author Dennis Heimbigner, Ed Hartnett
 * @internal
 */
int
pio_add_to_NCList(NC* ncp)
{
    int i;
    int new_id;
    int id_ok = 0;
    int ret;

    /* If this is the first file added, allocate the file list. */
    if(nc_filelist == NULL) {
	if (!(nc_filelist = calloc(1, sizeof(NC*)*NCFILELISTLENGTH)))
	    return NC_ENOMEM;
	numfiles = 0;
    }
    
#ifdef USE_REFCOUNT
    /* Check the refcount */
    if(ncp->refcount > 0)
	return NC_NOERR;
#endif

    /* Find the ncid for the file. */
    new_id = 0; /* id's begin at 1 */
    while (!id_ok)
    {
       /* Come up with an ID. */
       for(i=1; i < NCFILELISTLENGTH; i++) {
	  if(nc_filelist[i] == NULL) {new_id = i; break;}
       }

       /* Check that this ID is OK, if we are using PIO. */
       if ((ret = pio_check_ncid(new_id, &id_ok)))
	  return ret;
    }
    
    if(new_id == 0)
       return NC_ENOMEM; /* no more slots */

    /* Add the file. */
    nc_filelist[new_id] = ncp;
    numfiles++;

    /* Shift the ID to leave room for the group ID? */
    new_id = (new_id << ID_SHIFT);
    ncp->ext_ncid = new_id;
    return NC_NOERR;
}
#endif /* USE_PIO */

void
del_from_NCList(NC* ncp)
{
   unsigned int ncid = ((unsigned int)ncp->ext_ncid) >> ID_SHIFT;
   if(numfiles == 0 || ncid == 0 || nc_filelist == NULL) return;
   if(nc_filelist[ncid] != ncp) return;
#ifdef USE_REFCOUNT
   /* Check the refcount */
   if(ncp->refcount > 0)
	return; /* assume caller has decrecmented */
#endif

   nc_filelist[ncid] = NULL;
   numfiles--;

   /* If all files have been closed, release the filelist memory. */
   if (numfiles == 0)
      free_NCList();
}

NC *
find_in_NCList(int ext_ncid)
{
   NC* f = NULL;
   unsigned int ncid = ((unsigned int)ext_ncid) >> ID_SHIFT;
   if(numfiles > 0 && nc_filelist != NULL && ncid < NCFILELISTLENGTH)
	f = nc_filelist[ncid];
   return f;
}

/*
Added to support open by name
*/
NC*
find_in_NCList_by_name(const char* path)
{
   int i;
   NC* f = NULL;
   if(nc_filelist == NULL)
	return NULL;
   for(i=1; i < NCFILELISTLENGTH; i++) {
	if(nc_filelist[i] != NULL) {
	    if(strcmp(nc_filelist[i]->path,path)==0) {
		f = nc_filelist[i];
		break;
	    }				
	}
   }
   return f;
}

int
iterate_NCList(int index, NC** ncp)
{
    /* Walk from 0 ...; 0 return => stop */
    if(index < 0 || index >= NCFILELISTLENGTH)
	return NC_ERANGE;
    if(ncp) *ncp = nc_filelist[index];
    return NC_NOERR;
}

