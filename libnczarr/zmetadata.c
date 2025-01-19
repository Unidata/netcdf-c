/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/**************************************************/

extern int NCZMD2_initialize(void);
extern int NCZMD2_finalize(void);

/**************************************************/
////////////////////////////////////////////////////

int NCZMD_initialize(void)
{
    int stat = NC_NOERR;
    if((stat=NCZMD2_initialize())) goto done;
done:
    return THROW(stat);
}

int NCZMD_finalize(void)
{

    int stat = NC_NOERR;
    if((stat=NCZMD2_finalize())) goto done;
done:
    return THROW(stat);
}

/////////////////////////////////////////////////////////////////////
//     Fetch list of subnodes of .zmetadata or storage
/////////////////////////////////////////////////////////////////////

/**
Return a vector of names (not keys) representing the
next segment of legal objects that are immediately contained by the prefix key.
@param file -- the containing file
@param prefix -- the key into the tree where the search is to occur
@param matches -- return the set of names in this list; might be empty
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_list(NC_FILE_INFO_T* file, const char* prefix, struct NClist* matches)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->list(file, prefix, matches);
}

/**
Return a vector of keys representing the
list of all objects whose key is prefixed by the specified prefix arg.
In effect it returns the complete subtree below a specified prefix.
@param file -- the containing file
@param prefix -- the key into the tree whose subtree of keys is to be returned.
@param matches -- return the set of keys in this list; might be empty
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_listall(NC_FILE_INFO_T* file, const char* prefix, struct NClist* matches)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->listall(file, prefix, matches);
}

/**
Test if key refers to content-bearing object.
@param file -- the containing file
@param key -- the key into the tree to test
@return NC_NOERR if the key exists
@return NC_ENOOBJECT if the key does not exists
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_exists(NC_FILE_INFO_T* file, const char* key)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->exists(file, key);
}

/**
Open a consolidated metadata object
@param file -- the containing file
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_open(NC_FILE_INFO_T* file)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->open(file);
}

/**
Create a consolidated metadata object
@param file -- the containing file
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_create(NC_FILE_INFO_T* file)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->create(file);
}

/**
Close a consolidated metadata object
@param file -- the containing file
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_close(NC_FILE_INFO_T* file)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->close(file);
}

/**
Write a consolidated metadata object
@param file -- the containing file
@return NC_NOERR if the operation succeeded
@return NC_EXXX if the operation failed for one of several possible reasons
*/
int
NCZMD_consolidate(NC_FILE_INFO_T* file)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    return zfile->metadata_handler.dispatcher->consolidate(file);
}

/////////////////////////////////////////////////////////////////////
//     Fetch JSON content from .zmetadata or storage
/////////////////////////////////////////////////////////////////////

int
NCZMD_fetch_json_content(NC_FILE_INFO_T *file, NCZMD_MetadataType mdtype, const char *key, NCjson **jobjp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    if((stat = zfile->metadata_handler.dispatcher->fetch_json_content(file, mdtype, key, jobjp))) goto done;

done:   
    return THROW(stat);
}

#if 0
int
NCZMD_fetch_json_group(NC_FILE_INFO_T *file, NC_GRP_INFO_T *grp, const char *name, NCjson **jgroup)
{
    int stat = NC_NOERR;
    char *group= NULL;
    char *key = NULL;       
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    
    if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
        goto done;
    if ((stat = nczm_concat(group, name, &key)))
        goto done;
    if((stat = zfile->metadata_handler.dispatcher->fetch_json_content(file, NCZMD_GROUP, key, jgroup)))
    	goto done;

done:   
    nullfree(group);
    nullfree(key);
    return stat;
}

int
NCZMD_fetch_json_attrs(NC_FILE_INFO_T *file, NC_GRP_INFO_T *grp, const char *name, NCjson **jattrs)
{
    int stat = NC_NOERR;
    char *group= NULL;
    char *key = NULL;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
        goto done;
    if ((stat = nczm_concat(group, name, &key)))
        goto done;
    if((stat = zfile->metadata_handler.dispatcher->fetch_json_content(file, NCZMD_ATTRS, key , jattrs)))
        goto done;

done:   
    nullfree(group);
    nullfree(key);
    return stat;
}

int
NCZMD_fetch_json_array(NC_FILE_INFO_T *file, NC_GRP_INFO_T *grp, const char *name, NCjson **jarray)
{
    int stat = NC_NOERR;
    char *group= NULL;
    char *key = NULL;       
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
            goto done;

    if ((stat = nczm_concat(group, name, &key)))
            goto done;
    if((stat = zfile->metadata_handler.dispatcher->fetch_json_content(file, NCZMD_ARRAY, key, jarray)))
    	goto done;

done:
    nullfree(group);
    nullfree(key);
    return stat;
}
#endif /*0*/

////////////////////////////////////////////////////////////////////////////////
//                               Update in-memory + storage JSON content 
////////////////////////////////////////////////////////////////////////////////

int
NCZMD_update_json_content(NC_FILE_INFO_T *file, NCZMD_MetadataType mdtype, const char *key, const NCjson* jobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    if((stat = zfile->metadata_handler.dispatcher->update_json_content(file, mdtype, key, jobj))) goto done;

done:   
    return THROW(stat);
}

#if 0
int
NCZMD_update_json_group(NC_FILE_INFO_T *file, NC_GRP_INFO_T *grp, const char *name, const NCjson *jgroup)
{
    int stat = NC_NOERR;
    char *group= NULL;
    char *key = NULL;       
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
            goto done;
    if ((stat = nczm_concat(group, name, &key)))
            goto done;
    if((stat = zfile->metadata_handler.dispatcher->update_json_content(file, NCZMD_GROUP, key, jgroup)))
        goto done;

done:   
    nullfree(group);
    nullfree(key);
    return stat;
}

