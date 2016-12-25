/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdispatch.h"
#include "ncd4dispatch.h"
#include "d4includes.h"
#include "d4read.h"
#include "d4curlfunctions.h"

/**************************************************/
/* Forward */

static void applyclientparamcontrols(NCD4INFO*);
static int constrainable(NCURI*);
static void freeCurl(NCD4curl*);
static void freeInfo(NCD4INFO*);
static int paramcheck(NCD4INFO*, const char* key, const char* subkey);
static int set_curl_properties(NCD4INFO*);

/**************************************************/
/* Constants */

static const char* checkseps = "+,:;";

/**************************************************/
int
NCD4_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC* nc)
{
    int ret = NC_NOERR;
    NCD4INFO* d4info = NULL;
    const char* value;

    if(path == NULL)
	return THROW(NC_EDAPURL);

    assert(dispatch != NULL);

    /* Setup our NC and NCDAPCOMMON state*/

    d4info = (NCD4INFO*)calloc(1,sizeof(NCD4INFO));
    if(d4info == NULL) {ret = NC_ENOMEM; goto done;}

    nc->dispatchdata = d4info;
    nc->int_ncid = nc__pseudofd(); /* create a unique id */
    d4info->controller = (NC*)nc;

    /* Parse url and params */
    if(ncuriparse(nc->path,&d4info->uri) != NCU_OK)
	{ret = NC_EDAPURL; goto done;}

    if(!constrainable(d4info->uri))
	SETFLAG(d4info->controls.flags,NCF_UNCONSTRAINABLE);

    /* fail if we are unconstrainable but have constraints */
    if(FLAGSET(d4info->controls.flags,NCF_UNCONSTRAINABLE)) {
	if(d4info->uri->query != NULL) {
	    nclog(NCLOGWARN,"Attempt to constrain an unconstrainable data source: %s",
		   d4info->uri->query);
	    ret = THROW(NC_EDAPCONSTRAINT);
	    goto done;
	}
    }

    /* Use libsrc4 code (netcdf-4) for storing metadata and data*/
    {
	char tmpname[32];

        /* Create fake file name: exact name must be unique,
           but is otherwise irrelevant because we are using NC_DISKLESS
        */
        snprintf(tmpname,sizeof(tmpname),"%d",nc->int_ncid);

        /* Now, use the file to create the hidden, in-memory netcdf file.
	   We want this hidden file to always be NC_NETCDF4, so we need to
           force default format temporarily in case user changed it.
	*/
	{
	    int new = NC_NETCDF4;
	    int old = 0;
	    nc_set_default_format(new,&old); /* save and change */
            ret = nc_create(tmpname,NC_DISKLESS|NC_NETCDF4,&d4info->nc4id);
	    nc_set_default_format(old,&new); /* restore */
	}
        if(ret != NC_NOERR) goto done;
	/* Avoid fill */
	nc_set_fill(d4info->nc4id,NC_NOFILL,NULL);

    }

    /* process control client parameters */
    applyclientparamcontrols(d4info);

    /* Turn on logging; only do this after oc_open*/
    if((value = ncurilookup(d4info->uri,"log")) != NULL) {
	ncloginit();
        if(nclogopen(value))
	    ncsetlogging(1);
	ncloginit();
        if(nclogopen(value))
	    ncsetlogging(1);
    }

    /* Setup a curl connection */
    {
        CURL* curl = NULL; /* curl handle*/
	d4info->curl = (NCD4curl*)calloc(1,sizeof(NCD4curl));
	if(d4info->curl == NULL)
	    {ret = NC_ENOMEM; goto done;}
	/* create the connection */
        if((ret=NCD4_curlopen(&curl))!= NC_NOERR) goto done;
	d4info->curl->curl = curl;
        if((ret=set_curl_properties(d4info))!= NC_NOERR) goto done;	
        /* Set the one-time curl flags */
        if((ret=NCD4_set_flags_perlink(d4info))!= NC_NOERR) goto done;
#if 1 /* temporarily make per-link */
        if((ret=NCD4_set_flags_perfetch(d4info))!= NC_NOERR) goto done;
#endif
    }

    d4info->curl->packet = ncbytesnew();
    ncbytessetalloc(d4info->curl->packet,DFALTPACKETSIZE); /*initial reasonable size*/

    /* fetch the dmr + data*/
    {
	int inmem = FLAGSET(d4info->controls.flags,NCF_ONDISK) ? 0 : 1;
        if((ret = NCD4_readDAP(d4info,inmem))) goto done;
    }

    /* if the url goes astray to a random web page, then try to just dump it */
    {
	char* response = ncbytescontents(d4info->curl->packet);
	size_t responselen = ncbyteslength(d4info->curl->packet);

        /* Apply some heuristics to see what we have.
The leading byte will have the chunk flags, which should
be less than 0x0f (for now). However, it will not be zero if
the data was little-endian
	*/
        if(responselen == 0 || response[0] >= ' ') {
	    /* does not look like a chunk, so probable server failure */
	    if(responselen == 0)
	        nclog(NCLOGERR,"Empty DAP4 response");
	    else {/* probable html response */
		nclog(NCLOGERR,"Unexpected DAP response:");
		nclog(NCLOGERR,"==============================");
		nclogtext(NCLOGERR,response);
		nclog(NCLOGERR,"==============================\n");
	    }
	    ret = NC_EDAPSVC;
  	    fflush(stderr);
	    goto done;
	}
    }

    /* Build the meta data */
    if((d4info->substrate.metadata=NCD4_newmeta(NCD4_CSUM_ALL,
	ncbyteslength(d4info->curl->packet),
        ncbytescontents(d4info->curl->packet)))==NULL)
	{ret = NC_ENOMEM; goto done;}
    d4info->substrate.metadata->controller = d4info;
    d4info->substrate.metadata->ncid = d4info->nc4id; /* Transfer netcdf ncid */

    if(NCD4_isdmr(d4info->substrate.metadata->serial.rawdata)) {
	char* dmr = (char*)d4info->substrate.metadata->serial.rawdata;
	NCD4_setdmr(d4info->substrate.metadata,dmr);
    } else {
	if((ret=NCD4_dechunk(d4info->substrate.metadata))) goto done;
    }

#ifdef D4DUMPDMR
{
    fprintf(stderr,"=============\n");
    fputs(d4info->substrate.metadata->serial.dmr,stderr);
    fprintf(stderr,"\n=============");
    fflush(stderr);
}
#endif

    if((ret = NCD4_parse(d4info->substrate.metadata))) goto done;
#ifdef D4DEBUGMETA
{
    fprintf(stderr,"/////////////\n");
    NCbytes* buf = ncbytesnew();
    NCD4_print(d4info->substrate.metadata,buf);
    ncbytesnull(buf);
    fputs(ncbytescontents(buf),stderr);
    ncbytesfree(buf);
    fprintf(stderr,"/////////////\n");
    fflush(stderr);
}
#endif
    if((ret = NCD4_metabuild(d4info->substrate.metadata,d4info->substrate.metadata->ncid))) goto done;
    ret = nc_enddef(d4info->substrate.metadata->ncid);
    if(ret != NC_NOERR && ret != NC_EVARSIZE) goto done;
    if((ret = NCD4_databuild(d4info->substrate.metadata))) goto done;

    return THROW(ret);

done:
    if(ret) {
        if(d4info != NULL) NCD4_close(d4info->controller->ext_ncid);
    }
    return THROW(ret);
}

