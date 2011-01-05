/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id $
 *********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netcdf.h>
#include "utils.h"
#include "chunkspec.h"

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
 * Parse chunkspec string and convert into chunkspec_t structure.
 *   ncid: location ID of open netCDF file or group in an open file
 *   spec: string of form 
 *           dim1/n1,dim2/n2,...,dimk/nk
 *
 *         specifying chunk size (ni) to be used for dimension named
 *         dimi.  Dimension names may be absolute,
 *         e.g. "/grp_a/grp_a1/dim".  The "ni" part of the spec may be
 *         omitted, in which case it is assumed to be the entire
 *         dimension size.  That is also the default for dimensions
 *         not mentioned in the string.
 *
 *   chunkspec: Structure mapping dimension IDs to corresponding chunksizes.
 *              Allocated by function.  Free by invoking chunkspec_free().
 *
 * Returns NC_NOERR if no error, NC_EINVAL if spec has consecutive
 * unescaped commas or no chunksize specified for dimension.
 */
int
chunkspec_parse(int ncid, const char *spec, chunkspec_t *csp) {
    const char *cp;	   /* character cursor */
    const char *pp = spec; /* previous char cursor for detecting escapes */
    const char *np;	   /* beginning of current dimension name */
    size_t ndims = 1;
    int idim = 0;
    int ret;

    csp = (chunkspec_t *) emalloc(sizeof(chunkspec_t));
    if (!spec || *spec == '\0')
	return NC_NOERR; 
    /* Count unescaped commas, handle consecutive unescaped commas as error */
    for(cp = spec; *cp; cp++) {
	int comma_seen = 0;
	if(*cp == ',' && *pp != '\\') {
	    if(comma_seen) {	/* consecutive commas detected */
		return(NC_EINVAL);
	    }
	    comma_seen = 1;
	    ndims++;
	} else {
	    comma_seen = 0;
	}
	pp = cp;
    }
    csp->ndims = ndims;
    csp->dimids = (int *) emalloc(ndims * sizeof(int));
    csp->chunksizes = (size_t *) emalloc(ndims * sizeof(size_t));
    /* Look up dimension ids and assign chunksizes */
    cp = spec;
    pp = spec;
    np = spec;
    for(cp = spec; *cp; cp++) {
	if(*cp == '\0' || (*cp == ',' && *pp != '\\')) { /* found end of "dim/nn" part */
	    char* dimname = 0;
	    char *dp;
	    int dimid;
	    size_t chunkmax, chunksize;

	    for(; pp > np && *pp != '/'; pp--) { /* look backwards for "/" */
		continue;
	    }
	    if(*pp != '/') {	/* no '/' found, no chunksize specified for dimension */
		return(NC_EINVAL);
	    }
	    /* extract dimension name */
	    dimname = (char *) emalloc(pp - np + 1);
	    dp = dimname;
	    while(np < pp) {
		*dp++ = *np++;
	    }
	    *dp = '\0';
	    /* look up dimension id from dimension pathname */
	    ret = nc_inq_dimid2(ncid, dimname, &dimid);
	    if(ret != NC_NOERR)
		break;
	    csp->dimids[idim] = dimid;
	    /* parse and assign corresponding chunksize */
	    pp++; /* now points to first digit of chunksize, ',', or '\0' */
	    /* max chunk size is length of dimension */
	    ret = nc_inq_dimlen(ncid, dimid, &chunkmax);
	    if(ret != NC_NOERR)
		return(ret);
	    if(*pp == ',' || *pp == '\0') { /* no size specified, use dim len */
		chunksize = chunkmax;
	    } else {	      /* convert nnn string to long integer */
		char *ep;
		long val = strtol(pp, &ep, 0);
		if(ep == pp || errno == ERANGE || val < 1 || val > chunkmax)
		    return (NC_EINVAL);
		chunksize = val;
	    }
	    csp->chunksizes[idim] = chunksize;
	    /* set np to point to first char after comma */
	    if(*cp)
		np = cp + 1;
	    if(dimname)
		free(dimname);
	}
	pp = cp;
    };
    return NC_NOERR;
}

void
chunkspec_free(chunkspec_t *csp) {
    if(csp->dimids)
	free(csp->dimids);
    if(csp->chunksizes)
	free(csp->chunksizes);
}

