/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "ncuri.h"
#include "ncbytes.h"
#include "nclist.h"

/* Include netcdf.h to allow access to
   NC_ error return codes. */
#include "netcdf.h"

#define NCURIDEBUG

#ifdef NCURIDEBUG
static int failpoint = 0;
#define THROW(n) {failpoint=(n); goto fail;}
#else
#define THROW(n) {goto fail;}
#endif


#define PADDING 8

#define LBRACKET '['
#define RBRACKET ']'
#define EOFCHAR '\0'

#ifndef FIX
#define FIX(s) ((s)==NULL?"NULL":(s))
#endif

#ifndef NILLEN
#define NILLEN(s) ((s)==NULL?0:strlen(s))
#endif

#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif

#define terminate(p) {*(p) = EOFCHAR;}

#define endof(p) ((p)+strlen(p))

/* Allowable character sets for encode */
static char* pathallow =
"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$&'()*+,-./:;=?@_~";

static char* queryallow =
"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$&'()*+,-./:;=?@_~";

#ifndef HAVE_STRNCMP
#define strndup ncstrndup
/* Not all systems have strndup, so provide one*/
char*
ncstrndup(const char* s, size_t len)
{
    char* dup;
    if(s == NULL) return NULL;
    dup = (char*)malloc(len+1);
    if(dup == NULL) return NULL;
    memcpy((void*)dup,s,len);
    dup[len] = '\0';
    return dup;
}
#endif
/* Forward */
static int ncfind(char** params, const char* key);
static char* nclocate(char* p, const char* charlist);
static int parselist(const char* text, char lparen, char rparen, NClist*, char** nextp);

/**************************************************/
/*
A note about parameter support:
In the original url format for opendap (dap2), client parameters were
assumed to be one or more instances of bracketed pairs: e.g
    "[...][...]...".
These were assumed to be placed at the front of the url.  In this newer
version, the parameters may be encoded after a trailing # character each
separated by ampersand (&).  For back compatibility, the bracketed
parameter form is supported. However, if ncuribuild is used, all
parameters will be converted to the
    #...&...& format.
In any case, each parameter in turn is assumed to be a of the form
<name>=<value> or <name>; e.g. #x=y&z&a=b&w.
If the same parameter is specified more than once, then the first
occurrence is used; this is so that is possible to forcibly override
user specified parameters by prefixing.
IMPORTANT: the client parameter string is assumed to have blanks compressed out.
*/

/**************************************************/

