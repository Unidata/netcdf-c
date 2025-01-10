/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/**************************************************/

extern int NCZF0_initialize(void);
extern int NCZF0_finalize(void);

static int list_nodes(NC_FILE_INFO_T*, const char* prefix, struct NClist* matches);
static int listall_nodes(NC_FILE_INFO_T*, const char* prefix, struct NClist* matches);
static int exists_nodes(NC_FILE_INFO_T* file, const char* prefix);

static int fetch_json_content(NC_FILE_INFO_T* file, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);
static int update_json_content(NC_FILE_INFO_T* file, NCZMD_MetadataType zobj_t, const char *prefix, const NCjson *jobj);

static int open_noop(NC_FILE_INFO_T* file);
static int create_noop(NC_FILE_INFO_T* file);
static int close_noop(NC_FILE_INFO_T* file);
static int consolidate_noop(NC_FILE_INFO_T*);

/**************************************************/

static const NCZ_Metadata_Dispatcher NCZ_md_table = { /* Common to V2 and V3 */
        ZARRFORMAT2,
        NCZ_METADATA_VERSION,  /* Version of the dispatch table */
        ZARR_NOT_CONSOLIDATED, /* Flags*/
	.list = list_nodes,
	.listall = listall_nodes,
	.exists = exists_nodes,
	.open = open_noop,
	.create = create_noop,
	.close = close_noop,
	.consolidate = consolidate_noop,
        .fetch_json_content = fetch_json_content,
        .update_json_content = update_json_content,
};
const NCZ_Metadata_Dispatcher *NCZ_metadata_handler= &NCZ_md_table;

/******************************************************/

int
NCZMD0_initialize(void)
{
    return NC_NOERR;
}

int
NCZMD0_finalize(void)
{
    return NC_NOERR;
}

/*///////////////////////////////////////////////////
//  .zmetadata dispatch functions
///////////////////////////////////////////////////*/

static int
list_nodes(NC_FILE_INFO_T* file, const char* prefix, NClist* matches)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    if((stat = nczmap_list(zfile->map,prefix,matches))) goto done;
done:
    return stat;
}

static int
listall_nodes(NC_FILE_INFO_T* file, const char* prefix, NClist* matches)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    if((stat = nczmap_listall(zfile->map,prefix,matches))) goto done;
done:
    return stat;
}

static int
exists_nodes(NC_FILE_INFO_T* file, const char* prefix)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    if((stat = nczmap_exists(zfile->map,prefix))) goto done;
done:
    return stat;
}

static int
fetch_json_content(NC_FILE_INFO_T* file, NCZMD_MetadataType zobj_t, const char *key, NCjson **jobjp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCjson* jobj = NULL;

    NC_UNUSED(zobj_t);
    if((stat = NCZ_downloadjson(zfile->map, key, &jobj))) goto done;
    if(jobjp) {*jobjp = jobj; jobj = NULL;}
done:
    NCJreclaim(jobj);
    return stat;
}

static int
update_json_content(NC_FILE_INFO_T* file, NCZMD_MetadataType zobj_t, const char* key, const NCjson *jobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;

    NC_UNUSED(zobj_t);
    if((stat = NCZ_uploadjson(zfile->map, key, jobj))) goto done;
    zfile->metadata_handler.dirty = 0; /* after writing */
    
done:
    return stat;
}


static int
open_noop(NC_FILE_INFO_T* file)
{
    NC_UNUSED(file);
    return NC_NOERR;
}

static int
create_noop(NC_FILE_INFO_T* file)
{
    NC_UNUSED(file);
    return NC_NOERR;
}

static int
close_noop(NC_FILE_INFO_T* file)
{
    NC_UNUSED(file);
    return NC_NOERR;
}

static int
consolidate_noop(NC_FILE_INFO_T* file)
{
    NC_UNUSED(file);
    return NC_NOERR;
}
