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
#include "netcdf_filter.h"
#include "netcdf_aux.h"
#include "ncpathmgr.h"
#include "ncbytes.h"

static const char* USAGE =
"ncpathcvt [-c|-m|-u|-w] [-e] [-h] [-k] [-p] [-x] [-F] [-D <driveletter>] [-B<char>] [-S<char>] PATH\n"
"Options\n"
"  -h help"
"  -e add backslash escapes to '\' and ' '\n"
"  -B <char> convert occurrences of <char> to blank\n"
"  -D <driveletter> use driveletter when needed; defaults to 'c'\n"
"  -F convert occurrences of '\\' to '/'"
"  -S <char> use <char> as path separator when parsing;\n"
"     currently limited to ';' or ':' but defaults to ';'\n"
"Output type options:\n"
"  -c convert to Cygwin form of path\n"
"  -m convert to MSYS form of path: currently an alias for -w\n"
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
    int blank;
    int slash;
    int pathkind;
    int sep;
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
processdir(const char* indir, char** cvtdirp)
{
    char* cvtdir = NULL;

    if(cvtoptions.target == NCPD_UNKNOWN) {
        cvtdir = NCpathcvt(indir);
    } else {
        cvtdir = NCpathcvt_test(indir,cvtoptions.target,(char)cvtoptions.drive);
    }

    if(cvtdir && cvtoptions.escapes) {
	char* dir = cvtdir; cvtdir = NULL;
        cvtdir = escape(dir);
	free(dir);
    }
    if(cvtdir && cvtoptions.slash) {
	char* dir = cvtdir; cvtdir = NULL;
        cvtdir = slash(dir);
	free(dir);
    }

    if(cvtdirp) {*cvtdirp = cvtdir; cvtdir = NULL;}

    if(cvtdir) free(cvtdir);
    return 0;
}

int
main(int argc, char** argv)
{
    int c;
    char* inpath  = NULL;
    NCPluginList indirs = {0,NULL};
    NCbytes* outpath = ncbytesnew();
    int stat = NC_NOERR;
    size_t i;
    
    memset((void*)&cvtoptions,0,sizeof(cvtoptions));
    cvtoptions.drive = 'c';
    cvtoptions.sep = ';';

    while ((c = getopt(argc, argv, "B:D:FS:Xchkmpuwx")) != EOF) {
	switch(c) {
	case 'c': cvtoptions.target = NCPD_CYGWIN; break;
	case 'h': usage(NULL); break;
	case 'k': printlocalkind(); break;
	case 'm': cvtoptions.target = NCPD_WIN; break; /* Aliased */
	case 'p': cvtoptions.pathkind = 1; break;
	case 'u': cvtoptions.target = NCPD_NIX; break;
	case 'w': cvtoptions.target = NCPD_WIN; break;
	case 'x': cvtoptions.escapes = 1; break;
	case 'B':
	    cvtoptions.blank = optarg[0];
	    if(cvtoptions.blank < ' ' || cvtoptions.blank == '\177')
		usage("Bad -B argument");
	    break;
	case 'D': cvtoptions.drive = optarg[0]; break;
	case 'F': cvtoptions.slash = 1; break;
	case 'S': cvtoptions.sep = optarg[0]; break;
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

    if(argv[0] == NULL) {
	inpath = strdup("");
    } else {
        inpath = strdup(argv[0]);
    }
    if(inpath == NULL) usage("Out of memory");
    { /* translate blanks */
	char* p = inpath;
	for(;*p;p++) {
	    if(*p == cvtoptions.blank) *p = ' ';
	}
    } 

    if(cvtoptions.pathkind) {
	printpathkind(inpath);
	goto done;
    }

    /* Break using the path separator */
    if((stat = ncaux_plugin_path_parse(inpath,cvtoptions.sep,&indirs)))
	{usage(nc_strerror(stat));}
    for(i=0;i<indirs.ndirs;i++) {
	char* outdir = NULL;
	if((stat = processdir(indirs.dirs[i],&outdir))) 
	    {usage(nc_strerror(stat));}
	if(i > 0) ncbytesappend(outpath,cvtoptions.sep);
	ncbytescat(outpath,outdir);
	nullfree(outdir);
    }
    printf("%s",ncbytescontents(outpath));

done:
    if(inpath) free(inpath);
    ncaux_plugin_path_clear(&indirs);
    ncbytesfree(outpath);
    return 0;
}
