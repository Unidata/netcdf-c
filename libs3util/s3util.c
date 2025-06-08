#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "s3util.h"
#include <curl/curl.h>
#define MAX_LINE_LENGTH 4096
#define MAX_NUMBER_OF_PROFILES 16
#define LOG 0
static int profile_count = 0;
struct Aws_profile {
    char aws_access_key_id[256];
    char aws_secret_access_key[512];
    char aws_session_token[4096];
    char profile_name[128];
    char region[128];
} typedef aws_prof;

aws_prof profiles[MAX_NUMBER_OF_PROFILES];

#define GET_CREDS(var)                                                                            \
    {                                                                                             \
        char creds[64] = #var;                                                                    \
        char extract_key[128];                                                                    \
        snprintf(extract_key, sizeof(extract_key), "%s = ", creds);                               \
        int res = extract_keyword(line, extract_key, profiles[profile_count - 1].var);            \
        if (res == 0) {                                                                           \
            snprintf(extract_key, sizeof(extract_key), "%s=", creds);                             \
            res = extract_keyword(line, extract_key, profiles[profile_count - 1].var);            \
        }                                                                                         \
        if (res && LOG) {                                                                         \
            printf("Key word %s = %s with length %ld \n", creds, profiles[profile_count - 1].var, \
                   strlen(profiles[profile_count - 1].var));                                      \
        }                                                                                         \
    }

char* replace_substring(const char* str, const char* old_sub, const char* new_sub) {
    // Check for invalid inputs
    if (!str || !old_sub || !new_sub || !*old_sub) {
        return NULL;
    }

    // Calculate lengths
    size_t str_len = strlen(str);
    size_t old_len = strlen(old_sub);
    size_t new_len = strlen(new_sub);
    
    // Count occurrences of old_sub in str
    size_t count = 0;
    const char* temp = str;
    while ((temp = strstr(temp, old_sub))) {
        count++;
        temp += old_len;
    }

    // Calculate new string length
    size_t new_str_len = str_len + count * (new_len - old_len);
    
    // Allocate memory for new string
    char* result = (char*)malloc(new_str_len + 1);
    if (!result) {
        return NULL;
    }

    // Perform replacement
    char* current_pos = result;
    const char* str_pos = str;
    temp = str;

    while ((temp = strstr(str_pos, old_sub))) {
        // Copy characters before the match
        size_t chars_before = temp - str_pos;
        strncpy(current_pos, str_pos, chars_before);
        current_pos += chars_before;

        // Copy new substring
        strcpy(current_pos, new_sub);
        current_pos += new_len;

        // Move past the old substring
        str_pos = temp + old_len;
    }

    // Copy remaining characters
    strcpy(current_pos, str_pos);

    return result;
}


/**
 * @brief check if it's S3 link
 * A robust solution would be to use AWS C++ SDK kit
 * @return 1, if it's, 0 if it's not
 */
int is_s3_link(const char *link) {
    int result = 0;
    result |= (strstr(link, "s3://") != NULL);
    result |= ((strstr(link, ".s3.") != NULL) && (strstr(link, "https://") != NULL));
    return result;
}

/**
    @brief removes #mode specifier from the end of the link
 */
int remove_mode(char *link) {
    if (link == NULL)
        return 1;  // safety check

    char *pound = strchr(link, '#');
    if (pound) {
        *pound = '\0';  // Terminate at '#'
    }
    return 0;
}

int get_https_s3_link(const char *s3Url, char *ros3_hdf5_link, const char *region) {
    // check if it's indeed a url/s3 uri
    if (is_s3_link(s3Url) == 0)
        return 1;
    // we need to convert this link
    if ((strstr(s3Url, "s3://") != NULL)) {
        // Skip "s3://" prefix
        const char *path = s3Url + 5;
        // Find the first '/' to separate bucket and key
        const char *slash = strchr(path, '/');
        if (!slash) {
            return 1;  // No key found
        }
        // Extract bucket (between "s3://" and first '/')
        size_t bucketLen = slash - path;
        char *bucket = (char *)malloc(bucketLen + 1);
        if (!bucket) {
            return 1;  // Memory allocation failed
        }
        strncpy(bucket, path, bucketLen);
        (bucket)[bucketLen] = '\0';

        // Extract key (after the first '/')
        size_t keyLen = strlen(slash + 1);
        char *key = (char *)malloc(keyLen + 1);
        if (!key) {
            free(bucket);
            bucket = NULL;
            return 1;  // Memory allocation failed
        }
        strcpy(key, slash + 1);
        if (!bucket || !key || !region) {
            return 1;  // Invalid inputs
        }
        remove_mode(key);
        // Format: https://{bucket}.s3.{region}.amazonaws.com/{key}
        const char *prefix = "https://";
        const char *s3Middle = ".s3.";
        const char *s3Suffix = ".amazonaws.com/";

        // Calculate required length for the URL
        size_t urlLen = strlen(prefix) + strlen(bucket) + strlen(s3Middle) + strlen(region) +
                        strlen(s3Suffix) + strlen(key) + 1;

        // Construct the URL
        snprintf(ros3_hdf5_link, urlLen, "%s%s%s%s%s%s", prefix, bucket, s3Middle, region, s3Suffix, key);
        return 0;
    } else {
        strcpy(ros3_hdf5_link, s3Url);
    }

    return 0;
}

