#!/bin/bash

#####
# Help statement.
#####
DOHELP () {
	echo -e "Syntax: $0 -[i][x][h]\n\ti: Compile 32-bit libraries.\n\tx: Compile 64-bit libraries.\n\th: This help dialog."
	exit 0
}
	
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
    NAME="$VER"
    if [ "x$NC4" = "xON" ]; then
	NAME="$NAME-NC4"
    else
	NAME="$NAME-NC3"
    fi
    
    if [ x$DAP = "xON" ]; then
	NAME="$NAME-DAP"
    fi		

    NAME="$NAME-$PLTFRM"
    BDIR=$ALLBUILDS"build_$NAME"
    echo "Building $NAME"

    rm -rf $BDIR
    mkdir -p $BDIR
    cd $BDIR
    cmake $CURDIR -G"$CMAKEGEN" -DCPACK_PACKAGE_FILE_NAME=NetCDF-$NAME -DENABLE_TESTS=OFF -DBUILD_SHARED_LIBS=$SHARED -DENABLE_DAP=$DAP -DENABLE_NETCDF_4=$NC4 -D"CMAKE_PREFIX_PATH=$RESOURCE_DIR"
    CHECKERR

    cmake --build . --config Release
    CHECKERR

    cmake --build . --target package --config Release
    CHECKERR

    for X in zip dmg exe; do
	    TNAME="NetCDF-$NAME.$X"
	    if [ -f $TNAME ]; then
		    mv -f $TNAME $INSTALLDIRS
	    fi
    done

    cd $CURDIR
    echo "Finished building $NAME"
    
}

#####
# End functions, now parse arguments.
#####
BUILD32=""
BUILD64=""

if [ $# -gt 0 ]; then
	while getopts "ixh" Option; do
		case $Option in
			i ) BUILD32="TRUE";;
			x ) BUILD64="TRUE";;
			h ) DOHELP;;
			* ) DOHELP;;
		esac
	done
else 
	DOHELP
fi


#####
# Set up platform-specific variables 
#####

unamestr=`uname | cut -d " " -f 1`
VER="netCDF4.3RC1"
CURDIR=`pwd`
ALLBUILDS=$CURDIR/"all_builds/"
INSTALLDIRS=$ALLBUILDS"packages"
POSTFLAGS=""

mkdir -p $ALLBUILDS

case $unamestr in
    Darwin) echo "Configuring Darwin"
    CMAKEGEN32="Unix Makefiles"
    CMAKEGEN64="Unix Makefiles"
    #POSTFLAGS=-D'CMAKE_OSX_ARCHITECTURES=i386;x86_64'
    ;;
    MINGW32_NT-6.1) echo "Configuring MSYS/MinGW"
    CMAKEGEN32="Visual Studio 10"
    CMAKEGEN64="Visual Studio 10 Win64"
    POSTFLAGS="--config Release"
    ;;
    *) echo "Unknown platform: $unamestr"
    exit 1
    ;;
esac

#rm -rf $INSTALLDIRS
mkdir -p $INSTALLDIRS


#####
# 32-bit
#####
PLTFRM="32"


###
# Static
###
#RESOURCE_DIR=/c/share/w32/static


#SHARED=OFF

#for NC4_STATES in OFF ON; do
#    for DAP_STATES in OFF ON; do
#	NC4=$NC4_STATES
#	DAP=$DAP_STATES
#	DOBUILD
 #   done
#done

###
# Shared
###
if [ x$BUILD32 = "xTRUE" ]; then
	echo "Building 32-bit libraries."	
	RESOURCE_DIR=/c/share/w32/shared
	SHARED=ON

	for NC4_STATES in OFF ON; do
    		for DAP_STATES in OFF ON; do
			NC4=$NC4_STATES
			DAP=$DAP_STATES
			DOBUILD
	    	done
	done
fi

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
if [ x$BUILD64 = "xTRUE" ]; then
	echo "Building 64-bit libraries"
	RESOURCE_DIR=/c/share/x64/shared
	SHARED=ON

	for NC4_STATES in OFF ON; do
	    for DAP_STATES in OFF ON; do
		NC4=$NC4_STATES
		DAP=$DAP_STATES
		DOBUILD
	    done
	done
fi

exit 0
