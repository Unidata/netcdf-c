#!/bin/bash
#
# Unit tests for cmake. This script checks to see if CMake runs properly, 
# and if the code compiles properly (if CMake runs properly).
#
#set -x

DOBUILD="NO"
DOCLEAN="NO"
if [ $# -gt 0 ]; then
	
	while getopts "bhc" Option; do
	    case $Option in
		b ) DOBUILD="YES";;
                c ) DOCLEAN="YES";;
                * ) echo "Usage: $0 [-h|-b|-c]"; echo -e "-h:\tShow Help\n-b:\tExecute build checks.\n-c:\tClean directory between checks.";exit;
            esac
	done
	

fi

echo -e "\nRunning Tests:"
echo -e "\tBuilding after cmake:\t$DOBUILD"
echo -e "\tClean between builds:\t$DOCLEAN"
echo ""


BDIR="build_test"
LOGFILE="../build_test_output.txt"
echo `date` > $LOGFILE
BLDTYPE=""
if [ `uname -a | cut -d" " -f 1` = "MINGW32_NT-6.1" ]; then
    BLDTYPE="WIN"
fi

BUILDTYPE='-DBUILD_SHARED_LIBS=OFF -DBUILD_SHARED_LIBS=ON'
HDF5OPS='-DUSE_HDF5=OFF -DUSE_HDF5=ON'
DAPOPS='-DBUILD_DAP=OFF -DBUILD_DAP=ON'
DISKLESSOPS='-DENABLE_DISKLESS=OFF -DENABLE_DISKLESS=ON'
CDMROPS='-DENABLE_CDMREMOTE=OFF -DENABLE_CDMREMOTE=ON'




# Track configs which configurations fail.
CMAKEFAILS=""
BUILDFAILS=""

#mkdir -p $BDIR
#cd $BDIR
cmake_success=0
cmake_fail=0
build_success=0
build_fail=0
for BT in $BUILDTYPE; do
    for HOPS in $HDF5OPS; do
	for DOPS in $DAPOPS; do
	    for DIOPS in $DISKLESSOPS; do
		for COPS in $CDMROPS; do
		#for DOXS in $DOXOPS; do
		    #for IOPS in $INTOPS; do
			#for FOPS in $FSYOPS; do
			    #for VLOPS in $VALOPS; do
				#for N4OPS in $NC4OPS; do
		CUROPS="$BT $HOPS $DOPS $DIOPS $MMAPOPS $COPS"
		echo "Options: $CUROPS"
		if [ x$BLDTYPE = "xWIN" ]; then
		    cmake $CUROPS .. -G"MSYS Makefiles" >> $LOGFILE
		else
		    cmake $CUROPS .. >> $LOGFILE
		fi

		if [ $? -eq 0 ]; then
		    RET="PASS"
		    ((cmake_success++))
		else
		    RET="FAIL ($?)"
		    CMAKEFAILS="$CMAKEFAILS\n$CUROPS"
		    ((cmake_fail++))
		fi
		
		
		echo "CMake: $RET" 
		

		RET=""
		
		if [ x$DOBUILD = "xYES" ]; then
		    make -j >> $LOGFILE
		    
		    if [ $? -eq 0 ]; then
			RET="PASS"
			((build_success++))
		    else
			RET="FAIL ($?)"
			((build_fail++))
			BUILDFAILS="$BUILDFAILS\n$CUROPS"
		    fi
		    
		    echo "Build: $RET"
		    make clean
		fi
		RET=""
		echo ""
		#Clean up
		if [ x$DOCLEAN = "xYES" ]; then
		    rm -rf *
		fi
				#done
			    #done
			#done
		    #done
		#done
		done
	    done
	done
    done
done

echo -e "CMake:\tSuccess: $cmake_success\tFail: $cmake_fail"
if [ $cmake_fail -gt 0 ]; then
    echo -e "CMake Fails:$CMAKEFAILS\n\n"
fi

if [ x$DOBUILD = "xYES" ]; then
    echo -e "Build:\tSuccess: $build_success\tFail: $build_fail"
    if [ $build_fail -gt 0 ]; then
	echo -e "Build Fails:$BUILDFAILS"
    fi
fi

echo ""
echo ""