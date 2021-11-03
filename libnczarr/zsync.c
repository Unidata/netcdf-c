/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "zfilter.h"

#undef FILLONCLOSE

/* Forward */
static int ncz_collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jdimsp);
static int ncz_sync_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, int isclose);
static int ncz_jsonize_atts(NCindex* attlist, NCjson** jattrsp);

/**************************************************/
/**************************************************/
/* Synchronize functions to make map and memory be consistent.*/

/**
 * @internal Synchronize file metadata from memory to map.
 *
 * @param file Pointer to file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_sync_file(NC_FILE_INFO_T* file, int isclose)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    NC_UNUSED(isclose);

    LOG((3, "%s: file: %s", __func__, file->controller->path));
    ZTRACE(3,"file=%s isclose=%d",file->controller->path,isclose);

    /* Write out root group recursively */
    if((stat = ncz_sync_grp(file, file->root_grp, isclose)))
        goto done;

done:
    NCJreclaim(json);
    return ZUNTRACE(stat);
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
ncz_sync_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, int isclose)
{
    int i,stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    char version[1024];
    int purezarr = 0;
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    NCjson* json = NULL;
    NCjson* jgroup = NULL;
    NCjson* jdims = NULL;
    NCjson* jvars = NULL;
    NCjson* jsubgrps = NULL;
    NCjson* jsuper = NULL;
    NCjson* jtmp = NULL;

    LOG((3, "%s: dims: %s", __func__, key));

    zinfo = file->format_file_info;
    map = zinfo->map;

    purezarr = (zinfo->controls.flags & FLAG_PUREZARR)?1:0;

    /* Construct grp key */
    if((stat = NCZ_grpkey(grp,&fullpath)))
	goto done;

    if(!purezarr) {
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
        /* Create the "_NCZARR_GROUP" dict */
        if((stat = NCJnew(NCJ_DICT,&json)))
	    goto done;
        /* Insert the various dicts and arrays */
        if((stat = NCJinsert(json,"dims",jdims))) goto done;
        jdims = NULL; /* avoid memory problems */
        if((stat = NCJinsert(json,"vars",jvars))) goto done;
        jvars = NULL; /* avoid memory problems */
        if((stat = NCJinsert(json,"groups",jsubgrps))) goto done;
        jsubgrps = NULL; /* avoid memory problems */
    }

    /* build ZGROUP contents */
    if((stat = NCJnew(NCJ_DICT,&jgroup)))
	goto done;
    snprintf(version,sizeof(version),"%d",zinfo->zarr.zarr_version);
    if((stat = NCJaddstring(jgroup,NCJ_STRING,"zarr_format"))) goto done;
    if((stat = NCJaddstring(jgroup,NCJ_INT,version))) goto done;
    if(!purezarr && grp->parent == NULL) { /* Root group */
        snprintf(version,sizeof(version),"%lu.%lu.%lu",
		 zinfo->zarr.nczarr_version.major,
		 zinfo->zarr.nczarr_version.minor,
		 zinfo->zarr.nczarr_version.release);
	if((stat = NCJnew(NCJ_DICT,&jsuper))) goto done;	
	if((stat-NCJnewstring(NCJ_STRING,version,&jtmp))) goto done;
	if((stat = NCJinsert(jsuper,"version",jtmp))) goto done;
	jtmp = NULL;
	if((stat = NCJinsert(jgroup,NCZ_V2_SUPERBLOCK,jsuper))) goto done;
	jsuper = NULL;
    }

    if(!purezarr) {
        /* Insert the "_NCZARR_GROUP" dict */    
        if((stat = NCJinsert(jgroup,NCZ_V2_GROUP,json))) goto done;
        json = NULL;
    }

    /* build ZGROUP path */
    if((stat = nczm_concat(fullpath,ZGROUP,&key)))
	goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jgroup)))
	goto done;
    nullfree(key); key = NULL;

    /* Build the .zattrs object */
    assert(grp->att);
    if((stat = ncz_sync_atts(file,(NC_OBJ*)grp, grp->att, isclose)))
	goto done;

    /* Now synchronize all the variables */
    for(i=0; i<ncindexsize(grp->vars); i++) {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	if((stat = ncz_sync_var(file,var,isclose))) goto done;
    }

    /* Now recurse to synchronize all the subgrps */
    for(i=0; i<ncindexsize(grp->children); i++) {
	NC_GRP_INFO_T* g = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = ncz_sync_grp(file,g,isclose))) goto done;
    }

