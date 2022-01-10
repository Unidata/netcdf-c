/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "zincludes.h"
#include <stdarg.h>
#include "ncpathmgr.h"

/**************************************************/
/* Import the current implementations */


/**************************************************/

NCZM_FEATURES
nczmap_features(NCZM_IMPL impl)
{
    switch (impl) {
    case NCZM_FILE: return zmap_file.features;
#ifdef ENABLE_NCZARR_ZIP
    case NCZM_ZIP: return zmap_zip.features;
#endif
#ifdef ENABLE_S3_SDK
    case NCZM_S3: return zmap_s3sdk.features;
#endif
    default: break;
    }
    return NCZM_UNIMPLEMENTED;
}

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
    case NCZM_FILE:
        stat = zmap_file.create(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#ifdef ENABLE_NCZARR_ZIP
    case NCZM_ZIP:
        stat = zmap_zip.create(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#endif
#ifdef ENABLE_S3_SDK
    case NCZM_S3:
        stat = zmap_s3sdk.create(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#endif
    default:
	{stat = REPORT(NC_ENOTBUILT,"nczmap_create"); goto done;}
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
    case NCZM_FILE:
        stat = zmap_file.open(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#ifdef ENABLE_NCZARR_ZIP
    case NCZM_ZIP:
        stat = zmap_zip.open(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#endif
#ifdef ENABLE_S3_SDK
    case NCZM_S3:
        stat = zmap_s3sdk.open(path, mode, flags, parameters, &map);
	if(stat) goto done;
	break;
#endif
    default:
	{stat = REPORT(NC_ENOTBUILT,"nczmap_open"); goto done;}
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
nczmap_read(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content)
{
    return map->api->read(map, key, start, count, content);
}

int
nczmap_write(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content)
{
    return map->api->write(map, key, start, count, content);
}

/* Define a static qsort comparator for strings for use with qsort */
static int
cmp_strings(const void* a1, const void* a2)
{
    const char** s1 = (const char**)a1;
    const char** s2 = (const char**)a2;
    return strcmp(*s1,*s2);
}

int
nczmap_search(NCZMAP* map, const char* prefix, NClist* matches)
{
    int stat = NC_NOERR;
    if((stat = map->api->search(map, prefix, matches)) == NC_NOERR) {
        /* sort the list */
        if(nclistlength(matches) > 1) {
	    void* base = nclistcontents(matches);
            qsort(base, nclistlength(matches), sizeof(char*), cmp_strings);
	}
    }
    return stat;
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
    return NC_split_delim(path,delim,segments);
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

/* Concat multiple strings, but with no intervening separators */
int
nczm_appendn(char** resultp, int n, ...)
{
    va_list args;
    NCbytes* buf = ncbytesnew();
    int i;

    va_start(args, n);
    for(i=0;i<n;i++) {
	char* s = va_arg(args,char*);
	if(s != NULL) ncbytescat(buf,s);
    }
    ncbytesnull(buf);
    va_end(args);
    if(resultp) {*resultp = ncbytesextract(buf);}
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
    static const char* windrive = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

#ifdef _MSC_VER
    forward = (localize?0:1);
#endif
    /* If path comes from a url, then it may start with: /x:/...
       where x is a drive letter. If so, then remove leading / */
    if(strlen(path) >= 4
       && path[0] == '/' && strchr(windrive,path[1]) != NULL
       && path[2] == ':' && path[3] == '/')
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
2. forward slashed -- we will convert back to back slash in nczm_fixpath
*/

int
nczm_canonicalpath(const char* path, char** cpathp)
{
    int ret = NC_NOERR;
    char* cpath = NULL;
    char* tmp1 = NULL;

    if(path == NULL) 
	{cpath = NULL; goto done;}

    /* Process path to make it be absolute*/
    if((tmp1 = NCpathabsolute(path))==NULL) {ret = NC_ENOMEM; goto done;}

    /* Fix slashes to be forward for now */
    if((ret = nczm_localize(tmp1,&cpath,!LOCALIZE))) goto done;

    if(cpathp) {*cpathp = cpath; cpath = NULL;}
done:
    nullfree(tmp1);
    nullfree(cpath);
    return THROW(ret);    
}

/* extract the first segment of a path */
int
nczm_segment1(const char* path, char** seg1p)
{
    int ret = NC_NOERR;
    char* seg1 = NULL;
    const char* p = NULL;
    const char* q = NULL;
    ptrdiff_t delta;

    if(path == NULL) 
	{seg1 = NULL; goto done;}

    p = path;
    if(*p == '/') p++; /* skip any leading '/' */
    q = strchr(p,'/');
    if(q == NULL) q = p+strlen(p); /* point to stop character */
    delta = (q-p);
    if((seg1 = (char*)malloc(delta+1))==NULL)
        {ret = NC_ENOMEM; goto done;}
    memcpy(seg1,p,delta);
    seg1[delta] = '\0';

    if(seg1p) {*seg1p = seg1; seg1 = NULL;}
done:
    nullfree(seg1);
    return THROW(ret);    
}

/*
Extract the last segment from path.
*/

int
nczm_lastsegment(const char* path, char** lastp)
{
    int ret = NC_NOERR;
    const char* last = NULL;

    if(path == NULL)
	{if(lastp) *lastp = NULL; goto done;}

    last = strrchr(path,'/');
    if(last == NULL) last = path; else last++;

    if(lastp) *lastp = strdup(last);

done:
    return THROW(ret);    
}

/*
Extract the basename from a path.
Basename is last segment minus one extension.
*/

int
nczm_basename(const char* path, char** basep)
{
    int stat = NC_NOERR;
    char* base = NULL;
    char* last = NULL;
    const char* p = NULL;
    ptrdiff_t delta;

    if((stat=nczm_lastsegment(path,&last))) goto done;

    if(last == NULL) goto done;
    p = strrchr(last,'.');
    if(p == NULL) p = last+strlen(last);
    delta = (p - last);
    if((base = (char*)malloc(delta+1))==NULL)
        {stat = NC_ENOMEM; goto done;}
    memcpy(base,last,delta);
    base[delta] = '\0';
    if(basep) {*basep = base; base = NULL;}
done:
    nullfree(last);
    nullfree(base);
    return THROW(stat);    
}

/* bubble sort a list of strings */
void
nczm_sortlist(NClist* l)
{
    nczm_sortenvv(nclistlength(l),(char**)nclistcontents(l));
}

/* bubble sort a list of strings */
void
nczm_sortenvv(int n, char** envv)
{
    size_t i, switched;

    if(n <= 1) return;
    do {
	switched = 0;
        for(i=0;i<n-1;i++) {
	    char* ith = envv[i];
	    char* ith1 = envv[i+1];
	    if(strcmp(ith,ith1) > 0) {
	        envv[i] = ith1;
    	        envv[i+1] = ith;
	        switched = 1;
	    }
	}
    } while(switched);
#if 0
for(i=0;i<n;i++)
fprintf(stderr,"sorted: [%d] %s\n",i,(const char*)envv[i]);
#endif
}

void
NCZ_freeenvv(int n, char** envv)
{
   int i;
   char** p;
   if(envv == NULL) return;
   if(n < 0)
       {for(n=0, p = envv; *p; n++); /* count number of strings */}
    for(i=0;i<n;i++)
        if(envv[i]) free(envv[i]);
    free(envv);    
}
