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

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#undef DEBUG
//#define DEBUG 1
#define SELF_CLEAN

/* Mnemonic(s) */
#define FORCE 1

#define LONGCOUNT 1010

enum Actions {ERROR_ACTION, EXISTS_ACTION, SIZE_ACTION, READ_ACTION, WRITE_ACTION, DELETE_ACTION, LIST_ACTION, LONGLIST_ACTION, LISTALL_ACTION};

struct Options {
    int debug;
    int test;
    enum Actions action;
    const char* url;
    const char* key;
    const char* prefix;
} dumpoptions;

/* Upload data */
static const char* uploaddata = "line1\nline2\nline3";

//static const char* testurl = "https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}";

/* Global values */
NCURI* purl = NULL;
const char* activeprofile = NULL;
const char* accessid = NULL;
const char* accesskey = NULL;
const char* session_token = NULL;
const char* newurl = NULL;
NCS3INFO s3info;
void* s3client = NULL;

char* testkeypath = NULL;

/* Forward */
static void cleanup(void);

#define CHECK(code) do {stat = check(code,__func__,__LINE__); if(stat) {goto done;}} while(0)

static int
check(int code, const char* fcn, int line)
{
    if(code == NC_NOERR) return code;
    fprintf(stderr,"***FAIL: (%d) %s @ %s:%d\n",code,nc_strerror(code),fcn,line);
    abort();
}

static const char*
buildkey(const char* suffix)
{
    static char truekey[4096];
    if(suffix == NULL) suffix = "";
    if(strlen(suffix) > 0) assert(suffix[0] == '/');
    snprintf(truekey,sizeof(truekey),"%s%s",dumpoptions.prefix,suffix);
    return truekey;
}

static enum Actions
actionfor(const char* s)
{
    if(strcasecmp(s,"exists")==0) return EXISTS_ACTION;
    else if(strcasecmp(s,"size")==0) return SIZE_ACTION;
    else if(strcasecmp(s,"read")==0) return READ_ACTION;
    else if(strcasecmp(s,"write")==0) return WRITE_ACTION;
    else if(strcasecmp(s,"list")==0) return LIST_ACTION;
    else if(strcasecmp(s,"longlist")==0) return LONGLIST_ACTION;
    else if(strcasecmp(s,"listall")==0) return LISTALL_ACTION;
    else if(strcasecmp(s,"delete")==0) return DELETE_ACTION;
    return ERROR_ACTION;
}

static void
seturl(const char* url, const char* key, int force)
{
if(0) {
    if(force || dumpoptions.url == NULL)
        dumpoptions.url = url;
    if(force || dumpoptions.key == NULL)
        dumpoptions.key = key;
    printf("url=|%s| key=|%s|\n",
        dumpoptions.url?dumpoptions.url:"",
        dumpoptions.key?dumpoptions.key:"");
}
}

static int
profilesetup(const char* url)
{
    int stat = NC_NOERR;

    ncuriparse(url,&purl);
    if(purl == NULL) {
        fprintf(stderr,"URI parse fail: %s\n",url);
        goto done;
    }
    CHECK(NC_s3urlprocess(purl, &s3info, NULL));

    CHECK(NC_getactives3profile(purl, &activeprofile));
    CHECK(NC_s3profilelookup(activeprofile, "aws_access_key_id", &accessid));
    CHECK(NC_s3profilelookup(activeprofile, "aws_secret_access_key", &accesskey));
    CHECK(NC_s3profilelookup(activeprofile, "aws_session_token", &session_token));
    if(s3info.profile) free(s3info.profile);
    s3info.profile = (char*)nulldup(activeprofile);
    if(s3info.region == NULL) s3info.region = "";
    if(s3info.bucket == NULL) {stat = NC_ES3; goto done;}

#ifdef DEBUG
    printf("%s\n",NC_s3dumps3info(&s3info));
    printf("\taws_access_key_id=%s\n",(accesskey?accesskey:""));
#endif

done:
    return stat;
}

