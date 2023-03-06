#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

. ${srcdir}/d4test_common.sh

echo "test_parse.sh:"

cd ${DMRTESTFILES}
F=`ls -1 *.dmr | sed -e 's/[.]dmr//' |tr '\r\n' '  '`
cd $WD

setresultdir results_test_parse

if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.d4p ; fi
for f in $F ; do
    echo "testing: $f"
    ${VG} ${execdir}/test_parse ${DMRTESTFILES}/${f}.dmr > ./results_test_parse/${f}.d4p
    if test "x${TEST}" = x1 ; then
	diff -wBb ${BASELINE}/${f}.d4p ./results_test_parse/${f}.d4p 
    elif test "x${DIFF}" = x1 ; then
	echo "diff -wBb ${DMRTESTFILES}/${f}.dmr ./results_test_parse/${f}.d4p"
	rm -f ./tmp
	cat ./results_test_parse/${f}.d4p \
	| sed -e '/<Dimensions>/d' -e '/<Types>'/d -e '/<Variables>'/d -e '/<Groups>'/d \
	| sed -e '/<\/Dimensions>/d' -e '/<\/Types>'/d -e '/<\/Variables>'/d -e '/<\/Groups>'/d  \
	| sed -e '/_edu.ucar.opaque.size/,+2d' \
	| cat > ./tmp
	diff -wBb ${DMRTESTFILES}/${f}.dmr ./tmp
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results_test_parse/${f}.d4p ${BASELINE}/${f}.d4p	
    fi
done


