/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef _MSC_VER
#include <io.h>
#endif
#include "netcdf.h"
#include "ncuri.h"
#include "ncbytes.h"
#include "nclist.h"
#include "nclog.h"
#include "ncrc.h"
#include "ncpathmgr.h"

#define NC_MAX_PATH 4096

#define LBRACKET '['
#define RBRACKET ']'

#define AWSHOST ".amazonaws.com"

enum URLFORMAT {UF_NONE=0, UF_VIRTUAL=1, UF_PATH=2, UF_S3=3, UF_OTHER=4};

/* Forward */
static int endswith(const char* s, const char* suffix);

/**************************************************/
/**
 * Provide a hidden interface to allow utilities
 * to check if a given path name is really an ncdap4 url.
 * If no, return null, else return basename of the url
 * minus any extension.
 */

int
NC__testurl(const char* path, char** basenamep)
{
    NCURI* uri;
    int ok = NC_NOERR;
    if(ncuriparse(path,&uri))
	ok = NC_EURL;
    else {
	char* slash = (uri->path == NULL ? NULL : strrchr(uri->path, '/'));
	char* dot;
	if(slash == NULL) slash = (char*)path; else slash++;
        slash = nulldup(slash);
        if(slash == NULL)
            dot = NULL;
        else
            dot = strrchr(slash, '.');
        if(dot != NULL &&  dot != slash) *dot = '\0';
        if(basenamep)
            *basenamep=slash;
        else if(slash)
            free(slash);
    }
    ncurifree(uri);
    return ok;
}

/* Return 1 if this machine is little endian */
int
NC_isLittleEndian(void)
{
    union {
        unsigned char bytes[SIZEOF_INT];
	int i;
    } u;
    u.i = 1;
    return (u.bytes[0] == 1 ? 1 : 0);
}

char*
NC_backslashEscape(const char* s)
{
    const char* p;
    char* q;
    size_t len;
    char* escaped = NULL;

    len = strlen(s);
    escaped = (char*)malloc(1+(2*len)); /* max is everychar is escaped */
    if(escaped == NULL) return NULL;
    for(p=s,q=escaped;*p;p++) {
        char c = *p;
        switch (c) {
	case '\\':
	case '/':
	case '.':
	case '@':
	    *q++ = '\\'; *q++ = '\\';
	    break;
	default: *q++ = c; break;
        }
    }
    *q = '\0';
    return escaped;
}

char*
NC_backslashUnescape(const char* esc)
{
    size_t len;
    char* s;
    const char* p;
    char* q;

    if(esc == NULL) return NULL;
    len = strlen(esc);
    s = (char*)malloc(len+1);
    if(s == NULL) return NULL;
    for(p=esc,q=s;*p;) {
	switch (*p) {
	case '\\':
	     p++;
	     /* fall thru */
	default: *q++ = *p++; break;
	}
    }
    *q = '\0';
    return s;
}

char*
NC_entityescape(const char* s)
{
    const char* p;
    char* q;
    size_t len;
    char* escaped = NULL;
    const char* entity;

    len = strlen(s);
    escaped = (char*)malloc(1+(6*len)); /* 6 = |&apos;| */
    if(escaped == NULL) return NULL;
    for(p=s,q=escaped;*p;p++) {
	char c = *p;
	switch (c) {
	case '&':  entity = "&amp;"; break;
	case '<':  entity = "&lt;"; break;
	case '>':  entity = "&gt;"; break;
	case '"':  entity = "&quot;"; break;
	case '\'': entity = "&apos;"; break;
	default	 : entity = NULL; break;
	}
	if(entity == NULL)
	    *q++ = c;
	else {
	    len = strlen(entity);
	    memcpy(q,entity,len);
	    q+=len;
	}
    }
    *q = '\0';
    return escaped;
}

char*
/*
Depending on the platform, the shell will sometimes
pass an escaped octotherpe character without removing
the backslash. So this function is appropriate to be called
on possible url paths to unescape such cases. See e.g. ncgen.
*/
NC_shellUnescape(const char* esc)
{
    size_t len;
    char* s;
    const char* p;
    char* q;

    if(esc == NULL) return NULL;
    len = strlen(esc);
    s = (char*)malloc(len+1);
    if(s == NULL) return NULL;
    for(p=esc,q=s;*p;) {
	switch (*p) {
	case '\\':
	     if(p[1] == '#')
	         p++;
	     /* fall thru */
	default: *q++ = *p++; break;
	}
    }
    *q = '\0';
    return s;
}

/**
Wrap mktmp and return the generated path,
or null if failed.
Base is the base file path. XXXXX is appended
to allow mktmp add its unique id.
Return the generated path.
*/

