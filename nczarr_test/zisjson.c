/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

/* Parse input to see if it looks like json.
   Output 1 or 0.
*/


#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#else
#include <getopt.h>
#endif

#include "netcdf.h"
#include "nclist.h"
#include "ncjson.h"

#define MAXREAD 8192

/* Command line options */
struct Jsonpptions {
    int trace;
} jsonoptions;

static const char*
sortname(int thesort)
{
    switch(thesort) {
    default: break;
    case NCJ_INT: return "NCJ_INT";
    case NCJ_DOUBLE: return "NCJ_DOUBLE";
    case NCJ_BOOLEAN: return "NCJ_BOOLEAN";
    case NCJ_STRING: return "NCJ_STRING";
    case NCJ_DICT: return "NCJ_DICT";
    case NCJ_ARRAY: return "NCJ_ARRAY";
    case NCJ_NULL: return "NCJ_NULL";
    }
    return "?";
}


static void
jsontrace(NCjson* json, int depth)
{
    int i;
    if(json == NULL) goto done;
    printf("[%d] sort=%s",depth,sortname(NCJsort(json)));
    switch(NCJsort(json)) {
    case NCJ_INT:
    case NCJ_DOUBLE:
    case NCJ_BOOLEAN:
    case NCJ_STRING:
	printf(" string=|%s|\n",NCJstring(json));
	break;
    case NCJ_NULL:
	printf("\n");
	break;
    case NCJ_ARRAY:
	printf("\n");
	for(i=0;i<NCJlength(json);i++)
	    jsontrace(NCJith(json,i),depth+1);
	break;
    case NCJ_DICT:
	printf("\n");
	for(i=0;i<NCJlength(json);i+=2) {
	    jsontrace(NCJith(json,i),depth+1);
	    jsontrace(NCJith(json,i+1),depth+1);
	} break;
    default: break;
    }
done:
    return;
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    char text[MAXREAD+1];
    NCjson* json = NULL;
    int i, red, c;
    FILE* f = NULL;

    nc_initialize();
    memset((void*)&jsonoptions,0,sizeof(jsonoptions));

    while ((c = getopt(argc, argv, "t")) != EOF) {
	switch(c) {
	case 't': jsonoptions.trace = 1; break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   exit(1);
	}
    }	

    /* get file argument */
    argc -= optind;
    argv += optind;

    if (argc > 1) {
	fprintf(stderr, "zisjson: only one input file argument permitted\n");
	exit(1);
    }
    if (argc == 0)
	f = stdin;
    else {
        /* use argv[0] as input */
        f = fopen(argv[0],"r");
        if(f == NULL) {fprintf(stderr,"No such file: %s\n",argv[1]); exit(1);}
    }

    /* Read json from stdin */
    for(i=0;;i++) {
	unsigned char c;
	red = fread(&c, 1, 1, f);
	if(red != 1) break;
	if(i < MAXREAD) text[i] = (char)c;
    }
    if(i >= MAXREAD) {
	fprintf(stderr,"Input too long\n");	
	exit(1);
    }
    text[i] = '\0';
    if(i == 0) {
	stat = NC_EEMPTY;
    } else {
        stat = NCJparse(text,0,&json);
	if(!stat) {
	    if(jsonoptions.trace) jsontrace(json,0);
	    NCJreclaim(json);
	}
    }
    printf("%d",(stat?0:1)); /* parse success|failure */    
    if(f != stdin) fclose(f);
    return 0;
}
