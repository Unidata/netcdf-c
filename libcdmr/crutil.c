/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

/* Search for existence of matching param */

int
nccrparamcheck(NC_URL* ncurl, const char* name)
{
    int i;
    if(ncurl == NULL || name == NULL) return 0;
    if(ncurl->paramlist == NULL) return 0;
    for(i=0;i<nclistlength(ncurl->paramlist;i++) {
	char* param = (char*)nclistget(ncurl->paramlist);
	if(param == NULL) continue;
	if(strcmp(name,param) == 0) return 1;
    }
    return 0;
}
