/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "csincludes.h"

int
NCZ_open(const char *path, int mode,
	    int basepe, size_t *chunksizehintp,
	    int use_parallel, void* parameters,
	    struct NC_Dispatch* table, NC* ncp)
{
    int stat = NC_NOERR;
    NCZINFO* zinfo = NULL;
    CURL* curl = NULL;

    TRACE();

    if(path == NULL)
	return THROW(NC_EDAPURL);

    assert(dispatch != NULL);

    /* Setup our per-dataset state */

    zinfo = (NCZINFO*)calloc(1,sizeof(NCZINFO));
    if(zinfo == NULL) {stat = NC_ENOMEM; goto done;}

    nc->dispatchdata = zinfo;
    nc->int_ncid = nc__pseudofd(); /* create a unique id */
    zinfo->controller = (NC*)nc;

    /* Parse url and params */
    if(ncuriparse(nc->path,&zinfo->uri) != NCU_OK)
	{stat = NC_EDAPURL; goto done;}

    /* Load auth info from rc file */
    if((stat = NC_authsetup(&zinfo->auth, zinfo->uri))) goto done;
    NCZ_curl_protocols(zinfo);

    /* initialize curl handle*/
    curl = curl_easy_init();
    if (curl == NULL) {stat = NC_ECURL; goto done;}
    
    zinfo->curl->curl = curl;

    /* Determine if we are reading standard Zarr or netcdf-4 zarr */
    if((stat=computeformat(zinfo))) goto done;

done:
    return stat;
}

int
NCZ_create(const char *path, int cmode,
	  size_t initialsz, int basepe, size_t *chunksizehintp,
	  int use_parallel, void* parameters,
	  struct NC_Dispatch* table, NC* ncp)
{return NC_EPERM;}

/**************************************************/
static int
computeformat(NCZINFO* zinfo)
{
    int stat = NC_NOERR;

    /* Create the REST url for accessing the 
done:
    return stat;
}
