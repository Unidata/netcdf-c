#ifndef RC_H_
#define RC_H_

/* Max .dodsrc line size */
#define MAXRCLINESIZE 2048

/* Max number of triples in a .dodsrc */
#define MAXRCLINES 2048

/* Create a triple store for (url,key,value) and sorted by url */

/* Actual triple store */
extern struct NCCRTriplestore {
    int ntriples;
    struct NCCRTriple {
        char url[MAXRCLINESIZE];
        char key[MAXRCLINESIZE];
        char value[MAXRCLINESIZE];
   } triples[MAXRCLINES];
} *nccrdodsrc;

extern int nccrdodsrc_read(char *in_file_name);
extern int nccrdodsrc_process(OCstate* state);
extern char* nccrdodsrc_lookup(char* key, char* url);

extern int nccrcredentials_in_url(const char *url);
extern int nccrextract_credentials(const char *url, char **name, char **pw, char **result_url);

#endif /* RC_H_ */
