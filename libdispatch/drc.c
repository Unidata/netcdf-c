/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netcdf.h"
#include "ncbytes.h"
#include "ncuri.h"
#include "ncrc.h"
#include "nclog.h"
#include "ncwinpath.h"
#ifdef HAVE_CURL_H
#include <curl/curl.h>
#else
#define CURL_MAX_READ_SIZE (512*1024)
#endif

#define RCFILEENV "DAPRCFILE"

#define RTAG ']'
#define LTAG '['

#define TRIMCHARS " \t\r\n"

#undef MEMCHECK
#define MEMCHECK(x) if((x)==NULL) {goto nomem;} else {}

/* Forward */
static char* rcreadline(char** nextlinep);
static void rctrim(char* text);
static void rcorder(NClist* rc);
static int rccompile(const char* path);
static struct NCTriple* rclocate(const char* key, const char* hostport);
static int rcsearch(const char* prefix, const char* rcname, char** pathp);
static void rcfreetriples(NClist* rc);
#ifdef D4DEBUG
static void storedump(char* msg, NClist* triples);
#endif

/* Define default rc files and aliases, also defines search order*/
static char* rcfilenames[] = {".daprc",".dodsrc",NULL};

/**************************************************/
/* External Entry Points */


/* locate, read and compile the rc file, if any */
int
NC_rcload(void)
{
    int ret = NC_NOERR;
    char* path = NULL;

    if(ncrc_globalstate.rcinfo.ignore) {
        nclog(NCLOGDBG,"No runtime configuration file specified; continuing");
	return (NC_NOERR);
    }
    if(ncrc_globalstate.rcinfo.loaded) return (NC_NOERR);

    /* locate the configuration files in the following order:
       1. specified by NC_set_rcfile
       2. set by DAPRCFILE env variable
       3. ./<rcfile> (current directory)
       4. $HOME/<rcfile>
    */
    if(ncrc_globalstate.rcinfo.rcfile != NULL) { /* always use this */
	path = strdup(ncrc_globalstate.rcinfo.rcfile);
    } else if(getenv(RCFILEENV) != NULL && strlen(getenv(RCFILEENV)) > 0) {
        path = strdup(getenv(RCFILEENV));
    } else {
	char** rcname;
	int found = 0;
	for(rcname=rcfilenames;!found && *rcname;rcname++) {
	    ret = rcsearch(".",*rcname,&path);
    	    if(ret == NC_NOERR && path == NULL)  /* try $HOME */
	        ret = rcsearch(ncrc_globalstate.home,*rcname,&path);
	    if(ret != NC_NOERR)
		goto done;
	    if(path != NULL)
		found = 1;
	}
    }
    if(path == NULL) {
        nclog(NCLOGDBG,"Cannot find runtime configuration file; continuing");
    } else {
#ifdef D4DEBUG
        fprintf(stderr, "RC file: %s\n", path);
#endif
        if((ret=rccompile(path))) {
	    nclog(NCLOGERR, "Error parsing %s\n",path);
	    goto done;
	}
    }
done:
    ncrc_globalstate.rcinfo.loaded = 1; /* even if not exists */
    nullfree(path);
    return (ret);
}

/**
 * Locate a triple by property key and host+port (may be null|"")
 * If duplicate keys, first takes precedence.
 */
char*
NC_rclookup(const char* key, const char* hostport)
{
    struct NCTriple* triple = rclocate(key,hostport);
    return (triple == NULL ? NULL : triple->value);
}

/*!
Set the absolute path to use for the rc file.
WARNING: this MUST be called before any other
call in order for this to take effect.

\param[in] rcfile The path to use. If NULL, or "",
                  then do not use any rcfile.

\retval OC_NOERR if the request succeeded.
\retval OC_ERCFILE if the file failed to load
*/

int
NC_set_rcfile(const char* rcfile)
{
    int stat = NC_NOERR;
    FILE* f = NULL;

    if(rcfile != NULL && strlen(rcfile) == 0)
	rcfile = NULL;
    f = NCfopen(rcfile,"r");
    if(f == NULL) {
	stat = NC_ERCFILE;
        goto done;
    }
    fclose(f);
    nullfree(ncrc_globalstate.rcinfo.rcfile);
    ncrc_globalstate.rcinfo.rcfile = strdup(rcfile);
    /* Clear ncrc_globalstate.rcinfo */
    NC_rcclear(&ncrc_globalstate.rcinfo);
    /* (re) load the rcfile and esp the triplestore*/
    stat = NC_rcload();
done:
    return stat;
}

