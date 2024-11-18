/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zmetadata.h"

/**************************************************/

extern int NCZF2_initialize(void);
extern int NCZF2_finalize(void);

int NCZMD_v2_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
int NCZMD_v2_csl_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);

int NCZMD_v2_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
int NCZMD_v2_csl_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);


int v2_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);
int v2_csl_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);

/**************************************************/

static const NCZ_Metadata_Dispatcher NCZ_md2_table = {
    ZARRFORMAT2,
    NCZ_METADATA_VERSION,  /* Version of the dispatch table */
    ZARR_NOT_CONSOLIDATED, /* Flags*/

    .list_groups = NCZMD_v2_list_groups,
    .list_variables = NCZMD_v2_list_variables,

    .fetch_json_content = v2_json_content,
};

static const NCZ_Metadata_Dispatcher NCZ_csl_md2_table = {
    ZARRFORMAT2,
    NCZ_METADATA_VERSION, /* Version of the dispatch table */
    ZARR_CONSOLIDATED,	  /* Flags*/

    .list_groups = NCZMD_v2_csl_list_groups,
    .list_variables = NCZMD_v2_csl_list_variables,

    .fetch_json_content = v2_csl_json_content,
};

const NCZ_Metadata_Dispatcher *NCZ_metadata_handler2 = &NCZ_md2_table;
const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler2 = &NCZ_csl_md2_table;

/******************************************************/

int
NCZMD2_initialize(void)
{
    return NC_NOERR;
}

int
NCZMD2_finalize(void)
{
    return NC_NOERR;
}

////////////////////////////////////////////////////

int NCZMD_v2_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames)
{
	size_t i;
	int stat = NC_NOERR;
	char *grpkey = NULL;
	char *subkey = NULL;
	char *zgroup = NULL;
	NClist *matches = nclistnew();

	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	if ((stat = nczmap_list(zfile->map, grpkey, matches)))
		goto done;
	for (i = 0; i < nclistlength(matches); i++)
	{
		const char *name = nclistget(matches, i);
		if (name[0] == NCZM_DOT)
			continue; /* zarr/nczarr specific */
		/* See if name/.zgroup exists */
		if ((stat = nczm_concat(grpkey, name, &subkey)))
			goto done;
		if ((stat = nczm_concat(subkey, Z2GROUP, &zgroup)))
			goto done;
		if ((stat = nczmap_exists(zfile->map, zgroup)) == NC_NOERR)
			nclistpush(subgrpnames, strdup(name));
		stat = NC_NOERR;
		nullfree(subkey);
		subkey = NULL;
		nullfree(zgroup);
		zgroup = NULL;
	}
done:
	nullfree(grpkey);
	nullfree(subkey);
	nullfree(zgroup);
	nclistfreeall(matches);
	return stat;
}

int NCZMD_v2_csl_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames)
{
	size_t i;
	int stat = NC_NOERR;
	char *grpkey = NULL;
	char *subkey = NULL;
	char *zgroup = NULL;
	NClist *matches = nclistnew();
	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	const char *group = grpkey + (grpkey[0] == '/');
	size_t lgroup = strlen(group);

	const NCjson *jmetadata = NULL;
	NCJdictget(zfile->metadata_handler->jcsl, "metadata", &jmetadata);
	for (i = 0; i < NCJarraylength(jmetadata); i += 2)
	{
		NCjson *jname = NCJith(jmetadata, i);
		const char *fullname = NCJstring(jname);
		size_t lfullname = strlen(fullname);

		if (lfullname < lgroup ||
		    strncmp(fullname, group, lgroup) ||
		    (lgroup > 0 && fullname[lgroup] != NCZM_SEP[0]))
		{
			continue;
		}
		char *start = fullname + lgroup + (lgroup > 0);
		char *end = strchr(start, NCZM_SEP[0]);
		if (end == NULL)
			continue;
		size_t lname = end - start;
		// Ends with "/.zgroup
		if (strncmp(Z2METAROOT, end, sizeof(Z2METAROOT)) == 0)
		{
			nclistpush(subgrpnames, strndup(start, lname));
		}
	}
done:
	nullfree(grpkey);
	nullfree(subkey);
	nullfree(zgroup);
	nclistfreeall(matches);
	return stat;
}

