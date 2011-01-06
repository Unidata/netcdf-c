/*********************************************************************
 *   Copyright 2010, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   Thanks to Philippe Poilbarbe and Antonio S. Cofiño for 
 *   compression additions.
 *   $Id: nccopy.c 400 2010-08-27 21:02:52Z russ $
 *********************************************************************/

#include "config.h"		/* for USE_NETCDF4 macro */
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <netcdf.h>
#include "nciter.h"
#include "chunkspec.h"
#include "utils.h"

/* default bytes of memory we are willing to allocate for variable
 * values during copy */
#define COPY_BUFFER_SIZE (5000000)
#define SAME_AS_INPUT (-1)	/* default, if kind not specified */

#ifndef USE_NETCDF4
#define NC_CLASSIC_MODEL 0x0100 /* Enforce classic model if netCDF-4 not available. */
#endif

/* These are in unistd.h; for use with getopt() */
extern int optind;
extern int opterr;
extern char *optarg;

/* Global variables for command-line requests */
char *progname;	       /* for error messages */
static int option_deflate_level = -1;	/* default, compress output only if input compressed */
static int option_shuffle_vars = NC_NOSHUFFLE; /* default, no shuffling on compression */
static int option_fix_unlimdims = 0; /* default, preserve unlimited dimensions */

/* get group id in output corresponding to group igrp in input,
 * given parent group id (or root group id) parid in output. */
static int
get_grpid(int igrp, int parid, int *ogrpp) {
    int stat = NC_NOERR;
    int ogid;			/* like igrp but in output file */
#ifdef USE_NETCDF4
    int inparid;

    /* if not root group, get corresponding output groupid from group name */
    stat = nc_inq_grp_parent(igrp, &inparid);
    if(stat == NC_NOERR) {	/* not root group */
	char grpname[NC_MAX_NAME + 1];
	NC_CHECK(nc_inq_grpname(igrp, grpname));
	NC_CHECK(nc_inq_grp_ncid(parid, grpname, &ogid));
    } else if(stat == NC_ENOGRP) { /* root group */
	ogid = parid;
	stat = NC_NOERR;
    } else {
	NC_CHECK(stat);
    }
#else
    ogid = parid;
#endif	/* USE_NETCDF4 */
    *ogrpp = ogid;
    return stat;
}


#ifdef USE_NETCDF4
/* Get parent id needed to define a new group from its full name in an
 * open file identified by ncid.  Assumes all intermediate groups are
 * already defined.  */
static int
nc_inq_parid(int ncid, const char *fullname, int *locidp) {
    int stat = NC_NOERR;
    char *parent = strdup(fullname);
    char *slash = "/";		/* groupname separator */
    char *last_slash;
    if(parent == NULL) {
	NC_CHECK(NC_ENOMEM);
    }
    last_slash = strrchr(parent, '/');
    if(last_slash == parent) {	/* parent is root */
	free(parent);
	parent = strdup(slash);
    } else {
	*last_slash = '\0';	/* truncate to get parent name */
    }
    NC_CHECK(nc_inq_grp_full_ncid(ncid, parent, locidp));
       free(parent);
    return stat;
}

/* Return size of chunk in bytes for a variable varid in a group igrp, or 0 if
 * layout is contiguous */
static int
inq_var_chunksize(int igrp, int varid, size_t* chunksizep) {
    int stat = NC_NOERR;
    int ndims;
    size_t *chunksizes;
    int dim;
    int contig = 1;
    nc_type vartype;
    size_t value_size;
    size_t prod;

    NC_CHECK(nc_inq_vartype(igrp, varid, &vartype));
    /* from type, get size in memory needed for each value */
    NC_CHECK(nc_inq_type(igrp, vartype, NULL, &value_size));
    prod = value_size;
    NC_CHECK(nc_inq_varndims(igrp, varid, &ndims));
    chunksizes = (size_t *) emalloc((ndims + 1) * sizeof(size_t));
    if(ndims > 0) {
	NC_CHECK(nc_inq_var_chunking(igrp, varid, &contig, NULL));
    }
    if(contig == 1) {
	*chunksizep = 0;
    } else {
	NC_CHECK(nc_inq_var_chunking(igrp, varid, &contig, chunksizes));
	for(dim = 0; dim < ndims; dim++) {
	    prod *= chunksizes[dim];
	}
	*chunksizep = prod;
    }
    free(chunksizes);
    return stat;
}

/* Forward declaration, because copy_type, copy_vlen_type call each other */
static int copy_type(int igrp, nc_type typeid, int ogrp);

/* 
 * copy a user-defined variable length type in the group igrp to the
 * group ogrp
 */
