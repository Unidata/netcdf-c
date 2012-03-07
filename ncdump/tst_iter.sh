#!/bin/sh
# This shell script runs an ncdump bug test for netcdf
# Test if the nciter code is working.

set -e

echo "*** Running ncdump nc_iter test."

if test "x$CC" = "x" ; then CC="gcc"; fi

CLEANUP="geniter.c ref_tst_iter.cdl a.out tst_iter.nc tst_iter.cdl tmp"

rm -f $CLEANUP

echo "create ref_tst_iter.cdl"
cat > ref_tst_iter.cdl <<EOF
netcdf tst_iter {
dimensions:
x = 5 ;
y = 256 ;
z = 256;
variables:
int var(x,y,z) ;
data:
var =
EOF
cat >./geniter.c <<EOF
#include <stdlib.h>
#include <stdio.h>
#define N (5*256*256)
int main() {
  int i;
  for(i=0;i<N-1;i++) {printf("%d,\n",i);}
  printf("%d;\n}\n",N);
  return 0;
}
EOF

$CC ./geniter.c
./a.out >>ref_tst_iter.cdl

echo "*** create tst_iter.nc "
../ncgen/ncgen -k1 -o tst_iter.nc ./ref_tst_iter.cdl
echo "*** dumping tst_iter.nc to tst_iter.cdl..."
./ncdump tst_iter.nc > tst_iter.cdl
echo "*** reformat tst_iter.cdl"
mv tst_iter.cdl tmp
sed -e 's/\([0-9][,]\) /\1@/g' <tmp |tr '@' '\n' |sed -e '/^$/d' >./tst_iter.cdl

echo "*** comparing tst_iter.cdl with ref_tst_iter.cdl..."
diff -w tst_iter.cdl ./ref_tst_iter.cdl

# cleanup
rm -f $CLEANUP

echo "*** PASS: ncdump iter tests"
exit 0
