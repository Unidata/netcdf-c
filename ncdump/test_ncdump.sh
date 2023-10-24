#!/bin/sh

if test "x$SETX" != x; then set -x; fi

ERR() {
    RES=$?
    if [ $RES -ne 0 ]; then
        echo "Error found: $RES"
        exit $RES
    fi
}

# Remove the version information from _NCProperties
cleanncprops() {
  src="$1"
  dst="$2"
  rm -f $dst
  cat $src \
  | sed -e '/:_Endianness/d' \
  | sed -e 's/_SuperblockVersion = [12]/_SuperblockVersion = 0/' \
  | sed -e 's/\(netcdflibversion\|netcdf\)=.*|/\1=NNNN|/' \
  | sed -e 's/\(hdf5libversion\|hdf5\)=.*"/\1=HHHH"/' \
  | grep -v '_NCProperties' \
  | cat >$dst
}

createtestinputs() {
echo "*** Running tst_create_files.c to create solar test files."
${execdir}/tst_create_files ; ERR
echo "*** Testing tst_create_files output for netCDF-4 features."
${NCDUMP} tst_solar_1.nc | sed 's/e+0/e+/g' > tst_solar_1.cdl ; ERR
diff -b tst_solar_1.cdl $srcdir/ref_tst_solar_1.cdl ; ERR
${NCDUMP} tst_solar_2.nc | sed 's/e+0/e+/g' > tst_solar_2.cdl ; ERR
diff -b tst_solar_2.cdl $srcdir/ref_tst_solar_2.cdl ; ERR

echo "*** Running tst_group_data.c to create test files."
${execdir}/tst_group_data ; ERR
${NCDUMP} tst_group_data.nc | sed 's/e+0/e+/g' > tst_group_data.cdl ; ERR
diff -b tst_group_data.cdl $srcdir/ref_tst_group_data.cdl ; ERR

echo "*** Testing -v option with relative name and groups..."
${NCDUMP} -v var,var2 tst_group_data.nc | sed 's/e+0/e+/g' > tst_group_data.cdl ; ERR
diff -b tst_group_data.cdl $srcdir/ref_tst_group_data.cdl ; ERR

echo "*** Running tst_enum_data.c to create test files."
${execdir}/tst_enum_data ; ERR
${NCDUMP} tst_enum_data.nc | sed 's/e+0/e+/g' > tst_enum_data.cdl ; ERR
diff -b tst_enum_data.cdl $srcdir/ref_tst_enum_data.cdl ; ERR

echo "*** Running tst_opaque_data.c to create test files."
${execdir}/tst_opaque_data ; ERR
${NCDUMP} tst_opaque_data.nc | sed 's/e+0/e+/g' > tst_opaque_data.cdl ; ERR
diff -b tst_opaque_data.cdl $srcdir/ref_tst_opaque_data.cdl ; ERR

echo "*** Running tst_comp.c to create test files."
${execdir}/tst_comp ; ERR
${NCDUMP} tst_comp.nc | sed 's/e+0/e+/g' > tst_comp.cdl ; ERR
diff -b tst_comp.cdl $srcdir/ref_tst_comp.cdl ; ERR

echo "*** Running tst_comp2.c to create test files."
${execdir}/tst_comp2 ; ERR
${NCDUMP} tst_comp2.nc | sed 's/e+0/e+/g' > tst_comp2.cdl ; ERR
diff -b tst_comp2.cdl $srcdir/ref_tst_comp2.cdl ; ERR

echo "*** Running tst_nans.c to create test files."
${execdir}/tst_nans ; ERR
${NCDUMP} tst_nans.nc | sed 's/e+0/e+/g' > tst_nans.cdl ; ERR
diff -b tst_nans.cdl $srcdir/ref_tst_nans.cdl ; ERR

echo "*** Running tst_special_atts.c to create test files."
${execdir}/tst_special_atts ; ERR
${NCDUMP} -c -s tst_special_atts.nc  > tst_special_atts.cdl ; ERR
cleanncprops tst_special_atts.cdl tst_special_atts.tmp
cleanncprops $srcdir/ref_tst_special_atts.cdl ref_tst_special_atts.tmp
echo "*** comparing tst_special_atts.cdl with ref_tst_special_atts.cdl..."
diff -b tst_special_atts.tmp ref_tst_special_atts.tmp ; ERR

${execdir}/tst_string_data ; ERR

${execdir}/tst_fillbug
# echo "*** dumping tst_fillbug.nc to tst_fillbug.cdl..."
${NCDUMP} tst_fillbug.nc > tst_fillbug.cdl
# echo "*** comparing tst_fillbug.cdl with ref_tst_fillbug.cdl..."
diff -b tst_fillbug.cdl $srcdir/ref_tst_fillbug.cdl


}
