/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zmetadata.h"

/**************************************************/

extern int NCZF2_initialize(void);
extern int NCZF2_finalize(void);
extern int NCZF3_initialize(void);
extern int NCZF3_finalize(void);

/**************************************************/
////////////////////////////////////////////////////

int NCZMD_initialize(void)
{	
    int stat = NC_NOERR;
    if((stat=NCZMD2_initialize())) goto done;
    if((stat=NCZMD3_initialize())) goto done;
done:
    return THROW(stat);
}

int NCZMD_finalize(void)
{

    int stat = NC_NOERR;
    if((stat=NCZMD2_finalize())) goto done;
    if((stat=NCZMD3_finalize())) goto done;
done:
    return THROW(stat);
}

// Returns the list of subgroups from *grp
int NCZMD_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames)
{
	return zfile->metadata_handler->dispatcher->list_groups(zfile, grp, subgrpnames);
}

// Returns the list of variables from grp
int NCZMD_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames)
{
	return zfile->metadata_handler->dispatcher->list_variables(zfile, grp, varnames);
}


/////////////////////////////////////////////////////////////////////
//     Fetch JSON content from .zmetadata or storage
/////////////////////////////////////////////////////////////////////

int NCZMD_fetch_json_group(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jgroup)
{
	int stat = NC_NOERR;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &key)) != NC_NOERR)) 
		goto done;
	if (name && (stat = nczm_concat(key, name, &key)))
		goto done;

	stat = zfile->metadata_handler->dispatcher->fetch_json_content(zfile, NCZMD_GROUP, key, jgroup);
done:	
	nullfree(key);
	return stat;
}

int NCZMD_fetch_json_attrs(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jattrs)
{
	int stat = NC_NOERR;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &key)) != NC_NOERR)) 
		goto done;
	if (name && (stat = nczm_concat(key, name, &key)))
		goto done;

	stat = zfile->metadata_handler->dispatcher->fetch_json_content(zfile, NCZMD_ATTRS, key, jattrs);
done:	
	nullfree(key);
	return stat;
}

int NCZMD_fetch_json_array(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, const char *name, NCjson **jarray)
{
	int stat = NC_NOERR;
	char *key = NULL;	

	if (grp && ((stat = NCZ_grpkey(grp, &key)) != NC_NOERR)) 
		goto done;
	if (name && (stat = nczm_concat(key, name, &key)))
		goto done;

	stat = zfile->metadata_handler->dispatcher->fetch_json_content(zfile, NCZMD_ARRAY, key, jarray);
done:	
	nullfree(key);
	return stat;
}

////////////////////////////////////////////////////////////////////////////

int NCZMD_is_metadata_consolidated(NCZ_FILE_INFO_T *zfile)
{
	NCZ_Metadata *zmd = NULL;
	zmd = zfile->metadata_handler;
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

	NCZ_Metadata *zmd = zfile->metadata_handler;
	if ( !zmd || !zmd->dispatcher ) {
		return NC_EFILEMETA;
	}


	if (zmd->dispatcher->zarr_format >= ZARRFORMAT2)
	{
		*zarrformat = zmd->dispatcher->zarr_format;
		return NC_NOERR;
	}

	// At this point, we know that version 3 cannot be
	// otherwise the zarr.json would have been found
	// when setting the handler.
	// Last thing to do is to look for:
	//      .zattrs, .zgroup or .zarray

	if (!nczmap_exists(zfile->map, "/" Z2ATTRS) && !nczmap_exists(zfile->map, "/" Z2GROUP) && !nczmap_exists(zfile->map, "/" Z2ARRAY))
	{
		return NC_ENOTZARR;
	}

	*zarrformat = ZARRFORMAT2;
	return NC_NOERR;
}

int NCZMD_set_metadata_handler(NCZ_FILE_INFO_T *zfile, const NCZ_Metadata **mdhandlerp)
{
	int stat = NC_NOERR;
	const NCZ_Metadata_Dispatcher *zmd_dispatcher = NULL;
	NCjson *jcsl = NULL;

	if (zfile->metadata_handler != NULL)
	{
		stat = NC_EOBJECT;
		goto done;
	}

	zmd_dispatcher = NCZ_metadata_handler2;

	if ((stat = NCZ_downloadjson(zfile->map, Z2METADATA, &jcsl)) != NC_NOERR || jcsl == NULL)
	{
		if ((stat = NCZ_downloadjson(zfile->map, Z3METADATA, &jcsl)) != NC_NOERR || jcsl == NULL)
		{
			/* We've tried to get any json 
			objects for consolidated access
			but they seem to be absent */
			goto done;
		}
		NCjson * jtmp = NULL;
		if (NCJsort(jcsl) != NCJ_DICT || NCJdictget(jcsl,"consolidated_metadata", &jtmp) || NCJsort(jtmp) != NCJ_DICT)
		{
			zmd_dispatcher = NCZ_metadata_handler3;
		}
		else
		{
			zmd_dispatcher = NCZ_csl_metadata_handler3;
		}
	}
	else
	{
		zmd_dispatcher = NCZ_csl_metadata_handler2;
	}

	if (jcsl != NULL && NCJsort(jcsl) == NCJ_DICT)
	{
		NCZ_Metadata *zmdh = NULL;
		if ((zmdh = (NCZ_Metadata *)calloc(1, sizeof(NCZ_Metadata))) == NULL)
		{
			stat = NC_ENOMEM;
			goto done;
		}
		zmdh->jcsl = jcsl;
		zmdh->dispatcher = zmd_dispatcher;

		*mdhandlerp = (const NCZ_Metadata *)zmdh;
	}
	else
	{
		NCJreclaim(jcsl); // Free jcsl if not assigned
	}

done:
	return stat;
}
