/*
 * Copyright 2010 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * This header file is for the parallel I/O functions of netCDF.
 *
 */
/* "$Id: netcdf_par.h,v 1.1 2010/06/01 15:46:49 ed Exp $" */

#ifndef NCCONFIGURE_H
#define NCCONFIGURE_H 1

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/*
This is included in bottom
of config.h. It is where, 
typically, alternatives to
missing functions should be
defined and missing types defined.
*/

/*
#ifndef HAVE_SSIZE_T
typedef long ssize_t;
#define HAVE_SSIZE_T
#endif
*/

#ifdef _MSC_VER
#ifndef HAVE_SSIZE_T
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#define HAVE_SSIZE_T 1
#endif
#endif

/*Warning: Cygwin with -ansi does not define these functions
  in its headers.*/
#ifndef _WIN32
#if __STDC__ == 1 /*supposed to be same as -ansi flag */
extern char* strdup(const char*);
extern size_t strlcat(char*,const char*,size_t);
extern int snprintf(char*, size_t, const char*, ...); 
extern int strcasecmp(const char*, const char*);
extern long long int strtoll(const char*, char**, int);
extern unsigned long long int strtoull(const char*, char**, int);
#endif
#endif

#ifdef _WIN32
#define strlcat(d,s,n) strcat_s((d),(n),(s))
#endif

/* handle null arguments */
#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nulllen
#define nulllen(s) ((s)==NULL?0:strlen(s))
#endif
#ifndef nullfree
#define nullfree(s) {if((s)!=NULL) {free(s);} else {}}
#endif

#ifndef HAVE_UCHAR
typedef unsigned char uchar;
#endif

#ifndef HAVE_LONGLONG
typedef long long longlong;
typedef unsigned long long ulonglong;
#endif

#ifndef HAVE_USHORT
typedef unsigned short ushort;
#endif

#ifndef HAVE_UINT
typedef unsigned int uint;
#endif

#endif /* NCCONFIGURE_H */
