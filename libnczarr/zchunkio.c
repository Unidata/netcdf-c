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

int
buildchunkkey(int R, size_t* chunkindices, char** keyp)
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

int
readchunk(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, size_t* chunkindices, void** chunkdatap)
{
    /* set the contents of the specified chunk into chunkdatap */
    return NC_NOERR;
}

