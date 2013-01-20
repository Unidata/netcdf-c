/*********************************************************************
 *   Copyright 2011, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include <assert.h>
#include <ctype.h>
#include "utils.h"

/*
 * Print error message to stderr and exit
 */
void
error(const char *fmt, ...)
{
    va_list args ;

    (void) fprintf(stderr,"%s: ", progname);
    va_start(args, fmt) ;
    (void) vfprintf(stderr,fmt,args) ;
    va_end(args) ;

    (void) fprintf(stderr, "\n") ;
    (void) fflush(stderr);	/* to ensure log files are current */
    exit(EXIT_FAILURE);
}

void *
emalloc (			/* check return from malloc */
	size_t size)
{
    void   *p;

    p = (void *) malloc (size==0 ? 1 : size); /* malloc(0) not portable */
    if (p == 0) {
	error ("out of memory\n");
    }
    return p;
}


void
check(int err, const char* file, const int line)
{
    fprintf(stderr,"%s\n",nc_strerror(err));
    fprintf(stderr,"Location: file %s; line %d\n", file,line);
    fflush(stderr); fflush(stdout);
    exit(1);
}


/* 
 * Returns malloced name with chars special to CDL escaped.
 * Caller should free result when done with it.
 */
char*
escaped_name(const char* cp) {
    char *ret;			/* string returned */
    char *sp;
    assert(cp != NULL);

    /* For some reason, and on some machines (e.g. tweety)
       utf8 characters such as \343 are considered control character. */
/*    if(*cp && (isspace(*cp) | iscntrl(*cp)))*/
    if((*cp >= 0x01 && *cp <= 0x20) || (*cp == 0x7f))
    {
	error("name begins with space or control-character: %c",*cp);
    }

    ret = emalloc(4*strlen(cp) + 1); /* max if every char escaped */
    sp = ret;
    *sp = 0;			    /* empty name OK */
    /* Special case: leading number allowed, but we must escape it for CDL */
    if((*cp >= '0' && *cp <= '9'))
    {
	*sp++ = '\\';
    }
    for (; *cp; cp++) {
	if (isascii(*cp)) {
	    if(iscntrl(*cp)) {	/* render control chars as two hex digits, \%xx */
		snprintf(sp, 4,"\\%%%.2x", *cp);
		sp += 4;
	    } else {
		switch (*cp) {
		case ' ':
		case '!':
		case '"':
		case '#':
		case '$':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case ',':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '[':
		case ']':
		case '\\':
		case '^':
		case '`':
		case '{':
		case '|':
		case '}':
		case '~':
		    *sp++ = '\\';
		    *sp++ = *cp;
		    break;
		default:		/* includes '/' */
		    *sp++ = *cp;
		    break;
		}
	    }
	} else { 		/* not ascii, assume just UTF-8 byte */
	    *sp++ = *cp;
	}
    }
    *sp = 0;
    return ret;
}


/* 
 * Print name with escapes for special characters
 */
void
print_name(const char* name) {
    char *ename = escaped_name(name);
    fputs(ename, stdout);
    free(ename);
}

/* Missing functionality that should be in nc_inq_dimid(), to get
 * dimid from a full dimension path name that may include group
 * names */
int 
nc_inq_dimid2(int ncid, const char *dimname, int *dimidp) {
    int ret = NC_NOERR;
    /* If '/' doesn't occur in dimname, just return id found by
     * nc_inq_dimid() */
    char *sp = strrchr(dimname, '/');
    if(!sp) { /* No '/' in dimname, so return nc_inq_dimid() result */
	ret = nc_inq_dimid(ncid, dimname, dimidp);
    } 
#ifdef USE_NETCDF4
    else {  /* Parse group name out and get dimid using that */
	size_t grp_namelen = sp - dimname;
	char *grpname = emalloc(grp_namelen + 1);
	int grpid;
	strncpy(grpname, dimname, grp_namelen);
	grpname[grp_namelen] = '\0';
	ret = nc_inq_grp_full_ncid(ncid, grpname, &grpid);
	if(ret == NC_NOERR) {
	    ret = nc_inq_dimid(grpid, dimname, dimidp);
	}
	free(grpname);
    }	
#endif	/* USE_NETCDF4 */
    return ret;
}


/*
 * return 1 if varid identifies a record variable
 * else return 0
 */
int
isrecvar(int ncid, int varid)
{
    int ndims;
    int is_recvar = 0;
    int *dimids;

    NC_CHECK( nc_inq_varndims(ncid, varid, &ndims) );
#ifdef USE_NETCDF4
    if (ndims > 0) {
	int nunlimdims;
	int *recdimids;
	int dim, recdim;
	dimids = (int *) emalloc((ndims + 1) * sizeof(int));
	NC_CHECK( nc_inq_vardimid(ncid, varid, dimids) );
	NC_CHECK( nc_inq_unlimdims(ncid, &nunlimdims, NULL) );
	recdimids = (int *) emalloc((nunlimdims + 1) * sizeof(int));
	NC_CHECK( nc_inq_unlimdims(ncid, NULL, recdimids) );
	for (dim = 0; dim < ndims && is_recvar == 0; dim++) {
	    for(recdim = 0; recdim < nunlimdims; recdim++) {
		if(dimids[dim] == recdimids[recdim]) {
		    is_recvar = 1;
		    break;
		}		
	    }
	}
	free(dimids);
	free(recdimids);
    }
#else
    if (ndims > 0) {
	int recdimid;
	dimids = (int *) emalloc((ndims + 1) * sizeof(int));
	NC_CHECK( nc_inq_vardimid(ncid, varid, dimids) );
	NC_CHECK( nc_inq_unlimdim(ncid, &recdimid) );
	if(dimids[0] == recdimid)
	    is_recvar = 1;
	free(dimids);
    }
#endif /* USE_NETCDF4 */
    return is_recvar;
}

