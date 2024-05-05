#!/bin/bash

# This file is derived from ncdump/tst_nccopy5.sh
TESTNCZARR=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

if test "x$TESTNCZARR" = x1 ; then
. ${srcdir}/test_nczarr.sh
s3isolate "testdir_nccopy5"
else
isolate testdir_ncccopy5
fi
THISDIR=`pwd`
cd $ISOPATH

# Program to run
if test "x$TESTNCZARR" = x1 ; then
    CHUNKTEST="${execdir}/test_chunking"
else
    CHUNKTEST="${execdir}/tst_chunking"
fi

# For a netCDF-4 build, test nccopy chunking rules

echo ""

# Trim off leading and trailing whitespace
# Also remove any <cr> and de-tabify
# usage: trim <line>
# Leaves result in variable TRIMMED
trim() {
    # trim leading whitespace and remove <cr>
    TMP=`echo "$1" |tr -d '\r' | tr '\t' ' ' |sed -e 's/^[ 	]*//'`
    # trim trailing whitespace
    TRIMMED=`echo "$TMP" | sed -e 's/[ 	]*$//'`
}

if test "x$TESTNCZARR" = x1 ; then
# NCZARR does not support contiguous storage
checkfvar() {
return
}
else
# usage: checkfvar <file>
checkfvar() {
  # Make sure that fvar was not chunked
  C5FVAR=`sed -e '/fvar:_ChunkSizes/p' -e d <$1`
  if test "x$C5FVAR" != x ; then
      echo "***Fail: fvar was chunked"
      exit 1
  fi
}
fi

# usage: checkivar <file>
checkivar() {
  # Make sure that ivar was not chunked
  C5IVAR=`sed -e '/ivar:_ChunkSizes/p' -e d <$1`
  if test "x$C5IVAR" != x ; then
      echo "***Fail: ivar was chunked"
      exit 1
  fi
}

# usage: verifychunkline line1 line2
verifychunkline() {
    # trim leading whitespace
    trim "$1"; L1="$TRIMMED"
    trim "$2"; L2="$TRIMMED"
    if test "x$L1" != "x$L2" ; then
	echo "chunk line mismatch |$L1| |$L2|"
	exit 1;
    fi
}

# Remove any temporary files
cleanup() {
    rm -f tmp_nc5.nc tmp_nc5a.nc
    rm -f tmp_nc5.cdl tmp_nc5a.cdl tmp_nc5b.cdl
    rm -f tmp_nc5_omit.nc tmp_nc5_omit.cdl
}

buildfile() {
zext=$1
index=$2
if test "x$TESTNCZARR" = x1 ; then
fileargs "tmp_nc5_${index}_${zext}"
deletemap $zext $file
file="$fileurl"
else
file="tmp_nc5_${index}_${zext}.nc"
rm -f $file
fi
}

testcase1() {
zext=$1
buildfile ${zext} 1

rm -fr tmp1${zext}.dir
mkdir tmp1${zext}.dir
cd tmp1${zext}.dir

# Create a simple classic input file 
${CHUNKTEST} $file

# Save a .cdl version
${NCDUMP} -n tmp_nc5_base ${file} > tmp_nc5.cdl

echo "*** Test nccopy -c with per-variable chunking; classic->enhanced"
# This should produce same as -c dim0/,dim1/1,dim2/,dim3/1,dim4/,dim5/1,dim6/
# But note that the chunk product is less than default, so we need to reduce it (-M)
${NCCOPY} -M1000 -c ivar:7,1,2,1,5,1,9 ${file} tmp_nc34.nc
${NCDUMP} -n tmp_nc5_base tmp_nc34.nc > tmp_nc34.cdl
# Verify that the core cdl is the same
diff tmp_nc5.cdl tmp_nc34.cdl

# Look at the output chunking
${NCDUMP} -hs -n tmp_nc5_base tmp_nc34.nc > tmp_chunking.cdl
# extract the chunking line
TESTLINE=`sed -e '/ivar:_ChunkSizes/p' -e d <tmp_chunking.cdl`
# track line to match
BASELINE='ivar:_ChunkSizes = 7, 1, 2, 1, 5, 1, 9 ;'
verifychunkline "$TESTLINE" "$BASELINE"
# Make sure that fvar was not chunked
checkfvar tmp_chunking.cdl
cd ..
} # T1

testcase2() {
zext=$1
buildfile ${zext} 2
    
rm -fr tmp2${zext}.dir
mkdir tmp2${zext}.dir
cd tmp2${zext}.dir

${CHUNKTEST} ${file} deflate

# Save a .cdl version
${NCDUMP} -n tmp_nc5_base ${file} > tmp_nc5.cdl

echo "*** Test nccopy -c with per-variable chunking; enhanced->enhanced"
# Use -M to ensure that chunking takes effect
${NCCOPY} -M500 -c ivar:4,1,2,1,5,2,3 $file tmp_nc44.nc
${NCDUMP} -n tmp_nc5_base tmp_nc44.nc > tmp_nc44.cdl
diff tmp_nc5.cdl tmp_nc44.cdl

# Look at the output chunking
${NCDUMP} -hs -n tmp_nc5_base tmp_nc44.nc > tmp_chunking.cdl
# extract the chunking line
TESTLINE=`sed -e '/ivar:_ChunkSizes/p' -e d <tmp_chunking.cdl`
# track line to match
BASELINE='ivar:_ChunkSizes = 4, 1, 2, 1, 5, 2, 3 ;'
verifychunkline "$TESTLINE" "$BASELINE"
# Make sure that fvar was not chunked
checkfvar tmp_chunking.cdl
cd ..
} # T2

