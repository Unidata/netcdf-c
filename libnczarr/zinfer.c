/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Given various pieces of information and a map,
infer the format to be used.
Note that format1 (the oldest NCZarr format is now disallowed).

The current rules are as follows.

Creation:
1. Use the Zarr format specified in the mode flags, if any.
2. Otherwise use the default Zarr format
3. Use the chosen Zarr format for the NCZarr format also.
4. Use pure zarr if mode has "zarr" or "xarray" or "noxarray" tag.

Read:
2. If root contains ".zgroup", then
2.1 Zarr version is 2, and is verified by the .zgroup key "format".
2.2 If .zgroup contains key "_nczarr_superblock" then NCZarr version is 2.0.0 and can be verified by key "version".
2.3 Otherwise NCZarr version is NULL (i.e. pure Zarr).
3. If root subtree contains an object named "zarr.json" then
3.1 the Zarr format is V3.
3.2 If zarr.json is in root and contains key "_nczarr_superblock" then NCZarr version is 3.0.0 and can be verified by key "version".
4. If Zarr version is still unknown, then it defaults to 2.
5. If NCZarr version is still unknown then the NCZarr version is NULL (i.e. pure zarr).
*/

#include "zincludes.h"
#include "ncrc.h"
#include "ncjson.h"
#include "ncpathmgr.h"

#ifdef NETCDF_ENABLE_NCZARR_ZIP
#include <zip.h>
#endif

/**************************************************/

/* Tag search parameter */
struct TagParam {
    int zarrformat;
    int nczarrformat;
};

/**************************************************/
/*Forward*/

static int NCZ_infer_format(NC_FILE_INFO_T* file, int* zarrformatp, int* nczarrformatp);
static int NCZ_infer_storage_type(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCURI* url, NCZM_IMPL* implp);

static int infer_create_format(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, int* zarrformatp, int* nczarrformatp);
static int infer_open_format(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, int* zarrformatp, int* nczarrformatp);

static int tagsearch(NCZMAP* map, const char* prefix, const char* segment, void* param);

/**************************************************/

static int
NCZ_infer_format(NC_FILE_INFO_T* file, int* zarrformatp, int* nczarrformatp)
{
    int stat = NC_NOERR;
    int create;
    NCZ_FILE_INFO_T* zfile = NULL;
    NCZMAP* zmap = NULL;
    int zarrformat = 0;
    int nczarrformat = 0;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    create = zfile->creating;
    zmap = zfile->map;
    assert(zmap != NULL);
    /* Assume controls exist and have been applied */

    if(create) 
	stat = infer_create_format(file,zfile,zmap,&zarrformat,&nczarrformat);
    else
	stat = infer_open_format(file,zfile,zmap,&zarrformat,&nczarrformat);
    if(zarrformatp) *zarrformatp = zarrformat;
    if(nczarrformatp) *nczarrformatp = nczarrformat;
    return THROW(stat);
}

static int
infer_create_format(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, int* zarrformatp, int* nczarrformatp)
{
    int stat = NC_NOERR;
    int zarrformat = 0;
    int nczarrformat = NCZARRFORMAT0;

    switch (zfile->zarr.zarr_format) {
    case ZARRFORMAT2:
        zarrformat = ZARRFORMAT2;
	nczarrformat = NCZARRFORMAT2;
	break;
    case ZARRFORMAT3:
        zarrformat = ZARRFORMAT3;
	nczarrformat = NCZARRFORMAT3;
	break;
    default: stat = NC_ENOTZARR; break;
    }
    if(zarrformatp) *zarrformatp = zarrformat;
    if(nczarrformatp) *nczarrformatp = nczarrformat;
    return THROW(stat);
}

