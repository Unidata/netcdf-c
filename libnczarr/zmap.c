/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

/*
This API isolates the key-value pair mapping code
from the Zarr-based implementation of NetCDF-4.

It wraps an internal C dispatch table manager
for implementing an abstract data structurer
loosely based on the Amazon S3 storage model.

Technically, S3 is a Key-Value Pair model
mapping a text key to an S3 *object*.
The object has an associated small set of what
I will call tags, which are themselves of the
form of key-value pairs, but where the key and value
are always text. As far as I can tell, Zarr never
uses these tags, so we do not include them in the zmap
data structure.

In practice, S3 is actually a tree
where the "contains" relationship is determined
by matching prefixes of the object keys.
So in this sense the object whose name is "/x/y/z"
is contained in the object whose name is "/x/y".

For this API, we use the prefix approach so that
for example, creating an object contained in another
object is defined by the common prefix model.

*/

#include "zincludes.h"

/**************************************************/
/* Import the current implementations */

extern NCZMAP_DS_API zmap_nc4;

/**************************************************/

/* Create ; complain if already exists */
int
nczmap_create(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    NCURI* uri = NULL;
    const char* filepath = NULL; /* if path indicates a local file */
    
    if(path == NULL || strlen(path) == 0)
	{stat = NC_EINVAL; goto done;}

    if(mapp) *mapp = NULL;

    /* See if this is a local file */
    if(ncuriparse(path,&uri))
	filepath = path;
    else if(strcmp(uri->protocol,"file")==0)
	filepath = uri->path;

    switch (impl) {
    case NCZM_NC4:
	if(filepath == NULL)
	     {stat = NC_ENOTNC; goto done;}
        stat = zmap_nc4.create(filepath, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
    default:
	{stat = NC_ENOTBUILT; goto done;}
    }
    if(mapp) *mapp = map;
done:
    ncurifree(uri);
    return THROW(stat);
}

/*
Terminology:
protocol: map implementation
format: zarr | tiledb
*/

int
nczmap_open(NCZM_IMPL impl, const char *path0, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    NCURI* uri = NULL;
    const char* filepath = NULL; /* if path indicates a local file */

    if(path0 == NULL || strlen(path0) == 0)
	{stat = NC_EINVAL; goto done;}

    if(mapp) *mapp = NULL;

    /* See if this is a local file */
    if(ncuriparse(path0,&uri))
	filepath = path0;
    else if(strcmp(uri->protocol,"file")==0)
	filepath = uri->path;

    switch (impl) {
    case NCZM_NC4:
	if(filepath == NULL)
	     {stat = NC_ENOTNC; goto done;}
        stat = zmap_nc4.open(filepath, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
    default:
	{stat = NC_ENOTBUILT; goto done;}
    }

done:
    ncurifree(uri);
    if(!stat) {
        if(mapp) *mapp = map;
    }
    return THROW(stat);
}

int
nczmap_close(NCZMAP* map, int delete)
{
    int stat = NC_NOERR;
    if(map && map->api)
        stat = map->api->close(map,delete);
    return THROW(stat);
}

/**************************************************/
/* API Wrapper */

int
nczmap_exists(NCZMAP* map, const char* key)
{
    return map->api->exists(map, key);
}

int
nczmap_len(NCZMAP* map, const char* key, ssize64_t* lenp)
{
    return map->api->len(map, key, lenp);
}

int
nczmap_def(NCZMAP* map, const char* key, ssize64_t len)
{
    return map->api->def(map, key, len);
}

int
nczmap_read(NCZMAP* map, const char* key, ssize64_t start, ssize64_t count, void* content)
{
    return map->api->read(map, key, start, count, content);
}

int
nczmap_write(NCZMAP* map, const char* key, ssize64_t start, ssize64_t count, const void* content)
{
    return map->api->write(map, key, start, count, content);
}

int
nczmap_readmeta(NCZMAP* map, const char* key, ssize64_t count, char* content)
{
    return map->api->readmeta(map, key, count, content);
}

int
nczmap_writemeta(NCZMAP* map, const char* key, ssize64_t count, const char* content)
{
    return map->api->writemeta(map, key, count, content);
}

int
nczmap_search(NCZMAP* map, const char* prefix, NClist* matches)
{
    return map->api->search(map, prefix, matches);
}

/**************************************************/
/* Utilities */

/* Split a path into pieces along '/' character; elide any leading '/' */
int
nczm_split(const char* path, NClist* segments)
{
    int stat = NC_NOERR;
    const char* p = NULL;
    const char* q = NULL;
    ptrdiff_t len = 0;
    char* seg = NULL;

    if(path == NULL || strlen(path)==0 || segments == NULL)
	{stat = NC_EINVAL; goto done;}

    p = path;
    if(p[0] == NCZM_SEP[0]) p++;
    for(;*p;) {
	q = strchr(p,NCZM_SEP[0]);
	if(q==NULL)
	    q = p + strlen(p); /* point to trailing nul */
        len = (q - p);
	if(len == 0)
	    {stat = NC_EURL; goto done;}
	if((seg = malloc(len+1)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	memcpy(seg,p,len);
	seg[len] = '\0';
	nclistpush(segments,seg);
	seg = NULL; /* avoid mem errors */
	if(*q) p = q+1; else p = q;
    }

done:
    if(seg != NULL) free(seg);
    return THROW(stat);
}

/* Join the first nseg segments into a path using '/' character */
int
nczm_joinprefix(NClist* segments, int nsegs, char** pathp)
{
    int stat = NC_NOERR;
    int i;
    NCbytes* buf = NULL;

    if(segments == NULL)
	{stat = NC_EINVAL; goto done;}
    if((buf = ncbytesnew()))
	{stat = NC_ENOMEM; goto done;}
    if(nclistlength(segments) < nsegs)
	nsegs = nclistlength(segments);
    if(nsegs == 0) {
	ncbytescat(buf,NCZM_SEP);
	goto done;		
    }
    for(i=0;i<nsegs;i++) {
	const char* seg = nclistget(segments,i);
	ncbytescat(buf,NCZM_SEP);
	ncbytescat(buf,seg);		
    }

done:
    if(!stat) {
	if(pathp) *pathp = ncbytesextract(buf);
    }
    ncbytesfree(buf);
    return THROW(stat);
}

/* Convenience: Join all segments into a path using '/' character */
int
nczm_join(NClist* segments, char** pathp)
{
    return nczm_joinprefix(segments,nclistlength(segments),pathp);
}

/* Convenience: suffix an object name to a group path: caller frees*/
int
nczm_suffix(const char* prefix, const char* suffix, char** pathp)
{
    NCbytes* buf = ncbytesnew();

    if(prefix == NULL || strlen(prefix)==0) prefix = NCZM_SEP;
    if(suffix == NULL) suffix = "";
    ncbytescat(buf,prefix);
    if(ncbytesget(buf,ncbyteslength(buf)-1) == NCZM_SEP[0])
	ncbytessetlength(buf,ncbyteslength(buf)-1);
    if(strlen(suffix) > 0 && suffix[0] != NCZM_SEP[0])
	ncbytescat(buf,NCZM_SEP);
    ncbytescat(buf,suffix);
    if(pathp) *pathp = ncbytesextract(buf);
    ncbytesfree(buf);
    return NC_NOERR;
}

int
nczm_clear(NCZMAP* map)
{
    if(map) 
	nullfree(map->url);
    return NC_NOERR;
}
