/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the handling of aws profiles and regions.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "netcdf.h"
#include "ncrc.h"
#include "ncpathmgr.h"

#undef DEBUG

typedef struct ProfileTest {
    const char* profile;
        const char* access_key;
        const char* secret_key;
	const char* region;
} ProfileTest;

typedef struct URLTest {
    const char* url;
        const char* newurl;
        const char* profile;
        const char* region;
        const char* bucket;
} URLTest;

static ProfileTest PROFILETESTS[] = {
{"default", "ACCESSKEYDEFAULTXXXX", "DEFAULT/ef0ghijklmnopqr/defaultxxxxxxxxx",""},
{"ncar", "ACCESSKEYNCARXXXXXXX", "NCAR/ef0ghijklmnopqr/ncarxxxxxxxxxxxxxxx",""},
{"unidata", "ACCESSKEYUNIDATAXXXX", "UNIDATA/ef0ghijklmnopqr/unidataxxxxxxxxx", "us-west-1"},
{NULL, NULL,NULL,NULL}
};

static URLTest URLTESTS[] = {
{"s3://simplebucket#mode=nczarr,s3&aws.region=us-west-1",
    "https://s3.us-west-1.amazonaws.com/simplebucket#mode=nczarr,s3&aws.region=us-west-1","default","us-west-1","simplebucket"},
#if 0
{"s3://simplebucket#mode=nczarr,s3&aws.profile=unidata",
    "https://s3.us-west-1.amazonaws.com/simplebucket#mode=nczarr,s3&aws.profile=unidata","unidata","us-west-1","simplebucket"},
{"https://s3.eu-east-1.amazonaws.com/simplebucket#mode=nczarr,s3&aws.profile=none",
    "https://s3.eu-east-1.amazonaws.com/simplebucket#mode=nczarr,s3&aws.profile=none","none","eu-east-1","simplebucket"},
{"https://s3.eu-west-1.amazonaws.com/bucket2#mode=nczarr,s3",
    "https://s3.eu-west-1.amazonaws.com/bucket2#mode=nczarr,s3","default","eu-west-1","bucket2"},
#endif
{NULL, NULL,NULL,NULL,NULL}
};

static char* awstestdir0 = NULL;

void
failurltest(URLTest* test)
{
    fprintf(stderr,"***FAIL: urL=%s\n",test->url);
#ifdef DEBUG
    abort();
#endif
    exit(1);
}

void
failprofiletest(ProfileTest* test)
{
    fprintf(stderr,"***FAIL: profile=%s\n",test->profile);
#ifdef DEBUG
    abort();
#endif
    exit(1);
}

static int
testprofiles(void)
{
    int stat = NC_NOERR;
    ProfileTest* test;
    int index;

    for(index=0,test=PROFILETESTS;test->profile;test++,index++) {
	const char* accesskey = NULL;
	const char* region = NULL;

        if((stat = NC_s3profilelookup(test->profile, "aws_access_key_id", &accesskey))) goto done;
        if((stat = NC_s3profilelookup(test->profile, "aws_region", &region))) goto done;
	if(region == NULL) region = "";
#ifdef DEBUG
        printf("profile=%s aws_access_key_id=%s region=%s\n",
		test->profile,
		(accesskey?accesskey:""),
		(region?region:""));
#endif
	if(accesskey == NULL || strcasecmp(accesskey,test->access_key)!=0) failprofiletest(test);
	if(region == NULL || strcasecmp(region,test->region)!=0) failprofiletest(test);
    }
done:
   return stat;
}

static int
testurls(void)
{
    int stat = NC_NOERR;
    URLTest* test;
    int index;
    NCURI* url = NULL;
    NCURI* url2 = NULL;
    const char* profile = NULL;
    char* region = NULL;
    char* bucket = NULL;
    char* newurl = NULL;

    for(index=0,test=URLTESTS;test->url;test++,index++) {
	ncuriparse(test->url,&url);
	if(url == NULL) {
	    fprintf(stderr,"URI parse fail: %s\n",test->url);
	    goto done;
	}
        if((stat = NC_getactives3profile(url, &profile))) {
	    fprintf(stderr,"active profile fail: %s\n",test->url);
	    goto done;
	}
	if((stat = NC_s3urlrebuild(url, &url2, &bucket, &region))) {
	    fprintf(stderr,"url rebuild failed: %s\n",test->url);
	    goto done;
	}
	newurl = ncuribuild(url2,NULL,NULL,NCURIALL);
#ifdef DEBUG
        printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
	       test->url,newurl,bucket,region,profile);
#endif
	if(strcasecmp(newurl,test->newurl)!=0) failurltest(test);
	if(strcasecmp(profile,test->profile)!=0) failurltest(test);
	if(strcasecmp(region,test->region)!=0) failurltest(test);
	if(strcasecmp(bucket,test->bucket)!=0) failurltest(test);
        ncurifree(url); url = NULL;
        ncurifree(url2); url2 = NULL;
	nullfree(newurl); newurl = NULL;
	nullfree(bucket); bucket = NULL;
	nullfree(region); region = NULL;
    }
done:
    return stat;
}


int
main(int argc, char** argv)
{
    int stat = NC_NOERR;

    awstestdir0 = getenv("NC_TEST_AWS_DIR");
    if(awstestdir0 == NULL) {
	fprintf(stderr,"NC_TEST_AWS_DIR environment variable is undefined\n");
	goto done;
    }

    /* Load RC and .aws/config */
    if((stat = nc_initialize())) goto done;

    printf("testprofiles:\n-------------\n");
    stat = testprofiles();

    printf("testurls:\n--------\n");
    stat = testurls();

    printf("***PASS test_aws\n");

done:    
    if(stat) printf("*** FAIL: %s(%d)\n",nc_strerror(stat),stat);
    exit(stat?1:0);
}