static int
copy_vlen_type(int igrp, nc_type itype, int ogrp)
{
    int stat = NC_NOERR; 
    nc_type ibasetype;
    nc_type obasetype;		/* base type in target group */
    char name[NC_MAX_NAME];
    size_t size;
    char basename[NC_MAX_NAME];
    size_t basesize;
    nc_type vlen_type;

    NC_CHECK(nc_inq_vlen(igrp, itype, name, &size, &ibasetype));
    /* to get base type id in target group, use name of base type in
     * source group */
    NC_CHECK(nc_inq_type(igrp, ibasetype, basename, &basesize));
    stat = nc_inq_typeid(ogrp, basename, &obasetype);
    /* if no such type, create it now */
    if(stat == NC_EBADTYPE) {
	NC_CHECK(copy_type(igrp, ibasetype, ogrp));
	stat = nc_inq_typeid(ogrp, basename, &obasetype);
    }
    NC_CHECK(stat);

    /* Now we know base type exists in output and we know its type id */
    NC_CHECK(nc_def_vlen(ogrp, name, obasetype, &vlen_type));

    return stat;
}

/* 
 * copy a user-defined opaque type in the group igrp to the group ogrp
 */
static int
copy_opaque_type(int igrp, nc_type itype, int ogrp)
{
    int stat = NC_NOERR; 
    nc_type otype;
    char name[NC_MAX_NAME];
    size_t size;

    NC_CHECK(nc_inq_opaque(igrp, itype, name, &size));
    NC_CHECK(nc_def_opaque(ogrp, size, name, &otype));

    return stat;
}

/* 
 * copy a user-defined enum type in the group igrp to the group ogrp
 */
static int
copy_enum_type(int igrp, nc_type itype, int ogrp)
{
    int stat = NC_NOERR; 
    nc_type otype;
    nc_type basetype;
    size_t basesize;
    size_t nmembers;
    char name[NC_MAX_NAME];
    int i;

    NC_CHECK(nc_inq_enum(igrp, itype, name, &basetype, &basesize, &nmembers));
    NC_CHECK(nc_def_enum(ogrp, basetype, name, &otype));
    for(i = 0; i < nmembers; i++) { /* insert enum members */
	char ename[NC_MAX_NAME];
	long long val;		/* large enough to hold any integer type */
	NC_CHECK(nc_inq_enum_member(igrp, itype, i, ename, &val));
	NC_CHECK(nc_insert_enum(ogrp, otype, ename, &val));
    }
    return stat;
}

/* 
 * copy a user-defined compound type in the group igrp to the group ogrp
 */
static int
copy_compound_type(int igrp, nc_type itype, int ogrp)
{
    int stat = NC_NOERR; 
    char name[NC_MAX_NAME];
    size_t size;
    size_t nfields;
    nc_type otype;
    int fid;

    NC_CHECK(nc_inq_compound(igrp, itype, name, &size, &nfields));
    NC_CHECK(nc_def_compound(ogrp, size, name, &otype));

    for (fid = 0; fid < nfields; fid++) {
	char fname[NC_MAX_NAME];
	char ftypename[NC_MAX_NAME];
	size_t foff;
	nc_type iftype, oftype;
	int fndims;

	NC_CHECK(nc_inq_compound_field(igrp, itype, fid, fname, &foff, &iftype, &fndims, NULL));
	/* type ids in source don't necessarily correspond to same
	 * typeids in destination, so look up destination typeid by using
	 * field type name */
	NC_CHECK(nc_inq_type(igrp, iftype, ftypename, NULL));
	NC_CHECK(nc_inq_typeid(ogrp, ftypename, &oftype));
	if(fndims == 0) {
	    NC_CHECK(nc_insert_compound(ogrp, otype, fname, foff, oftype));
	} else {		/* field is array type */
	    int *fdimsizes;
	    fdimsizes = (int *) emalloc((fndims + 1) * sizeof(int));
	    stat = nc_inq_compound_field(igrp, itype, fid, NULL, NULL, NULL, 
					 NULL, fdimsizes);
	    NC_CHECK(nc_insert_array_compound(ogrp, otype, fname, foff, oftype, fndims, fdimsizes));
	    free(fdimsizes);
	}
    }
    return stat;
}


/* 
 * copy a user-defined type in the group igrp to the group ogrp
 */
static int
copy_type(int igrp, nc_type typeid, int ogrp)
{
    int stat = NC_NOERR; 
    nc_type type_class;

    NC_CHECK(nc_inq_user_type(igrp, typeid, NULL, NULL, NULL, NULL, &type_class));

    switch(type_class) {
    case NC_VLEN:
	NC_CHECK(copy_vlen_type(igrp, typeid, ogrp));
	break;
    case NC_OPAQUE:
	NC_CHECK(copy_opaque_type(igrp, typeid, ogrp));
	break;
    case NC_ENUM:
	NC_CHECK(copy_enum_type(igrp, typeid, ogrp));
	break;
    case NC_COMPOUND:
	NC_CHECK(copy_compound_type(igrp, typeid, ogrp));
	break;
    default:
	NC_CHECK(NC_EBADTYPE);
    }
    return stat;
}

/* Copy a group and all its subgroups, recursively, from iroot to
 * oroot, the ncids of input file and output file.  This just creates
 * all the groups in the destination, but doesn't copy anything that's
 * in the groups yet. */
