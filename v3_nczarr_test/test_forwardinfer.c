#include <netcdf.h>

int main(int argc, char **argv) {
   int ncid;
   nc_create("file://foo.zarr#mode=nczarr,noxarray", 0, &ncid);
}