void
NC_rcclear(NCRCinfo* info)
{
    if(info == NULL) return;
    nullfree(info->rcfile);
    rcfreetriples(info->triples);
}

void
rcfreetriples(NClist* rc)
{
    int i;
    for(i=0;i<nclistlength(rc);i++) {
	NCTriple* t = (NCTriple*)nclistget(rc,i);
	nullfree(t->host);
	nullfree(t->key);
	nullfree(t->value);
	free(t);
    }
    nclistfree(rc);
}

/**************************************************/
/* RC processing functions */

static char*
rcreadline(char** nextlinep)
{
    char* line;
    char* p;

    line = (p = *nextlinep);
    if(*p == '\0') return NULL; /*signal done*/
    for(;*p;p++) {
	if(*p == '\r' && p[1] == '\n') *p = '\0';
	else if(*p == '\n') break;
    }
    *p++ = '\0'; /* null terminate line; overwrite newline */
    *nextlinep = p;
    return line;
}

/* Trim TRIMCHARS from both ends of text; */
static void
rctrim(char* text)
{
    char* p = text;
    size_t len = 0;
    int i;

    /* locate first non-trimchar */
    for(;*p;p++) {
       if(strchr(TRIMCHARS,*p) == NULL) break; /* hit non-trim char */
    }
    memmove(text,p,strlen(p)+1);
    len = strlen(text);
    /* locate last non-trimchar */
    if(len > 0) {
        for(i=(len-1);i>=0;i--) {
            if(strchr(TRIMCHARS,text[i]) == NULL) {
                text[i+1] = '\0'; /* elide trailing trimchars */
                break;
            }
        }
    }
}

/* Order the triples: those with urls must be first,
   but otherwise relative order does not matter.
*/
static void
rcorder(NClist* rc)
{
    int i;
    int len = nclistlength(rc);
    NClist* tmprc = NULL;
    if(rc == NULL || len == 0) return;
    tmprc = nclistnew();
    /* Copy rc into tmprc and clear rc */
    for(i=0;i<len;i++) {
        NCTriple* ti = nclistget(rc,i);
        nclistpush(tmprc,ti);
    }
    nclistclear(rc);
    /* Two passes: 1) pull triples with host */
    for(i=0;i<len;i++) {
        NCTriple* ti = nclistget(tmprc,i);
	if(ti->host == NULL) continue;
	nclistpush(rc,ti);
    }
    /* pass 2 pull triples without host*/
    for(i=0;i<len;i++) {
        NCTriple* ti = nclistget(tmprc,i);
	if(ti->host != NULL) continue;
	nclistpush(rc,ti);
    }
#ifdef D4DEBUG
    storedump("reorder:",rc);
#endif
    nclistfree(tmprc);
}

