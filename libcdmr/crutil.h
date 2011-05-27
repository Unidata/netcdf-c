/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef CRUTIL_H
#define CRUTIL_H

extern int crstrindex(char* s, char* match);

extern void crcollectnodepath(CRnode*, NClist* path);

/* In order to allow greater lattitude in
   what characters are allowed in names,
   we avoid stringifying the pathname; instead
   we use a linked list of names.
*/

/* Forward */
typedef struct CRpath {
    char* name; /* this segment of the full path */
    struct CRpath* next;
} CRpath;

extern CRpath* crpathappend(CRpath*,char*);
extern void crpathfree(CRpath*);
extern int crpathmatch(CRpath*,CRpath*);


#endif /*CRUTIL_H*/
