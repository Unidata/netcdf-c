/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "zfilter.h"
#include "znc4.h"

#ifndef nulldup
#define nulldup(x) ((x)?strdup(x):(x))
#endif

#undef FILLONCLOSE

/*mnemonics*/
#define DICTOPEN '{'
#define DICTCLOSE '}'
#define FIXED 0
#define UNLIM 1

/* Forward */
static int ncz_encode_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp);
static int ncz_encode_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
static int ncz_create_computed_var_attributes(NC_FILE_INFO_T* file,NC_VAR_INFO_T* var);
static int ncz_flush_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var);
static int ncz_decode_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* subgrpnames);
static int ncz_decode_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj0);
static int ncz_decode_var1(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* varname);
static int ncz_decode_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* varnames);
static int ncz_decode_atts(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts);

static int ncz_encode_filters(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj);
static int ncz_decode_filters(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const NClist* filters);

static int get_group_content_pure(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrps);
static int reifydimrefs(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NC_VAR_INFO_T* var, size64_t* shapes, NClist* dimrefs, NClist* dimdecls);
static int definedim(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* basename, size64_t shape, int isunlimited, NC_DIM_INFO_T** dimp);

/**************************************************/
/* Synchronize functions to make map and memory
be consistent. There are two sets of functions,
1) _encode_ - push nc4internal.h structures to storage
2) _decode_ - pull storage structures and create corresponding nc4internal.h structures.
            These functions rely on the format specific decode functions to extract
	    and return the relevant info from the format json.
*/
/**************************************************/

/*
 * @internal Push nc4internal.h structures to storage
 *
 * @param file Pointer to file info struct.
 * @param isclose 1 => we closing file as opposed to sync'ing it.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_encode_file(NC_FILE_INFO_T* file, int isclose)
{
    int stat = NC_NOERR;

    NC_UNUSED(isclose);

    LOG((3, "%s: file: %s", __func__, file->controller->path));
    ZTRACE(3,"file=%s isclose=%d",file->controller->path,isclose);

    /* Write out root group recursively */
    if((stat = ncz_encode_grp(file, file->root_grp)))
        goto done;

done:
    return ZUNTRACE(stat);
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
static int
ncz_encode_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_FILE_INFO_T* zinfo = NULL;
    int purezarr = 0;
    NCjson* jnczgrp = NULL;
    NCjson* jsuper = NULL;
    NCjson* jtypes = NULL;
    struct ZOBJ zobj = NCZ_emptyzobj();

    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);

    zinfo = file->format_file_info;
    TESTPUREZARR;

    /* Create|Update the dual attributes */
    if((stat = NCZ_ensure_dual_attributes(file,(NC_OBJ*)grp))) goto done;

    if(!purezarr) {
        if(grp->parent == NULL) { /* Root group */
	    if((stat=NCZF_encode_superblock(file,&jsuper))) goto done;
	}
        /* encode _nczarr_group */
	if((stat=NCZF_encode_nczarr_group(file,grp,&jnczgrp))) goto done;
    }


    /* Assemble group JSON object */
    if((stat=NCZF_encode_group(file,grp,&zobj.jobj))) goto done;

    /* Assemble JSON'ized attributes: Optionally uses _nczarr_group &/or _nczarr_superblock */
    if((stat = NCZF_encode_attributes(file,(NC_OBJ*)grp,&jnczgrp,&jsuper,&zobj.jatts))) goto done;

    /* upload group json and (depending on version) the group attributes */
    if((stat = NCZF_upload_grp(file,grp,&zobj))) goto done;

    /* encode and upload the vars in this group and sync the data */
    for(i=0;i<ncindexsize(grp->vars);i++) {
        NC_VAR_INFO_T* var = (NC_VAR_INFO_T*)ncindexith(grp->vars,i);
	if((stat = ncz_encode_var_meta(file,var))) goto done;
	if((stat = ncz_flush_var(file,var))) goto done;
    }
    
    /* encode and upload the sub-groups in this group */
    for(i=0;i<ncindexsize(grp->children);i++) {
        NC_GRP_INFO_T* subgrp = (NC_GRP_INFO_T*)ncindexith(grp->children,i);
	if((stat = ncz_encode_grp(file,subgrp))) goto done;
    }
    