#if 0
static int
s3treesetup(void)
{
    int stat = NC_NOERR;
    size64_t size = 0;
    void* content = NULL;
    char key[8192];

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", "/netcdf-c/test_s3",!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}

    snprintf(key,sizeof(key),"%s/%s",dumpoptions.key,"x");
    CHECK(NC_s3sdkwriteobject(s3client, s3info.bucket, key, 0, NULL, NULL));
    snprintf(key,sizeof(key),"%s/%s",dumpoptions.key,"x/y");
    CHECK(NC_s3sdkwriteobject(s3client, s3info.bucket, key, 0, NULL, NULL));
    snprintf(key,sizeof(key),"%s/%s",dumpoptions.key,"x/y/z");
    CHECK(NC_s3sdkwriteobject(s3client, s3info.bucket, key, 0, NULL, NULL));
    snprintf(key,sizeof(key),"%s/%s",dumpoptions.key,"x/y/w");
    CHECK(NC_s3sdkwriteobject(s3client, s3info.bucket, key, 0, NULL, NULL));
done:
    cleanup();
    return stat;
}
#endif

static int
testbucketexists(void)
{
    int stat = NC_NOERR;
    int exists = 0;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}",NULL,!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    CHECK(NC_s3sdkbucketexists(s3client, s3info.bucket, &exists, NULL));
    printf("testbucketexists: exists=%d\n",exists);
    if(!exists) {stat = NC_EINVAL; goto done;}

done:
    cleanup();
    return stat;
}

static int
testinfo(void)
{
    int stat = NC_NOERR;
    unsigned long long size = 0;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}",buildkey("/dir1/nested1/file1.txt"),!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s => {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    CHECK(NC_s3sdkinfo(s3client, s3info.bucket, buildkey(dumpoptions.key), &size, NULL));
    printf("testinfo: size=%llu\n",size);

done:
    cleanup();
    return stat;
}

static int
testread(void)
{
    int stat = NC_NOERR;
    unsigned long long size = 0;
    void* content = NULL;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", "/netcdf-c/test_s3.txt",!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    CHECK(NC_s3sdkinfo(s3client, s3info.bucket, buildkey(dumpoptions.key), &size, NULL));
    printf("testread: size=%llu\n",size);
    content = calloc(1,size+1);
    CHECK(NC_s3sdkread(s3client, s3info.bucket, buildkey(dumpoptions.key), 0, size, content, NULL));
    ((char*)content)[size] = '\0';
    printf("testread: content=|%s|\n",(char*)content);
    free(content);

done:
    cleanup();
    return stat;
}

static int
testwrite(void)
{
    int stat = NC_NOERR;
    size64_t size = 0;
    void* content = NULL;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", "/netcdf-c/test_s3.txt",!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    CHECK(NC_s3sdkwriteobject(s3client, s3info.bucket, buildkey(dumpoptions.key), strlen(uploaddata), uploaddata, NULL));

    /* Verify existence and size */
    CHECK(NC_s3sdkinfo(s3client, s3info.bucket, buildkey(dumpoptions.key), &size, NULL));
    printf("testwrite: size=%llu\n",size);

    content = calloc(1,size+1); /* allow for trailing nul */
    CHECK(NC_s3sdkread(s3client, s3info.bucket, buildkey(dumpoptions.key), 0, size, content, NULL));
    ((char*)content)[size] = '\0';
    printf("testwrite: content=|%s|\n",(const char*)content);
    free(content);

done:
    cleanup();
    return stat;
}

static int
testlist(void)
{
    int stat = NC_NOERR;
    size_t i,nkeys = 0;
    char** keys = NULL;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", buildkey("/dir1"),!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s => info=%s\n",dumpoptions.url,NC_s3dumps3info(&s3info));
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    CHECK(NC_s3sdklist(s3client, s3info.bucket, buildkey(dumpoptions.key), &nkeys, &keys, NULL));
    printf("testlist: nkeys=%u; keys:\n",(unsigned)nkeys);
    for(i=0;i<nkeys;i++) {
        printf("\t|%s|\n",keys[i]);
    }
    printf("\n");

done:
    for(i=0;i<nkeys;i++) nullfree(keys[i]);
    nullfree(keys); keys = NULL;
    cleanup();
    return stat;
}

