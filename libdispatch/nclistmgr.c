/*********************************************************************
   Copyright 2010, UCAR/Unidata See netcdf/COPYRIGHT file for
   copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nc.h"
#include "ncglobal.h"

#define ID_SHIFT (16)

int
add_to_NCList(NC* ncp)
{
    int i;
    int new_id;
    int stat = NC_NOERR;

#ifdef USE_REFCOUNT
    /* Check the refcount */
    if(ncp->refcount > 0)
	{stat = NC_NOERR; goto done;}
#endif

    new_id = 0; /* id's begin at 1 */
    NCLOCK();
    for(i=1; i < nc_global->files.boundary; i++) {
	if(nc_global->files.filelist[i] == NULL) {new_id = i; break;}
    }
    if(new_id == 0) {
	if(nc_global->files.boundary == nc_global->files.maxfiles)
	    {stat = NC_ENOMEM; goto done;} /* no more slots */
	new_id = nc_global->files.boundary++;
    }
    nc_global->files.filelist[new_id] = ncp;
    new_id = (new_id << ID_SHIFT);
    ncp->ext_ncid = new_id;
done:
    NCUNLOCK();
    return stat;
}

void
del_from_NCList(NC* ncp)
{
   int nfiles;
   NC* tmp;
   unsigned int ncid = ((unsigned int)ncp->ext_ncid) >> ID_SHIFT;
   NCLOCK();
   if(nc_global->files.boundary <= 1 || ncid == 0) goto done;
   tmp = nc_global->files.filelist[ncid];
   if(tmp != ncp) goto done;
#ifdef USE_REFCOUNT
   /* Check the refcount */
   if(ncp->refcount > 0)
	goto done; /* assume caller has decrecmented */
#endif
   nc_global->files.filelist[ncid] = NULL;
   if(ncid == (nc_global->files.boundary - 1))
	nc_global->files.boundary--;
done:
    NCUNLOCK();
    return;
}

NC *
find_in_NCList(int ext_ncid)
{
   NC* f = NULL;
   unsigned int ncid = ((unsigned int)ext_ncid) >> ID_SHIFT;
   NCLOCK();
   if(numfiles > 0 && nc_filelist != NULL && ncid < NCFILELISTLENGTH)
	f = nc_filelist[ncid];
   NCUNLOCK();

   /* for classic files, ext_ncid must be a multiple of (1<<ID_SHIFT) */
   if (f != NULL && f->model == NC_FORMATX_NC3 && (ext_ncid % (1<<ID_SHIFT)))
       return NULL;
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
   NCLOCK();
   for(i=1; i < nc_global->files.boundary; i++) {
	if(nc_global->files.filelist[i] != NULL) {
	    if(strcmp(nc_global->files.filelist[i]->path,path)==0) {
		f = nc_global->files.filelist[i];
		break;
	    }				
	}
   }

   NCUNLOCK();
   return f;
}
