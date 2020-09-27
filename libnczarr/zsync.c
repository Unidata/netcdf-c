/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

#undef FILLONCLOSE

/* Forward */
static int ncz_collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jdimsp);
static int ncz_sync_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);

static int ncz_jsonize_atts(NCindex* attlist, NCjson** jattrsp);
static int load_jatts(NCZMAP* map, NC_OBJ* container, NCjson** jattrsp, NClist** atypes);
static int zconvert(nc_type typeid, size_t typelen, void* dst, NCjson* src);
static int computeattrinfo(const char* name, NClist* atypes, NCjson* values,
		nc_type* typeidp, size_t* lenp, void** datap);
static int parse_group_content(NCjson* jcontent, NClist* dimdefs, NClist* varnames, NClist* subgrps);
static int parse_group_content_pure(NCZ_FILE_INFO_T*  zinfo, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrps);
static int define_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
static int define_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* diminfo);
static int define_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames);
static int define_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* subgrpnames);
static int searchvars(NCZ_FILE_INFO_T*, NC_GRP_INFO_T*, NClist*);
static int searchsubgrps(NCZ_FILE_INFO_T*, NC_GRP_INFO_T*, NClist*);
static int locategroup(NC_FILE_INFO_T* file, size_t nsegs, NClist* segments, NC_GRP_INFO_T** grpp);
static int parsedimrefs(NC_FILE_INFO_T*, NClist* dimrefs, NC_DIM_INFO_T** dims);
static int simulatedimrefs(NC_FILE_INFO_T* file, int rank, size64_t* shapes, NC_DIM_INFO_T** dims);
static int decodeints(NCjson* jshape, size64_t* shapes);
static int computeattrdata(nc_type* typeidp, NCjson* values, size_t* lenp, void** datap);
static int inferattrtype(NCjson* values, nc_type* typeidp);
static int mininttype(unsigned long long u64);

/**************************************************/
/**************************************************/
/* Synchronize functions to make map and memory
be consistent. There are two sets of functions,
1) _sync_ - push memory to map (optionally create target)
2) _read_ - pull map data into memory
These functions are generally non-recursive. It is assumed
that the recursion occurs in the caller's code.
*/

/**
 * @internal Synchronize file metadata from memory to map.
 *
 * @param file Pointer to file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_sync_file(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    LOG((3, "%s: file: %s", __func__, file->controller->path));

    /* Write out root group recursively */
    if((stat = ncz_sync_grp(file, file->root_grp)))
	goto done;

done:
    NCJreclaim(json);
    return THROW(stat);
}

/**
 * @internal Synchronize dimension data from memory to map.
 *
 * @param grp Pointer to grp struct containing the dims.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jdimsp)
{
    int i, stat=NC_NOERR;
    NCjson* jdims = NULL;

    LOG((3, "%s: ", __func__));

    NCJnew(NCJ_DICT,&jdims);   
    for(i=0; i<ncindexsize(grp->dim); i++) {
	NC_DIM_INFO_T* dim = (NC_DIM_INFO_T*)ncindexith(grp->dim,i);
	char slen[128];
	snprintf(slen,sizeof(slen),"%llu",(unsigned long long)dim->len);
	if((stat = NCJaddstring(jdims,NCJ_STRING,dim->hdr.name))) goto done;
	if((stat = NCJaddstring(jdims,NCJ_INT,slen))) goto done;
    }
    if(jdimsp) {*jdimsp = jdims; jdims = NULL;}
done:
    NCJreclaim(jdims);
    return THROW(stat);
}

/**
 * @internal Recursively synchronize group from memory to map.
 *
 * @param file Pointer to file struct
 * @param grp Pointer to grp struct
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_sync_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp)
{
    int i,stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    char version[1024];
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    NCjson* json = NULL;
    NCjson* jgroup = NULL;
    NCjson* jdims = NULL;
    NCjson* jvars = NULL;
    NCjson* jsubgrps = NULL;

    LOG((3, "%s: dims: %s", __func__, key));

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Construct grp key */
    if((stat = NCZ_grpkey(grp,&fullpath)))
	goto done;

    /* Create dimensions dict */
    if((stat = ncz_collect_dims(file,grp,&jdims))) goto done;

    /* Create vars list */
    if((stat = NCJnew(NCJ_ARRAY,&jvars)))
	goto done;
    for(i=0; i<ncindexsize(grp->vars); i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	if((stat = NCJaddstring(jvars,NCJ_STRING,var->hdr.name))) goto done;
    }

    /* Create subgroups list */
    if((stat = NCJnew(NCJ_ARRAY,&jsubgrps)))
	goto done;
    for(i=0; i<ncindexsize(grp->children); i++) {
	NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = NCJaddstring(jsubgrps,NCJ_STRING,g->hdr.name))) goto done;
    }

    /* build ZGROUP path */
    if((stat = nczm_concat(fullpath,ZGROUP,&key)))
	goto done;
    if((stat = NCJnew(NCJ_DICT,&json)))
	goto done;
    snprintf(version,sizeof(version),"%d",zinfo->zarr.zarr_version);
    if((stat = NCJaddstring(json,NCJ_STRING,"zarr_format"))) goto done;
    if((stat = NCJaddstring(json,NCJ_INT,version))) goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,json)))
	goto done;
    nullfree(key); key = NULL;

    if(!(zinfo->features.flags & FLAG_PUREZARR)) {
	/* Create the NCZGROUP json object */
	if((stat = NCJnew(NCJ_DICT,&jgroup)))
	    goto done;
	/* Insert the various dicts and arrays */
	if((stat = NCJinsert(jgroup,"dims",jdims))) goto done;
	jdims = NULL; /* avoid memory problems */
	if((stat = NCJinsert(jgroup,"vars",jvars))) goto done;
	jvars = NULL; /* avoid memory problems */
	if((stat = NCJinsert(jgroup,"groups",jsubgrps))) goto done;
	jsubgrps = NULL; /* avoid memory problems */
	/* build NCZGROUP path */
	if((stat = nczm_concat(fullpath,NCZGROUP,&key)))
	    goto done;
	/* Write to map */
	if((stat=NCZ_uploadjson(map,key,jgroup)))
	    goto done;
	nullfree(key); key = NULL;
    }
    /* Build NCZATTR object and the .zattrs object */
    assert(grp->att);
    if((stat = ncz_sync_atts(file,(NC_OBJ*)grp, grp->att)))
	goto done;

    /* Now synchronize all the variables */
    for(i=0; i<ncindexsize(grp->vars); i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	if((stat = ncz_sync_var(file,var))) goto done;
    }

    /* Now recurse to synchronize all the subgrps */
    for(i=0; i<ncindexsize(grp->children); i++) {
	NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = ncz_sync_grp(file,g))) goto done;
    }

done:
    NCJreclaim(json);
    NCJreclaim(jgroup);
    NCJreclaim(jdims);
    NCJreclaim(jvars);
    NCJreclaim(jsubgrps);
    nullfree(fullpath);
    nullfree(key);
    return THROW(stat);
}

