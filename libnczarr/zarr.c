/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/**************************************************/
/* Forwards */

static int applycontrols(NCZ_FILE_INFO_T* zinfo);

/***************************************************/
/* API */

/**
@internal Create the topmost dataset object and setup up
          NCZ_FILE_INFO_T state.
@param zinfo - [in] the internal state
@return NC_NOERR
@author Dennis Heimbigner
*/

int
ncz_create_dataset(NC_FILE_INFO_T* file, NC_GRP_INFO_T* root, const char** controls)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCZ_GRP_INFO_T* zgrp = NULL;
    NCURI* uri = NULL;
    NC* nc = NULL;
    NCjson* json = NULL;
    char* key = NULL;

    ZTRACE(3,"file=%s root=%s controls=%s",file->hdr.name,root->hdr.name,(controls?nczprint_envv(controls):"null"));

    nc = (NC*)file->controller;

    /* Add struct to hold NCZ-specific file metadata. */
    if (!(zinfo = calloc(1, sizeof(NCZ_FILE_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    file->format_file_info = zinfo;
    zinfo->common.file = file;

    /* Add struct to hold NCZ-specific group info. */
    if (!(zgrp = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    root->format_grp_info = zgrp;
    zgrp->common.file = file;

    /* Fill in NCZ_FILE_INFO_T */
    zinfo->created = 1;
    zinfo->common.file = file;
    zinfo->native_endianness = (NCZ_isLittleEndian() ? NC_ENDIAN_LITTLE : NC_ENDIAN_BIG);
    if((zinfo->envv_controls=NCZ_clonestringvec(0,controls)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* fill in some of the zinfo and zroot fields */
    zinfo->zarr.zarr_version = atoi(ZARRVERSION);
    sscanf(NCZARRVERSION,"%lu.%lu.%lu",
	   &zinfo->zarr.nczarr_version.major,
	   &zinfo->zarr.nczarr_version.minor,
	   &zinfo->zarr.nczarr_version.release);

    zinfo->default_maxstrlen = NCZ_MAXSTR_DEFAULT;

    /* Apply client controls */
    if((stat = applycontrols(zinfo))) goto done;

    /* Load auth info from rc file */
    if((stat = ncuriparse(nc->path,&uri))) goto done;
    if(uri) {
	if((stat = NC_authsetup(&zinfo->auth, uri)))
	    goto done;
    }

    /* initialize map handle*/
    if((stat = nczmap_create(zinfo->controls.mapimpl,nc->path,nc->mode,zinfo->controls.flags,NULL,&zinfo->map)))
	goto done;

done:
    ncurifree(uri);
    NCJreclaim(json);
    nullfree(key);
    return ZUNTRACE(stat);
}

/**
@internal Open the topmost dataset object.
@param file - [in] the file struct
@param controls - [in] the fragment list in envv form from uri
@return NC_NOERR
@author Dennis Heimbigner
*/

int
ncz_open_dataset(NC_FILE_INFO_T* file, const char** controls)
{
    int stat = NC_NOERR;
    NC* nc = NULL;
    NC_GRP_INFO_T* root = NULL;
    NCURI* uri = NULL;
    void* content = NULL;
    NCjson* json = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;
    int mode;
    NClist* modeargs = NULL;
    char* nczarr_version = NULL;
    char* zarr_format = NULL;

    ZTRACE(3,"file=%s controls=%s",file->hdr.name,(controls?nczprint_envv(controls):"null"));

    /* Extract info reachable via file */
    nc = (NC*)file->controller;
    mode = nc->mode;

    root = file->root_grp;
    assert(root != NULL && root->hdr.sort == NCGRP);

    /* Add struct to hold NCZ-specific file metadata. */
    if (!(file->format_file_info = calloc(1, sizeof(NCZ_FILE_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    zinfo = file->format_file_info;

    /* Fill in NCZ_FILE_INFO_T */
    zinfo->created = 0;
    zinfo->common.file = file;
    zinfo->native_endianness = (NCZ_isLittleEndian() ? NC_ENDIAN_LITTLE : NC_ENDIAN_BIG);
    if((zinfo->envv_controls = NCZ_clonestringvec(0,controls))==NULL) /*0=>envv style*/
	{stat = NC_ENOMEM; goto done;}
    zinfo->default_maxstrlen = NCZ_MAXSTR_DEFAULT;

    /* Add struct to hold NCZ-specific group info. */
    if (!(root->format_grp_info = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    ((NCZ_GRP_INFO_T*)root->format_grp_info)->common.file = file;

    /* Apply client controls */
    if((stat = applycontrols(zinfo))) goto done;

    /* initialize map handle*/
    if((stat = nczmap_open(zinfo->controls.mapimpl,nc->path,mode,zinfo->controls.flags,NULL,&zinfo->map)))
	goto done;

    if((stat = ncz_read_superblock(file,&nczarr_version,&zarr_format))) goto done;

    if(nczarr_version == NULL) /* default */
        nczarr_version = strdup(NCZARRVERSION);
    if(zarr_format == NULL) /* default */
       zarr_format = strdup(ZARRVERSION);
    /* Extract the information from it */
    if(sscanf(zarr_format,"%d",&zinfo->zarr.zarr_version)!=1)
	{stat = NC_ENCZARR; goto done;}		
    if(sscanf(nczarr_version,"%lu.%lu.%lu",
		    &zinfo->zarr.nczarr_version.major,
		    &zinfo->zarr.nczarr_version.minor,
		    &zinfo->zarr.nczarr_version.release) == 0)
	{stat = NC_ENCZARR; goto done;}

    /* Load auth info from rc file */
    if((stat = ncuriparse(nc->path,&uri))) goto done;
    if(uri) {
	if((stat = NC_authsetup(&zinfo->auth, uri)))
	    goto done;
    }

done:
    nullfree(zarr_format);
    nullfree(nczarr_version);
    ncurifree(uri);
    nclistfreeall(modeargs);
    if(json) NCJreclaim(json);
    nullfree(content);
    return ZUNTRACE(stat);
}

/**
 * @internal Determine whether file is netCDF-4.
 *
 * For libzarr, this is always true.
 *
 * @param h5 Pointer to HDF5 file info struct.
 *
 * @returns NC_NOERR No error.
 * @author Dennis Heimbigner.
 */
int
NCZ_isnetcdf4(struct NC_FILE_INFO* h5)
{
    int isnc4 = 1;
    NC_UNUSED(h5);
    return isnc4;
}

/**
 * @internal Determine version info
 *
 * For libzarr, this is not well defined
 *
 * @param majorp Pointer to major version number
 * @param minorp Pointer to minor version number
 * @param releasep Pointer to release version number
 *
 * @returns NC_NOERR No error.
 * @author Dennis Heimbigner.
 */
int
NCZ_get_libversion(unsigned long* majorp, unsigned long* minorp,unsigned long* releasep)
{
    unsigned long m0,m1,m2;
    sscanf(NCZARRVERSION,"%lu.%lu.%lu",&m0,&m1,&m2);
    if(majorp) *majorp = m0;
    if(minorp) *minorp = m1;
    if(releasep) *releasep = m2;
    return NC_NOERR;
}

/**
 * @internal Determine "superblock" number.
 *
 * For libzarr, use the value of the major part of the nczarr version.
 *
 * @param superblocp Pointer to place to return superblock.
 * use the nczarr format version major as the superblock number.
 *
 * @returns NC_NOERR No error.
 * @author Dennis Heimbigner.
 */
int
NCZ_get_superblock(NC_FILE_INFO_T* file, int* superblockp)
{
    NCZ_FILE_INFO_T* zinfo = file->format_file_info;
    if(superblockp) *superblockp = zinfo->zarr.nczarr_version.major;
    return NC_NOERR;
}

/**************************************************/
/* Utilities */

#if 0
/**
@internal Open the root group object
@param dataset - [in] the root dataset object
@param rootp - [out] created root group
@return NC_NOERR
@author Dennis Heimbigner
*/
static int
ncz_open_rootgroup(NC_FILE_INFO_T* dataset)
{
    int stat = NC_NOERR;
    int i;
    NCZ_FILE_INFO_T* zfile = NULL;
    NC_GRP_INFO_T* root = NULL;
    void* content = NULL;
    char* rootpath = NULL;
    NCjson* json = NULL;

    ZTRACE(3,"dataset=",dataset->hdr.name);

    zfile = dataset->format_file_info;

    /* Root should already be defined */
    root = dataset->root_grp;

    assert(root != NULL);

    if((stat=nczm_concat(NULL,ZGROUP,&rootpath)))
	goto done;
    if((stat = NCZ_downloadjson(zfile->map, rootpath, &json)))
	goto  done;
    /* Process the json */ 
    for(i=0;i<nclistlength(json->contents);i+=2) {
	const NCjson* key = nclistget(json->contents,i);
	const NCjson* value = nclistget(json->contents,i+1);
	if(strcmp(NCJstring(key),"zarr_format")==0) {
	    int zversion;
	    if(sscanf(NCJstring(value),"%d",&zversion)!=1)
		{stat = NC_ENOTNC; goto done;}		
	    /* Verify against the dataset */
	    if(zversion != zfile->zarr.zarr_version)
		{stat = NC_ENOTNC; goto done;}
	}
    }

done:
    if(json) NCJreclaim(json);
    nullfree(rootpath);
    nullfree(content);
    return ZUNTRACE(stat);
}
#endif


static const char*
controllookup(const char** envv_controls, const char* key)
{
    const char** p;
    for(p=envv_controls;*p;p+=2) {
	if(strcasecmp(key,*p)==0) {
	    return p[1];
	}
    }
    return NULL;
}


static int
applycontrols(NCZ_FILE_INFO_T* zinfo)
{
    int i,stat = NC_NOERR;
    const char* value = NULL;
    NClist* modelist = nclistnew();
    int noflags = 0; /* track non-default negative flags */

    if((value = controllookup((const char**)zinfo->envv_controls,"mode")) != NULL) {
	if((stat = NCZ_comma_parse(value,modelist))) goto done;
    }
    /* Process the modelist first */
    zinfo->controls.mapimpl = NCZM_DEFAULT;
    zinfo->controls.flags |= FLAG_XARRAYDIMS; /* Always support XArray convention where possible */
    for(i=0;i<nclistlength(modelist);i++) {
        const char* p = nclistget(modelist,i);
	if(strcasecmp(p,PUREZARRCONTROL)==0)
	    zinfo->controls.flags |= (FLAG_PUREZARR);
	else if(strcasecmp(p,XARRAYCONTROL)==0)
	    zinfo->controls.flags |= FLAG_PUREZARR;
	else if(strcasecmp(p,NOXARRAYCONTROL)==0)
	    noflags |= FLAG_XARRAYDIMS;
	else if(strcasecmp(p,"zip")==0) zinfo->controls.mapimpl = NCZM_ZIP;
	else if(strcasecmp(p,"file")==0) zinfo->controls.mapimpl = NCZM_FILE;
	else if(strcasecmp(p,"s3")==0) zinfo->controls.mapimpl = NCZM_S3;
    }
    /* Apply negative controls by turning off negative flags */
    /* This is necessary to avoid order dependence of mode flags when both positive and negative flags are defined */
    zinfo->controls.flags &= (~noflags);

    /* Process other controls */
    if((value = controllookup((const char**)zinfo->envv_controls,"log")) != NULL) {
	zinfo->controls.flags |= FLAG_LOGGING;
        ncsetlogging(1);
    }
    if((value = controllookup((const char**)zinfo->envv_controls,"show")) != NULL) {
	if(strcasecmp(value,"fetch")==0)
	    zinfo->controls.flags |= FLAG_SHOWFETCH;
    }
done:
    nclistfreeall(modelist);
    return stat;
}

#if 0
/**
@internal Rewrite attributes into a group or var
@param map - [in] the map object for storage
@param container - [in] the containing object
@param jattrs - [in] the json for .zattrs
@param jtypes - [in] the json for .ztypes
@return NC_NOERR
@author Dennis Heimbigner
*/
int
ncz_unload_jatts(NCZ_FILE_INFO_T* zinfo, NC_OBJ* container, NCjson* jattrs, NCjson* jtypes)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* akey = NULL;
    char* tkey = NULL;
    NCZMAP* map = zinfo->map;

    assert((NCJsort(jattrs) == NCJ_DICT));
    assert((NCJsort(jtypes) == NCJ_DICT));

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
    if((stat = nczm_concat(fullpath,ZATTRS,&akey)))
	goto done;

    /* Always write as V2 */

    {
        NCjson* k = NULL;
        NCjson* v = NULL;
	/* remove any previous version */
        if(!NCJremove(jattrs,NCZ_V2_ATTRS,1,&k,&v)) {
	    NCJreclaim(k); NCJreclaim(v);
	}
    }

    if(!(zinfo->controls.flags & FLAG_PUREZARR)) {
        /* Insert the jtypes into the set of attributes */
         if((stat = NCJinsert(jattrs,NCZ_V2_ATTRS,jtypes))) goto done;
    }

    /* Upload the .zattrs object */
    if((stat=NCZ_uploadjson(map,tkey,jattrs)))
	goto done;

done:
    if(stat) {
	NCJreclaim(jattrs);
	NCJreclaim(jtypes);
    }
    nullfree(fullpath);
    nullfree(akey);
    nullfree(tkey);
    return stat;
}
#endif