static int
testlistlong(void)
{
    int stat = NC_NOERR;
    size_t i,nkeys = 0;
    char** keys = NULL;
    char path[4096];
    unsigned char checklist[LONGCOUNT];

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", buildkey("/dir1"),!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s => info=%s\n",dumpoptions.url,NC_s3dumps3info(&s3info));
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    for(i=0;i<LONGCOUNT;i++) { /* create many keys */
        newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
        printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
        snprintf(path,sizeof(path),"%s/getkey_%d",buildkey(dumpoptions.key),(int)i);
        CHECK(NC_s3sdkwriteobject(s3client, s3info.bucket, path, strlen(uploaddata), uploaddata, NULL));
    }
    CHECK(NC_s3sdklist(s3client, s3info.bucket, buildkey(dumpoptions.key), &nkeys, &keys, NULL));
    printf("testlistlong: nkeys=%u; keys:\n",(unsigned)nkeys);
    if(nkeys != LONGCOUNT) {
        fprintf(stderr,"*** nkeys mismatch: create=%d found=%d\n",LONGCOUNT,(int)nkeys);
    }
    /* Verify that all the created keys are present */
    memset(checklist,0,sizeof(checklist));
    for(i=0;i<nkeys;i++) {
	long index = -1;
	char* suffix = strrchr(keys[i],'/');
#define GETKEY "/getkey_"
#define GETKEYLEN strlen(GETKEY)
	if(suffix != NULL && strlen(suffix) > GETKEYLEN && memcmp(suffix,GETKEY,GETKEYLEN)==0) {
	    sscanf(suffix,GETKEY"%ld",&index);
	    if(index >= 0) checklist[index] = 1;
	}
    }
    printf("\n");
    for(i=0;i<GETKEYLEN;i++) {
        if(checklist[i] == 0)
	    fprintf(stderr,"checklist[%d]=0\n",(int)i);
    }    

#ifdef SELF_CLEAN
    /* Clean up s3 store */
    stat = NC_NOERR;
    for(i=0;i<nkeys;i++) {
        printf("\tkey=%s: ",keys[i]);
        stat = NC_s3sdkdeletekey(s3client, s3info.bucket, keys[i], NULL);
        switch (stat) {
        case NC_NOERR:     printf("deleted\n");        break;
        case NC_ENOOBJECT: printf("does not exist\n"); break;
        case NC_EEMPTY:    printf("has no content\n"); break;
        default:           printf("failed\n");         break;
        }
        stat = NC_NOERR; /* reset */
    }    
#endif

done:
    cleanup();
    for(i=0;i<nkeys;i++) nullfree(keys[i]);
    nullfree(keys);
    return stat;
}

static int
testlistall(void)
{
    int stat = NC_NOERR;
    size_t i,nkeys = 0;
    char** keys = NULL;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", buildkey(NULL),!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s => info=%s\n",dumpoptions.url,NC_s3dumps3info(&s3info));
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    CHECK(NC_s3sdklistall(s3client, s3info.bucket, buildkey(dumpoptions.key), &nkeys, &keys, NULL));
    printf("testlistall: nkeys=%u; keys:\n",(unsigned)nkeys);
    for(i=0;i<nkeys;i++) {
        printf("\t|%s|\n",keys[i]);
    }
    printf("\n");

done:
    cleanup();
    for(i=0;i<nkeys;i++) nullfree(keys[i]);
    nullfree(keys);
    return stat;
}

static int
testdeletekey(void)
{
    int stat = NC_NOERR;
    size64_t size = 0;

    seturl("https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}", "/netcdf-c/test_s3.txt",!FORCE);

    CHECK(profilesetup(dumpoptions.url));
    newurl = ncuribuild(purl,NULL,NULL,NCURIALL);
#ifdef DEBUG
    printf("url=%s {url=%s bucket=%s region=%s profile=%s}\n",
               dumpoptions.url,newurl,s3info.bucket,s3info.region,activeprofile);
#endif
    if((s3client = NC_s3sdkcreateclient(&s3info))==NULL) {CHECK(NC_ES3);}
    stat = NC_s3sdkdeletekey(s3client, s3info.bucket, buildkey(dumpoptions.key), NULL);

    printf("testdeletekey: url %s%s: ",newurl,buildkey(dumpoptions.key));
    switch (stat) {
    case NC_NOERR:     printf("deleted\n");        break;
    case NC_ENOOBJECT: printf("does not exist\n"); break;
    case NC_EEMPTY:    printf("has no content\n"); break;
    default:           printf("failed\n");         break;
    }
    stat = NC_NOERR; /* reset */
    
    /* Verify deleted and size */
    stat = NC_s3sdkinfo(s3client, s3info.bucket, buildkey(dumpoptions.key), &size, NULL);
    printf("testdeletekey: url %s%s: ",newurl,buildkey(dumpoptions.key));
    switch (stat) {
    case NC_NOERR:     printf("not deleted; size=%d\n",(int)size); break;
    case NC_ENOOBJECT: printf("deleted\n"); break;
    case NC_EEMPTY:    printf("has no content\n"); break;
    default:           printf("failed\n"); goto done;
    }
    stat = NC_NOERR; /* reset */

done:
    cleanup();
    return stat;
}

