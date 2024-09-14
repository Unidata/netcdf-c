/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *	See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#include "netcdf.h"
#include "netcdf_filter.h"
#include "netcdf_aux.h"

#undef DEBUG

/* Define max number of -x actions */
#define NACTIONS 64
/* Define max length of -x action string */
#define NACTIONLEN 4096

/* Define max no. of dirs in path list */
#define NDIRSMAX 64

typedef enum Action {
ACT_NONE=0,
ACT_READ=1,
ACT_WRITE=2,
/* Synthetic Actions */
ACT_CLEAR=3,
ACT_FORMATX=4,
} Action;

static struct ActionTable {
    Action op;
    const char* opname;
} actiontable[] = {
{ACT_NONE,"none"},
{ACT_READ,"read"},
{ACT_WRITE,"write"},
{ACT_CLEAR,"clear"},
{ACT_FORMATX,"formatx"},
{ACT_NONE,NULL}
};

static struct FormatXTable {
    const char* name;
    int formatx;
} formatxtable[] = {
{"all",0},
{"hdf5",NC_FORMATX_NC_HDF5},
{"nczarr",NC_FORMATX_NCZARR},
{"zarr",NC_FORMATX_NCZARR},
{NULL,0}
};

/* command line options */
struct Dumpptions {
    int debug;
    size_t nactions;
    int formatx; /* default formatx */
    struct Execute {
	Action action;
	char* name;
	char* arg;
    } actions[NACTIONS];
} dumpoptions;

/* Forward */

#define NCCHECK(expr) nccheck((expr),__LINE__)
static void ncbreakpoint(int stat) {stat=stat;}
static int nccheck(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
	fflush(stderr);
	ncbreakpoint(stat);
	exit(1);
    }
    return stat;
}

static void
pluginusage(void)
{
    fprintf(stderr,"usage: tst_pluginpath [-d] -x <command>[:<arg>],<command>[:<arg>]...\n");
    fprintf(stderr,"\twhere <command> is one of: read | write | clear| or formatx.\n");
    fprintf(stderr,"\twhere <arg> is arbitrary string (with '\\,' to escape commas); arg can be missing or empty.\n");
    exit(1);
}

static int
decodeformatx(const char* name)
{
    struct FormatXTable* p = formatxtable;
    for(;p->name != NULL;p++) {
	if(strcasecmp(p->name,name)==0) return p->formatx;
    }
    return -1;
}

static Action
decodeop(const char* name)
{
    struct ActionTable* p = actiontable;
    for(;p->opname != NULL;p++) {
	if(strcasecmp(p->opname,name)==0) return p->op;
    }
    return ACT_NONE;
}

/* Unescape all escaped characters in s */
static void
descape(char* s)
{
    char* p;
    char* q;
    if(s == NULL) goto done;
    for(p=s,q=s;*p;) {
	if(*p == '\\' && p[1] != '\0') p++;
	*q++ = *p++;
    }
    *q = *p; /* nul terminate */
done:
    return;
}

/* A version of strchr that takes escapes into account */
static char*
xstrchr(char* s, char c)
{
    int leave;
    char* p;
    for(leave=0,p=s;!leave;p++) {
	switch (*p) {
	case '\\': p++; break;
	case '\0': return NULL; break;
	default: if(*p == c) return p; break;
	}
    }
    return NULL;
}

static void
parseactionlist(const char* cmds0)
{
    size_t i,cmdlen;
    char cmds[NACTIONLEN+2];
    char* p;
    char* q;
    size_t ncmds;
    int leave;

    memset(cmds,0,sizeof(cmds));
    if(cmds0 == NULL) cmdlen = 0; else cmdlen = strlen(cmds0);
    if(cmdlen == 0) {fprintf(stderr,"error: -x must have non-empty argument.\n"); pluginusage();}
    if(cmdlen > NACTIONLEN) {fprintf(stderr,"error: -x argument too lone; max is %zu\n",(size_t)NACTIONLEN); pluginusage();}
    strncpy(cmds,cmds0,cmdlen);
    /* split into command + formatx + arg strings and count */
    ncmds = 0;
    for(leave=0,p=cmds;!leave;p=q) {
	q = xstrchr(p,',');
	if(q == NULL) {
	    q = cmds+cmdlen; /* point to trailing nul */
	    leave = 1;
	} else {
	    *q++ = '\0'; /* overwrite ',' and skip to start of the next command*/
	}	
	ncmds++;
    }
    if(ncmds > NACTIONS) {fprintf(stderr,"error: -x must have not more than %zu commands.\n",(size_t)NACTIONS); pluginusage();}
    dumpoptions.nactions = ncmds;
    /* Now process each command+formatx+arg triple */
    for(p=cmds,i=0;i<ncmds;i++) {
	size_t clen,alen;
	clen = strlen(p);
	if(clen > NACTIONLEN) {fprintf(stderr,"error: -x cmd '%s' too long; max is %zu\n",p,(size_t)NACTIONLEN); pluginusage();}
	/* search for ':' taking escapes into account */
	q = xstrchr(p,':');
	if(q == NULL)
	    q = p+clen; /* point to trailing nul */
	else
	    *q++ = '\0'; /* overwrite ':' and skip to start of the arg*/
	dumpoptions.actions[i].name = nulldup(p);
	/* Get the argument, if any */
	alen = strlen(q);
	if(alen > 0) {
	    dumpoptions.actions[i].arg = strdup(q);
	}
	p += (clen+1); /* move to next cmd+arg pair */
    }
    /* De-escape names and args and compute action enum */
    for(i=0;i<dumpoptions.nactions;i++) {
	descape(dumpoptions.actions[i].name);
	descape(dumpoptions.actions[i].arg);
	dumpoptions.actions[i].action = decodeop(dumpoptions.actions[i].name);
    }
    return;
}

