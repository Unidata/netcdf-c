/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "zincludes.h"
#ifdef HAVE_UNISTD_H
#include "unistd.h"
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef _MSC_VER
#include "XGetopt.h"
int opterr;
int optind;
#endif

/* Arguments from command line */
struct Options {
    int debug;
    char* cmd;
};

struct Test {
    char* cmd;
    int (*test)(void);
};

static char* url = NULL;
static struct Options options;

#define NCCHECK(expr) nccheck((expr),__LINE__)
/* Non-static to shut up lint */
void
nccheck(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}

void
makeurl(const char* file)
{
    char wd[4096];
    NCbytes* buf = ncbytesnew();
    ncbytescat(buf,"file://");
    (void)getcwd(wd, sizeof(wd));
    ncbytescat(buf,wd);
    ncbytescat(buf,"/");
    ncbytescat(buf,file);
    ncbytescat(buf,"#mode=nczarr"); /* => use default file: format */
    url = ncbytesextract(buf);
    ncbytesfree(buf);
    fprintf(stderr,"url=|%s|\n",url);
    fflush(stderr);
}

int
setup(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    memset((void*)&options,0,sizeof(options));
    while ((c = getopt(argc, argv, "dc:")) != EOF) {
	switch(c) {
	case 'd': 
	    options.debug = 1;	    
	    break;
	case 'c':
	    if(options.cmd != NULL) {
		fprintf(stderr,"error: multiple commands specified\n");
		stat = NC_EINVAL;
		goto done;
	    }
	    if(optarg == NULL || strlen(optarg) == 0) {
		fprintf(stderr,"error: bad command\n");
		stat = NC_EINVAL;
		goto done;
	    }
	    options.cmd = strdup(optarg);
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   stat = NC_EINVAL;
	   goto done;
	}
    }
    if(options.cmd == NULL) {
	fprintf(stderr,"no command specified\n");
	stat = NC_EINVAL;
	goto done;
    }
done:
    return stat;
}

int
findtest(const char* cmd, struct Test* tests, struct Test** thetest)
{
    int stat = NC_NOERR;
    struct Test* t = NULL;
    *thetest = NULL;
    for(t=tests;t->cmd;t++) {
	if(strcasecmp(t->cmd,cmd)==0) {*thetest = t; break;}
    }
    if(*thetest == NULL) {stat = NC_EINVAL; goto done;}
done:
    return stat;
}
