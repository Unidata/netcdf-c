/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

#define  MINIMIM_CSL_REP_RAW "{\"metadata\":{},\"zarr_consolidated_format\":1}"

int NCZMD_v2_list_nodes(NCZ_FILE_INFO_T *zfile, const char * key, NClist *groups, NClist *vars);

int NCZMD_v2_list_groups(NCZ_FILE_INFO_T *zfile, const char * key, NClist *groups);
int NCZMD_v2_list_variables(NCZ_FILE_INFO_T *zfile, const char * key, NClist * variables);
int fetch_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);

int update_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj, const char *key, const NCjson *jobj);

int validate_consolidated_json_noop_v2(const NCjson *json);

static const NCZ_Metadata NCZ_md2_table = {
	ZARRFORMAT2,
	NCZ_METADATA_VERSION,
	ZARR_NOT_CONSOLIDATED,
	.jcsl = NULL,

	.list_nodes = NCZMD_v2_list_nodes,
	.list_groups = NCZMD_v2_list_groups,
	.list_variables = NCZMD_v2_list_variables,

	.fetch_json_content = fetch_json_content_v2,
	.update_json_content = update_json_content_v2,
    .validate_consolidated = validate_consolidated_json_noop_v2,
};

const NCZ_Metadata *NCZ_metadata_handler2 = &NCZ_md2_table;

int NCZMD_v2_list_nodes(NCZ_FILE_INFO_T *zfile, const char * key, NClist *groups, NClist *variables)
{
	size_t i;
	int stat = NC_NOERR;
	char *subkey = NULL;
	char *zkey = NULL;
	NClist *matches = nclistnew();

	if ((stat = nczmap_search(zfile->map, key, matches)))
		goto done;
	for (i = 0; i < nclistlength(matches); i++)
	{
		const char *name = nclistget(matches, i);
		if (name[0] == NCZM_DOT)
			continue;
		if ((stat = nczm_concat(key, name, &subkey)))
			goto done;
		if ((stat = nczm_concat(subkey, Z2GROUP, &zkey)))
			goto done;
		if (NC_NOERR == nczmap_exists(zfile->map, zkey) && groups != NULL)
			nclistpush(groups, strdup(name));

		nullfree(zkey);
		zkey = NULL;
		if ((stat = nczm_concat(subkey, Z2ARRAY, &zkey)))
			goto done;
		if (NC_NOERR == nczmap_exists(zfile->map, zkey) && variables != NULL)
			nclistpush(variables, strdup(name));
		stat = NC_NOERR;

		nullfree(subkey);
		subkey = NULL;
		nullfree(zkey);
		zkey = NULL;
	}

done:
	nullfree(subkey);
	nullfree(zkey);
	nclistfreeall(matches);
	return stat;
}


int NCZMD_v2_list_groups(NCZ_FILE_INFO_T *zfile, const char * key, NClist *groups)
{
	return NCZMD_v2_list_nodes(zfile, key, groups, NULL);
}

int NCZMD_v2_list_variables(NCZ_FILE_INFO_T *zfile, const char * key, NClist *variables)
{
	return NCZMD_v2_list_nodes(zfile, key, NULL, variables);
}

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
			return NC_EINVAL;
	}
	return NC_NOERR;
}

int fetch_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj, const char *prefix, NCjson **jobj)
{
	int stat = NC_NOERR;
	const char *suffix;
	char * key = NULL;
	if ((stat = zarr_obj_type2suffix(zobj, &suffix))
		|| (stat = nczm_concat(prefix, suffix, &key))){
		goto done;
	}

	stat = NCZ_downloadjson(zfile->map, key, jobj);
done:
	nullfree(key);
	return stat;
}

int update_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj, const char *prefix, const NCjson *jobj)
{
	int stat = NC_NOERR;
	const char *suffix;
	char * key = NULL;
	if ((stat = zarr_obj_type2suffix(zobj, &suffix))
		|| (stat = nczm_concat(prefix, suffix, &key))){
		goto done;
	}

	stat = NCZ_uploadjson(zfile->map, key, jobj);
done:
	nullfree(key);
	return stat;
}

int validate_consolidated_json_noop_v2(const NCjson *json){
    NC_UNUSED(json);
    return NC_NOERR;
}