static int
infer_open_format(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCZMAP* map, int* zarrformatp, int* nczarrformatp)
{
    int stat = NC_NOERR;
    int zarrformat = 0;
    int nczarrformat = 0;
    NCjson* jrootgrp = NULL;
    const NCjson* jsuperg = NULL;
    const NCjson* jsupera = NULL;
    struct TagParam param;

    /* Probe the map for tell-tale objects and dict keys */

    if(zarrformat == 0) {
        /* We need to search subtree for a V2 or V3 tag */
	param.zarrformat = 0; param.nczarrformat = 0;
        switch(stat = nczmap_walk(map,"/",tagsearch, &param)) {
        case NC_NOERR:
	    /* No tag was found, so its not a zarr file */
	    stat = NC_ENOTZARR;
 	    goto done;
        case NC_EOBJECT: /* Arbitrary error signaling found and format is in param */
	    stat = NC_NOERR;
	    switch(param.zarrformat) {
	    case ZARRFORMAT2: case ZARRFORMAT3: zarrformat = param.zarrformat; break;
	    default: stat = NC_ENOTZARR; goto done;
	    }
	    break;
        default: stat = NC_ENOTZARR; goto done;
	}
    }

    if(zarrformat == ZARRFORMAT2 && nczarrformat == 0) {
	NCjson* jrootatts = NULL;
        /* Download /.zattrs  and /.zgroup */
        if((stat = NCZ_downloadjson(zfile->map, Z2ATTSROOT, &jrootgrp))) goto done;
        if((stat = NCZ_downloadjson(zfile->map, Z2METAROOT, &jrootatts))) goto done;
        /* Look for superblock */
	if(jrootgrp != NULL) NCJdictget(jrootgrp,NCZ_V2_SUPERBLOCK,&jsuperg);
	if(jrootatts != NULL) NCJdictget(jrootatts,NCZ_V2_SUPERBLOCK,&jsupera);
	if(jsuperg == NULL && jsupera == NULL) nczarrformat = NCZARRFORMAT0; else nczarrformat = NCZARRFORMAT2;
	NCJreclaim(jrootgrp); jrootgrp = NULL;
	NCJreclaim(jrootatts); jrootatts = NULL;
    }

    if(zarrformat == ZARRFORMAT3 && nczarrformat == 0) {
	const NCjson* jrootatts = NULL;
        /* Look for "/zarr.json" */
        if((stat = NCZ_downloadjson(zfile->map, Z3METAROOT, &jrootgrp))) goto done;
	if(jrootgrp == NULL || NCJsort(jrootgrp) != NCJ_DICT) {
	    nczarrformat = NCZARRFORMAT0;
	} else {
	    NCJdictget(jrootgrp,"attributes",&jrootatts);
	    if(jrootatts == NULL || NCJsort(jrootatts) != NCJ_DICT) {
		nczarrformat = NCZARRFORMAT0;
	    } else {
                /* Look for the _nczarr_superblock tag */
                NCJdictget(jrootatts,NCZ_V3_SUPERBLOCK,&jsupera);
   	        if(jsupera == NULL)
	            nczarrformat = NCZARRFORMAT0;  /* Pure zarr V3 file */
	        else
	            nczarrformat = NCZARRFORMAT3;
	    }
	}
	NCJreclaim(jrootgrp); jrootgrp = NULL;
    }

    if(zarrformat == 0) {stat = NC_ENOTZARR; goto done;}

    if(zarrformatp) *zarrformatp = zarrformat;
    if(nczarrformatp) *nczarrformatp = nczarrformat;

done:
    if(stat == NC_ENOTZARR) {
        if(zarrformatp) *zarrformatp = 0;
	if(nczarrformatp) *nczarrformatp = NCZARRFORMAT0;
    }
    NCJreclaim(jrootgrp);
    return THROW(stat);
}

static int
tagsearch(NCZMAP* map, const char* prefix, const char* key, void* param)
{
    struct TagParam* formats = (struct TagParam*)param;
    const char* segment = NULL;
    size_t seglen = 0;

    /* Validate */
    segment = strrchr(key,'/');
    if(segment == NULL) segment = key; else segment++;
    seglen = strlen(segment);
    if(seglen == 0) return NC_NOERR;
    
    if(strcasecmp(segment,"zarr.json")==0) {
        formats->zarrformat = ZARRFORMAT3;
	return NC_EOBJECT; /* arbitrary error telling walker to stop */
    }
    if(seglen < 2) return NC_NOERR; /* keep looking */
    if(segment[0] != '.' || segment[1] != 'z') return NC_NOERR;
    if(strcasecmp(segment,Z2GROUP)==0
       || strcasecmp(segment,Z2ARRAY)==0
       || strcasecmp(segment,Z2ATTRS)==0) {
	formats->zarrformat = ZARRFORMAT2;
	return NC_EOBJECT; /* V2 object found */
    }
    return NC_NOERR; /* Keep looking */
}

/**************************************************/
/**
Given various pieces of information and a URL,
infer the store type: currently file,zip,s3.

The current rules are as follows.

Creation:
1. Use the store type specified in the URL: "file", "zip", "s3".

Read:
1. If the URL specifies a store type, then use that type unconditionally.
2. If the URL protocol is "file", then treat the URL path as a file path.
2.1 If the path references a directory, then the store type is "file".
2.2 If the path references a file, and can be opened by libzip, then the store type is "zip"
2.3 Otherwise fail with NC_ENOTZARR.
3. If the url protocol is "http" or "https" then:
3.1 Apply the function NC_iss3 and if it succeeds, the store type is s3|gs3.
3.2 If the mode contains "file", then storetype is file -- meaning REST API to a file store.
*/