/**
 * @internal Synchronize variable data from memory to map.
 *
 * @param var Pointer to var struct
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_sync_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int i,stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    char number[1024];
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    char* dimpath = NULL;
    NCjson* jvar = NULL;
    NCjson* jncvar = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jtmp = NULL;
    size64_t shape[NC_MAX_VAR_DIMS];
    NCZ_VAR_INFO_T* zvar = var->format_var_info;
	
    LOG((3, "%s: dims: %s", __func__, key));

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Construct var path */
    if((stat = NCZ_varkey(var,&fullpath)))
	goto done;

    /* Create the zarray json object */
    if((stat = NCJnew(NCJ_DICT,&jvar)))
	goto done;

    /* zarr_format key */
    snprintf(number,sizeof(number),"%d",zinfo->zarr.zarr_version);
    if((stat = NCJaddstring(jvar,NCJ_STRING,"zarr_format"))) goto done;
    if((stat = NCJaddstring(jvar,NCJ_INT,number))) goto done;

    /* Collect the shape vector */
    for(i=0;i<var->ndims;i++) {
	NC_DIM_INFO_T* dim = var->dim[i];
	shape[i] = dim->len;
    }

    /* shape key */
    /* Integer list defining the length of each dimension of the array.*/
    /* Create the list */
    if((stat = NCJnew(NCJ_ARRAY,&jtmp))) goto done;
    for(i=0;i<var->ndims;i++) {
	snprintf(number,sizeof(number),"%llu",shape[i]);
	NCJaddstring(jtmp,NCJ_INT,number);
    }
    if((stat = NCJinsert(jvar,"shape",jtmp))) goto done;
    jtmp = NULL;

    /* dtype key */
    /* A string or list defining a valid data type for the array. */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"dtype"))) goto done;
    {	/* Add the type name */
	const char* dtypename;
	int endianness = var->type_info->endianness;
	int islittle;
	switch (endianness) {
	case NC_ENDIAN_LITTLE: islittle = 1; break;
	case NC_ENDIAN_BIG: islittle = 0; break;
	case NC_ENDIAN_NATIVE: abort(); /* should never happen */
	}
	int atomictype = var->type_info->hdr.id;
	assert(atomictype > 0 && atomictype <= NC_MAX_ATOMIC_TYPE && atomictype != NC_STRING);
	if((stat = ncz_zarr_type_name(atomictype,islittle,&dtypename))) goto done;
	if((stat = NCJaddstring(jvar,NCJ_STRING,dtypename))) goto done;
    }

    /* chunks key */
    /* It is not clear if the zarr format supports the concept
       of contiguous, so we will simulate by:
       1. setting a flag in _nczvar (below)
       2. making the chunk sizes all be same as the max dim size (here)
    */
    /* list of chunk sizes */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"chunks"))) goto done;
    /* Create the list */
    if((stat = NCJnew(NCJ_ARRAY,&jtmp))) goto done;
    for(i=0;i<var->ndims;i++) {
	size64_t len = (var->storage == NC_CONTIGUOUS ? shape[i] : var->chunksizes[i]);
	snprintf(number,sizeof(number),"%lld",len);
	NCJaddstring(jtmp,NCJ_INT,number);
    }
    if((stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;
    
    /* fill_value key */
    if(!var->no_fill) {
	int fillsort;
	int atomictype = var->type_info->hdr.id;
	NCjson* jfill = NULL;
	/* A scalar value providing the default value to use for uninitialized
	   portions of the array, or ``null`` if no fill_value is to be used. */
	/* Use the defaults defined in netdf.h */
	assert(atomictype > 0 && atomictype <= NC_MAX_ATOMIC_TYPE && atomictype != NC_STRING);
	if((stat = ncz_fill_value_sort(atomictype,&fillsort))) goto done;
	if(var->fill_value == NULL) { /* use default */
	    size_t typelen;
            if((stat = NC4_inq_atomic_type(atomictype, NULL, &typelen))) goto done;
	    var->fill_value = (atomictype == NC_CHAR ? malloc(typelen+1) : malloc(typelen));
	    if(var->fill_value == NULL) {stat = NC_ENOMEM; goto done;}
	    if((stat = nc4_get_default_fill_value(atomictype,var->fill_value))) goto done;
	}
        /* Convert var->fill_value to a string */
	if((stat = NCZ_stringconvert(atomictype,1,var->fill_value,&jfill)))
	    goto done;
	if((stat = NCJinsert(jvar,"fill_value",jfill))) goto done;
    }

    /* order key */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"order"))) goto done;
    /* "C" means row-major order, i.e., the last dimension varies fastest;
       "F" means column-major order, i.e., the first dimension varies fastest.*/
    /* Default to C for now */ 
    if((stat = NCJaddstring(jvar,NCJ_STRING,"C"))) goto done;

    /* compressor key */
    /* A JSON object identifying the primary compression codec and providing
       configuration parameters, or ``null`` if no compressor is to be used. */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"compressor"))) goto done;
    /* Default to null for now */ 
    if((stat = NCJnew(NCJ_NULL,&jtmp))) goto done;
    if((stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;

    /* filters key */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"filters"))) goto done;
    /* A list of JSON objects providing codec configurations, or ``null``
       if no filters are to be applied. */
    if((stat = NCJnew(NCJ_NULL,&jtmp))) goto done;
    if((stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;

    /* build .zarray path */
    if((stat = nczm_concat(fullpath,ZARRAY,&key)))
	goto done;

    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jvar)))
	goto done;
    
    nullfree(key); key = NULL;
    
    /* Build the NCZVAR object */
    {
	/* Create the dimrefs json object */
	if((stat = NCJnew(NCJ_ARRAY,&jdimrefs)))
	    goto done;
	/* Walk the dimensions and capture the fullpath names */
	for(i=0;i<var->ndims;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    if((stat = NCZ_dimkey(dim,&dimpath))) goto done;
	    NCJaddstring(jdimrefs,NCJ_STRING,dimpath);
	    nullfree(dimpath); dimpath = NULL;
	}
	/* Create the NCZVAR json object */
	if((stat = NCJnew(NCJ_DICT,&jncvar)))
	    goto done;
	/* Insert dimrefs  */
	if((stat = NCJinsert(jncvar,"dimrefs",jdimrefs)))
	    goto done;
	jdimrefs = NULL; /* Avoid memory problems */

	/* Add the _Storage flag */
	if(var->storage == NC_CONTIGUOUS) {
	     if((stat = NCJnewstring(NCJ_STRING,"contiguous",&jtmp)))goto done;
	} else if(var->storage == NC_COMPACT) {
	     if((stat = NCJnewstring(NCJ_STRING,"compact",&jtmp)))goto done;
	} else {/* chunked */
	     if((stat = NCJnewstring(NCJ_STRING,"chunked",&jtmp)))goto done;
	}	
	if((stat = NCJinsert(jncvar,"storage",jtmp))) goto done;
	jtmp = NULL;

	if(!(zinfo->features.flags & FLAG_PUREZARR)) {
	    /* Write out NCZVAR */
	    if((stat = nczm_concat(fullpath,NCZVAR,&key)))
		goto done;
	    /* Write to map */
	    if((stat=NCZ_uploadjson(map,key,jncvar)))
		goto done;
	    nullfree(key); key = NULL;
	}
    }
    /* Build .zattrs object including _nczattr object */
    assert(var->att);
    if((stat = ncz_sync_atts(file,(NC_OBJ*)var, var->att)))
	goto done;

    /* flush only chunks that have been written */
    if(zvar->cache) {
        if((stat = NCZ_flush_chunk_cache(zvar->cache)))
	    goto done;
    }

done:
    nullfree(fullpath);
    nullfree(key);
    nullfree(dimpath);
    NCJreclaim(jvar);
    NCJreclaim(jncvar);
    NCJreclaim(jtmp);
    return THROW(stat);
}

