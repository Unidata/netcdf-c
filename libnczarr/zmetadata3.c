/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zmetadata.h"

/**************************************************/

extern int NCZF3_initialize(void);
extern int NCZF3_finalize(void);

int NCZMD_v3_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
int NCZMD_v3_csl_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);

int NCZMD_v3_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);
int NCZMD_v3_csl_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames);


int v3_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);
int v3_csl_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);

/**************************************************/

static const NCZ_Metadata_Dispatcher NCZ_md3_table = {
    ZARRFORMAT3,
    NCZ_METADATA_VERSION,  /* Version of the dispatch table */
    ZARR_NOT_CONSOLIDATED, /* Flags*/

    NCZMD_v3_list_groups,
    NCZMD_v3_list_variables,

    .fetch_json_content = v3_json_content,
};

static const NCZ_Metadata_Dispatcher NCZ_csl_md3_table = {
    ZARRFORMAT3,
    NCZ_METADATA_VERSION, /* Version of the dispatch table */
    ZARR_CONSOLIDATED,	  /* Flags*/

    NCZMD_v3_csl_list_groups,
    NCZMD_v3_csl_list_variables,

    .fetch_json_content = v3_csl_json_content,
};

const NCZ_Metadata_Dispatcher *NCZ_metadata_handler3 = &NCZ_md3_table;
const NCZ_Metadata_Dispatcher *NCZ_csl_metadata_handler3 = &NCZ_csl_md3_table;
int
NCZMD3_initialize(void)
{
    return NC_NOERR;
}

int
NCZMD3_finalize(void)
{
    return NC_NOERR;
}

////////////////////////////////////////////////////

int NCZMD_v3_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames)
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
		if ((stat = nczm_concat(subkey, Z3GROUP, &zgroup)))
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

int extract_node_types(const NCjson *jobj, const char * prefix, const char *node_type, NClist **nodenames){
	size_t lprefix = strlen(prefix);
	for (int i = 0; i < NCJarraylength(jobj); i += 2)	{
		const NCjson *jname = NCJith(jobj, i);
		const NCjson *jvalue = NCJith(jobj, i+1);

		//check if json types are the expected string for key and dict for value
		if (NCJsort(jname) != NCJ_STRING || NCJsort(jvalue) != NCJ_DICT){
			continue;
		}
		// check if it value dict has key "node_type" and matches "array"
		if (NCJdictget(jvalue, "node_type", &jvalue) || NCJsort(jvalue) != NCJ_STRING 
		|| strncmp(node_type, NCJstring(jvalue), strlen(node_type + 1))){
			continue;
		}
		// check if prefix matches item's name but ensure it's not looking at itself
		if(strncmp(prefix, NCJstring(jname),lprefix) || lprefix == strlen(NCJstring(jname))){
			continue;
		}
	
		// if we've reach here it means it's the correct type for sure
		// but we must check if it's directly under our current group

		const char *start = NCJstring(jname) + lprefix + (lprefix > 0);
		if (strchr(start, NCZM_SEP[0])) // Zarr separator found => not a direct group under grpkey
			continue;

		// It's a subgroup
		const size_t lname = strlen(start);
		nclistpush(nodenames, strndup(start, lname));
	}
	return NC_NOERR;
}

int NCZMD_v3_csl_list_groups(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *subgrpnames)
{
	int stat = NC_NOERR;
	char *grpkey = NULL;
	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	// In the consolidated representation '/' is never observed as the key's first char
	const char *group = grpkey + (grpkey[0] == '/'); 
	const size_t lgroup = strlen(group);

	const NCjson *jmetadata = NULL;
	if (( stat = NCJdictget(zfile->metadata_handler->jcsl, "consolidated_metadata", &jmetadata) ) || jmetadata == NULL || NCJsort(jmetadata) != NCJ_DICT){
		goto done;
	}

	if (( stat = NCJdictget(jmetadata, "metadata", &jmetadata) ) || jmetadata == NULL || NCJsort(jmetadata) != NCJ_DICT){
		goto done;
	}


	// for each key, check if is a dict and has node_type ="group"
	extract_node_types(jmetadata, group, "group", subgrpnames);
	
done:
	nullfree(grpkey);
	return stat;
}


int NCZMD_v3_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames)
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
		if ((stat = nczm_concat(varkey, Z3ARRAY, &zarray)))
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

