/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#ifndef NCRC_H
#define NCRC_H

#include "ncbytes.h"
#include "nclist.h"
#include "ncuri.h"

typedef struct NCTriple {
	char* host; /* combined host:port */
        char* key;
        char* value;
} NCTriple;

typedef struct NCRCinfo {
    struct curlflags {
        int proto_file; /* Is file: supported? */
        int proto_https; /* is https: supported? */
	int compress; /*CURLOPT_ENCODING*/
	int verbose; /*CURLOPT_ENCODING*/
	int timeout; /*CURLOPT_TIMEOUT*/
	int maxredirs; /*CURLOPT_MAXREDIRS*/
	char* useragent; /*CURLOPT_USERAGENT*/
	/* track which of these are created by oc */
#define COOKIECREATED 1
#define NETRCCREATED 2
	int createdflags;
	char* cookiejar; /*CURLOPT_COOKIEJAR,CURLOPT_COOKIEFILE*/
	char* netrc; /*CURLOPT_NETRC,CURLOPT_NETRC_FILE*/
    } curlflags;
    struct ssl {
	int   verifypeer; /* CURLOPT_SSL_VERIFYPEER;
                             do not do this when cert might be self-signed
                             or temporarily incorrect */
	int   verifyhost; /* CURLOPT_SSL_VERIFYHOST; for client-side verification */
        char* certificate; /*CURLOPT_SSLCERT*/
	char* key; /*CURLOPT_SSLKEY*/
	char* keypasswd; /*CURLOPT_SSLKEYPASSWD*/
        char* cainfo; /* CURLOPT_CAINFO; certificate authority */
	char* capath;  /*CURLOPT_CAPATH*/
    } ssl;
    struct proxy {
	char *host; /*CURLOPT_PROXY*/
	int port; /*CURLOPT_PROXYPORT*/
	char* user; /*CURLOPT_PROXYUSERNAME*/
	char* pwd; /*CURLOPT_PROXYPASSWORD*/
    } proxy;
    struct credentials {
	char *user; /*CURLOPT_USERNAME*/
	char *pwd; /*CURLOPT_PASSWORD*/
    } creds;
} NCRCinfo;

/* Collect global state info in one place */
typedef struct NCRCglobalstate {
    int initialized;
    char* tempdir; /* track a usable temp dir */
    char* home; /* track $HOME for use in creating $HOME/.oc dir */
    struct {
        int proto_file;
        int proto_https;
    } curl;
    struct {
	int ignore; /* if 1, then do not use any rc file */
	int loaded;
        NClist* triples; /* the rc file triple store fields*/
        char* rcfile; /* specified rcfile; overrides anything else */
    } rc;
} NCRCglobalstate;

extern NCRCglobalstate ncrc_globalstate;

/* From drc.c */
/* read and compile the rc file, if any */
extern int NC_rcload(void);
extern int NC_rcprocess(NCRCinfo* info, NCURI* uri);
extern char* NC_rclookup(const char* key, const char* hostport);
extern void NC_rcfreetriples(NClist* rc);
extern void NC_rcclear(NCRCinfo* info);

/* From dutil.c */
extern int NC__testurl(const char* path, char** basenamep);
extern int NC_isLittleEndian(void);
extern char* NC_backslashEscape(const char* s);
extern char* NC_backslashUnescape(const char* esc);
extern char* NC_entityescape(const char* s);
extern int NC_readfile(const char* filename, NCbytes* content);
extern int NC_mktmp(const char* base, char** tmpnamep);
extern char* NC_combinehostport(const char* host, const char* port);

#endif /*NCRC_H*/
