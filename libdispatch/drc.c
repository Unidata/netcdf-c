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

#define RCFILEENV "DAPRCFILE"

#define RTAG ']'
#define LTAG '['

#define TRIMCHARS " \t\r\n"

#undef MEMCHECK
#define MEMCHECK(x) if((x)==NULL) {goto nomem;} else {}

/* Forward */
static char* rcreadline(char** nextlinep);
static void rctrim(char* text);
static NClist* rcorder(NClist* rc);
static int rccompile(const char* path);
static int rcsetinfocurlflag(NCRCinfo* info, const char* flag, const char* value);
static struct NCTriple* rclocate(const char* key, const char* hostport);
static int rcsearch(const char* prefix, const char* rcname, char** pathp);
static int parsecredentials(const char* userpwd, char** userp, char** pwdp);
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

    if(ncrc_globalstate.rc.ignore) {
        nclog(NCLOGDBG,"No runtime configuration file specified; continuing");
	return (NC_NOERR);
    }
    if(ncrc_globalstate.rc.loaded) return (NC_NOERR);

    /* locate the configuration files in the following order:
       1. specified by NC_set_rcfile
       2. set by DAPRCFILE env variable
       3. ./<rcfile> (current directory)
       4. $HOME/<rcfile>
    */
    if(ncrc_globalstate.rc.rcfile != NULL) { /* always use this */
	path = strdup(ncrc_globalstate.rc.rcfile);
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
    ncrc_globalstate.rc.loaded = 1; /* even if not exists */
    nullfree(path);
    return (ret);
}

int
NC_parseproxy(NCRCinfo* info, const char* surl)
{
    int ret = NC_NOERR;
    NCURI* uri = NULL;
    if(surl == NULL || strlen(surl) == 0)
	return (NC_NOERR); /* nothing there*/
    if(ncuriparse(surl,&uri) != NCU_OK)
	return (NC_EURL);
    info->proxy.user = uri->user;
    info->proxy.pwd = uri->password;
    info->proxy.host = strdup(uri->host);
    if(uri->port != NULL)
        info->proxy.port = atoi(uri->port);
    else
        info->proxy.port = 80;
    return (ret);
}

