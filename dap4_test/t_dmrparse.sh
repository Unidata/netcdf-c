#!/bin/sh

if test "x$srcdir" = "x" ; then srcdir=`dirname $0`; fi; export srcdir

. ${srcdir}/test_common.sh

cd ${DMRTESTFILES}
F=`ls -1 *.dmr | sed -e 's/[.]dmr//' |tr '\r\n' '  '`
cd $WD

if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.d4p ; fi
for f in $F ; do
    if ! ./t_dmrparse ${DMRTESTFILES}/${f}.dmr > ./results/${f}.d4p ; then
	failure "${f}"
    fi
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINE}/${f}.d4p ./results/${f}.d4p ; then
	    failure "${f}"
	fi
    elif test "x${DIFF}" = x1 ; then
	echo "diff -wBb ${DMRTESTFILES}/${f}.dmr ./results/${f}.d4p"
	rm -f ./tmp
	cat ./results/${f}.d4p \
	| sed -e '/<Dimensions>/d' -e '/<Types>'/d -e '/<Variables>'/d -e '/<Groups>'/d \
	| sed -e '/<\/Dimensions>/d' -e '/<\/Types>'/d -e '/<\/Variables>'/d -e '/<\/Groups>'/d  \
	| sed -e '/_edu.ucar.opaque.size/,+2d' \
	| cat > ./tmp
	if ! diff -wBb ${DMRTESTFILES}/${f}.dmr ./tmp ; then
	    failure "${f}" 
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results/${f}.d4p ${BASELINE}/${f}.d4p	
    fi
done

echo "*** Pass"
exit 0

