/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include "d4curlfunctions.h"

static size_t WriteFileCallback(void*, size_t, size_t, void*);
static size_t WriteMemoryCallback(void*, size_t, size_t, void*);

struct Fetchdata {
        FILE* stream;
        size_t size;
};

long
ncd4_fetchhttpcode(CURL* curl)
{
    long httpcode = 200;
    CURLcode cstat = CURLE_OK;
    /* Extract the http code */
#ifdef HAVE_CURLINFO_RESPONSE_CODE
    cstat = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&httpcode);
#else
    cstat = curl_easy_getinfo(curl,CURLINFO_HTTP_CODE,&httpcode);
#endif
    if(cstat != CURLE_OK) {
	httpcode = 0;
        nclog(NCLOGERR, "curl error: %s", curl_easy_strerror(cstat));
    }
    return httpcode;
}

ncerror
ncd4_fetchurl_file(CURL* curl, const char* url, FILE* stream,
                off_t* sizep, long* filetime)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    struct Fetchdata fetchdata;

    /* Set the URL */
    cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
    if (cstat != CURLE_OK)
	goto fail;

    /* send all data to this function  */
    cstat = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    if (cstat != CURLE_OK)
	goto fail;

    /* we pass our file to the callback function */
    cstat = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&fetchdata);
     if (cstat != CURLE_OK)
	goto fail;

    /* One last thing; always try to get the last modified time */
    cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);
    if (cstat != CURLE_OK)
        goto fail;

    fetchdata.stream = stream;
    fetchdata.size = 0;
    cstat = curl_easy_perform(curl);

    if (cstat != CURLE_OK)
        goto fail;

    if (stat == NC_NOERR) {
        /* return the file size*/
#ifdef D4DEBUG
	nclog(NCLOGNOTE,"filesize: %lu bytes",fetchdata.size);
#endif
        if (sizep != NULL)
            *sizep = fetchdata.size;
        /* Get the last modified time */
        if(filetime != NULL)
            cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
        if(cstat != CURLE_OK) goto fail;
    }
    return THROW(stat);

fail:
    nclog(NCLOGERR, "curl error: %s", curl_easy_strerror(cstat));
    return THROW(NC_ECURL);
}

ncerror
ncd4_fetchurl(CURL* curl, const char* url, NCbytes* buf, long* filetime,
           struct credentials* creds)
{
    ncerror stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    size_t len;
    long httpcode = 0;

    /* Set the URL */
    cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
    if (cstat != CURLE_OK)
        goto fail;

    /* send all data to this function  */
    cstat = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    if (cstat != CURLE_OK)
        goto fail;

    /* we pass our file to the callback function */
    cstat = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)buf);
    if (cstat != CURLE_OK)
        goto fail;

    /* One last thing; always try to get the last modified time */
    cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);

    cstat = curl_easy_perform(curl);

    if(cstat == CURLE_PARTIAL_FILE) {
        /* Log it but otherwise ignore */
        nclog(NCLOGWARN, "curl error: %s; ignored",
               curl_easy_strerror(cstat));
        cstat = CURLE_OK;
    }
    httpcode = ncd4_fetchhttpcode(curl);

    if(cstat != CURLE_OK) goto fail;

    /* Get the last modified time */
    if(filetime != NULL)
        cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
    if(cstat != CURLE_OK) goto fail;

    /* Null terminate the buffer*/
    len = ncbyteslength(buf);
    ncbytesappend(buf, '\0');
    ncbytessetlength(buf, len); /* dont count null in buffer size*/
#ifdef D4DEBUG
    nclog(NCLOGNOTE,"buffersize: %lu bytes",(off_t)ncbyteslength(buf));
#endif
    return THROW(stat);

fail:
    nclog(NCLOGERR, "curl error: %s", curl_easy_strerror(cstat));
    switch (httpcode) {
    case 401: stat = NC_EAUTH; break;
    case 404: stat = ENOENT; break;
    case 500: stat = NC_EDAPSVC; break;
    case 200: break;
    default: stat = NC_ECURL; break;
    }
    return THROW(stat);
}