int
NCD4_close(int ncid)
{
    int ret = NC_NOERR;
    NC* nc;
    NCD4INFO* d4info;

    ret = NC_check_id(ncid, (NC**)&nc);
    if(ret != NC_NOERR) goto done;
    d4info = (NCD4INFO*)nc->dispatchdata;

    /* We call abort rather than close to avoid
       trying to write anything or try to pad file length
     */
    ret = nc_abort(getnc4id(nc));

    freeInfo(d4info);

done:
    return THROW(ret);
}

int
NCD4_abort(int ncid)
{
    return NCD4_close(ncid);
}

static void
freeInfo(NCD4INFO* d4info)
{
    if(d4info == NULL) return;
    d4info->controller = NULL; /* break link */
    nullfree(d4info->rawurltext);
    nullfree(d4info->urltext);
    ncurifree(d4info->uri);
    freeCurl(d4info->curl);
    nullfree(d4info->data.memory);
    nullfree(d4info->data.ondiskfilename);
    if(d4info->data.ondiskfile != NULL)
	fclose(d4info->data.ondiskfile);
    nullfree(d4info->substrate.filename);
    NCD4_reclaimMeta(d4info->substrate.metadata);
    free(d4info);    
}

static void
freeCurl(NCD4curl* curl)
{
    if(curl == NULL) return;
    NCD4_curlclose(curl->curl);
    ncbytesfree(curl->packet);
    nullfree(curl->errdata.code);
    nullfree(curl->errdata.message);
    nullfree(curl->curlflags.useragent);
    nullfree(curl->curlflags.cookiejar);
    nullfree(curl->curlflags.netrc);
    nullfree(curl->ssl.certificate);
    nullfree(curl->ssl.key);
    nullfree(curl->ssl.keypasswd);
    nullfree(curl->ssl.cainfo);
    nullfree(curl->ssl.capath);
    nullfree(curl->proxy.host);
    nullfree(curl->proxy.userpwd);
    nullfree(curl->creds.userpwd);
}

/* Define the set of protocols known to be constrainable */
static char* constrainableprotocols[] = {"http", "https",NULL};

