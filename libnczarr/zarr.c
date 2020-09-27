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

    ZTRACE("%s/%s %s",file->hdr.name,root->hdr.name,controls);

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
    if((zinfo->controls=NCZ_clonestringvec(0,controls)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* fill in some of the zinfo and zroot fields */
    zinfo->zarr.zarr_version = ZARRVERSION;
    sscanf(NCZARRVERSION,"%lu.%lu.%lu",
	   &zinfo->zarr.nczarr_version.major,
	   &zinfo->zarr.nczarr_version.minor,
	   &zinfo->zarr.nczarr_version.release);

    /* Apply client controls */
    if((stat = applycontrols(zinfo))) goto done;

    /* Load auth info from rc file */
    if((zinfo->auth = calloc(1,sizeof(NCauth)))==NULL)
	{stat = NC_ENOMEM; goto done;}
    if((stat = ncuriparse(nc->path,&uri))) goto done;
    if(uri) {
	if((stat = NC_authsetup(zinfo->auth, uri)))
	    goto done;
    }

    /* initialize map handle*/
    if((stat = nczmap_create(zinfo->features.mapimpl,nc->path,nc->mode,zinfo->features.flags,NULL,&zinfo->map)))
	goto done;

    /* Create super block (NCZMETAROOT) */
    if((stat = ncz_create_superblock(zinfo))) goto done;

done:
    ncurifree(uri);
    NCJreclaim(json);
    nullfree(key);
    return stat;
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
    int i,stat = NC_NOERR;
    NC* nc = NULL;
    NC_GRP_INFO_T* root = NULL;
    NCURI* uri = NULL;
    void* content = NULL;
    NCjson* json = NULL;
    NCZ_FILE_INFO_T* zinfo = NULL;
    int mode;
    NClist* modeargs = NULL;

    ZTRACE("%s %s",file->hdr.name,controls);

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
    if((zinfo->controls = NCZ_clonestringvec(0,controls))==NULL) /*0=>envv style*/
	{stat = NC_ENOMEM; goto done;}

    /* Add struct to hold NCZ-specific group info. */
    if (!(root->format_grp_info = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    ((NCZ_GRP_INFO_T*)root->format_grp_info)->common.file = file;

    /* Apply client controls */
    if((stat = applycontrols(zinfo))) goto done;

    /* initialize map handle*/
    if((stat = nczmap_open(zinfo->features.mapimpl,nc->path,mode,zinfo->features.flags,NULL,&zinfo->map)))
	goto done;

    if(!(zinfo->features.flags & FLAG_PUREZARR)
        && (stat = NCZ_downloadjson(zinfo->map, NCZMETAROOT, &json)) == NC_NOERR) {
        /* Extract the information from it */
        for(i=0;i<nclistlength(json->contents);i+=2) {
    	    const NCjson* key = nclistget(json->contents,i);
	    const NCjson* value = nclistget(json->contents,i+1);
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
    } else { /* zinfo->features.purezarr || no object */
	zinfo->zarr.zarr_version = ZARRVERSION;
	sscanf(NCZARRVERSION,"%lu.%lu.%lu",
		    &zinfo->zarr.nczarr_version.major,
		    &zinfo->zarr.nczarr_version.minor,
		    &zinfo->zarr.nczarr_version.release);
    }

    /* Load auth info from rc file */
    if((zinfo->auth = calloc(1,sizeof(NCauth)))==NULL)
	{stat = NC_ENOMEM; goto done;}
    if((stat = ncuriparse(nc->path,&uri))) goto done;
    if(uri) {
	if((stat = NC_authsetup(zinfo->auth, uri)))
	    goto done;
    }

done:
    ncurifree(uri);
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

    if((stat=nczm_concat(NULL,ZGROUP,&rootpath)))
	goto done;
    if((stat = NCZ_downloadjson(zfile->map, rootpath, &json)))
	goto  done;
    /* Process the json */ 
    for(i=0;i<nclistlength(json->contents);i+=2) {
	const NCjson* key = nclistget(json->contents,i);
	const NCjson* value = nclistget(json->contents,i+1);
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
ncz_unload_jatts(NCZ_FILE_INFO_T* zinfo, NC_OBJ* container, NCjson* jattrs, NCjson* jtypes)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;
    char* akey = NULL;
    char* tkey = NULL;
    NCZMAP* map = zinfo->map;

    assert((jattrs->sort = NCJ_DICT));
    assert((jtypes->sort = NCJ_DICT));

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

    /* Upload the .zattrs object */
    if((stat=NCZ_uploadjson(map,tkey,jattrs)))
	goto done;

    if(!(zinfo->features.flags & FLAG_PUREZARR)) {
        /* Construct the path to the .nczattr object */
        if((stat = nczm_concat(fullpath,NCZATTR,&tkey)))
   	    goto done;
        /* Upload the .nczattr object */
        if((stat=NCZ_uploadjson(map,tkey,jtypes)))
	    goto done;
    }
    
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

static const char*
controllookup(const char** controls, const char* key)
{
    const char** p;
    for(p=controls;*p;p++) {
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

    if((value = controllookup((const char**)zinfo->controls,"mode")) != NULL) {
	if((stat = NCZ_comma_parse(value,modelist))) goto done;
    }
    /* Process the modelist first */
    zinfo->features.mapimpl = NCZM_DEFAULT;
    for(i=0;i<nclistlength(modelist);i++) {
        const char* p = nclistget(modelist,i);
	if(strcasecmp(p,PUREZARR)==0) zinfo->features.flags |= FLAG_PUREZARR;
	else if(strcasecmp(p,"bytes")==0) zinfo->features.flags |= FLAG_BYTERANGE;
	else if(strcasecmp(p,"s3")==0) zinfo->features.mapimpl = NCZM_S3;
	else if(strcasecmp(p,"nz4")==0) zinfo->features.mapimpl = NCZM_NC4;
	else if(strcasecmp(p,"nzf")==0) zinfo->features.mapimpl = NCZM_FILE;
	else if(strcasecmp(p,"nzrf")==0)
	    {zinfo->features.mapimpl = NCZM_FILE; zinfo->features.flags |= FLAG_BYTERANGE;}
	else if(strcasecmp(p,"nzr4")==0)
	    {zinfo->features.mapimpl = NCZM_NC4; zinfo->features.flags |= FLAG_BYTERANGE;}
    }
    /* Process other controls */
    if((value = controllookup((const char**)zinfo->controls,"log")) != NULL) {
	zinfo->features.flags |= FLAG_LOGGING;
	ncloginit();
        if(nclogopen(value))
	    ncsetlogging(1);
	ncloginit();
        if(nclogopen(value))
	    ncsetlogging(1);
    }
    if((value = controllookup((const char**)zinfo->controls,"show")) != NULL) {
	if(strcasecmp(value,"fetch")==0)
	    zinfo->features.flags |= FLAG_SHOWFETCH;
    }
done:
    nclistfreeall(modelist);
    return stat;
}