static idnode_t*
newidnode(void) {
    idnode_t *newvp = (idnode_t*) emalloc(sizeof(idnode_t));
    return newvp;
}

/*
 * Get a new, empty variable list.
 */
idnode_t*
newidlist(void) {
    idnode_t *vp = newidnode();

    vp -> next = 0;
    vp -> id = -1;		/* bad id */

    return vp;
}

void
idadd(idnode_t* vlist, int varid) {
    idnode_t *newvp = newidnode();
    
    newvp -> next = vlist -> next;
    newvp -> id = varid;
    vlist -> next = newvp;
}

/* 
 * return true if id is member of list that idlist points to.
 */
bool_t
idmember(const idnode_t* idlist, int id)
{
    idnode_t *vp = idlist -> next;

    for (; vp ; vp = vp->next)
      if (vp->id == id)
	return true;
    return false;    
}

/* 
 * Return true if group identified by grpid is member of grpids, a list of groups.
 * nlgrps is number of groups in the list.
 */
bool_t
group_wanted(int grpid, int nlgrps, const idnode_t* grpids)
{
    /* If -g not specified, all groups are wanted */
    if(nlgrps == 0) return true;
    /* if -g specified, look for match in group id list */
    return idmember(grpids, grpid);
}

/* Determine whether a group named formatting_specs.lgrps[igrp] exists
 * in a netCDF file or group with id ncid.  If so, return the count of
 * how many matching groups were found, else return a count of 0.  If
 * the name begins with "/", it is interpreted as an absolute group
 * name, in which case only 0 or 1 is returned.  Otherwise, interpret
 * it as a relative name, and the total number of occurrences within
 * the file/group identified by ncid is returned.  
 *
 * Also has side effect of updating the ngrpids and the associate
 * grpids array that represent the group list specified by the -g
 * option.  TODO: put this in its own function instead.
 */
static size_t
nc_inq_grpname_count(int ncid, int igrp, char **lgrps, idnode_t *grpids) {
    size_t count = 0;
#ifdef USE_NETCDF4
    int numgrps;
    int *ncids;
    int g;
    int grpid;
    int status;
#endif
    char *grpname = lgrps[igrp];

    /* permit empty string to also designate root group */
    if(grpname[0] == '\0' || STREQ(grpname,"/")) { 
	count = 1;
	idadd(grpids, ncid);
	return count;
    }
#ifdef USE_NETCDF4
    /* Handle absolute group names */
    if(grpname[0] == '/') {
	int grpid;
	status = nc_inq_grp_full_ncid(ncid, grpname, &grpid);
	if(status == NC_NOERR) {
	    count = 1;
	    idadd(grpids, grpid);
	} else if(status == NC_ENOGRP) {
	    count = 0;
	} else {
	    error("when looking up group %s: %s ", grpname, nc_strerror(status));
	}
	return count;
    }
    
    /* look in this group */
    status = nc_inq_grp_ncid(ncid, grpname, &grpid);
    if (status == NC_NOERR) {
	count++;
	idadd(grpids, grpid);
    }
    /* if this group has subgroups, call recursively on each of them */
    NC_CHECK( nc_inq_grps(ncid, &numgrps, NULL) );
    if(numgrps > 0) {
	/* Allocate memory to hold the list of group ids. */
	ncids = emalloc(numgrps * sizeof(int));
	/* Get the list of group ids. */
	NC_CHECK( nc_inq_grps(ncid, NULL, ncids) );
	/* Call this function recursively for each group. */
	for (g = 0; g < numgrps; g++) {
	    count += nc_inq_grpname_count(ncids[g], igrp, lgrps, grpids);
	}
	free(ncids);
    }
#endif /* USE_NETCDF4 */
    return count;    
}

/* Check if any group names specified with "-g grp1,...,grpn" are
 * missing.  Returns total number of matching groups if no missing
 * groups detected, otherwise exits. */
int
grp_matches(int ncid, int nlgrps, char** lgrps, idnode_t *grpids) {
    int ig;
    size_t total = 0;

    for (ig=0; ig < nlgrps; ig++) {
	size_t count = nc_inq_grpname_count(ncid, ig, lgrps, grpids);
	if(count == 0) {
	    error("%s: No such group", lgrps[ig]);
	    return 0;
	}
	total += count;
    }
    return total;
}

/* Returns 1 if string s1 ends with string s2, 0 otherwise. */
int
strendswith(const char *s1, const char *s2) {
    size_t m1 = strlen(s1);
    size_t m2 = strlen(s2);
    if (m1 < m2)
	return 0;
    return (strcmp(s1 + (m1 - m2), s2) == 0);
}
