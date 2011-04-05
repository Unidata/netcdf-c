/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCRPROTO_H
#define NCCRPROTO_H

extern int nccr_cvtasterr(ast_err err);
extern int nccr_decodeheader(bytes_t* packet, size_t offset, Header** hdrp);
extern int nccr_walk_Header(Header* node, nclist* nodes, nchashmap* map);

#endif /*NCCRPROTO_H*/