/* Do a simple uri parse: return NCU_OK if success, NCU_XXX if failed */
int
ncuriparse(const char* uri0, NCURI** durip)
{
    NCURI* duri = NULL;
    NClist* params = NULL;
    char* p;
    char* q;
    int i,c;
    int isfile;
    int hashost;
    char* uri;

    if(uri0 == NULL || strlen(uri0) == 0)
	{THROW(NCU_EBADURL);}

    duri = (NCURI*)calloc(1,sizeof(NCURI));
    if(duri == NULL)
      {THROW(NCU_ENOMEM);}

    /* save original uri */
    duri->uri = strdup(uri0);

    /* make mutable copy */
    uri = strdup(uri0);

    /* Walk the uri and do the following:
	1. remove leading and trailing whitespace
	2. convert all '\\' -> '\' (Temp hack to remove escape characters
                                    inserted by Windows or MinGW)
    */
    for(q=uri,p=uri;*p;p++) {if((*p == '\\' && p[1] == '\\') || *p < ' ') {continue;} else {*q++ = *p;}}
    *q = '\0';

    p = uri;

    /* collect any prefix bracketed parameters */
    if(*p == LBRACKET) {
	char* next = NULL;
	if(params == NULL) params = nclistnew();
	if(!parselist(p,LBRACKET,RBRACKET,params,&next))
  	    {THROW(NCU_EBADURL);}
	p = next;
    }
    /* Tag the protocol */
    duri->protocol = p;
    p = strchr(p,':');
    if(!p)
	{THROW(NCU_EBADURL);}
    terminate(p); /*overwrite colon*/
    p++; /* skip the colon */
    if(strlen(duri->protocol)==0)
	{THROW(NCU_EBADURL);}
    duri->protocol = strdup(duri->protocol); /* save the protocol */

    /*
       The legal formats for file: urls are a problem since
       many variants are often accepted.
       By RFC, the proper general format is: file://host/path,
       where the 'host' can be omitted and defaults to 'localhost'.
       So, assuming no host, the format is: file:///path.
       Some implementations, however, ignore the host, and allow
       the format: file:/path.
       Note in all cases, the leading '/' is considered part of the path,
       which is then assumed to be an absolute path.
       The rules implemented here (for file:) are as follows:
       1. file:/X, where X does not start with a slash: treat /X as the path.
       2. file:///X, where X does not start with a slash: treat /X as the path.
       This explicitly disallows this case: file://X.
    */

    isfile = (strcmp(duri->protocol,"file")==0);
    if(isfile) {
	hashost = 0; /* assume */
        if(p[0] == '/' && p[1] == '/' && p[2] == '/') /* we have file:///...*/
	    p += 2; /* make it point to the start of the path */
        else if(p[0] == '/' && p[1] != '/') /* we have file:/...*/
	    p = p; /* leave p; it points to the start of the path */
	else if(p[0] == '/' && p[1] == '/' && p[2] != '/') { /* we have file://...*/
	    hashost = 1; /* Assume we have a hostname */
	    p += 2;
        } else /* everything else is illegal */
	    {THROW(NCU_EPATH);}
    } else {
        if(p[0] != '/' || p[1] != '/') /* must be proto:// */
	    {THROW(NCU_EPATH);}
	p += 2;
        hashost = 1; /* Assume we have a hostname */
    }
    if(!hashost)    
        duri->path = p;
    else { /* assume there should be a host section */
	char* q;
        /* locate the end of the host section and therefore the start
           of the path|query|fragment  */
	duri->host = p;
        p  = nclocate(p,"/?#");
	if(p == NULL) { /* rest of uri is empty */
	    duri->host = strdup(duri->host); /* rest of uri is all host */
	    duri->path = strdup("/"); /* default */
	} else {
	    /* Temporarily overwrite the char past the host+port */
	    int c = *p;
	    terminate(p);
	    duri->host = strdup(duri->host);	    
	    *p = c; /* restore the character */
	    duri->path = p;
	}
    }
    /* Nullify duri->host for consistency */
    if(duri->host != NULL && strlen(duri->host)==0) {nullfree(duri->host); duri->host = NULL;}

    if(duri->host != NULL) {/* Parse the host section */
        char* pp;
	/* Check for leading user:pwd@ */
        char* newhost = strchr(duri->host,'@');
        if(newhost != NULL) {
	    size_t rem;
	    if(newhost == duri->host)
		{THROW(NCU_EUSRPWD);} /* we have proto://@ */
	    terminate(newhost); /* overwrite '@' */
	    newhost++; /* should point past usr+pwd */
	    duri->user = duri->host;
	    /* Break user+pwd into two pieces */
	    pp = strchr(duri->user,':');
	    if(pp == NULL)
		{THROW(NCU_EUSRPWD);} /* we have user only */
	    terminate(pp); /* overwrite ':' */
	    pp++;
	    if(strlen(duri->user)==0)
		{THROW(NCU_EUSRPWD);} /* we have empty user */
	    if(strlen(pp)==0)
		{THROW(NCU_EUSRPWD);} /* we have empty password */
	    duri->user = strdup(duri->user);
	    duri->password = strdup(pp);	    
	    /* compress usr+pwd out of duri->host */
	    rem = strlen(newhost);
	    memmove(duri->host,newhost,rem);
	    duri->host[rem] = EOFCHAR;
	}
	/* Breakup host into host + port */
	pp = duri->host;
        pp = strchr(pp,':');
        if(pp != NULL) { /* there is a port */
	    terminate(pp); /* overwrite ':' */
	    pp++; /* skip colon */
	    if(strlen(duri->host) == 0)
		{THROW(NCU_EBADURL);} /* empty host */
	    if(strlen(pp)==0)
		{THROW(NCU_EBADURL);} /* empty port */
	    duri->port = strdup(pp);
	    /* The port must look something like a number */
	    for(pp=duri->port;*pp;pp++) {
	        if(strchr("0123456789-",*pp) == NULL)
		    {THROW(NCU_EPORT);}  /* probably not a real port, fail */
	    }
	} /* else no port */
    }

    /* find the end of the file section and the start of the
       constraints and/or suffixparams
    */
    p = duri->path;
    p = nclocate(p,"?#");
    if(p == NULL) {
	p = duri->path;
	p = endof(p);
    }
    c = *p;
    *p = EOFCHAR;
    if(c == '?') {
	char* q;
	p++;
	duri->query = p; /* temp */
        q = nclocate(p,"#");
	if(q == NULL) q = p + strlen(p); /* make point to end of the query */
	p = q;
	c = *p;
    }
    if(c == '#') {
	*p = EOFCHAR;
	p++;
	duri->fragment = p; /* temp */
    }
    if(duri->path != NULL)
        duri->path = (strlen(duri->path) > 0 ? strdup(duri->path) : NULL);
    if(duri->query != NULL)
	duri->query = (strlen(duri->query) > 0 ? strdup(duri->query) : NULL);
    if(duri->fragment != NULL)
        duri->fragment = (strlen(duri->fragment) > 0 ? strdup(duri->fragment) : NULL);

    /* Parse the query if exists */
    if(duri->query != NULL) {
	NClist* qp = nclistnew();
	if(parselist(duri->query,EOFCHAR,'&',qp,NULL)) {
	    nclistpush(qp,NULL);
	    duri->querylist = nclistextract(qp);
	} else
	    {THROW(NC_ENOMEM);}
	nclistfree(qp);
    }
    /* Parse the fragment if exists */
    if(duri->fragment != NULL) {
	if(params == NULL) params = nclistnew();
	if(!parselist(duri->fragment,EOFCHAR,'&',params,NULL))
	    {THROW(NC_ENOMEM);}
    }
    if(params != NULL && nclistlength(params) > 0) {
	nclistpush(params,NULL);
        duri->fraglist = nclistextract(params);
    }
    nclistfree(params);

#ifdef NCXDEBUG
	{
        fprintf(stderr,"duri:");
	fprintf(stderr," protocol=|%s|",FIX(duri->protocol));
	fprintf(stderr," user=|%s|",FIX(duri->user));
	fprintf(stderr," password=|%s|",FIX(duri->password));
	fprintf(stderr," host=|%s|",FIX(duri->host));
	fprintf(stderr," port=|%s|",FIX(duri->port));
	fprintf(stderr," path=|%s|",FIX(duri->path));
	fprintf(stderr," query=|%s|",FIX(duri->query));
	fprintf(stderr," fragment=|%s|",FIX(duri->fragment));
        fprintf(stderr,"\n");
    }
#endif
    if(durip != NULL)
      *durip = duri;
    else
      ncurifree(duri);
    return NCU_OK;

fail:
    if(duri != NULL) {
	ncurifree(duri);
    }
    return NCU_EBADURL;
}

