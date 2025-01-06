/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"

/**************************************************/

extern int NCZF2_initialize(void);
extern int NCZF2_finalize(void);

static int list_nodes_csl_v2(NC_FILE_INFO_T*, const char* prefix, struct NClist* matches);
static int listall_nodes_csl_v2(NC_FILE_INFO_T*, const char* prefix, struct NClist* matches);
static int exists_key_csl_v2(NC_FILE_INFO_T* file, const char* prefix);

static int fetch_csl_json_content_v2(NC_FILE_INFO_T* file, NCZMD_MetadataType zarr_obj_type, const char *key, NCjson **jobj);
static int update_csl_json_content_v2(NC_FILE_INFO_T* file, NCZMD_MetadataType zobj_t, const char *prefix, const NCjson *jobj);

static int open_csl_v2(NC_FILE_INFO_T* file);
static int create_csl_v2(NC_FILE_INFO_T* file);
static int close_csl_v2(NC_FILE_INFO_T* file);
static int consolidate_csl_v2(NC_FILE_INFO_T*);

/**************************************************/
static const NCZ_Metadata_Dispatcher NCZ_csl_md2_table = {
        ZARRFORMAT2,
        NCZ_METADATA_VERSION, /* Version of the dispatch table */
        ZARR_CONSOLIDATED,        /* Flags*/
	.list = list_nodes_csl_v2,
	.listall = listall_nodes_csl_v2,
	.exists = exists_key_csl_v2,
	.consolidate = consolidate_csl_v2,
	.close = close_csl_v2,
	.open = open_csl_v2,
	.create = create_csl_v2,
        .fetch_json_content = fetch_csl_json_content_v2,
        .update_json_content = update_csl_json_content_v2,
};
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

/*///////////////////////////////////////////////////
//  .zmetadata dispatch functions
///////////////////////////////////////////////////*/

static int
list_nodes_csl_v2(NC_FILE_INFO_T* file, const char* prefix, NClist* matches)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t i,plen;
    NCZ_Metadata* zmd = &zfile->metadata_handler;
    char* seg1 = NULL;
    NClist* segments = nclistnew();

    assert(zmd->jmeta != NULL && NCJsort(zmd->jmeta)==NCJ_DICT);
    if(prefix[0] == '/') prefix++; /* drop leading '/' for search purposes */
    plen = strlen(prefix);
    /* Walk the metadata nodes and collect the matches */
    for(i=0;i<NCJdictlength(zmd->jmeta);i++) {
	const NCjson* jkey = NCJdictkey(zmd->jmeta,i);
	const char* skey = NCJstring(jkey);
	size_t slen = strlen(skey);
	size_t j, found;
	/* Check for prefix taking root key into acct. */
	if((plen == 0 && slen > 0) || strncmp(skey,prefix,plen) == 0) {
	    const char* suffix = NULL;
	    /* This is a match */
	    suffix = &skey[plen]; /* point past the prefix */
	    assert(strlen(suffix) > 0);
	    nclistclearall(segments);
	    ncz_splitkey(suffix,segments);
	    if(nclistlength(segments) > 0) { /* test if just the prefix */
		seg1 = (char*)nclistremove(segments,0);
		/* suppress duplicates */
		for(found=0,j=0;j<nclistlength(matches);j++) {
		    const char* name = (const char*)nclistget(matches,j);
		    if(strcmp(name,seg1)==0) {found = 1; break;}
		}
		if(!found) nclistpush(matches,seg1); else nullfree(seg1);
		seg1 = NULL;
	    }
	}
    }
    nullfree(seg1);
    nclistfreeall(segments);
    return stat;
}

static int
listall_nodes_csl_v2(NC_FILE_INFO_T* file, const char* prefix, NClist* matches)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t i,plen;
    NCZ_Metadata* zmd = &zfile->metadata_handler;
    NCbytes* key = ncbytesnew();

    assert(zmd->jmeta != NULL && NCJsort(zmd->jmeta)==NCJ_DICT);
    if(prefix[0] == '/') prefix++; /* drop leading '/' for search purposes */
    plen = strlen(prefix);
    ncbytescat(key,"/");
    /* Walk the metadata nodes and collect the matches (with leading '/') */
    for(i=0;i<NCJdictlength(zmd->jmeta);i++) {
	NCjson* jkey = NCJdictkey(zmd->jmeta,i);
	const char* skey = NCJstring(jkey);
	if(strncmp(skey,prefix,plen) > 0) {
	    /* This is a match and is not just the prefix*/
	    ncbytessetlength(key,1);
	    ncbytescat(key,prefix); /* add leading '/' */
	    nclistpush(matches,strdup(ncbytescontents(key)));
	}
    }
    return stat;
}