/* Create a triple store from a file */
static int
rccompile(const char* path)
{
    int ret = NC_NOERR;
    NClist* rc = NULL;
    char* contents = NULL;
    NCbytes* tmp = ncbytesnew();
    NCURI* uri = NULL;
    char* nextline = NULL;

    if((ret=NC_readfile(path,tmp))) {
        nclog(NCLOGERR, "Could not open configuration file: %s",path);
	goto done;
    }
    contents = ncbytesextract(tmp);
    if(contents == NULL) contents = strdup("");
    /* Either reuse or create new  */
    rc = ncrc_globalstate.rcinfo.triples;
    if(rc != NULL)
        rcfreetriples(rc); /* clear out any old data */
    else {
        rc = nclistnew();
        ncrc_globalstate.rcinfo.triples = rc;
    }
    nextline = contents;
    for(;;) {
	char* line;
	char* key;
        char* value;
	size_t llen;
        NCTriple* triple;

	line = rcreadline(&nextline);
	if(line == NULL) break; /* done */
        rctrim(line);  /* trim leading and trailing blanks */
        if(line[0] == '#') continue; /* comment */
	if((llen=strlen(line)) == 0) continue; /* empty line */
	triple = (NCTriple*)calloc(1,sizeof(NCTriple));
	if(triple == NULL) {ret = NC_ENOMEM; goto done;}
	if(line[0] == LTAG) {
	    char* url = ++line;
            char* rtag = strchr(line,RTAG);
            if(rtag == NULL) {
                nclog(NCLOGERR, "Malformed [url] in %s entry: %s",path,line);
                free(triple);
		continue;
            }
            line = rtag + 1;
            *rtag = '\0';
            /* compile the url and pull out the host */
            if(uri) ncurifree(uri);
            if(ncuriparse(url,&uri) != NCU_OK) {
                nclog(NCLOGERR, "Malformed [url] in %s entry: %s",path,line);
                free(triple);
		continue;
            }
            ncbytesclear(tmp);
            ncbytescat(tmp,uri->host);
            if(uri->port != NULL) {
		ncbytesappend(tmp,':');
                ncbytescat(tmp,uri->port);
            }
            ncbytesnull(tmp);
            triple->host = ncbytesextract(tmp);
	    if(strlen(triple->host)==0)
		{free(triple->host); triple->host = NULL;}
	}
        /* split off key and value */
        key=line;
        value = strchr(line, '=');
        if(value == NULL)
            value = line + strlen(line);
        else {
            *value = '\0';
            value++;
        }
	triple->key = strdup(key);
        triple->value = strdup(value);
        rctrim(triple->key);
        rctrim(triple->value);
#ifdef D4DEBUG
	fprintf(stderr,"rc: host=%s key=%s value=%s\n",
		(triple->host != NULL ? triple->host : "<null>"),
		triple->key,triple->valu);
#endif
	nclistpush(rc,triple);
	triple = NULL;
    }
    rcorder(rc);

done:
    if(contents) free(contents);
    ncurifree(uri);
    ncbytesfree(tmp);
    return (ret);
}

/**
 * (Internal) Locate a triple by property key and host+port (may be null or "").
 * If duplicate keys, first takes precedence.
 */
static struct NCTriple*
rclocate(const char* key, const char* hostport)
{
    int i,found;
    NClist* rc = ncrc_globalstate.rcinfo.triples;
    NCTriple* triple = NULL;

    if(ncrc_globalstate.rcinfo.ignore)
	return NULL;

    if(key == NULL || rc == NULL) return NULL;
    if(hostport == NULL) hostport = "";

    for(found=0,i=0;i<nclistlength(rc);i++) {
	triple = (NCTriple*)nclistget(rc,i);
        size_t hplen = (triple->host == NULL ? 0 : strlen(triple->host));
        int t;
        if(strcmp(key,triple->key) != 0) continue; /* keys do not match */
        /* If the triple entry has no url, then use it
           (because we have checked all other cases)*/
        if(hplen == 0) {found=1;break;}
        /* do hostport match */
	t = 0;
	if(triple->host != NULL)
            t = strcmp(hostport,triple->host);
        if(t ==  0) {found=1; break;}
    }
    return (found?triple:NULL);
}

/**
 * Locate rc file by searching in directory prefix.
 * Prefix must end in '/'
 */
static
int
rcsearch(const char* prefix, const char* rcname, char** pathp)
{
    char* path = NULL;
    FILE* f = NULL;
    int plen = strlen(prefix);
    int rclen = strlen(rcname);
    int ret = NC_NOERR;

    size_t pathlen = plen+rclen+1; /*+1 for '/' */
    path = (char*)malloc(pathlen+1); /* +1 for nul*/
    if(path == NULL) {ret = NC_ENOMEM;	goto done;}
    strncpy(path,prefix,pathlen);
    strncat(path,"/",pathlen);
    strncat(path,rcname,pathlen);
    /* see if file is readable */
    f = fopen(path,"r");
    if(f != NULL)
        nclog(NCLOGDBG, "Found rc file=%s",path);
done:
    if(f == NULL || ret != NC_NOERR) {
	nullfree(path);
	path = NULL;
    }
    if(f != NULL)
      fclose(f);
    if(pathp != NULL)
      *pathp = path;
    else {
      nullfree(path);
      path = NULL;
    }
    return (ret);
}