done:
    NCZ_clear_zobj(&zobj);    
    NCZ_reclaim_json(jnczgrp);
    NCZ_reclaim_json(jtypes);
    NCZ_reclaim_json(jsuper);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Synchronize variable meta data from memory to map.
 *
 * @param file Pointer to file struct
 * @param var Pointer to var struct
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_encode_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCjson* jnczvar = NULL;
    int purezarr = 0;
    NCZ_VAR_INFO_T* zvar = var->format_var_info;
    NClist* filtersj = nclistnew();
    struct ZOBJ zobj = NCZ_emptyzobj();

    ZTRACE(3,"file=%s var=%s",file->controller->path,var->hdr.name);

    zinfo = file->format_file_info;
    TESTPUREZARR;

    /* Make sure that everything is established */

    /* ensure the fill value */
    if((stat = NCZ_ensure_fill_value(var))) goto done; /* ensure var->fill_value is set */
    assert(var->no_fill || var->fill_value != NULL);
    /* ensure the chunk cache */
    if((stat = NCZ_adjust_var_cache(var))) goto done;
    /* rebuild the fill chunk */
    if((stat = NCZ_ensure_fill_chunk(zvar->cache))) goto done;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    /* Build the filter working parameters for any filters */
    if((stat = NCZ_filter_setup(file,var))) goto done;
#endif

    /* There is a sort of cycle between _nczarr_array and the attributes in that
       the attributes must contain _nczarr_array as an attribute and _nczar_array
       must contain the attribute types (including _nczarr_array).
       We break this by building _nczarr_array first, then building the attributes.
    */     
    /* Build the _nczarr_array object */
    if(!purezarr) {
        if((stat=NCZF_encode_nczarr_array(file,var,&jnczvar))) goto done;
    }

    /* Some attributes need to be computed because they are not stored in the NC_XXX_INFO_T structs.
     *  The current such attributes are:
     * 1. xarray attribute
     */
    if((stat = ncz_create_computed_var_attributes(file,var))) goto done;

    /* Create|Update the dual attributes */
    if((stat = NCZ_ensure_dual_attributes(file,(NC_OBJ*)var))) goto done;

    /* Convert to JSON */
    if((stat=NCZF_encode_attributes(file,(NC_OBJ*)var,&jnczvar,NULL,&zobj.jatts))) goto done;

    /* Encode the filters */
    if((stat=ncz_encode_filters(file,var,filtersj))) goto done;

    /* encode the var JSON including (optionally) the attributes */
    if((stat=NCZF_encode_var(file,var,filtersj,&zobj.jobj))) goto done;

    /* Write out the the var JSON and the corresponding attributes and chunks */
    if((stat = NCZF_upload_var(file,var,&zobj))) goto done;
    var->created = 1;

done:
    NCZ_clear_zobj(&zobj);
    NCZ_reclaim_json(jnczvar);
    NCZ_reclaim_json_list(filtersj);
    return ZUNTRACE(THROW(stat));
}

/* Some attributes need to be computed because they are not stored in the NC_XXX_INFO_T structs.
 *  The current such attributes are:
 * 1. xarray attribute
 */
static int
ncz_create_computed_var_attributes(NC_FILE_INFO_T* file,NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    NC_GRP_INFO_T* parent = var->container;
    NC_ATT_INFO_T* special = NULL;
    char* xarraydims = NULL;
    int isnew = 0;

    if(parent->parent != NULL) goto done; /* Only do this for root group */

    if(zinfo->flags & FLAG_XARRAYDIMS) { /* test if we should generate xarray dimensions */
	special = NULL;
	isnew = 0;
	/* get/create the xarray attribute as type NC_CHAR */
	if((stat = NCZ_getattr(file,(NC_OBJ*)var,NC_XARRAY_DIMS,NC_CHAR,&special,&isnew))) goto done;
	if(isnew) {
	    size_t zarr_rank;
	    if((stat = NCZF_encode_xarray(file,var->ndims,var->dim,&xarraydims,&zarr_rank))) goto done;
	    if((stat = NCZ_set_att_data(file,special,strlen(xarraydims),xarraydims))) goto done;
	}
    }

done:
    nullfree(xarraydims);
    return THROW(stat);
}

/*
Flush all modified chunks to disk. Create any that are missing
and fill as needed.
*/
static int
ncz_flush_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;

    NC_UNUSED(file);

    ZTRACE(3,"var=%s",var->hdr.name);

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

    {
	if(zvar->scalar) {
	    if((chunkodom = nczodom_new(1,start,stop,stride,stop))==NULL)
	} else {
	    /* Iterate over all the chunks to create missing ones */
	    if((chunkodom = nczodom_new(var->ndims,start,stop,stride,stop))==NULL)
	        {stat = NC_ENOMEM; goto done;}
	}
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
    }
    nczodom_free(chunkodom);
    nullfree(key);
    }
#endif /*FILLONCLOSE*/