static int
exists_key_csl_v2(NC_FILE_INFO_T* file, const char* prefix)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    size_t i,plen;
    NCZ_Metadata* zmd = &zfile->metadata_handler;

    assert(zmd->jmeta != NULL && NCJsort(zmd->jmeta)==NCJ_DICT);
    if(prefix[0] == '/') prefix++; /* drop leading '/' for search purposes */
    plen = strlen(prefix);
    /* Walk the metadata nodes and see if there is a prefix match */
    for(i=0;i<NCJdictlength(zmd->jmeta);i++) {
	NCjson* jkey = NCJdictkey(zmd->jmeta,i);
	const char* skey = NCJstring(jkey);
	if(strncmp(skey,prefix,plen) == 0) {stat = NC_NOERR; goto done;}
    }
    stat = NC_ENOOBJECT;
done:
    return stat;
}

static int
fetch_csl_json_content_v2(NC_FILE_INFO_T* file, NCZMD_MetadataType zobj_t, const char* key, NCjson **jobjp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCjson* jobj = NULL;
    NCjson *jkey = NULL;
    NCZ_Metadata* zmd = &zfile->metadata_handler;

    NC_UNUSED(zobj_t);
    assert(zmd->jmeta != NULL);
    if(key[0] == '/') key++; /* remove any leading key */
    /* Meta-data is stored a mostly flat format using the whole key (with leading / removed) */
    if ((stat = NCJdictget(zmd->jmeta, key, (NCjson**)&jkey))) goto done;
    NCJcheck(NCJclone(jkey, &jobj));
    if(jobj !=  NULL)
        {if(jobjp) {*jobjp = jobj; jobj = NULL;}}
done:
    return stat;
}

static int
update_csl_json_content_v2(NC_FILE_INFO_T* file, NCZMD_MetadataType zobj_t, const char *key, const NCjson *jobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCjson* jclone = NULL;
    NCZ_Metadata* zmd = &zfile->metadata_handler;

    NC_UNUSED(zobj_t);
    if(key[0] == '/') key++;

    /* Create the metadata skeleton if it does not exist */
    if(zmd->jcsl == NULL) {
	if((stat = NCJparse(MINIMIM_CSL_REP2_RAW,0,&zmd->jcsl))) goto done;
	NCJcheck(NCJdictget(zmd->jcsl,"metadata",(NCjson**)&zmd->jmeta));
    }
    /* Insert/overwrite the key+value */
    NCJcheck(NCJclone(jobj,&jclone));
    NCJcheck(NCJinsert((NCjson*)zmd->jmeta,key,jclone));
    zmd->dirty = 1;

#if 0 /* Do we need this? */
    // Allocating representation if doesn't exist
    // Updating the internal JSON representation to be synced later
    NCjson * jrep = NULL;
    if ((stat = NCJdictget(zfile->metadata_handler.jcsl,"metadata", (NCjson**)&jrep)) || jrep == NULL) {
            goto done;
    }
#endif /*0*/
    
done:
    return stat;
}

////////////////////////////////////////////////////////////////////////////
/* Writes .zmetadata file into storage */

static int
consolidate_csl_v2(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata* zmd = &zfile->metadata_handler;

    if (zmd->dirty) {
	if(zmd->jcsl == NULL) {stat = NC_EZARRMETA; goto done;}
        stat = NCZ_uploadjson(zfile->map, Z2METADATA ,zmd->jcsl);
	zmd->dirty = 0;
    }
done:
    return stat;
}

static int
close_csl_v2(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata* zmd = &zfile->metadata_handler;
    NCJreclaim(zmd->jcsl);
    zmd->jcsl = NULL;
    zmd->jmeta = NULL;
    return stat;
}

static int
open_csl_v2(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata* zmd = &zfile->metadata_handler;

    /* Read /.zmetadata */
    if(zmd->jcsl == NULL) {
        if((stat = NCZ_downloadjson(zfile->map,Z2METADATA,&zmd->jcsl))) goto done;
    }
    if(zmd->jcsl == NULL || NCJsort(zmd->jcsl) != NCJ_DICT) {stat = NC_EZARRMETA; goto done;}
    /* Pull out the "metadata" key and save it */
    NCJcheck(NCJdictget(zmd->jcsl,"metadata",(NCjson**)&zmd->jmeta));
    if(zmd->jmeta == NULL || NCJsort(zmd->jmeta) != NCJ_DICT) {stat = NC_EZARRMETA; goto done;}
done:
    return stat;
}

static int
create_csl_v2(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_Metadata* zmd = &zfile->metadata_handler;

    /* Create the JSON skeleton */
    if((stat = NCJparse(MINIMIM_CSL_REP2_RAW,0,&zmd->jcsl))) goto done; /* Create the metadata skeleton */
    /* Pull out the "metadata" key and save it */
    NCJcheck(NCJdictget(zmd->jcsl,"metadata",(NCjson**)&zmd->jmeta));

done:
    return stat;
}