done:
    NCJreclaim(jtmp);
    NCJreclaim(jsuper);
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
 * @internal Synchronize variable meta data from memory to map.
 *
 * @param file Pointer to file struct
 * @param var Pointer to var struct
 * @param isclose If this called as part of nc_close() as opposed to nc_enddef().
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_sync_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, int isclose)
{
    int i,stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    char number[1024];
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    char* dimpath = NULL;
    NClist* dimrefs = NULL;
    NCjson* jvar = NULL;
    NCjson* jncvar = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jtmp = NULL;
    NCjson* jfill = NULL;
    size64_t shape[NC_MAX_VAR_DIMS];
    NCZ_VAR_INFO_T* zvar = var->format_var_info;
    NClist* filterchain = NULL;
    NCjson* jfilter = NULL;
	    
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
    /* but might be scalar */
    if(var->ndims == 0)
        shape[0] = 1;

    /* shape key */
    /* Integer list defining the length of each dimension of the array.*/
    /* Create the list */
    if((stat = NCJnew(NCJ_ARRAY,&jtmp))) goto done;
    for(i=0;i<var->ndims+zvar->scalar;i++) {
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
    for(i=0;i<(var->ndims+zvar->scalar);i++) {
	size64_t len = (var->storage == NC_CONTIGUOUS ? shape[i] : var->chunksizes[i]);
	snprintf(number,sizeof(number),"%lld",len);
	NCJaddstring(jtmp,NCJ_INT,number);
    }
    if((stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;
    
    /* fill_value key */
    if(var->no_fill) {
	if((stat=NCJnew(NCJ_NULL,&jfill))) goto done;
    } else {/*!var->no_fill*/
	int atomictype = var->type_info->hdr.id;
	if((stat = ncz_ensure_fill_value(var))) goto done;
        assert(var->fill_value != NULL);
        /* Convert var->fill_value to a string */
	if((stat = NCZ_stringconvert(atomictype,1,var->fill_value,&jfill))) goto done;
	assert(jfill->sort != NCJ_ARRAY);
    }
    if((stat = NCJinsert(jvar,"fill_value",jfill))) goto done;
    jfill = NULL;

    /* order key */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"order"))) goto done;
    /* "C" means row-major order, i.e., the last dimension varies fastest;
       "F" means column-major order, i.e., the first dimension varies fastest.*/
    /* Default to C for now */ 
    if((stat = NCJaddstring(jvar,NCJ_STRING,"C"))) goto done;

    /* Compressor and Filters */
    filterchain = (NClist*)var->filters;    

    /* compressor key */
    /* From V2 Spec: A JSON object identifying the primary compression codec and providing
       configuration parameters, or ``null`` if no compressor is to be used. */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"compressor"))) goto done;
    if(nclistlength(filterchain) > 0) {
	struct NCZ_Filter* filter = (struct NCZ_Filter*)nclistget(filterchain,nclistlength(filterchain)-1);
        /* encode up the compressor */
        if((stat = NCZ_filter_jsonize(file,var,filter,&jtmp))) goto done;
    } else { /* no filters at all */
        /* Default to null */ 
        if((stat = NCJnew(NCJ_NULL,&jtmp))) goto done;
    }
    if(jtmp && (stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;

    /* filters key */
    /* From V2 Spec: A list of JSON objects providing codec configurations,
       or null if no filters are to be applied. Each codec configuration
       object MUST contain a "id" key identifying the codec to be used. */
    /* A list of JSON objects providing codec configurations, or ``null``
       if no filters are to be applied. */
    if((stat = NCJaddstring(jvar,NCJ_STRING,"filters"))) goto done;
    if(nclistlength(filterchain) > 1) {
	int k;
	/* jtmp holds the array of filters */
	if((stat = NCJnew(NCJ_ARRAY,&jtmp))) goto done;
	for(k=0;k<nclistlength(filterchain)-1;k++) {
 	    struct NCZ_Filter* filter = (struct NCZ_Filter*)nclistget(filterchain,k);
	    /* encode up the filter as a string */
	    if((stat = NCZ_filter_jsonize(file,var,filter,&jfilter))) goto done;
	    if((stat = NCJappend(jtmp,jfilter))) goto done;
	}
    } else { /* no filters at all */
        if((stat = NCJnew(NCJ_NULL,&jtmp))) goto done;
    }
    if((stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;

    /* dimension_separator key */
    /* Single char defining the separator in chunk keys */
    if(zvar->dimension_separator != DFALT_DIM_SEPARATOR) {
	char sep[2];
	sep[0] = zvar->dimension_separator;/* make separator a string*/
	sep[1] = '\0';
        if((stat = NCJnewstring(NCJ_STRING,sep,&jtmp))) goto done;
        if((stat = NCJinsert(jvar,"dimension_separator",jtmp))) goto done;
        jtmp = NULL;
    }
    
    /* Capture dimref names as FQNs */
    if(var->ndims > 0) {
        if((dimrefs = nclistnew())==NULL) {stat = NC_ENOMEM; goto done;}
	for(i=0;i<var->ndims;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    if((stat = NCZ_dimkey(dim,&dimpath))) goto done;
	    nclistpush(dimrefs,dimpath);
	    dimpath = NULL;
	}
    }

    /* Build the NCZ_V2_ARRAY object */
    {
	/* Create the dimrefs json object */
	if((stat = NCJnew(NCJ_ARRAY,&jdimrefs)))
	    goto done;
	for(i=0;i<nclistlength(dimrefs);i++) {
	    const char* dim = nclistget(dimrefs,i);
	    NCJaddstring(jdimrefs,NCJ_STRING,dim);
	}
	if((stat = NCJnew(NCJ_DICT,&jncvar)))
	    goto done;
	
	/* Insert dimrefs  */
	if((stat = NCJinsert(jncvar,"dimrefs",jdimrefs)))
	    goto done;
	jdimrefs = NULL; /* Avoid memory problems */

	/* Add the _Storage flag */
	/* Record if this is a scalar; use the storage field */	
	if(var->ndims == 0) {
	    if((stat = NCJnewstring(NCJ_STRING,"scalar",&jtmp)))goto done;
	} else if(var->storage == NC_CONTIGUOUS) {
	     if((stat = NCJnewstring(NCJ_STRING,"contiguous",&jtmp)))goto done;
	} else if(var->storage == NC_COMPACT) {
	     if((stat = NCJnewstring(NCJ_STRING,"compact",&jtmp)))goto done;
	} else {/* chunked */
	     if((stat = NCJnewstring(NCJ_STRING,"chunked",&jtmp)))goto done;
	}	
	if((stat = NCJinsert(jncvar,"storage",jtmp))) goto done;
	jtmp = NULL;

	if(!(zinfo->controls.flags & FLAG_PUREZARR)) {
	    if((stat = NCJinsert(jvar,NCZ_V2_ARRAY,jncvar))) goto done;
	    jncvar = NULL;
	}
    }

    /* build .zarray path */
    if((stat = nczm_concat(fullpath,ZARRAY,&key)))
	goto done;

    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jvar)))
	goto done;
    nullfree(key); key = NULL;

    /* Build .zattrs object */
    assert(var->att);
    if((stat = ncz_sync_atts(file,(NC_OBJ*)var, var->att, isclose)))
	goto done;

done:
    nclistfreeall(dimrefs);
    nullfree(fullpath);
    nullfree(key);
    nullfree(dimpath);
    NCJreclaim(jvar);
    NCJreclaim(jncvar);
    NCJreclaim(jtmp);
    NCJreclaim(jfill);
    return THROW(stat);
}

