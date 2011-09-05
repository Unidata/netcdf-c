/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

#include "rpc_includes.h"

NCPath*
ncpath_append(NCPath* path, char* name)
{
    NCPath* newsegment = NULL;
    newsegment = (NCPath*)malloc(sizeof(NCPath));
    if(newsegment == NULL) return NULL;
    newsegment->name = strdup(name);
    newsegment->next = NULL;

    if(path == NULL) {
	path = newsegment;
    } else {
        /* Find last node */
	NCPath* last = path;
        while(last->next != NULL) last = last->next;
	last->next = newsegment;
    }	
    return path;
}


void
ncpath_free(NCPath* path)
{
    while(path != NULL) {
        NCPath* curr = path;
	if(curr->name) free(curr->name);
	path = curr->next;
	free(curr);
    }
}

int
ncpath_match(NCPath* path1, NCPath* path2)
{
    while(path1 != NULL && path2 != NULL) {
	if(strcmp(path1->name,path2->name)!=0) return 0;
	path1 = path1->next;
	path2 = path2->next;
    }
    if(path1 == NULL && path2 == NULL) return 1;
    return 0;
} 

NCPath*
ncpath_dup(NCPath* path)
{
    NCPath* newpath = NULL;
    while(path != NULL) {
	ncpath_append(newpath,path->name);
	path = path->next;
    }
    return newpath;
}

char*
ncpath_tostring(NCPath* path, char* sep, char*(encode)(char*,NCBytes*))
{
    NCBytes* buf;
    char* result;

    if(path == NULL) return NULL;
    if(sep == NULL) sep = ".";
    buf = ncbytesnew();
    int first = 1;
    while(path != NULL) {
	if(!first) ncbytescat(buf,path);
	first = 0;
	if(encode != NULL)
	    encode(path->name,buf);	    
	else
	    ncbytescat(buf,path->name);
	path = path->next;
    }
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