/*
Flush all chunks to disk. Create any that are missing
and fill as needed.
*/
int
ncz_write_var(NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;

    /* Flush the cache */
    if(zvar->cache) {
        if((stat = NCZ_flush_chunk_cache(zvar->cache))) goto done;
    }
    
#ifdef FILLONCLOSE
    /* If fill is enabled, then create missing chunks */
    if(!var->no_fill) {
        int i;
    NCZOdometer* chunkodom =  NULL;
    NC_FILE_INFO_T* file = var->container->nc4_info;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZMAP* map = zfile->map;	 
    size64_t start[NC_MAX_VAR_DIMS];
    size64_t stop[NC_MAX_VAR_DIMS];
    size64_t stride[NC_MAX_VAR_DIMS];
    char* key = NULL;
        for(i=0;i<var->ndims;i++) {
	    size64_t nchunks = ceildiv(var->dim[i]->len,var->chunksizes[i]);
	    start[i] = 0;
	    stop[i] = nchunks;
	    stride[i] = 1;
        }
	/* Iterate over all the chunks to create missing ones */
	if((chunkodom = nczodom_new(var->ndims,start,stop,stride,stop))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	for(;nczodom_more(chunkodom);nczodom_next(chunkodom)) {
	    size64_t* indices = nczodom_indices(chunkodom);	
	    /* Convert to key */
	    if((stat = NCZ_buildchunkpath(zvar->cache,indices,&key))) goto done;
	    switch (stat = nczmap_exists(map,key)) {
	    case NC_NOERR: goto next; /* already exists */
	    case NC_EEMPTY: break; /* does not exist, create it with fill */
	    default: goto done; /* some other error */
	    }
            /* If we reach here, then chunk does not exist, create it with fill */
	    if((stat=nczmap_defineobj(map,key))) goto done;
	    /* ensure fillchunk exists */
	    if(zvar->cache->fillchunk == NULL) {
		nc_type typecode;
		size_t typesize;
	        void* fillvalue = NULL;
		typecode = var->type_info->hdr.id;
		if((stat = NC4_inq_atomic_type(typecode, NULL, &typesize))) goto done;
	        if((stat = ncz_get_fill_value(file, var, &fillvalue))) goto done;
		if((stat = NCZ_create_fill_chunk(zvar->cache->chunksize, typesize, fillvalue, &zvar->cache->fillchunk)))
		    goto done;
	    }
	    if((stat=nczmap_write(map,key,0,zvar->cache->chunksize,zvar->cache->fillchunk))) goto done;
next:
	    nullfree(key);
	    key = NULL;
	}
    nczodom_free(chunkodom);
    nullfree(key);
    }
#endif /*FILLONCLOSE*/

done:
    return THROW(stat);
}

/**
 * @internal Synchronize attribute data from memory to map.
 *
 * @param container Pointer to grp|var struct containing the attributes
 * @param key the name of the map entry
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_sync_atts(NC_FILE_INFO_T* file, NC_OBJ* container, NCindex* attlist)
{
    int i,stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jtype = NULL;
    NCjson* jnczarr = NULL;
    NCjson* jnczattr = NULL;
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    char* content = NULL;
    char* dimpath = NULL;
    NCjson* jname = NULL;

    LOG((3, "%s", __func__));

    zinfo = file->format_file_info;
    map = zinfo->map;

    if(ncindexsize(attlist) == 0) 
	goto done; /* do nothing */

    /* Create the jncattr object */
    if((stat = NCJnew(NCJ_DICT,&jnczattr)))
	goto done;

    /* Create the jncattr.types object */
    if((stat = NCJnew(NCJ_DICT,&jtypes)))
	goto done;
    /* Walk all the attributes and collect the types by attribute name */
    for(i=0;i<ncindexsize(attlist);i++) {
	NC_ATT_INFO_T* a = (NC_ATT_INFO_T*)ncindexith(attlist,i);
	char tname[1024];
	if(a->nc_typeid >= NC_STRING)
	    {stat = THROW(NC_ENCZARR); goto done;}	
	snprintf(tname,sizeof(tname),"%u",a->nc_typeid);
	if((stat = NCJnewstring(NCJ_STRING,tname,&jtype)))
	    goto done;
	if((stat = NCJinsert(jtypes,a->hdr.name,jtype))) /* add {name: type} */
	    goto done;
	jtype = NULL;
    }
    /* Insert into the NCZATTR json */
    if((stat = NCJinsert(jnczattr,"types",jtypes)))
	    goto done;
    jtypes = NULL;	

    /* Construct container path */
    if(container->sort == NCGRP)
	stat = NCZ_grpkey((NC_GRP_INFO_T*)container,&fullpath);
    else
	stat = NCZ_varkey((NC_VAR_INFO_T*)container,&fullpath);
    if(stat)
	goto done;

    /* Upload the NCZATTR object */
    if((stat = nczm_concat(fullpath,NCZATTR,&key)))
	goto done;
    if(!(zinfo->features.flags & FLAG_PUREZARR)) {
	/* Write to map */
	if((stat=NCZ_uploadjson(map,key,jnczattr)))
	    goto done;
    }
    nullfree(key); key = NULL;

    /* Jsonize the attribute list */
    if((stat = ncz_jsonize_atts(attlist,&jatts)))
	goto done;

    /* write .zattrs path */
    if((stat = nczm_concat(fullpath,ZATTRS,&key)))
	goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jatts)))
	goto done;
    nullfree(key); key = NULL;

done:
    nullfree(fullpath);
    nullfree(key);
    nullfree(content);
    nullfree(dimpath);
    NCJreclaim(jatts);
    NCJreclaim(jtypes);
    NCJreclaim(jtype);
    NCJreclaim(jnczarr);
    NCJreclaim(jnczattr);
    NCJreclaim(jname);
    return THROW(stat);
}


/**************************************************/

/**
@internal Convert a list of attributes to corresponding json.
Note that this does not push to the file.
@param attlist - [in] the attributes to dictify
@param jattrsp - [out] the json'ized att list
@return NC_NOERR
@author Dennis Heimbigner
*/
static int
ncz_jsonize_atts(NCindex* attlist, NCjson** jattrsp)
{
    int stat = NC_NOERR;
    int i;
    NCjson* jattrs = NULL;
    NCjson* akey = NULL;
    NCjson* jdata = NULL;

    if((stat = NCJnew(NCJ_DICT,&jattrs))) goto done;

    /* Iterate over the attribute list */
    for(i=0;i<ncindexsize(attlist);i++) {
	NC_ATT_INFO_T* att = (NC_ATT_INFO_T*)ncindexith(attlist,i);
	/* Create the attribute dict value*/
	if((stat = NCZ_stringconvert(att->nc_typeid,att->len,att->data,&jdata)))
	    goto done;
	if((stat = NCJinsert(jattrs,att->hdr.name,jdata))) goto done;
	jdata = NULL;
    }

    if(jattrsp) {*jattrsp = jattrs; jattrs = NULL;}

done:
    NCJreclaim(akey);
    NCJreclaim(jdata);
    NCJreclaim(jattrs);
    NCJreclaim(jdata);
    return THROW(stat);
}

/**
@internal Extract attributes from a group or var and return
the corresponding NCjson dict.
@param map - [in] the map object for storage
@param container - [in] the containing object
@param jattrsp - [out] the json for .zattrs
@param jtypesp - [out] the json for .ztypes
@return NC_NOERR
@author Dennis Heimbigner
*/
static int
load_jatts(NCZMAP* map, NC_OBJ* container, NCjson** jattrsp, NClist** atypesp)
{
    int i,stat = NC_NOERR;
    char* fullpath = NULL;
    char* key = NULL;
    NCjson* jnczarr = NULL;
    NCjson* jattrs = NULL;
    NCjson* jncattr = NULL;
    NClist* atypes = NULL; /* envv list */

    /* alway return (possibly empty) list of types */
    atypes = nclistnew();

    if(container->sort == NCGRP) {
	NC_GRP_INFO_T* grp = (NC_GRP_INFO_T*)container;
	/* Get grp's fullpath name */
	if((stat = NCZ_grpkey(grp,&fullpath)))
	    goto done;
    } else {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)container;
	/* Get var's fullpath name */
	if((stat = NCZ_varkey(var,&fullpath)))
	    goto done;
    }

    /* Construct the path to the .zattrs object */
    if((stat = nczm_concat(fullpath,ZATTRS,&key)))
	goto done;

    /* Download the .zattrs object: may not exist */
    switch ((stat=NCZ_downloadjson(map,key,&jattrs))) {
    case NC_NOERR: break;
    case NC_EEMPTY: stat = NC_NOERR; break; /* did not exist */
    default: goto done; /* failure */
    }
    nullfree(key); key = NULL;

    if(jattrs != NULL) {
	/* Construct the path to the NCZATTR object */
	if((stat = nczm_concat(fullpath,NCZATTR,&key))) goto done;
	/* Download the NCZATTR object: may not exist if pure zarr */
	switch((stat=NCZ_downloadjson(map,key,&jncattr))) {
	case NC_NOERR: break;
	case NC_EEMPTY: stat = NC_NOERR; jncattr = NULL; break;
	default: goto done; /* failure */
	}
	nullfree(key); key = NULL;

	if(jncattr != NULL) {
	    NCjson* jtypes = NULL;
	    /* jncattr attribute should be a dict */
	    if(jncattr->sort != NCJ_DICT) {stat = THROW(NC_ENCZARR); goto done;}
	    /* Extract "types */
	    if((stat = NCJdictget(jncattr,"types",&jtypes))) goto done;
	    if(jtypes->sort != NCJ_DICT) {stat = THROW(NC_ENCZARR); goto done;}
	    /* Convert to an envv list */
	    for(i=0;i<nclistlength(jtypes->contents);i+=2) {
		const NCjson* key = nclistget(jtypes->contents,i);
		const NCjson* value = nclistget(jtypes->contents,i+1);
		if(key->sort != NCJ_STRING) {stat = THROW(NC_ENCZARR); goto done;}
		if(value->sort != NCJ_STRING) {stat = THROW(NC_ENCZARR); goto done;}
		nclistpush(atypes,strdup(key->value));
		nclistpush(atypes,strdup(value->value));
	    }
	}
    }
    if(jattrsp) {*jattrsp = jattrs; jattrs = NULL;}
    if(atypesp) {*atypesp = atypes; atypes = NULL;}