done:
    return ZUNTRACE(THROW(stat));
}

/**************************************************/
/*
 * @internal pull storage structures and create corresponding nc4internal.h structures
 */

/**

 * @param file Pointer to file info struct.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_decode_file(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = file->format_file_info;
    NC_GRP_INFO_T* root = NULL;
    const NCjson* jsuper = NULL;
    NClist* varnames = nclistnew();
    NClist* subgroupnames = nclistnew();
    struct ZOBJ zobj = NCZ_emptyzobj();

    LOG((3, "%s: file: %s", __func__, file->controller->path));
    ZTRACE(3,"file=%s",file->controller->path);
    
    /* Download the root group object and associated attributes  */
    root = file->root_grp;
    if((stat = NCZF_download_grp(file, root, &zobj))) goto done;

    /* Decode the group metadata to get only the superblock */
    if((stat = NCZF_decode_group(file,root,&zobj,NULL,(NCjson**)&jsuper))) goto done;

    if(jsuper != NULL) {
        /* Ok, process superblock */
        if((stat = NCZF_decode_superblock(file,jsuper,NULL,NULL))) goto done;
    } else {
	zinfo->flags |= FLAG_PUREZARR;	
    }	
    
    /* Fill in the root object ignoring any superblock */
    if((stat = ncz_decode_grp(file,root,&zobj))) goto done;

    /* Create and fill the subgroups for this group */
    if((stat = ncz_decode_subgrps(file,root,subgroupnames))) goto done;

done:
    NCZ_clear_zobj(&zobj);
    nclistfreeall(varnames);
    nclistfreeall(subgroupnames);
    return ZUNTRACE(THROW(stat));
}

static int
ncz_decode_subgrps(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* subgrpnames)
{
    int stat = NC_NOERR;
    size_t i;
    struct ZOBJ zobj = NCZ_emptyzobj();

    ZTRACE(3,"file=%s parent=%s |subgrpnames|=%u",file->controller->path,parent->hdr.name,nclistlength(subgrpnames));

    /* Create and load each subgrp in turn */
    for(i = 0; i < nclistlength(subgrpnames); i++) {
	const char* subgrpname = (const char*)nclistget(subgrpnames,i);
	NC_GRP_INFO_T* subgrp = NULL;
	/* Create the group object */
	if((stat=ncz4_create_grp(file,parent,subgrpname,&subgrp))) goto done;
	/* Download the group's metadata */
	if((stat = NCZF_download_grp(file,subgrp,&zobj))) goto done;
	/* Fill in the group object */
        if((stat = ncz_decode_grp(file,subgrp,&zobj))) goto done;
        NCZ_clear_zobj(&zobj);
    }

done:
    NCZ_clear_zobj(&zobj);
    return ZUNTRACE(THROW(stat));
}

/**
 * @internal Read group data from storage
 *
 * @param file Pointer to file struct
 * @param grp Pointer to grp struct
 * @param zobj the grp|atts for this grp; may be NULL
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_decode_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    size_t i;
    NCZ_FILE_INFO_T* zinfo = file->format_file_info;
    NClist* varnames = nclistnew();
    NClist* subgrps = nclistnew();
    NClist* dimdefs = nclistnew();
    int purezarr = 0;
    const NCjson* jnczgrp = NULL;

    ZTRACE(3,"file=%s parent=%s",file->controller->path,(parent?parent->hdr.name:"NULL"));

    TESTPUREZARR;

    /* Decode the group metadata */
    if((stat = NCZF_decode_group(file,grp,zobj,(NCjson**)&jnczgrp,NULL))) goto done;
    if(!purezarr && zobj->jobj == NULL) {stat = NC_ENOTZARR; goto done;}
    if(purezarr) {
	if((stat = get_group_content_pure(file,grp,varnames,subgrps))) goto done;
    } else { /*!purezarr*/
        /* Decode the _nczarr_group */
        if((stat = NCZF_decode_nczarr_group(file,grp,jnczgrp,varnames,subgrps,dimdefs))) goto done;
    }

    /* Declare the dimensions in this group */
    for(i=0;i<nclistlength(dimdefs);i++) {
	struct NCZ_DimInfo* di = (struct NCZ_DimInfo*)nclistget(dimdefs,i);
	if((stat = ncz4_create_dim(file,grp,di,NULL))) goto done;
    }

    /* Process attributes */
    if((stat=ncz_decode_atts(file,(NC_OBJ*)grp,zobj->jatts))) goto done;

    if(nclistlength(subgrps) > 0) {
	/* Define sub-groups */
	if((stat = ncz_decode_subgrps(file,grp,subgrps))) goto done;
    }

    if(nclistlength(varnames) > 0) {
	/* Define vars taking xarray into account */
	if((stat = ncz_decode_vars(file,grp,varnames))) goto done;
    }
    
