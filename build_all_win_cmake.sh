#!/bin/bash

#####
# Function to check for error.
#####
CHECKERR () {
    ERRCODE=$?
    if [[ ! $TEST -eq 0 ]]; then
	echo "Error $ERRCODE caught. Exit."
	exit $ERRCODE
    fi
}

#####
# Function to do the building.
#####
DOBUILD () {
    BTYPE="static"
    if [ x$SHARED = "xON" ]; then
	BTYPE="shared"
    fi

    if [ $PLTFRM = "32" ]; then
	CMAKEGEN=$CMAKEGEN32
    else
	CMAKEGEN=$CMAKEGEN64
    fi

    # The packages are built 32 & 64 bit on OSX.
    if [ $unamestr == "Darwin" ]; then
	NAME="$VER-NC4_$NC4-DAP_$DAP-$BTYPE"
    else
	NAME="$VER-NC4_$NC4-DAP_$DAP-$PLTFRM-$BTYPE"
    fi
    BDIR=$ALLBUILDS"build_$NAME"
    echo "Building $NAME"

    rm -rf $BDIR
    mkdir -p $BDIR
    cd $BDIR
    CMAKE_PREFIX_PATH=$RESOURCE_DIR cmake $CURDIR -G"$CMAKEGEN" -DCPACK_PACKAGE_FILE_NAME=NetCDF-$NAME -DENABLE_TESTS=OFF -DBUILD_SHARED_LIBS=$SHARED -DENABLE_DAP=$DAP -DENABLE_NETCDF_4=$NC4
    CHECKERR

    cmake --build .
    CHECKERR

    cmake --build . --target package
    CHECKERR

    mv -f *.zip *.dmg *.exe $INSTALLDIRS
    cd $CURDIR
    echo "Finished building $NAME"
    
}


#####
# Set up platform-specific variables 
#####

unamestr=`uname | cut -d " " -f 1`
VER="4.2.x-snapshot"
CURDIR=`pwd`
ALLBUILDS=$CURDIR/"all_builds/"
INSTALLDIRS=$ALLBUILDS"packages"
POSTFLAGS=""

mkdir -p $ALLBUILDS

case $unamestr in
    Darwin) echo "Configuring Darwin"
    CMAKEGEN32="Unix Makefiles"
    CMAKEGEN64="Unix Makefiles"
    POSTFLAGS=-D'CMAKE_OSX_ARCHITECTURES=i386;x86_64'
    ;;
    MINGW32_NT-6.1) echo "Configuring MSYS/MinGW"
    CMAKEGEN32="Visual Studio 10"
    CMAKEGEN64="Visual Studio 10 Win64"
    ;;
    *) echo "Unknown platform: $unamestr"
    exit 1
    ;;
esac

rm -rf $INSTALLDIRS
mkdir -p $INSTALLDIRS


#####
# 32-bit
#####
PLTFRM="32"


###
# Static
###
RESOURCE_DIR=/c/share/w32/static


SHARED=OFF

for NC4_STATES in OFF ON; do
    for DAP_STATES in OFF ON; do
	NC4=$NC4_STATES
	DAP=$DAP_STATES
	DOBUILD
    done
done

###
# Shared
###

SHARED=ON

for NC4_STATES in OFF ON; do
    for DAP_STATES in OFF ON; do
	NC4=$NC4_STATES
	DAP=$DAP_STATES
	DOBUILD
    done
done

# Bail if on OSX; 32 & 64 were built together.
if [ $unamestr = "Darwin" ]; then
	exit 0
fi

####
# 64-bit
####
PLTFRM="64"

###
# Shared
###
RESOURCE_DIR=/c/share/x64/shared


SHARED=ON

for NC4_STATES in OFF ON; do
    for DAP_STATES in OFF ON; do
	NC4=$NC4_STATES
	DAP=$DAP_STATES
	DOBUILD
    done
done


exit 0
