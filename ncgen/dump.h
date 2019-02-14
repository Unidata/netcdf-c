/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/

#ifndef NCGEN_DUMP_H
#define NCGEN_DUMP_H

/*#define F*/

/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/dump.h,v 1.2 2010/04/04 19:39:44 dmh Exp $
 *********************************************************************/

extern void dumptransform(Datalist*);
extern void dumpdatalist(Datalist*,char*);
extern void dumpconstant(NCConstant*,char*);
extern void bufdump(Datalist*,Bytebuffer*);
extern void dumpgroup(Symbol* g);

#endif
