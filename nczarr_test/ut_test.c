/*
x *      Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"
#include "ncpathmgr.h"
#include "nclog.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

struct ZUTEST zutester;

struct UTOptions utoptions;

/*Forward*/
static void canonicalfile(char** fp);

void
usage(int err)
{
    if(err) {
	fprintf(stderr,"error: (%d) %s\n",err,nc_strerror(err));
    }
    fprintf(stderr,"usage:");
        fprintf(stderr," -D/*debug*/");
        fprintf(stderr," -x<cmd,cmd,...>");
        fprintf(stderr," -f<inputfilename>");
        fprintf(stderr," -o<outputfilename>");
        fprintf(stderr," -k<kind>");
        fprintf(stderr," -d<dim>=<len>");
        fprintf(stderr," -v<type>var(<dim/chunksize,dim/chunksize...>)");
        fprintf(stderr," -s<slices>");
        fprintf(stderr," -W<int>,<int>...");
	fprintf(stderr,"\n");	
    fflush(stderr);
    exit(1);
}

int
ut_init(int argc, char** argv, struct UTOptions * options)
{
    int stat = NC_NOERR;
    int c;
    Dimdef* dimdef = NULL;
    Vardef* vardef = NULL;

    nc_initialize();

    if(options != NULL) {
	options->dimdefs = nclistnew();
	options->vardefs = nclistnew();
        while ((c = getopt(argc, argv, "T:Dx:f:o:p:k:d:v:s:W:")) != EOF) {
            switch(c) {
            case 'T':  
	        nctracelevel(atoi(optarg));
                break;
            case 'D':  
                options->debug = 1;     
                break;
            case 'x': /*execute*/
		if(parsestringvector(optarg,0,&options->cmds) <= 0) usage(THROW(0));
                break;
            case 'f':
		options->file = strdup(optarg);
                break;
            case 'o':
		options->output = strdup(optarg);
                break;
            case 'p':
		options->profile = strdup(optarg);
                break;
            case 'k': /*implementation*/
		options->kind = strdup(optarg);
                break;
            case 'd': /*dimdef*/
		if((stat=parsedimdef(optarg,&dimdef))) usage(THROW(stat));
		nclistpush(options->dimdefs,dimdef);
		dimdef = NULL;
                break;
            case 'v': /*vardef*/
		if((stat=parsevardef(optarg,options->dimdefs,&vardef))) usage(THROW(stat));
		nclistpush(options->vardefs,vardef);
		vardef = NULL;
                break;
            case 's': /*slices*/
		if((stat=parseslices(optarg,&options->nslices,options->slices))) usage(THROW(stat));
                break;
            case 'W': /*walk data*/
		options->idatalen = parseintvector(optarg,4,(void**)&options->idata);
                break;
            case '?':
               fprintf(stderr,"unknown option: '%c'\n",c);
               stat = NC_EINVAL;
               goto done;
            }
        }
    }

    canonicalfile(&options->file);
    canonicalfile(&options->output);
    
done:
    return THROW(stat);
}

void
ut_final(void)
{
    nc_finalize();
}

#if 0
static void
getpathcwd(char** cwdp)
{
    char buf[4096];
    (void)NCgetcwd(buf,sizeof(buf));
    if(cwdp) *cwdp = strdup(buf);
}
#endif