done:
    NCJreclaim(jncattr);
    if(stat) {
	NCJreclaim(jnczarr);
	nclistfreeall(atypes);
    }
    nullfree(fullpath);
    nullfree(key);
    return THROW(stat);
}

/* Convert a json value to actual data values of an attribute. */
static int
zconvert(nc_type typeid, size_t typelen, void* dst0, NCjson* src)
{
    int stat = NC_NOERR;
    int i;
    size_t len;
    char* dst = dst0; /* Work in char* space so we can do pointer arithmetic */

    switch (src->sort) {
    case NCJ_ARRAY:
	for(i=0;i<nclistlength(src->contents);i++) {
	    NCjson* value = nclistget(src->contents,i);
	    assert(value->sort != NCJ_STRING);
	    if((stat = NCZ_convert1(value, typeid, dst)))
		goto done;
	    dst += typelen;
	}
	break;
    case NCJ_STRING:
	len = strlen(src->value);
	memcpy(dst,src->value,len);
	dst[len] = '\0'; /* nul terminate */
	break;	
    case NCJ_INT: case NCJ_DOUBLE: case NCJ_BOOLEAN:
	if((stat = NCZ_convert1(src, typeid, dst)))
	    goto done;
	break;
    default: stat = THROW(NC_ENCZARR); goto done;
    }

done:
    return THROW(stat);
}

/*
Extract type and data for an attribute
*/
static int
computeattrinfo(const char* name, NClist* atypes, NCjson* values,
		nc_type* typeidp, size_t* lenp, void** datap)
{
    int stat = NC_NOERR;
    int i;
    size_t len;
    void* data;
    nc_type typeid;

    /* Get type info for the given att */
    typeid = NC_NAT;
    for(i=0;i<nclistlength(atypes);i+=2) {
	const char* aname = nclistget(atypes,i);
	if(strcmp(aname,name)==0) {
	    const char* atype = nclistget(atypes,i+1);
	    unsigned long tid;
	    if(sscanf(atype,"%lu",&tid) != 1) {stat = THROW(NC_ENCZARR); goto done;}
	    typeid = (nc_type)tid;
	    break;
	}
    }
    if(typeid >= NC_STRING)
	{stat = NC_EINTERNAL; goto done;}
    if((stat = computeattrdata(&typeid, values, &len, &data))) goto done;

    if(typeidp) *typeidp = typeid;
    if(lenp) *lenp = len;
    if(datap) {*datap = data; data = NULL;}

done:
    nullfree(data);
    return THROW(stat);
}

/*
Extract data for an attribute
*/
static int
computeattrdata(nc_type* typeidp, NCjson* values, size_t* lenp, void** datap)
{
    int stat = NC_NOERR;
    size_t datalen;
    void* data = NULL;
    size_t typelen;
    nc_type typeid = NC_NAT;

    /* Get assumed type */
    if(typeidp) typeid = *typeidp;
    if(typeid == NC_NAT) inferattrtype(values,&typeid);
    if(typeid == NC_NAT) {stat = NC_EBADTYPE; goto done;}

    /* Collect the length of the attribute */
    switch (values->sort) {
    case NCJ_DICT: stat = NC_EINTERNAL; goto done;
    case NCJ_ARRAY:
	datalen = nclistlength(values->contents);
	break;
    case NCJ_STRING: /* requires special handling as an array of characters */
	datalen = strlen(values->value);
	break;
    default:
	datalen = 1;
	break;
    }

    /* Allocate data space */
    if((stat = NC4_inq_atomic_type(typeid, NULL, &typelen)))
	goto done;
    if(typeid == NC_CHAR)
        data = malloc(typelen*(datalen+1));
    else
        data = malloc(typelen*datalen);
    if(data == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* convert to target type */	
    if((stat = zconvert(typeid, typelen, data, values)))
	goto done;

    if(lenp) *lenp = datalen;
    if(datap) {*datap = data; data = NULL;}

done:
    nullfree(data);
    return THROW(stat);
}

static int
inferattrtype(NCjson* values, nc_type* typeidp)
{
    nc_type typeid;
    NCjson* j = NULL;
    unsigned long long u64;
    long long i64;

    if(NCJlength(values) == 0) return NC_EINVAL;
    switch (values->sort) {
    case NCJ_ARRAY:
	/* use the first value to decide */
	if(NCJarrayith(values,0,&j)) return NC_EINVAL;
	switch(j->sort) {
	case NCJ_INT:
	    if(j->value[0] == '-') {
		sscanf(j->value,"%lld",&i64);
		u64 = (unsigned long long)i64;
	    } else
		sscanf(j->value,"%llu",&u64);
	    typeid = mininttype(u64);
	    break;
	case NCJ_DOUBLE:
	    typeid = NC_DOUBLE;
	    break;
	case NCJ_BOOLEAN:
	    typeid = NC_UBYTE;
	    break;
	default: return NC_EINVAL;
	}
	break;
    case NCJ_STRING: /* requires special handling as an array of characters */
	typeid = NC_CHAR;
	break;
    default:
	return NC_EINTERNAL;
    }
    if(typeidp) *typeidp = typeid;
    return NC_NOERR;
}

static int
mininttype(unsigned long long u64)
{
    long long i64 = (long long)u64; /* keep bit pattern */
    if(u64 >= NC_MAX_INT64) return NC_UINT64;
    if(i64 < 0) {
	if(i64 >= NC_MIN_BYTE) return NC_BYTE;
	if(i64 >= NC_MIN_SHORT) return NC_SHORT;
	if(i64 >= NC_MIN_INT) return NC_INT;
	return NC_INT64;
    }
    if(i64 <= NC_MAX_BYTE) return NC_BYTE;
    if(i64 <= NC_MAX_UBYTE) return NC_UBYTE;
    if(i64 <= NC_MAX_SHORT) return NC_SHORT;
    if(i64 <= NC_MAX_USHORT) return NC_USHORT;
    if(i64 <= NC_MAX_INT) return NC_INT;
    if(i64 <= NC_MAX_UINT) return NC_UINT;
    return NC_INT64;
}
	    

/**
 * @internal Read file data from map to memory.
 *
 * @param file Pointer to file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_read_file(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    LOG((3, "%s: file: %s", __func__, file->controller->path));

    /* _nczarr should already have been read in ncz_open_dataset */

    /* Now load the groups starting with root */
    if((stat = define_grp(file,file->root_grp)))
	goto done;

done:
    NCJreclaim(json);
    return THROW(stat);
}

