/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information.
*/
/*$Id$*/

#ifndef _ocrc_H_
#define _ocrc_H_ 1

/*
This ocrc parser is a modified version of json.
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

typedef enum ocrcclass {
ocrc_unknown = 0,
ocrc_map = 1,
ocrc_array = 2,
ocrc_pair = 3,
ocrc_const = 4,
/* Following are used to define constant classes */
ocrc_true=5,
ocrc_false=6,
ocrc_null=7,
ocrc_number=8,
ocrc_string=9
} ocrcclass;

/* ocrcnode is technically the union of several kinds of nodes:
   pairs, constants maps, and arrays; the last two are unified as list.
   Since the node size is so small, we do not bother with a formal union{...}.
*/
typedef struct ocrcnode {
    ocrcclass nodeclass;
    ocrcclass constclass; /*ocrc_string .. ocrc_null*/
        char* constvalue; /* jclass == ocrc_const */
        struct {
            struct ocrcnode* key;
            struct ocrcnode* value;
        } pair; /* jclass == ocrc_pair */
        struct {
            size_t nvalues;
            struct ocrcnode** values; /* set of pairs */
        } list ; /* jclass == ocrc_map|ocrc_array */
} ocrcnode;

typedef struct ocrcerror {
    int lineno;
    int charno;
    char* errmsg; /* may be null */
} ocrcerror;

extern ocrcnode* ocrc_const_true;
extern ocrcnode* ocrc_const_false;
extern ocrcnode* ocrc_const_null;

extern int ocrc(FILE* src, ocrcnode** nodep, ocrcerror*);
extern void ocrc_nodefree(ocrcnode* node);
extern void ocrc_dump(ocrcnode* node, FILE* f);
extern void ocrc_dumpmeta(ocrcnode* node, FILE* f);

extern ocrcnode* ocrc_lookup(ocrcnode* node, char* key);
extern ocrcnode* ocrc_get(ocrcnode* node, int index);

extern int ocrc_urlmatch(ocrcnode* map, char* url, ocrcnode*** matchp);

#endif /*_ocrc_H_*/
