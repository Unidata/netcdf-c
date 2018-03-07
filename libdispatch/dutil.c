/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
#include "nclog.h"
#include "ncwinpath.h"

#define NC_MAX_PATH 4096

#define LBRACKET '['
#define RBRACKET ']'

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
    if(ncuriparse(path,&uri) != NCU_OK)
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

int
NC_readfile(const char* filename, NCbytes* content)
{
    int ret = NC_NOERR;
    FILE* stream = NULL;
    char part[1024];

#ifdef _MSC_VER
    stream = NCfopen(filename,"r");
#else
    stream = NCfopen(filename,"rb");
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
    int fd;
    char* cvtpath = NULL;
    char tmp[NC_MAX_PATH];
#ifdef HAVE_MKSTEMP
    mode_t mask;
#endif

    /* Make sure that this path conversion has been applied */
    cvtpath = NCpathcvt(base);
    strncpy(tmp,cvtpath,sizeof(tmp));
    nullfree(cvtpath);
	strncat(tmp, "XXXXXX", sizeof(tmp) - strlen(tmp) - 1);

#ifdef HAVE_MKSTEMP
    /* Note Potential problem: old versions of this function
       leave the file in mode 0666 instead of 0600 */
    mask=umask(0077);
    fd = mkstemp(tmp);
    (void)umask(mask);
#else /* !HAVE_MKSTEMP */
    {
#ifdef HAVE_MKTEMP
#ifdef _MSC_VER
        /* Use _mktemp_s */
	_mktemp_s(tmp,sizeof(tmp)-1);
#else /*!_MSC_VER*/
        mktemp(tmp);
	tmo[sizeof[tmp]-1] = '\0';
#endif
#else /* !HAVE_MKTEMP */
	/* Need to simulate by using some kind of pseudo-random number */
	{
	    int rno = rand();
	    char spid[7];
	    if(rno < 0) rno = -rno;
            snprintf(spid,sizeof(spid),"%06d",rno);
            strncat(tmp,spid,sizeof(tmp));
	}
#endif /* HAVE_MKTEMP */
#ifdef _MSC_VER
        fd=NCopen3(tmp,O_RDWR|O_BINARY|O_CREAT, _S_IREAD|_S_IWRITE);
#else
        fd=NCopen3(tmp,O_RDWR|O_CREAT|O_EXCL, S_IRWXU);
#endif
    }
#endif /* !HAVE_MKSTEMP */
    if(fd < 0) {
       nclog(NCLOGERR, "Could not create temp file: %s",tmp);
       return NULL;
    } else
	close(fd);
    return strdup(tmp);
}

