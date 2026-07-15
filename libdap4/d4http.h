/*********************************************************************
  *   Copyright 2018, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *********************************************************************/

/** @file d4http.h
 * @brief Legacy HTTP/curl function prototypes (superseded by ncd4.h).
 *
 * These declarations are retained for reference; the active API is
 * declared in ncd4.h as NCD4_fetch*, NCD4_curlopen, NCD4_curlclose,
 * and NCD4_ping.
 * @author Dennis Heimbigner
 */

#ifndef D4HTTP_H
#define D4HTTP_H 1

/** @internal Open a libcurl easy handle. */
extern int curlopen(CURL** curlp);
/** @internal Close a libcurl easy handle. */
extern void curlclose(CURL* curl);

/** @internal Fetch a URL into an NCbytes buffer. */
extern ncerror ncd4_fetchurl(CURL* curl, const char* url, NCbytes* buf, long* filetime, struct credentials* creds);
/** @internal Fetch a URL into a FILE stream. */
extern ncerror ncd4_fetchurl_file(CURL* curl, const char* url, FILE* stream, d4size_t* sizep, long* filetime);

/** @internal Return the HTTP response code from the last transfer. */
extern long ncd4_fetchhttpcode(CURL* curl);

/** @internal Fetch only the Last-Modified time for a URL. */
extern ncerror ncd4_fetchlastmodified(CURL* curl, char* url, long* filetime);

/** @internal Open a libcurl easy handle (NCD4 variant). */
extern ncerror ncd4_curlopen(CURL** curlp);
/** @internal Close a libcurl easy handle (NCD4 variant). */
extern void ncd4_curlclose(CURL* curlp);

/** @internal Ping a URL to verify server reachability. */
extern ncerror ncd4_ping(const char* url);

#endif /*D4HTTP_H*/