done:
    NCZ_reclaim_diminfo_list(dimdefs);
    nclistfreeall(varnames);
    nclistfreeall(subgrps);
    return ZUNTRACE(THROW(stat));
}

#if 0
/**
 * @internal Materialize dimensions into memory
 *
 * @param file Pointer to file info struct.
 * @param parent Pointer to parent grp info struct.
 * @param diminfo vector of struct NCZ_DimInfo*
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_decode_dims(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* diminfo)
{
    int stat = NC_NOERR;
    size_t i;

    ZTRACE(3,"file=%s parent=%s |diminfo|=%u",file->controller->path,parent->hdr.name,nclistlength(diminfo));

    for(i=0;i<nclistlength(diminfo);i++) {
	struct NCZ_DimInfo* dim = (struct NCZ_DimInfo*)nclistget(diminfo,i);
	if((stat = ncz4_create_dim(file,parent,dim->name,dim->shape,dim->unlimited,NULL))) goto done;
    }

done:
    return ZUNTRACE(THROW(stat));
}
#endif /*0*/

/**
 * @internal Materialize single var into memory;
 * Take xarray and purezarr into account.
 *
 * @param file Pointer to file info struct.
 * @param parent Pointer to parent grp info struct.
 * @param varname name of variable in this group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_decode_var1(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* varname)
{
    int stat = NC_NOERR;
    NC_VAR_INFO_T* var = NULL;
    struct ZOBJ zobj = NCZ_emptyzobj();
    NClist* filters = nclistnew();
    NClist* dimrefs = nclistnew(); /* NClist<char*> */
    NClist* dimdecls = nclistnew(); /* NClist<struct NCZ_DimInfo> */
    size64_t* shapes = NULL;
    size64_t* chunks = NULL;
    size_t i, rank;
    NCbytes* fqn = ncbytesnew();
    NCZ_VAR_INFO_T* zvar = NULL;

    ZTRACE(3,"file=%s parent=%s varname=%s",file->controller->path,parent->hdr.name,varname);

    /* Create and Download */
    if((stat = ncz4_create_var(file,parent,varname,&var))) goto done;
    zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    if((stat = NCZF_download_var(file,var,&zobj))) goto done;
    if((stat=NCZF_decode_var(file,var,&zobj,filters,&shapes,&chunks,dimrefs))) goto done;
    rank = var->ndims;
    assert(zvar->scalar || nclistlength(dimrefs)==rank);
    assert(rank == 0 || var->dim != NULL);
    assert(rank == 0 || var->dimids != NULL);

    if(rank > 0) {
	/* Convert dimrefs to corresponding dimdecls */
	if((stat = reifydimrefs(file,parent,var,shapes,dimrefs,dimdecls))) goto done;
    }
    
    /* Process chunks and shapes */
    assert(var->chunksizes == NULL);
    if(rank == 0) { /* Scalar */
	/* Scalars still need a chunk and cache */
	var->dimids = NULL;
	var->dim = NULL;
	if((var->chunksizes = (size_t*)malloc(sizeof(size_t)))==NULL) {stat = NC_ENOMEM; goto done;}
	var->chunksizes[0] = 1;
	zvar->chunkproduct = 1;
    } else {
	if((var->chunksizes = (size_t*)malloc(rank * sizeof(size_t)))==NULL) {stat = NC_ENOMEM; goto done;}
	zvar->chunkproduct = 1;
	assert(nclistlength(dimdecls) == rank);
	for(i=0;i<rank;i++) {
	    NC_DIM_INFO_T* dim = (NC_DIM_INFO_T*)nclistget(dimdecls,i);
	    var->dim[i] = dim;
	    var->dimids[i] = dim->hdr.id;
	    var->chunksizes[i] = (size_t)chunks[i];
	    zvar->chunkproduct *= var->chunksizes[i];
	}
    }
    zvar->chunksize = zvar->chunkproduct * var->type_info->size;

    /* Create the cache */
    if((stat = NCZ_create_chunk_cache(var,var->type_info->size*zvar->chunkproduct,zvar->dimension_separator,&zvar->cache)))
	goto done;

    /* Process attributes */
    if((stat=ncz_decode_atts(file,(NC_OBJ*)var,zobj.jatts))) goto done;

    /* Process filters */
    if((stat = ncz_decode_filters(file,var,filters))) goto done;

