#include <stdio.h>
#include <netcdf.h>

int main() {
    printf("NetCDF library version: %s\n", nc_inq_libvers());
    return 0;
}