static int
copy_groups(int iroot, int oroot)
{
    int stat = NC_NOERR;
    int numgrps;
    int *grpids;
    int i;

    /* get total number of groups and their ids, including all descendants */
    NC_CHECK(nc_inq_grps_full(iroot, &numgrps, NULL));
    grpids = emalloc(numgrps * sizeof(int));
    NC_CHECK(nc_inq_grps_full(iroot, NULL, grpids));
    /* create corresponding new groups in ogrp, except for root group */
    for(i = 1; i < numgrps; i++) {
	char *grpname_full;
	char grpname[NC_MAX_NAME];
	size_t len_name;
	int ogid, oparid;
	/* get full group name of input group */
	NC_CHECK(nc_inq_grpname_full(grpids[i], &len_name, NULL));
	grpname_full = emalloc(len_name + 1);
	NC_CHECK(nc_inq_grpname_full(grpids[i], &len_name, grpname_full));
	/* get id of parent group of corresponding group in output.
	 * Note that this exists, because nc_inq_groups returned
	 * grpids in preorder, so parents are always copied before
	 * their subgroups */
	NC_CHECK(nc_inq_parid(oroot, grpname_full, &oparid));
	NC_CHECK(nc_inq_grpname(grpids[i], grpname));
	/* define corresponding group in output */
	NC_CHECK(nc_def_grp(oparid, grpname, &ogid));
	free(grpname_full);
    }
    free(grpids);
    return stat;    
}

/* 
 * Copy the user-defined types in this group (igrp) and all its
 * subgroups, recursively, to corresponding group in output (ogrp)
 */
static int
copy_types(int igrp, int ogrp)
{
    int stat = NC_NOERR; 
    int ntypes;
    nc_type *types = NULL;
    int numgrps;
    int *grpids = NULL;
    int i;

    NC_CHECK(nc_inq_typeids(igrp, &ntypes, NULL));

    if(ntypes > 0) {
	types = (nc_type *) emalloc(ntypes * sizeof(nc_type));
	NC_CHECK(nc_inq_typeids(igrp, &ntypes, types));
	for (i = 0; i < ntypes; i++) {
	    NC_CHECK(copy_type(igrp, types[i], ogrp));
	}
	free(types);
    }

    /* Copy types from subgroups */
    NC_CHECK(nc_inq_grps(igrp, &numgrps, NULL));
    if(numgrps > 0) {
	grpids = (int *)emalloc(sizeof(int) * numgrps);
	NC_CHECK(nc_inq_grps(igrp, &numgrps, grpids));
	for(i = 0; i < numgrps; i++) {
	    int ogid;
	    /* get groupid in output corresponding to grpids[i] in
	     * input, given parent group (or root group) ogrp in
	     * output */
	    NC_CHECK(get_grpid(grpids[i], ogrp, &ogid));
	    NC_CHECK(copy_types(grpids[i], ogid));
	}
	free(grpids);
    }
    return stat;
}

/* Copy all netCDF-4 specific variable properties such as chunking,
 * endianness, deflation, checksumming, fill, etc. */
static int
copy_var_specials(int igrp, int varid, int ogrp, int o_varid)
{
    int stat = NC_NOERR;
    {				/* handle chunking parameters */
	int ndims;
	NC_CHECK(nc_inq_varndims(igrp, varid, &ndims));
	if (ndims > 0) {		/* no chunking for scalar variables */
	    int contig = 0;
	    NC_CHECK(nc_inq_var_chunking(igrp, varid, &contig, NULL));
	    if(contig == 1) {
		NC_CHECK(nc_def_var_chunking(ogrp, o_varid, NC_CONTIGUOUS, NULL));
	    } else {
		size_t *chunkp = (size_t *) emalloc(ndims * sizeof(size_t));
		int *dimids = (int *) emalloc(ndims * sizeof(int));
		int idim;
		NC_CHECK(nc_inq_var_chunking(igrp, varid, NULL, chunkp));
		NC_CHECK(nc_inq_vardimid(igrp, varid, dimids));
		for(idim = 0; idim < ndims; idim++) {
		    int dimid = dimids[idim];
		    size_t chunksize = chunkspec_size(dimid);
		    if(chunkspec_size(dimid) > 0) { /* found in chunkspec */
			chunkp[idim] = chunksize;
		    }
		}
		/* explicitly set chunking, even if default */
		NC_CHECK(nc_def_var_chunking(ogrp, o_varid, NC_CHUNKED, chunkp));
		free(dimids);
		free(chunkp);
	    }
	}
    }
    {				/* handle compression parameters */
	int shuffle, deflate, deflate_level;
	NC_CHECK(nc_inq_var_deflate(igrp, varid, &shuffle, &deflate, &deflate_level));
	if(option_deflate_level >= 0) { /* change output compression, if requested */
	  deflate	= (option_deflate_level > 0);
	  deflate_level = option_deflate_level;
	}
	if(deflate != 0 || shuffle != 0) {
	    NC_CHECK(nc_def_var_deflate(ogrp, o_varid, shuffle, deflate, deflate_level));
	}
    }
    {				/* handle checksum parameters */
	int fletcher32 = 0;
	NC_CHECK(nc_inq_var_fletcher32(igrp, varid, &fletcher32));
	if(fletcher32 != 0) {
	    NC_CHECK(nc_def_var_fletcher32(ogrp, o_varid, fletcher32));
	}
    }
    {				/* handle endianness */
	int endianness = 0;
	NC_CHECK(nc_inq_var_endian(igrp, varid, &endianness));
	if(endianness != NC_ENDIAN_NATIVE) { /* native is the default */
	    NC_CHECK(nc_def_var_endian(ogrp, o_varid, endianness));
	}
    }
    return stat;
}

