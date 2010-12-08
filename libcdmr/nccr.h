#ifndef NCCR_H
#define NCCR_H

#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "ncbytes.h"
#include "nclist.h"

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nc.h"

#include "crdebug.h"
#include "crutil.h"

/* Forward */
struct Curl;

/**************************************************/
/* The NCCR structure is subtype of NC_INFO_TYPE_T (libsrc4) */

typedef struct NCCDMR {
    NC*   controller; /* Parent instance of NCDAP3 or NCDAP4 */
    char* urltext; /* as given to open()*/
    NC_URL* url;
    /* Store curl state  info */
    struct NCCURLSTATE {
        struct Curl* curl;
        int curlflags;
	int compress;
	int verbose;
	int followlocation;
	int maxredirs;
	char* useragent;
	char* cookiejar;
	char* cookiefile;
	int   validate;
        char* certificate;
	char* key;
	char* keypasswd;
        char* cainfo; /* certificate authority */
	char* capath; 
	char *host;
	int port;
	char *username;
	char *password;
    } curl;
} NCCDMR;

typedef struct NCCURLSTATE NCCURLSTATE;

typedef struct NCCR {
    NC_FILE_INFO_T info;
    NCCDMR*	   cdmr;
} NCCR;

/**************************************************/
/**************************************************/
/* Give PSEUDOFILE a value */
#define PSEUDOFILE "/tmp/pseudofileXXXXXX"

/* Replacement for strdup (in libsrc) */
#ifdef HAVE_STRDUP
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#else
extern char* nulldup(const char*);
#endif

#define nulllen(s) (s==NULL?0:strlen(s))
#define nullstring(s) (s==NULL?"(null)":s)

/**************************************************/

/**********************************************************/

extern int nccrceparse(char*, int, NClist**, NClist**, char**);

/**********************************************************/

#endif /*NCCR_H*/