static int
NCZ_infer_storage_type(NC_FILE_INFO_T* file, NCZ_FILE_INFO_T* zfile, NCURI* url, NCZM_IMPL* implp)
{
    int ret = NC_NOERR;
    int create;
    NCZM_IMPL impl = NCZM_UNDEF;

    assert(zfile != NULL);
    create = zfile->creating;

    /* mode storetype overrides all else */
    if(NC_testmode(url, "file")) impl = NCZM_FILE;
    else if(NC_testmode(url, "s3")) impl = NCZM_S3;
    else if(NC_testmode(url, "gs3")) impl = NCZM_GS3;
#ifdef NETCDF_ENABLE_NCZARR_ZIP
    else if(NC_testmode(url, "zip")) impl = NCZM_ZIP;
#endif
    if(!create) { /* Reading a file of some kind */
	if(strcasecmp(url->protocol,"file")==0) {
	    struct stat buf;
	    /* Either file or zip */
	    if(NCstat(url->path,&buf)<0) {ret = errno; goto done;}
	    if(S_ISDIR(buf.st_mode))
		impl = NCZM_FILE; /* only possibility */
#ifdef NETCDF_ENABLE_NCZARR_ZIP
	    else if(S_ISREG(buf.st_mode)) {
		/* Should be zip, but verify */
                zip_flags_t zipflags = ZIP_RDONLY;
		zip_t* archive = NULL;
	        int zerrno = ZIP_ER_OK;
	        /* Open the file */
                archive =  zip_open(url->path,(int)zipflags,&zerrno);
		if(archive != NULL) {
		    impl = NCZM_ZIP;
		    zip_close(archive);
		}		
	    }
#endif
	}
    }

    if(impl == NCZM_UNDEF)
	{ret = NC_ENOTZARR; goto done;}

    if(implp) *implp = impl;
done:
    return THROW(ret);
}

/**
Figure out the storage type and create and return a corresponding map.

@param file
@param url
@param mode
@param constraints
@param params
@return NC_NOERR | NC_EXXX
*/

int
NCZ_get_map(NC_FILE_INFO_T* file, NCURI* url, mode_t mode, size64_t constraints, void* params, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    int create = 0;
    NCZMAP* map = NULL;
    NCZM_IMPL impl = NCZM_UNDEF;
    NCZ_FILE_INFO_T* zfile = NULL;
    char* path = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    create = zfile->creating;

    if((stat = NCZ_infer_storage_type(file, zfile, url, &impl))) goto done;
    
    if((path = ncuribuild(url,NULL,NULL,NCURIALL))==NULL) {stat = NC_ENCZARR; goto done;}

    switch (impl) {
    case NCZM_FILE: case NCZM_ZIP: case NCZM_S3: case NCZM_GS3:
	if(create)
	    {if((stat = nczmap_create(impl,path,mode,constraints,params,&map))) goto done;}
	else
    	    {if((stat = nczmap_open(impl,path,mode,constraints,params,&map))) goto done;}
	break;
    case NCZM_UNDEF:
    default: stat = NC_ENOTZARR; goto done;
    }

    if(mapp) {*mapp = map; map = NULL;}

done:
    nullfree(path);
    if(map) (void)nczmap_close(map,0);
    return THROW(stat);
}


/**
Figure out the formatter to use.

@param file
@param formatterp

@return NC_NOERR | NC_EXXX
*/

int
NCZ_get_formatter(NC_FILE_INFO_T* file, const NCZ_Formatter** formatterp)
{
    int stat = NC_NOERR;
    const NCZ_Formatter* formatter = NULL;
    NCZ_FILE_INFO_T* zfile = NULL;
    int zarr_format = 0;
    int nczarr_format = 0;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);

    /* Infer the NCZarr+Zarr formats */
    if((stat = NCZ_infer_format(file, &zarr_format, &nczarr_format))) goto done;

    zfile->zarr.zarr_format = zarr_format;
    zfile->zarr.nczarr_format = nczarr_format;

    /* If the nczarr_format is NULL, then that implies pure zarr,
       so use the zarr format instead. */
    if(nczarr_format != 0) {
        switch(nczarr_format) {
        case 2: formatter = NCZ_formatter2; break;
        case 3: formatter = NCZ_formatter3; break;
        default: stat = NC_ENCZARR; goto done;
        }
    } else { /* Decide based on zarr format plus the fact that it is pure zarr */
        switch(zarr_format) {
        case 2: formatter = NCZ_formatter2; break;
        case 3: formatter = NCZ_formatter3; break;
        default: stat = NC_ENCZARR; goto done;
        }
    }

    if(formatterp) *formatterp = formatter;

done:
    return THROW(stat);
}