/**
 * @internal Read group data from map to memory
 *
 * @param file Pointer to file struct
 * @param grp Pointer to grp struct
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
define_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    NCjson* json = NULL;
    NCjson* jncgroup = NULL;
    NClist* dimdefs = nclistnew();
    NClist* varnames = nclistnew();
    NClist* subgrps = nclistnew();
    int nodimrefs = 0;

    LOG((3, "%s: dims: %s", __func__, key));

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Construct grp path */
    if((stat = NCZ_grpkey(grp,&fullpath)))
	goto done;

    if(zinfo->features.flags & FLAG_PUREZARR) {
	if((stat = parse_group_content_pure(zinfo,grp,varnames,subgrps)))
	    goto done;
	nodimrefs = 1;
    } else { /*!(zinfo->features.flags & FLAG_PUREZARR) */
	/* build NCZGROUP path */
	if((stat = nczm_concat(fullpath,NCZGROUP,&key)))
	    goto done;
	/* Read */
	switch (stat=NCZ_downloadjson(map,key,&jncgroup)) {
	case NC_NOERR: /* we read it */
	    /* Pull out lists about group content */
	    if((stat = parse_group_content(jncgroup,dimdefs,varnames,subgrps)))
		goto done;
	    break;
	case NC_EEMPTY: /* probably pure zarr, so does not exist, use search */
	    if((stat = parse_group_content_pure(zinfo,grp,varnames,subgrps)))
		goto done;
	    nodimrefs = 1;
	    break;
	default: goto done;
	}
	nullfree(key); key = NULL;

    }
    if(!nodimrefs) {
	/* Define dimensions */
	if((stat = define_dims(file,grp,dimdefs))) goto done;
    }

    /* Define vars */
    if((stat = define_vars(file,grp,varnames))) goto done;

    /* Define sub-groups */
    if((stat = define_subgrps(file,grp,subgrps))) goto done;

done:
    NCJreclaim(json);
    NCJreclaim(jncgroup);
    nclistfreeall(dimdefs);
    nclistfreeall(varnames);
    nclistfreeall(subgrps);
    nullfree(fullpath);
    nullfree(key);
    return THROW(stat);
}


/**
@internal Read attributes from a group or var and create a list
of annotated NC_ATT_INFO_T* objects. This will process
_NCProperties attribute specially.
@param zfile - [in] the containing file (annotation)
@param container - [in] the containing object
@return NC_NOERR
@author Dennis Heimbigner
*/
int
ncz_read_atts(NC_FILE_INFO_T* file, NC_OBJ* container)
{
    int stat = NC_NOERR;
    int i;
    char* fullpath = NULL;
    char* key = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCZMAP* map = NULL;
    NC_ATT_INFO_T* att = NULL;
    NCindex* attlist = NULL;
    NCjson* jattrs = NULL;
    NClist* atypes = NULL;
    nc_type typeid;
    size_t len;
    void* data = NULL;
    NC_ATT_INFO_T* fillvalueatt = NULL;

    zinfo = file->format_file_info;
    map = zinfo->map;

    if(container->sort == NCGRP)
	attlist = ((NC_GRP_INFO_T*)container)->att;
    else
	attlist = ((NC_VAR_INFO_T*)container)->att;

    switch ((stat = load_jatts(map, container, &jattrs, &atypes))) {
    case NC_NOERR: break;
    case NC_EEMPTY:  /* container has no attributes */
        stat = NC_NOERR;
	break;
    default: goto done; /* true error */
    }

    if(jattrs != NULL) {
	/* Iterate over the attributes to create the in-memory attributes */
	/* Watch for reading _FillValue */
	for(i=0;i<nclistlength(jattrs->contents);i+=2) {
	    NCjson* key = nclistget(jattrs->contents,i);
	    NCjson* value = nclistget(jattrs->contents,i+1);
	    const NC_reservedatt* ra = NULL;
    
	    /* See if this is reserved attribute */
	    ra = NC_findreserved(key->value);
	    if(ra != NULL) {
		/* case 1: grp=root, varid==NC_GLOBAL, flags & READONLYFLAG */
		if(container->sort == NCGRP
		    && file->root_grp == (NC_GRP_INFO_T*)container) {
		    /* This is the real thing */ 
		    if(strcmp(key->value,NCPROPS)==0) {
			/* Setup provenance */
			if(value->sort != NCJ_STRING)
			    {stat = THROW(NC_ENCZARR); goto done;} /*malformed*/
			if((stat = NCZ_read_provenance(file,key->value,value->value)))
			    goto done;
		    }
		    /* Otherwise ignore the reserved attribute */
		    continue;
		}
	    }
	    /* Create the attribute */
	    /* Collect the attribute's type and value  */
	    if((stat = computeattrinfo(key->value,atypes,value,
				   &typeid,&len,&data)))
		goto done;
	    if((stat = ncz_makeattr(container,attlist,key->value,typeid,len,data,&att)))
		goto done;
	    /* Is this _FillValue ? */
	    if(strcmp(att->hdr.name,_FillValue)==0) fillvalueatt = att;
	}
    }
    /* If we have not read a _FillValue, then go ahead and create it */
    if(fillvalueatt == NULL && container->sort == NCVAR) {
	if((stat = ncz_create_fillvalue((NC_VAR_INFO_T*)container)))
	    goto done;
    }

    /* Remember that we have read the atts for this var or group. */
    if(container->sort == NCVAR)
	((NC_VAR_INFO_T*)container)->atts_read = 1;
    else
	((NC_GRP_INFO_T*)container)->atts_read = 1;

done:
    NCJreclaim(jattrs);
    nclistfreeall(atypes);
    nullfree(fullpath);
    nullfree(key);
    return THROW(stat);
}

/**
 * @internal Materialize dimensions into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param diminfo List of (name,length) pairs
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
define_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* diminfo)
{
    int i,stat = NC_NOERR;

    /* Reify each dim in turn */
    for(i = 0; i < nclistlength(diminfo); i+=2) {
	NC_DIM_INFO_T* dim = NULL;
	size64_t len = 0;
	const char* name = nclistget(diminfo,i);
	const char* value = nclistget(diminfo,i+1);

	/* Create the NC_DIM_INFO_T object */
	sscanf(value,"%lld",&len); /* Get length */
	if(len < 0) {stat = NC_EDIMSIZE; goto done;}
	if((stat = nc4_dim_list_add(grp, name, (size_t)len, -1, &dim)))
	    goto done;
	if((dim->format_dim_info = calloc(1,sizeof(NCZ_DIM_INFO_T))) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	((NCZ_DIM_INFO_T*)dim->format_dim_info)->common.file = file;
    }

done:
    return THROW(stat);
}

