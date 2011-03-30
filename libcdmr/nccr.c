/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <curl/curl.h>
#include "curlwrap.h"

#include "netcdf.h"
#include "nc.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nc4dispatch.h"

#include "nccr.h"
#include "crdebug.h"
#include "nccrdispatch.h"
#include "ast.h"
#include "ncstreamx.h"
#include "ncbytes.h"
#include "nclog.h"

/* Mnemonic */
#define getncid(drno) (((NC*)drno)->ext_ncid)

extern NC_FILE_INFO_T* nc_file;

static NCerror skiptoheader(bytes_t* packet, size_t* offsetp);
static void freeNCCDMR(NCCDMR* cdmr);

/**************************************************/
int
NCCR_new_nc(NC** ncpp)
{
    NCCR* ncp;
    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(struct NCCR)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}

/**************************************************/
/* See ncd4dispatch.c for other version */
int
NCCR_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    NCerror ncstat = NC_NOERR;
    NC_URL* tmpurl;
    NCCR* nccr = NULL; /* reuse the ncdap3 structure*/
    NC_HDF5_FILE_INFO_T* h5 = NULL;
    NC_GRP_INFO_T *grp = NULL;
    int ncid = -1;
    int fd;
    char* tmpname = NULL;
    NClist* shows;
    bytes_t buf;
    long filetime;
    ast_runtime* rt = NULL;
    ast_err aststat = AST_NOERR;
    Header* hdr = NULL;
    size_t offset;

    LOG((1, "nc_open_file: path %s mode %d", path, mode));

    if(nc_urlparse(path,&tmpurl) != NC_NOERR) PANIC("libcdmr: non-url path");
    nc_urlfree(tmpurl); /* no longer needed */

    /* Check for legal mode flags */
    if((mode & NC_WRITE) != 0) ncstat = NC_EINVAL;
    else if(mode & (NC_WRITE|NC_CLOBBER)) ncstat = NC_EPERM;
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    mode = (mode & ~(NC_MPIIO | NC_MPIPOSIX));
    /* Despite the above check, we want the file to be initially writable */
    mode |= (NC_WRITE|NC_CLOBBER);

    /* Use NCCR code to establish a pseudo file */
    tmpname = nulldup(PSEUDOFILE);
    fd = mkstemp(tmpname);
    if(fd < 0) {THROWCHK(errno); goto done;}
    /* Now, use the file to create the hdf5 file */
    ncstat = NC4_create(tmpname,NC_NETCDF4|NC_CLOBBER,
			0,0,NULL,0,NULL,dispatch,(NC**)&nccr);
    ncid = nccr->info.ext_ncid;
    /* unlink the temp file so it will automatically be reclaimed */
    unlink(tmpname);
    free(tmpname);
    /* Avoid fill */
    dispatch->set_fill(ncid,NC_NOFILL,NULL);
    if(ncstat)
	{THROWCHK(ncstat); goto done;}
    /* Find our metadata for this file. */
    ncstat = nc4_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5);
    if(ncstat)
	{THROWCHK(ncstat); goto done;}

    /* Setup tentative NCCR state*/
    nccr->info.dispatch = dispatch;
    nccr->cdmr = (NCCDMR*)calloc(1,sizeof(NCCDMR));
    if(nccr->cdmr == NULL) {ncstat = NC_ENOMEM; goto done;}
    nccr->cdmr->controller = (NC*)nccr;
    nccr->cdmr->urltext = nulldup(path);
    nc_urlparse(nccr->cdmr->urltext,&nccr->cdmr->url);

    /* Create the curl connection (does not make the server connection)*/
    ncstat = nccr_curlopen(&nccr->cdmr->curl.curl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    shows = nc_urllookup(nccr->cdmr->url,"show");
    if(nc_urllookupvalue(shows,"fetch"))
	nccr->cdmr->controls |= SHOWFETCH;

    /* fetch meta data */
    buf = bytes_t_null;
    NCbytes* completeurl = ncbytesnew();
    ncbytescat(completeurl,nccr->cdmr->url->url);
    ncbytescat(completeurl,"?req=header");
    ncstat = nccr_fetchurl(nccr->cdmr->curl.curl,ncbytescontents(completeurl),
			   &buf,&filetime);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Parse the meta data */
    ncstat = skiptoheader(&buf,&offset);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    aststat = ast_byteio_new(AST_READ,buf.bytes+offset,buf.nbytes-offset,&rt);
    if(aststat != AST_NOERR) goto done;    
    { Header* h = NULL;
    aststat = Header_read(rt,&h);
    hdr = h;
    }
    if(aststat != AST_NOERR) goto done;    
    aststat = ast_reclaim(rt);
    if(aststat != AST_NOERR) goto done;
    if(buf.bytes != NULL) free(buf.bytes);

    /* build the meta data */
    ncstat = nccr_buildnc(nccr,hdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Mark as no longer indef and no longer writable*/
    h5->flags &= ~(NC_INDEF);
    h5->no_write = 1;

done:
    if(aststat != AST_NOERR) {ncstat = nccr_cvtasterr(aststat);}
    if(ncstat) {
        if(nccr != NULL) {
	    int ncid = nccr->info.ext_ncid;
            freeNCCDMR(nccr->cdmr);
            NCCR_abort(ncid);
        }
    } else {
        if(ncpp) *ncpp = (NC*)nccr;
    }
    return THROW(ncstat);
}

int
NCCR_close(int ncid)
	{
    NC_GRP_INFO_T *grp;
    NC_HDF5_FILE_INFO_T *h5;
    NCCR* nccr = NULL;
    int ncstat = NC_NOERR;

    LOG((1, "nc_close: ncid 0x%x", ncid));
    /* Find our metadata for this file. */
    ncstat = nc4_find_nc_grp_h5(ncid, (NC_FILE_INFO_T**)&nccr, &grp, &h5);
    if(ncstat != NC_NOERR) return THROW(ncstat);

    /* This must be the root group. */
    if (grp->parent) ncstat = NC_EBADGRPID;

    /* Destroy/close the NCCR state */
    freeNCCDMR(nccr->cdmr);

    /* Destroy/close the NC_FILE_INFO_T state */
    NCCR_abort(ncid);

    return THROW(ncstat);
}

/**************************************************/
/* Auxilliary routines                            */
/**************************************************/

static NCerror
skiptoheader(bytes_t* packet, size_t* offsetp)
{
    NCerror status = NC_NOERR;
    unsigned long long vlen;
    size_t size,offset;

    /* Check the structure of the resulting data */
    if(packet->nbytes < (strlen(MAGIC_HEADER) + strlen(MAGIC_HEADER))) {
	nclog(NCLOGERR,"Curl data too short: %d\n",packet->nbytes);
	status = NC_ECURL;
	goto done;
    }
    if(memcmp(packet->bytes,MAGIC_HEADER,strlen(MAGIC_HEADER)) != 0) {
	nclog(NCLOGERR,"MAGIC_HEADER missing\n");
	status = NC_ECURL;
	goto done;
    }
    offset = strlen(MAGIC_HEADER);
    /* Extract the proposed count as a varint */
    vlen = varint_decode(10,packet->bytes+offset,&size);
    offset += size;
    if(vlen != (packet->nbytes-offset)) {
	nclog(NCLOGERR,"Curl data size mismatch\n");
	status = NC_ECURL;
	goto done;
    }
    if(offsetp) *offsetp = offset;

done:
    return status;    
}

static void
freeNCCDMR(NCCDMR* cdmr)
{
    if(cdmr == NULL) return;
    if(cdmr->urltext) free(cdmr->urltext);
    nc_urlfree(cdmr->url);
    if(cdmr->curl.curl) nccr_curlclose(cdmr->curl.curl);
    if(cdmr->curl.host) free(cdmr->curl.host);
    if(cdmr->curl.useragent) free(cdmr->curl.useragent);
    if(cdmr->curl.cookiefile) free(cdmr->curl.cookiefile);
    if(cdmr->curl.certificate) free(cdmr->curl.certificate);
    if(cdmr->curl.key) free(cdmr->curl.key);
    if(cdmr->curl.keypasswd) free(cdmr->curl.keypasswd);
    if(cdmr->curl.cainfo) free(cdmr->curl.cainfo);
    if(cdmr->curl.capath) free(cdmr->curl.capath);
    if(cdmr->curl.username) free(cdmr->curl.username);
    if(cdmr->curl.password) free(cdmr->curl.password);
    free(cdmr);
}

