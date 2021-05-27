#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <netcdf.h>

#define CHECK(err) {if(err) report(err,__LINE__);}

static void
report(int err, int lineno)
{
    fprintf(stderr,"Error: %d: %s\n", lineno, nc_strerror(err));
    exit(1);
}

void
usage(void)
{
    fprintf(stderr,"usage: printfqn <filename> <varname>\n");
    exit(0);
}

int
get_type_parent(int ncid, int tid, int* parentp)
{
    int stat = NC_NOERR;
    int i;
    int nids;
    int ids[4096];

    /* Does this group have the type we are searching for? */
    if((stat=nc_inq_typeids(ncid,&nids,ids))) goto done;
    assert(nids < 4096);

    /* Search for this typeid */
    for(i=0;i<nids;i++) {
	if(ids[i] == tid) {
	    if(parentp) *parentp = ncid;
	    goto done;
	}
    }

    /* Else Search subgroups. */
    if((stat=nc_inq_grps(ncid,&nids,ids))) goto done;
    assert(nids < 4096);

    /* Recurse on each subgroup */
    for(i=0;i<nids;i++) {
	switch (stat = get_type_parent(ids[i],tid,parentp)) {
	case NC_EBADTYPE: break; /* not found; keep looking */
	case NC_NOERR: goto done; /* found it */
	default: goto done; /* some other error */
	}
    }
    stat = NC_EBADTYPE; /* Not found */

done:
    return stat;

}

int
get_variable_info(int ncid, const char* name, int* gidp, int* vidp)
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
	    goto done;
	}
    }

    /* Else Search subgroups. */
    if((stat=nc_inq_grps(ncid,&nids,ids))) goto done;
    assert(nids < 4096);

    /* Recurse on each subgroup */
    for(i=0;i<nids;i++) {
	switch (stat = get_variable_info(ids[i],name,gidp,vidp)) {
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
    const char* filename = NULL;
    char varname[NC_MAX_NAME];
    size_t fqnlen, namelen;
    char fqn[4096];
    char name[NC_MAX_NAME];
    
    if(argc < 3) usage();

    filename = argv[1];
    strcpy(varname,argv[2]);

    CHECK(nc_open(filename,NC_NETCDF4,&ncid));

    /* Locate the parent group for the variable */
    CHECK(get_variable_info(ncid,varname,&gid,&varid));

    /* Get the type id of the variable */
    CHECK(nc_inq_vartype(gid,varid,&tid));
    /* Get the type name */
    CHECK(nc_inq_type(ncid,tid,name,&namelen));

    /* Get the containing group id for the type */
    CHECK(get_type_parent(ncid,tid,&gid));

    /* Get the FQN name for the containing type */
    CHECK(nc_inq_grpname_full(gid,&fqnlen,fqn));
    assert(fqnlen < sizeof(fqn));

    if(strcmp(fqn,"/")==0) fqn[0] = '\0';

    /* report result  with no newline */
    printf("|%s/%s|",fqn,name);

    return 0;
}
