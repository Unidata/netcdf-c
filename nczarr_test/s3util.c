/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#include "zincludes.h"
#include "ncs3sdk.h"
#include "ncpathmgr.h"
#include "nclog.h"
#include "ncuri.h"
#include "netcdf_aux.h"

#undef NODELETE

#undef DEBUG

#define DATANAME "data"

typedef enum S3op {
S3OP_NONE=0,
S3OP_LIST=1,
S3OP_CLEAR=2,
S3OP_PRINT=3,
S3OP_UPLOAD=4,
S3OP_DOWNLOAD=5,
} S3op;

static struct S3ops {
    S3op s3op;
    const char* opnames[3];
} s3ops[] = {
{S3OP_LIST,{"list","l",NULL}},
{S3OP_CLEAR,{"clear","c",NULL}},
{S3OP_PRINT,{"print","p",NULL}},
{S3OP_UPLOAD,{"upload","u",NULL}},
{S3OP_DOWNLOAD,{"download","d",NULL}},
{S3OP_NONE,{NULL,NULL,NULL}},
};

/* Command line options */
struct Dumpptions {
    int debug;
    S3op s3op;
    NCURI* url;
    char* key; /* via -k flag */    
    char* rootkey; /* from url | key */
    nc_type nctype; /* for printing content */
    char* filename;
    char* profile;
} dumpoptions;

struct S3SDK {
    NCS3INFO s3;
    void* s3client;
    char* errmsg;
} s3sdk;

/* Forward */
static int s3list(void);
static int s3clear(void);
static int s3print(void);
static int s3upload(void);
static int s3download(void);
static nc_type typefor(const char* t);
static void printcontent(size64_t len, const char* content, nc_type nctype);

static void
usage(void)
{
    fprintf(stderr,"usage: s3util  list|print|upload|download|clear -u <url> [-k <key.] [-f <filename>]\n");
    exit(1);
}

static S3op
decodeop(const char* name)
{
    struct S3ops* s3op = s3ops;
    const char** s = NULL;
    for(;s3op->opnames[0] != NULL;s3op++) {
        for(s=s3op->opnames;*s;s++) {
	    if(strcasecmp(*s,name)==0) return s3op->s3op;
	}
    }
    return S3OP_NONE;
}

static int
s3setup(void)
{
    int stat = NC_NOERR;
    s3sdk.s3client = NC_s3sdkcreateclient(&s3sdk.s3);
    return stat;
}

