/********************************************************************* \
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdap.h"
#include "nc3dispatch.h"
#include "ncd2dispatch.h"
#include "dapalign.h"
#include "dapdump.h"
#include "oc.h"


/*Forward*/
static NCerror getseqdimsize(NCDAPCOMMON*, CDFnode* seq, size_t* sizep);
static int fieldindex(CDFnode* parent, CDFnode* child);
static NCerror countsequence(NCDAPCOMMON*, CDFnode* node, size_t*);
static NCerror makeseqdim(NCDAPCOMMON*, CDFnode* node, size_t, CDFnode**);
static NCerror computeseqcountconstraints(NCDAPCOMMON*,CDFnode*,NCbytes*);
static void computeseqcountconstraints3r(NCDAPCOMMON*, CDFnode*, CDFnode**);