int NCZMD_v3_csl_list_variables(NCZ_FILE_INFO_T *zfile, NC_GRP_INFO_T *grp, NClist *varnames)
{
	size_t i;
	int stat = NC_NOERR;
	char *grpkey = NULL;
	/* Compute the key for the grp */
	if ((stat = NCZ_grpkey(grp, &grpkey)))
		goto done;
	// In the consolidated representation '/' is never observed as the key's first char
	const char *group = grpkey + (grpkey[0] == '/'); 
	const size_t lgroup = strlen(group);

	const NCjson *jmetadata = NULL;
	if (( stat = NCJdictget(zfile->metadata_handler->jcsl, "consolidated_metadata", &jmetadata) ) || jmetadata == NULL || NCJsort(jmetadata) != NCJ_DICT){
		goto done;
	}

	if (( stat = NCJdictget(jmetadata, "metadata", &jmetadata) ) || jmetadata == NULL || NCJsort(jmetadata) != NCJ_DICT){
		goto done;
	}

	// for each key, check if is a dict and has node_type ="array"
	extract_node_types(jmetadata, group, "array", varnames);

done:
	nullfree(grpkey);
	return stat;
}

// Returns error if consolidated metadata is not found 
// node_name should match item name on the consolidated representation, meaning no '/' prefix
static int get_consolidated_json_node(const NCjson * zobj, const char* c_node_name ,const char * node_type, const NCjson ** jitem){
	size_t i;
	int stat = NC_NOERR;

	// In the consolidated representation '/' is never observed as the key's first char
	const char *node_name = c_node_name + (c_node_name[0] == '/'); 
	const size_t lnode_name = strlen(node_name);

	const NCjson *jmetadata = NULL;
	if (( stat = NCJdictget(zobj, "consolidated_metadata", &jmetadata) ) || jmetadata == NULL || NCJsort(jmetadata) != NCJ_DICT){
		goto done;
	}

	if (( stat = NCJdictget(jmetadata, "metadata", &jmetadata) ) || jmetadata == NULL || NCJsort(jmetadata) != NCJ_DICT){
		goto done;
	}

	// for each key, check if is a dict and has matching node_type 
	for (i = 0; i < NCJarraylength(jmetadata); i += 2)	{
		const NCjson *jname = NCJith(jmetadata, i);
		if (NCJsort(jname) != NCJ_STRING || strncmp(NCJstring(jname),node_name, lnode_name+1)){
			continue;
		}

		const NCjson *jvalue = NCJith(jmetadata, i+1);
		//check if json types are the expected string for key and dict for value
		if (NCJsort(jname) != NCJ_STRING ||NCJsort(jvalue) != NCJ_DICT){
			continue;
		}

		// check if it value dict has key "node_type" and matches node_type ["array","group"]
		if (NCJsort(jvalue) != NCJ_DICT	|| NCJdictget(jvalue, "node_type", &jname) || jname == NULL 
		 || NCJsort(jname) != NCJ_STRING || strncmp(node_type, NCJstring(jname),strlen(node_type))){
			continue;
		}
		
		*jitem = jvalue;
		break; //object found!		
	}
done:
	return stat;
}

// Static function only valid for v3!
static int zarr_obj_type2suffix(NCZMD_MetadataType zarr_obj_type, const char **suffix){
	switch (zarr_obj_type)
	{
		case NCZMD_GROUP:
			*suffix = Z3GROUP;
			break;
		case NCZMD_ATTRS:
			*suffix = Z3ATTRS;
			break;
		case NCZMD_ARRAY:
			*suffix = Z3ARRAY;
			break;
		default:
			return NC_EINVAL; // 	Invalid type
	}
	return NC_NOERR;
}

int v3_csl_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, NCjson **jobj)
{
	int stat = NC_NOERR;
	const char *suffix;
	char * key = NULL;
	if ( (stat = zarr_obj_type2suffix(zobj_t, &suffix))
	   ||(stat = nczm_concat(prefix, suffix, &key))){
		goto done;
	}
	
	if (prefix == NULL || (*prefix == '/' && strlen(prefix)==1) ){
		stat = NCJclone(zfile->metadata_handler->jcsl, jobj);
		goto done;		
	}


	stat = get_consolidated_json_node(zfile->metadata_handler->jcsl, prefix, "group", jobj);

done:
	nullfree(key);
	return stat;

}

int v3_json_content(NCZ_FILE_INFO_T *zfile, NCZMD_MetadataType zobj_t, const char *prefix, NCjson **jobj)
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