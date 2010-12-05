#include "ncdispatch.h"
#define INITCOORD1 if(coord_one[0] != 1) {int i; for(i=0;i<NC_MAX_VAR_DIMS;i++) coord_one[i] = 1;}

/*
static nc_type longtype = (sizeof(long) == sizeof(int)?NC_INT:NC_INT64);
static nc_type ulongtype = (sizeof(unsigned long) == sizeof(unsigned int)?NC_UINT:NC_UINT64);
*/

NC_Dispatch* NC3_dispatch_table = NULL;

#ifdef USE_NETCDF4
NC_Dispatch* NC4_dispatch_table = NULL;
#endif

#ifdef USE_DAP
NC_Dispatch* NCD3_dispatch_table = NULL;
#endif

#if defined(USE_DAP) && defined(USE_NETCDF4)
NC_Dispatch* NCD4_dispatch_table = NULL;
#endif

#if defined(USE_CDMREMOTE) && defined(USE_NETCDF4)
NC_Dispatch* NCCR_dispatch_table = NULL;
#endif

int
NC_testurl(const char* path)
{
    void* tmpurl = NULL;
    if(nc_urlparse(path,&tmpurl) == NC_NOERR) {
	nc_urlfree(tmpurl);
	return 1;
    }
    return 0;
}

/* Return the OR of some of the NC_DISPATCH flags */
int
NC_urlmodel(const char* path)
{
    int model = 0;
    NC_URL* tmpurl = NULL;
    if(nc_urlparse(path,&tmpurl) == NC_NOERR) {
	if(nc_urllookup(tmpurl,"netcdf4")
	   || nc_urllookup(tmpurl,"netcdf-4")) {
	    model = (NC_DISPATCH_NC4 | NC_DISPATCH_NCD);
	} else if(nc_urllookup(tmpurl,"netcdf3")
	   || nc_urllookup(tmpurl,"netcdf-3")) {
	    model = (NC_DISPATCH_NC3 | NC_DISPATCH_NCD);
	} else if(nc_urllookup(tmpurl,"cdmremote")
	   || nc_urllookup(tmpurl,"cdmr")) {
	    model = (NC_DISPATCH_NC4 | NC_DISPATCH_NCR);
	} else {
	    model = 0;
	}
	nc_urlfree(tmpurl);
    }
    return model;
}

/* Override dispatch table management */
static NC_Dispatch* NC_dispatch_override = NULL;

/* Override dispatch table management */
NC_Dispatch*
NC_get_dispatch_override(void) {
    return NC_dispatch_override;
}

void NC_set_dispatch_override(NC_Dispatch* d)
{
    NC_dispatch_override = d;
}

/* Overlay by treating the tables as arrays of void*.
   Overlay rules are:
        overlay    base    merge
        -------    ----    -----
          null     null     null
          null      y        y
           x       null      x
           x        y        x
*/

int
NC_dispatch_overlay(const NC_Dispatch* overlay, const NC_Dispatch* base, NC_Dispatch* merge)
{
    void** voverlay = (void**)overlay;
    void** vmerge;
    int i, count = sizeof(NC_Dispatch) / sizeof(void*);
    /* dispatch table must be exact multiple of sizeof(void*) */
    assert(count * sizeof(void*) == sizeof(NC_Dispatch));
    *merge = *base;
    vmerge = (void**)merge;
    for(i=0;i<count;i++) {
        if(voverlay[i] == NULL) continue;
        vmerge[i] = voverlay[i];
    }
    /* Finally, the merge model should always be the overlay model */
    merge->model = overlay->model;
    return NC_NOERR;
}