done:
    nullfree(shapes);
    nullfree(chunks);
    nclistfreeall(dimrefs);
    nclistfree(dimdecls);
    nclistfree(filters);
    ncbytesfree(fqn);
    NCZ_clear_zobj(&zobj);
    return THROW(stat);
}

/**
 * @internal Materialize vars into memory;
 * Take xarray and purezarr into account.
 *
 * @param file Pointer to file info struct.
 * @param grp Pointer to grp info struct.
 * @param varnames List of names of variables in this group
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
static int
ncz_decode_vars(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NClist* varnames)
{
    int stat = NC_NOERR;
    size_t i;

    ZTRACE(3,"file=%s grp=%s |varnames|=%u",file->controller->path,grp->hdr.name,nclistlength(varnames));

    /* Load each var in turn */
    for(i = 0; i < nclistlength(varnames); i++) {
	const char* varname = (const char*)nclistget(varnames,i);
	if((stat = ncz_decode_var1(file,parent,varname))) goto done;
    }

done:
    return ZUNTRACE(THROW(stat));
}

/**************************************************/
/**
@internal Read attributes from a group or var and create a list
of annotated NC_ATT_INFO_T* objects. This will process
_NCProperties attribute specially.
@param file - [in] the containing file
@param container - [in] the containing object (group|var)
@param jatts - [in] the set of attributes from the container
@return ::NC_NOERR
@author Dennis Heimbigner
*/
static int
ncz_decode_atts(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts)
{
    int stat = NC_NOERR;
    NC_VAR_INFO_T* var = NULL;
    NC_ATT_INFO_T* special = NULL;
    struct NCZ_AttrInfo ainfo = NCZ_emptyAttrInfo();

    ZTRACE(3,"file=%s container=%s",file->controller->path,container->name);

    if(jatts != NULL) {
        if((stat = NCZF_decode_attributes(file,container,jatts))) goto done;
    }

    /* Look for special per-var attributes */
    if(container->sort == NCVAR)
	var = (NC_VAR_INFO_T*)container;

    /* _FillValue */
    if(var != NULL && !var->no_fill) {
	special = NULL;
	stat = nc4_find_grp_att(var->container,var->hdr.id,NC_FillValue,0,&special);
	/* If we have not read a _FillValue attribute, then go ahead and create it */
	if(stat == NC_ENOTATT) {
	    stat = NC_NOERR; /*reset*/
	    ainfo.name = NC_FillValue;
	    ainfo.nctype = var->type_info->hdr.id;
	    if((stat = NC4_inq_atomic_type(ainfo.nctype, NULL, &ainfo.typelen))) goto done;
	    ainfo.datalen = 1;
	    if((stat = NC_copy_data_all(file->controller,ainfo.nctype,var->fill_value,ainfo.datalen,&ainfo.data))) goto done;
	    if((stat = ncz_makeattr(file,(NC_OBJ*)var,&ainfo,&special))) goto done;
	} else if(stat != NC_NOERR) goto done;
    }

    /* _Quantize_XXX */
    if(var != NULL) {
	NC_ATT_INFO_T* qatt;
	int mode;
	/* Look for quantization attributes */
	for(qatt=NULL,mode=1;mode<=NC_QUANTIZE_MAX;mode++,qatt=NULL) {
	    const char* attmodename = NC_findquantizeattname(mode); /* get matching att name */
	    /* See if this att is defined */
	    stat = nc4_find_grp_att(var->container,var->hdr.id,attmodename,0,&qatt);
	    if(stat == NC_NOERR) {assert(qatt != NULL); break;}
	    if(stat != NC_ENOTATT) goto done; /* true error */
	    /* else keep looking */
	}
	stat = NC_NOERR; /* reset */
	if(qatt != NULL) {
	     if(qatt->len != 1 || qatt->data == NULL) {stat = NC_ENCZARR; goto done;}
	     /* extract the mode and NSD/NSB */
	     var->quantize_mode = mode;
	     var->nsd = ((int*)qatt->data)[0];
	}
    }

    /* Remember that we have read the atts for this var or group. */
    if(container->sort == NCVAR)
	((NC_VAR_INFO_T*)container)->atts_read = 1;
    else
	((NC_GRP_INFO_T*)container)->atts_read = 1;

done:
   NCZ_clearAttrInfo(file,&ainfo);
   return ZUNTRACE(THROW(stat));
}

/**************************************************/
#ifdef NETCDF_ENABLE_NCZARR_FILTERS

