/* Copyright 2018, UCAR/Unidata.
   See the COPYRIGHT file for more information.
*/

#ifndef NCJSON_H
#define NCJSON_H 1

#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif
#endif

/**************************************************/
/* Json object sorts (note use of term sort rather than e.g. type or discriminant) */
#define NCJ_UNDEF    0
#define NCJ_STRING   1
#define NCJ_INT      2
#define NCJ_DOUBLE   3
#define NCJ_BOOLEAN  4
#define NCJ_DICT     5
#define NCJ_ARRAY    6
#define NCJ_NULL     7

#define NCJ_NSORTS   8

/* No flags are currently defined, but the argument is a placeholder */

/* Define a struct to store primitive values as unquoted
   strings. The sort will provide more info.  Do not bother with
   a union since the amount of saved space is minimal.
*/

typedef struct NCjson {
    int sort;     /* of this object */
    char* string; /* sort != DICT|ARRAY */
    struct NCjlist {
	    int len;
	    struct NCjson** contents;
    } list; /* sort == DICT|ARRAY */
} NCjson;

/* Structure to hold result of convertinf one json sort to  value of another type;
   don't use union so we can know when to reclaim sval
*/
struct NCJconst {int bval; long long ival; double dval; char* sval;};

/**************************************************/
/* Extended API */

/* Return 0 if ok else -1 */

#if defined(__cplusplus)
extern "C" {
#endif

/* Parse a JSON string */
DLLEXPORT int NCJparse(const char* text, unsigned flags, NCjson** jsonp);

/* Reclaim a JSON tree */
DLLEXPORT extern void NCJreclaim(NCjson* json);

/* Create a new JSON node of a given sort */
DLLEXPORT extern int NCJnew(int sort, NCjson** objectp);

/* Create new json object with given string content */
DLLEXPORT extern int NCJnewstring(int sort, const char* value, NCjson** jsonp);

/* Create new json object with given counted string content */
DLLEXPORT extern int NCJnewstringn(int sort, size_t len, const char* value, NCjson** jsonp);

/* Get dict key value by name */
DLLEXPORT extern int NCJdictget(const NCjson* dict, const char* key, NCjson** valuep);

/* Convert one json sort to  value of another type; don't use union so we can know when to reclaim sval */
DLLEXPORT extern int NCJcvt(const NCjson* value, int outsort, struct NCJconst* output);

#ifndef NETCDF_JSON_H

/* Insert an atomic value to an array or dict object. */
DLLEXPORT int NCJaddstring(NCjson* json, int sort, const char* s);

/* Append value to an array or dict object. */
DLLEXPORT extern int NCJappend(NCjson* object, NCjson* value);

/* Insert key-value pair into a dict object. key will be copied */
DLLEXPORT extern int NCJinsert(NCjson* object, char* key, NCjson* value);

/* Unparser to convert NCjson object to text in buffer */
DLLEXPORT extern int NCJunparse(const NCjson* json, unsigned flags, char** textp);

/* Deep clone a json object */
DLLEXPORT extern int NCJclone(const NCjson* json, NCjson** clonep);

/* dump NCjson* object to output file */
DLLEXPORT extern void NCJdump(const NCjson* json, unsigned flags, FILE*);
#endif

#if defined(__cplusplus)
}
#endif

/* Getters */
#define NCJsort(x) ((x)->sort)
#define NCJstring(x) ((x)->string)
#define NCJlength(x) ((x)==NULL ? 0 : (x)->list.len)
#define NCJcontents(x) ((x)->list.contents)
#define NCJith(x,i) ((x)->list.contents[i])

/* Setters */
#define NCJsetsort(x,s) (x)->sort=(s)
#define NCJsetstring(x,y) (x)->string=(y)
#define NCJsetcontents(x,c) (x)->list.contents=(c)
#define NCJsetlength(x,l) (x)->list.len=(l)

/* Misc */
#define NCJisatomic(j) ((j)->sort != NCJ_ARRAY && (j)->sort != NCJ_DICT && (j)->sort != NCJ_NULL && (j)->sort != NCJ_UNDEF)

/**************************************************/

#endif /*NCJSON_H*/