/* set variable to compression specified on command line */
static int
set_var_compressed(int ogrp, int o_varid)
{
    int stat = NC_NOERR;
    if (option_deflate_level >= 0) {
	int deflate = 1;
	NC_CHECK(nc_def_var_deflate(ogrp, o_varid, option_shuffle_vars, deflate, option_deflate_level));
    }
    return stat;
}

/* Release the variable chunk cache allocated for variable varid in
 * group igrp  with it.  This is not necessary, but will save some
 * memory if processing one variable at a time.  */
static int
free_var_chunk_cache(int igrp, int varid)
{
    int stat = NC_NOERR;
    size_t chunk_cache_size = 1;
    size_t cache_nelems = 1;
    float cache_preemp = 0;
    int inkind;
    NC_CHECK(nc_inq_format(igrp, &inkind));
    if(inkind == NC_FORMAT_NETCDF4 || inkind == NC_FORMAT_NETCDF4_CLASSIC) {
	int contig = 1;
	NC_CHECK(nc_inq_var_chunking(igrp, varid, &contig, NULL));
	if(contig == 0) {	/* chunked */
	    NC_CHECK(nc_set_var_chunk_cache(igrp, varid, chunk_cache_size, cache_nelems, cache_preemp));
	}
    }
    return stat;
}
#endif /* USE_NETCDF4 */

/* Copy dimensions from group igrp to group ogrp, also fill in dimmap */
static int
copy_dims(int igrp, int ogrp, int *dimmap)
{
    int stat = NC_NOERR;
    int ndims;
    int nunlims;
    int dgrp;
#ifdef USE_NETCDF4
    int *dimids;
    int *unlimids;
#else
    int unlimid;
#endif /* USE_NETCDF4 */    

    NC_CHECK(nc_inq_ndims(igrp, &ndims));

#ifdef USE_NETCDF4
   /* In netCDF-4 files, dimids may not be sequential because they
    * may be defined in various groups, and we are only looking at one
    * group at a time. */
    /* Find the dimension ids in this group, don't include parents. */
    dimids = (int *) emalloc((ndims + 1) * sizeof(int));
    NC_CHECK(nc_inq_dimids(igrp, NULL, dimids, 0));
    /* Find the number of unlimited dimensions and get their IDs */
    NC_CHECK(nc_inq_unlimdims(igrp, &nunlims, NULL));
    unlimids = (int *) emalloc((nunlims + 1) * sizeof(int));
    NC_CHECK(nc_inq_unlimdims(igrp, NULL, unlimids));
#else
    NC_CHECK(nc_inq_unlimdim(igrp, &unlimid));
#endif /* USE_NETCDF4 */

    /* Copy each dimension to output, including unlimited dimension(s) */
    for (dgrp = 0; dgrp < ndims; dgrp++) {
	char name[NC_MAX_NAME];
	size_t length;
	int is_unlim;
	int uld;
	int idimid, odimid;

	is_unlim = 0;
#ifdef USE_NETCDF4
	idimid = dimids[dgrp];
	for (uld = 0; uld < nunlims; uld++) {
	    if(idimid == unlimids[uld]) {
		is_unlim = 1;
		break;
	    }	  
	}
#else
	idimid = dgrp;
	if(unlimid != -1 && (idimid == unlimid)) {
	    is_unlim = 1;
	}
#endif /* USE_NETCDF4 */

	stat = nc_inq_dim(igrp, idimid, name, &length);
	if (stat == NC_EDIMSIZE && sizeof(size_t) < 8) {
	    error("dimension \"%s\" requires 64-bit platform", name);
	}	
	NC_CHECK(stat);
	if(is_unlim && !option_fix_unlimdims) {
	    NC_CHECK(nc_def_dim(ogrp, name, NC_UNLIMITED, &odimid));
	} else {
	    NC_CHECK(nc_def_dim(ogrp, name, length, &odimid));
	}
	/* Store (idimid, odimid) mapping for later use */
	dimmap[idimid] = odimid;
    }
#ifdef USE_NETCDF4
    free(dimids);
    free(unlimids);
#endif /* USE_NETCDF4 */    
    return stat;
}

/* Copy the attributes for variable ivar in group igrp to variable
 * ovar in group ogrp.  Global (group) attributes are specified by
 * using the varid NC_GLOBAL */
static int
copy_atts(int igrp, int ivar, int ogrp, int ovar)
{
    int natts;
    int iatt;
    int stat = NC_NOERR;

    NC_CHECK(nc_inq_varnatts(igrp, ivar, &natts));
    
    for(iatt = 0; iatt < natts; iatt++) {
	char name[NC_MAX_NAME];
	NC_CHECK(nc_inq_attname(igrp, ivar, iatt, name));
	NC_CHECK(nc_copy_att(igrp, ivar, name, ogrp, ovar));
    }
    return stat;
}