/**
 * @internal Materialize vars into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param varnames List of names of variables in this group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
define_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames)
{
    int stat = NC_NOERR;
    int i,j;
    char* varpath = NULL;
    char* key = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCZ_VAR_INFO_T* zvar = NULL;
    NCZMAP* map = NULL;
    NCjson* jvar = NULL;
    NCjson* jncvar = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jvalue = NULL;
    NClist* dimrefs = NULL;
    int hasdimrefs;
    nc_type typeid;

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Load each var in turn */
    for(i = 0; i < nclistlength(varnames); i++) {
	NC_VAR_INFO_T* var;
	const char* varname = nclistget(varnames,i);

	/* Create the NC_VAR_INFO_T object */
	if((stat = nc4_var_list_add2(grp, varname, &var)))
	    goto done;

	/* And its annotation */
	if((zvar = calloc(1,sizeof(NCZ_VAR_INFO_T)))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	var->format_var_info = zvar;
	zvar->common.file = file;

	/* Set filter list */
	var->filters = (void*)nclistnew();

	/* Construct var path */
	if((stat = NCZ_varkey(var,&varpath)))
	    goto done;

	/* Construct the path to the zarray object */
	if((stat = nczm_concat(varpath,ZARRAY,&key)))
	    goto done;
	/* Download the zarray object */
	if((stat=NCZ_readdict(map,key,&jvar)))
	    goto done;
	nullfree(key); key = NULL;
	assert((jvar->sort == NCJ_DICT));

	if(zinfo->features.flags & FLAG_PUREZARR) {
	    var->storage = NC_CHUNKED;
	    hasdimrefs = 0;
	} else { /*zinfo->features.flags & FLAG_PUREZARR*/
	    /* Download the NCZVAR object */
	    if((stat = nczm_concat(varpath,NCZVAR,&key))) goto done;
	    if((stat=NCZ_readdict(map,key,&jncvar))) goto done;
	    nullfree(key); key = NULL;
	    assert((jncvar->sort == NCJ_DICT));
	    /* Extract storage flag */
	    if((stat = NCJdictget(jncvar,"storage",&jvalue)))
		goto done;
	    if(jvalue != NULL) {
		if(strcmp(jvalue->value,"chunked") == 0) {
		    var->storage = NC_CHUNKED;
		} else if(strcmp(jvalue->value,"compact") == 0) {
		    var->storage = NC_COMPACT;
		} else { /*storage = NC_CONTIGUOUS;*/
		    var->storage = NC_CONTIGUOUS;
		}
	    }
	    /* Extract dimrefs list  */
	    switch ((stat = NCJdictget(jncvar,"dimrefs",&jdimrefs))) {
	    case NC_NOERR: /* Extract the dimref names */
		assert((jdimrefs->sort == NCJ_ARRAY));
		hasdimrefs = 0; /* until we have one */	    
		dimrefs = nclistnew();
		for(j=0;j<nclistlength(jdimrefs->contents);j++) {
		    const NCjson* dimpath = nclistget(jdimrefs->contents,j);
		    assert(dimpath->sort == NCJ_STRING);
		    nclistpush(dimrefs,strdup(dimpath->value));
		    hasdimrefs = 1;
		}
		break;
	    case NC_EEMPTY: /* simulate it from the shape of the variable */
		stat = NC_NOERR;
		hasdimrefs = 0;
		break;
	    default: goto done;
	    }
	    jdimrefs = NULL;
	}

	/* Use jvar to set up the rest of the NC_VAR_INFO_T object */
	/* Verify the format */
	{
	    int version;
	    if((stat = NCJdictget(jvar,"zarr_format",&jvalue))) goto done;
	    sscanf(jvalue->value,"%d",&version);
	    if(version != zinfo->zarr.zarr_version)
		{stat = THROW(NC_ENCZARR); goto done;}
	}
	/* Set the type and endianness of the variable */
	{
	    nc_type vtype;
	    int endianness;
	    if((stat = NCJdictget(jvar,"dtype",&jvalue))) goto done;
	    /* Convert dtype to nc_type + endianness */
	    if((stat = ncz_dtype2typeinfo(jvalue->value,&vtype,&endianness)))
		goto done;
	    if(vtype > NC_NAT && vtype < NC_STRING) {
		/* Locate the NC_TYPE_INFO_T object */
		if((stat = ncz_gettype(file,grp,vtype,&var->type_info)))
		    goto done;
	    } else {stat = NC_EBADTYPE; goto done;}
	    if(endianness == NC_ENDIAN_LITTLE || endianness == NC_ENDIAN_BIG) {
		var->endianness = endianness;
	    } else {stat = NC_EBADTYPE; goto done;}
	    var->type_info->endianness = var->endianness; /* Propagate */
	}
	/* shape */
	{
	    int rank;
	    if((stat = NCJdictget(jvar,"shape",&jvalue))) goto done;
	    if(jvalue->sort != NCJ_ARRAY) {stat = THROW(NC_ENCZARR); goto done;}
	    /* Verify the rank */
	    rank = nclistlength(jvalue->contents);
	    if(hasdimrefs) { /* verify rank consistency */
		if(nclistlength(dimrefs) != rank)
		    {stat = THROW(NC_ENCZARR); goto done;}
	    }
	    /* Set the rank of the variable */
	    if((stat = nc4_var_set_ndims(var, rank))) goto done;
	    if(hasdimrefs) {
		if((stat = parsedimrefs(file, dimrefs, var->dim)))
		    goto done;
	    } else { /* simulate the dimrefs */
		size64_t shapes[NC_MAX_VAR_DIMS];
		if((stat = decodeints(jvalue, shapes))) goto done;
		if((stat = simulatedimrefs(file, rank, shapes, var->dim)))
		    goto done;
	    }
	    /* fill in the dimids */
	    for(j=0;j<rank;j++)
		var->dimids[j] = var->dim[j]->hdr.id;
	}
	/* chunks */
	{
	    int rank;
	    size64_t chunks[NC_MAX_VAR_DIMS];
	    if((stat = NCJdictget(jvar,"chunks",&jvalue))) goto done;
	    if(jvalue != NULL && jvalue->sort != NCJ_ARRAY)
		{stat = THROW(NC_ENCZARR); goto done;}
	    /* Verify the rank */
	    rank = nclistlength(jvalue->contents);
	    if(rank > 0) {
		var->storage = NC_CHUNKED;
		if(var->ndims != rank)
		    {stat = THROW(NC_ENCZARR); goto done;}
		if((var->chunksizes = malloc(sizeof(size_t)*rank)) == NULL)
		    {stat = NC_ENOMEM; goto done;}
		if((stat = decodeints(jvalue, chunks))) goto done;
		/* validate the chunk sizes */
		zvar->chunkproduct = 1;
		for(j=0;j<rank;j++) {
		    NC_DIM_INFO_T* d = var->dim[j]; /* matching dim */
		    if(chunks[j] == 0 || chunks[j] > d->len)
			{stat = THROW(NC_ENCZARR); goto done;}
		    var->chunksizes[j] = (size_t)chunks[j];
		    zvar->chunkproduct *= chunks[j];
		}
		/* Create the cache */
		zvar->chunk_cache_nelems = var->chunk_cache_nelems;
		if((stat = NCZ_create_chunk_cache(var,var->type_info->size*zvar->chunkproduct,&zvar->cache)))
		    goto done;
	    }
	}
	/* fill_value */
	{
	    if((stat = NCJdictget(jvar,"fill_value",&jvalue))) goto done;
	    if(jvalue == NULL)
		var->no_fill = 1;
	    else {
		typeid = var->type_info->hdr.id;
		var->no_fill = 0;
		if((stat = computeattrdata(&typeid, jvalue, NULL, &var->fill_value)))
		    goto done;
		assert(typeid == var->type_info->hdr.id);
		/* Note that we do not create the _FillValue
		   attribute here to avoid having to read all
		   the attributes and thus foiling lazy read.*/
	    } 
	}
	/* Capture row vs column major; currently, column major not used*/
	{
	    if((stat = NCJdictget(jvar,"order",&jvalue))) goto done;
	    if(strcmp(jvalue->value,"C")==1)
		((NCZ_VAR_INFO_T*)var->format_var_info)->order = 1;
	    else ((NCZ_VAR_INFO_T*)var->format_var_info)->order = 0;
	}
	/* compressor ignored */
	{
	    if((stat = NCJdictget(jvar,"compressor",&jvalue))) goto done;
	    /* ignore */
	}
	/* filters ignored */
	{
	    if((stat = NCJdictget(jvar,"filters",&jvalue))) goto done;
	    /* ignore */
	}
	/* Clean up from last cycle */
	nclistfreeall(dimrefs); dimrefs = NULL;
        nullfree(varpath); varpath = NULL;
	NCJreclaim(jvar); jvar = NULL;
	NCJreclaim(jncvar); jncvar = NULL;
    }

done:
    nullfree(varpath);
    nullfree(key);
    nclistfreeall(dimrefs);
    NCJreclaim(jvar);
    NCJreclaim(jncvar);
    return THROW(stat);
}

