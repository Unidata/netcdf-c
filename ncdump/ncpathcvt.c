/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#else
#include <getopt.h>
#endif

#include "netcdf.h"
#include "ncpathmgr.h"

static const char* USAGE =
"ncpathcvt [-c|-C|-m|-u|-w] [-h] [-e] [-F] [-d <driveletter>] [-B<char>] [-k] [-p] PATH\n"
"Options\n"
"  -h help"
"  -e add backslash escapes to '\' and ' '\n"
"  -d <driveletter> use driveletter when needed; defaults to 'c'\n"
"  -B <char> convert occurrences of <char> to blank\n"
"  -F convert occurrences of '\\' to '/'"
"Output type options:\n"
"  -c convert to Cygwin form of path\n"
"  -C return canonical form of path\n"
"  -m convert to MSYS form of path\n"
"  -u convert to Unix form of path\n"
"  -w convert to Windows form of path\n"
"Other options:\n"
"  -k return kind of the local environment\n"
"  -p return kind of the input path\n"
"\n"
"Default is to convert to the format used by the platform.\n"
;

#undef DEBUG

struct Options {
    int target;
    int escapes;
    int drive;
    int debug;
    int canon;
    int blank;
    int slash;
    int pathkind;
} cvtoptions;

static char* escape(const char* path);
static void usage(const char* msg);

static void
usage(const char* msg)
{
    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    fprintf(stderr,"%s",USAGE);
    if(msg == NULL) exit(0); else exit(1);
}

static char*
escape(const char* path)
{
    size_t slen = strlen(path);
    const char* p;
    char* q;
    char* epath = NULL;
    const char* escapes = " \\";

    epath = (char*)malloc((2*slen) + 1);
    if(epath == NULL) usage("out of memtory");
    p = path;
    q = epath;
    for(;*p;p++) {
	if(strchr(escapes,*p) != NULL)
	    *q++ = '\\';
        *q++ = *p;
    }
    *q = '\0';
    return epath;
}

static char*
slash(const char* path)
{
    size_t slen = strlen(path);
    const char* p;
    char* q;
    char* epath = NULL;

    epath = (char*)malloc(slen + 1);
    if(epath == NULL) usage("out of memtory");
    p = path;
    q = epath;
    for(;*p;p++) {
	if(*p == '\\')
	    *q++ = '/';
        else *q++ = *p;
    }
    *q = '\0';
    return epath;
}

void
printlocalkind(void)
{
    const char* s = NULL;
    int kind = NCgetlocalpathkind();
    switch (kind) {
    case NCPD_NIX: s = "unix"; break;
    case NCPD_MSYS: s = "msys"; break;
    case NCPD_CYGWIN: s = "cygwin"; break;
    case NCPD_WIN: s = "win"; break;
    default: s = "unknown"; break;
    }
    printf("%s",s);
    exit(0);
}

void
printenv(void)
{
#ifdef __CYGWIN__
    printf(" __CYGWIN__");
#else
    printf(" !__CYGWIN__");
#endif
#ifdef _MSC_VER
    printf(" _MSC_VER");
#else
    printf(" !_MSC_VER");
#endif
#ifdef _WIN32
    printf(" _WIN32");
#else
    printf(" !_WIN32");
#endif
#ifdef __MSYS__
    printf(" __MSYS__");
#else
    printf(" !__MSYS__");
#endif
#ifdef __MSYS2__
    printf(" __MSYS2__");
#else
    printf(" !__MSYS2__");
#endif
#ifdef __MINGW32__
    printf(" __MINGW32__");
#else
    printf(" !__MINGW32__");
#endif
    printf("\n");
    exit(0);
}

void
printpathkind(const char* path)
{
    const char* s = NULL;
    int kind = NCgetinputpathkind(path);
    switch (kind) {
    case NCPD_NIX: s = "unix"; break;
    case NCPD_MSYS: s = "msys"; break;
    case NCPD_CYGWIN: s = "cygwin"; break;
    case NCPD_WIN: s = "win"; break;
    case NCPD_REL: s = "relative"; break;
    default: s = "unknown"; break;
    }
    printf("%s",s);
    exit(0);
}

int
main(int argc, char** argv)
{
    int c;
    char* cvtpath = NULL;
    char* inpath, *canon = NULL;

    memset((void*)&cvtoptions,0,sizeof(cvtoptions));
    cvtoptions.drive = 'c';

    while ((c = getopt(argc, argv, "B:CFcD:d:ehkmpuwX")) != EOF) {
	switch(c) {
	case 'c': cvtoptions.target = NCPD_CYGWIN; break;
	case 'd': cvtoptions.drive = optarg[0]; break;
	case 'e': cvtoptions.escapes = 1; break;
	case 'h': usage(NULL); break;
	case 'k': printlocalkind(); break;
	case 'm': cvtoptions.target = NCPD_MSYS; break;
	case 'p': cvtoptions.pathkind = 1; break;
	case 'u': cvtoptions.target = NCPD_NIX; break;
	case 'w': cvtoptions.target = NCPD_WIN; break;
	case 'B':
	    cvtoptions.blank = optarg[0];
	    if(cvtoptions.blank < ' ' || cvtoptions.blank == '\177')
		usage("Bad -B argument");
	    break;
	case 'C': cvtoptions.canon = 1; break;
	case 'F': cvtoptions.slash = 1; break;
	case 'D':
	    sscanf(optarg,"%d",&cvtoptions.debug);
	    break;
	case 'X': printenv(); break;
	case '?':
	   usage("unknown option");
	   break;
	}
    }

    argc -= optind;
    argv += optind;

    /* If no file arguments left or more than one, print usage message. */
    if (argc == 0)
       usage("no path specified");
    if (argc > 1)
       usage("more than one path specified");

    /* translate blanks */
    inpath = (char*)malloc(strlen(argv[0])+1);
    if(inpath == NULL) usage("Out of memory");
    {
	const char* p = argv[0];
	char* q = inpath;
	for(;*p;p++) {
	    char c = *p;
	    if(c == cvtoptions.blank) c = ' ';
	    *q++ = c;
	}
	*q = '\0';
    } 

    if(cvtoptions.pathkind) {
	printpathkind(inpath);
	goto done;
    }

    /* Canonicalize */
    if(NCpathcanonical(inpath,&canon))
       usage("Could not convert to canonical form");

    if(cvtoptions.canon) {
	cvtpath = canon; canon = NULL;
    } else if(cvtoptions.target == NCPD_UNKNOWN) {
        cvtpath = NCpathcvt(canon);
    } else {
        cvtpath = NCpathcvt_test(canon,cvtoptions.target,(char)cvtoptions.drive);
    }

    if(cvtpath && cvtoptions.escapes) {
	char* path = cvtpath; cvtpath = NULL;
        cvtpath = escape(path);
	free(path);
    }
    if(cvtpath && cvtoptions.slash) {
	char* path = cvtpath; cvtpath = NULL;
        cvtpath = slash(path);
	free(path);
    }
    printf("%s",cvtpath);
done:
    if(canon) free(canon);
    if(inpath) free(inpath);
    if(cvtpath) free(cvtpath);
    return 0;
}

