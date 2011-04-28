/*
 * rc.h
 *
 *  Created on: Mar 5, 2009
 *      Author: rikki
 */

#ifndef _CURLFUNCTION_H_
#define _CURLFUNCTION_H_

extern int ocset_curl_flags(CURL*, OCstate*);
extern int ocset_user_password(CURL*, const char *user, const char *pwd);
extern int ocset_proxy(CURL*, OCstate*);
extern int ocset_ssl(CURL*, OCstate*);

#endif /*_CURLFUNCTION_H_*/