/* copy the schema for a single variable in group igrp to group ogrp */
static int
copy_var(int igrp, int varid, int ogrp, int *dimmap)
{
    int stat = NC_NOERR;
    int ndims;
    int *idimids;		/* ids of dims for input variable */
    int *odimids;		/* ids of dims for output variable */
    char name[NC_MAX_NAME];
    nc_type typeid, o_typeid;
    int natts;
    int i;
    int o_varid;

    NC_CHECK(nc_inq_varndims(igrp, varid, &ndims));
    idimids = (int *) emalloc((ndims + 1) * sizeof(int));
    NC_CHECK(nc_inq_var(igrp, varid, name, &typeid, NULL, idimids, &natts));
    o_typeid = typeid;
#ifdef USE_NETCDF4
    if (typeid > NC_STRING) {	/* user-defined type */
	/* type ids in source don't necessarily correspond to same
	 * typeids in destination, so look up destination typeid by
	 * using type name */
	char type_name[NC_MAX_NAME];
	NC_CHECK(nc_inq_type(igrp, typeid, type_name, NULL));
	NC_CHECK(nc_inq_typeid(ogrp, type_name, &o_typeid));
    }
#endif	/* USE_NETCDF4 */

    /* get the corresponding dimids in the output file */
    odimids = (int *) emalloc((ndims + 1) * sizeof(int));
    for(i = 0; i < ndims; i++) {
	odimids[i] = dimmap[idimids[i]];
    }

    /* define the output variable */
    NC_CHECK(nc_def_var(ogrp, name, o_typeid, ndims, odimids, &o_varid));

    /* attach the variable attributes to the output variable */
    NC_CHECK(copy_atts(igrp, varid, ogrp, o_varid));
#ifdef USE_NETCDF4    
    {
	int inkind;
	int outkind;
	NC_CHECK(nc_inq_format(igrp, &inkind));
	NC_CHECK(nc_inq_format(ogrp, &outkind));
	if(outkind == NC_FORMAT_NETCDF4 || outkind == NC_FORMAT_NETCDF4_CLASSIC) {
	    if((inkind == NC_FORMAT_NETCDF4 || inkind == NC_FORMAT_NETCDF4_CLASSIC)) {
		/* Copy all netCDF-4 specific variable properties such as
		 * chunking, endianness, deflation, checksumming, fill, etc. */
		NC_CHECK(copy_var_specials(igrp, varid, ogrp, o_varid));
	    }
	    else {	     /* classic or 64-bit offset input file */
		/* Set compression if specified on command line option */
	    	NC_CHECK(set_var_compressed(ogrp, o_varid));
	    }
	}
    }
#endif	/* USE_NETCDF4 */
    free(idimids);
    free(odimids);
    return stat;
}

/* copy the schema for all the variables in group igrp to group ogrp */
static int
copy_vars(int igrp, int ogrp, int *dimmap)
{
    int stat = NC_NOERR;
    int nvars;
    int varid;
    
    NC_CHECK(nc_inq_nvars(igrp, &nvars));
    for (varid = 0; varid < nvars; varid++) {
	NC_CHECK(copy_var(igrp, varid, ogrp, dimmap));
    }
    return stat;
}

/* Copy the schema in a group and all its subgroups, recursively, from
 * group igrp in input to parent group ogrp in destination.  Use
 * dimmap array to map input dimids to output dimids. */
static int
copy_schema(int igrp, int ogrp, int *dimmap) 
{
    int stat = NC_NOERR;
    int ogid;			/* like igrp but in output file */
    int i;

    /* get groupid in output corresponding to group igrp in input,
     * given parent group (or root group) ogrp in output */
    NC_CHECK(get_grpid(igrp, ogrp, &ogid));

    NC_CHECK(copy_dims(igrp, ogid, dimmap)); /* also fills in part of dimmap */
    NC_CHECK(copy_atts(igrp, NC_GLOBAL, ogid, NC_GLOBAL));
    NC_CHECK(copy_vars(igrp, ogid, dimmap));
#ifdef USE_NETCDF4    
    {
	int numgrps;
	int *grpids;
	/* Copy schema from subgroups */
	stat = nc_inq_grps(igrp, &numgrps, NULL);
	grpids = (int *)emalloc((numgrps + 1) * sizeof(int));
	NC_CHECK(nc_inq_grps(igrp, &numgrps, grpids));
	
	for(i = 0; i < numgrps; i++) {
	    NC_CHECK(copy_schema(grpids[i], ogid, dimmap));
	}
	free(grpids);
    }
#endif	/* USE_NETCDF4 */
    return stat;    
}

/* Return number of values for a variable varid in a group igrp */
static int
inq_nvals(int igrp, int varid, long long *nvalsp) {
    int stat = NC_NOERR;
    int ndims;
    int *dimids;
    int dim;
    long long nvals = 1;

    NC_CHECK(nc_inq_varndims(igrp, varid, &ndims));
    dimids = (int *) emalloc((ndims + 1) * sizeof(int));
    NC_CHECK(nc_inq_vardimid (igrp, varid, dimids));
    for(dim = 0; dim < ndims; dim++) {
	size_t len;
	NC_CHECK(nc_inq_dimlen(igrp, dimids[dim], &len));
	nvals *= len;
    }
    if(nvalsp)
	*nvalsp = nvals;
    free(dimids);
    return stat;
}