int NCZMD_v2_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames)
{
	int stat = NC_NOERR;
	size_t i;
	char *grpkey = NULL;
	char *varkey = NULL;
	char *zarray = NULL;
	NClist *matches = nclistnew();

	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	/* Get the map and search group */
	if ((stat = nczmap_list(zfile->map, grpkey, matches)))
		goto done;
	for (i = 0; i < nclistlength(matches); i++)
	{
		const char *name = nclistget(matches, i);
		if (name[0] == NCZM_DOT)
			continue; /* zarr/nczarr specific */
		/* See if name/.zarray exists */
		if ((stat = nczm_concat(grpkey, name, &varkey)))
			goto done;
		if ((stat = nczm_concat(varkey, Z2ARRAY, &zarray)))
			goto done;
		switch (stat = nczmap_exists(zfile->map, zarray))
		{
		case NC_NOERR:
			nclistpush(varnames, strdup(name));
			break;
		case NC_ENOOBJECT:
			stat = NC_NOERR;
			break; /* ignore */
		default:
			goto done;
		}
		nullfree(varkey);
		varkey = NULL;
		nullfree(zarray);
		zarray = NULL;
	}

done:
	nullfree(grpkey);
	nullfree(varkey);
	nullfree(zarray);
	nclistfreeall(matches);
	return stat;
}

int NCZMD_v2_csl_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames)
{
	size_t i;
	int stat = NC_NOERR;
	char *grpkey = NULL;
	char *varkey = NULL;
	char *zarray = NULL;
	NClist *matches = nclistnew();
	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	const char *group = grpkey + (grpkey[0] == '/');
	size_t lgroup = strlen(group);

	const NCjson *jmetadata = NULL;
	NCJdictget(zfile->metadata_handler->jcsl, "metadata", &jmetadata);
	for (i = 0; i < NCJarraylength(jmetadata); i += 2)
	{
		NCjson *jname = NCJith(jmetadata, i);
		const char *fullname = NCJstring(jname);
		size_t lfullname = strlen(fullname);
		if (lfullname < lgroup ||
		    strncmp(fullname, group, lgroup) ||
		    (lgroup > 0 && fullname[lgroup] != NCZM_SEP[0]))
		{
			continue;
		}
		char *start = fullname + lgroup + (lgroup > 0);
		char *end = strchr(start, NCZM_SEP[0]);
		if (end == NULL)
			continue;
		size_t lname = end - start;
		// Ends with ".zarray"
		if (strncmp("/" Z2ARRAY, end, sizeof("/" Z2ARRAY)) == 0)
		{
			nclistpush(varnames, strndup(start, lname));
		}
	}
done:
	nullfree(grpkey);
	nullfree(varkey);
	nullfree(zarray);
	nclistfreeall(matches);
	return stat;
}

// Static function only valid for V2!
static int zarr_obj_type2suffix(NCZMD_MetadataType zarr_obj_type, const char **suffix){
	switch (zarr_obj_type)
	{
		case NCZMD_GROUP:
			*suffix = Z2GROUP;
			break;
		case NCZMD_ATTRS:
			*suffix = Z2ATTRS;
			break;
		case NCZMD_ARRAY:
			*suffix = Z2ARRAY;
			break;
		default:
			return NC_EINVAL; // 	Invalid type
	}
	return NC_NOERR;
}

int v2_csl_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, NCjson **jobj)
{
	int stat = NC_NOERR;
	NCjson *jtmp = NULL;
	const char *suffix;
	char * key = NULL;
	if ( (stat = zarr_obj_type2suffix(zobj_t, &suffix))
	   ||(stat = nczm_concat(prefix, suffix, &key))){
		return stat;
	}
	
	if (NCJdictget(zfile->metadata_handler->jcsl, "metadata", &jtmp) == 0 
	&& jtmp && NCJsort(jtmp) == NCJ_DICT)
	{
		NCjson *tmp = NULL;
		if ((stat = NCJdictget(jtmp, key + (key[0] == '/'), &tmp)))
			goto done;
		if (tmp)
			NCJclone(tmp, jobj);
	}
done:
	nullfree(key);
	return stat;

}

int v2_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, NCjson **jobj)
{
	int stat = NC_NOERR;
	const char *suffix;
	char * key = NULL;
	if ((stat = zarr_obj_type2suffix(zobj_t, &suffix))
	    || (stat = nczm_concat(prefix, suffix, &key))){
		goto done;
	}

	stat = NCZ_downloadjson(zfile->map, key, jobj);
done:
	nullfree(key);
	return stat;
}
////////////////////////////////////////////////////////////////////////////