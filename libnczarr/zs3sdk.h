/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef ZS3SDK_H
#define ZS3SDK_H 1

#ifdef __cplusplus
extern "C" {
#endif

void NCZ_s3sdkinitialize(void);
void NCZ_s3sdkfinalize(void);
int NCZ_s3sdkcreateconfig(const char* host, const char* reqion, void** configp);
int NCZ_s3sdkcreateclient(void* config, void** clientp);
int NCZ_s3sdkbucketexists(void* s3client, const char* bucket, int* existsp, char** errmsgp);
int NCZ_s3sdkbucketcreate(void* s3client, const char* region, const char* bucket, char** errmsgp);
int NCZ_s3sdkbucketdelete(void* s3client, const char* region, const char* bucket, char** errmsgp);
int NCZ_s3sdkinfo(void* client0, const char* bucket, const char* pathkey, unsigned long long* lenp, char** errmsgp);
int NCZ_s3sdkread(void* client0, const char* bucket, const char* pathkey, unsigned long long start, unsigned long long count, void* content, char** errmsgp);
int NCZ_s3sdkwriteobject(void* client0, const char* bucket, const char* pathkey, unsigned long long count, const void* content, char** errmsgp);
int NCZ_s3sdkclose(void* s3client0, void* config0, const char* bucket, const char* rootkey, int deleteit, char** errmsgp);
int NCZ_s3sdkgetkeys(void* s3client0, const char* bucket, const char* prefix, size_t* nkeysp, char*** keysp, char** errmsgp);
int NCZ_s3sdkdeletekey(void* client0, const char* bucket, const char* pathkey, char** errmsgp);
int NCZ_s3sdkcreatekey(void* s3client0, const char* bucket, const char* pathkey, char** errmsgp);

#ifdef __cplusplus
}
#endif

#endif /*ZS3SDK_H*/
