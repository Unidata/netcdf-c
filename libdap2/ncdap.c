/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdap.h"

#ifdef HAVE_GETRLIMIT
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/time.h>
#  endif
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/resource.h>
#  endif
#endif

#include "oclog.h"

#include "nc3internal.h"
#include "nc3dispatch.h"
#include "ncd2dispatch.h"
#include "dapalign.h"
#include "dapdump.h"

static NCerror buildncstructures(NCDAPCOMMON*);
static NCerror builddims(NCDAPCOMMON*);
static NCerror buildvars(NCDAPCOMMON*);
static NCerror buildglobalattrs(NCDAPCOMMON*,CDFnode* root);


static char* getdefinename(CDFnode* node);

static int ncd2initialized = 0;


/**************************************************/
/* Add an extra function whose sole purpose is to allow
   configure(.ac) to test for the presence of thiscode.
*/
int nc__opendap(void) {return 0;}


