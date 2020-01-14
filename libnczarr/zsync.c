/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/* Forward */
static int ncz_collect_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jdimsp);
static int ncz_sync_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);

static int ncz_jsonize_atts(NCindex* attlist, NCjson** jattrsp);
static int load_jatts(NCZMAP* map, NC_OBJ* container, NCjson** jattrsp, NClist** atypes);
static int zconvert(nc_type typeid, size_t typelen, void* dst, NCjson* src);
static int computeattrinfo(const char* name, NClist* atypes, NCjson* values,
		nc_type* typeidp, size_t* lenp, void** datap);
static int parse_group_content(NCjson* jcontent, NClist* dimdefs, NClist* varnames, NClist* subgrps);
static int define_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
static int define_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* diminfo);
static int define_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames);
static int define_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* subgrpnames);
static int searchvars(NC_FILE_INFO_T*, NC_GRP_INFO_T*, NClist*);
static int searchsubgrps(NC_FILE_INFO_T*, NC_GRP_INFO_T*, NClist*);
static int locategroup(NC_FILE_INFO_T* file, size_t nsegs, NClist* segments, NC_GRP_INFO_T** grpp);
static int parsedimrefs(NC_FILE_INFO_T*, NClist* dimrefs, NC_DIM_INFO_T** dims);
static int simulatedimrefs(NC_FILE_INFO_T* file, int rank, size64_t* shapes, NC_DIM_INFO_T** dims);
static int decodeints(NCjson* jshape, size64_t* shapes);
static int computeattrdata(nc_type typeid, NCjson* values, size_t* lenp, void** datap);

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
 * @internal Synchronize file data from memory to map.
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
    char version[1024];
    NCjson* json = NULL;
    NCZMAP* map = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;

    LOG((3, "%s: file: %s", __func__, file->controller->path));

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Overwrite in all cases */
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
    /* Write back to map */
    if((stat=NCZ_uploadjson(map,NCZMETAROOT,json)))
	goto done;

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

    /* Construct grp path */
    if((stat = NCZ_grppath(grp,&fullpath)))
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

    /* build ZGROUP path */
    if((stat = nczm_suffix(fullpath,ZGROUP,&key)))
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

    /* build NCZGROUP path */
    if((stat = nczm_suffix(fullpath,NCZGROUP,&key)))
	goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jgroup)))
	goto done;
    nullfree(key); key = NULL;

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
    ssize64_t shape[NC_MAX_VAR_DIMS];

    LOG((3, "%s: dims: %s", __func__, key));

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Construct var path */
    if((stat = NCZ_varpath(var,&fullpath)))
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
	shape[i] = (ssize64_t)dim->len;
    }

    /* shape key */
    /* Integer list defining the length of each dimension of the array.*/
    /* Create the list */
    if((stat = NCJnew(NCJ_ARRAY,&jtmp))) goto done;
    for(i=0;i<var->ndims;i++) {
	snprintf(number,sizeof(number),"%lld",shape[i]);
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
	int islittle = (endianness == NC_ENDIAN_LITTLE
			|| NCZ_isLittleEndian());
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
	ssize64_t len = (var->contiguous?shape[i]:(ssize64_t)var->chunksizes[i]);
	snprintf(number,sizeof(number),"%lld",len);
	NCJaddstring(jtmp,NCJ_INT,number);
    }
    if((stat = NCJappend(jvar,jtmp))) goto done;
    jtmp = NULL;
    
    /* fill_value key */
    if(!var->no_fill) {
	const char* strfill = NULL;
	int fillsort;
	int atomictype = var->type_info->hdr.id;
	NCjson* jfill = NULL;
        /* A scalar value providing the default value to use for uninitialized
           portions of the array, or ``null`` if no fill_value is to be used. */
        /* Use the defaults defined in netdf.h */
	assert(atomictype > 0 && atomictype <= NC_MAX_ATOMIC_TYPE && atomictype != NC_STRING);
	if((stat = ncz_fill_value_sort(atomictype,&fillsort))) goto done;
	if(var->fill_value == NULL) { /* use default */
	    if((stat = ncz_default_fill_value(atomictype,&strfill))) goto done;
            if((stat = NCJnewstring(fillsort,strfill,&jfill))) goto done;
	} else { /* use fill_value */
	    /* Convert var->fill_value to a string */
	    if((stat = NCZ_stringconvert(atomictype,1,var->fill_value,&jfill)))
		goto done;
	}
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
    if((stat = nczm_suffix(fullpath,ZARRAY,&key)))
	goto done;

    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jvar)))
	goto done;

    /* Build the NCZVAR object */
    {
	/* Create the dimrefs json object */
	if((stat = NCJnew(NCJ_ARRAY,&jdimrefs)))
	    goto done;
	/* Walk the dimensions and capture the fullpath names */
	for(i=0;i<var->ndims;i++) {
	    NC_DIM_INFO_T* dim = var->dim[i];
	    if((stat = NCZ_dimpath(dim,&dimpath))) goto done;
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

	/* Add the contiguous flag */
	if((stat = NCJnewstring(NCJ_BOOLEAN,
		(var->contiguous?"true":"false"),
		&jtmp))) goto done;
	if((stat = NCJinsert(jncvar,"contiguous",jtmp))) goto done;
	jtmp = NULL;

	/* Write out NCZVAR */
        if((stat = nczm_suffix(fullpath,NCZVAR,&key)))
	    goto done;
        /* Write to map */
        if((stat=NCZ_uploadjson(map,key,jncvar)))
	    goto done;
    }

    /* Build .zattrs object including _nczattr object */
    assert(var->att);
    if((stat = ncz_sync_atts(file,(NC_OBJ*)var, var->att)))
	goto done;

done:
    nullfree(fullpath);
    nullfree(key);
    nullfree(dimpath);
    NCJreclaim(jvar);
    NCJreclaim(jtmp);
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
	stat = NCZ_grppath((NC_GRP_INFO_T*)container,&fullpath);
    else
	stat = NCZ_varpath((NC_VAR_INFO_T*)container,&fullpath);
    if(stat)
	goto done;

    /* Upload the NCZATTR object */
    if((stat = nczm_suffix(fullpath,NCZATTR,&key)))
	goto done;
    /* Write to map */
    if((stat=NCZ_uploadjson(map,key,jnczattr)))
	goto done;

    /* Jsonize the attribute list */
    if((stat = ncz_jsonize_atts(attlist,&jatts)))
	goto done;

    /* write .zattrs path */
    if((stat = nczm_suffix(fullpath,ZATTRS,&key)))
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
	if((stat = NCZ_grppath(grp,&fullpath)))
	    goto done;
    } else {
	NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)container;
	/* Get var's fullpath name */
	if((stat = NCZ_varpath(var,&fullpath)))
	    goto done;
    }

    /* Construct the path to the .zattrs object */
    if((stat = nczm_suffix(fullpath,ZATTRS,&key)))
	goto done;

    /* Download the .zattrs object: may not exist */
    switch ((stat=NCZ_downloadjson(map,key,&jattrs))) {
    case NC_NOERR: break;
    case NC_EACCESS: stat = NC_NOERR; break; /* did not exist */
    default: goto done; /* failure */
    }
    nullfree(key); key = NULL;

    if(jattrs != NULL) {
        /* Construct the path to the NCZATTR object */
        if((stat = nczm_suffix(fullpath,NCZATTR,&key))) goto done;
        /* Download the NCZATTR object: may not exist if pure zarr */
        switch((stat=NCZ_downloadjson(map,key,&jncattr))) {
        case NC_NOERR: break;
        case NC_EACCESS: stat = NC_NOERR; jncattr = NULL; break;
        default: goto done; /* failure */
        }
        nullfree(key); key = NULL;

        if(jncattr == NULL) { /* Pure Zarr, fake it */
	    assert(0);
        } else {
	    NCjson* jtypes = NULL;
   	    /* jncattr attribute should be a dict */
            if(jncattr->sort != NCJ_DICT) {stat = THROW(NC_ENCZARR); goto done;}
  	    /* Extract "types */
            if((stat = NCJdictget(jncattr,"types",&jtypes))) goto done;
            if(jtypes->sort != NCJ_DICT) {stat = THROW(NC_ENCZARR); goto done;}
  	    /* Convert to an envv list */
	    for(i=0;i<nclistlength(jtypes->dict);i+=2) {
	        const NCjson* key = nclistget(jtypes->dict,i);
	        const NCjson* value = nclistget(jtypes->dict,i+1);
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
	for(i=0;i<nclistlength(src->array);i++) {
	    NCjson* value = nclistget(src->array,i);
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
    if(typeid == NC_NAT || typeid >= NC_STRING)
	{stat = NC_EINTERNAL; goto done;}

    if((stat = computeattrdata(typeid, values, &len, &data))) goto done;

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
computeattrdata(nc_type typeid, NCjson* values, size_t* lenp, void** datap)
{
    int stat = NC_NOERR;
    size_t len;
    void* data = NULL;
    size_t typelen;

    /* Now, collect the length of the attribute */
    switch (values->sort) {
    case NCJ_DICT: stat = NC_EINTERNAL; goto done;
    case NCJ_ARRAY:
	len = nclistlength(values->array);
	break;
    case NCJ_STRING: /* requires special handling as an array of characters */
	len = strlen(values->value);
	break;
    default:
	len = 1;
	break;
    }

    /* Allocate data space */
    if((stat = NC4_inq_atomic_type(typeid, NULL, &typelen)))
	goto done;
    if((data = malloc(typelen*len)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* convert to target type */	
    if((stat = zconvert(typeid, typelen, data, values)))
	goto done;

    if(lenp) *lenp = len;
    if(datap) {*datap = data; data = NULL;}

done:
    nullfree(data);
    return THROW(stat);
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

    LOG((3, "%s: dims: %s", __func__, key));

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Construct grp path */
    if((stat = NCZ_grppath(grp,&fullpath)))
	goto done;

    /* build NCZGROUP path */
    if((stat = nczm_suffix(fullpath,NCZGROUP,&key)))
	goto done;
    /* Read */
    switch (stat=NCZ_downloadjson(map,key,&jncgroup)) {
    case NC_NOERR: break; /* we read it */
    case NC_EACCESS: /* probably pure zarr, so does not exist, use search */
	nclistclear(varnames);
	if((stat = searchvars(file,grp,varnames)))
	nclistclear(subgrps);
	if((stat = searchsubgrps(file,grp,subgrps)))
	break;
    default: goto done;
    }
    nullfree(key); key = NULL;

    /* Pull out lists about group content */
    if((stat = parse_group_content(jncgroup,dimdefs,varnames,subgrps)))
	goto done;

    /* Define dimensions */
    if((stat = define_dims(file,grp,dimdefs))) goto done;

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
    case NC_EACCESS: stat = NC_NOERR; break; /* container has no attributes */
    default: goto done; /* true error */
    }

    if(jattrs != NULL) {
        /* Iterate over the attributes to create the in-memory attributes */
        /* Watch for reading _FillValue */
        for(i=0;i<nclistlength(jattrs->dict);i+=2) {
            NCjson* key = nclistget(jattrs->dict,i);
            NCjson* value = nclistget(jattrs->dict,i+1);
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
        ssize64_t len = 0;
        const char* name = nclistget(diminfo,i);
        const char* value = nclistget(diminfo,i+1);

        /* Create the NC_DIM_INFO_T object */
        sscanf(value,"%lld",&len); /* Get length */
        if(len < 0) {stat = NC_EDIMSIZE; goto done;}
        if((stat = nc4_dim_list_add(grp, name, (size_t)len, -1, &dim)))
            goto done;
        if((dim->format_dim_info = calloc(1,sizeof(NCZ_DIM_INFO_T))) == NULL)
            {stat = NC_ENOMEM; goto done;}
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
    NCZMAP* map = NULL;
    NClist* segments = nclistnew();
    NCjson* jvar = NULL;
    NCjson* jncvar = NULL;
    NCjson* jdimrefs = NULL;
    NCjson* jvalue = NULL;
    NClist* dimrefs = NULL;
    int hasdimrefs;
    int contiguous = 0;

    zinfo = file->format_file_info;
    map = zinfo->map;

    /* Load each var in turn */
    for(i = 0; i < nclistlength(varnames); i++) {
        NC_VAR_INFO_T* var;
        const char* varname = nclistget(varnames,i);

        /* Clean up from last cycle */
        nclistfreeall(dimrefs);
        dimrefs = nclistnew();

        /* Create the NC_VAR_INFO_T object */
        if((stat = nc4_var_list_add2(grp, varname, &var)))
            goto done;

        /* And its annotation */
        if((var->format_var_info = calloc(1,sizeof(NCZ_VAR_INFO_T)))==NULL)
            {stat = NC_ENOMEM; goto done;}

        /* Construct var path */
        if((stat = NCZ_varpath(var,&varpath)))
            goto done;

        /* Construct the path to the zarray object */
        if((stat = nczm_suffix(varpath,ZARRAY,&key)))
            goto done;
        /* Download the zarray object */
        if((stat=NCZ_readdict(map,key,&jvar)))
            goto done;
        nullfree(key); key = NULL;
        assert((jvar->sort == NCJ_DICT));
        /* Download the NCZVAR object */
        if((stat = nczm_suffix(varpath,NCZVAR,&key))) goto done;
        if((stat=NCZ_readdict(map,key,&jncvar))) goto done;
        nullfree(key); key = NULL;
        assert((jncvar->sort == NCJ_DICT));

        /* Extract contiguous flag */
        contiguous = 0;
        if((stat = NCJdictget(jncvar,"contiguous",&jvalue)) == NC_NOERR
           && jvalue != NULL && strcmp(jvalue->value,"true") == 0)
            contiguous = 1;

        /* Extract dimrefs list  */
        switch ((stat = NCJdictget(jncvar,"dimrefs",&jdimrefs))) {
        case NC_NOERR: /* Extract the dimref names */
            assert((jdimrefs->sort == NCJ_ARRAY));
            hasdimrefs = 0; /* until we have one */     
            for(j=0;j<nclistlength(jdimrefs->array);j++) {
                const NCjson* dimpath = nclistget(jdimrefs->array,j);
                assert(dimpath->sort == NCJ_STRING);
                nclistpush(dimrefs,strdup(dimpath->value));
                hasdimrefs = 1;
            }
            break;
        case NC_EACCESS: /* simulate it from the shape of the variable */
	    stat = NC_NOERR;
            hasdimrefs = 0;
            break;
        default: goto done;
        }
        jdimrefs = NULL;

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
                if((stat = ncz_gettype(vtype,&var->type_info)))
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
            rank = nclistlength(jvalue->array);
            if(hasdimrefs) { /* verify rank consistency */
                if(nclistlength(dimrefs) != rank)
                    {stat = THROW(NC_ENCZARR); goto done;}
            }
            /* Set the rank of the variable */
            if((stat = nc4_var_set_ndims(var, rank))) goto done;
            if(hasdimrefs) {
                if((stat = parsedimrefs(file,dimrefs, var->dim)))
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
            rank = nclistlength(jvalue->array);
            var->contiguous = (rank == 0) || contiguous;
            if(rank > 0) {
                if(var->ndims != rank)
                    {stat = THROW(NC_ENCZARR); goto done;}
                if((var->chunksizes = malloc(sizeof(size_t)*rank)) == NULL)
                    {stat = NC_ENOMEM; goto done;}
                if((stat = decodeints(jvalue, chunks))) goto done;
                /* validate the chunk sizes */
                for(j=0;j<rank;j++) {
                    NC_DIM_INFO_T* d = var->dim[j]; /* matching dim */
                    if(chunks[j] == 0 || chunks[j] > d->len)
                        {stat = THROW(NC_ENCZARR); goto done;}
                    var->chunksizes[j] = (size_t)chunks[j];
                    if(var->contiguous && var->chunksizes[j] != d->len)
                        {stat = THROW(NC_ENCZARR); goto done;}
                }
            }
        }
        /* fill_value */
        {
            if((stat = NCJdictget(jvar,"fill_value",&jvalue))) goto done;
            if(jvalue == NULL)
                var->no_fill = 1;
            else {
                var->no_fill = 0;
                if((stat = computeattrdata(var->type_info->hdr.id, jvalue, NULL, &var->fill_value)))
                    goto done;
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
    }

done:
    nullfree(varpath);
    nullfree(key);
    nclistfreeall(segments);
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
        for(i=0;i<nclistlength(jvalue->dict);i+=2) {
            NCjson* jname = nclistget(jvalue->dict,i);
            NCjson* jlen = nclistget(jvalue->dict,i+1);
            char norm_name[NC_MAX_NAME + 1];
            ssize64_t len;
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
        for(i=0;i<nclistlength(jvalue->array);i++) {
            NCjson* jname = nclistget(jvalue->array,i);
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
        for(i=0;i<nclistlength(jvalue->array);i++) {
            NCjson* jname = nclistget(jvalue->array,i);
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
searchvars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames)
{
    return NC_NOERR;
}

static int
searchsubgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* subgrpnames)
{
    return NC_NOERR;
}

/* Convert a list of integer strings to integers */
static int
decodeints(NCjson* jshape, size64_t* shapes)
{
    int i, stat = NC_NOERR;

    for(i=0;i<nclistlength(jshape->array);i++) {
        long long v;
        NCjson* jv = nclistget(jshape->array,i);
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
        /* See if there is a dimension named "_dim<dimlen>", if not create */
        snprintf(shapename,sizeof(shapename),"_dim%llu",dimlen);
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
            if((stat = nc4_dim_list_add(root,shapename,(size_t)dimlen, -1, &thed)))
                goto done;
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
        if((stat = ncz_splitpath(dimpath,segments)))
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