static size_t
WriteFileCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
    size_t realsize = size * nmemb;
    size_t count;
    struct Fetchdata* fetchdata;
    fetchdata = (struct Fetchdata*) data;
    if(realsize == 0)
        nclog(NCLOGWARN,"WriteFileCallback: zero sized chunk");
    count = fwrite(ptr, size, nmemb, fetchdata->stream);
    if (count > 0) {
            fetchdata->size += (count * size);
    } else {
        nclog(NCLOGWARN,"WriteFileCallback: zero sized write");
    }
#ifdef PROGRESS
    nclog(NCLOGNOTE,"callback: %lu bytes",(off_t)realsize);
#endif
    return count;
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    NCbytes* buf = (NCbytes*) data;
    if(realsize == 0)
        nclog(NCLOGWARN,"WriteMemoryCallback: zero sized chunk");
    /* Optimize for reading potentially large dods datasets */
    if(!ncbytesavail(buf,realsize)) {
        /* double the size of the packet */
        ncbytessetalloc(buf,2*ncbytesalloc(buf));
    }
    ncbytesappendn(buf, ptr, realsize);
#ifdef PROGRESS
    nclog(NCLOGNOTE,"callback: %lu bytes",(off_t)realsize);
#endif
    return realsize;
}

ncerror
ncd4_curlopen(CURL** curlp)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    CURL* curl;
    /* initialize curl*/
    curl = curl_easy_init();
    if (curl == NULL)
        stat = NC_ECURL;
    else {
        cstat = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
        if (cstat != CURLE_OK)
            stat = NC_ECURL;
    }
    if (curlp)
        *curlp = curl;
    return THROW(stat);
}

void
ncd4_curlclose(CURL* curl)
{
    if (curl != NULL)
        curl_easy_cleanup(curl);
}

ncerror
ncd4_fetchlastmodified(CURL* curl, char* url, long* filetime)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;

    /* Set the URL */
    cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
    if (cstat != CURLE_OK)
        goto fail;

    /* Ask for head */
    cstat = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30); /* 30sec timeout*/
    cstat = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
    cstat = curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    cstat = curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    cstat = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);

    cstat = curl_easy_perform(curl);
    if(cstat != CURLE_OK) goto fail;
    if(filetime != NULL)
        cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
    if(cstat != CURLE_OK) goto fail;
    return THROW(stat);

fail:
    nclog(NCLOGERR, "curl error: %s", curl_easy_strerror(cstat));
    return THROW(NC_ECURL);
}

ncerror
ncd4_ping(const char* url)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    CURL* curl = NULL;
    NCbytes* buf = NULL;

    /* Create a CURL instance */
    stat = ncd4_curlopen(&curl);
    if(stat != NC_NOERR) return stat;

    /* Use redirects */
    cstat = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    if (cstat != CURLE_OK)
        goto done;
    cstat = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (cstat != CURLE_OK)
        goto done;

    /* use a very short timeout: 10 seconds */
    cstat = curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)10);
    if (cstat != CURLE_OK)
        goto done;

    /* fail on HTTP 400 code errors */
    cstat = curl_easy_setopt(curl, CURLOPT_FAILONERROR, (long)1);
    if (cstat != CURLE_OK)
        goto done;

    /* Try to get the file */
    buf = ncbytesnew();
    stat = ncd4_fetchurl(curl,url,buf,NULL,NULL);
    if(stat == NC_NOERR) {
        /* Don't trust curl to return an error when request gets 404 */
        long http_code = 0;
        cstat = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &http_code);
        if (cstat != CURLE_OK)
            goto done;
        if(http_code >= 400) {
            cstat = CURLE_HTTP_RETURNED_ERROR;
            goto done;
        }
    } else
        goto done;

done:
    ncbytesfree(buf);
    ncd4_curlclose(curl);
    if(cstat != CURLE_OK) {
        nclog(NCLOGERR, "curl error: %s", curl_easy_strerror(cstat));
        stat = NC_EDAPSVC;
    }
    return THROW(stat);
}