int
NC_rcfile_insert(const char* key, const char* value, const char* hostport)
{
    int ret = NC_NOERR;
    /* See if this key already defined */
    struct NCTriple* triple = NULL;
    NClist* rc = ncrc_globalstate.rcinfo.triples;

    if(rc == NULL) {
	rc = nclistnew();
	if(rc == NULL) {ret = NC_ENOMEM; goto done;}
    }
    triple = rclocate(key,hostport);
    if(triple == NULL) {
	triple = (NCTriple*)calloc(1,sizeof(NCTriple));
	if(triple == NULL) {ret = NC_ENOMEM; goto done;}
	triple->key = strdup(key);
	triple->value = NULL;
        rctrim(triple->key);
        triple->host = (hostport == NULL ? NULL : strdup(hostport));
	nclistpush(rc,triple);
    }
    if(triple->value != NULL) free(triple->value);
    triple->value = strdup(value);
    rctrim(triple->value);
done:
    return ret;
}

#ifdef D4DEBUG
static void
storedump(char* msg, NClist* triples)
{
    int i;

    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    if(triples == NULL || nclistlength(triples)==0) {
        fprintf(stderr,"<EMPTY>\n");
        return;
    }
    for(i=0;i<nclistlength(triples);i++) {
	NCTriple* t = (NCTriple*)nclistget(triples,i);
        fprintf(stderr,"\t%s\t%s\t%s\n",
                ((t->host == NULL || strlen(t->host)==0)?"--":t->host),t->key,t->value);
    }
    fflush(stderr);
}
#endif

/* Type specific parsers */
int
getbool(int* field, const char* value)
{
    if(value != NULL) {
        if(strcmp(value,"1")==0 || strcasecmp(value,"on")==0 || strcasecmp(value,"true")==0)
	    *field = 1;
        else if(strcmp(value,"0")==0 || strcasecmp(value,"off")==0 || strcasecmp(value,"false")==0)   
   	    *field = 0;
	else
	    return NC_EINVAL;
    }
    return NC_NOERR;
}

int
getlong(long* field, const char* value)
{
    if(value != NULL) {
	long f;
	int count = sscanf(value,"%ld",&f);
	if(count != 1 || f < 0)
	    return NC_EINVAL;
	*field = f;
    }
    return NC_NOERR;
}

int
getstring(char** field, const char* value)
{
    if(value != NULL) {
	char *f = strdup(value);
	if(f == NULL)
	    return NC_ENOMEM;
	*field = f;
    }
    return NC_NOERR;
}

