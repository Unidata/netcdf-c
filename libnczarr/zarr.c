/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/**************************************************/
/* Forwards */

static NCZM_IMPL computemapimpl(NClist*, int cmode);

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
ncz_create_dataset(NC_FILE_INFO_T* file, NC_GRP_INFO_T* root)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCZ_GRP_INFO_T* zgrp = NULL;
    NCURI* uri = NULL;
    NCZM_IMPL mapimpl = NCZM_UNDEF;
    NC* nc = NULL;
    NCjson* json = NULL;
    char* key = NULL;
    const char* smode = NULL;
    NClist* modeargs = NULL;

    ZTRACE();

    nc = (NC*)file->controller;

    /* Add struct to hold NCZ-specific file metadata. */
    if (!(zinfo = calloc(1, sizeof(NCZ_FILE_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    file->format_file_info = zinfo;

    /* Add struct to hold NCZ-specific group info. */
    if (!(zgrp = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    root->format_grp_info = zgrp;

    /* Parse url and params */
    if(ncuriparse(nc->path,&uri))
	{stat = NC_EDAPURL; goto done;}

    /* Get the "mode=" arg */
    if((smode = ncurilookup(uri,"mode")) != NULL) {
	modeargs = nclistnew();
	if((stat = NCZ_comma_parse(smode,modeargs)));
    }

    /* Figure out the map implementation */
    if((mapimpl = computemapimpl(modeargs,nc->mode)) == NCZM_UNDEF)
	mapimpl = NCZM_DEFAULT;

    /* fill in some of the zinfo and zroot fields */
    zinfo->zarr.zarr_version = ZARRVERSION;
    sscanf(NCZARRVERSION,"%lu.%lu.%lu",
	   &zinfo->zarr.nczarr_version.major,
	   &zinfo->zarr.nczarr_version.minor,
	   &zinfo->zarr.nczarr_version.release);

    zinfo->created = 1;

    /* initialize map handle*/
    if((stat = nczmap_create(mapimpl,nc->path,nc->mode,0,NULL,&zinfo->map)))
	goto done;

done:
    nclistfree(modeargs);
    NCJreclaim(json);
    nullfree(key);
    return stat;
}

/**
@internal Open the topmost dataset object.
@param file - [in] the file struct
@param root - [in] the root group
@param fraglist - [in] the fragment list in envv form from uri
@return NC_NOERR
@author Dennis Heimbigner
*/

int
ncz_open_dataset(NC_FILE_INFO_T* file)
{
    int i,stat = NC_NOERR;
    NC* nc = NULL;
    NC_GRP_INFO_T* root = NULL;
    NCURI* uri = NULL;
    void* content = NULL;
    NCjson* json = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;
    NCZM_IMPL mapimpl = NCZM_UNDEF;
    int mode;
    const char* path = NULL;
    NClist* modeargs = NULL;

    ZTRACE();

    /* Extract info reachable via file */
    nc = (NC*)file->controller;
    mode = nc->mode;
    path = nc->path;

    root = file->root_grp;
    assert(root != NULL && root->hdr.sort == NCGRP);

    if(ncuriparse(path,&uri))
	{stat = NC_EURL; goto done;}

    /* Add struct to hold NCZ-specific file metadata. */
    if (!(file->format_file_info = calloc(1, sizeof(NCZ_FILE_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    zinfo = file->format_file_info;

    /* Add struct to hold NCZ-specific group info. */
    if (!(root->format_grp_info = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}

    /* Get the "mode=" from uri*/
    {
	const char* modes = ncurilookup(uri,"mode");
	if(modes != NULL) {
  	    modeargs = nclistnew();
	    if((stat = NCZ_comma_parse(modes,modeargs)));
	}
    }

    /* Figure out the map implementation */
    if((mapimpl = computemapimpl(modeargs,mode)) == NCZM_UNDEF)
	mapimpl = NCZM_DEFAULT;

    /* initialize map handle*/
    if((stat = nczmap_open(mapimpl,nc->path,mode,0,NULL,&zinfo->map)))
	goto done;

    if((stat = NCZ_downloadjson(zinfo->map, NCZMETAROOT, &json)))
	goto done;
    /* Extract the information from it */
    for(i=0;i<nclistlength(json->dict);i+=2) {
	const NCjson* key = nclistget(json->dict,i);
	const NCjson* value = nclistget(json->dict,i+1);
	if(strcmp(key->value,"zarr_format")==0) {
	    if(sscanf(value->value,"%d",&zinfo->zarr.zarr_version)!=1)
		{stat = NC_ENOTNC; goto done;}		
	} else if(strcmp(key->value,"nczarr_version")==0) {
	    sscanf(value->value,"%lu.%lu.%lu",
		&zinfo->zarr.nczarr_version.major,
		&zinfo->zarr.nczarr_version.minor,
		&zinfo->zarr.nczarr_version.release);
	}
    }

done:
    nclistfreeall(modeargs);
    if(json) NCJreclaim(json);
    nullfree(content);
    return stat;
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

    ZTRACE();

    zfile = dataset->format_file_info;

    /* Root should already be defined */
    root = dataset->root_grp;

    assert(root != NULL);

    if((stat=nczm_suffix(NULL,ZGROUP,&rootpath)))
	goto done;
    if((stat = NCZ_downloadjson(zfile->map, rootpath, &json)))
	goto  done;
    /* Process the json */ 
    for(i=0;i<nclistlength(json->dict);i+=2) {
	const NCjson* key = nclistget(json->dict,i);
	const NCjson* value = nclistget(json->dict,i+1);
	if(strcmp(key->value,"zarr_format")==0) {
	    int zversion;
	    if(sscanf(value->value,"%d",&zversion)!=1)
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
    return stat;
}
#endif

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
ncz_unload_jatts(NCZMAP* map, NC_OBJ* container, NCjson* jattrs, NCjson* jtypes)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* akey = NULL;
    char* tkey = NULL;

    assert((jattrs->sort = NCJ_DICT));
    assert((jtypes->sort = NCJ_DICT));

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
    if((stat = nczm_suffix(fullpath,ZATTRS,&akey)))
	goto done;

    /* Upload the .zattrs object */
    if((stat=NCZ_uploadjson(map,tkey,jattrs)))
	goto done;

    /* Construct the path to the .zattrtypes object */
    if((stat = nczm_suffix(fullpath,NCZATTR,&tkey)))
	goto done;

    /* Upload the .ztypes object */

    if((stat=NCZ_uploadjson(map,tkey,jtypes)))
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

static NCZM_IMPL
computemapimpl(NClist* modeargs, int cmode)
{
    int i;

    if(modeargs == NULL) return NCZM_DEFAULT;

    for(i=0;i<nclistlength(modeargs);i++) {
	const char* mode = nclistget(modeargs,i);
	if(strcasecmp(mode,"s3")==0) return NCZM_S3;
	if(strcasecmp(mode,"ncz")==0) return NCZM_NC4;
    }
    return NCZM_DEFAULT; /* could not determine map impl */
}



