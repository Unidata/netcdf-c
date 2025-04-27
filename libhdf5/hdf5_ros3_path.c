#include "hdf5_ros3_path.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
/**
 * @brief check if it's S3 link
 * A robust solution would be to use AWS C++ SDK kit
 * @return 1, if it's, 0 if it's not
 */
int is_s3_link(const char* link)
{
    int result = 0;
    result |=(strstr(link, "s3://") != NULL);
    result |=((strstr(link, ".s3.") != NULL) && (strstr(link, "https://") != NULL));
    return result;
}

int get_hdf5_ros3_link(const char *s3Url, char *ros3_hdf5_link, const char * region)
{

    // we need to convert this link
    if ((strstr(s3Url, "s3://") != NULL))
    {
        // Skip "s3://" prefix
        const char *path = s3Url + 5;
        // Find the first '/' to separate bucket and key
        const char *slash = strchr(path, '/');
        if (!slash)
        {
            return -1; // No key found
        }
        // Extract bucket (between "s3://" and first '/')
        size_t bucketLen = slash - path;
        char *bucket = (char *)malloc(bucketLen + 1);
        if (!bucket)
        {
            return -1; // Memory allocation failed
        }
        strncpy(bucket, path, bucketLen);
        (bucket)[bucketLen] = '\0';

        // Extract key (after the first '/')
        size_t keyLen = strlen(slash + 1);
        char *key = (char *)malloc(keyLen + 1);
        if (!key)
        {
            free(bucket);
            bucket = NULL;
            return -1; // Memory allocation failed
        }
        strcpy(key, slash + 1);
        if (!bucket || !key || !region) {
            return -1; // Invalid inputs
        }
        remove_mode(key);
        // Format: https://{bucket}.s3.{region}.amazonaws.com/{key}
        const char* prefix = "https://";
        const char* s3Middle = ".s3.";
        const char* s3Suffix = ".amazonaws.com/";
    
        // Calculate required length for the URL
        size_t urlLen = strlen(prefix) + strlen(bucket) + strlen(s3Middle) +
                       strlen(region) + strlen(s3Suffix) + strlen(key) + 1;
    
          // Construct the URL
        snprintf(ros3_hdf5_link, urlLen, "%s%s%s%s%s%s",
                 prefix, bucket, s3Middle, region, s3Suffix, key);
        return 0;
    }
    else
    {
        strcpy(ros3_hdf5_link,s3Url);
    }

    return 0;
}

int remove_mode(char *link)
{
    if (link == NULL) return -1; // safety check

    char *pound = strchr(link, '#');
    if (pound)
    {
        *pound = '\0'; // Terminate at '#'
    }
    return 0;
}