static void
canonicalfile(char** fp)
{
    size_t len;
    char* f = NULL;
    char* abspath = NULL;
    NCURI* uri = NULL;
#ifdef _WIN32
    int fwin32=0, cwd32=0;
#endif

    if(fp == NULL || *fp == NULL) return;
    f = *fp;
    len = strlen(f);
    if(len <= 1) return;
    ncuriparse(f,&uri);
    if(uri != NULL) {ncurifree(uri); return;} /* its a url */

#if 1
    abspath = NCpathabsolute(f);
#else
    if(f[0] == '/' || f[0] == '\\' || hasdriveletter(f))
        return; /* its already absolute */
#ifdef _WIN32
    for(p=f;*p;p++) {if(*p == '\\') {*p = '/';}}
#endif
    if(len >= 2 && memcmp(f,"./",2)==0) {
	offset = 1; /* leave the '/' */
    } else if(len >= 3 && memcmp(f,"../",3)==0) {
	offset = 2;
    } else
        offset = 0;
    getpathcwd(&cwd);
    len2 = strlen(cwd);
#ifdef _WIN32
    for(cwd32=0,p=cwd;*p;p++) {if(*p == '\\') {*p = '/'; cwd32 = 1;}}
#endif
    if(offset == 2) {
        p = strrchr(cwd,'/');
        /* remove last segment including the preceding '/' */
	if(p == NULL) {cwd[0] = '\0';} else {*p = '\0';}
    }
    len2 = (len-offset)+strlen(cwd);
    if(offset == 0) len2++; /* need to add '/' */
    abspath = (char*)malloc(len2+1);
    abspath[0] = '\0';
    strlcat(abspath,cwd,len2+1);
    if(offset == 0) strlcat(abspath,"/",len2+1);
    strlcat(abspath,f+offset,len2+1);
#ifdef _WIN32
    if(fwin32)
     for(p=abspath;*p;p++) {if(*p == '/') {*p = '\\';}}
#endif
    nullfree(cwd);
#endif
    nullfree(f);
fprintf(stderr,"canonicalfile: %s\n",abspath);
    *fp = abspath;
}

void
nccheck(int stat, int line)
{
    if(stat) {
        fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

char*
makeurl(const char* file, NCZM_IMPL impl, struct UTOptions* options)
{
    char* url = NULL;
    NCbytes* buf = ncbytesnew();
    NCURI* uri = NULL;
    const char* kind = impl2kind(impl);
    char* urlpath = NULL;
    char* p;

    if(file && strlen(file) > 0) {
	switch (impl) {
	case NCZM_FILE:
	case NCZM_ZIP:
            /* Massage file to make it usable as URL path */
	    urlpath = strdup(file);
	    for(p=urlpath;*p;p++) {if(*p == '\\') *p = '/';}
            ncbytescat(buf,"file://");
            ncbytescat(buf,urlpath);
	    nullfree(urlpath); urlpath = NULL;
            ncbytescat(buf,"#mode=nczarr"); /* => use default file: format */
	    ncbytescat(buf,",");
	    ncbytescat(buf,kind);
	    break;
	case NCZM_S3:
	    /* Assume that we have a complete url */
	    if(ncuriparse(file,&uri)) return NULL;
	    if(options->profile) {
		const char* profile = ncurifragmentlookup(uri,"aws.profile");
		if(profile == NULL) {
		    ncurisetfragmentkey(uri,"aws.profile",options->profile);
		    /* rebuild the url */
		    file = (const char*)ncuribuild(uri,NULL,NULL,NCURIALL); /* BAD but simple */
		}
	    }
	    ncbytescat(buf,file);
	    break;
	default: abort();
	}
	url = ncbytesextract(buf);
    }
    ncurifree(uri);
    ncbytesfree(buf);
    fprintf(stderr,"url=|%s|\n",url);
    fflush(stderr);
    return url;
}

struct Test*
findtest(const char* cmd, struct Test* tests)
{
    struct Test* t = NULL;
    for(t=tests;t->cmd;t++) {
        if(strcasecmp(t->cmd,cmd)==0) return t;
    }
    return NULL;
}

int
runtests(const char** cmds, struct Test* tests)
{
    int stat = NC_NOERR;
    struct Test* test = NULL;
    const char** cmd = NULL;
    if(cmds == NULL) return THROW(NC_EINVAL);
    for(cmd=cmds;*cmd;cmd++) {
        for(test=tests;test->cmd;test++) {
	    if(strcmp(test->cmd,*cmd)==0) {
		if(test->cmd == NULL) return THROW(NC_EINVAL);
		if((stat=test->test())) goto done; /* Execute */
	    }
	}
    }
done:
    return THROW(stat);
}
