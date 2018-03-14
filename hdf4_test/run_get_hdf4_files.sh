#!/bin/sh

# This shell gets some sample HDF4 files from the netCDF ftp site for
# testing. Then it runs program tst_interops3 on the test file to
# check that HDF4 reading works.

# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e
echo ""
echo "Getting HDF4 sample files from Unidata FTP site..."

file_list="AMSR_E_L2_Rain_V10_200905312326_A.hdf AMSR_E_L3_DailyLand_V06_20020619.hdf \
    MYD29.A2009152.0000.005.2009153124331.hdf MYD29.A2002185.0000.005.2007160150627.hdf \
    MOD29.A2000055.0005.005.2006267200024.hdf"
echo "Getting HDF4 test files $file_list"

# Try to get files 3 times, with a random delay between attempts.
for f1 in $file_list
do
    if ! test -f $f1; then
        failed=1
        num_tries=0
        delay=3
        while [ $failed -eq 1 -a $num_tries -lt 3 ]
        do
	    if wget "ftp://ftp.unidata.ucar.edu/pub/netcdf/sample_data/hdf4/$f1.gz"; then
                failed=0
                num_tries=0
                delay=3
	        gunzip -f $f1.gz
            else
                sleep $delay
                num_tries=`expr $num_tries + 1`
                delay=$(( RANDOM % (10 - 3 + 1 ) + 5 ))
            fi
        done
        # If we can't get a file, we can't proceed.
        if test $failed -eq 1; then
            echo "Could not get HDF4 files from Unidata FTP site!"
            exit 1
        fi
    fi
done


echo ""
echo "Running test program to check HDF4 sample files..."
${execdir}/tst_interops3

echo "SUCCESS!!!"

exit 0
