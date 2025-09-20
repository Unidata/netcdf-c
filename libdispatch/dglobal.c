/*
Copyright (c) 1998-2018 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "netcdf.h"
#include "ncglobal.h"
#include "nclist.h"
#include "ncuri.h"
#include "ncrc.h"
#include "ncs3sdk.h"

/**************************************************/
/* Global State constants and state */

/* The singleton global state object */
static NCglobalstate* nc_globalstate = NULL;

/* Forward */
static int NC_createglobalstate(void);

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
	if((nc_globalstate->chunkcache = calloc(1,sizeof(struct ChunkCache)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
    }

    /* Get environment variables */
    if(getenv(NCRCENVIGNORE) != NULL)
        nc_globalstate->rcinfo->ignore = 1;
    tmp = getenv(NCRCENVRC);
    if(tmp != NULL && strlen(tmp) > 0)
        nc_globalstate->rcinfo->rcfile = strdup(tmp);
    /* Initialize chunk cache defaults */
    nc_globalstate->chunkcache->size = DEFAULT_CHUNK_CACHE_SIZE;		    /**< Default chunk cache size. */
    nc_globalstate->chunkcache->nelems = DEFAULT_CHUNKS_IN_CACHE;	    /**< Default chunk cache number of elements. */
    nc_globalstate->chunkcache->preemption = DEFAULT_CHUNK_CACHE_PREEMPTION; /**< Default chunk cache preemption. */
    
done:
    return stat;
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
	nullfree(gs->chunkcache);
	NC_clearawsparams(&gs->aws);
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

/* Aws Param management */
void
NC_cloneawsparams(struct NCAWSPARAMS* clone, struct NCAWSPARAMS* aws)
{
    NC_clearawsparams(clone);
    clone->region = nulldup(aws->region);
    clone->default_region = nulldup(aws->default_region);
    clone->config_file = nulldup(aws->config_file);
    clone->profile = nulldup(aws->profile);
    clone->session_token = nulldup(aws->session_token);
    clone->access_key_id = nulldup(aws->access_key_id);
    clone->secret_access_key = nulldup(aws->secret_access_key);
}

void
NC_clearawsparams(struct NCAWSPARAMS* aws)
{
    nullfree(aws->region);
    nullfree(aws->default_region);
    nullfree(aws->config_file);
    nullfree(aws->profile);
    nullfree(aws->session_token);
    nullfree(aws->access_key_id);
    nullfree(aws->secret_access_key);
    nullfree(aws->session_token);
    memset(aws,0,sizeof(struct NCAWSPARAMS));
}

NCAWSPARAMS
NC_awsparams_empty(void)
{
    NCAWSPARAMS aws;
    memset(&aws,0,sizeof(NCAWSPARAMS));
    return aws;
}

/**************************************************/
/* Capture environmental Info */

/*
When loading the globalstate AWS key values, load in the following order:
1. .rc file without URI patterns.
2. environment variables
Note: 2 takes precedent over 1.

When loading the NCZ_FILE_INFO_T AWS key values, load in the following order:
1. existing globalstate values
2. .rc file with URI patterns.
3. environment variables 
4. URI fragment keys
Note: 4 takes precedent over 3 takes precedent over and 2 takes precedent over 1.

*/

/* Load the globalstate.aws fields */
EXTERNL void
NC_awsglobal(void)
{
    NCAWSPARAMS aws;
    NCglobalstate* gs = NC_getglobalstate();

#if 0
    aws = NC_awsparams_empty();
    NC_clearawsparams(&gs->aws);
    
    /* Get .rc information */
    NC_clearawsparams(&aws);
    NC_awsrc(&aws,NULL);
    NC_awsparamsmerge(&gs->aws,&aws);

    /* Get environment information; overrides .rc file*/
    NC_clearawsparams(&aws);
    NC_awsenvironment(&aws);
    NC_awsparamsmerge(&gs->aws,&aws);
    NC_clearawsparams(&aws);   

    /* Do some defaulting */
    if(gs->aws.default_region == NULL) gs->aws.default_region = nulldup(AWS_GLOBAL_DEFAULT_REGION);
    if(gs->aws.region == NULL) gs->aws.region = nulldup(gs->aws.default_region);
#endif
}

#if 0
/*
When loading the NCZ_FILE_INFO_T AWS key values, load in the following order:
1. existing globalstate values
2. .rc file with URI patterns.
3. environment variables 
4. URI fragment keys
Note: 4 takes precedent over 3 and 3 takes precedent over 2 and 2 takes precedent over 1.
*/

/* Load the NCZ_FILE_INFO_T aws fields*/
EXTERNL void
NC_awsnczfile(NCAWSPARAMS* zfileaws, NCURI* uri)
{
    struct NCAWSPARAMS aws;
    NCglobalstate* gs = NC_getglobalstate();
    
    aws = NC_awsparams_empty();
    NC_clearawsparams(zfileaws);

    /* Initialize the aws from gs->aws */
    NC_awsparamsmerge(zfileaws,&gs->aws);

    /* Get .rc information with uri from the open/create path */
    NC_clearawsparams(&aws);
    NC_awsrc(&aws,uri);
    NC_awsparamsmerge(zfileaws,&aws);

    /* Get environment information; overrides .rc file*/
    NC_clearawsparams(&aws);
    NC_awsenvironment(&aws);
    NC_awsparamsmerge(zfileaws,&aws);

    /* Get URI fragment information */
    NC_clearawsparams(&aws);
    NC_awsfrag(&aws,uri);
    NC_awsparamsmerge(zfileaws,&aws);
    NC_clearawsparams(&aws);

    /* Do some defaulting */
    if(zfileaws->default_region == NULL) zfileaws->default_region = nulldup(AWS_GLOBAL_DEFAULT_REGION);
    if(zfileaws->region == NULL) zfileaws->region = nulldup(zfileaws->default_region);
}

EXTERNL void
NC_awsparamsmerge(struct NCAWSPARAMS* baseaws, struct NCAWSPARAMS* newaws)
{
    if(newaws->region != NULL) {nullfree(baseaws->region); baseaws->region = nulldup(newaws->region);}
    if(newaws->default_region != NULL) {nullfree(baseaws->default_region); baseaws->default_region = nulldup(newaws->default_region);}
    if(newaws->access_key_id != NULL) {nullfree(baseaws->access_key_id); baseaws->access_key_id = nulldup(newaws->access_key_id);}
    if(newaws->secret_access_key != NULL) {nullfree(baseaws->secret_access_key); baseaws->secret_access_key = nulldup(newaws->secret_access_key);}
    if(newaws->session_token != NULL) {nullfree(baseaws->session_token); baseaws->session_token = nulldup(newaws->session_token);}
    if(newaws->profile != NULL) {nullfree(baseaws->profile); baseaws->profile = nulldup(newaws->profile);}
    if(newaws->config_file != NULL) {nullfree(baseaws->config_file); baseaws->config_file = nulldup(newaws->config_file);}
}

/**************************************************/
/* Lookup functors */

/* Collect aws params from env variables */
EXTERNL void
NC_awsenvironment(struct NCAWSPARAMS* aws)
{
    NC_clearawsparams(aws);
    aws->region = nulldup(getenv("AWS_REGION"));
    aws->default_region = nulldup(getenv("AWS_DEFAULT_REGION"));
    aws->access_key_id = nulldup(getenv("AWS_ACCESS_KEY_ID"));
    aws->secret_access_key = nulldup(getenv("AWS_SECRET_ACCESS_KEY"));
    aws->session_token = nulldup(getenv("AWS_SESSION_TOKEN"));
    aws->profile = nulldup(getenv("AWS_PROFILE"));
    aws->config_file = nulldup(getenv("AWS_CONFIG_FILE"));
}

/* Setup aws params from .rc file */
EXTERNL void
NC_awsrc(struct NCAWSPARAMS* aws, NCURI* uri)
{
    NC_clearawsparams(aws);
    aws->region = nulldup(NC_rclookupx(uri,"AWS.REGION"));
    aws->default_region = nulldup(NC_rclookupx(uri,"AWS.DEFAULT_REGION"));
    aws->access_key_id = nulldup(NC_rclookupx(uri,"AWS.ACCESS_KEY_ID"));
    aws->secret_access_key = nulldup(NC_rclookupx(uri,"AWS.SECRET_ACCESS_KEY"));
    aws->session_token = nulldup(NC_rclookupx(uri,"AWS.SESSION_TOKEN"));
    aws->profile = nulldup(NC_rclookupx(uri,"AWS.PROFILE"));
    aws->config_file = nulldup(NC_rclookupx(uri,"AWS.CONFIG_FILE"));
}

/* Setup aws params from URI fragment */
EXTERNL void
NC_awsfrag(struct NCAWSPARAMS* aws, NCURI* uri)
{
    NC_clearawsparams(aws);
    aws->region = nulldup(ncurifragmentlookup(uri,"aws.region"));
    aws->default_region = nulldup(ncurifragmentlookup(uri,"aws.default_region"));
    aws->access_key_id = nulldup(ncurifragmentlookup(uri,"aws.access_key_id"));
    aws->secret_access_key = nulldup(ncurifragmentlookup(uri,"aws.secret_access_key"));
    aws->session_token = nulldup(ncurifragmentlookup(uri,"aws.session_token"));
    aws->profile = nulldup(ncurifragmentlookup(uri,"aws.profile"));
    aws->config_file = nulldup(ncurifragmentlookup(uri,"aws.config_file"));
}
#endif /*0*/
