#ifndef S3UTIL_H
#define S3UTIL_H
#ifdef __cplusplus
extern "C" {
#endif
int is_s3_link(const char* link);
int remove_mode(char *link);
int get_https_s3_link(const char *s3Url, char *ros3_hdf5_link, const char * region);
int find_profile(const char *s3_url, char **access_key , char **secret_key, char **session_token , char **region, const char *aws_profile );
int check_access(const char *s3_url, const char *access_key , const char *secret_key, const char *session_token ,  const char *region );
#ifdef __cplusplus
}
#endif
#endif