/**
 * @internal Materialize subgroups into memory
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param subgrpnames List of names of subgroups in this group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
define_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* subgrpnames)
{
    int i,stat = NC_NOERR;

    /* Load each subgroup name in turn */
    for(i = 0; i < nclistlength(subgrpnames); i++) {
	NC_GRP_INFO_T* g = NULL;
	const char* gname = nclistget(subgrpnames,i);
	char norm_name[NC_MAX_NAME];
	/* Check and normalize the name. */
	if((stat = nc4_check_name(gname, norm_name)))
	    goto done;
	if((stat = nc4_grp_list_add(file, grp, norm_name, &g)))
	    goto done;
	if(!(g->format_grp_info = calloc(1, sizeof(NCZ_GRP_INFO_T))))
	    {stat = NC_ENOMEM; goto done;}
	((NCZ_GRP_INFO_T*)g->format_grp_info)->common.file = file;
    }

    /* Recurse to fill in subgroups */
    for(i=0;i<ncindexsize(grp->children);i++) {
	NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = define_grp(file,g)))
	    goto done;
    }

done:
    return THROW(stat);
}

/**************************************************/
/* Utilities */

static int
parse_group_content(NCjson* jcontent, NClist* dimdefs, NClist* varnames, NClist* subgrps)
{
    int i,stat = NC_NOERR;
    NCjson* jvalue = NULL;

    if((stat=NCJdictget(jcontent,"dims",&jvalue))) goto done;
    if(jvalue != NULL) {
	if(jvalue->sort != NCJ_DICT) {stat = THROW(NC_ENCZARR); goto done;}
	/* Extract the dimensions defined in this group */
	for(i=0;i<nclistlength(jvalue->contents);i+=2) {
	    NCjson* jname = nclistget(jvalue->contents,i);
	    NCjson* jlen = nclistget(jvalue->contents,i+1);
	    char norm_name[NC_MAX_NAME + 1];
	    size64_t len;
	    /* Verify name legality */
	    if((stat = nc4_check_name(jname->value, norm_name)))
		{stat = NC_EBADNAME; goto done;}
	    /* check the length */
	    sscanf(jlen->value,"%lld",&len);
	    if(len <= 0)
		{stat = NC_EDIMSIZE; goto done;}		
	    nclistpush(dimdefs,strdup(norm_name));
	    nclistpush(dimdefs,strdup(jlen->value));
	}
    }

    if((stat=NCJdictget(jcontent,"vars",&jvalue))) goto done;
    if(jvalue != NULL) {
	/* Extract the variable names in this group */
	for(i=0;i<nclistlength(jvalue->contents);i++) {
	    NCjson* jname = nclistget(jvalue->contents,i);
	    char norm_name[NC_MAX_NAME + 1];
	    /* Verify name legality */
	    if((stat = nc4_check_name(jname->value, norm_name)))
		{stat = NC_EBADNAME; goto done;}
	    nclistpush(varnames,strdup(norm_name));
	}
    }

    if((stat=NCJdictget(jcontent,"groups",&jvalue))) goto done;
    if(jvalue != NULL) {
	/* Extract the subgroup names in this group */
	for(i=0;i<nclistlength(jvalue->contents);i++) {
	    NCjson* jname = nclistget(jvalue->contents,i);
	    char norm_name[NC_MAX_NAME + 1];
	    /* Verify name legality */
	    if((stat = nc4_check_name(jname->value, norm_name)))
		{stat = NC_EBADNAME; goto done;}
	    nclistpush(subgrps,strdup(norm_name));
	}
    }

done:
    return THROW(stat);
}

static int
parse_group_content_pure(NCZ_FILE_INFO_T*  zinfo, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrps)
{
    int stat = NC_NOERR;

    nclistclear(varnames);
    if((stat = searchvars(zinfo,grp,varnames))) goto done;
    nclistclear(subgrps);
    if((stat = searchsubgrps(zinfo,grp,subgrps))) goto done;

done:
    return THROW(stat);
}


#if 0
static int
parse_var_dims_pure(NCZ_FILE_INFO_T*  zinfo, NC_GRP_INFO_T* grp, NC_VAR_INFO_T* var, size64_t* shapes)
{
    int stat = NC_NOERR;
    char* varkey = NULL;
    char* zakey = NULL;
    NCjson* jvar = NULL;
    NCjson* jvalue = NULL;

    /* Construct var path */
    if((stat = NCZ_varkey(var,&varkey))) goto done;
    /* Construct .zarray path */
    if((stat = nczm_concat(varkey,ZARRAY,&zakey))) goto done;
    /* Download the zarray object */
    if((stat=NCZ_readdict(zinfo->map,zakey,&jvar)))
	goto done;
    assert((jvar->sort == NCJ_DICT));
    nullfree(varkey); varkey = NULL;
    nullfree(zakey); zakey = NULL;
    /* Extract the shape */
    if((stat=NCJdictget(jvar,"shape",&jvalue))) goto done;
    if((stat = decodeints(jvalue, shapes))) goto done;

done:
    NCJreclaim(jvar);
    NCJreclaim(jvalue);
    nullfree(varkey); varkey = NULL;
    nullfree(zakey); zakey = NULL;
    return THROW(stat);
}
#endif