/**************************************************/

static int
actionclear(const struct Execute* action)
{
    int stat = NC_NOERR;
    if((stat=nc_plugin_path_write(dumpoptions.formatx,0,NULL))) goto done;   
done:
    return NCCHECK(stat);
}

static int
actionformatx(const struct Execute* action)
{
    int stat = NC_NOERR;
    dumpoptions.formatx = decodeformatx(action->arg);
    if(dumpoptions.formatx < 0) {fprintf(stderr,"error: formatx: illegal argument: %s\n",action->arg); pluginusage();}
    return NCCHECK(stat);
}

static int
actionread(const struct Execute* action)
{
    int stat = NC_NOERR;
    size_t ndirs;
    char** dirs = NULL;
    char* text = NULL;
    size_t textlen;
   
    if((stat=nc_plugin_path_read(dumpoptions.formatx,&ndirs,NULL))) goto done;
    assert(ndirs > 0);
    if((dirs = (char**)calloc(ndirs,sizeof(char*)))==NULL) {stat = NC_ENOMEM; goto done;}
    if((stat = nc_plugin_path_read(dumpoptions.formatx,&ndirs,dirs))) goto done;
    if((stat = ncaux_plugin_path_tostring(ndirs,dirs,0,&textlen,NULL))) goto done;
    if((text = (char*)malloc(textlen))==NULL) {stat = NC_ENOMEM; goto done;}
    if((stat = ncaux_plugin_path_tostring(ndirs,dirs,0,&textlen,text))) goto done;
    if(stat == NC_NOERR) printf("%s\n",text);   
done:
    nullfree(text);
    ncaux_plugin_path_freestringvec(ndirs,dirs);
    return NCCHECK(stat);
}

static int
actionwrite(const struct Execute* action)
{
    int stat = NC_NOERR;
    const char* text = action->arg;
    size_t ndirs;
    char** dirs = NULL;

    if(text == NULL) text = "";
    if((stat=ncaux_plugin_path_parse(text,0,&ndirs,NULL))) goto done;
    if(ndirs > 0) {
	if((dirs = (char**)calloc(ndirs,sizeof(char*)))==NULL) {stat = NC_ENOMEM; goto done;}
        if((stat=ncaux_plugin_path_parse(text,0,&ndirs,dirs))) goto done;
    }
    if((stat=nc_plugin_path_write(dumpoptions.formatx,ndirs,dirs))) goto done;   

done:
    ncaux_plugin_path_freestringvec(ndirs,dirs);
    return NCCHECK(stat);
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    size_t i;

    /* Init options */
    memset((void*)&dumpoptions,0,sizeof(dumpoptions));

    while ((c = getopt(argc, argv, "dvx:X:")) != EOF) {
	switch(c) {
	case 'd': 
	    dumpoptions.debug = 1;	    
	    break;
	case 'v': 
	    pluginusage();
	    goto done;
	case 'x':
	    parseactionlist(optarg);
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   {stat = NC_EINVAL; goto done;}
	}
    }

    for(i=0;i<dumpoptions.nactions;i++) {
#ifdef DEBUG
fprintf(stderr,">>>> [%zu] %s(%d) : %s\n",i,
					  dumpoptions.actions[i].name,
					  dumpoptions.actions[i].action,
					  dumpoptions.actions[i].arg);
#endif
	switch (dumpoptions.actions[i].action) {
	default:
	    fprintf(stderr,"Illegal action: %s\n",dumpoptions.actions[i].name);
	    pluginusage();
	    break;
	case ACT_CLEAR: if((stat=actionclear(&dumpoptions.actions[i]))) goto done; break;
	case ACT_FORMATX: if((stat=actionformatx(&dumpoptions.actions[i]))) goto done; break;
	case ACT_READ: if((stat=actionread(&dumpoptions.actions[i]))) goto done; break;
	case ACT_WRITE: if((stat=actionwrite(&dumpoptions.actions[i]))) goto done; break;
	}
    }

done:
    fflush(stdout);
    if(stat)
	fprintf(stderr,"fail: %s\n",nc_strerror(stat));
    return (stat ? 1 : 0);    
}
