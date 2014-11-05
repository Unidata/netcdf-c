/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdap.h"

#include "dapdump.h"

extern CDFnode* v4node;

static NCerror buildcdftreer(NCDAPCOMMON*,OCddsnode,CDFnode*,CDFtree*,CDFnode**);
static void defdimensions(OCddsnode, CDFnode*, NCDAPCOMMON*, CDFtree*);
static NCerror  attachsubsetr(CDFnode*, CDFnode*);
static void free1cdfnode(CDFnode* node);

/* Define Procedures that are common to both
   libncdap3 and libncdap4
*/

