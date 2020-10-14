/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "zincludes.h"
#include "ncpathmgr.h"

/**************************************************/
/* Import the current implementations */

extern NCZMAP_DS_API zmap_nz4;
extern NCZMAP_DS_API zmap_nzf;
#ifdef ENABLE_S3_SDK
extern NCZMAP_DS_API zmap_s3sdk;
#endif

/**************************************************/
int
nczmap_create(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    NCURI* uri = NULL;
    
    if(path == NULL || strlen(path) == 0)
	{stat = NC_EINVAL; goto done;}

    if(mapp) *mapp = NULL;

    switch (impl) {
    case NCZM_NC4:
        stat = zmap_nz4.create(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
    case NCZM_FILE:
        stat = zmap_nzf.create(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#ifdef ENABLE_S3_SDK
    case NCZM_S3:
        stat = zmap_s3sdk.create(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#endif
    default:
	{stat = NC_ENOTBUILT; goto done;}
    }
    if(mapp) *mapp = map;
done:
    ncurifree(uri);
    return THROW(stat);
}

int
nczmap_open(NCZM_IMPL impl, const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    NCURI* uri = NULL;

    if(path == NULL || strlen(path) == 0)
	{stat = NC_EINVAL; goto done;}

    if(mapp) *mapp = NULL;

    switch (impl) {
    case NCZM_NC4:
        stat = zmap_nz4.open(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
    case NCZM_FILE:
        stat = zmap_nzf.open(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#ifdef ENABLE_S3_SDK
    case NCZM_S3:
        stat = zmap_s3sdk.open(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#endif
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

/**************************************************/
/* API Wrapper */

int
nczmap_close(NCZMAP* map, int delete)
{
    int stat = NC_NOERR;
    if(map && map->api)
        stat = map->api->close(map,delete);
    return THROW(stat);
}

int
nczmap_exists(NCZMAP* map, const char* key)
{
    return map->api->exists(map, key);
}

int
nczmap_len(NCZMAP* map, const char* key, size64_t* lenp)
{
    return map->api->len(map, key, lenp);
}

int
nczmap_defineobj(NCZMAP* map, const char* key)
{
    return map->api->defineobj(map, key);
}

int
nczmap_read(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content)
{
    return map->api->read(map, key, start, count, content);
}

int
nczmap_write(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content)
{
    return map->api->write(map, key, start, count, content);
}

int
nczmap_search(NCZMAP* map, const char* prefix, NClist* matches)
{
    return map->api->search(map, prefix, matches);
}

/**************************************************/
/* Utilities */

int
nczm_split(const char* path, NClist* segments)
{
    return nczm_split_delim(path,NCZM_SEP[0],segments);
}

int
nczm_split_delim(const char* path, char delim, NClist* segments)
{
    int stat = NC_NOERR;
    const char* p = NULL;
    const char* q = NULL;
    ptrdiff_t len = 0;
    char* seg = NULL;

    if(path == NULL || strlen(path)==0 || segments == NULL)
	{stat = NC_EINVAL; goto done;}

    p = path;
    if(p[0] == delim) p++;
    for(;*p;) {
	q = strchr(p,delim);
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
    nullfree(seg);
    return THROW(stat);
}

/* concat the the segments with each segment preceded by '/' */
int
nczm_join(NClist* segments, char** pathp)
{
    int stat = NC_NOERR;
    int i;
    NCbytes* buf = NULL;

    if(segments == NULL)
	{stat = NC_EINVAL; goto done;}
    if((buf = ncbytesnew())==NULL)
	{stat = NC_ENOMEM; goto done;}
    if(nclistlength(segments) == 0)
        ncbytescat(buf,"/");
    else for(i=0;i<nclistlength(segments);i++) {
	const char* seg = nclistget(segments,i);
	if(seg[0] != '/')
	    ncbytescat(buf,"/");
	ncbytescat(buf,seg);		
    }

done:
    if(!stat) {
	if(pathp) *pathp = ncbytesextract(buf);
    }
    ncbytesfree(buf);
    return THROW(stat);
}

int
nczm_concat(const char* prefix, const char* suffix, char** pathp)
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

/* A segment is defined as a '/' plus characters following up
   to the end or upto the next '/'
*/
int
nczm_divide_at(const char* key, int nsegs, char** prefixp, char** suffixp)
{
    int stat = NC_NOERR;
    char* prefix = NULL;
    char* suffix = NULL;
    size_t len, i;
    ptrdiff_t delta;
    const char* p;
    int abssegs = (nsegs >= 0 ?nsegs: -nsegs);
    int presegs = 0;
 
    /* Special case */
    if(key == NULL || strlen(key) == 0) goto done;

    p = (key[0] == '/' ? key+1 : key);
    /* Count number of segments */
    for(len=0;;) {
        const char* q = strchr(p,'/');    
	len++;
	if(q == NULL) break;
	p = q+1; /* start past leading '/' of next segment */
    }
    if(abssegs > len)
	{stat = NC_EINVAL; goto done;}
    /* find split point */
    if(nsegs >= 0)
	{presegs = abssegs;}
    else
	{presegs = (len - abssegs);}

    /* skip past the first presegs segments */
    for(p=key,i=0;i<presegs;i++) {
        const char* q = strchr(p+1,'/'); 
	if(q == NULL) {p = (p + strlen(p)); break;}
	else p = q;
    }
    /* p should point at the presegs+1 start point */
    delta = (p-key);    
    if(prefixp) {
        prefix = malloc(delta+1);
        memcpy(prefix,key,delta);
        prefix[delta] = '\0';
        *prefixp = prefix;
    } 
    if(suffixp) {
        suffix = strdup(p);
        *suffixp = suffix;
    }
done:
    return stat;
}

int
nczm_clear(NCZMAP* map)
{
    if(map) 
	nullfree(map->url);
    return NC_NOERR;
}

int
nczm_isabsolutepath(const char* path)
{
    if(path == NULL) return 0;
    switch (path[0]) {
    case '\\': return 1;
    case '/': return 1;
    case '\0': break;
    default:
	/* Check for windows drive letter */
	if(NChasdriveletter(path)) return 1;
        break;
    }
    return 0;
}

/* Convert forward slash to backslash ( !localize) or vice-versa (localize)*/
int
nczm_localize(const char* path, char** localpathp, int localize)
{
    int stat = NC_NOERR;
    char* localpath = NULL;
    char* p;
    int forward = 1;
    int offset = 0;

#ifdef _MSC_VER
    forward = (localize?0:1);
#endif
    /* If path comes from a url, then it may start with: /x:/...
       where x is a drive letter. If so, then remove leading / */
    if(path[0] == '/' && NChasdriveletter(path+1))
	offset = 1;
    if((localpath = strdup(path+offset))==NULL) return NC_ENOMEM;

    for(p=localpath;*p;p++) {
	if(forward && *p == '\\') *p = '/';
	else if(!forward && *p == '/') *p = '\\';
    }
    if(localpathp) {*localpathp = localpath; localpath = NULL;}
    nullfree(localpath);
    return stat;
}

/* Convert path0 to be:
1. absolute -- including drive letters
2. forward slashed -- we will convert back to back slash in
   nczm_fixpath
*/

int
nczm_canonicalpath(const char* path, char** cpathp)
{
    int ret = NC_NOERR;
    char* cpath = NULL;
    char* tmp = NULL;

    if(path == NULL) 
	{cpath = NULL; goto done;}

    /* Process path to make it be windows compatible */
    if((tmp = NCpathcvt(path))==NULL) {ret = NC_ENOMEM; goto done;}

    /* Fix slashes to be forward for now */
    if((ret = nczm_localize(tmp,&cpath,!LOCALIZE))) goto done;

    if(cpathp) {*cpathp = cpath; cpath = NULL;}
done:
    nullfree(tmp);
    nullfree(cpath);
    return THROW(ret);    
}
