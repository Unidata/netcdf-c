#include <netcdf.h>
#include <stdio.h>
#include <string.h>

#define FAIL_CODE 2
#define BAIL(e) do {                                      \
        printf("NetCDF error in file %s, line %d, %s\n",  \
               __FILE__, __LINE__, nc_strerror(e));       \
        return FAIL_CODE;                                  \
    } while (0)

#define STR1 ""     /* write empty string in this version */

int
main()
{
    int ncid, res, len1;

    /* Open existing file in read/write mode */
    if ((res = nc_open("zero-len-attribute-test.nc", NC_WRITE, &ncid)))
        BAIL(res);
    
    len1 = strlen (STR1);
    /* printf ("Write global attribute, strlen = %d\n", len1); */

    if ((res = nc_put_att_text(ncid, NC_GLOBAL, "comment", len1, STR1)))
        BAIL(res);

    if ((res = nc_close(ncid)))
        BAIL(res);

    return 0;
}
