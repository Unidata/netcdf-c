/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
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
#include "ncpathmgr.h"

#undef DEBUG

#define AWSHOST ".amazonaws.com"

typedef enum S3op {
S3_NONE=0,
S3_HOST=1,
S3_BUCKET=2,
S3_KEY=3,
} S3op;

/* Command line options */
struct S3options {
    int debug;
    S3op op;
    char* url;
} s3options;

/*Forward*/
static int processurl(S3op op, const char* url, char** piece);

static void
zs3usage(void)
{
    fprintf(stderr,"usage: zs3parse [-h|-b|-k] <url>|<file>\n");
    exit(1);
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    char* piece = NULL;

    memset((void*)&s3options,0,sizeof(s3options));

    while ((c = getopt(argc, argv, "vhbk")) != EOF) {
	switch(c) {
	case 'b': 
	    s3options.op = S3_BUCKET;
	    break;
	case 'h': 
	    s3options.op = S3_HOST;
	    break;
	case 'k': 
	    s3options.op = S3_KEY;
	    break;
	case 'v': 
	    zs3usage();
	    goto done;
	case '?':
	   fprintf(stderr,"unknown option: %c\n",c);
	   goto fail;
	}
    }

    /* get url|file argument */
    argc -= optind;
    argv += optind;

    if (argc > 1) {
	fprintf(stderr, "zs3parse: only one url|file argument permitted\n");
	goto fail;
    }
    if (argc == 0) {
	fprintf(stderr, "zs3parse: no url|file specified\n");
	goto fail;
    }
    s3options.url = strdup(argv[0]);

    stat = processurl(s3options.op, s3options.url, &piece);
    if(stat == NC_NOERR) {
        if(piece == NULL) goto fail;
	printf("%s",piece);
    }    
done:
    nullfree(piece);
    /* Reclaim s3options */
    nullfree(s3options.url);
    if(stat)
	fprintf(stderr,"fail: %s\n",nc_strerror(stat));
    return (stat ? 1 : 0);    
fail:
    stat = NC_EINVAL;
    goto done;
}

static int
processurl(S3op op, const char* surl, char** piece)
{
    int stat = NC_NOERR;
    NClist* segments = NULL;
    NCbytes* buf = ncbytesnew();
    char* value = NULL;    
    char* host = NULL;    
    char* bucket = NULL;    
    char* prefix = NULL;    
    NCURI* url = NULL;

    ncuriparse(surl,&url);
    if(url == NULL)
        {stat = NC_EURL; goto done;}
    /* do some verification */
    if(strcmp(url->protocol,"https") != 0
       && strcmp(url->protocol,"http") != 0)
        {stat = NC_EURL; goto done;}

    if(url->host == NULL || strlen(url->host) == 0)
        {stat = NC_EURL; goto done;}
    if((host = strdup(url->host))==NULL)
	    {stat = NC_ENOMEM; goto done;}
    /* We have to process the path to get the bucket,
       and remove it from the path */
    if(url->path == NULL || strlen(url->path) == 0)
        {stat = NC_EURL; goto done;}
    /* split the path by "/"  */
    nclistfreeall(segments);
    segments = nclistnew();
    if((stat = nczm_split_delim(url->path,'/',segments))) goto done;
    if(nclistlength(segments) == 0)
	{stat = NC_EURL; goto done;}
    bucket = ((char*)nclistremove(segments,0));
    if((stat = nczm_join(segments,&prefix))) goto done;
    nclistfreeall(segments); segments = NULL;

    switch (op) {
    case S3_HOST: value = host; host = NULL; break;
    case S3_BUCKET: value = bucket; bucket = NULL; break;
    case S3_KEY: value = prefix; prefix = NULL; break;
    default: stat = NC_EURL; goto done;
    }
    
    if(piece) {*piece = value; value = NULL;}

done:
    ncurifree(url);
    nullfree(value); 
    nullfree(host);
    nullfree(bucket);
    nullfree(prefix);
    ncbytesfree(buf);
    nclistfreeall(segments);
    return stat;
}