/**
 * @internal Synchronize variable meta data and data from memory to map.
 *
 * @param file Pointer to file struct
 * @param var Pointer to var struct
 * @param isclose If this called as part of nc_close() as opposed to nc_enddef().
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_sync_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, int isclose)
{
    int stat = NC_NOERR;
    NCZ_VAR_INFO_T* zvar = var->format_var_info;
    
    if(isclose) {
	if((stat = ncz_sync_var_meta(file,var,isclose))) goto done;
    }

    /* flush only chunks that have been written */
    if(zvar->cache) {
        if((stat = NCZ_flush_chunk_cache(zvar->cache)))
	    goto done;
    }

done:
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

    if(var->ndims == 0) { /* scalar */
	start[i] = 0;
	stop[i] = 1;
        stride[i] = 1;
    } else {
        for(i=0;i<var->ndims;i++) {
	    size64_t nchunks = ceildiv(var->dim[i]->len,var->chunksizes[i]);
	    start[i] = 0;
	    stop[i] = nchunks;
	    stride[i] = 1;
        }
    }

	/* Iterate over all the chunks to create missing ones */
	if((chunkodom = nczodom_new(var->ndims+zvar->scalar,start,stop,stride,stop))==NULL)
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
	    assert(zvar->cache->fillchunk != NULL);
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
ncz_sync_atts(NC_FILE_INFO_T* file, NC_OBJ* container, NCindex* attlist, int isclose)
{
    int i,stat = NC_NOERR;
    NC_VAR_INFO_T* var = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCjson* jatts = NULL;
    NCjson* jtypes = NULL;
    NCjson* jtype = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jdict = NULL;
    NCjson* jint = NULL;
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;
    char* content = NULL;
    char* dimpath = NULL;
    int isxarray = 0;
    int isrootgroup = 0;

    LOG((3, "%s", __func__));

    zinfo = file->format_file_info;
    map = zinfo->map;

    if(zinfo->controls.flags & FLAG_XARRAYDIMS) isxarray = 1;

    if(container->sort == NCVAR) {
	var = (NC_VAR_INFO_T*)container;
	if(var->container && var->container->parent == NULL)
	    isrootgroup = 1;
    }

    if(!isxarray && ncindexsize(attlist) == 0) 
	goto done; /* do nothing */

    if(ncindexsize(attlist) > 0) {
        /* Create the jncattr.types object */
        if((stat = NCJnew(NCJ_DICT,&jtypes)))
	    goto done;
        /* Walk all the attributes and collect the types by attribute name */
        for(i=0;i<ncindexsize(attlist);i++) {
	    NC_ATT_INFO_T* a = (NC_ATT_INFO_T*)ncindexith(attlist,i);
	    const char* tname = NULL;
	    const NC_reservedatt* ra = NC_findreserved(a->hdr.name);
	    /* If reserved and hidden, then ignore */
	    if(ra && (ra->flags & HIDDENATTRFLAG)) continue;
	    if(a->nc_typeid >= NC_STRING)
	        {stat = THROW(NC_ENCZARR); goto done;}	
	    if((stat = ncz_zarr_type_name(a->nc_typeid,1,&tname))) goto done;
	    if((stat = NCJnewstring(NCJ_STRING,tname,&jtype)))
	        goto done;
	    if((stat = NCJinsert(jtypes,a->hdr.name,jtype))) /* add {name: type} */
	        goto done;
	    jtype = NULL;
        }
    }
    
    /* Construct container path */
    if(container->sort == NCGRP)
	stat = NCZ_grpkey((NC_GRP_INFO_T*)container,&fullpath);
    else
	stat = NCZ_varkey((NC_VAR_INFO_T*)container,&fullpath);
    if(stat)
	goto done;

    /* Jsonize the attribute list */
    if((stat = ncz_jsonize_atts(attlist,&jatts)))
	goto done;

    if(var) {
        if(isrootgroup && isxarray) {
	    NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)container;
	    /* Insert the XARRAY _ARRAY_ATTRIBUTE attribute */
	    if((stat = NCJnew(NCJ_ARRAY,&jdimrefs)))
	        goto done;
	    /* Walk the dimensions and capture the names */
	    for(i=0;i<var->ndims;i++) {
	        NC_DIM_INFO_T* dim = var->dim[i];
		char* dimname = strdup(dim->hdr.name);
		if(dimname == NULL) {stat = NC_ENOMEM; goto done;}
	        NCJaddstring(jdimrefs,NCJ_STRING,dimname);
   	        nullfree(dimname); dimname = NULL;
	    }
	    /* Add the _ARRAY_DIMENSIONS attribute */
	    if((stat = NCJinsert(jatts,NC_XARRAY_DIMS,jdimrefs))) goto done;
	    jdimrefs = NULL;
        }
    }
    if(!(zinfo->controls.flags & FLAG_PUREZARR)) {
	/* Insert the _NCZARR_ATTR attribute */
        if((stat = NCJnew(NCJ_DICT,&jdict)))
	        goto done;
	if((stat = NCJinsert(jdict,"types",jtypes))) goto done;
	jtypes = NULL;
	if((stat = NCJinsert(jatts,NCZ_V2_ATTR,jdict))) goto done;	
	jdict = NULL;
    }

    /* Add Quantize Attribute */
    if(var && var->quantize_mode > 0) {    
	char mode[64];
	snprintf(mode,sizeof(mode),"%d",var->nsd);
        if((stat = NCJnewstring(NCJ_INT,mode,&jint)))
	        goto done;
	/* Insert the quantize attribute */
	switch (var->quantize_mode) {
	case NC_QUANTIZE_BITGROOM:
	    if((stat = NCJinsert(jatts,NC_QUANTIZE_ATT_NAME,jint))) goto done;	
	    jint = NULL;
	    break;
	default: break;
	}
    }

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
    NCJreclaim(jdimrefs);
    NCJreclaim(jdict);
    NCJreclaim(jint);
    return THROW(stat);
}

/**
@internal Convert a list of attributes to corresponding json.
Note that this does not push to the file.
Also note that attributes of length 1 are stored as singletons, not arrays.
This is to be more consistent with pure zarr. 
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
