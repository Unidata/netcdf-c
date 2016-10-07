/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"

#define LBRACKET '['
#define RBRACKET ']'

/**************************************************/
/* Forward */

static char* backslashEscape(const char* s);
static char* deEscape(const char* esc);

/**************************************************/
/**
 * Provide a hidden interface to allow utilities
 * to check if a given path name is really an ncdap4 url.
 * If no, return null, else return basename of the url
 * minus any extension.
 */

int
ncd4__testurl(const char* path, char** basenamep)
{
    NCURI* uri;
    int ok = ncuriparse(path,&uri);
    if(ok) {
	char* slash = (uri->file == NULL ? NULL : strrchr(uri->file, '/'));
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
    else  {
      if(slash)
        free(slash);
    }
    ncurifree(uri);
    }
    return ok;
}

/* Return 1 if this machine is big endian */
int
NCD4_isBigEndian(void)
{
    union {
        unsigned char bytes[SIZEOF_INT];
	int i;
    } u;
    u.i = 1;
    return (u.bytes[0] == 1 ? 0 : 1);
}

/* Compute the size of an atomic type, except opaque */
int
NCD4_typesize(nc_type tid)
{
    switch(tid) {
    case NC_BYTE: case NC_UBYTE: case NC_CHAR: return 1;
    case NC_SHORT: case NC_USHORT: return sizeof(short);
    case NC_INT: case NC_UINT: return sizeof(int);
    case NC_FLOAT: return  sizeof(float);
    case NC_DOUBLE: return  sizeof(double);
    case NC_INT64: case NC_UINT64: return sizeof(long long);
    case NC_STRING: return sizeof(char*);
    default: break;
    }
    return 0;
}

d4size_t
NCD4_dimproduct(NCD4node* node)
{
    int i;
    d4size_t product = 1;
    for(i=0;i<nclistlength(node->dims);i++) {
        NCD4node* dim = (NCD4node*)nclistget(node->dims,i);
        product *= dim->dim.size;
    }
    return product;
}

char*
NCD4_makeFQN(NCD4node* node)
{
    char* fqn = NULL;
    int i;
    NCD4node* g = node;
    NClist* path = nclistnew();
    size_t estimate;

    for(estimate=0;g != NULL;g=g->container) {
	estimate += strlen(g->name);		
	nclistinsert(path,0,g);
    }
    estimate = (estimate*2) + 2*nclistlength(path);
    /* start at 1 to avoid dataset */
    fqn = (char*)malloc(estimate+1);
    if(fqn == NULL) goto done;
    fqn[0] = '\0';
    /* Create the group-based fqn prefix */
    for(i=1;i<nclistlength(path);i++) {
	NCD4node* elem = (NCD4node*)nclistget(path,i);
	if(elem->sort != NCD4_GROUP) break;
	/* Add in the group name */
	char* escaped = backslashEscape(elem->name);
	if(escaped == NULL) {free(fqn); fqn = NULL; goto done;}
	strcat(fqn,"/");
	strcat(fqn,escaped);
	free(escaped);
    }
    /* Add in the final name part (if not group) */
    if(i < nclistlength(path)) {
	int last = nclistlength(path)-1;
	NCD4node* n = (NCD4node*)nclistget(path,last);
	char* name = NCD4_makeName(n,".");
	strcat(fqn,"/");	
	strcat(fqn,name);
	nullfree(name);
    }

done:
    nclistfree(path);
    return fqn;
}

/*
create the last part of the fqn
*/
char*
NCD4_makeName(NCD4node* elem, const char* sep)
{
    int i;
    size_t estimate = 0;
    NCD4node* n;
    NClist* path = nclistnew();
    char* fqn = NULL;

    /* Collect the path upto, but not including, the first containing group */
    for(estimate=0,n=elem;n->sort != NCD4_GROUP;n=n->container) {
	nclistinsert(path,0,n);
	estimate += (1+(2*strlen(n->name)));
    }

    fqn = (char*)malloc(estimate+1);
    if(fqn == NULL) goto done;    
    fqn[0] = '\0';

    for(i=0;i<nclistlength(path);i++) {
	NCD4node* elem = (NCD4node*)nclistget(path,i);
	char* escaped = backslashEscape(elem->name);
	if(escaped == NULL) {free(fqn); fqn = NULL; goto done;}
	if(i > 0)
	    strcat(fqn,sep);
	strcat(fqn,escaped);
	free(escaped);
    }
done:
    nclistfree(path);
    return fqn;
}

static char*
backslashEscape(const char* s)
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

int
NCD4_parseFQN(const char* fqn0, NClist* pieces)
{
    int ret = NC_NOERR;
    char* p;
    char* start;
    char* lastpiece;
    char* fqn = NULL;
    int count;

    if(fqn0 == NULL) fqn0 = "/";
    nclistpush(pieces,strdup("/")); /* set root */
    count = 1;
    fqn = strdup(fqn0);
    start = (fqn[0] == '/' ? fqn+1 : fqn);
    /* Step 1 Break fqn into pieces at occurrences of '/' */
    for(p=start,lastpiece=p;*p;) {
	switch(*p) {
	case '\\': /* leave the escapes in place */
	    p+=2;
	    break;
	case '/': /*capture the piece name */
	    *p++ = '\0';	    
	    count++;
	    lastpiece = p;
	    break;
	default: /* ordinary char */
	    p++;
	    break;
	}
    }
    /* Step 2, walk the final piece to break up based on '.' */
    for(p=lastpiece;*p;) {
	switch(*p) {
	case '\\': /* leave the escapes in place */
	    p+=2;
	    break;
	case '.': /*capture the piece name */
	    *p++ = '\0';	    
	    count++;
	    break;
	default: /* ordinary char */
	    p++;
	    break;
	}
    }
    /* Step 3: capture and de-scape the pieces */
    for(p=start;count > 0;count--) {
	size_t len = strlen(p);
	char* descaped = deEscape(p);
	nclistpush(pieces,descaped);
	p += (len+1);
    }        
    if(fqn != NULL) free(fqn);
    return THROW(ret);
}

static char*
deEscape(const char* esc)
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

/**************************************************/
/* Error reporting */

int
NCD4_error(int code, const int line, const char* file, const char* fmt, ...)
{
    fprintf(stderr,"(%s:%d) ",file,line);
    va_list argv;
    va_start(argv,fmt);
    vfprintf(stderr,fmt,argv);
    fprintf(stderr,"\n");
    fflush(stderr);
    return code;
}

int
NCD4_errorNC(int code, const int line, const char* file)
{
    return NCD4_error(code,line,file,nc_strerror(code));
}