char*
NC_mktmp(const char* base)
{
    int fd = -1;
    char* tmp = NULL;
    size_t len;

    len = strlen(base)+6+1;
    if((tmp = (char*)malloc(len))==NULL)
        goto done;
    strncpy(tmp,base,len);
    strlcat(tmp, "XXXXXX", len);
    fd = NCmkstemp(tmp);
    if(fd < 0) {
       nclog(NCLOGERR, "Could not create temp file: %s",tmp);
       goto done;
    }
done:
    if(fd >= 0) close(fd);
    return tmp;
}

int
NC_readfile(const char* filename, NCbytes* content)
{
    int ret = NC_NOERR;
    FILE* stream = NULL;
    char part[1024];

#ifdef _WIN32
    stream = NCfopen(filename,"rb");
#else
    stream = NCfopen(filename,"r");
#endif
    if(stream == NULL) {ret=errno; goto done;}
    for(;;) {
	size_t count = fread(part, 1, sizeof(part), stream);
	if(count <= 0) break;
	ncbytesappendn(content,part,count);
	if(ferror(stream)) {ret = NC_EIO; goto done;}
	if(feof(stream)) break;
    }
    ncbytesnull(content);
done:
    if(stream) fclose(stream);
    return ret;
}

int
NC_writefile(const char* filename, size_t size, void* content)
{
    int ret = NC_NOERR;
    FILE* stream = NULL;
    void* p;
    size_t remain;

#ifdef _WIN32
    stream = NCfopen(filename,"wb");
#else
    stream = NCfopen(filename,"w");
#endif
    if(stream == NULL) {ret=errno; goto done;}
    p = content;
    remain = size;
    while(remain > 0) {
	size_t written = fwrite(p, 1, remain, stream);
	if(ferror(stream)) {ret = NC_EIO; goto done;}
	if(feof(stream)) break;
	remain -= written;
    }
done:
    if(stream) fclose(stream);
    return ret;
}

/*
Parse a path as a url and extract the modelist.
If the path is not a URL, then return a NULL list.
If a URL, but modelist is empty or does not exist,
then return empty list.
*/
int
NC_getmodelist(const char* path, NClist** modelistp)
{
    int stat=NC_NOERR;
    NClist* modelist = NULL;
    NCURI* uri = NULL;
    const char* modestr = NULL;
    const char* p = NULL;
    const char* endp = NULL;

    ncuriparse(path,&uri);
    if(uri == NULL) goto done; /* not a uri */

    /* Get the mode= arg from the fragment */
    modelist = nclistnew();    
    modestr = ncurifragmentlookup(uri,"mode");
    if(modestr == NULL || strlen(modestr) == 0) goto done;
    /* Parse the mode string at the commas or EOL */
    p = modestr;
    for(;;) {
	char* s;
	ptrdiff_t slen;
	endp = strchr(p,',');
	if(endp == NULL) endp = p + strlen(p);
	slen = (endp - p);
	if((s = malloc(slen+1)) == NULL) {stat = NC_ENOMEM; goto done;}
	memcpy(s,p,slen);
	s[slen] = '\0';
	nclistpush(modelist,s);
	if(*endp == '\0') break;
	p = endp+1;
    }

done:
    if(stat == NC_NOERR) {
	if(modelistp) {*modelistp = modelist; modelist = NULL;}
    }
    ncurifree(uri);
    nclistfree(modelist);
    return stat;
}

/*
Check "mode=" list for a path and return 1 if present, 0 otherwise.
*/
int
NC_testmode(const char* path, const char* tag)
{
    int stat = NC_NOERR;
    int found = 0;
    int i;
    NClist* modelist = NULL;

    if((stat = NC_getmodelist(path, &modelist))) goto done;
    for(i=0;i<nclistlength(modelist);i++) {
	const char* value = nclistget(modelist,i);
	if(strcasecmp(tag,value)==0) {found = 1; break;}
    }        
    
done:
    nclistfreeall(modelist);
    return found;
}

#ifdef __APPLE__
int isinf(double x)
{
    union { unsigned long long u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) == 0x7ff00000 &&
           ( (unsigned)ieee754.u == 0 );
}

int isnan(double x)
{
    union { unsigned long long u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) +
           ( (unsigned)ieee754.u != 0 ) > 0x7ff00000;
}

#endif /*APPLE*/


/**************************************************/
/* Generic S3 Utilities */

/*
Rebuild an S3 url into a canonical path-style url.
If region is not in the host, then use specified region
if provided, otherwise us-east-1.
@param url (in) the current url
@param region	(in) region to use if needed; NULL => us-east-1
		(out) region from url or the input region
@param pathurlp (out) the resulting pathified url string
@param bucketp (out) the bucket from the url
*/

