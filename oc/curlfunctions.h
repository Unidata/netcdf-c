/*
 * rc.h
 *
 *  Created on: Mar 5, 2009
 *      Author: rikki
 */

#ifndef _CURLFUNCTION_H_
#define _CURLFUNCTION_H_

extern int set_curl_flags(CURL*, OCstate*);
extern int set_user_password(CURL*, const char *userC, const char *passwordC);
extern int set_proxy(CURL*, struct OCproxy*);
extern int set_cookies(CURL*, const char *cook);
extern int set_verify(CURL* curl);
extern int set_compression(CURL* curl);
extern int set_credentials(CURL*, struct OCcredentials*);

#endif /*_CURLFUNCTION_H_*/



