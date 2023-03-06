/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

#define DATANAME "data"

#define OPT_NONE	0
#define OPT_OBJDUMP	1
#define OPT_TYPE	2

/* Command line options */
struct Options {
    int debug;
    int typetag;
    int action;
    char* infile;
} options;

/* Forward */
static int objdump(void);

#define NCCHECK(expr) nccheck((expr),__LINE__)
void nccheck(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}

static void
usage(void)
{
    fprintf(stderr,"usage: z4dump <options> <file>\n");
    exit(1);
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;

    memset((void*)&options,0,sizeof(options));

    /* Set defaults */
    options.action = OPT_OBJDUMP;

    while ((c = getopt(argc, argv, "dt")) != EOF) {
	switch(c) {
	case 'd': 
	    options.debug = 1;	    
	    break;
	case 't': 
	    options.typetag = 1;	    
	    break;
	case 'v': 
	    usage();
	    goto done;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   goto fail;
	}
    }

    /* get file argument */
    argc -= optind;
    argv += optind;

    if (argc > 1) {
	fprintf(stderr, "z4dump: only one input file argument permitted\n");
	goto fail;
    }
    if (argc == 0) {
	fprintf(stderr, "z4dump: no input file specified\n");
	goto fail;
    }
    options.infile = strdup(argv[0]);

    switch (options.action) {
    case OPT_OBJDUMP:
	if((stat = objdump())) goto done;
	break;
    default:
	fprintf(stderr,"Unimplemented action\n");
	goto fail;
    }    

done:
    if(stat)
	nc_strerror(stat);
    return (stat ? 1 : 0);    
fail:
    stat = NC_EINVAL;
    goto done;
}

static int
objdump(void)
{
    int stat = NC_NOERR;
    int i, ncid, grpid;
    size_t len;
    char* name = NULL;
    NClist* stack = nclistnew(); /* stack for depth first walk */
    int ngrps;
    int* subgrps = NULL;

    NCCHECK(nc_open(options.infile,NC_NETCDF4,&ncid));

    /* Depth first walk all the groups */
    nclistpush(stack,(void*)(uintptr_t)ncid);
    while(nclistlength(stack) > 0) {
	grpid = (uintptr_t)nclistget(stack,0); /* zero pos is always top of stack */
	nclistremove(stack,0); /* pop stack */
	NCCHECK(nc_inq_grpname_full(grpid,&len,NULL));
	if((name = malloc(len+1)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	NCCHECK(nc_inq_grpname_full(grpid,&len,name));
	name[len] = '\0';
	printf("%s",name);
	if(options.typetag) {
	    int varid;
	    if((stat=nc_get_att_text(grpid,NC_GLOBAL,DATANAME,NULL)) == NC_NOERR) {
		printf("(M)");
	    } else if((stat=nc_inq_varid(grpid,DATANAME,&varid)) == NC_NOERR) {
		printf("(D)");
	    }
	}
	printf("\n");
	nullfree(name); name = NULL; /* cleanup */
	/* Get subgroups of grpid and push onto the stack in proper order */
	NCCHECK(nc_inq_grps(grpid,&ngrps,NULL));
        if(ngrps > 0) {
	    if((subgrps = malloc(sizeof(int)*ngrps)) == NULL)
		{stat = NC_ENOMEM; goto done;}
	    NCCHECK(nc_inq_grps(grpid,&ngrps,subgrps));
	    for(i=(ngrps-1);i>=0;i--) {
		nclistinsert(stack,0,(void*)(uintptr_t)subgrps[i]); /* push in order */
	    }
	}        
    }

done:
    return stat;
}