int
NC_s3urlrebuild(NCURI* url, NCURI** newurlp, char** bucketp, char** outregionp)
{
    int i,stat = NC_NOERR;
    NClist* hostsegments = NULL;
    NClist* pathsegments = NULL;
    NCbytes* buf = ncbytesnew();
    NCURI* newurl = NULL;
    char* bucket = NULL;
    char* host = NULL;
    char* path = NULL;
    char* region = NULL;
    
    if(url == NULL)
        {stat = NC_EURL; goto done;}

    /* Parse the hostname */
    hostsegments = nclistnew();
    /* split the hostname by "." */
    if((stat = NC_split_delim(url->host,'.',hostsegments))) goto done;

    /* Parse the path*/
    pathsegments = nclistnew();
    /* split the path by "/" */
    if((stat = NC_split_delim(url->path,'/',pathsegments))) goto done;

    /* Distinguish path-style from virtual-host style from s3: and from other.
       Virtual: https://bucket-name.s3.Region.amazonaws.com/<path>
       Path: https://s3.Region.amazonaws.com/bucket-name/<path>
       S3: s3://bucket-name/<path>
       Other: https://<host>/bucketname/<path>
    */
    if(url->host == NULL || strlen(url->host) == 0)
        {stat = NC_EURL; goto done;}
    if(strcmp(url->protocol,"s3")==0 && nclistlength(hostsegments)==1) {
	bucket = strdup(url->host);
	region = NULL; /* unknown at this point */
    } else if(endswith(url->host,AWSHOST)) { /* Virtual or path */
	switch (nclistlength(hostsegments)) {
	default: stat = NC_EURL; goto done;
	case 4:
            if(strcasecmp(nclistget(hostsegments,0),"s3")!=0)
	        {stat = NC_EURL; goto done;}
	    region = strdup(nclistget(hostsegments,1));
	    if(nclistlength(pathsegments) > 0)
	        bucket = nclistremove(pathsegments,0);
	    break;
	case 5:
            if(strcasecmp(nclistget(hostsegments,1),"s3")!=0)
	        {stat = NC_EURL; goto done;}
	    region = strdup(nclistget(hostsegments,2));
    	    bucket = strdup(nclistget(hostsegments,0));
	    break;
	}
    } else {
        if((host = strdup(url->host))==NULL)
	    {stat = NC_ENOMEM; goto done;}
        /* region is unknown */
	region = NULL;
	/* bucket is assumed to be start of the path */
	if(nclistlength(pathsegments) > 0)
	    bucket = nclistremove(pathsegments,0);
    }
    /* If region is null, use default */
    if(region == NULL) {
        const char* region0 = NULL;
	/* Get default region */
	if((stat = NC_getdefaults3region(url,&region0))) goto done;
	region = strdup(region0);
    }
    /* Construct the revised host */
    ncbytescat(buf,"s3.");
    ncbytescat(buf,region);
    ncbytescat(buf,AWSHOST);
    host = ncbytesextract(buf);

    /* Construct the revised path */
    ncbytesclear(buf);
    ncbytescat(buf,"/");
    if(bucket == NULL)
        {stat = NC_EURL; goto done;}
    ncbytescat(buf,bucket);
    for(i=0;i<nclistlength(pathsegments);i++) {
	ncbytescat(buf,"/");
	ncbytescat(buf,nclistget(pathsegments,i));
    }
    path = ncbytesextract(buf);
    /* complete the new url */
    if((newurl=ncuriclone(url))==NULL) {stat = NC_ENOMEM; goto done;}
    ncurisethost(newurl,host);
    ncurisetpath(newurl,path);
    /* return various items */
    if(newurlp) {*newurlp = newurl; newurl = NULL;}
    if(bucketp) {*bucketp = bucket; bucket = NULL;}
    if(outregionp) {*outregionp = region; region = NULL;}

done:
    nullfree(region);
    nullfree(bucket)
    nullfree(host)
    nullfree(path)
    ncurifree(newurl);
    ncbytesfree(buf);
    nclistfreeall(hostsegments);
    nclistfreeall(pathsegments);
    return stat;
}

static int
endswith(const char* s, const char* suffix)
{
    ssize_t ls, lsf, delta;
    if(s == NULL || suffix == NULL) return 0;
    ls = strlen(s);
    lsf = strlen(suffix);
    delta = (ls - lsf);
    if(delta < 0) return 0;
    if(memcmp(s+delta,suffix,lsf)!=0) return 0;
    return 1;
}

int
NC_split_delim(const char* path, char delim, NClist* segments)
{
    int stat = NC_NOERR;
    const char* p = NULL;
    const char* q = NULL;
    ptrdiff_t len = 0;
    char* seg = NULL;

    if(path == NULL || strlen(path)==0 || segments == NULL)
        goto done;
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
    return stat;
}