/* Copy data from variable varid in group igrp to corresponding group
 * ogrp. */
static int
copy_var_data(int igrp, int varid, int ogrp, size_t copybuf_size) {
    int stat = NC_NOERR;
    nc_type vartype;
    long long nvalues;		/* number of values for this variable */
    size_t ntoget;		/* number of values to access this iteration */
    size_t value_size;		/* size of a single value of this variable */
    static void *buf = 0;	/* buffer for the variable values */
    char varname[NC_MAX_NAME];
    int ovarid;
    size_t *start;
    size_t *count;
    nciter_t *iterp;		/* opaque structure for iteration status */
    int do_realloc = 0;
    size_t chunksize;

    NC_CHECK(inq_nvals(igrp, varid, &nvalues));
    if(nvalues == 0)
	return stat;
    /* get corresponding output variable */
    NC_CHECK(nc_inq_varname(igrp, varid, varname));
    NC_CHECK(nc_inq_varid(ogrp, varname, &ovarid));
    NC_CHECK(nc_inq_vartype(igrp, varid, &vartype));
    /* from type, get size in memory needed for each value */
    NC_CHECK(nc_inq_type(igrp, vartype, NULL, &value_size));
    if(value_size > copybuf_size) {
	copybuf_size = value_size;
	do_realloc = 1;
    }
#ifdef USE_NETCDF4    
    /* For chunked variables, copy_buf must also be at least as large as
     * size of a chunk in input */
    {
	NC_CHECK(inq_var_chunksize(igrp, varid, &chunksize));
	if(chunksize > copybuf_size) {
	    copybuf_size = chunksize;
	    do_realloc = 1;
	}
    }
#endif	/* USE_NETCDF4 */
    if(buf && do_realloc) {
	free(buf);
	buf = 0;
    }
    if(buf == 0) {		/* first time or needs to grow */
	buf = emalloc(copybuf_size);
	memset((void*)buf,0,copybuf_size);
    }

    /* initialize variable iteration */
    NC_CHECK(nc_get_iter(igrp, varid, copybuf_size, &iterp));

    start = (size_t *) emalloc((iterp->rank + 1) * sizeof(size_t));
    count = (size_t *) emalloc((iterp->rank + 1) * sizeof(size_t));
    /* nc_next_iter() initializes start and count on first call,
     * changes start and count to iterate through whole variable on
     * subsequent calls. */
    while((ntoget = nc_next_iter(iterp, start, count)) > 0) {
	NC_CHECK(nc_get_vara(igrp, varid, start, count, buf));
	NC_CHECK(nc_put_vara(ogrp, ovarid, start, count, buf));
#ifdef USE_NETCDF4
	/* we have to explicitly free values for strings and vlens */
	if(vartype == NC_STRING) {
	    NC_CHECK(nc_free_string(ntoget, (char **)buf));
	} else if(vartype > NC_STRING) { /* user-defined type */
	    nc_type vclass;
	    NC_CHECK(nc_inq_user_type(igrp, vartype, NULL, NULL, NULL, NULL, &vclass));
	    if(vclass == NC_VLEN) {
		NC_CHECK(nc_free_vlens(ntoget, (nc_vlen_t *)buf));
	    }
	}
	/* We're all done with this input and output variable, so if
	 * either variable is chunked, we might as well free up its
	 * variable chunk cache */
	NC_CHECK(free_var_chunk_cache(igrp, varid));
	NC_CHECK(free_var_chunk_cache(ogrp, ovarid));
#endif	/* USE_NETCDF4 */
    } /* end main iteration loop */
    free(start);
    free(count);
    NC_CHECK(nc_free_iter(iterp));
    return stat;
}

/* Copy data from variables in group igrp to variables in
 * corresponding group with parent ogrp, and all subgroups
 * recursively  */
static int
copy_data(int igrp, int ogrp, size_t copybuf_size)
{
    int stat = NC_NOERR;
    int ogid;
    int numgrps;
    int *grpids;
    int i;
    int nvars;
    int varid;

    /* get groupid in output corresponding to group igrp in input,
     * given parent group (or root group) ogrp in output */
    NC_CHECK(get_grpid(igrp, ogrp, &ogid));
    
    /* Copy data from this group */
    NC_CHECK(nc_inq_nvars(igrp, &nvars));
    for (varid = 0; varid < nvars; varid++) {
	NC_CHECK(copy_var_data(igrp, varid, ogid, copybuf_size));
    }
#ifdef USE_NETCDF4
    /* Copy data from subgroups */
    stat = nc_inq_grps(igrp, &numgrps, NULL);
    grpids = (int *)emalloc((numgrps + 1) * sizeof(int));
    NC_CHECK(nc_inq_grps(igrp, &numgrps, grpids));

    for(i = 0; i < numgrps; i++) {
	NC_CHECK(copy_data(grpids[i], ogid, copybuf_size));
    }
    free(grpids);
#endif	/* USE_NETCDF4 */
    return stat;
}

