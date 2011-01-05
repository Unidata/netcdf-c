/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id $
 *********************************************************************/
#ifndef _CHUNKSPEC_H_
#define _CHUNKSPEC_H_

typedef struct {
    size_t ndims;		/* number of dimensions in chunkspec string */
    int *dimids;		/* ids for dimensions in chunkspec string */
    size_t *chunksizes;		/* corresponding chunk sizes */
} chunkspec_t;

/* Parse chunkspec string and convert into chunkspec_t structure */
extern int
chunkspec_parse(int ncid, const char *spec, chunkspec_t *csp);

/* Free chunkspec_t structure */
extern void
chunkspec_free(chunkspec_t *csp);

/* Missing functionality that should be in nc_inq_dimid(), to get
 * dimid from a full dimension path name that may include group
 * names */
extern int 
nc_inq_dimid2(int ncid, const char *dimname, int *dimidp);

#endif	/* _CHUNKSPEC_H_  */