static int
searchvars(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* varnames)
{
    int i,stat = NC_NOERR;
    char* grpkey = NULL;
    char* varkey = NULL;
    char* zarray = NULL;
    NClist* matches = nclistnew();
    
    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(grp,&grpkey))) goto done;
    /* Get the map and search group */
    if((stat = nczmap_search(zfile->map,grpkey,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
	const char* name = nclistget(matches,i);
	if(name[0] == NCZM_DOT) continue; /* zarr/nczarr specific */
	/* See if name/.zarray exists */
	if((stat = nczm_concat(grpkey,name,&varkey))) goto done;
	if((stat = nczm_concat(varkey,ZARRAY,&zarray))) goto done;
	if((stat = nczmap_exists(zfile->map,zarray)) == NC_NOERR)
	    nclistpush(varnames,strdup(name));
	stat = NC_NOERR;
	nullfree(varkey); varkey = NULL;
	nullfree(zarray); zarray = NULL;
    }

done:
    nullfree(grpkey);
    nullfree(varkey);
    nullfree(zarray);
    nclistfreeall(matches);
    return stat;
}

static int
searchsubgrps(NCZ_FILE_INFO_T* zfile, NC_GRP_INFO_T* grp, NClist* subgrpnames)
{
    int i,stat = NC_NOERR;
    char* grpkey = NULL;
    char* subkey = NULL;
    char* zgroup = NULL;
    NClist* matches = nclistnew();
    
    /* Compute the key for the grp */
    if((stat = NCZ_grpkey(grp,&grpkey))) goto done;
    /* Get the map and search group */
    if((stat = nczmap_search(zfile->map,grpkey,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
	const char* name = nclistget(matches,i);
	if(name[0] == NCZM_DOT) continue; /* zarr/nczarr specific */
	/* See if name/.zgroup exists */
	if((stat = nczm_concat(grpkey,name,&subkey))) goto done;
	if((stat = nczm_concat(subkey,ZGROUP,&zgroup))) goto done;
	if((stat = nczmap_exists(zfile->map,zgroup)) == NC_NOERR)
	    nclistpush(subgrpnames,strdup(name));
	stat = NC_NOERR;
	nullfree(subkey); subkey = NULL;
	nullfree(zgroup); zgroup = NULL;
    }

done:
    nullfree(grpkey);
    nullfree(subkey);
    nullfree(zgroup);
    nclistfreeall(matches);
    return stat;
}

/* Convert a list of integer strings to integers */
static int
decodeints(NCjson* jshape, size64_t* shapes)
{
    int i, stat = NC_NOERR;

    for(i=0;i<nclistlength(jshape->contents);i++) {
	long long v;
	NCjson* jv = nclistget(jshape->contents,i);
	if((stat = NCZ_convert1(jv,NC_INT64,(char*)&v))) goto done;
	if(v < 0) {stat = THROW(NC_ENCZARR); goto done;}
	shapes[i] = (size64_t)v;
    }

done:
    return THROW(stat);
}

static int
simulatedimrefs(NC_FILE_INFO_T* file, int rank, size64_t* shapes, NC_DIM_INFO_T** dims)
{
    int i, j, stat = NC_NOERR;
    NC_GRP_INFO_T* root = file->root_grp;
    NC_DIM_INFO_T* thed = NULL;
    int match = 0;

    for(i=0;i<rank;i++) {
	size64_t dimlen = shapes[i];
	char shapename[NC_MAX_NAME];
	match = 0;
	/* See if there is a dimension named ".zdim<dimlen>", if not create */
	snprintf(shapename,sizeof(shapename),".zdim%llu",dimlen);
	for(j=0;j<ncindexsize(root->dim);j++) {
	    thed = (NC_DIM_INFO_T*)ncindexith(root->dim,j);
	    if(strcmp(thed->hdr.name,shapename)==0) {
		if(dimlen != (size64_t)thed->len)
		    {stat = THROW(NC_ENCZARR); goto done;} /* we have a problem */
		match = 1;
		break;
	    }
	}
	if(!match) { /* create the dimension */
	    /* This code is a subset of NCZ_def_dim */
	    thed = (NC_DIM_INFO_T*)ncindexlookup(root->dim,shapename);
	    if(thed == NULL) { /*create the dim*/
		if ((stat = nc4_dim_list_add(root, shapename, (size_t)dimlen, -1, &thed)))
		    goto done;
		/* Create struct for NCZ-specific dim info. */
		if (!(thed->format_dim_info = calloc(1, sizeof(NCZ_DIM_INFO_T))))
		    {stat = NC_ENOMEM; goto done;}
		((NCZ_DIM_INFO_T*)thed->format_dim_info)->common.file = file;
	    }
	    assert(thed != NULL);
	}
	/* Save the id */
	dims[i] = thed;
    }

done:
    return THROW(stat);
}

/*
Given a list of segments, find corresponding group.
*/
static int
locategroup(NC_FILE_INFO_T* file, size_t nsegs, NClist* segments, NC_GRP_INFO_T** grpp)
{
    int i, j, found, stat = NC_NOERR;
    NC_GRP_INFO_T* grp = NULL;

    grp = file->root_grp;
    for(i=0;i<nsegs;i++) {
	const char* segment = nclistget(segments,i);
	char norm_name[NC_MAX_NAME];
	found = 0;
	if((stat = nc4_check_name(segment,norm_name))) goto done;
	for(j=0;j<ncindexsize(grp->children);j++) {
	    NC_GRP_INFO_T* subgrp = (NC_GRP_INFO_T*)ncindexith(grp->children,j);
	    if(strcmp(subgrp->hdr.name,norm_name)==0) {
		grp = subgrp;
		found = 1;
		break;
	    }
	}
	if(!found) {stat = NC_ENOGRP; goto done;}	
    }
    /* grp should be group of interest */
    if(grpp) *grpp = grp;

done:
    return THROW(stat);
}

static int
parsedimrefs(NC_FILE_INFO_T* file, NClist* dimrefs, NC_DIM_INFO_T** dims)
{
    int i, stat = NC_NOERR;
    NClist* segments = NULL;

    for(i=0;i<nclistlength(dimrefs);i++) {
	NC_GRP_INFO_T* g = NULL;
	NC_DIM_INFO_T* d = NULL;
	int j;
	const char* dimpath = nclistget(dimrefs,i);
	const char* dimname = NULL;
	/* Locate the corresponding NC_DIM_INFO_T* object */
	/* Clear the list */
	nclistfreeall(segments);
	segments = nclistnew();
	if((stat = ncz_splitkey(dimpath,segments)))
	    goto done;
	if((stat=locategroup(file,nclistlength(segments)-1,segments,&g)))
	    goto done;
	/* Lookup the dimension */
	dimname = nclistget(segments,nclistlength(segments)-1);
	d = NULL;
	for(j=0;j<ncindexsize(g->dim);j++) {
	    d = (NC_DIM_INFO_T*)ncindexith(g->dim,j);
	    if(strcmp(d->hdr.name,dimname)==0) {
		dims[i] = d;/* match */
		break;
	    }
	}
    }
done:
    nclistfreeall(segments);
    return THROW(stat);
}

/**
 * @internal Get the metadata for a variable.
 *
 * @param var Pointer to var info struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOMEM Out of memory.
 * @return ::NC_EHDFERR HDF5 returned error.
 * @return ::NC_EVARMETA Error with var metadata.
 * @author Ed Hartnett
 */
int
ncz_get_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int retval = NC_NOERR;

    assert(file && var && var->format_var_info);
    LOG((3, "%s: var %s", __func__, var->hdr.name));

    /* Have we already read the var metadata? */
    if (var->meta_read)
	return NC_NOERR;

#ifdef LOOK
    /* Get the current chunk cache settings. */
    if ((access_pid = H5Dget_access_plist(hdf5_var->hdf_datasetid)) < 0)
	BAIL(NC_EVARMETA);

    /* Learn about current chunk cache settings. */
    if ((H5Pget_chunk_cache(access_pid, &(var->chunk_cache_nelems),
			    &(var->chunk_cache_size), &rdcc_w0)) < 0)
	BAIL(NC_EHDFERR);
    var->chunk_cache_preemption = rdcc_w0;

    /* Get the dataset creation properties. */
    if ((propid = H5Dget_create_plist(hdf5_var->hdf_datasetid)) < 0)
	BAIL(NC_EHDFERR);

    /* Get var chunking info. */
    if ((retval = get_chunking_info(propid, var)))
	BAIL(retval);

    /* Get filter info for a var. */
    if ((retval = get_filter_info(propid, var)))
	BAIL(retval);

    /* Get fill value, if defined. */
    if ((retval = get_fill_info(propid, var)))
	BAIL(retval);

    /* Is this a deflated variable with a chunksize greater than the
     * current cache size? */
    if ((retval = nc4_adjust_var_cache(var->container, var)))
	BAIL(retval);

    if (var->coords_read && !var->dimscale)
	if ((retval = get_attached_info(var, hdf5_var, var->ndims, hdf5_var->hdf_datasetid)))
	    return retval;
#endif

    /* Remember that we have read the metadata for this var. */
    var->meta_read = NC_TRUE;

    return retval;
}

int
ncz_create_superblock(NCZ_FILE_INFO_T* zinfo)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;
    NCZMAP* map = NULL;
    char version[1024];

    map = zinfo->map;

    /* create superblock json */
    if((stat = NCJnew(NCJ_DICT,&json)))
	goto done;

    /* fill */
    snprintf(version,sizeof(version),"%d",zinfo->zarr.zarr_version);
    if((stat = NCJaddstring(json,NCJ_STRING,"zarr_format"))) goto done;
    if((stat = NCJaddstring(json,NCJ_INT,version))) goto done;
    if((stat = NCJaddstring(json,NCJ_STRING,"nczarr_version"))) goto done;
    {
	char ver[1024];
	snprintf(ver,sizeof(ver),"%lu.%lu.%lu",
	   zinfo->zarr.nczarr_version.major,
	   zinfo->zarr.nczarr_version.minor,
	   zinfo->zarr.nczarr_version.release);
	if((stat = NCJaddstring(json,NCJ_STRING,ver))) goto done;
    }
    if(!(zinfo->features.flags & FLAG_PUREZARR)) {
	/* Write back to map */
	if((stat=NCZ_uploadjson(map,NCZMETAROOT,json)))
	    goto done;
    }
done:
    NCJreclaim(json);
    return THROW(stat);
}