/* Allocate an int array for mapping input dimids to output dimids, to
 * be filled in and used later, with odimid = dimmap[idimid]. */
void
dimmap_init(int ncid, int** dimmap_p) {
    int numgrps;
    int *grpids;
    int igrp;
    int ndims=0;
    /* get total number of groups and their ids, including all descendants */
    NC_CHECK(nc_inq_grps_full(ncid, &numgrps, NULL));
    grpids = emalloc(numgrps * sizeof(int));
    NC_CHECK(nc_inq_grps_full(ncid, NULL, grpids));
    for(igrp = 0; igrp < numgrps; igrp++) {
	int ndims_local;
	nc_inq_ndims(grpids[igrp], &ndims_local);
	ndims += ndims_local;
    }
    *dimmap_p = (int *) emalloc(ndims * sizeof(int));
    free(grpids); 
}


/* copy infile to outfile using netCDF API, kind specifies which
 * netCDF format for output: -1 -> same as input, 1 -> classic, 2 ->
 * 64-bit offset, 3 -> netCDF-4, 4 -> netCDF-4 classic model.
 * However, if compression or shuffling was specified and kind was -1,
 * kind is changed to format 4 that supports compression for input of
 * type 1 or 2.
 */
static int
copy(char* infile, char* outfile, 
     int kind, 			/* kind of output file requested */
     size_t copybuf_size, 	/* size of buffer used for copying data */
     const char* chunkspec_s	/* unparsed chunkspec string, from command line */
    )
{
    int stat = NC_NOERR;
    int igrp, ogrp;
    int inkind, outkind;
    int *dimmap;		/* array for mapping dimids from input to output */

    NC_CHECK(nc_open(infile,NC_NOWRITE,&igrp));

    NC_CHECK(nc_inq_format(igrp, &inkind));

    if (kind == SAME_AS_INPUT) {	/* default, kind not specified */
	outkind = inkind;
	/* allow kind to be deduced in this case, instead of returning error */
	if ((inkind == NC_FORMAT_CLASSIC || inkind == NC_FORMAT_64BIT) &&
	    (option_deflate_level > 0 || option_shuffle_vars == NC_SHUFFLE) ) { 
	    kind = NC_FORMAT_NETCDF4_CLASSIC;
	    outkind = kind;
	}
    } else {
	outkind = kind;
    }

#ifdef USE_NETCDF4
    if(chunkspec_s) {
	/* Now that input is open, can parse chunkspec_s into binary
	 * structure. */
	NC_CHECK(chunkspec_parse(igrp, chunkspec_s));
    }
#endif	/* USE_NETCDF4 */

    switch(outkind) {
    case NC_FORMAT_CLASSIC:
	NC_CHECK(nc_create(outfile,NC_CLOBBER,&ogrp));
	break;
    case NC_FORMAT_64BIT:
	NC_CHECK(nc_create(outfile,NC_CLOBBER|NC_64BIT_OFFSET,&ogrp));
	break;
#ifdef USE_NETCDF4
    case NC_FORMAT_NETCDF4:
	NC_CHECK(nc_create(outfile,NC_CLOBBER|NC_NETCDF4,&ogrp));
	break;
    case NC_FORMAT_NETCDF4_CLASSIC:
	NC_CHECK(nc_create(outfile,NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL,&ogrp));
	break;
#else
    case NC_FORMAT_NETCDF4:
    case NC_FORMAT_NETCDF4_CLASSIC:
	error("built without ability to create netCDF-4 files");
	break;
#endif	/* USE_NETCDF4 */
    default:
	error("bad value (%d) for -k option\n", kind);
	break;
    }
    NC_CHECK(nc_set_fill(ogrp, NC_NOFILL, NULL));

#ifdef USE_NETCDF4
    /* Because types in one group may depend on types in a different
     * group, need to create all groups before defining types */
    if(inkind == NC_FORMAT_NETCDF4) {
	NC_CHECK(copy_groups(igrp, ogrp));
	NC_CHECK(copy_types(igrp, ogrp));
    }
#endif	/* USE_NETCDF4 */

    dimmap_init(igrp, &dimmap);	/* alloc and initialize dimid mapping from igrp to ogrp */
    /* TODO: pass in &chunkspec separately or combine with dimmap? */
    NC_CHECK(copy_schema(igrp, ogrp, dimmap));
    free(dimmap);
    NC_CHECK(nc_enddef(ogrp));
    NC_CHECK(copy_data(igrp, ogrp, copybuf_size));

    NC_CHECK(nc_close(igrp));
    NC_CHECK(nc_close(ogrp));
    return stat;
}

static void
usage(void)
{
#define USAGE   "\
  [-k n]    kind of netCDF format for output file, default same as input\n\
	    1 classic, 2 64-bit offset, 3 netCDF-4, 4 netCDF-4 classic model\n\
  [-d n]    deflation compression level, default same as input (0=none 9=max)\n\
  [-s]      adds shuffle option to deflation compression\n\
  [-c chunkspec] specifies chunking for dimensions like \"dim1/N1,dim2/N2,...\"\n\
  [-u]      converts unlimited dimensions to fixed-size dimensions in output copy\n\
  [-m n]    size in bytes of copy buffer, default is 5000000 bytes\n\
  infile    name of netCDF input file\n\
  outfile   name for netCDF output file\n"

    error("%s [-k n] [-d n] [-s] [-c chunkspec] [-u] [-m n] infile outfile\n%s",
	  progname, USAGE);
}

