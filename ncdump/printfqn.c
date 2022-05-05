#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && ! defined(__MINGW32__)
#include "XGetopt.h"
#endif

#include <netcdf.h>

#define CHECK(err) {if(err) report(err,__LINE__);}

/* Command line options */
struct Options {
    int debug;
    enum What {NONE=0, DIM=1, TYPE=2} what;
    char file[4096];
    char var[NC_MAX_NAME+1];
    char object[NC_MAX_NAME+1];
} options;

/**************************************************/


static void
report(int err, int lineno)
{
    fprintf(stderr,"Error: %d: %s\n", lineno, nc_strerror(err));
    exit(1);
}

void
usage(void)
{
    fprintf(stderr,"usage: printfqn [-D] [-V] -t|-d -v <varname> -f <filename> \n");
    exit(0);
}

int
get_id_parent(int ncid, int id, int* parentp, enum What what)
{
    int stat = NC_NOERR;
    int i;
    int nids;
    int ids[4096];

    /* Does this group have the id we are searching for? */
    if(what == TYPE) {
        if((stat=nc_inq_typeids(ncid,&nids,ids))) goto done;
    } else if(what == DIM) {
        if((stat=nc_inq_dimids(ncid,&nids,ids,0))) goto done;
    } else
        abort();
    assert(nids < 4096);

    /* Search for this id */
    for(i=0;i<nids;i++) {
	if(ids[i] == id) {
	    if(parentp) *parentp = ncid;
	    goto done;
	}
    }

    /* Else Search subgroups. */
    if((stat=nc_inq_grps(ncid,&nids,ids))) goto done;
    assert(nids < 4096);

    /* Recurse on each subgroup */
    for(i=0;i<nids;i++) {
	switch (stat = get_id_parent(ids[i],id,parentp,what)) {
	case NC_ENOTFOUND: break; /* not found; keep looking */
	case NC_NOERR: goto done; /* found it */
	default: goto done; /* some other error */
	}
    }
    stat = NC_ENOTFOUND; /* Not found */

done:
    return stat;

}

int
get_variable_info(int ncid, const char* name, int* gidp, int* vidp, int* tidp, int* ndimsp, int* dimids)
{
    int stat = NC_NOERR;
    int i;
    int nids;
    int ids[4096];
    char varname[NC_MAX_NAME];

    /* Assume only one occurrence of the variable in dataset */

    /* Does this group have the variable we are searching for? */
    if((stat=nc_inq_varids(ncid,&nids,ids))) goto done;
    assert(nids < 4096);

    /* Search for this variable name */
    for(i=0;i<nids;i++) {
	if((stat = nc_inq_varname(ncid,ids[i],varname))) goto done;
	if(strcmp(name,varname)==0) {
	    if(gidp) *gidp = ncid;
	    if(vidp) *vidp = ids[i];
	    if((stat = nc_inq_vartype(ncid,ids[i],tidp))) goto done;
    	    if((stat = nc_inq_varndims(ncid,ids[i],ndimsp))) goto done;
       	    if((stat = nc_inq_vardimid(ncid,ids[i],dimids))) goto done;
	    goto done;
	}
    }

    /* Else Search subgroups. */
    if((stat=nc_inq_grps(ncid,&nids,ids))) goto done;
    assert(nids < 4096);

    /* Recurse on each subgroup */
    for(i=0;i<nids;i++) {
	switch (stat = get_variable_info(ids[i],name,gidp,vidp,tidp,ndimsp,dimids)) {
	case NC_ENOTVAR: break; /* not found; keep looking */
	case NC_NOERR: goto done; /* found it */
	default: goto done; /* some other error */
	}
    }
    stat = NC_ENOTVAR; /* Not found */

done:
    return stat;
}

int
main(int argc, char** argv)
{
    int ncid, varid, gid, tid;
    size_t fqnlen, namelen;
    char fqn[4096];
    char name[NC_MAX_NAME];
    int ndims;
    int dimids[NC_MAX_VAR_DIMS];
    int c;

    memset((void*)&options,0,sizeof(options));

    while ((c = getopt(argc, argv, "DVdtv:f:")) != EOF) {
	switch(c) {
	case 'D': 
	    options.debug = 1;	    
	    break;
	case 'V': 
	    usage();
	    break;
	case 'd':
	    options.what = DIM;
	    break;
	case 't':
	    options.what = TYPE;
	    break;
	case 'v':
	    strcpy(options.var,optarg);
	    break;
	case 'f':
	    strcpy(options.file,optarg);
	    break;
	case ':':
	   fprintf(stderr,"option has no argument: %c\n",c);
	    usage();
	case '?':
	   fprintf(stderr,"unknown option\n");
	    usage();
	}
    }

    CHECK(nc_open(options.file,NC_NETCDF4,&ncid));

    /* Locate the parent group for the variable */
    CHECK(get_variable_info(ncid,options.var,&gid,&varid,&tid,&ndims,dimids));

    if(options.what == TYPE) {
        /* Get the simple type name from the variable */
        CHECK(nc_inq_type(ncid,tid,name,&namelen));
	/* Get the containing group id for the type (might not be same as ncid) */
        CHECK(get_id_parent(ncid,tid,&gid,TYPE));
    } else if(options.what == DIM) {
	/* get name of dimids[0] from the variable */
        CHECK(nc_inq_dimname(ncid,dimids[0],name));
        /* Get the containing group id for the type */
        CHECK(get_id_parent(ncid,dimids[0],&gid,DIM));
    }

    /* Get the FQN name for the containing group of the id */
    CHECK(nc_inq_grpname_full(gid,&fqnlen,fqn));
    assert(fqnlen < sizeof(fqn));

    if(strcmp(fqn,"/")==0) fqn[0] = '\0';

    /* report result  with no newline */
    printf("%s/%s",fqn,name);

    return 0;
}
