#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <hdf5.h>
#include <netcdf.h>

#define LEN 3

int
main(int argc, char** argv)
{
    char v5[256];  
    char vn[256];  
    char* major, *minor, *patch;
    char* p;
    int i;

    strcpy(v5,H5_VERSION);
    major = v5; minor = NULL; patch = NULL;
    if((p = strchr(major,'.'))) {*p++ = '\0'; minor = p;}
    if((p = strchr(minor,'.'))) {*p++ = '\0'; patch = p;}
    vn[0]='\0';

    i = LEN - strlen(major);
    while(i-- > 0) strcat(vn,"0");
    strcat(vn,major);

    i = LEN - strlen(minor);
    while(i-- > 0) strcat(vn,"0");
    strcat(vn,minor);

    i = LEN - strlen(patch);
    while(i-- > 0) strcat(vn,"0");
    strcat(vn,patch);

    printf("%s",vn);
    return 0;
}