static void
clearlist(char** list)
{
    if(list != NULL) {
	char** p;	
        for(p=list;*p;p++) {nullfree(*p);}
	nullfree(list);
    }    
}

void
ncurifree(NCURI* duri)
{
    char** p;
    if(duri == NULL) return;
    nullfree(duri->uri);
    nullfree(duri->protocol);
    nullfree(duri->user);
    nullfree(duri->password);
    nullfree(duri->host);
    nullfree(duri->port);
    nullfree(duri->path);
    nullfree(duri->query);
    nullfree(duri->fragment);
    clearlist(duri->querylist);
    clearlist(duri->fraglist);
    free(duri);
}

/* Replace the protocol */
int
ncurisetprotocol(NCURI* duri,const char* protocol)
{
    nullfree(duri->protocol);
    duri->protocol = strdup(protocol);
    return (NCU_OK);
}

/* Replace the query */
int
ncurisetquery(NCURI* duri,const char* query)
{
    failpoint = NCU_OK;
    clearlist(duri->querylist);    
    nullfree(duri->query);
    duri->query = NULL;
    duri->querylist = NULL;
    if(query != NULL && strlen(query) > 0) {
	NClist* params = nclistnew();
	duri->query = strdup(query);
	if(!parselist(duri->query,EOFCHAR,'&',params,NULL))
	    {THROW(NC_EURL);}
	nclistpush(params,NULL);
	duri->querylist = nclistextract(params);
	nclistfree(params);
    }
fail:
    return failpoint;
}