static int
ncz_encode_filters(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj)
{
    int stat = NC_NOERR;
    size_t i;
    NClist* filters = (NClist*)var->filters;
    NCjson* jfilter = NULL;

    for(i=0;i<nclistlength(filters);i++) {
	NCZ_Filter* filter = (NCZ_Filter*)nclistget(filters,i);
        if((stat=NCZF_encode_filter(file,filter,&jfilter))) goto done;
	nclistpush(filtersj,jfilter); jfilter = NULL;	
    }

done:
    return THROW(stat);
}

/**
@internal Read filter codecs from avar and apply them
to a variable.
@param file - [in] the containing file
@param var - [in] the containing var
@return ::NC_NOERR || NC_EXXX
@author Dennis Heimbigner
*/
static int
ncz_decode_filters(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const NClist* filters)
{
    int stat = NC_NOERR;
    NCZ_Filter* filter = NULL;

    ZTRACE(3,"file=%s var=%s",file->controller->path,var->hdr.name);

    if(nclistlength(filters) > 0) {
	size_t i;
	for(i=0;i<nclistlength(filters);i++) {
	    NCjson* jfilter = (NCjson*)nclistget(filters,i);
	    assert(jfilter != NULL);
	    if((filter = (NCZ_Filter*)calloc(1,sizeof(NCZ_Filter)))==NULL) {stat = NC_ENOMEM; goto done;}
	    if((stat = NCZF_decode_filter(file,var,jfilter,filter))) goto done;
	    if((stat=NCZ_addfilter(file,var,&filter))) goto done; /* addfilter will take control of filter object */
	    NCZ_filter_free(filter); filter = NULL;
	}
    }
done:
    NCZ_filter_free(filter);
    return ZUNTRACE(THROW(stat));
}
#else
static int
ncz_encode_filters(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj)
{
    NC_UNUSED(file);
    NC_UNUSED(var);
    NC_UNUSED(filtersj);
    return NC_NOERR;
}

static int
ncz_decode_filters(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const NClist* filters)
{
    NC_UNUSED(file);
    NC_UNUSED(var);
    NC_UNUSED(filters);
    return NC_NOERR;
}
#endif /*NETCDF_ENABLE_NCZARR_FILTERS*/

/**************************************************/
/* Utilities */

/**
Return the list of var names and subgrp names that are
immediately below the specified group.
*/
static int
get_group_content_pure(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrps)
{
    int stat = NC_NOERR;

    ZTRACE(3,"zinfo=%s grp=%s |varnames|=%u |subgrps|=%u",zinfo->common.file->controller->path,grp->hdr.name,(unsigned)nclistlength(varnames),(unsigned)nclistlength(subgrps));

    nclistclear(varnames);
    if((stat = NCZF_searchobjects(file,grp,varnames,subgrps))) goto done;

done:
    return ZUNTRACE(THROW(stat));
}

#if 0
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
static int
ncz_get_var_meta(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int retval = NC_NOERR;

    assert(file && var && var->format_var_info);
    LOG((3, "%s: var %s", __func__, var->hdr.name));
    ZTRACE(3,"file=%s var=%s",file->controller->path,var->hdr.name);
    
    /* Have we already read the var metadata? */
    if (var->meta_read)
	goto done;

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
    if ((retval = nc4_adjust_var_cache(var)))
	BAIL(retval);

    /* Is there an attribute which means quantization was used? */
    if ((retval = get_quantize_info(var)))
	BAIL(retval);

    if (var->coords_read && !var->dimscale)
	if ((retval = get_attached_info(var, hdf5_var, var->ndims, hdf5_var->hdf_datasetid)))
	    goto done;;
#endif

    /* Remember that we have read the metadata for this var. */
    var->meta_read = NC_TRUE;
done:
#if 0
#endif
    return ZUNTRACE(retval);
}
#endif /*0*/

/**
Insert an attribute into a list of attribute, including typing
Takes control of javalue but not atype
@param jatts
@param jtypes
@param aname
@param javaluep
@parame atypep
*/
int
ncz_insert_attr(NCjson* jatts, NCjson* jtypes, const char* aname, NCjson** javaluep, const char* atype)
{
    int stat = NC_NOERR;
    if(jatts != NULL) {
        if(jtypes != NULL) {
	    NCJinsertstring(jtypes,aname,atype);
        }
        NCJinsert(jatts,aname,*javaluep);
	*javaluep = NULL;
    }
    return THROW(stat);
}

