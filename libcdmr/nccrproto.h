/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCRPROTO_H
#define NCCRPROTO_H

/*Forward*/
struct NClist;

extern int nccr_cvtasterr(ast_err err);
extern int nccr_decodeheader(bytes_t* packet, Header** hdrp);
extern int nccr_walk_Header(Header* node, struct NClist* nodes);
extern int nccr_compute_pathnames(struct NClist* nodes);
extern int nccr_map_dimensions(struct NClist* nodes);
extern void nccr_deref_dimensions(struct NClist* nodes);

extern int nccr_decodedataheader(bytes_t* buf, Data** datahdrp);
extern int nccr_mapdataheader(NCCDMR*, Header*, Data*);

#endif /*NCCRPROTO_H*/
