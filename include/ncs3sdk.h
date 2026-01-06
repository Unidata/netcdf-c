/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef NCS3SDK_H
#define NCS3SDK_H 1

/* Track the server type, if known */
typedef enum NCS3SVC {
	NCS3UNK=0, /* unknown */
	NCS3=1,     /* s3.amazon.aws */
	NCS3GS=2,   /* storage.googleapis.com */
#ifdef NETCDF_ENABLE_ZOH
	NCS3ZOH=4,  /* ZoH Server */
#endif
} NCS3SVC;

/* Opaque Handles */
struct NClist;
struct AWSprofile;

typedef struct NCS3INFO {
    char* host; /* non-null if other*/
    char* region; /* region */
    char* bucket; /* bucket name */
    char* rootkey;
    char* profile;
    NCS3SVC svc;
} NCS3INFO;

/* Opaque Types */
struct NClist;
struct NCglobalstate;

#ifndef DECLSPEC
#ifdef DLL_NETCDF
  #ifdef DLL_EXPORT /* define when building the library */
    #define DECLSPEC __declspec(dllexport)
  #else
    #define DECLSPEC __declspec(dllimport)
  #endif
#else
  #define DECLSPEC
#endif
#endif /*!DECLSPEC*/

#ifdef __cplusplus
extern "C" {
#endif

/* API for ncs3sdk_XXX.[c|cpp] */
DECLSPEC int NC_s3sdkinitialize(void);
DECLSPEC int NC_s3sdkfinalize(void);
DECLSPEC void* NC_s3sdkcreateclient(NCS3INFO* context);
DECLSPEC int NC_s3sdkbucketexists(void* s3client, const char* bucket, int* existsp, char** errmsgp);
DECLSPEC int NC_s3sdkbucketcreate(void* s3client, const char* region, const char* bucket, char** errmsgp);
DECLSPEC int NC_s3sdkbucketdelete(void* s3client, NCS3INFO* info, char** errmsgp);
DECLSPEC int NC_s3sdkinfo(void* client0, const char* bucket, const char* pathkey, unsigned long long* lenp, char** errmsgp);
DECLSPEC int NC_s3sdkread(void* client0, const char* bucket, const char* pathkey, unsigned long long start, unsigned long long count, void* content, char** errmsgp);
DECLSPEC int NC_s3sdkwriteobject(void* client0, const char* bucket, const char* pathkey, unsigned long long count, const void* content, char** errmsgp);
DECLSPEC int NC_s3sdkclose(void* s3client0, char** errmsgp);
DECLSPEC int NC_s3sdktruncate(void* s3client0, const char* bucket, const char* prefix, char** errmsgp);
DECLSPEC int NC_s3sdklist(void* s3client0, const char* bucket, const char* prefix, size_t* nkeysp, char*** keysp, char** errmsgp);
DECLSPEC int NC_s3sdklistall(void* s3client0, const char* bucket, const char* prefixkey0, size_t* nkeysp, char*** keysp, char** errmsgp);
DECLSPEC int NC_s3sdkdeletekey(void* client0, const char* bucket, const char* pathkey, char** errmsgp);

/* From ds3util.c */
DECLSPEC void NC_s3sdkenvironment(void);

DECLSPEC int NC_getdefaults3region(NCURI* uri, const char** regionp);
DECLSPEC int NC_s3urlprocess(NCURI* url, NCS3INFO* s3, NCURI** newurlp);
DECLSPEC int NC_s3clear(NCS3INFO* s3);
DECLSPEC int NC_s3clone(NCS3INFO* s3, NCS3INFO** news3p);
DECLSPEC const char* NC_s3dumps3info(NCS3INFO* info);
DECLSPEC void NC_s3freeprofilelist(struct NClist* profiles);
DECLSPEC int NC_getactives3profile(NCURI* uri, const char** profilep);
DECLSPEC int NC_s3profilelookup(const char* profile, const char* key, const char** valuep);
DECLSPEC void NC_s3getcredentials(const char *profile, const char **region, const char **accessid, const char **accesskey);
DECLSPEC int NC_authgets3profile(const char* profile, struct AWSprofile** profilep);
DECLSPEC int NC_iss3(NCURI* uri, enum NCS3SVC*);
DECLSPEC int NC_s3urlrebuild(NCURI* url, struct NCS3INFO* s3, NCURI** newurlp);

#ifdef __cplusplus
}
#endif

#endif /*NCS3SDK_H*/
