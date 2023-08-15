/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/

#ifndef NCLOG_H
#define NCLOG_H

#include <stdarg.h>
#include "ncexternl.h"

#define NCCATCH

#define NCENVLOGGING "NCLOGGING"
#define NCENVTRACING "NCTRACING"

/* Suggested tag values */
#define NCLOGNOTE 0
#define NCLOGWARN 1
#define NCLOGERR 2
#define NCLOGDBG 3

/* Support ptr valued arguments that are used to store results */
#define PTRVAL(t,p,d) ((t)((p) == NULL ? (d) : *(p)))

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
extern "C" {
#endif

EXTERNL void ncloginit(void);
EXTERNL int ncsetlogging(int tf);
EXTERNL int nclogopen(FILE* stream);

/* The tag value is an arbitrary integer */
EXTERNL void nclog(int tag, const char* fmt, ...);
EXTERNL int ncvlog(int tag, const char* fmt, va_list ap);
EXTERNL void nclogtext(int tag, const char* text);
EXTERNL void nclogtextn(int tag, const char* text, size_t count);

/* Trace support */
EXTERNL int nctracelevel(int level);
EXTERNL void nctrace(int level, const char* fcn, const char* fmt, ...);
EXTERNL void nctracemore(int level, const char* fmt, ...);
EXTERNL void ncvtrace(int level, const char* fcn, const char* fmt, va_list ap);
EXTERNL int ncuntrace(const char* fcn, int err,const char* fmt,...);
EXTERNL int ncthrow(int err,const char* file,int line);
EXTERNL int ncbreakpoint(int err);

/* Debug support */
#if defined(NCCATCH)
#define NCTHROW(e) ((e) == NC_NOERR ? (e) : ncthrow(e,__FILE__,__LINE__))
#else
#define NCTHROW(e) (e)
#endif

#ifdef HAVE_EXECINFO_H
EXTERNL void ncbacktrace(void);
#else
#define ncbacktrace()
#endif

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
}
#endif

#endif /*NCLOG_H*/
