/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id $
 *********************************************************************/
#ifndef _CHUNKSPEC_H_
#define _CHUNKSPEC_H_

/* Parse chunkspec string and convert into chunkspec_t structure */
extern int
chunkspec_parse(int ncid, const char *spec);

/* Return size in chunkspec string specified for dimension corresponding to dimid, 0 if not found */
extern size_t
chunkspec_size(int dimid);

#endif	/* _CHUNKSPEC_H_  */
