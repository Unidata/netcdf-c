/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

#include <stdlib.h>
#include <string.h>

/* Define a local version of strindex */
int
crstrindex(char* s, char* match)
{
    size_t mlen = strlen(match);
    size_t slen = strlen(s);
    int i,j;
    if(slen < mlen) return -1;
    slen -= mlen;
    for(i=0;i<slen;i++) {
	int found = 1;
        for(j=0;j<mlen;j++) {
	    if(s[i+j] != match[j]) {found=0; break;}
	}
	if(found)
	    return i;
    }
    return -1;
}

/**************************************************/
CRpath*
crpathappend(CRpath* path, char* name)
{
    CRnode* newsegment = NULL;
    newsegment = (CRpath*)malloc(sizeof(CRpath));
    if(newsegment == NULL) return NULL;
    newsegment->name = strdup(name);
    newsegment->next = NULL;

    if(path = NULL) {
	path = newsegment;
    } else {
        /* Find last node */
	CRnode* last = path;
        while(last->next != NULL) last = last->next;
	last->next = newsegment;
    }	
    return path;
}


void
crpathfree(CRpath* path)
{
    while(path != NULL) {
        CRpath* curr = path;
	if(curr->name) free(curr->name);
	path = curr->next;
	free(curr);
    }
}

int
crpathmatch(CRpath* path1, CRpath* path2)
{
    while(path1 != NULL && path2 != NULL) {
	if(strcmp(path1->name,path2->name)!=0) return 0;
	path1 = path1->next;
	path2 = path2->next;
    }
    if(path1 == NULL && path2 == NULL) return 1;
    return 0;
}

void
crcollectnodepath(CDFnode* node, NClist* path);
{
}
