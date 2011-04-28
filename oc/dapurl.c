/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "oclist.h"
#include "ocinternal.h"
#include "occlientparams.h"
#include "ocdebug.h"

#define LBRACKET '['
#define RBRACKET ']'

static char* legalprotocols[] = {
"file:",
"http:",
"https:",
"ftp:",
NULL /* NULL terminate*/
};


/* Do a simple url parse*/
int
dapurlparse(const char* url0, DAPURL* dapurl)
{
    char* url;
    char** pp;
    char* p;
    char* p1;
    int c;

    /* accumulate parse points*/
    char* protocol = NULL;
    char* params = NULL;
    char* baseurl = NULL;
    char* constraint = NULL;
    char* stop;

    memset((void*)dapurl,0,sizeof(DAPURL));

    /* copy url and remove all whitespace*/
    url = strdup(url0);

    p = url;
    p1 = url;
    while((c=*p1++)) {if(c != ' ' && c != '\t') *p++ = c;}

    p = url;
    stop = p + strlen(p);

    /* break up the url string into pieces*/
    if(*p == LBRACKET) {
	params = p+1;
	/* find end of the clientparams*/
        for(;*p;p++) {if(p[0] == RBRACKET && p[1] != LBRACKET) break;}
	if(*p == 0) goto fail; /* malformed client params*/
	*p = '\0'; /* leave off the trailing rbracket for now */
	p++; /* move past the params*/
    }

    /* verify that the url starts with an acceptable protocol*/
    for(pp=legalprotocols;*pp;pp++) {
        if(strncmp(p,*pp,strlen(*pp))==0) break;
    }
    if(*pp == NULL) goto fail; /* illegal protocol*/
    /* save the protocol */
    protocol = *pp;

    baseurl = p;

    /* Look for '?' */
    constraint = strchr(p,'?');
    if(constraint) {
	*constraint++ = '\0';
    }

    /* assemble the component pieces*/
    dapurl->url = nulldup(url0);
    dapurl->base = nulldup(baseurl);
    dapurl->protocol = nulldup(protocol);
    /* remove trailing ':' */
    dapurl->protocol[strlen(protocol)-1] = '\0';
    dapurl->constraint = nulldup(constraint);
    dapurlsetconstraints(dapurl,constraint);
    if(params != NULL) {
        dapurl->params = (char*)ocmalloc(1+2+strlen(params));
        strcpy(dapurl->params,"[");
        strcat(dapurl->params,params);
        strcat(dapurl->params,"]");
    }

    if(ocdebug > 0) {
        fprintf(stderr,"dapurl: params=|%s| base=|%s| projection=|%s| selection=|%s|\n",
		dapurl->params, dapurl->base, dapurl->projection, dapurl->selection);

    }
    free(url);
    return 1;

fail:
    if(url != NULL) free(url);
    return 0;
}

/* Call must free the actual url instance.*/
void
dapurlclear(DAPURL* dapurl)
{
    if(dapurl->url != NULL) {free(dapurl->url);}
    if(dapurl->base != NULL) {free(dapurl->base);}
    if(dapurl->protocol != NULL) {free(dapurl->protocol);}
    if(dapurl->constraint != NULL) {free(dapurl->constraint);}
    if(dapurl->projection != NULL) {free(dapurl->projection);}
    if(dapurl->selection != NULL) {free(dapurl->selection);}
    if(dapurl->params != NULL) {free(dapurl->params);}
    if(dapurl->paramlist != NULL) ocparamfree(dapurl->paramlist);
    memset((void*)dapurl,0,sizeof(DAPURL));
}

/* Replace the constraints */
void
dapurlsetconstraints(DAPURL* durl,const char* constraints)
{
    char* proj = NULL;
    char* select = NULL;
    const char* p;

    if(durl->constraint == NULL) ocfree(durl->constraint);
    if(durl->projection != NULL) ocfree(durl->projection);
    if(durl->selection != NULL) ocfree(durl->selection);
    durl->constraint = NULL;	
    durl->projection = NULL;	
    durl->selection = NULL;

    if(constraints == NULL || strlen(constraints)==0) return;

    durl->constraint = strdup(constraints);
    if(*durl->constraint == '?')
	strcpy(durl->constraint,durl->constraint+1);

    p = durl->constraint;
    proj = (char*) p;
    select = strchr(proj,'&');
    if(select != NULL) {
        size_t plen = (select - proj);
	if(plen == 0) {
	    proj = NULL;
	} else {
	    proj = (char*)ocmalloc(plen+1);
	    memcpy((void*)proj,p,plen);
	    proj[plen] = '\0';
	}
	select = nulldup(select);
    } else {
	proj = nulldup(proj);
	select = NULL;
    }
    durl->projection = proj;
    durl->selection = select;
}


/* Construct a complete DAP URL without the client params
   and optionally with the constraints;
   caller frees returned string
*/

char*
dapurlgeturl(DAPURL* durl, const char* prefix, const char* suffix,
             int withconstraints, int withparams)
{
    size_t len = 0;
    char* newurl;

    len += strlen(durl->base);
    if(prefix != NULL) len += strlen(prefix);
    if(suffix != NULL) len += strlen(suffix);
    if(withparams && durl->params != NULL && strlen(durl->params) > 0)
	len += strlen(durl->params);
    if(withconstraints
	&& durl->constraint != NULL  && strlen(durl->constraint)> 0) {
	len += 1; /* leading '?' */
	len += strlen(durl->constraint);
    }
    len += 1; /* null terminator */
    
    newurl = (char*)ocmalloc(len);
    if(!newurl) return NULL;
    newurl[0] = '\0';
    if(prefix != NULL) strcat(newurl,prefix);
    if(withparams && durl->params != NULL && strlen(durl->params) > 0)
	strcat(newurl,durl->params);
    strcat(newurl,durl->base);
    if(suffix != NULL) strcat(newurl,suffix);
    if(withconstraints
	&& durl->constraint != NULL  && strlen(durl->constraint)> 0) {
	strcat(newurl,"?");
	strcat(newurl,durl->constraint);
    }
    return newurl;
}

int
dapurldecodeparams(DAPURL* dapurl)
{
    int ok = 0;
    if(dapurl->paramlist == NULL && dapurl->params != NULL) {
	OClist* list = ocparamdecode(dapurl->params);
	dapurl->paramlist = list;
	ok = 1;
    }
    return ok;
}

/*! NULL result => entry not found.
    Empty value should be represented as a zero length string */
const char*
dapurllookup(DAPURL* durl, const char* clientparam)
{
    /* make sure that durl->paramlist exists */
    if(durl->paramlist == NULL) dapurldecodeparams(durl);
    return ocparamlookup(durl->paramlist,clientparam);    
}

