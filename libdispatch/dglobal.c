/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "netcdf.h"
#include "ncglobal.h"
#include "ncaws.h"
#include "ncpathmgr.h"
#include "nclist.h"
#include "ncuri.h"
#include "ncrc.h"
#include "nclog.h"
#include "ncs3sdk.h"

/**************************************************/

#define MAXPATH 1024

/**************************************************/
/* Global State constants and state */

/* The singleton global state object */
static NCglobalstate* nc_globalstate = NULL;

/* Forward */
static int NC_createglobalstate(void);
static void gs_chunkcache_init(NCglobalstate* gs);
static void gs_paths_init(NCglobalstate* gs);

/** \defgroup global_state Global state functions. */
/** \{

\ingroup global_state
*/

/* NCglobal state management */

static int
NC_createglobalstate(void)
{
    int stat = NC_NOERR;
    const char* tmp = NULL;
    
    if(nc_globalstate == NULL) {
        nc_globalstate = calloc(1,sizeof(NCglobalstate));
	if(nc_globalstate == NULL) {stat = NC_ENOMEM; goto done;}
	/* Initialize struct pointers */
	if((nc_globalstate->rcinfo = calloc(1,sizeof(struct NCRCinfo)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	if((nc_globalstate->rcinfo->entries = nclistnew())==NULL)
	    {stat = NC_ENOMEM; goto done;}
	if((nc_globalstate->rcinfo->s3profiles = nclistnew())==NULL)
	    {stat = NC_ENOMEM; goto done;}
	memset(&nc_globalstate->chunkcache,0,sizeof(struct ChunkCache));
    }

    /* Initialize chunk cache defaults */
    gs_chunkcache_init(nc_globalstate);
    
    /* Initialize various paths */
    gs_paths_init(nc_globalstate);
    
    /* Get .rc state */
    if(getenv(NCRCENVIGNORE) != NULL)
        nc_globalstate->rcinfo->ignore = 1;
    tmp = getenv(NCRCENVRC);
    if(tmp != NULL && strlen(tmp) > 0)
        nc_globalstate->rcinfo->rcfile = strdup(tmp);

    ncrc_initialize();

    /* Initialize aws defaults from .rc, env vars, aws profiles */
    NC_awsglobal();

    /* load AWS Profiles: .aws/config &/ credentials */
    if(NC_aws_load_credentials(nc_globalstate)) {
        nclog(NCLOGWARN,"AWS config file not loaded");
    }

#if 0
    /* Initialize chunk cache defaults */
    nc_globalstate->chunkcache.size = DEFAULT_CHUNK_CACHE_SIZE;		    /**< Default chunk cache size. */
    nc_globalstate->chunkcache.nelems = DEFAULT_CHUNKS_IN_CACHE;	    /**< Default chunk cache number of elements. */
    nc_globalstate->chunkcache.preemption = DEFAULT_CHUNK_CACHE_PREEMPTION; /**< Default chunk cache preemption. */
#endif

done:
    return stat;
}

/* Initialize chunk cache defaults */
static void
gs_chunkcache_init(NCglobalstate* gs)
{    
    gs->chunkcache.size = DEFAULT_CHUNK_CACHE_SIZE;		/**< Default chunk cache size. */
    gs->chunkcache.nelems = DEFAULT_CHUNKS_IN_CACHE;		/**< Default chunk cache number of elements. */
    gs->chunkcache.preemption = DEFAULT_CHUNK_CACHE_PREEMPTION;/**< Default chunk cache preemption. */
}

static void
gs_paths_init(NCglobalstate* gs)
{
    /* Capture temp dir*/
    {
	char* tempdir = NULL;
#if defined _WIN32 || defined __MSYS__ || defined __CYGWIN__
        tempdir = getenv("TEMP");
#else
	tempdir = "/tmp";
#endif
        if(tempdir == NULL) {
	    fprintf(stderr,"Cannot find a temp dir; using ./\n");
	    tempdir = ".";
	}
	gs->tempdir= strdup(tempdir);
    }

    /* Capture $HOME */
    {
#if defined(_WIN32) && !defined(__MINGW32__)
        char* home = getenv("USERPROFILE");
#else
        char* home = getenv("HOME");
#endif
        if(home == NULL) {
	    /* use cwd */
	    home = malloc(MAXPATH+1);
	    NCgetcwd(home,MAXPATH);
        } else
	    home = strdup(home); /* make it always free'able */
	assert(home != NULL);
        NCpathcanonical(home,&gs->home);
	nullfree(home);
    }
 
    /* Capture $CWD */
    {
        char cwdbuf[4096];

        cwdbuf[0] = '\0';
	(void)NCgetcwd(cwdbuf,sizeof(cwdbuf));

        if(strlen(cwdbuf) == 0) {
	    /* use tempdir */
	    strcpy(cwdbuf, gs->tempdir);
	}
        gs->cwd = strdup(cwdbuf);
    }

}

/* Get global state */
NCglobalstate*
NC_getglobalstate(void)
{
    if(nc_globalstate == NULL)
        NC_createglobalstate();
    return nc_globalstate;
}

void
NC_freeglobalstate(void)
{
    NCglobalstate* gs = nc_globalstate;
    if(gs != NULL) {
        nullfree(gs->tempdir);
        nullfree(gs->home);
        nullfree(gs->cwd);
	memset(&gs->chunkcache,0,sizeof(struct ChunkCache));
	NC_clearawsparams(gs->aws);
	nullfree(gs->aws);
        if(gs->rcinfo) {
	    NC_rcclear(gs->rcinfo);
	    free(gs->rcinfo);
	}
	nclistfree(gs->pluginpaths);
	free(gs);
	nc_globalstate = NULL;
    }
}

/** \} */
