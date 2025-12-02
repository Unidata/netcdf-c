/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/**************************************************/

extern int NCZF2_initialize(void);
extern int NCZF2_finalize(void);

#define  MINIMIM_CSL_REP_RAW "{\"metadata\":{},\"zarr_consolidated_format\":1}"

int NCZMD_v2_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
int NCZMD_v2_csl_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);

int NCZMD_v2_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
int NCZMD_v2_csl_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);

int fetch_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);
int fetch_csl_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);

int update_csl_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, const NCjson *jobj);
int update_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, const NCjson *jobj);

int validate_consolidated_json_noop_v2(const NCjson *json);
int validate_consolidated_json_v2(const NCjson *json);

/**************************************************/

static const NCZ_Metadata_Dispatcher NCZ_md2_table = {
	ZARRFORMAT2,
	NCZ_METADATA_VERSION,  /* Version of the dispatch table */
	ZARR_NOT_CONSOLIDATED, /* Flags*/

	.list_groups = NCZMD_v2_list_groups,
	.list_variables = NCZMD_v2_list_variables,

	.fetch_json_content = fetch_json_content_v2,
	.update_json_content = update_json_content_v2,
    .validate_consolidated = validate_consolidated_json_noop_v2,
};

static const NCZ_Metadata_Dispatcher NCZ_csl_md2_table = {
	ZARRFORMAT2,
	NCZ_METADATA_VERSION, /* Version of the dispatch table */
	ZARR_CONSOLIDATED,	  /* Flags*/

	.list_groups = NCZMD_v2_csl_list_groups,
	.list_variables = NCZMD_v2_csl_list_variables,

	.fetch_json_content = fetch_csl_json_content_v2,
	.update_json_content = update_csl_json_content_v2,
    .validate_consolidated = validate_consolidated_json_v2,
};

const NCZ_Metadata_Dispatcher *NCZ_metadata_handler2 = &NCZ_md2_table;
const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler2 = &NCZ_csl_md2_table;

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
	/* Get the map and search group */
	if ((stat = nczmap_search(zfile->map, grpkey, matches)))
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
	NCJdictget(zfile->metadata_handler.jcsl, "metadata", &jmetadata);
	for (i = 0; i < NCJdictlength(jmetadata); i++)
	{
		NCjson *jname = NCJdictkey(jmetadata, i);
		const char *fullname = NCJstring(jname);
		size_t lfullname = strlen(fullname);
		if (lfullname < lgroup ||
			strncmp(fullname, group, lgroup) ||
			(lgroup > 0 && fullname[lgroup] != NCZM_SEP[0]))
		{
			continue;
		}
		const char *start = fullname + lgroup + (lgroup > 0);
		const char *end = strchr(start, NCZM_SEP[0]);
		if (end == NULL || end <= start)
			continue;
		size_t lname = (size_t)(end - start);
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
	size_t i;
	int stat = NC_NOERR;
	char *grpkey = NULL;
	char *varkey = NULL;
	char *zarray = NULL;
	NClist *matches = nclistnew();

	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	/* Get the map and search group */
	if ((stat = nczmap_search(zfile->map, grpkey, matches)))
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
		if ((stat = nczmap_exists(zfile->map, zarray)) == NC_NOERR)
			nclistpush(varnames, strdup(name));
		stat = NC_NOERR;
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
	NCJdictget(zfile->metadata_handler.jcsl, "metadata", &jmetadata);
	for (i = 0; i < NCJdictlength(jmetadata); i++)
	{
		NCjson *jname = NCJdictkey(jmetadata, i);
		const char *fullname = NCJstring(jname);
		size_t lfullname = strlen(fullname);
		if (lfullname < lgroup ||
			strncmp(fullname, group, lgroup) ||
			(lgroup > 0 && fullname[lgroup] != NCZM_SEP[0]))
		{
			continue;
		}
		const char *start = fullname + lgroup + (lgroup > 0);
		const char *end = strchr(start, NCZM_SEP[0]);
		if (end == NULL || end <= start)
			continue;
		size_t lname = (size_t)(end - start);
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

int fetch_csl_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, NCjson **jobj)
{
	int stat = NC_NOERR;
	const NCjson *jtmp = NULL;
	const char *suffix;
	char * key = NULL;
	if ( (stat = zarr_obj_type2suffix(zobj_t, &suffix))
	   ||(stat = nczm_concat(prefix, suffix, &key))){
		return stat;
	}
	
	if (NCJdictget(zfile->metadata_handler.jcsl, "metadata", &jtmp) == 0 
	&& jtmp && NCJsort(jtmp) == NCJ_DICT)
	{
		NCjson *tmp = NULL;
		if ((stat = NCJdictget(jtmp, key + (key[0] == '/'), (const NCjson**)&tmp)))
			goto done;
		if (tmp)
			NCJclone(tmp, jobj);
	}
done:
	nullfree(key);
	return stat;

}

int fetch_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, NCjson **jobj)
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
//		 Write to internal JSON pointer and/or directly to storage
/////////////////////////////
int update_csl_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, const NCjson *jobj)
{
	int stat = NC_NOERR;

	// uses normal implementation to write all the .z files
	if ((stat=update_json_content_v2(zfile,zobj_t,prefix,jobj))){
		goto done;
	}
	// Allocating representation if doesn't exist
	if (zfile->metadata_handler.jcsl == NULL && 
		(stat = NCJparse(MINIMIM_CSL_REP_RAW,0,&zfile->metadata_handler.jcsl))){
		goto done;
	}
	// Updating the internal JSON representation to be synced later
	NCjson * jrep = NULL;
	if ((stat = NCJdictget(zfile->metadata_handler.jcsl,"metadata", (const NCjson**)&jrep)) || jrep == NULL) {
		goto done;
	}
	
	const char *suffix;
	char * key = NULL;
	if ((stat = zarr_obj_type2suffix(zobj_t, &suffix))
		|| (stat = nczm_concat(prefix, suffix, &key))){
		goto done;
	}
	// Concatenate will add separator as prefix if prefix NULL
	const char * mdkey= key[0] == '/'?key+1:key;
	NCjson * jval = NULL;
	NCJclone(jobj,&jval);
	// We overwrite existing values if key is the same
	NCJinsert(jrep, mdkey, jval);
done:
	// No frees at this point
	free(key);
	return stat;

}

int update_json_content_v2(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, const NCjson *jobj)
{
	int stat = NC_NOERR;
	const char *suffix;
	char * key = NULL;
	if ((stat = zarr_obj_type2suffix(zobj_t, &suffix))
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

// Checks if the content of .zmetadata contains a valid non empty json
//  - with non empty json in "metadata"
//  - with "zarr_consolidated_format" == 1
int validate_consolidated_json_v2(const NCjson *json)
{
    if (json == NULL || NCJsort(json) != NCJ_DICT || NCJdictlength(json) == 0)
        return NC_EZARRMETA;

    const NCjson *jtmp = NULL;
    NCJdictget(json, "metadata", &jtmp);
    if (jtmp == NULL || NCJsort(jtmp) != NCJ_DICT || NCJdictlength(jtmp) == 0)
        return NC_EZARRMETA;

    jtmp = NULL;
    struct NCJconst format = {0,0,0,0};
    NCJdictget(json, "zarr_consolidated_format", &jtmp);
    if (jtmp == NULL || NCJsort(jtmp) != NCJ_INT || NCJcvt(jtmp,NCJ_INT, &format ) || format.ival != 1)
        return NC_EZARRMETA;

    return NC_NOERR;
}