int
NC_rcprocess(NCRCinfo* info, NCURI* url)
{
    int ret = NC_NOERR;
    char* url_hostport = NULL;

    if(!ncrc_globalstate.rc.loaded)
	NC_rcload();

    if(url != NULL)
	url_hostport = NC_combinehostport(url->host,url->port);    

    /* Note, we still must do this function even if
       ncrc_globalstate.rc.ignore is set in order
       to getinfo e.g. host+port  from url
    */

    rcsetinfocurlflag(info,"HTTP.DEFLATE",
		      NC_rclookup("HTTP.DEFLATE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.VERBOSE",
			NC_rclookup("HTTP.VERBOSE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.TIMEOUT",
			NC_rclookup("HTTP.TIMEOUT",url_hostport));
    rcsetinfocurlflag(info,"HTTP.USERAGENT",
			NC_rclookup("HTTP.USERAGENT",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIEFILE",
			NC_rclookup("HTTP.COOKIEFILE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIE_FILE",
			NC_rclookup("HTTP.COOKIE_FILE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIEJAR",
			NC_rclookup("HTTP.COOKIEJAR",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIE_JAR",
			NC_rclookup("HTTP.COOKIE_JAR",url_hostport));
    rcsetinfocurlflag(info,"HTTP.PROXY_SERVER",
			NC_rclookup("HTTP.PROXY_SERVER",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.VALIDATE",
			NC_rclookup("HTTP.SSL.VALIDATE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.CERTIFICATE",
			NC_rclookup("HTTP.SSL.CERTIFICATE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.KEY",
			NC_rclookup("HTTP.SSL.KEY",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.KEYPASSWORD",
			NC_rclookup("HTTP.SSL.KEYPASSWORD",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.CAINFO",
			NC_rclookup("HTTP.SSL.CAINFO",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.CAPATH",
			NC_rclookup("HTTP.SSL.CAPATH",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.VERIFYPEER",
			NC_rclookup("HTTP.SSL.VERIFYPEER",url_hostport));
    rcsetinfocurlflag(info,"HTTP.NETRC",
			NC_rclookup("HTTP.NETRC",url_hostport));
    { /* Handle various cases for user + password */
	/* First, see if the user+pwd was in the original url */
	char* user = NULL;
	char* pwd = NULL;
	if(url->user != NULL && url->password != NULL) {
	    user = url->user;
	    pwd = url->password;
	} else {
   	    user = NC_rclookup("HTTP.CREDENTIALS.USER",url_hostport);
	    pwd = NC_rclookup("HTTP.CREDENTIALS.PASSWORD",url_hostport);
	}
	if(user != NULL && pwd != NULL) {
            user = strdup(user); /* so we can consistently reclaim */
            pwd = strdup(pwd);
	} else {
	    /* Could not get user and pwd, so try USERPASSWORD */
	    const char* userpwd = NC_rclookup("HTTP.CREDENTIALS.USERPASSWORD",url_hostport);
	    if(userpwd != NULL) {
		ret = parsecredentials(userpwd,&user,&pwd);
		if(ret) return ret;
	    }
        }
        rcsetinfocurlflag(info,"HTTP.USERNAME",user);
        rcsetinfocurlflag(info,"HTTP.PASSWORD",pwd);
	nullfree(user);
	nullfree(pwd);
    }
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

void
NC_rcfreetriples(NClist* rc)
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
    nullfree(ncrc_globalstate.rc.rcfile);
    ncrc_globalstate.rc.rcfile = strdup(rcfile);
    /* (re) load the rcfile and esp the triplestore*/
    stat = NC_rcload();
done:
    return stat;
}

void
NC_rcclear(NCRCinfo* info)
{
    if(info == NULL) return;
    if(info->curlflags.createdflags & COOKIECREATED) {
#ifdef _MSC_VER
        DeleteFile(curl->curlflags.cookiejar);
#else
        remove(curl->curlflags.cookiejar);
#endif
    }
    nullfree(info->curlflags.useragent);
    nullfree(info->curlflags.cookiejar);
    nullfree(info->curlflags.netrc);
    nullfree(info->ssl.certificate);
    nullfree(info->ssl.key);
    nullfree(info->ssl.keypasswd);
    nullfree(info->ssl.cainfo);
    nullfree(info->ssl.capath);
    nullfree(info->proxy.host);
    nullfree(info->proxy.user);
    nullfree(info->proxy.pwd);
    nullfree(info->creds.user);
    nullfree(info->creds.pwd);
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
    size_t len;
    int i;

    len = strlen(text);
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
static NClist*
rcorder(NClist* rc)
{
    int i,j;
    int len = nclistlength(rc);
    NClist* newrc = nclistnew();
    if(rc == NULL || len == 0) return newrc;
    /* Two passes: 1) pull triples with host */
    for(i=0;i<len;i++) {
        NCTriple* ti = nclistget(rc,i);
	if(ti->host == NULL) continue;
	nclistpush(newrc,ti);
    }
    /* pass 2 pull triples without host*/
    for(i=0;i<len;i++) {
        NCTriple* ti = nclistget(rc,i);
	if(ti->host != NULL) continue;
	nclistpush(newrc,ti);
    }
#ifdef D4DEBUG
    storedump("reorder:",newrc);
#endif
    return newrc;
}

/* Create a triple store from a file */
static int
rccompile(const char* path)
{
    int ret = NC_NOERR;
    NClist* rc = ncrc_globalstate.rc.triples;
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
    NC_rcfreetriples(rc); /* clear out any old data */
    rc = nclistnew();
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
                continue;
            }
            line = rtag + 1;
            *rtag = '\0';
            /* compile the url and pull out the host */
	    if(uri) ncurifree(uri);
	    if(ncuriparse(url,&uri) != NCU_OK) {
                nclog(NCLOGERR, "Malformed [url] in %s entry: %s",path,line);
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
		triple->host,triple->key,triple->valu);
#endif
	nclistpush(rc,triple);
	triple = NULL;
    }
    rcorder(rc);

done:
    ncurifree(uri);
    ncbytesfree(tmp);
    return (ret);
}

static int
rcsetinfocurlflag(NCRCinfo* info, const char* flag, const char* value)
{
    int ret = NC_NOERR;
    if(value == NULL) goto done;
    if(strcmp(flag,"HTTP.DEFLATE")==0) {
        if(atoi(value)) info->curlflags.compress = 1;
#ifdef D4DEBUG
        nclog(NCLOGNOTE,"HTTP.DEFLATE: %ld", infoflags.compress);
#endif
    }
    if(strcmp(flag,"HTTP.VERBOSE")==0) {
        if(atoi(value)) info->curlflags.verbose = 1;
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.VERBOSE: %ld", info->curlflags.verbose);
#endif
    }
    if(strcmp(flag,"HTTP.TIMEOUT")==0) {
        if(atoi(value)) info->curlflags.timeout = atoi(value);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.TIMEOUT: %ld", info->curlflags.timeout);
#endif
    }
    if(strcmp(flag,"HTTP.USERAGENT")==0) {
        if(atoi(value)) info->curlflags.useragent = strdup(value);
        MEMCHECK(info->curlflags.useragent);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.USERAGENT: %s", info->curlflags.useragent);
#endif
    }
    if(
	strcmp(flag,"HTTP.COOKIEFILE")==0
        || strcmp(flag,"HTTP.COOKIE_FILE")==0
        || strcmp(flag,"HTTP.COOKIEJAR")==0
        || strcmp(flag,"HTTP.COOKIE_JAR")==0
      ) {
	nullfree(info->curlflags.cookiejar);
        info->curlflags.cookiejar = strdup(value);
        MEMCHECK(info->curlflags.cookiejar);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.COOKIEJAR: %s", info->curlflags.cookiejar);
#endif
    }
    if(strcmp(flag,"HTTP.PROXY_SERVER")==0) {
        ret = NC_parseproxy(info,value);
        if(ret != NC_NOERR) goto done;
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.PROXY_SERVER: %s", value);
#endif
    }
    if(strcmp(flag,"HTTP.SSL.VALIDATE")==0) {
        if(atoi(value)) {
	    info->ssl.verifypeer = 1;
	    info->ssl.verifyhost = 1;
#ifdef D4DEBUG
                nclog(NCLOGNOTE,"HTTP.SSL.VALIDATE: %ld", 1);
#endif
	}
    }

    if(strcmp(flag,"HTTP.SSL.CERTIFICATE")==0) {
	nullfree(info->ssl.certificate);
        info->ssl.certificate = strdup(value);
        MEMCHECK(info->ssl.certificate);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.CERTIFICATE: %s", info->ssl.certificate);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.KEY")==0) {
	nullfree(info->ssl.key);
        info->ssl.key = strdup(value);
        MEMCHECK(info->ssl.key);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.KEY: %s", info->ssl.key);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.KEYPASSWORD")==0) {
	nullfree(info->ssl.keypasswd) ;
        info->ssl.keypasswd = strdup(value);
        MEMCHECK(info->ssl.keypasswd);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.KEYPASSWORD: %s", info->ssl.keypasswd);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.CAINFO")==0) {
	nullfree(info->ssl.cainfo) ;
        info->ssl.cainfo = strdup(value);
        MEMCHECK(info->ssl.cainfo);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.CAINFO: %s", info->ssl.cainfo);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.CAPATH")==0) {
	nullfree(info->ssl.capath) ;
        info->ssl.capath = strdup(value);
        MEMCHECK(info->ssl.capath);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.CAPATH: %s", info->ssl.capath);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.VERIFYPEER")==0) {
        const char* s = value;
        int tf = 0;
        if(s == NULL || strcmp(s,"0")==0 || strcasecmp(s,"false")==0)
            tf = 0;
        else if(strcmp(s,"1")==0 || strcasecmp(s,"true")==0)
            tf = 1;
        else
            tf = 1; /* default if not null */
        info->ssl.verifypeer = tf;
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.VERIFYPEER: %d", info->ssl.verifypeer);
#endif
    }

    if(strcmp(flag,"HTTP.NETRC")==0) {
        nullfree(info->curlflags.netrc);
        info->curlflags.netrc = strdup(value);
        MEMCHECK(info->curlflags.netrc);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.NETRC: %s", info->curlflags.netrc);
#endif
    }

    if(strcmp(flag,"HTTP.CREDENTIALS.USERNAME")==0) {
        nullfree(info->creds.user);
        info->creds.user = strdup(value);
        MEMCHECK(info->creds.user);
    }
    if(strcmp(flag,"HTTP.CREDENTIALS.PASSWORD")==0) {
        nullfree(info->creds.pwd);
        info->creds.pwd = strdup(value);
        MEMCHECK(info->creds.pwd);
    }

done:
    return (ret);

nomem:
    return (NC_ENOMEM);
}


/**
 * (Internal) Locate a triple by property key and host+port (may be null or "").
 * If duplicate keys, first takes precedence.
 */
static struct NCTriple*
rclocate(const char* key, const char* hostport)
{
    int i,found;
    NClist* rc = ncrc_globalstate.rc.triples;
    NCTriple* triple = NULL;

    if(ncrc_globalstate.rc.ignore)
	return NULL;
    if(!ncrc_globalstate.rc.loaded)
	NC_rcload();

    if(key == NULL || rc == NULL) return NULL;
    if(hostport == NULL) hostport = "";

    for(found=0,i=0;i<nclistlength(rc);i++) {
	triple = (NCTriple*)nclistget(rc,i);
        size_t hplen = strlen(triple->host);
        int t;
        if(strcmp(key,triple->key) != 0) continue; /* keys do not match */
        /* If the triple entry has no url, then use it
           (because we have checked all other cases)*/
        if(hplen == 0) {found=1;break;}
        /* do hostport match */
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

/*
Given form user:pwd, parse into user and pwd
and do %xx unescaping
*/
static int
parsecredentials(const char* userpwd, char** userp, char** pwdp)
{
    char* user = NULL;
    char* pwd = NULL;

    if(userpwd == NULL)
	return NC_EINVAL;
    user = strdup(userpwd);
    if(user == NULL)
	return NC_ENOMEM;
    pwd = strchr(user,':');
    if(pwd == NULL)
	return NC_EINVAL;
    *pwd = '\0';
    pwd++;
    if(userp)
	*userp = ncuridecode(user);
    if(pwdp)
	*pwdp = ncuridecode(pwd);
    free(user);
    return NC_NOERR;
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