static int
constrainable(NCURI* durl)
{
   char** protocol = constrainableprotocols;
   for(;*protocol;protocol++) {
	if(strcmp(durl->protocol,*protocol)==0)
	    return 1;
   }
   return 0;
}

/*
Set curl properties for link based on rc files etc.
*/
static int
set_curl_properties(NCD4INFO* d4info)
{
    int ret = NC_NOERR;

    /* defaults first */
    NCD4_rcdefault(d4info);

    /* extract the relevant triples into d4info */
    NCD4_rcprocess(d4info);

    if(d4info->curl->curlflags.useragent == NULL) {
        size_t len = strlen(DFALTUSERAGENT) + strlen(VERSION) + 1;
	char* agent = (char*)malloc(len+1);
	strncpy(agent,DFALTUSERAGENT,len);
	strncat(agent,VERSION,len);
        d4info->curl->curlflags.useragent = agent;
    }

    /* Some servers (e.g. thredds and columbia) appear to require a place
       to put cookies in order for some security functions to work
    */
    if(d4info->curl->curlflags.cookiejar != NULL
       && strlen(d4info->curl->curlflags.cookiejar) == 0) {
	free(d4info->curl->curlflags.cookiejar);
	d4info->curl->curlflags.cookiejar = NULL;
    }

    if(d4info->curl->curlflags.cookiejar == NULL) {
	/* If no cookie file was defined, define a default */
	char tmp[4096+1];
        int ok;
	pid_t pid = getpid();
	snprintf(tmp,sizeof(tmp)-1,"%s/%s.%ld/",NCD4_globalstate->tempdir,"netcdf",(long)pid);
#ifdef _WIN32
	ok = mkdir(tmp);
#else
	ok = mkdir(tmp,S_IRUSR | S_IWUSR | S_IXUSR);
#endif
	if(ok != 0 && errno != EEXIST) {
	    fprintf(stderr,"Cannot create cookie directory\n");
	    goto fail;
	}
	errno = 0;
	/* Create the unique cookie file name */
	ok = NCD4_mktmp(tmp,&d4info->curl->curlflags.cookiejar);
	d4info->curl->curlflags.createdflags |= COOKIECREATED;
	if(ok != NC_NOERR && errno != EEXIST) {
	    fprintf(stderr,"Cannot create cookie file\n");
	    goto fail;
	}
	errno = 0;
    }
    assert(d4info->curl->curlflags.cookiejar != NULL);

    /* Make sure the cookie jar exists and can be read and written */
    {
	FILE* f = NULL;
	char* fname = d4info->curl->curlflags.cookiejar;
	/* See if the file exists already */
        f = fopen(fname,"r");
	if(f == NULL) {
	    /* Ok, create it */
	    f = fopen(fname,"w+");
	    if(f == NULL) {
	        fprintf(stderr,"Cookie file cannot be read and written: %s\n",fname);
	        {ret= NC_EPERM; goto fail;}
	    }
	} else { /* test if file can be written */
	    fclose(f);
	    f = fopen(fname,"r+");
	    if(f == NULL) {
	        fprintf(stderr,"Cookie file is cannot be written: %s\n",fname);
	        {ret = NC_EPERM; goto fail;}
	    }
	}
	if(f != NULL) fclose(f);
    }

    return THROW(ret);

fail:
    return THROW(ret);
}

static void
applyclientparamcontrols(NCD4INFO* info)
{
    /* clear the flags */
    CLRFLAG(info->controls.flags,NCF_CACHE);
    CLRFLAG(info->controls.flags,NCF_SHOWFETCH);
    CLRFLAG(info->controls.flags,NCF_NC4);
    CLRFLAG(info->controls.flags,NCF_NCDAP);

    /* Turn on any default on flags */
    SETFLAG(info->controls.flags,DFALT_ON_FLAGS);
    SETFLAG(info->controls.flags,(NCF_NC4|NCF_NCDAP));

    if(paramcheck(info,"show","fetch"))
	SETFLAG(info->controls.flags,NCF_SHOWFETCH);

    if(paramcheck(info,"translate","nc4"))
	info->controls.translation = NCD4_TRANSNC4;
}

/* Search for substring in value of param. If substring == NULL; then just
   check if param is defined.
*/
static int
paramcheck(NCD4INFO* nccomm, const char* key, const char* subkey)
{
    const char* value;
    char* p;

    if(nccomm == NULL || key == NULL) return 0;
    if((value=ncurilookup(nccomm->uri,key)) == NULL)
	return 0;
    if(subkey == NULL) return 1;
    p = strstr(value,subkey);
    if(p == NULL) return 0;
    p += strlen(subkey);
    if(*p != '\0' && strchr(checkseps,*p) == NULL) return 0;
    return 1;
}
