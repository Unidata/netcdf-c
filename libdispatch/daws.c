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
#include "ncuri.h"
#include "ncrc.h"

/**************************************************/
/* Local Macros */

#ifndef REPLACE
#define REPLACE(dst,src) do{nullfree(dst); dst = nulldup(src);}while(0)
#endif

/**************************************************/
/* Forward */

static void NC_awsparamsmerge(struct NCAWSPARAMS* baseaws, struct NCAWSPARAMS* newaws);

/**************************************************/
/* Aws Param management */

void
NC_clearawsparams(struct NCAWSPARAMS* aws)
{
    assert(aws != NULL);
    nullfree(aws->config_file);
    nullfree(aws->profile);
    nullfree(aws->region);
    nullfree(aws->default_region);
    nullfree(aws->access_key_id);
    nullfree(aws->secret_access_key);
    nullfree(aws->session_token);
    memset(aws,0,sizeof(struct NCAWSPARAMS));
}

#if 0
void
NC_cloneawsparams(struct NCAWSPARAMS* clone, struct NCAWSPARAMS* aws)
{
    NC_clearawsparams(clone);
    clone->config_file = nulldup(aws->config_file);
    clone->profile = nulldup(aws->profile);
    clone->region = nulldup(aws->region);
    clone->default_region = nulldup(aws->default_region);
    clone->access_key_id = nulldup(aws->access_key_id);
    clone->secret_access_key = nulldup(aws->secret_access_key);
}
#endif

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
Note: precedence order: 2 over 1.
*/

/* Load the globalstate.aws fields */
void
NC_awsglobal(void)
{
    NCglobalstate* gs = NC_getglobalstate();
    NCAWSPARAMS aws;

    if(gs->aws == NULL)
	gs->aws = (NCAWSPARAMS*)calloc(1,sizeof(NCAWSPARAMS));
    NC_clearawsparams(gs->aws);

    aws = NC_awsparams_empty();
    NC_clearawsparams(&aws);
    
    /* Get .rc information */
    NC_clearawsparams(&aws);
    NC_awsrc(&aws,NULL);
    NC_awsparamsmerge(gs->aws,&aws);

    /* Get environment information; overrides .rc file*/
    NC_clearawsparams(&aws);
    NC_awsenvironment(&aws);
    NC_awsparamsmerge(gs->aws,&aws);
    NC_clearawsparams(&aws);   

    /* Do some defaulting */
    if(gs->aws->default_region == NULL) gs->aws->default_region = nulldup(AWS_GLOBAL_DEFAULT_REGION);
    if(gs->aws->region == NULL) gs->aws->region = nulldup(gs->aws->default_region);
}

/*
When loading the NCZ_FILE_INFO_T AWS key values, load in the following order:
1. existing globalstate values
2. .rc file with URI patterns.
3. environment variables 
4. URI fragment keys
Note: precedence order: 4 over 3 over 2 over 1.
*/

/* Load the NCZ_FILE_INFO_T aws fields*/
void
NC_awsnczfile(NCAWSPARAMS* zfileaws, NCURI* uri)
{
    struct NCAWSPARAMS aws;
    NCglobalstate* gs = NC_getglobalstate();
    
    aws = NC_awsparams_empty();
    NC_clearawsparams(zfileaws);

    /* Initialize the aws from gs->aws */
    NC_awsparamsmerge(zfileaws,gs->aws);

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

static void
NC_awsparamsmerge(struct NCAWSPARAMS* baseaws, struct NCAWSPARAMS* newaws)
{
    assert(baseaws != NULL && newaws != NULL);
    if(newaws->config_file != NULL)       REPLACE(baseaws->config_file,newaws->config_file);
    if(newaws->profile != NULL)           REPLACE(baseaws->profile,newaws->profile);
    if(newaws->region != NULL)            REPLACE(baseaws->region,newaws->region);
    if(newaws->default_region != NULL)    REPLACE(baseaws->default_region,newaws->default_region);
    if(newaws->access_key_id != NULL)     REPLACE(baseaws->access_key_id,newaws->access_key_id);
    if(newaws->secret_access_key != NULL) REPLACE(baseaws->secret_access_key,newaws->secret_access_key);
    if(newaws->session_token != NULL) REPLACE(baseaws->session_token,newaws->session_token);
}

/**************************************************/
/* Lookup functors */

/* Collect aws params from env variables */
void
NC_awsenvironment(struct NCAWSPARAMS* aws)
{
    NC_clearawsparams(aws);
    aws->profile = nulldup(getenv(AWS_ENV_PROFILE));
    aws->config_file = nulldup(getenv(AWS_ENV_CONFIG_FILE));
    aws->region = nulldup(getenv(AWS_ENV_REGION));
    aws->default_region = nulldup(getenv(AWS_ENV_DEFAULT_REGION));
    aws->access_key_id = nulldup(getenv(AWS_ENV_ACCESS_KEY_ID));
    aws->secret_access_key = nulldup(getenv(AWS_ENV_SECRET_ACCESS_KEY));
    aws->session_token = nulldup(getenv(AWS_ENV_SESSION_TOKEN));
}

/* Setup aws params from .rc file */
void
NC_awsrc(struct NCAWSPARAMS* aws, NCURI* uri)
{
    NC_clearawsparams(aws);
    aws->profile = nulldup(NC_rclookupx(uri,AWS_RC_PROFILE));
    aws->config_file = nulldup(NC_rclookupx(uri,AWS_RC_CONFIG_FILE));
    aws->region = nulldup(NC_rclookupx(uri,AWS_RC_REGION));
    aws->default_region = nulldup(NC_rclookupx(uri,AWS_RC_DEFAULT_REGION));
    aws->access_key_id = nulldup(NC_rclookupx(uri,AWS_RC_ACCESS_KEY_ID));
    aws->secret_access_key = nulldup(NC_rclookupx(uri,AWS_RC_SECRET_ACCESS_KEY));
    aws->session_token = nulldup(NC_rclookupx(uri,AWS_RC_SESSION_TOKEN));
}

/* Setup aws params from URI fragment */
void
NC_awsfrag(struct NCAWSPARAMS* aws, NCURI* uri)
{
    NC_clearawsparams(aws);
    aws->profile = nulldup(ncurifragmentlookup(uri,AWS_FRAG_PROFILE));
    aws->config_file = nulldup(ncurifragmentlookup(uri,AWS_FRAG_CONFIG_FILE));
    aws->region = nulldup(ncurifragmentlookup(uri,AWS_FRAG_REGION));
    aws->default_region = nulldup(ncurifragmentlookup(uri,AWS_FRAG_DEFAULT_REGION));
    aws->access_key_id = nulldup(ncurifragmentlookup(uri,AWS_FRAG_ACCESS_KEY_ID));
    aws->secret_access_key = nulldup(ncurifragmentlookup(uri,AWS_FRAG_SECRET_ACCESS_KEY));
    aws->session_token = nulldup(ncurifragmentlookup(uri,AWS_FRAG_SESSION_TOKEN));
}
