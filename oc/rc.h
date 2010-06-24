/*
 * rc.h
 *
 *  Created on: Mar 5, 2009
 *      Author: rikki
 */

#ifndef RC_H_
#define RC_H_

extern int dods_compress;
extern int dods_verify;
extern struct OCproxy *pstructProxy;
extern char *cook;
extern char *userName;
extern char *password;

extern int credentials_in_url(const char *url);
extern int extract_credentials(const char *url, char **name, char **pw, char **result_url);
extern int set_credentials(CURL* curl, struct OCcredentials* creds);
extern int read_dodsrc(char *in_file_name, OCstate* state);
#ifdef OBSOLETE
extern int write_dodsrc(char *out_file_name);
#endif

extern int set_proxy(CURL* curl, struct OCproxy*);
extern int set_cookies(CURL* curl, const char *cook);
extern int set_verify(CURL* curl);
extern int set_compression(CURL* curl);

#endif /* RC_H_ */
