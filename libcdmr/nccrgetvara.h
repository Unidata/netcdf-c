/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/getvara.h,v 1.11 2010/05/27 21:34:08 dmh Exp $
 *********************************************************************/
#ifndef NCCRGETVARA_H
#define NCCRGETVARA_H

typedef struct NCCRgetvara {
    void* memory; /* where result is put*/
    struct CRprojection* projection;
    nc_type dsttype;
    CRnode* target;
    CRnode* target0;
} NCCRgetvara;

#endif /*NCCRGETVARA_H*/