int
NCZMD_update_json_attrs(NC_FILE_INFO_T *file, NC_GRP_INFO_T *grp, const char *name, const NCjson *jattrs)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    char *group= NULL;
    char *key = NULL;

    if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
            goto done;
    if ((stat = nczm_concat(group, name, &key)))
            goto done;
    if((stat = zfile->metadata_handler.dispatcher->update_json_content(file, NCZMD_ATTRS, key , jattrs)))
        goto done;

done:   
    nullfree(group);
    nullfree(key);
    return stat;
}

int
NCZMD_update_json_array(NC_FILE_INFO_T *file, NC_GRP_INFO_T *grp, const char *name, const NCjson *jarray)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    char *group= NULL;
    char *key = NULL;       

    if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
            goto done;
    if ((stat = nczm_concat(group, name, &key)))
            goto done;
    if((stat = zfile->metadata_handler.dispatcher->update_json_content(file, NCZMD_ARRAY, key, jarray)))
        goto done;

done:
    nullfree(group);
    nullfree(key);
    return stat;
}
#endif /*0*/

////////////////////////////////////////////////////////////////////////////

int
NCZMD_is_metadata_consolidated(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata *zmd = &zfile->metadata_handler;
    if(zmd->jcsl == NULL ||
       NCJsort(zmd->jcsl) != NCJ_DICT ||
       (zfile->flags & FLAG_NOCONSOLIDATED))
	{stat = NC_ENOOBJECT; goto done;}
done:
    return stat;
}

int
NCZMD_get_metadata_format(NC_FILE_INFO_T* file, int* zarrformat)
{
    // Only pure Zarr is determined
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata *zmd = &zfile->metadata_handler;

    if(!zmd->dispatcher ) {stat = NC_EFILEMETA; goto done;}
    if(zmd->dispatcher->zarr_format >= ZARRFORMAT2) {
    *zarrformat = zmd->dispatcher->zarr_format;
    goto done;
    }
    /* Last thing to do is to look for:
       .zattrs, .zgroup or .zarray
    */
    if(!nczmap_exists(zfile->map, "/" Z2ATTRS)
       && !nczmap_exists(zfile->map, "/" Z2GROUP)
       && !nczmap_exists(zfile->map, "/" Z2ARRAY))
    goto done;
    *zarrformat = ZARRFORMAT2;
done:
    return THROW(stat);
}

/* Inference of the metadata handler */
int
NCZMD_set_metadata_handler(NC_FILE_INFO_T *file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata* zmd = &zfile->metadata_handler;
    const NCZ_Metadata_Dispatcher *zmd_dispatcher = NULL;
    int disallowzmetadata = 0;
    NCjson* jcsl = NULL;
    const NCjson* jmeta = NULL;

    /* Override from env var or mode flag */
    if((zfile->flags & FLAG_NOCONSOLIDATED) || getenv(NCZARRDEFAULTNOMETA) != NULL) disallowzmetadata = 1;

   /* First, figure out and set the dispatcher */
    if(disallowzmetadata) {
	zmd_dispatcher = NCZ_metadata_handler;
    } else if(zfile->creating) {
	switch (zfile->zarr.zarr_format) {
	case 2:
	    zmd_dispatcher = NCZ_csl_metadata_handler2;
	    break;
#ifdef NETCDF_ENABLE_NCZARR_V3
	case 3:
	    zmd_dispatcher = NCZ_csl_metadata_handler3;
	    break;
#endif
	default:
	    zmd_dispatcher = NCZ_metadata_handler;
	    break;
	}
    } else if(zmd_dispatcher == NULL) { /* opening a file */
	/* See if /.zmetadata exists */
	switch (zfile->zarr.zarr_format) {
	case 2: /* Try to download .zmetadata */
	    if((stat = NCZ_downloadjson(zfile->map,Z2METADATA,&jcsl))) goto done;
	    if(jcsl != NULL)
		NCJcheck(NCJdictget(jcsl,"metadata",(NCjson**)&jmeta));
	    break;
#ifdef NETCDF_ENABLE_NCZARR_V3
	case 3: /* For V3, we need to look inside the root group's zarr.json */
	    if((stat = NCZ_downloadjson(zfile->map,Z3METADATA,&jcsl))) goto done;
	    if(jcsl != NULL)
		NCJcheck(NCJdictget(jcsl,"metadata",(NCjson**)&jmeta));
	    break;
#endif
	default:
	    break;
	}
	if(jmeta != NULL && zfile->zarr.zarr_format == 2)
	    zmd_dispatcher = NCZ_csl_metadata_handler2;
#ifdef NETCDF_ENABLE_NCZARR_V3
	else if(jmeta && zfile->zarr.zarr_format == 3)
	    zmd_dispatcher = NCZ_csl_metadata_handler3;
#endif
	else
	    zmd_dispatcher = NCZ_metadata_handler;	    
    }
    zmd->dispatcher = zmd_dispatcher;
    assert(zmd->dispatcher != NULL);
    
    zmd->jcsl = jcsl; jcsl = NULL;
    zmd->jmeta = jmeta; jmeta = NULL;

    /* Now open/create the consolidated metadata object */
    if(zfile->creating) {
	if((stat=NCZMD_create(file))) goto done;
    } else {
	if((stat=NCZMD_open(file))) goto done;
    }

done:
    return THROW(stat);
}