/**************************************************/
#if 0
/* Convert an attribute "types list to an envv style list */
static int
jtypesatypes(NCjson* jtypes, NClist* atypes)
{
    int stat = NC_NOERR;
    size_t i;
    for(i=0;i<NCJarraylength(jtypes);i+=2) {
	const NCjson* key = NCJith(jtypes,i);
	const NCjson* value = NCJith(jtypes,i+1);
	if(NCJsort(key) != NCJ_STRING) {stat = (THROW(NC_ENCZARR)); goto done;}
	if(NCJsort(value) != NCJ_STRING) {stat = (THROW(NC_ENCZARR)); goto done;}
	nclistpush(atypes,strdup(NCJstring(key)));
	nclistpush(atypes,strdup(NCJstring(value)));
    }
done:
    return stat;
}
#endif

#if 0
/* See if there is reason to believe the specified path is a legitimate (NC)Zarr file
 * Do a breadth first walk of the tree starting at file path.
 * @param file to validate
 * @return ::NC_NOERR if it looks ok
 * @return ::NC_ENOTNC if it does not look ok
 */
static int
ncz_validate(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    int validate = 0;
    NCbytes* prefix = ncbytesnew();
    NClist* queue = nclistnew();
    NClist* nextlevel = nclistnew();
    NCZMAP* map = zinfo->map;
    char* path = NULL;
    char* segment = NULL;
    size_t seglen;
	    
    ZTRACE(3,"file=%s",file->controller->path);

    path = strdup("/");
    nclistpush(queue,path);
    path = NULL;
    do {
        nullfree(path); path = NULL;
	/* This should be full path key */
	path = nclistremove(queue,0); /* remove from front of queue */
	/* get list of next level segments (partial keys) */
	assert(nclistlength(nextlevel)==0);
        if((stat=nczmap_search(map,path,nextlevel))) {validate = 0; goto done;}
        /* For each s in next level, test, convert to full path, and push onto queue */
	while(nclistlength(nextlevel) > 0) {
            segment = nclistremove(nextlevel,0);
            seglen = nulllen(segment);
	    if((seglen >= 2 && memcmp(segment,".z",2)==0) || (seglen >= 4 && memcmp(segment,".ncz",4)==0)) {
		validate = 1;
	        goto done;
	     }
	     /* Convert to full path */
	     ncbytesclear(prefix);
	     ncbytescat(prefix,path);
	     if(strlen(path) > 1) ncbytescat(prefix,"/");
	     ncbytescat(prefix,segment);
	     /* push onto queue */
	     nclistpush(queue,ncbytesextract(prefix));
 	     nullfree(segment); segment = NULL;
	 }
    } while(nclistlength(queue) > 0);
done:
    if(!validate) stat = NC_ENOTNC;
    nullfree(path);
    nullfree(segment);
    nclistfreeall(queue);
    nclistfreeall(nextlevel);
    ncbytesfree(prefix);
    return ZUNTRACE(THROW(stat));
}
#endif

#if 0
/**
Insert _nczarr_attrs into .zattrs
Take control of jtypes
@param jatts
@param jtypes
*/
static int
insert_nczarr_attrs(NCjson* jatts, NCjson* jtypes)
{
    NCjson* jdict = NULL;
    if(jatts != NULL && jtypes != NULL) {
	NCJinsertstring(jtypes,NCZ_ATTRS,NC_JSON_DTYPE); /* type for _nczarr_attrs */
        NCJnew(NCJ_DICT,&jdict);
        NCJinsert(jdict,"types",jtypes);
        NCJinsert(jatts,NCZ_ATTR,jdict);
        jdict = NULL;
    }
    return NC_NOERR;
}
#endif /*0*/

#if 0
/**
Upload a .zattrs object
Optionally take control of jatts and jtypes
@param file
@param container
@param jattsp
@param jtypesp
*/
static int
upload_attrs(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson* jatts)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NC_VAR_INFO_T* var = NULL;
    NC_GRP_INFO_T* grp = NULL;
    NCZMAP* map = NULL;
    char* fullpath = NULL;
    char* key = NULL;

    ZTRACE(3,"file=%s grp=%s",file->controller->path,grp->hdr.name);

    if(jatts == NULL) goto done;    

    zinfo = file->format_file_info;
    map = zinfo->map;

    if(container->sort == NCVAR) {
        var = (NC_VAR_INFO_T*)container;
    } else if(container->sort == NCGRP) {
        grp = (NC_GRP_INFO_T*)container;
    }

    /* Construct container path */
    if(container->sort == NCGRP)
	stat = NCZ_grpkey(grp,&fullpath);
    else
	stat = NCZ_varkey(var,&fullpath);
    if(stat) goto done;

    /* write .zattrs*/
    if((stat = nczm_concat(fullpath,ZATTRS,&key))) goto done;
    if((stat=NCZ_uploadjson(map,key,jatts))) goto done;
    nullfree(key); key = NULL;

