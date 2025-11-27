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

// Returns the list of subgroups from *grp
int NCZMD_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames)
{
	return zfile->metadata_handler.dispatcher->list_groups(zfile, grp, subgrpnames);
}

// Returns the list of variables from grp
int NCZMD_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames)
{
	return zfile->metadata_handler.dispatcher->list_variables(zfile, grp, varnames);
}


/////////////////////////////////////////////////////////////////////
//     Fetch JSON content from .zmetadata or storage
/////////////////////////////////////////////////////////////////////

int NCZMD_fetch_json_group(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jgroup)
{
	int stat = NC_NOERR;
	char *group= NULL;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
		goto done;
	if ((stat = nczm_concat(group, name, &key)))
		goto done;

	stat = zfile->metadata_handler.dispatcher->fetch_json_content(zfile, NCZMD_GROUP, key, jgroup);
done:	
	nullfree(group);
	nullfree(key);
	return stat;
}

int NCZMD_fetch_json_attrs(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jattrs)
{
	int stat = NC_NOERR;
	char *group= NULL;
	char *key = NULL;

	if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
		goto done;
	if ((stat = nczm_concat(group, name, &key)))
		goto done;

	stat = zfile->metadata_handler.dispatcher->fetch_json_content(zfile, NCZMD_ATTRS, key , jattrs);
done:	
	nullfree(group);
	nullfree(key);
	return stat;
}

int NCZMD_fetch_json_array(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jarray)
{
	int stat = NC_NOERR;
	char *group= NULL;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
		goto done;

	if ((stat = nczm_concat(group, name, &key)))
		goto done;
	

	stat = zfile->metadata_handler.dispatcher->fetch_json_content(zfile, NCZMD_ARRAY, key, jarray);
done:
	nullfree(group);
	nullfree(key);
	return stat;
}

////////////////////////////////////////////////////////////////////////////////
//    				 Update in-memory + storage JSON content 
////////////////////////////////////////////////////////////////////////////////

int NCZMD_update_json_group(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, const NCjson *jgroup)
{
	int stat = NC_NOERR;
	char *group= NULL;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
		goto done;
	if ((stat = nczm_concat(group, name, &key)))
		goto done;

	stat = zfile->metadata_handler.dispatcher->update_json_content(zfile, NCZMD_GROUP, key, jgroup);
done:	
	nullfree(group);
	nullfree(key);
	return stat;
}

int NCZMD_update_json_attrs(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, const NCjson *jattrs)
{
	int stat = NC_NOERR;
	char *group= NULL;
	char *key = NULL;

	if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
		goto done;
	if ((stat = nczm_concat(group, name, &key)))
		goto done;

	stat = zfile->metadata_handler.dispatcher->update_json_content(zfile, NCZMD_ATTRS, key , jattrs);
done:	
	nullfree(group);
	nullfree(key);
	return stat;
}

int NCZMD_update_json_array(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, const NCjson *jarray)
{
	int stat = NC_NOERR;
	char *group= NULL;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &group)) != NC_NOERR)) 
		goto done;

	if ((stat = nczm_concat(group, name, &key)))
		goto done;
	

	stat = zfile->metadata_handler.dispatcher->update_json_content(zfile, NCZMD_ARRAY, key, jarray);
done:
	nullfree(group);
	nullfree(key);
	return stat;
}

////////////////////////////////////////////////////////////////////////////
// Writes .zmetadata file into storage
int NCZMD_consolidate(NCZ_FILE_INFO_T *zfile) {
	int stat = NC_NOERR;
	if (zfile->creating == 1 && zfile->metadata_handler.jcsl !=NULL){
		stat = NCZ_uploadjson(zfile->map, Z2METADATA ,zfile->metadata_handler.jcsl);
	}
	return stat;
}
////////////////////////////////////////////////////////////////////////////

int NCZMD_is_metadata_consolidated(NCZ_FILE_INFO_T *zfile)
{
	NCZ_Metadata *zmd = &(zfile->metadata_handler);
	if (zmd == NULL ||
		zmd->jcsl == NULL ||
		NCJsort(zmd->jcsl) != NCJ_DICT ||
		!(zmd->dispatcher->flags & ZARR_CONSOLIDATED))
	{
		return NC_ENOOBJECT;
	}
	return NC_NOERR;
}

int NCZMD_get_metadata_format(NCZ_FILE_INFO_T *zfile, int *zarrformat)
{ // Only pure Zarr is determined

    NCZ_Metadata *zmd = &(zfile->metadata_handler);
	if ( !zmd || !zmd->dispatcher ) {
		return NC_EFILEMETA;
	}


	if (zmd->dispatcher->zarr_format >= ZARRFORMAT2)
	{
		*zarrformat = zmd->dispatcher->zarr_format;
		return NC_NOERR;
	}

	// Last thing to do is to look for:
	//      .zattrs, .zgroup or .zarray

	if (!nczmap_exists(zfile->map, "/" Z2ATTRS) && !nczmap_exists(zfile->map, "/" Z2GROUP) && !nczmap_exists(zfile->map, "/" Z2ARRAY))
	{
		return NC_ENOTZARR;
	}

	*zarrformat = ZARRFORMAT2;
	return NC_NOERR;
}

//Inference of the metadata handler
int NCZMD_set_metadata_handler(NCZ_FILE_INFO_T *zfile)
{
    NCjson *jcsl = NULL;

    if (zfile->creating) {
        zfile->metadata_handler.jcsl = NULL;
        zfile->metadata_handler.dispatcher = NCZ_csl_metadata_handler2;
        return NC_NOERR;
    }
    
    zfile->metadata_handler.jcsl = NULL;
	zfile->metadata_handler.dispatcher = NCZ_metadata_handler2; // default

    if (NCZ_downloadjson(zfile->map, Z2METADATA, &jcsl) || jcsl == NULL) {
        nclog(NCLOGNOTE, "Dataset not consolidated! Doing so will improve performance");
        return NC_NOERR;
    }
    
    if (NCZ_csl_metadata_handler2->validate_consolidated(jcsl) != NC_NOERR) {
        nclog(NCLOGWARN,"Consolidated metadata is invalid, ignoring it!");
        return NC_EZARRMETA;
    }

    // Use consolidated metadata handler
    zfile->metadata_handler.jcsl = jcsl;
    zfile->metadata_handler.dispatcher = NCZ_csl_metadata_handler2;
    return NC_NOERR;
}

void NCZMD_free_metadata_handler(NCZ_Metadata * zmd){
	if (zmd == NULL) return;
	NCJreclaim(zmd->jcsl);
    zmd->jcsl = NULL;
}
