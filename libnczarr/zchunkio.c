/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

/*
From Zarr V2 Specification:
"The compressed sequence of bytes for each chunk is stored under
a key formed from the index of the chunk within the grid of
chunks representing the array.  To form a string key for a
chunk, the indices are converted to strings and concatenated
with the period character (".") separating each index. For
example, given an array with shape (10000, 10000) and chunk
shape (1000, 1000) there will be 100 chunks laid out in a 10 by
10 grid. The chunk with indices (0, 0) provides data for rows
0-1000 and columns 0-1000 and is stored under the key "0.0"; the
chunk with indices (2, 4) provides data for rows 2000-3000 and
columns 4000-5000 and is stored under the key "2.4"; etc."
*/

/**
 * @param R Rank
 * @param chunkindices The chunk indices
 * @param keyp Return the chunk key string
 */
int
ncz_buildchunkkey(int R, size64_t* chunkindices, char** keyp)
{
    int stat = NC_NOERR;
    int r;
    NCbytes* key = ncbytesnew();

    if(keyp) *keyp = NULL;
    
    for(r=0;r<R;r++) {
	char index[64];
        if(r > 0) ncbytesappend(key,'.');
	/* Print as decimal with no leading zeros */
	snprintf(index,sizeof(index),"%lu",(unsigned long)chunkindices[r]);	
    }
    ncbytesnull(key);
    if(keyp) *keyp = ncbytesextract(key);

    ncbytesfree(key);
    return stat;
}

/**
 * @internal Push data to chunk of a file.
 * If chunk does not exist, create it
 *
 * @param file Pointer to file info struct.
 * @param proj Chunk projection
 * @param data Buffer containing the chunk data to write
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
 */
int
ncz_put_chunk(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCProjection* proj, void* data)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO* zinfo = NULL;
    NCZ_VAR_INFO* zvar = NULL;
    NCZMAP* map = NULL;
    NCjson* json = NULL;
    NCSlice* slice = NULL;

    LOG((3, "%s: file: %p", __func__, file);

    zinfo = file->format_file_info;
    map = zinfo->map;
    zvar = var->format_var_info;
    slice = proj->slice;

    if(slice->stride == 1) {
	/* We can write directly to the chunk without pre-read */
	if((stat = nczmap_write(map,chunk,start,count,data) != NC_NOERR))
	    goto done;

    } else {/*(slice->stride > 1)*/
	/* We need to pre-read the chunk to avoid overwrites */
	
	if((stat = nczmap_write(map,chunk,start,count,data) != NC_NOERR))
	    goto done;
    }


    /* Write .zarr and store in memory */
    if((stat=NCZ_getdict(map,ZMETAROOT,&json)))
	{stat = NC_ENCZARR; goto done;}
    assert(json->sort = NCJ_DICT);
    if((stat = NCJdictget(json,"zarr_format",&value)))
	goto done;
    if(value->sort != NCJ_INT)
	{stat = NC_ENCZARR; goto done;}
    sscanf(value->value,"%d",zinfo->zarr.zarr_version);
    if((stat = NCJdictget(json,"nczarr_format",&value)))
	goto done;
    if(value->sort != NCJ_STRING)
	{stat = NC_ENCZARR; goto done;}
    zinfo->zarr.nczarr_version = strdup(value->value);

done:
    NCJreclaim(json);
    return stat;
}