int extract_keyword(const char *orginal_line, const char *keyword, char *value) {
    char *begin = strstr(orginal_line, keyword);

    if (begin) {
        char *end = strstr(begin + strlen(keyword), " ");
        char *end_tab = strstr(begin + strlen(keyword), "\t");
        if (end == NULL)
            end = end_tab;
        if (end && end_tab) {
            end = end < end_tab ? end : end_tab;
        }
        if (end)
            strncpy(value, begin + strlen(keyword), end - begin - strlen(keyword));
        else
            strncpy(value, begin + strlen(keyword), strlen(begin) - strlen(keyword));
        if (value[strlen(value) - 1] == '\n') {
            value[strlen(value) - 1] = '\0';
        }
        return 1;
    }

    return 0;
}

int is_it_profile(const char *orginal_line, char *value) {
    // check if it's a profile:
    char *begin_prof = strstr(orginal_line, "[");
    char *end_prof = strstr(orginal_line, "]");
    if (begin_prof != NULL && end_prof != NULL) {
        strncpy(value, begin_prof + 1, end_prof - begin_prof - 1);
        value[end_prof - begin_prof - 1] = '\0';
        return 1;
    }
    return 0;
}

int parse_aws_cred(const char *aws_profile) {
    char * aws_creds = replace_substring(aws_profile,"config","credentials");
    FILE *file = fopen(aws_creds, "r");
    // read credentials
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        // check if it's a profile:
        if (is_it_profile(line, profiles[profile_count].profile_name)) {
            // printf("%s \n", profiles[profile_count].profile_name);
            // lets find it's credentials:
            profiles[profile_count].aws_access_key_id[0] = '\0';
            profiles[profile_count].aws_secret_access_key[0] = '\0';
            profiles[profile_count].aws_session_token[0] = '\0';
            profiles[profile_count].region[0] = '\0';
            profile_count++;
        }
        if (profile_count) {
            // look for credentials
            GET_CREDS(aws_access_key_id);
            GET_CREDS(aws_secret_access_key);
            GET_CREDS(aws_session_token);
        }
    }

    fclose(file);
	free(aws_creds);
    // read profile
    file = fopen(aws_profile, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    int prof_to_fil = -1;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char config_profile_name[128];
        config_profile_name[0] = '\0';

        if (is_it_profile(line, config_profile_name))
        {
            int res = 0;
            for (size_t iprof = 0; iprof < profile_count; iprof++)
            {
                if (strstr(config_profile_name, profiles[iprof].profile_name) != NULL)
                {
                    prof_to_fil = iprof;
                  }
            }
        }
        if (prof_to_fil != -1)
        {
            int res = extract_keyword(line, "region = ", profiles[prof_to_fil].region);
            if (res == 0)
            {
                res = extract_keyword(line, "region=", profiles[prof_to_fil].region);
            }
        }
    }
    fclose(file);
    return 0;
}

int check_access(const char *s3_url, const char *access_key, const char *secret_key,
                 const char *session_token, const char *region) {
    CURL *curl = curl_easy_init();
    if (!curl)
        return 1;
    char auth[256];
    char url[1024];
    int status = get_https_s3_link(s3_url, url, region);
    if (status) {
        fprintf(stderr, "-E-: couldn't convert the link %s\n", s3_url);
        return status;
    }

    snprintf(auth, sizeof(auth), "%s:%s", access_key, secret_key);
    char provider[256];
    snprintf(provider, sizeof(provider), "aws:amz:%s:s3", region);
    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERPWD, auth);
    curl_easy_setopt(curl, CURLOPT_AWS_SIGV4, provider);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // HEAD request

    // Add security token header if present
    struct curl_slist *headers = NULL;
    if (session_token && *session_token) {
        char token_header[1024];
        snprintf(token_header, sizeof(token_header), "x-amz-security-token: %s", session_token);
        headers = curl_slist_append(headers, token_header);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Execute request
    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    int return_status = 0;
    // Output result
    if (res == CURLE_OK) {
        // printf("HTTP Status: %ld\n", response_code);
        if (response_code == 200) {
            return_status = 0;  // printf("✅ Access granted\n");
        } else if (response_code == 403) {
            return_status = 1;  // printf("❌ Permission denied\n");
        } else if (response_code == 404) {
            return_status = 1;  // printf("❌ Object not found\n");
        }
    } else {
        return_status = 1;  // fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    }

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return return_status;
}

int find_profile(const char *s3_url, char **access_key, char **secret_key, char **session_token, char **region,
                 const char *aws_profile) {
    *access_key = strdup(getenv("AWS_ACCESS_KEY_ID"));
    *secret_key = strdup(getenv("AWS_SECRET_ACCESS_KEY"));
    *session_token = strdup(getenv("AWS_SESSION_TOKEN"));
    *region = strdup(getenv("AWS_REGION"));
    int status = check_access(s3_url, *access_key, *secret_key, *session_token, *region);
    if (status) {
        free(*access_key);
        free(*secret_key);
        free(*session_token);
        free(*region);
    }
    else
        return status;
    if (aws_profile) {
        status = parse_aws_cred(aws_profile);
        for (int iprof = 0; iprof < profile_count; iprof++) {
            status =
                check_access(s3_url, profiles[iprof].aws_access_key_id, profiles[iprof].aws_secret_access_key,
                             profiles[iprof].aws_session_token, profiles[iprof].region);
            if (status == 0) {
                *access_key = strdup(profiles[iprof].aws_access_key_id);
                *secret_key = strdup(profiles[iprof].aws_secret_access_key);
                *session_token = strdup(profiles[iprof].aws_session_token);
                *region = strdup(profiles[iprof].region);
                break;
            }
        }
    }
    return status;
}
