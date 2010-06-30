/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */


#include "ocinternal.h"
#include "ocdebug.h"
#include "ocdata.h"
#include "occontent.h"

#include "rc.h"

/* Set various general curl flags */
int
set_curl_flags(CURL* curl, struct OCcurlflags* flags)
{
    if (flags->verify)
	{if (set_verify(curl) != OC_NOERR) goto fail;}
    if (flags->compress)
	{if (set_compression(curl) != OC_NOERR) goto fail;}
    if (flags->cookies)
	{if (set_cookies(curl, flags->cookies) != OC_NOERR) goto fail;}
    return OC_NOERR;
fail:
    return OC_ECURL;
}

/* This is called with arguments while the other functions in this file are
 * used with global values read from the.dodsrc file.
 */
int
set_user_password(CURL* curl, const char *userC, const char *passwordC)
{
	CURLcode cstat;
	int iUserPassSize = strlen(userC) + strlen(passwordC) + 2;
	char *userPassword = malloc(sizeof(char) * iUserPassSize);
	if (!userPassword) {
		oc_log(LOGERR,
		      "Failed to allocate memory for the username and/or password.\n");
		return OC_ENOMEM;
	}
	strncpy(userPassword, userC, iUserPassSize);
	strncat(userPassword, ":", iUserPassSize);
	strncat(userPassword, passwordC, iUserPassSize);
	cstat = curl_easy_setopt(curl, CURLOPT_USERPWD, userPassword);
	if (cstat != CURLE_OK) {
		free(userPassword);
		return OC_ECURL;
	}

	cstat = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long) CURLAUTH_ANY);
	if (cstat != CURLE_OK) {
		free(userPassword);
		return OC_ECURL;
	}

	free(userPassword);
	return OC_NOERR;
}

int
set_proxy(CURL* curl, struct OCproxy *proxy)
{
	CURLcode cstat;
	cstat = curl_easy_setopt(curl, CURLOPT_PROXY, proxy->host);
	if (cstat != CURLE_OK)
		return OC_ECURL;

	cstat = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy->port);
	if (cstat != CURLE_OK)
		return OC_ECURL;

	if (proxy->username) {
		int userPassSize = strlen(proxy->username) + strlen(
				proxy->password) + 2;
		char *userPassword = malloc(sizeof(char) * userPassSize);
		if (!userPassword) {
			oc_log(LOGERR, "Out of Memory\n");
			return OC_ENOMEM;
		}
		strncpy(userPassword, proxy->username, userPassSize);
		strncat(userPassword, ":", userPassSize);
		strncat(userPassword, proxy->password, userPassSize);
		cstat = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userPassword);
		if (cstat != CURLE_OK) {
			free(userPassword);
			return OC_ECURL;
		}

#ifdef CURLOPT_PROXYAUTH
		cstat = curl_easy_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
		if(cstat != CURLE_OK) goto fail;
#endif
		free(userPassword);
	}

	return OC_NOERR;
}

int
set_cookies(CURL* curl, const char *cook)
{
	CURLcode cstat;
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cook);
	if (cstat != CURLE_OK)
		return OC_ECURL;

	cstat = curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
	if (cstat != CURLE_OK)
		return OC_ECURL;

	return OC_NOERR;
}

int
set_verify(CURL* curl)
{
	CURLcode cstat;
	cstat = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	if (cstat != CURLE_OK)
		return OC_ECURL;
	cstat = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	if (cstat != CURLE_OK)
		return OC_ECURL;

	return OC_NOERR;
}

int
set_compression(CURL* curl)
{
#ifdef CURLOPT_ENCODING
	CURLcode cstat;
	cstat = curl_easy_setopt(curl, CURLOPT_ENCODING, 'deflate, gzip');
	if(cstat != CURLE_OK)
		return OC_ECURL;
#endif
	return OC_NOERR;
}

int
set_credentials(CURL* curl, struct OCcredentials* creds)
{
    CURLcode cstat = CURLE_OK;
    if(creds->ssl_certificate) {
	cstat = curl_easy_setopt(curl, CURLOPT_SSLCERT, creds->ssl_certificate);
	if(cstat != CURLE_OK) goto fail;
    }
    if(creds->ssl_key) {
	cstat = curl_easy_setopt(curl, CURLOPT_SSLKEY, creds->ssl_key);
	if(cstat != CURLE_OK) goto fail;
    }
    if(creds->cainfo) {
	cstat = curl_easy_setopt(curl, CURLOPT_CAINFO, creds->cainfo);
	if(cstat != CURLE_OK) goto fail;
    }
    if(creds->capath) {
	cstat = curl_easy_setopt(curl, CURLOPT_CAPATH, creds->capath);
	if(cstat != CURLE_OK) goto fail;
    }
    if(creds->cookiefile) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, creds->cookiefile);
	if(cstat != CURLE_OK) goto fail;
    }
    if(creds->cookiejar) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEJAR, creds->cookiejar);
	if(cstat != CURLE_OK) goto fail;
    }
    return OC_NOERR;

fail:
    return OC_ECURL;
}