#if 0
/* Replace the constraints */
int
ncurisetconstraints(NCURI* duri,const char* constraints)
{
    char* proj = NULL;
    char* select = NULL;
    const char* p;

    if(duri->constraint != NULL) free(duri->constraint);
    if(duri->projection != NULL) free(duri->projection);
    if(duri->selection != NULL) free(duri->selection);
    duri->constraint = NULL;
    duri->projection = NULL;
    duri->selection = NULL;

    if(constraints == NULL || strlen(constraints)==0) return (NCU_ECONSTRAINTS);

    duri->constraint = nulldup(constraints);
    if(*duri->constraint == '?')
	nclshift1(duri->constraint);

    p = duri->constraint;
    proj = (char*) p;
    select = strchr(proj,'&');
    if(select != NULL) {
        size_t plen = (size_t)(select - proj);
	if(plen == 0) {
	    proj = NULL;
	} else {
	    proj = (char*)malloc(plen+1);
	    memcpy((void*)proj,p,plen);
	    proj[plen] = EOFCHAR;
	}
	select = nulldup(select);
    } else {
	proj = nulldup(proj);
	select = NULL;
    }
    duri->projection = proj;
    duri->selection = select;
    return NCU_OK;
}
#endif

/* Construct a complete NC URI.
   Optionally with the constraints.
   Optionally with the user parameters.
   Caller frees returned string.
   Optionally encode the pieces.
*/

char*
ncuribuild(NCURI* duri, const char* prefix, const char* suffix, int flags)
{
    size_t len = 0;
    char* newuri = NULL;
    NCbytes* buf = ncbytesnew();
#ifdef NEWESCAPE
    const int encode = (flags&NCURIENCODE ? 1 : 0);
#else
    const int encode = 0;
#endif

    if(prefix != NULL)
	ncbytescat(buf,prefix);

    ncbytescat(buf,duri->protocol);
    ncbytescat(buf,"://"); /* this will produce file:///... */

    if((flags & NCURIPWD) && duri->user != NULL && duri->password != NULL) {
	ncbytescat(buf,duri->user);
	ncbytescat(buf,":");
	ncbytescat(buf,duri->password);
	ncbytescat(buf,"@");
    }
    if(duri->host != NULL) ncbytescat(buf,duri->host);
    if(duri->port != NULL) {
	ncbytescat(buf,":");
	ncbytescat(buf,duri->port);
    }
    if((flags & NCURIPATH)) {
	if(duri->path == NULL)
	    ncbytescat(buf,"/");
	else if(encode) {
	    char* encoded = ncuriencode(duri->path,pathallow);
	    ncbytescat(buf,encoded);
	    nullfree(encoded);
	} else 	
	    ncbytescat(buf,duri->path);
    }

    /* The suffix is intended to some kind of path extension (e.g. .dds)
       so insert here
    */
    if(suffix != NULL)
	ncbytescat(buf,suffix);

    if((flags & NCURIQUERY) && duri->querylist != NULL) {
	char** p;
	int first = 1;
	for(p=duri->querylist;*p;p+=2,first=0) {
	    ncbytescat(buf,(first?"?":"&"));
	    ncbytescat(buf,p[0]);
	    if(p[1] != NULL && strlen(p[1]) > 0) {
		ncbytescat(buf,"=");
		if(encode) {
		    char* encoded = ncuriencode(p[1],queryallow);
		    ncbytescat(buf,encoded);
	            nullfree(encoded);
		} else 	
		    ncbytescat(buf,p[1]);
	    }
	}	
    }
    if((flags & NCURIFRAG) && duri->fraglist != NULL) {
	char** p;
	int first = 1;
	for(p=duri->fraglist;*p;p+=2,first=0) {
	    ncbytescat(buf,(first?"#":"&"));
	    ncbytescat(buf,p[0]);
	    if(p[1] != NULL && strlen(p[1]) > 0) {
		ncbytescat(buf,"=");
		if(encode) {
		    char* encoded = ncuriencode(p[1],queryallow);
		    ncbytescat(buf,encoded);
	            nullfree(encoded);
		} else 	
		    ncbytescat(buf,p[1]);
	    }
	}	
    }
    ncbytesnull(buf);
    newuri = ncbytesextract(buf);
    ncbytesfree(buf);
    return newuri;
}