testcase3() {
zext=$1
buildfile ${zext} 3
    
rm -fr tmp3${zext}.dir
mkdir tmp3${zext}.dir
cd tmp3${zext}.dir

${CHUNKTEST} ${file} group

# Save a .cdl version
${NCDUMP} -n tmp_nc5_base ${file} > tmp_nc5.cdl

echo "*** Test nccopy -c with FQN var name; enhanced ->enhanced"
${NCCOPY} -M500 -c /g/ivar:4,1,2,1,5,2,3 ${file} tmp_nc44.nc
${NCDUMP} -n tmp_nc5_base tmp_nc44.nc > tmp_nc44.cdl
diff tmp_nc5.cdl tmp_nc44.cdl

# Verify chunking
${NCDUMP} -hs -n tmp_nc5_base tmp_nc44.nc > tmp_chunking.cdl
# extract the chunking line
TESTLINE=`sed -e '/ivar:_ChunkSizes/p' -e d <tmp_chunking.cdl`
# track line to match
BASELINE='ivar:_ChunkSizes = 4, 1, 2, 1, 5, 2, 3 ;'
verifychunkline "$TESTLINE" "$BASELINE"
# Make sure that fvar was not chunked
checkfvar tmp_chunking.cdl
cd ..
} #T3

testcase4() {
zext=$1
buildfile ${zext} 4
    
rm -fr tmp4${zext}.dir
mkdir tmp4${zext}.dir
cd tmp4${zext}.dir

${CHUNKTEST} ${file} unlimited

# Save a .cdl version
${NCDUMP} -n tmp_nc5_base ${file} > tmp_nc5.cdl

echo "*** Test nccopy -c with unlimited dimension; classic ->enhanced"
# Warning: make sure that nccopy does not convert small chunking to contiguous => -M<small value>
${NCCOPY} -M50 -c ivar:5,3 $file tmp_nc34.nc
${NCDUMP} -n tmp_nc5_base tmp_nc34.nc > tmp_nc34.cdl
diff tmp_nc5.cdl tmp_nc34.cdl

# Verify chunking
${NCDUMP} -hs -n tmp_nc5_base tmp_nc34.nc > tmp_chunking.cdl
# extract the chunking line
TESTLINE=`sed -e '/ivar:_ChunkSizes/p' -e d <tmp_chunking.cdl`
# track line to match
BASELINE='   ivar:_ChunkSizes = 5, 3 ;   '
verifychunkline "$TESTLINE" "$BASELINE"

# Make sure that fvar was not chunked
checkfvar tmp_chunking.cdl
cd ..
} # T4

testcase5() {
zext=$1
buildfile ${zext} 5
    
rm -fr tmp5${zext}.dir
mkdir tmp5${zext}.dir
cd tmp5${zext}.dir

${CHUNKTEST} ${file}

# Save a .cdl version
${NCDUMP} -n tmp_nc5_base ${file} > tmp_nc5_omit.cdl

echo "*** Test nccopy -c fvar: to suppress chunking; classic ->enhanced"
${NCCOPY} -M500 -c ivar:7,1,2,1,5,1,9 -c fvar: $file tmp_nc34_omit.nc
${NCDUMP} -n tmp_nc5_base tmp_nc34_omit.nc > tmp_nc34_omit.cdl
diff tmp_nc5_omit.cdl tmp_nc34_omit.cdl

# Verify chunking of ivar
${NCDUMP} -hs -n tmp_nc5_omit tmp_nc34_omit.nc > tmp_chunking_omit.cdl
# extract the chunking line
TESTLINE=`sed -e '/ivar:_ChunkSizes/p' -e d <tmp_chunking_omit.cdl`
# track line to match
BASELINE='   ivar:_ChunkSizes = 7, 1, 2, 1, 5, 1, 9 ;   '
verifychunkline "$TESTLINE" "$BASELINE"

# Make sure that fvar was not chunked
checkfvar tmp_chunking_omit.cdl
cd ..
} # T5

testcases() {
    testcase1 $1
    testcase2 $1
    testcase3 $1
    testcase4 $1
    testcase5 $1
}

if test "x$TESTNCZARR" != x ; then
    testcases file
    if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcases zip ; fi
    if test "x$FEATURE_S3TESTS" = xyes ; then testcases s3 ; fi
    if test "x$FEATURE_S3TESTS" = xyes ; then s3sdkdelete "/${S3ISOPATH}" ; fi # Cleanup
else
    testcases nc
fi

echo "*** All nccopy tests passed!"
exit 0

