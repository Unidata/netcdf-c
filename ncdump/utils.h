/*********************************************************************
 *   Copyright 2011, University Corporation for Atmospheric Research
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

#include "config.h"

extern char *progname;		/* for error messages */

#ifndef NO_NETCDF_2
#define NO_NETCDF_2		/* assert we aren't using any netcdf-2 stuff */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK(stat,f) if(stat != NC_NOERR) {check(stat,#f,__FILE__,__LINE__);} else {}

/* Print error message to stderr and exit */
extern void	error ( const char *fmt, ... );

/* Check error on malloc and exit with message if out of memory */
extern void*    emalloc ( size_t size );

/* Check error return.  If bad, print error message and exit. */
extern void check(int err, const char* fcn, const char* file, const int line);

/* Return malloced name with chars special to CDL escaped. */
char* escaped_name(const char* cp);

/* Print name of netCDF var, dim, att, group, type, member, or enum
 * symbol with escaped special chars */
void print_name(const char *name);

#ifdef __cplusplus
}
#endif