done:
    nullfree(key);
    nullfree(fullpath);
    return ZUNTRACE(THROW(stat));
}
#endif /*0*/

#if 0
/**
@internal Get contents of a meta object; fail it it does not exist
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json || NULL if not exists
@return NC_NOERR
@return NC_EXXX
@author Dennis Heimbigner
*/
static int
readarray(NCZMAP* zmap, const char* key, NCjson** jsonp)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    if((stat = NCZ_downloadjson(zmap,key,&json))) goto done;
    if(json != NULL && NCJsort(json) != NCJ_ARRAY) {stat = NC_ENCZARR; goto done;}
    if(jsonp) {*jsonp = json; json = NULL;}
done:
    NCZ_reclaim_json(json);
    return stat;
}
#endif

#if 0
static int
downloadzarrobj(NC_FILE_INFO_T* file, struct ZARROBJ* zobj, const char* fullpath, const char* objname)
{
    int stat = NC_NOERR;
    char* key = NULL;
    NCZMAP* map = ((NCZ_FILE_INFO_T*)file->format_file_info)->map;

    /* Download .zXXX and .zattrs */
    nullfree(zobj->prefix);
    zobj->prefix = strdup(fullpath);
    NCZ_reclaim_json(zobj->obj); zobj->obj = NULL;
    NCZ_reclaim_json(zobj->atts); zobj->obj = NULL;
    if((stat = nczm_concat(fullpath,objname,&key))) goto done;
    if((stat=NCZ_downloadjson(map,key,&zobj->obj))) goto done;
    nullfree(key); key = NULL;
    if((stat = nczm_concat(fullpath,ZATTRS,&key))) goto done;
    if((stat=NCZ_downloadjson(map,key,&zobj->atts))) goto done;
done:
    nullfree(key);
    return THROW(stat);
}
#endif /*0*/

/* Convert dimrefs to dimension declarations  (possibly creating them) */
static int
reifydimrefs(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, NC_VAR_INFO_T* var, size64_t* shapes, NClist* dimrefs, NClist* dimdecls)
{
    int stat = NC_NOERR;
    size_t i;
    size_t rank = var->ndims;    
    NCbytes* fqn = NULL;
    char* basename = NULL;

    fqn = ncbytesnew();
    for(i=0;i<rank;i++) {
	NC_OBJ* obj = NULL;
	const char* dimref = (const char*)nclistget(dimrefs,i);
	nullfree(basename); basename = NULL;
	ncbytesclear(fqn);
	if(dimref == NULL || strlen(dimref) == 0) {stat = NC_ENOTZARR; goto done;}
	if(dimref[0] == '/') { /* => FQN */
	    switch (stat = NCZ_locateFQN(file->root_grp, dimref, NCDIM, &obj, &basename)) {
	    case NC_NOERR: break; /* Dimension exists */
	    case NC_ENOOBJECT: /* Need to create dimension */
		if((stat = definedim(file,(NC_GRP_INFO_T*)obj,basename,shapes[i],(shapes[i]==0?UNLIM:FIXED),(NC_DIM_INFO_T**)&obj))) goto done;
		break;
	    default: goto done; /* some kind of real error */
	    }
	    nclistpush(dimdecls,obj);
	} else { /* search upwards for the dimension decl */
	    assert(strchr(dimref,'/')==NULL);
	    if((stat = NCZ_search_name(parent,dimref,NCDIM,&obj))) goto done;
	    if(obj == NULL) {
	        /* Need to create dimension in parent group */
		if((stat = definedim(file,parent,dimref,shapes[i],(shapes[i]==0?UNLIM:FIXED),(NC_DIM_INFO_T**)&obj))) goto done;
	    }
            nclistpush(dimdecls,(NC_DIM_INFO_T*)obj);
	}
    }
done:
    nullfree(basename);
    ncbytesfree(fqn);
    return THROW(stat);
}

static int
definedim(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* basename, size64_t shape, int unlim, NC_DIM_INFO_T** dimp)
{
    int stat = NC_NOERR;
    struct NCZ_DimInfo dimdef;
    assert(parent->hdr.sort == NCGRP);
    strncpy(dimdef.norm_name,basename,sizeof(dimdef.norm_name));
    /* Use shape as the size */
    dimdef.shape = (size_t)shape;
    dimdef.unlimited = unlim;
    if((stat = ncz4_create_dim(file,parent,&dimdef,dimp))) goto done;
done:
    return THROW(stat);
}