#if 0
int
ncuridecodeparams(NCURI* ncuri)
{
    char* cp = NULL;
    int i,c;
    size_t nparams, len;
    char* params = NULL;
    char** plist;

    if(ncuri == NULL) return NCU_EPARAMS;
    if(ncuri->params == NULL) return NCU_OK;

    len = (strlen(ncuri->params)+1);
    params = (char*)malloc(len); /* so we can modify */
    if(!params) return NC_ENOMEM;
    memcpy(params,ncuri->params,len);
    params[len] = '\0';

    /* Pass 1 to break string into pieces at the ampersands
       and count # of pairs */
    nparams=0;
    for(cp=params;(c=*cp);cp++) {
	if(c == '\\') {cp++;}
	else if(c == '&') {*cp = EOFCHAR; nparams++;}
    }
    nparams++; /* for last one */

    /* plist is an env style list */
    plist = (char**)calloc(1,sizeof(char*)*(2*nparams+1)); /* +1 for null termination */
    if(plist == NULL) {
      if(params) free(params);
      return NCU_ENOMEM;
    }

    /* Break up each param into a (name,value) pair*/
    /* and insert into the param list */
    /* parameters of the form name name= are converted to name=""*/
    for(cp=params,i=0;i<nparams;i++) {
	char* next = cp+strlen(cp)+1; /* save ptr to next pair*/
	char* vp;
	/*break up the ith param*/
	vp = strchr(cp,'=');
	if(vp != NULL) {*vp = EOFCHAR; vp++;} else {vp = "";}
	plist[2*i] = nulldup(cp);
	plist[2*i+1] = nulldup(vp);
	cp = next;
    }
    plist[2*nparams] = NULL;
    free(params);
    if(ncuri->paramlist != NULL)
	ncparamfree(ncuri->paramlist);
    ncuri->paramlist = plist;
    return NCU_OK;
}
#endif

const char*
ncurilookup(NCURI* uri, const char* key)
{
  int i;
  char* value = NULL;
  if(uri == NULL || key == NULL || uri->fraglist == NULL) return NULL;
  i = ncfind(uri->fraglist,key);
  if(i < 0)
    return NULL;
  value = uri->fraglist[(2*i)+1];
  return value;
}

int
ncuriremoveparam(NCURI* uri, const char* key)
{
    char** p;
    char** q = NULL;

    if(uri->fraglist == NULL) return NCU_OK;
    for(q=uri->fraglist,p=uri->fraglist;*p;) {
        if(strcmp(key,*p)==0) {
	    p += 2; /* skip this entry */	
	} else {
	    *q++ = *p++; /* move key */
	    *q++ = *p++; /* move value */
	}
    }
    return NCU_OK;
}




#if 0
int
ncurisetparams(NCURI* uri, const char* newparams)
{
    if(uri == NULL) return NCU_EBADURL;
    if(uri->paramlist != NULL) ncparamfree(uri->paramlist);
    uri->paramlist = NULL;
    if(uri->params != NULL) free(uri->params);
    uri->params = nulldup(newparams);
    return NCU_OK;
}
#endif

/* Internal version of lookup; returns the paired index of the key */
static int
ncfind(char** params, const char* key)
{
    int i;
    char** p;
    for(i=0,p=params;*p;p+=2,i++) {
	if(strcmp(key,*p)==0) return i;
    }
    return -1;
}


#if 0
static void
ncparamfree(char** params)
{
    char** p;
    if(params == NULL) return;
    for(p=params;*p;p+=2) {
	free(*p);
	if(p[1] != NULL) free(p[1]);
    }
    free(params);
}
#endif

/* Return the ptr to the first occurrence of
   any char in the list. Return NULL if no
   occurrences
*/
static char*
nclocate(char* p, const char* charlist)
{
    for(;*p;p++) {
	if(*p == '\\') p++;
	else if(strchr(charlist,*p) != NULL)
	    return p;
    }
    return NULL;
}

#if 0
/* Shift every char starting at p 1 place to the left */
static void
nclshift1(char* p)
{
    if(p != NULL && *p != EOFCHAR) {
	char* q = p++;
	while((*q++=*p++));
    }
}

/* Shift every char starting at p 1 place to the right */
static void
ncrshift1(char* p)
{
    char cur;
    cur = 0;
    do {
	char next = *p;
	*p++ = cur;
	cur = next;
    } while(cur != 0);
    *p = 0; /* make sure we are still null terminated */
}
#endif

/* Provide % encoders and decoders */

static char* hexchars = "0123456789abcdefABCDEF";

static void
toHex(unsigned int b, char hex[2])
{
    hex[0] = hexchars[(b >> 4) & 0xff];
    hex[1] = hexchars[(b) & 0xff];
}


static int
fromHex(int c)
{
    if(c >= '0' && c <= '9') return (int) (c - '0');
    if(c >= 'a' && c <= 'f') return (int) (10 + (c - 'a'));
    if(c >= 'A' && c <= 'F') return (int) (10 + (c - 'A'));
    return 0;
}


/* Return a string representing encoding of input; caller must free;
   watch out: will encode whole string, so watch what you give it.
   Allowable argument specifies characters that do not need escaping.
 */

