#!/bin/bash
#
# Unit tests for cmake. This script checks to see if CMake runs properly, 
# and if the code compiles properly (if CMake runs properly).
#
set -x
BDIR="build_test"
LOGFILE="../build_test_output.txt"
echo `date` > $LOGFILE

BUILDTYPE='-D"BUILD_SHARED_LIBS=OFF" -D"BUILD_SHARED_LIBS=ON"'
HDF5OPS='-D"USE_HDF5=OFF -D"USE_HDF5=ON"'
DAPOPS='-D"BUILD_DAP=OFF -D"BUILD_DAP=ON"'
DISKLESSOPS='-D"BUILD_DISKLESS=OFF -D"BUILD_DISKLESS=ON"'
MMAPOPS='-D"BUILD_MMAP=OFF -D"BUILD_MMAP=ON"'
 
#mkdir -p $BDIR
#cd $BDIR

for BT in $BUILDTYPE; do
    echo $BT
    for HOPS in $HDFOPS; do
	echo $HOPS
	for DOPS in $DAPOPS; do
	    for DIOPS in $DISKLESSOPS; do
		for MMAPOPS in $MMAPOPS; do
		   
		    cmake $BT $HOPS $DOPS $DIOPS $MMAPOPS .. #>> $LOGFILE
		    
		    if [ $? -eq 0 ]; then
			RET="PASS"
		    else
			RET="FAIL ($?)"
		    fi
		    
 		    echo "Test [$BT $HOPS $DOPS $DIOPS $MMAPOPS]: $RET"
		    RET=""

		done
	    done
	done
    done
done

echo "Finished"