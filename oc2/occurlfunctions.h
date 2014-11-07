/*
 * occurlfunction.h
 *
 *  Created on: Mar 5, 2009
 *      Author: rikki
 */

#ifndef _CURLFUNCTION_H_
#define _CURLFUNCTION_H_

extern void oc_curl_protocols(struct OCGLOBALSTATE*);
extern CURLcode ocset_curl_flags(OCstate*);
extern OCerror ocset_user_password(OCstate*);
extern OCerror ocset_proxy(OCstate*);
extern OCerror ocset_ssl(OCstate*);
extern void oc_curl_setup(OCstate* state);
extern void oc_curl_debug(OCstate* state);
extern void oc_curl_printerror(OCstate* state);

#endif /*_CURLFUNCTION_H_*/



