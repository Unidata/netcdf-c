#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Which test cases to exercise
ORDER=1

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
    cat $1 \
 	| sed -e '/:_IsNetcdf4/d' \
	| sed -e '/:_Endianness/d' \
	| sed -e '/_NCProperties/d' \
	| sed -e '/_SuperblockVersion/d' \
	| cat > $2
}

# Function to extract _Filter attribute from a file
# These attributes might be platform dependent
getfilterattr() {
sed -e '/var.*:_Filter/p' -ed <$1 >$2
}

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}

# Execute the specified tests

if test "x$ORDER" = x1 ; then
echo "*** Testing multiple filter order of invocation"
rm -f filterorder.txt
${execdir}/test_filter_order >filterorder.txt
diff -b -w ${srcdir}/ref_filter_order.txt filterorder.txt
fi

echo "*** Pass: all selected tests passed"

#cleanup
rm -f filterorder.txt
exit 0