int
main(int argc, char**argv)
{
    char* inputfile = NULL;
    char* outputfile = NULL;
    int kind = SAME_AS_INPUT; /* default, output same format as input */
    int c;
    size_t copybuf_size = COPY_BUFFER_SIZE; /* default */
    char* chunkspec = 0;

/* table of formats for legal -k values */
    struct Kvalues {
	char* name;
	int kind;
    } legalkinds[] = {
	{"1", NC_FORMAT_CLASSIC},
	{"classic", NC_FORMAT_CLASSIC},
	
	/* The 64-bit offset kind (2) */
	{"2", NC_FORMAT_64BIT},
	{"64-bit-offset", NC_FORMAT_64BIT},
	{"64-bit offset", NC_FORMAT_64BIT},
	
	/* NetCDF-4 HDF5 format */
	{"3", NC_FORMAT_NETCDF4},
	{"hdf5", NC_FORMAT_NETCDF4},
	{"netCDF-4", NC_FORMAT_NETCDF4},
	{"netCDF4", NC_FORMAT_NETCDF4},
	{"enhanced", NC_FORMAT_NETCDF4},

	/* NetCDF-4 HDF5 format, but using only nc3 data model */
	{"4", NC_FORMAT_NETCDF4_CLASSIC},
	{"hdf5-nc3", NC_FORMAT_NETCDF4_CLASSIC},
	{"netCDF-4 classic model", NC_FORMAT_NETCDF4_CLASSIC},
	{"netCDF4_classic", NC_FORMAT_NETCDF4_CLASSIC},
	{"enhanced-nc3", NC_FORMAT_NETCDF4_CLASSIC},

	/* null terminate*/
	{NULL,0}
    };

    opterr = 1;
    progname = argv[0];

    if (argc <= 1)
    {
       usage();
    }

    while ((c = getopt(argc, argv, "k:d:sum:c:")) != -1) {
	switch(c) {
        case 'k': /* for specifying variant of netCDF format to be generated 
                     Possible values are:
                     1 (=> classic 32 bit)
                     2 (=> classic 64 bit offsets)
                     3 (=> netCDF-4/HDF5)
                     4 (=> classic, but stored in netCDF-4/HDF5 format)
                     Also allow string versions of above
                     "classic"
                     "64-bit-offset"
                     "64-bit offset"
		     "enhanced" | "hdf5" | "netCDF-4"
                     "enhanced-nc3" | "hdf5-nc3" | "netCDF-4 classic model"
		   */
	    {
		struct Kvalues* kvalue;
		char *kind_name = (char *) emalloc(strlen(optarg)+1);
		(void)strcpy(kind_name, optarg);
	        for(kvalue=legalkinds;kvalue->name;kvalue++) {
		    if(strcmp(kind_name,kvalue->name) == 0) {
		        kind = kvalue->kind;
			break;
		    }
		}
		if(kvalue->name == NULL) {
		    error("invalid format: %s", kind_name);
		}
	    }
	    break;
	case 'd':		/* non-default compression level specified */
	    option_deflate_level = strtol(optarg, NULL, 10);
	    if(option_deflate_level < 0 || option_deflate_level > 9) {
		error("invalid deflation level: %d", option_deflate_level);
	    }
	    break;
	case 's':		/* shuffling, may improve compression */
	    option_shuffle_vars = NC_SHUFFLE;
	    break;
	case 'u':		/* convert unlimited dimensions to fixed size */
	    option_fix_unlimdims = 1;
	    break;
	case 'm':		/* non-default size of data copy buffer */
	{
	    char *suffix = 0;	/* "k" for kilobytes or "m" for megabytes */
	    copybuf_size = strtoll(optarg, &suffix, 10);
	    switch (suffix[0]) {
	    case 'k':
	    case 'K':
		copybuf_size *= 1000;
		break;
	    case 'm':
	    case 'M':
		copybuf_size *= 1000000;
		break;
	    case 'g':
	    case 'G':
		copybuf_size *= 1000000000;
		break;
	    default:
		error("Suffix for '-m' option value not k, m, or g: %c", suffix[0]);
	    }		
	    break;
	}
	case 'c':               /* optional chunking spec for each dimension in list */
	{
	    /* save chunkspec string for parsingg later, once we know input ncid */
	    chunkspec = strdup(optarg);
	    break;
	}
	default: 
	    usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 2) {
	error("one input file and one output file required");
    }
    inputfile = argv[0];
    outputfile = argv[1];

    if(strcmp(inputfile, outputfile) == 0) {
	error("output would overwrite input");
    }

    if(copy(inputfile, outputfile, kind, copybuf_size, chunkspec) != NC_NOERR)
        exit(1);
    return 0;
}
