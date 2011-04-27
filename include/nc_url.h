/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef NC_URL_H
#define NC_URL_H

/*! This is an open structure meaning
	it is ok to directly access its fields*/
typedef struct NC_URL {
    char* url;        /* as passed by the caller */
    char* protocol;
    char* user; /* from user:password@ */
    char* password; /* from user:password@ */
    char* host;	      /*!< host*/
    char* port;	      /*!< host */
    char* file;	      /*!< file */
    char* constraint; /*!< projection+selection */
    char* projection; /*!< without leading '?'*/
    char* selection;  /*!< with leading '&'*/
    char* params;     /* all params */
    char** paramlist;    /*!<null terminated list */
} NC_URL;

extern int nc_urlparse(const char* s, NC_URL** nc_url);
extern void nc_urlfree(NC_URL* nc_url);

/* Replace the constraints */
extern void nc_urlsetconstraints(NC_URL*,const char* constraints);

/* Construct a complete NC_ URL; caller frees returned string */

/* Define flags to control what is included */
#define NC_URLCONSTRAINTS 1
#define NC_URLUSERPWD	  2
#define NC_URLPARAMS	  4

extern char* nc_urlgeturl(NC_URL*,const char* prefix, const char* suffix, int flags);

extern int nc_urldecodeparams(NC_URL* nc_url);

/*! NULL result => entry not found.
    Empty value should be represented as a zero length string */
extern const char* nc_urllookup(NC_URL*, const char* param);


extern char** nc_paramdecode(char* params0);
extern const char* nc_paramlookup(char** params, const char* key);
extern void nc_paramfree(char** params);
extern int nc_paramdelete(char** params, const char* key);
extern char** nc_paraminsert(char** params, const char* key, const char* value);
extern int nc_paramreplace(char** params, const char* key, const char* value);


#endif /*NC_URL_H*/