static int
s3shutdown(int deleteit)
{
    int stat = NC_NOERR;
    stat = NC_s3sdkclose(s3sdk.s3client, &s3sdk.s3, deleteit, &s3sdk.errmsg);
    return stat;
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    char* tmp = NULL;

    nc_initialize();

    memset((void*)&dumpoptions,0,sizeof(dumpoptions));

    dumpoptions.nctype = NC_UBYTE; /* default */

    while ((c = getopt(argc, argv, "df:hk:p:t:T:u:v")) != EOF) {
	switch(c) {
	case 'd': 
	    dumpoptions.debug = 1;	    
	    break;
	case 'f':
	    dumpoptions.filename = strdup(optarg);
	    break;
	case 'h':
	    usage();
	    return 0;
	case 'k': {
	    size_t len = strlen(optarg);
	    dumpoptions.key = (char*)malloc(len+1+1);
	    if(*optarg != '/') {
	        fprintf(stderr,"warning: -k option does not start with '/': %s",optarg);
		dumpoptions.key[0] = '/';
		memcpy(dumpoptions.key+1,optarg,len);
		len++;
	    } else
		memcpy(dumpoptions.key,optarg,strlen(optarg));
	    dumpoptions.key[len] = '\0';
	    } break;
	case 'p': 
	    dumpoptions.profile = strdup(optarg);
	    break;
	case 't': 
	    dumpoptions.nctype = typefor(optarg);
	    break;
	case 'u': {
 	        char* p = NC_shellUnescape(optarg);
		ncuriparse(p,&dumpoptions.url);
		nullfree(p);
		if(dumpoptions.url == NULL) {
		    fprintf(stderr,"malformed -f option: %s",optarg);
		    stat = NC_EINVAL;
		    goto done;
		}
	    } break;
	case 'v': 
	    usage();
	    goto done;
	case 'T':
	    nctracelevel(atoi(optarg));
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   stat = NC_EINVAL;
	   goto done;
	}
    }

    /* get command argument */
    argc -= optind;
    argv += optind;

    if (argc > 1) {
	fprintf(stderr, "s3util: only one command argument permitted\n");
	stat = NC_EINVAL;
	goto done;
    }
    if (argc == 0) {
	fprintf(stderr, "s3util: no command specified\n");
	stat = NC_EINVAL;
	goto done;
    }

    dumpoptions.s3op = decodeop(argv[0]);

    memset(&s3sdk,0,sizeof(s3sdk));

    if((stat = NC_s3urlprocess(dumpoptions.url, &s3sdk.s3))) goto done;
    if(s3sdk.s3.rootkey != NULL && dumpoptions.key != NULL) {
	/* Make the root key be the concatenation of rootkey+dumpoptions.key */
        if((stat = nczm_concat(s3sdk.s3.rootkey,dumpoptions.key,&tmp))) goto done;
	nullfree(s3sdk.s3.rootkey);
	s3sdk.s3.rootkey = tmp; tmp = NULL;
    } else if(dumpoptions.key != NULL) {
	s3sdk.s3.rootkey = dumpoptions.key;
	dumpoptions.key = NULL;
    }
    if(s3sdk.s3.rootkey == NULL || strlen(s3sdk.s3.rootkey)==0)
        s3sdk.s3.rootkey = strdup("/");

    switch (dumpoptions.s3op) {
    default:
	fprintf(stderr,"Default action: list\n");
	/* fall thru */
    case S3OP_LIST:
	if((stat = s3list())) goto done;
	break;
    case S3OP_CLEAR:
	if((stat = s3clear())) goto done;
	break;
    case S3OP_PRINT:
	if((stat = s3print())) goto done;
	break;
    case S3OP_UPLOAD:
	if((stat = s3upload())) goto done;
	break;
    case S3OP_DOWNLOAD:
	if((stat = s3download())) goto done;
	break;
    }    

done:
    /* Reclaim dumpoptions */
    ncurifree(dumpoptions.url);
    nullfree(dumpoptions.rootkey);
    nullfree(tmp);
    NC_s3clear(&s3sdk.s3);
    nc_finalize();
    if(stat)
	fprintf(stderr,"fail: %s\n",nc_strerror(stat));
    return (stat ? 1 : 0);    
}

static int
s3list(void)
{
    int stat = NC_NOERR;
    size_t nkeys = 0;
    char** keys = NULL;

    if(s3setup()) goto done;

    stat = NC_s3sdksearch(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, &nkeys, &keys, &s3sdk.errmsg);
    if(stat) goto done;

    if(nkeys > 0) {
        size_t i;
	/* Sort the list -- shortest first */
	nczm_sortenvv(nkeys,keys);
	for(i=0;i<nkeys;i++) {
	    printf("[%u] %s\n",(unsigned)i,keys[i]);
	}

    } else
	printf("<empty>\n");

done:
    s3shutdown(0);
    NCZ_freeenvv(nkeys,keys);    
    return stat;
}

static int
s3clear(void)
{
    int stat = NC_NOERR;
    size_t nkeys = 0;
    char** keys = NULL;

    if(s3setup()) goto done;

    stat = NC_s3sdksearch(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, &nkeys, &keys, &s3sdk.errmsg);
    if(stat) goto done;

    if(nkeys > 0) {
	size_t i;
	/* Sort the list -- shortest first */
	nczm_sortenvv(nkeys,keys);
        printf("deleted keys:\n");
	for(i=0;i<nkeys;i++) {
            printf("\t%s\n",keys[i]);
#ifndef NODELETE
	    if((stat = NC_s3sdkdeletekey(s3sdk.s3client, s3sdk.s3.bucket, keys[i], &s3sdk.errmsg)))
		goto done;
#endif
	}
    }

done:
    s3shutdown(0);
    NCZ_freeenvv(nkeys,keys);    
    return stat;
}

