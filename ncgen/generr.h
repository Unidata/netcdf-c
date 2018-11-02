/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id: generr.h,v 1.2 2010/05/24 19:59:57 dmh Exp $ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/generr.h,v 1.2 2010/05/24 19:59:57 dmh Exp $ */

#ifndef GENERR_H
#define GENERR_H

extern int error_count;

#ifndef NO_STDARG
#define vastart(argv,fmt) va_start(argv,fmt)
#else
#define vastart(argv,fmt) va_start(argv)
#endif

#ifndef NO_STDARG
#define vaend(argv,fmt) va_end(argv)
#else
#define vaend(argv,fmt) va_end(argv)
#endif

#include <stdarg.h>

/* Report non-fatal errors */
extern void derror(const char *fmt, ...);

/* Report semantic fatal errors */
extern  void semerror(const int, const char *fmt, ...);

/* Report semantic warnings */
extern  void semwarn(const int, const char *fmt, ...);

/* Fatal Internal Error */
extern int panic(const char* fmt, ...);

/* Provide a version of snprintf that panics if the buffer is overrun*/
extern void nprintf(char* buffer, size_t size, const char *fmt, ...);

/* Internal varargs functions */
extern void vderror(const char *fmt, va_list argv);
extern void vdwarn(const char *fmt, va_list argv);

#endif /*GENERR_H*/
