/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"

#define D4RCFILEENV "DAPRCFILE"

#define RTAG ']'
#define LTAG '['

#define TRIMCHARS " \t\r\n"

#undef MEMCHECK
#define MEMCHECK(x) if((x)==NULL) {goto nomem;} else {}

/* Forward */
static int rccompile(const char* path);
static struct NCD4triple* rclocate(char* key, char* hostport);
static NClist* rcorder(NClist* rc);
static char* rcreadline(char**);
static int rcsearch(const char* prefix, const char* rcname, char** pathp);
static void rctrim(char* text);
static int rcsetinfocurlflag(NCD4INFO*, const char* flag, const char* value);
static int parsecredentials(const char* userpwd, char** userp, char** pwdp);
#ifdef D4DEBUG
static void storedump(char* msg, NClist* triples);
#endif

/* Define default rc files and aliases, also defines search order*/
static char* rcfilenames[] = {".daprc",".dodsrc",NULL};

/* Define the curl flag defaults in envv style */
static const char* RCDEFAULTFLAGS[] = {
"HTTP.TIMEOUT","10", /*seconds */
NULL
};

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
    d4memmove(text,p,strlen(p)+1);
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
        NCD4triple* ti = nclistget(rc,i);
	if(ti->host == NULL) continue;
	nclistpush(newrc,ti);
    }
    /* pass 2 pull triples without host*/
    for(i=0;i<len;i++) {
        NCD4triple* ti = nclistget(rc,i);
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
    NClist* rc = NCD4_globalstate->rc.rc;
    char* contents = NULL;
    NCbytes* tmp = ncbytesnew();
    NCURI* uri = NULL;
    char* nextline = NULL;

    if((ret=NCD4_readfile(path,tmp))) {
        nclog(NCLOGERR, "Could not open configuration file: %s",path);	
	goto done;    
    }
    contents = ncbytesextract(tmp);
    if(contents == NULL) contents = strdup("");
    NCD4_rcfree(rc); /* clear out any old data */
    rc = nclistnew();
    nextline = contents;
    for(;;) {
	char* line;
	char* key;
        char* value;
	size_t llen;
        NCD4triple* triple;

	line = rcreadline(&nextline);
	if(line == NULL) break; /* done */
        rctrim(line);  /* trim leading and trailing blanks */
        if(line[0] == '#') continue; /* comment */
	if((llen=strlen(line)) == 0) continue; /* empty line */
	triple = (NCD4triple*)calloc(1,sizeof(NCD4triple));
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
    return THROW(ret);
}

/* locate, read and compile the rc file, if any */
int
NCD4_rcload(void)
{
    int ret = NC_NOERR;
    char* path = NULL;

    if(NCD4_globalstate->rc.ignore) {
        nclog(NCLOGDBG,"No runtime configuration file specified; continuing");
	return THROW(NC_NOERR);
    }
    if(NCD4_globalstate->rc.loaded) return THROW(NC_NOERR);

    /* locate the configuration files in the following order:
       1. specified by NCD4_set_rcfile
       2. set by DAPRCFILE env variable
       3. '.'
       4. $HOME
    */
    if(NCD4_globalstate->rc.rcfile != NULL) { /* always use this */
	path = strdup(NCD4_globalstate->rc.rcfile);
    } else if(getenv(RCFILEENV) != NULL && strlen(getenv(RCFILEENV)) > 0) {
        path = strdup(getenv(RCFILEENV));
    } else {
	char** rcname;
	int found = 0;
	for(rcname=rcfilenames;!found && *rcname;rcname++) {
	    ret = rcsearch(".",*rcname,&path);
    	    if(ret == NC_NOERR && path == NULL)  /* try $HOME */
	        ret = rcsearch(NCD4_globalstate->home,*rcname,&path);
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
    NCD4_globalstate->rc.loaded = 1; /* even if not exists */
    nullfree(path);
    return THROW(ret);
}

static int
rcsetinfocurlflag(NCD4INFO* info, const char* flag, const char* value)
{
    int ret = NC_NOERR;
    if(value == NULL) goto done;
    if(strcmp(flag,"HTTP.DEFLATE")==0) {
        if(atoi(value)) info->curl->curlflags.compress = 1;
#ifdef D4DEBUG
        nclog(NCLOGNOTE,"HTTP.DEFLATE: %ld", info->curlflags.compress);
#endif
    }
    if(strcmp(flag,"HTTP.VERBOSE")==0) {
        if(atoi(value)) info->curl->curlflags.verbose = 1;
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.VERBOSE: %ld", info->curl->curlflags.verbose);
#endif
    }
    if(strcmp(flag,"HTTP.TIMEOUT")==0) {
        if(atoi(value)) info->curl->curlflags.timeout = atoi(value);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.TIMEOUT: %ld", info->curl->curlflags.timeout);
#endif
    }
    if(strcmp(flag,"HTTP.USERAGENT")==0) {
        if(atoi(value)) info->curl->curlflags.useragent = strdup(value);
        MEMCHECK(info->curl->curlflags.useragent);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.USERAGENT: %s", info->curl->curlflags.useragent);
#endif
    }
    if(
	strcmp(flag,"HTTP.COOKIEFILE")==0
        || strcmp(flag,"HTTP.COOKIE_FILE")==0
        || strcmp(flag,"HTTP.COOKIEJAR")==0
        || strcmp(flag,"HTTP.COOKIE_JAR")==0
      ) {
	nullfree(info->curl->curlflags.cookiejar);
        info->curl->curlflags.cookiejar = strdup(value);
        MEMCHECK(info->curl->curlflags.cookiejar);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.COOKIEJAR: %s", info->curl->curlflags.cookiejar);
#endif
    }
    if(strcmp(flag,"HTTP.PROXY_SERVER")==0) {
        ret = NCD4_parseproxy(info,value);
        if(ret != NC_NOERR) goto done;
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.PROXY_SERVER: %s", value);
#endif
    }
    if(strcmp(flag,"HTTP.SSL.VALIDATE")==0) {
        if(atoi(value)) {
	    info->curl->ssl.verifypeer = 1;
	    info->curl->ssl.verifyhost = 1;
#ifdef D4DEBUG
                nclog(NCLOGNOTE,"HTTP.SSL.VALIDATE: %ld", 1);
#endif
	}
    }

    if(strcmp(flag,"HTTP.SSL.CERTIFICATE")==0) {
	nullfree(info->curl->ssl.certificate);
        info->curl->ssl.certificate = strdup(value);
        MEMCHECK(info->curl->ssl.certificate);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.CERTIFICATE: %s", info->curl->ssl.certificate);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.KEY")==0) {
	nullfree(info->curl->ssl.key);
        info->curl->ssl.key = strdup(value);
        MEMCHECK(info->curl->ssl.key);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.KEY: %s", info->curl->ssl.key);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.KEYPASSWORD")==0) {
	nullfree(info->curl->ssl.keypasswd) ;
        info->curl->ssl.keypasswd = strdup(value);
        MEMCHECK(info->curl->ssl.keypasswd);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.KEYPASSWORD: %s", info->curl->ssl.keypasswd);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.CAINFO")==0) {
	nullfree(info->curl->ssl.cainfo) ;
        info->curl->ssl.cainfo = strdup(value);
        MEMCHECK(info->curl->ssl.cainfo);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.CAINFO: %s", info->curl->ssl.cainfo);
#endif
    }

    if(strcmp(flag,"HTTP.SSL.CAPATH")==0) {
	nullfree(info->curl->ssl.capath) ;
        info->curl->ssl.capath = strdup(value);
        MEMCHECK(info->curl->ssl.capath);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.CAPATH: %s", info->curl->ssl.capath);
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
        info->curl->ssl.verifypeer = tf;
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.SSL.VERIFYPEER: %d", info->curl->ssl.verifypeer);
#endif
    }

    if(strcmp(flag,"HTTP.NETRC")==0) {
        nullfree(info->curl->curlflags.netrc);
        info->curl->curlflags.netrc = strdup(value);
        MEMCHECK(info->curl->curlflags.netrc);
#ifdef D4DEBUG
            nclog(NCLOGNOTE,"HTTP.NETRC: %s", info->curl->curlflags.netrc);
#endif
    }

    if(strcmp(flag,"HTTP.CREDENTIALS.USERNAME")==0) {
        nullfree(info->curl->creds.user);
        info->curl->creds.user = strdup(value);
        MEMCHECK(info->curl->creds.user);
    }
    if(strcmp(flag,"HTTP.CREDENTIALS.PASSWORD")==0) {
        nullfree(info->curl->creds.pwd);
        info->curl->creds.pwd = strdup(value);
        MEMCHECK(info->curl->creds.pwd);
    }

done:
    return THROW(ret);

nomem:
    return THROW(NC_ENOMEM);
}

