/*
 * rc.h
 *
 *  Created on: Mar 5, 2009
 *      Author: rikki
 */

#ifndef RC_H_
#define RC_H_

extern int occredentials_in_url(const char *url);
extern int ocextract_credentials(const char *url, char **name, char **pw, char **result_url);
extern int ocread_dodsrc(char *in_file_name, OCstate* state);
#ifdef OBSOLETE
extern int ocwrite_dodsrc(char *out_file_name);
#endif

#endif /* RC_H_ */
