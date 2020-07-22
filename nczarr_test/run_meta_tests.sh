#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

NCZDUMP=../ncdump/ncdump
NCZGEN=../ncgen/ncgen

echo ""
echo "*** Integration Testing: Demo use of ncdump and ncgen"

# Process _NCProperties to remove version specific info
# Usage cleanncprops <src> <dst>
cleanncprops() {
  src="$1"
  dst="$2"
  rm -f $dst
  cat $src \
  | sed -e 's/_SuperblockVersion=[0-9][0-9]*/_Superblockversion=0/' \
  | sed -e 's/\(\\"_NCProperties\\":[ 	]*\\"version=\)[0-9],.*/\1\\"}" ;/' \
  | cat >$dst
}

echo ""; echo "*** Test meta-data write/read"
CMD="${execdir}/t_meta${ext} -c"
$CMD create
${NCDUMP} ./testmeta.ncz >./t_meta_create.cdl
cleanncprops t_meta_create.cdl tmp_meta_create.cdl
diff -wb ${srcdir}/ref_t_meta_create.cdl ./tmp_meta_create.cdl
rm -f tmp_meta_create.cdl
$CMD dim1
${NCDUMP} ./testmeta.ncz >./t_meta_dim1.cdl
cleanncprops t_meta_dim1.cdl tmp_meta_dim1.cdl
diff -wb ${srcdir}/ref_t_meta_dim1.cdl ./tmp_meta_dim1.cdl
rm -f tmp_meta_dim1.cdl
$CMD var1
${NCDUMP} ./testmeta.ncz >./t_meta_var1.cdl
cleanncprops t_meta_var1.cdl tmp_meta_var1.cdl
diff -wb ${srcdir}/ref_t_meta_var1.cdl ./tmp_meta_var1.cdl
rm -f tmp_meta_var1.cdl

# Use zarr enabled ncgen to create an ncz file
${NCZGEN} -4 -o '[mode=nczarr]file://t_ncgen.ncz' ${srcdir}/ref_t_ncgen.cdl
# Use zarr enabled ncdump
${NCZDUMP} -h -n t_ncgen '[mode=nczarr]file://t_ncgen.ncz' > t_ncgen.cdl
# compare
diff -wb ${srcdir}/ref_t_ncgen.cdl ./t_ncgen.cdl
