/*********************************************************************
 *   Copyright 2010, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   "$Id"
 *********************************************************************/

#include "config.h"		/* for USE_NETCDF4 macro */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netcdf.h>
#include "ncgiter.h"

#define CHECK(stat,f) if(stat != NC_NOERR) {check(stat,#f,__FILE__,__LINE__);} else {}

static void
check(int err, const char* fcn, const char* file, const int line)
{
    fprintf(stderr,"%s\n",nc_strerror(err));
    fprintf(stderr,"Location: function %s; file %s; line %d\n",
	    fcn,file,line);
    fflush(stderr); fflush(stdout);
    exit(1);
}

/* Check error return from malloc, and allow malloc(0) with subsequent free */
static void *
emalloc (size_t size)
{
    void   *p;

    p = (void *) malloc (size==0 ? 1 : size); /* don't malloc(0) */
    if (p == 0) {
	fprintf(stderr,"Out of memory\n");
	exit(1);
    }
    return p;
}

/* initialize and return a new empty stack */
static ncgiter_t *
gs_init() {
    ncgiter_t *s = emalloc(sizeof(ncgiter_t));
    s->ngrps = 0;
    s->top = NULL;
    return s;
}

/* free a stack and all its nodes */
static void
gs_free(ncgiter_t *s) {
    grpnode_t *n0, *n1;
    n0 = s->top;
    while (n0) {
	n1 = n0->next;
	free(n0);
	n0 = n1;
    }
    free(s);
}

/* test if a stack is empty */
static int
gs_empty(ncgiter_t *s)
{
    return s->ngrps == 0;
}

/* push a grpid on stack */
static void
gs_push(ncgiter_t *s, int grpid)
{
    grpnode_t *node = emalloc(sizeof(grpnode_t));
 
    node->grpid = grpid;
    node->next = gs_empty(s) ? NULL : s->top;
    s->top = node;
    s->ngrps++;
}

/* pop value off stack and return */
static int 
gs_pop(ncgiter_t *s)
{
    if (gs_empty(s)) {
	return -1;		/* underflow, stack is empty */
    } else {			/* pop a node */
	grpnode_t *top = s->top;
	int value = top->grpid;
	s->top = top->next;
	/* TODO: first call to free gets seg fault with libumem */
	free(top);
	s->ngrps--;
	return value;
    }
}

/* return top value on stack without popping stack */
static int 
gs_top(ncgiter_t *s)
{
    if (gs_empty(s)) {
	return -1;		/* underflow, stack is empty */
    } else {			/* get top value */
	grpnode_t *top = s->top;
	int value = top->grpid;
	return value;
    }
}

/* Begin public interface */

/* Initialize group iterator for start group and all its descendant
 * groups. */
int
nc_get_giter(int grpid,	       /* start group id */
	    ncgiter_t **iterp  /* returned opaque iteration state */
    ) 
{
    int stat = NC_NOERR;

    stat = nc_inq_grpname(grpid, NULL); /* check if grpid is valid */
    if(stat != NC_EBADGRPID && stat != NC_EBADID) {
	*iterp = gs_init();
	gs_push(*iterp, grpid);
    }

    return stat;
}

/* 
 * Get group id of next group.  On first call gets start group id,
 * subsequently returns other subgroup ids in preorder.  Returns zero
 * when no more groups left.
 */
int
nc_next_giter(ncgiter_t *iterp, int *grpidp) {
    int stat = NC_NOERR;
    int numgrps;
    int *grpids;
    int i;

    if(gs_empty(iterp)) {
	*grpidp = 0;		/* not a group, signals iterator is done */
    } else {
	*grpidp = gs_pop(iterp);
	stat = nc_inq_grps(*grpidp, &numgrps, NULL);
	CHECK(stat, nc_inq_grps);
	if(numgrps > 0) {
	    grpids = (int *)emalloc(sizeof(int) * numgrps);
	    stat = nc_inq_grps(*grpidp, &numgrps, grpids);
	    CHECK(stat, nc_inq_grps);
	    for(i = numgrps - 1; i >= 0; i--) { /* push ids on stack in reverse order */
		gs_push(iterp, grpids[i]);
	    }
	    free(grpids);
	}
    }
    return stat;
}

/*
 * Release group iter.
 */
void
nc_rel_giter(ncgiter_t *iterp)
{
    gs_free(iterp);
}

/* 
 * Get total number of groups (including the top-level group and all
 * descendant groups, recursively) and all descendant subgroup ids
 * (including the input rootid of the start group) for a group and
 * all its descendants, in preorder.
 *
 * If grpids or numgrps is NULL, it will be ignored.  So typical use
 * is to call with grpids NULL to get numgrps, allocate enough space
 * for the group ids, then call again to get them.
 */
int
nc_inq_grps_full(int rootid, int *numgrps, int *grpids) 
{
    int stat = NC_NOERR;
    ncgiter_t *giter;		/* pointer to group iterator */
    int grpid;
    size_t count;

    stat = nc_get_giter(rootid, &giter);
    CHECK(stat, nc_get_giter);
    
    count = 0;
    stat = nc_next_giter(giter, &grpid);
    CHECK(stat, nc_next_giter);
    while(grpid != 0) {
	if(grpids)
	    grpids[count] = grpid;
	count++;
	stat = nc_next_giter(giter, &grpid);
	CHECK(stat, nc_next_iter);
    }
    if(numgrps)
	*numgrps = count;
    nc_rel_giter(giter);
    return stat;
}

#ifdef TEST_GITER_MAIN
/* Test on input file by printing all group names in iteration order */
int 
main(int argc, char *argv[])
{
    int stat = NC_NOERR;
    char* infile = NULL;
    int ncid;
    ncgiter_t *giter;		/* pointer to group iterator */
    int grpid;
    size_t namelen;
    char grpname[NC_MAX_NAME];

    infile = argv[1];

    stat = nc_open(infile, NC_NOWRITE, &ncid);
    CHECK(stat, nc_open);

    stat = nc_get_giter(ncid, &giter);
    CHECK(stat, nc_get_giter);

    stat = nc_next_giter(giter, &grpid);
    CHECK(stat, nc_next_giter);
    while(grpid != 0) {
	/* get group name from group id */
	stat = nc_inq_grpname(grpid, grpname);
	CHECK(stat, nc_get_iter);
	printf("%d %s\n", grpid, grpname);
	stat = nc_next_giter(giter, &grpid);
	CHECK(stat, nc_next_iter);
    }
    
    nc_rel_giter(giter);

    /* Now try the simpler API */
    {
	size_t ngrps0, ngrps;
	int *grpids;
	int i;

	printf("Same thing with simpler API\n");
	stat = nc_inq_grps_full(ncid, &ngrps0, NULL);
	CHECK(stat, nc_inq_grps_full);
	grpids = emalloc(ngrps0 * sizeof(int));
	stat = nc_inq_grps_full(ncid, &ngrps, grpids);
	CHECK(stat, nc_inq_grps_full);
	assert(ngrps0 == ngrps);
	for(i = 0; i < ngrps; i++) {
	    /* get group name from group id */
	    stat = nc_inq_grpname(grpids[i], grpname);
	    CHECK(stat, nc_inq_grpname);
	    printf("%d %s\n", grpids[i], grpname);
	}
    }

    return 0;
}
#endif	/* TEST_GITER_MAIN */
