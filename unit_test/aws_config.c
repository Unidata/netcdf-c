/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "netcdf.h"
#include "ncrc.h"
#include "ncpathmgr.h"
#include "ncs3sdk.h"
#include "ncuri.h"
#include "nc4internal.h"
#include "ncaws.h"

NCS3INFO s3info;
void* s3client = NULL;

/* Forward */
static void cleanup(void);

#define STR(p) p?p:"NULL"
#define CHECK(code) do {stat = check(code,__func__,__LINE__); if(stat) {goto done;}} while(0)

static int
check(int code, const char* fcn, int line)
{
    if(code == NC_NOERR) return code;
    fprintf(stderr,"***FAIL: (%d) %s @ %s:%d\n",code,nc_strerror(code),fcn,line);
    abort();
}

static void
cleanup(void)
{
    if(s3client)
        NC_s3sdkclose(s3client, NULL);
    s3client = NULL;
    NC_s3clear(&s3info);
}

int
main(int argc, char** argv)
{
    int c = 0,stat = NC_NOERR;

    /* Load RC and .aws/config */
    CHECK(nc_initialize()); /* Will invoke NC_s3sdkinitialize()); */
    NCglobalstate* gs = NC_getglobalstate();
    //Not checking, aborts if ~/.aws/config doesn't exist
    CHECK(NC_aws_load_credentials(gs));
    
    // Lets ensure the active profile is loaded
    // from the configuration files instead of an URL
    const char* activeprofile = NULL;
    CHECK(NC_getactives3profile(NULL, &activeprofile));

    fprintf(stderr, "Active profile:%s\n", STR(activeprofile));
    
    // ARGV contains should contain "key[=value]" to verify
    // if key was parsed when loading the aws config and if it's
    // value is updated in case it's redefined on the .aws/credentials
    for(int i = 1; i < argc; i++) {
        const char *argkey = strtok(argv[i],"=");
        const char *argvalue = strtok(NULL,"=");
        const char* value = NULL;

        NC_s3profilelookup(activeprofile,argkey,&value);
        fprintf(stderr, "%s\t%s -> %s\n",value?"":"*** FAIL:", argv[i],value?value:"NOT DEFINED!");
        if ( value == NULL 
            || (argvalue != NULL 
                && strncmp(value, argvalue, strlen(value)))
        ){
                c++;
                stat |= NC_ERCFILE;
        }
    }

done:
    cleanup();
    if(stat)
        printf("*** FAIL: a total of %d keys were not found on the profile %s\n", c, STR(activeprofile));
    else
        printf("***PASS\n");
    (void)NC_s3sdkfinalize();
    (void)nc_finalize();
    exit(stat?:0);
}