static int
s3print(void)
{
    int stat = NC_NOERR;
    size_t nkeys = 0;
    char** keys = NULL;
    size64_t count;
    char* content = NULL;

    if(s3setup()) goto done;

    if((stat = NC_s3sdkinfo(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, &count,&s3sdk.errmsg)))
	goto done;

    if((content = (char*)calloc(1,count+1))==NULL)
        {stat = NC_ENOMEM; goto done;}

    if((stat = NC_s3sdkread(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, 0, count, (void*) content, &s3sdk.errmsg)))
	goto done;

    printcontent(count,content,dumpoptions.nctype);

done:
    if(content) free(content);
    s3shutdown(0);
    NCZ_freeenvv(nkeys,keys);    
    return stat;
}

static int
s3upload(void)
{
    int stat = NC_NOERR;
    size_t red = 0;
    void* content = NULL;

    if(s3setup()) goto done;

    if((stat = ncaux_readfile(dumpoptions.filename,&red,&content)))
        goto done;

    if((stat = NC_s3sdkwriteobject(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, red, content, &s3sdk.errmsg)))
	goto done;

done:
    if(content) free(content);
    s3shutdown(0);
    return stat;
}

static int
s3download(void)
{
    int stat = NC_NOERR;
    size64_t count;
    char* content = NULL;

    if(s3setup()) goto done;

    if((stat = NC_s3sdkinfo(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, &count,&s3sdk.errmsg)))
	goto done;

    if((content = (char*)calloc(1,count))==NULL)
        {stat = NC_ENOMEM; goto done;}

    if((stat = NC_s3sdkread(s3sdk.s3client, s3sdk.s3.bucket, s3sdk.s3.rootkey, 0, count, (void*) content, &s3sdk.errmsg)))
	goto done;

    if((stat = ncaux_writefile(dumpoptions.filename,count,content)))
        goto done;

done:
    if(content) free(content);
    s3shutdown(0);
    return stat;
}

static void
printcontent(size64_t len, const char* content, nc_type nctype)
{
    size64_t i;

    if(len == 0) {
        printf("<empty>\n");
	return;
    }

    if(nctype == NC_STRING) printf("|");
    for(i=0;i<len;i++) {
	    if(nctype != NC_STRING && i > 0) printf(", ");
	    switch(nctype) {
	    case NC_BYTE: printf("%d",((char*)content)[i]); break;
	    case NC_SHORT: printf("%d",((short*)content)[i]); break;		
	    case NC_INT: printf("%d",((int*)content)[i]); break;		
	    case NC_INT64: printf("%lld",((long long*)content)[i]); break;		
	    case NC_UBYTE: printf("%u",((unsigned char*)content)[i]); break;
	    case NC_USHORT: printf("%u",((unsigned short*)content)[i]); break;		
	    case NC_UINT: printf("%u",((unsigned int*)content)[i]); break;		
	    case NC_UINT64: printf("%llu",((unsigned long long*)content)[i]); break;		
	    case NC_FLOAT: printf("%f",((float*)content)[i]); break;		
	    case NC_DOUBLE: printf("%lf",((double*)content)[i]); break;		
	    case NC_STRING: putc(content[i],stdout); break;
	    default: abort();
	    }
    }
    if(nctype == NC_STRING) printf("|\n");
}

static nc_type
typefor(const char* t)
{
    if(strcmp(t,"NC_BYTE")==0) return NC_BYTE;
    else if(strcmp(t,"NC_SHORT")==0) return NC_SHORT;
    else if(strcmp(t,"NC_INT")==0) return NC_INT;
    else if(strcmp(t,"NC_INT64")==0) return NC_INT64;
    else if(strcmp(t,"NC_UBYTE")==0) return NC_UBYTE;
    else if(strcmp(t,"NC_USHORT")==0) return NC_USHORT;
    else if(strcmp(t,"NC_UINT")==0) return NC_UINT;
    else if(strcmp(t,"NC_UINT64")==0) return NC_UINT64;
    else if(strcmp(t,"NC_FLOAT")==0) return NC_FLOAT;
    else if(strcmp(t,"NC_DOUBLE")==0) return NC_DOUBLE;
    else if(strcmp(t,"NC_STRING")==0) return NC_STRING;
    return NC_NAT;
}