int
NCD4_rcprocess(NCD4INFO* info)
{
    int ret = NC_NOERR;
    char hostport[NC_MAX_PATH];
    char* url_hostport = hostport; /* WATCH OUT: points to previous variable */
    NCURI* uri = info->uri;

    assert (NCD4_globalstate != NULL);

    if(!NCD4_globalstate->rc.loaded)
	NCD4_rcload();

    /* Note, we still must do this function even if
       NCD4_globalstate->rc.ignore is set in order
       to getinfo e.g. host+port  from url
    */

    url_hostport = NULL;
    if(uri != NULL) {
        NCD4_hostport(uri,url_hostport,sizeof(hostport));
    }

    rcsetinfocurlflag(info,"HTTP.DEFLATE",
		      NCD4_rclookup("HTTP.DEFLATE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.VERBOSE",
			NCD4_rclookup("HTTP.VERBOSE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.TIMEOUT",
			NCD4_rclookup("HTTP.TIMEOUT",url_hostport));
    rcsetinfocurlflag(info,"HTTP.USERAGENT",
			NCD4_rclookup("HTTP.USERAGENT",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIEFILE",
			NCD4_rclookup("HTTP.COOKIEFILE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIE_FILE",
			NCD4_rclookup("HTTP.COOKIE_FILE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIEJAR",
			NCD4_rclookup("HTTP.COOKIEJAR",url_hostport));
    rcsetinfocurlflag(info,"HTTP.COOKIE_JAR",
			NCD4_rclookup("HTTP.COOKIE_JAR",url_hostport));
    rcsetinfocurlflag(info,"HTTP.PROXY_SERVER",
			NCD4_rclookup("HTTP.PROXY_SERVER",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.VALIDATE",
			NCD4_rclookup("HTTP.SSL.VALIDATE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.CERTIFICATE",
			NCD4_rclookup("HTTP.SSL.CERTIFICATE",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.KEY",
			NCD4_rclookup("HTTP.SSL.KEY",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.KEYPASSWORD",
			NCD4_rclookup("HTTP.SSL.KEYPASSWORD",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.CAINFO",
			NCD4_rclookup("HTTP.SSL.CAINFO",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.CAPATH",
			NCD4_rclookup("HTTP.SSL.CAPATH",url_hostport));
    rcsetinfocurlflag(info,"HTTP.SSL.VERIFYPEER",
			NCD4_rclookup("HTTP.SSL.VERIFYPEER",url_hostport));
    rcsetinfocurlflag(info,"HTTP.NETRC",
			NCD4_rclookup("HTTP.NETRC",url_hostport));
    { /* Handle various cases for user + password */
	/* First, see if the user+pwd was in the original url */
	char* user = NULL;
	char* pwd = NULL;
	if(uri->user != NULL && uri->password != NULL) {
	    user = uri->user;
	    pwd = uri->password;
	} else {
   	    user = NCD4_rclookup("HTTP.CREDENTIALS.USER",url_hostport);
	    pwd = NCD4_rclookup("HTTP.CREDENTIALS.PASSWORD",url_hostport);
	}
	if(user != NULL && pwd != NULL) {
            user = strdup(user); /* so we can consistently reclaim */
            pwd = strdup(pwd);
	} else {
	    /* Could not get user and pwd, so try USERPASSWORD */
	    const char* userpwd = NCD4_rclookup("HTTP.CREDENTIALS.USERPASSWORD",url_hostport);
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
    return THROW(ret);
}

/**
 * (Internal) Locate a triple by property key and host+port (may be null or "").
 * If duplicate keys, first takes precedence.
 */
static struct NCD4triple*
rclocate(char* key, char* hostport)
{
    int i,found;
    NClist* rc = NCD4_globalstate->rc.rc;
    NCD4triple* triple = NULL;

    if(NCD4_globalstate->rc.ignore)
	return NULL;
    if(!NCD4_globalstate->rc.loaded)
	NCD4_rcload();

    if(key == NULL || rc == NULL) return NULL;
    if(hostport == NULL) hostport = "";

    for(found=0,i=0;i<nclistlength(rc);i++) {
	triple = (NCD4triple*)nclistget(rc,i);
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
 * Locate a triple by property key and host+port (may be null|"")
 * If duplicate keys, first takes precedence.
 */
char*
NCD4_rclookup(char* key, char* hostport)
{
    struct NCD4triple* triple = rclocate(key,hostport);
    return (triple == NULL ? NULL : triple->value);
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
	NCD4triple* t = (NCD4triple*)nclistget(triples,i);
        fprintf(stderr,"\t%s\t%s\t%s\n",

                ((t->host == NULL || strlen(t->host)==0)?"--":t->host),t->key,t->value);

    }
    fflush(stderr);
}
#endif

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
    return THROW(ret);
}

void
NCD4_rcfree(NClist* rc)
{
    int i;
    for(i=0;i<nclistlength(rc);i++) {
	NCD4triple* t = (NCD4triple*)nclistget(rc,i);
	nullfree(t->host);
	nullfree(t->key);
	nullfree(t->value);
	free(t);
    }
    nclistfree(rc);
}

int
NCD4_parseproxy(NCD4INFO* info, const char* surl)
{
    int ret = NC_NOERR;
    NCURI* uri = NULL;
    if(surl == NULL || strlen(surl) == 0)
	return THROW(NC_NOERR); /* nothing there*/
    if(ncuriparse(surl,&uri) != NCU_OK)
	return THROW(NC_EURL);
    info->curl->proxy.user = uri->user;
    info->curl->proxy.pwd = uri->password;
    info->curl->proxy.host = strdup(uri->host);
    if(uri->port != NULL)
        info->curl->proxy.port = atoi(uri->port);
    else
        info->curl->proxy.port = 80;
    return THROW(ret);
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

int
NCD4_rcdefault(NCD4INFO* info)
{
    int ret = NC_NOERR;
    const char** p;
    for(p=RCDEFAULTFLAGS;*p;p+=2) {
	ret = rcsetinfocurlflag(info,p[0],p[1]);
	if(ret) {
            nclog(NCLOGERR, "RC file defaulting failed for: %s=%s",p[0],p[1]);
	}
    }
    return THROW(ret);
}