/* Set a single NCRCFIELD field */
int
NC_rcloadfield(NCRCFIELDS* fields, const char* key, const char* value, const char* host)
{
    int useit;
    int stat = NC_NOERR;
    if(strcmp("HTTP.VERBOSE",key)==0) {
        useit = (host == NULL && fields->HTTP_VERBOSE < 0);
        if(useit) {if((stat=getbool(&fields->HTTP_VERBOSE,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.DEFLATE",key)==0) {
        useit = (host == NULL && fields->HTTP_DEFLATE < 0);
        if(useit) {if((stat=getbool(&fields->HTTP_DEFLATE,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.COOKIEJAR",key)==0) {
        useit = (host == NULL && fields->HTTP_COOKIEJAR == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_COOKIEJAR,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.NETRC",key)==0) {
        useit = (host == NULL && fields->HTTP_NETRC == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_NETRC,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.USERAGENT",key)==0) {
        useit = (host == NULL && fields->HTTP_USERAGENT == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_USERAGENT,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.CREDENTIALS.USERNAME",key)==0) {
        useit = (host == NULL && fields->HTTP_CREDENTIALS_USERNAME == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_CREDENTIALS_USERNAME,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.CREDENTIALS.PASSWORD",key)==0) {
        useit = (host == NULL && fields->HTTP_CREDENTIALS_PASSWORD == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_CREDENTIALS_PASSWORD,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.CREDENTIALS.USERPASSWORD",key)==0) {
        useit = (host == NULL
                                && fields->HTTP_CREDENTIALS_USERNAME == NULL
                                && fields->HTTP_CREDENTIALS_PASSWORD == NULL);
        if(useit) {
            char* p;
            char* s;
            char* usrpwd = NULL;
            if((stat=getstring(&usrpwd,value))!=NC_NOERR)
                goto done;
            s = strdup(usrpwd);
            if(s == NULL) {stat = NC_ENOMEM; goto done;}
            /* Split */
            p = strchr(s,':');
            if(p == NULL) {stat = NC_EINVAL; goto done;}
            *p++ = '\0';            
            if(strlen(s) == 0 || strlen(p) == 0) {stat = NC_EINVAL; goto done;}
            fields->HTTP_CREDENTIALS_USERNAME = s;
            s = strdup(p);
            if(s == NULL) {stat = NC_ENOMEM; goto done;}
            fields->HTTP_CREDENTIALS_PASSWORD = s;
        }
    } else if(strcmp("HTTP.SSL.CERTIFICATE",key)==0) {
        useit = (host == NULL && fields->HTTP_SSL_CERTIFICATE == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_SSL_CERTIFICATE,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.SSL.KEY",key)==0) {
        useit = (host == NULL && fields->HTTP_SSL_KEY == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_SSL_KEY,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.SSL.KEYPASSWORD",key)==0) {
        useit = (host == NULL && fields->HTTP_SSL_KEYPASSWORD == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_SSL_KEYPASSWORD,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.SSL.CAPATH",key)==0) {
        useit = (host == NULL && fields->HTTP_SSL_CAPATH == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_SSL_CAPATH,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.SSL.VALIDATE",key)==0) {
        useit = (host == NULL && fields->HTTP_SSL_VALIDATE < 0);
        if(useit) {if((stat=getbool(&fields->HTTP_SSL_VALIDATE,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.TIMEOUT",key)==0) {
        useit = (host == NULL && fields->HTTP_TIMEOUT < 0);
        if(useit) {if((stat=getlong(&fields->HTTP_TIMEOUT,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.PROXY.SERVER",key)==0) {
        useit = (host == NULL && fields->HTTP_PROXY_SERVER == NULL);
        if(useit) {if((stat=getstring(&fields->HTTP_PROXY_SERVER,value))!=NC_NOERR) goto done;}
    } else if(strcmp("HTTP.READ.BUFFERSIZE",key)==0) {
        useit = (host == NULL && fields->HTTP_READ_BUFFERSIZE < 0);
        if(useit) {
            if(strcasecmp(value,"max")==0)
                fields->HTTP_READ_BUFFERSIZE = CURL_MAX_READ_SIZE;
            if((stat=getlong(&fields->HTTP_READ_BUFFERSIZE,value))!=NC_NOERR) goto done;
        }
    } else if(strcmp("HTTP.KEEPALIVE",key)==0) {
        useit = (host == NULL && !fields->HTTP_KEEPALIVE.defined);
        if(useit) {
            char* v;
            if((stat=getstring(&v,value))!=NC_NOERR) goto done;    
            /* Parse the KEEPALIVE value */
            /* The keepalive value is of the form 0 or n/m,
               where n is the idle time and m is the interval time;
               setting either to zero will prevent that field being set. */
            if(strcasecmp(v,"on")==0) {
                fields->HTTP_KEEPALIVE.active = 1;                  
            } else {
                unsigned long idle=0;
                unsigned long interval=0;
                if(sscanf(v,"%lu/%lu",&idle,&interval) != 2)
                    {stat = NC_EINVAL; goto done;}
                fields->HTTP_KEEPALIVE.wait = idle;
                fields->HTTP_KEEPALIVE.repeat = interval;
                fields->HTTP_KEEPALIVE.active = 1;                  
            }
            fields->HTTP_KEEPALIVE.defined = 1;
        }
    } // else ignore
done:
    if(stat != NC_NOERR) {
        /* log error */
        nclog(NCLOGERR,"Illegal .daprc param  %s=%s error=%d",key,value,stat);
    }
    stat = NC_NOERR;
    return stat;
}

/**
With respect to a host+port, extract all the values for legal .daprc fields.
*/
int
NC_rcloadfields(NCRCFIELDS* fields, const char* hostport)
{
    int stat = NC_NOERR;
    int i;
    NClist* triples = ncrc_globalstate.rcinfo.triples;
    /* Iterate over all defined triples */
    for(i=0;i<nclistlength(triples);i++) {
	NCTriple* t = (NCTriple*)nclistget(triples,i);
	if(memcmp("HTTP.",t->key,5)!=0) continue; /* short circuit */
        if(t->host == NULL || strcmp(t->host,hostport)==0)
	    (void)NC_rcloadfield(fields,t->key,t->value,t->host);
	stat = NC_NOERR;
    }
    return stat; /* never fail for now */
}