char*
ncuriencode(char* s, char* allowable)
{
    size_t slen;
    char* encoded;
    char* inptr;
    char* outptr;

    if(s == NULL) return NULL;

    slen = strlen(s);
    encoded = (char*)malloc((3*slen) + 1); /* max possible size */

    for(inptr=s,outptr=encoded;*inptr;) {
	int c = *inptr++;
        if(c == ' ') {
	    *outptr++ = '+';
        } else {
            /* search allowable */
            int c2;
	    char* a = allowable;
	    while((c2=*a++)) {
		if(c == c2) break;
	    }
            if(c2) {*outptr++ = (char)c;}
            else {
		char hex[2];
		toHex(c,hex);
		*outptr++ = '%';
		*outptr++ = hex[0];
		*outptr++ = hex[1];
            }
        }
    }
    *outptr = EOFCHAR;
    return encoded;
}

/* Return a string representing decoding of input; caller must free;*/
char*
ncuridecode(char* s)
{
    return ncuridecodeonly(s,NULL);
}

/* Return a string representing decoding of input only for specified
   characters;  caller must free
*/
char*
ncuridecodeonly(char* s, char* only)
{
    size_t slen;
    char* decoded;
    char* outptr;
    char* inptr;
    unsigned int c;

    if (s == NULL) return NULL;

    slen = strlen(s);
    decoded = (char*)malloc(slen+1); /* Should be max we need */

    outptr = decoded;
    inptr = s;
    while((c = (unsigned int)*inptr++)) {
	if(c == '+' && only != NULL && strchr(only,'+') != NULL)
	    *outptr++ = ' ';
	else if(c == '%') {
            /* try to pull two hex more characters */
	    if(inptr[0] != EOFCHAR && inptr[1] != EOFCHAR
		&& strchr(hexchars,inptr[0]) != NULL
		&& strchr(hexchars,inptr[1]) != NULL) {
		/* test conversion */
		int xc = (fromHex(inptr[0]) << 4) | (fromHex(inptr[1]));
		if(only == NULL || strchr(only,xc) != NULL) {
		    inptr += 2; /* decode it */
		    c = (unsigned int)xc;
                }
            }
        }
        *outptr++ = (char)c;
    }
    *outptr = EOFCHAR;
    return decoded;
}

static int
parselist(const char* text0, char lsep, char rsep, NClist* list, char** nextp)
{
    char lparen[2];
    char rparen[2];
    int singlesep = 0; /* keep single separator in rparen */

    if(lsep == EOFCHAR && rparen == EOFCHAR) {rsep = '&';}
    if(lsep != EOFCHAR && rparen == EOFCHAR) {rsep = lsep; lsep = EOFCHAR;}
    lparen[0] = lsep; lparen[1] = EOFCHAR;
    rparen[0] = rsep; rparen[1] = EOFCHAR;

    singlesep = (lsep == EOFCHAR && rsep != EOFCHAR ? 1 : 0);

    if(strlen(text0) > 0) {
	char* text = strdup(text0);
	char* sp = text; /* start of parameter */
	int more;

	if(!singlesep && *text0 != lsep)
	    return 0;

	text = strdup(text0);
	sp = text; /* start of parameter */
	more = (*sp != EOFCHAR);
	while(more) {
	    char *key, *value, *eq, *ep;

	    if(!singlesep && *sp != lsep)
		return 0; /* malformed */
	    if(!singlesep && *sp == lsep)
		sp++; /* skip lparen */
            /* use nclocate because \\ escapes might be present */
	    ep = nclocate(sp,rparen);
	    if(singlesep) {
	        if(ep == NULL) {/* NULL => we are at last parameter */
	            ep = endof(sp);
		    more = 0;
		}
	    } else {
		if(ep == NULL)
		    return 0; /* missing the terminating right paren */
		more = (ep[1] == lsep ? 1 : 0);
	    }
	    terminate(ep); /* overwrite rsep (ok if ep was null) */
            eq = strchr(sp,'=');
	    if(eq != NULL && eq < ep) { /* value is present */	    
	        terminate(eq); eq++;
	        key = strdup(sp);
	        value = strdup(eq);
	    } else {/* there is no value */
		key = strdup(sp);
		value = strdup("");
	    }
	    nclistpush(list,key);
	    nclistpush(list,value);
	    sp = ep+1;
	}
        if(nextp) *nextp = sp;
    }
    return 1;
}
