/*********************************************************************
   Copyright 2010, UCAR/Unidata See netcdf/COPYRIGHT file for
   copying and redistribution conditions.
 *********************************************************************/

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nc.h"
#include "ncglobal.h"

#define ID_SHIFT (16)
#define NCFILELISTLENGTH 0x10000

/* Common */
int
count_NCList(void)
{
    int nfiles;
    LOCK;
    nfiles = nc_global->numfiles;
    UNLOCK;
    return nfiles;
}


void
free_NCList(void)
{
    LOCK;
    if(nc_global->numfiles > 0) goto done; /* not empty */
    if(nc_global->filelist != NULL) free(nc_global->filelist);
    nc_global->filelist = NULL;
done:
    UNLOCK;
    return;
}

int
add_to_NCList(NC* ncp)
{
    int i;
    int new_id;
    int stat = NC_NOERR;
    LOCK;
    if(nc_global->filelist == NULL) {
	if (!(nc_global->filelist = calloc(1, sizeof(NC*)*NCFILELISTLENGTH)))
	    {stat = NC_ENOMEM; goto unlock;}
	nc_global->numfiles = 0;
    }
    UNLOCK;

#ifdef USE_REFCOUNT
    /* Check the refcount */
    if(ncp->refcount > 0)
	{stat = NC_NOERR; goto done;}
#endif

    new_id = 0; /* id's begin at 1 */
    LOCK;
    for(i=1; i < NCFILELISTLENGTH; i++) {
	if(nc_global->filelist[i] == NULL) {new_id = i; break;}
    }
    UNLOCK;
    if(new_id == 0) {stat = NC_ENOMEM; goto done;} /* no more slots */
    LOCK;
    nc_global->filelist[new_id] = ncp;
    nc_global->numfiles++;
    UNLOCK;
    new_id = (new_id << ID_SHIFT);
    ncp->ext_ncid = new_id;
done:
    return stat;
unlock:
    UNLOCK;
    goto done;
}

void
del_from_NCList(NC* ncp)
{
   int nfiles;
   NC* tmp;
   unsigned int ncid = ((unsigned int)ncp->ext_ncid) >> ID_SHIFT;
   LOCK;
   if(nc_global->numfiles == 0 || ncid == 0 || nc_global->filelist == NULL) {
	UNLOCK;
	goto done;
   }
   tmp = nc_global->filelist[ncid];
   UNLOCK;
   if(tmp != ncp) goto done;
#ifdef USE_REFCOUNT
   /* Check the refcount */
   if(ncp->refcount > 0)
	goto done; /* assume caller has decrecmented */
#endif
   LOCK; 
   nc_global->filelist[ncid] = NULL;
   nc_global->numfiles--;
   nfiles = nc_global->numfiles;
   UNLOCK;

   /* If all files have been closed, release the filelist memory. */
   if (nfiles == 0)
      free_NCList();
done:
    return;
}

NC *
find_in_NCList(int ext_ncid)
{
   NC* f = NULL;
   unsigned int ncid = ((unsigned int)ext_ncid) >> ID_SHIFT;
   LOCK;
   if(nc_global->numfiles > 0 && nc_global->filelist != NULL && ncid < NCFILELISTLENGTH)
	f = nc_global->filelist[ncid];
   UNLOCK;
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
   LOCK;
   if(nc_global->filelist == NULL) {
	UNLOCK;
	return NULL;
   }
   LOCK;
   for(i=1; i < NCFILELISTLENGTH; i++) {
	if(nc_global->filelist[i] != NULL) {
	    if(strcmp(nc_global->filelist[i]->path,path)==0) {
		f = nc_global->filelist[i];
		break;
	    }				
	}
   }
   UNLOCK;
   return f;
}

#if 0
int
iterate_NCList(int index, NC** ncp)
{
    /* Walk from 0 ...; 0 return => stop */
    if(index < 0 || index >= NCFILELISTLENGTH)
	return NC_ERANGE;
    if(ncp) *ncp = nc_global->filelist[index];
    return NC_NOERR;
}
#endif

