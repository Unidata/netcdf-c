/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/** @file d4debug.h
 * @brief Debug and error-handling macros for the DAP4 client.
 *
 * Provides PANIC/ASSERT macros, the THROW/THROWCHK error-propagation
 * wrappers, conditional debug-dump flags, and logging shorthands.
 * @author Dennis Heimbigner
 */

#ifndef D4DEBUG_H
#define D4DEBUG_H

#include <assert.h>
#include <stdarg.h>

#undef D4CATCH /**< When defined, errors are caught via d4throw() for breakpoint debugging. */

#undef D4DEBUG       /**< When defined, enables all debug output. */
#undef D4DEBUGPARSER /**< When defined, enables parser debug output. */
#undef D4DEBUGMETA   /**< When defined, enables metadata-build debug output. */
#undef D4DUMPDAP     /**< When defined, dumps dechunked DAP data to stderr. */
#undef D4DUMPRAW     /**< When defined, dumps raw wire bytes to stderr. */
#undef D4DEBUGDATA   /**< When defined, enables data-processing debug output. */
#undef D4DUMPDMR     /**< When defined, dumps the DMR XML text to stderr. */
#undef D4DUMPCSUM    /**< When defined, dumps checksum computation details. */

#ifdef D4DEBUG
#define D4DEBUGPARSER
#define D4DEBUGMETA
#define D4DEBUGDATA
#define D4DUMPCSUM
#define D4DUMPDMR
#define D4DUMPDAP
#endif

/** Print @p msg to stderr and abort via assert(). */
#define PANIC(msg) assert(d4panic(msg));
/** Print a formatted message with one argument to stderr and abort. */
#define PANIC1(msg,arg) assert(d4panic(msg,arg));
/** Print a formatted message with two arguments to stderr and abort. */
#define PANIC2(msg,arg1,arg2) assert(d4panic(msg,arg1,arg2));

/** Assert @p expr; on failure print the expression text and abort. */
#define ASSERT(expr) if(!(expr)) {PANIC(#expr);} else {}

extern int ncd4debug; /**< Global debug verbosity level for the DAP4 client. */

/** Print a formatted panic message to stderr; always returns 0 (for use in assert). */
extern int d4panic(const char* fmt, ...);

/** Return NC_ENOMEM immediately if @p var is NULL. */
#define MEMCHECK(var) {if((var)==NULL) return (NC_ENOMEM);}

#ifdef D4CATCH
/** Return @p e after passing it through d4throw() for breakpoint debugging. */
#define THROW(e) d4throw(e)
/** Call d4throw(@p e) and discard the result. */
#define THROWCHK(e) (void)d4throw(e)
/** Set a debugger breakpoint here to catch errors near their origin. */
extern int d4breakpoint(int err);
/** Pass @p err through d4breakpoint() if non-zero; return @p err. */
extern int d4throw(int err);
#else
/** Return @p e unchanged (no-op when D4CATCH is not defined). */
#define THROW(e) (e)
/** No-op when D4CATCH is not defined. */
#define THROWCHK(e)
#endif

#ifdef D4DEBUG
#define SHOWFETCH (1)
#else
#define SHOWFETCH FLAGSET(nccomm->controls,NCF_SHOWFETCH)
#endif

#define LOG0(level,msg)        nclog(level,msg)        /**< Log a message with no arguments. */
#define LOG1(level,msg,a1)     nclog(level,msg,a1)     /**< Log a message with one argument. */
#define LOG2(level,msg,a1,a2)  nclog(level,msg,a1,a2)  /**< Log a message with two arguments. */

/** Return the string name of an NCD4sort value (for debug output). */
extern const char* NCD4_sortname(NCD4sort sort);
/** Return the string name of an nc_type sub-sort value (for debug output). */
extern const char* NCD4_subsortname(nc_type subsort);

/**
 * @internal Copy all top-level variable data from the DAP4 response into
 * the substrate NetCDF-4 file for debugging purposes.
 * @param info NCD4INFO for the open connection.
 * @return NC_NOERR on success, or a netCDF error code.
 */
extern int NCD4_debugcopy(NCD4INFO* info);

#endif /*D4DEBUG_H*/
