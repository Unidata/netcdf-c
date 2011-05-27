/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCR_H
#define NCCR_H

/**************************************************/
/*Forwards*/
struct NC;
struct NC_URI;
struct NClist;
struct Data;

/**************************************************/
/* The NCCR structure is subtype of NC_INFO_TYPE_T (libsrc4) */

typedef struct NCCDMR {
    struct NC*   controller; /* Parent instance of NCDAP3 or NCDAP4 */
    char* urltext; /* as given to open()*/
    struct NC_URI* uri;
    /* Track some flags */
    int controls;
    struct CCEconstraint* urlconstraint; /* constraint from url */
    struct Header* ncstreamhdr; /* Parsed result */
    struct Data* datahdr; /* Parsed result */
    struct NClist* variables; /* set of visible variables */
    /* provide a collection of the  ncStream nodes*/
    struct NClist* streamnodes;
    /* Store curl state info */
    struct NCCURLSTATE {
        CURL* curl;
        int curlflags;
	int compress;
	int verbose;
	int followlocation;
	int maxredirs;
	char *host;
	int port;
	char *username;
	char *password;
	char* useragent;
	char* cookiejar;
	char* cookiefile;
	int   validate;
        char* certificate;
	char* key;
	char* keypasswd;
        char* cainfo; /* certificate authority */
	char* capath; 
    } curl;
} NCCDMR;

typedef struct NCCURLSTATE NCCURLSTATE;

typedef struct NCCR {
    NC_FILE_INFO_T info;
    NCCDMR*	   cdmr;
} NCCR;

/**************************************************/
/* Define various flags (powers of 2)*/
#define SHOWFETCH (0x1)


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

/**********************************************************/
/* Forwards */
struct Header;
struct NClist;

extern int nccrceparse(char*, int, struct NClist**, struct NClist**, char**);

extern int crbuildnc(NCCR*, struct Header*);

/**********************************************************/
#endif /*NCCR_H*/