static void
cleanup(void)
{
    if(s3client)
        NC_s3sdkclose(s3client, NULL);
    s3client = NULL;
    NC_s3clear(&s3info);
    ncurifree(purl); purl = NULL;
}

int
main(int argc, char** argv)
{
    int c,stat = NC_NOERR;

    /* Load RC and .aws/config */
    CHECK(nc_initialize());
    CHECK(NC_s3sdkinitialize());

    /* Init options */
    memset((void*)&dumpoptions,0,sizeof(dumpoptions));

    while ((c = getopt(argc, argv, "dhk:tu:P:")) != EOF) {
        switch(c) {
        case 'd':
            dumpoptions.debug = 1;
            break;
        case 'h':
            fprintf(stderr,"usage: test_s3 [-d][-h][-u <url>][-k <key>] <action>\n");
            goto done;
        case 'k':
            dumpoptions.key = strdup(optarg);
            break;
        case 't':
            dumpoptions.test = 1;
            break;
        case 'u':
            dumpoptions.url = strdup(optarg);
            break;
        case 'P':
	    if(strlen(optarg) == 0) {
		fprintf(stderr,"Empty prefix option\n");
		stat = NC_EINVAL;
		goto done;
	    }
            dumpoptions.prefix = strdup(optarg);
            break;
        case '?':
           fprintf(stderr,"unknown option\n");
           stat = NC_EINVAL;
           goto done;
        }
    }

    if(dumpoptions.url== NULL) {
        fprintf(stderr,"no -u argument\n");
        stat = NC_EINVAL;
        goto done;
    }        

    if(dumpoptions.test) {
        /* Mimic run_s3sdk.sh test */
        printf("Test: testwrite\n");
        if((stat = testwrite())) goto done;
        printf("Test: testread\n");
        if((stat = testread())) goto done;
        printf("Test: testinfo\n");
        if((stat = testinfo())) goto done;
        printf("Test: testlist\n");
        if((stat = testlist())) goto done;
        printf("Test: testlistlong\n");
        if((stat = testlistlong())) goto done;
        printf("Test: testlistall\n");
        if((stat = testlistall())) goto done;
        printf("Test: testdeletekey\n");
        if((stat = testdeletekey())) goto done;
    } else {
        /* get action argument */
        argc -= optind;
        argv += optind;
        if (argc > 1) {
            fprintf(stderr, "test_s3: only one action argument permitted\n");
            stat = NC_EINVAL; goto done;
        }
        if (argc == 0) {
            fprintf(stderr, "test_s3: no action argument specified\n");
            stat = NC_EINVAL; goto done;
        }

        dumpoptions.action = actionfor(argv[0]);

        if(dumpoptions.key == NULL
	   && dumpoptions.action != EXISTS_ACTION
	   && dumpoptions.action != LIST_ACTION
	   && dumpoptions.action != LISTALL_ACTION
   	   && dumpoptions.action != LONGLIST_ACTION)
	    {
            fprintf(stderr,"no -k argument\n");
            stat = NC_EINVAL;
            goto done;
	    }
        if(dumpoptions.url == NULL)
	    {
            fprintf(stderr,"no -u argument\n");
            stat = NC_EINVAL;
            goto done;
	    }
        if(dumpoptions.prefix == NULL)
	    {
            fprintf(stderr,"no -P argument\n");
            stat = NC_EINVAL;
            goto done;
	    }
	   

        switch (dumpoptions.action) {
        case EXISTS_ACTION: stat = testbucketexists(); break;
        case SIZE_ACTION: stat = testinfo(); break;
        case READ_ACTION: stat = testread(); break;
        case WRITE_ACTION: stat = testwrite(); break;
        case LIST_ACTION: stat = testlist(); break;
        case LONGLIST_ACTION: stat = testlistlong(); break;
        case LISTALL_ACTION: stat = testlistall(); break;
        case DELETE_ACTION: stat = testdeletekey(); break;
        case ERROR_ACTION: /* fall thru */
        default: fprintf(stderr,"Illegal action\n"); exit(1);
        }
    }
done:
    cleanup();
    if(stat)
        printf("*** FAIL: %s(%d)\n",nc_strerror(stat),stat);
    else
        printf("***PASS\n");
    (void)NC_s3sdkfinalize();
    (void)nc_finalize();
    exit(stat?1:0);
}
