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
#include "nchashmap.h"

#include "nc4internal.h"
#include "nc.h"
#include "netcdf.h"

/**************************************************/
/* The NCCR structure is subtype of NC_INFO_TYPE_T (libsrc4) */

typedef struct NCCDMR {
    NC*   controller; /* Parent instance of NCDAP3 or NCDAP4 */
} NCCDMR

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
