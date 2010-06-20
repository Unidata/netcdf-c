/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information.
*/
/*$Id$*/

#ifndef _ncrc_H_
#define _ncrc_H_ 1

/*
This ncrc parser is a modified version of json.
The modifications are as follows:
- the character set is utf8.
- "object" is referred to here as "map" 
- if the first token is not "{", then
  implicit "{...}" are added around the input.
- Commas are not used.
- a new format for strings (called words)
  has been added in which the quotes
  are not required. The word must
  have the following properties:
  - it does not contain white space or any of the characers: []{}:/
  - If a word parses as a C number (using "%lg" scanf format),
    then it will be returned as a number token.
- "//...\n" style comments are supported.
- \xdd constants in strings are accepted.
- \udddd constants are not.
- ',' is optional to allow for more json-like input; ';' is allowed
  as an alias for ','.
- '=' is allowed as an alias for ':'.
*/

typedef enum ncrcclass {
ncrc_unknown = 0,
ncrc_map = 1,
ncrc_array = 2,
ncrc_pair = 3,
ncrc_const = 4,
/* Following are used to define constant classes */
ncrc_true=5,
ncrc_false=6,
ncrc_null=7,
ncrc_number=8,
ncrc_string=9
} ncrcclass;

/* ncrcnode is technically the union of several kinds of nodes:
   pairs, constants maps, and arrays; the last two are unified as list.
   Since the node size is so small, we do not bother with a formal union{...}.
*/
typedef struct ncrcnode {
    ncrcclass nodeclass;
    ncrcclass constclass; /*ncrc_string .. ncrc_null*/
        char* constvalue; /* jclass == ncrc_const */
        struct {
            struct ncrcnode* key;
            struct ncrcnode* value;
        } pair; /* jclass == ncrc_pair */
        struct {
            size_t nvalues;
            struct ncrcnode** values; /* set of pairs */
        } list ; /* jclass == ncrc_map|ncrc_array */
} ncrcnode;

typedef struct ncrcerror {
    int lineno;
    int charno;
    char* errmsg; /* may be null */
} ncrcerror;

extern ncrcnode* ncrc_const_true;
extern ncrcnode* ncrc_const_false;
extern ncrcnode* ncrc_const_null;

extern int ncrc(FILE* src, ncrcnode** nodep, ncrcerror*);
extern void ncrc_nodefree(ncrcnode* node);
extern void ncrc_dump(ncrcnode* node, FILE* f);
extern void ncrc_dumpmeta(ncrcnode* node, FILE* f);

extern ncrcnode* ncrc_lookup(ncrcnode* node, char* key);
extern ncrcnode* ncrc_get(ncrcnode* node, int index);

extern int ncrc_urlmatch(ncrcnode* map, char* url, ncrcnode*** matchp);

#endif /*_ncrc_H_*/
