#!/bin/sh

# This shell gets files from the netCDF fortran library

set -e
echo ""
file_list="fort-attio.c fort-control.c fort-dim.c		\
fort-genatt.c fort-geninq.c fort-genvar.c fort-lib.c fort-misc.c	\
fort-v2compat.c fort-vario.c fort-var1io.c fort-varaio.c fort-varmio.c	\
fort-varsio.c cfortran.h fort-lib.h ncfortran.h netcdf.inc"
echo "Getting Fortran files $file_list"

for f1 in $file_list
do
    if ! test -f $f1; then
	cp ${F77_DIR}/fortran/$f1 .
    fi
done

echo "SUCCESS!!!"

exit 0
