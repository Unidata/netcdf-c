/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include <stddef.h>

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
ncz_create_dataset(NC_FILE_INFO_T* file, NC_GRP_INFO_T* root, NClist* urlcontrols)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;
    NCZ_GRP_INFO_T* zgrp = NULL;
    NCURI* uri = NULL;
    NC* nc = NULL;
    NCjson* json = NULL;
    char* key = NULL;

    ZTRACE(3,"root=%s urlcontrols=%s",root->hdr.name,(urlcontrols?nczprint_envlist(urlcontrols):"null"));

    nc = (NC*)file->controller;

    /* Add struct to hold NCZ-specific file metadata. */
    if (!(zfile = calloc(1, sizeof(NCZ_FILE_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    file->format_file_info = zfile;
    zfile->common.file = file;

    /* Add struct to hold NCZ-specific group info. */
    if (!(zgrp = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    root->format_grp_info = zgrp;
    zgrp->common.file = file;

    /* Fill in NCZ_FILE_INFO_T */
    zfile->creating = 1;
    zfile->common.file = file;
    zfile->native_endianness = (NCZ_isLittleEndian() ? NC_ENDIAN_LITTLE : NC_ENDIAN_BIG);
    if((zfile->urlcontrols=nclistclone(urlcontrols,1)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* Apply client controls */
    if((stat = applycontrols(zfile))) goto done;

    /* Load auth info from rc file */
    if((stat = ncuriparse(nc->path,&uri))) goto done;
    if(uri) {
	if((stat = NC_authsetup(&zfile->auth, uri)))
	    goto done;
    }

    /* default the zarr format */
    if(zfile->zarr.zarr_format == 0)
        zfile->zarr.zarr_format = NC_getglobalstate()->zarr.default_zarrformat;

    /* initialize map handle*/
    if((stat = NCZ_get_map(file,uri,(mode_t)nc->mode,zfile->flags,NULL,&zfile->map))) goto done;

    /* And get the format dispatcher */
    if((stat = NCZ_get_create_formatter(file, (const NCZ_Formatter**)&zfile->dispatcher))) goto done;

    /* And get the consolidated metadata handler */
    if((stat = NCZMD_set_metadata_handler(file))) goto done;

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
ncz_open_dataset(NC_FILE_INFO_T* file, NClist* urlcontrols)
{
    int stat = NC_NOERR;
    NC* nc = NULL;
    NC_GRP_INFO_T* root = NULL;
    NCURI* uri = NULL;
    NCZ_FILE_INFO_T* zfile = NULL;
    NClist* modeargs = NULL;

    ZTRACE(3,"file=%s urlcontrols=%s",file->hdr.name,(urlcontrols?nczprint_envlist(urlcontrols):"null"));

    /* Extract info reachable via file */
    nc = (NC*)file->controller;

    root = file->root_grp;
    assert(root != NULL && root->hdr.sort == NCGRP);

    /* Add struct to hold NCZ-specific file metadata. */
    if (!(file->format_file_info = calloc(1, sizeof(NCZ_FILE_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    zfile = file->format_file_info;

    /* Fill in NCZ_FILE_INFO_T */
    zfile->creating = 0;
    zfile->common.file = file;
    zfile->native_endianness = (NCZ_isLittleEndian() ? NC_ENDIAN_LITTLE : NC_ENDIAN_BIG);
    if((zfile->urlcontrols = nclistclone(urlcontrols,1))==NULL) /*0=>envv style*/
	{stat = NC_ENOMEM; goto done;}
    
    /* Add struct to hold NCZ-specific group info. */
    if (!(root->format_grp_info = calloc(1, sizeof(NCZ_GRP_INFO_T))))
        {stat = NC_ENOMEM; goto done;}
    ((NCZ_GRP_INFO_T*)root->format_grp_info)->common.file = file;

    /* Apply client controls */
    if((stat = applycontrols(zfile))) goto done;

    /* Load auth info from rc file */
    if((stat = ncuriparse(nc->path,&uri))) goto done;
    if(uri) {
	if((stat = NC_authsetup(&zfile->auth, uri)))
	    goto done;
    }

    /* initialize map handle*/
    if((stat = NCZ_get_map(file,uri,(mode_t)nc->mode,zfile->flags,NULL,&zfile->map))) goto done;

    /* Get the zarr_format */
    if((stat = NCZ_infer_open_zarr_format(file))) goto done;

    /* And add the consolidated metadata manager to file */
    /* Must follow NCZ_infer_ope_zarr_format because it uses the discovered zarr format */
    if((stat = NCZMD_set_metadata_handler(file))) goto done;

    /* Set the nczarr format; must follow set_metadata_handler because it needs to read metadata */
    if((stat = NCZ_infer_open_nczarr_format(file))) goto done;
    
    /* And get the format dispatcher: uses discovered zarr and nczarr formats and the metadata handler */
    if((stat = NCZ_get_open_formatter(file, (const NCZ_Formatter**)&zfile->dispatcher))) goto done;

    /* Load the meta-data */
    if((stat = ncz_decode_file(file))) goto done;

done:
    ncurifree(uri);
    nclistfreeall(modeargs);
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
    sscanf(NCZARR_PACKAGE_VERSION,"%lu.%lu.%lu",&m0,&m1,&m2);
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
    NCZ_FILE_INFO_T* zfile = file->format_file_info;
    if(superblockp) *superblockp = zfile->zarr.nczarr_format;
    return NC_NOERR;
}

/**************************************************/
/* Utilities */

static const char*
controllookup(NClist* controls, const char* key)
{
    size_t i;
    for(i=0;i<nclistlength(controls);i+=2) {
        const char* p = (char*)nclistget(controls,i);
	if(strcasecmp(key,p)==0) {
	    return (const char*)nclistget(controls,i+1);
	}
    }
    return NULL;
}

/**
Look to various sources to get control information
for a given dataset. Current sources:
1. From URL:
    * "mode=..."
    * "log"
    * "show=..."
2. Environment variables:

@param zinfo modified to add controls
@return NC_NOERR if success
@return NC_EXXX if failures
*/
static int
applycontrols(NCZ_FILE_INFO_T* zinfo)
{
    size_t i;
    int stat = NC_NOERR;
    const char* value = NULL;
    NClist* modelist = nclistnew();
    size64_t noflags = 0; /* track non-default negative flags */

#ifdef NETCDF_ENABLE_ZOH
    /* Track http: vs https: */
    if(strncmp("http://",zinfo->common.file->hdr.name,7) == 0 ||
       strncmp("https://",zinfo->common.file->hdr.name,8) == 0 ){
	zinfo->mapimpl = NCZM_ZOH;
    }
#endif
    /* Apply controls from URL mode=... */
    if((value = controllookup(zinfo->urlcontrols,"mode")) != NULL) {
	if((stat = NCZ_comma_parse(value,modelist))) goto done;
    }

    /* Process the modelist */
    zinfo->mapimpl = NCZM_DEFAULT;
    zinfo->flags |= FLAG_XARRAYDIMS; /* Always support XArray convention where possible */
    for(i=0;i<nclistlength(modelist);i++) {
        const char* p = nclistget(modelist,i);
	if(strcasecmp(p,PUREZARRCONTROL)==0)
	    zinfo->flags |= FLAG_PUREZARR;
	else if(strcasecmp(p,XARRAYCONTROL)==0)
	    zinfo->flags |= FLAG_XARRAYDIMS;
	else if(strcasecmp(p,NOXARRAYCONTROL)==0)
	    noflags |= FLAG_XARRAYDIMS;
	else if(strcasecmp(p,ZARRFORMAT2_STRING)==0)
	    zinfo->zarr.zarr_format = ZARRFORMAT2;
	else if(strcasecmp(p,ZARRFORMAT3_STRING)==0)
	    zinfo->zarr.zarr_format = ZARRFORMAT3;
	else if(strcasecmp(p,NOZMETADATACONTROL)==0)
	    zinfo->flags |= FLAG_NOCONSOLIDATED;
	else if(strcasecmp(p,ZMETADATACONTROL)==0)
	    noflags |= FLAG_NOCONSOLIDATED;
    }

    /* Apply negative controls by turning off negative flags */
    /* This is necessary to avoid order dependence of mode flags when both positive and negative flags are defined */
    zinfo->flags &= (~noflags);

    /* Process other controls */
    if((value = controllookup(zinfo->urlcontrols,"log")) != NULL) {
	zinfo->flags |= FLAG_LOGGING;
        ncsetloglevel(NCLOGNOTE);
    }
    if((value = controllookup(zinfo->urlcontrols,"show")) != NULL) {
	if(strcasecmp(value,"fetch")==0)
	    zinfo->flags |= FLAG_SHOWFETCH;
    }

    /* Environment Variables */

done:
    nclistfreeall(modelist);
    return stat;
}
