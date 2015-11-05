#!/bin/bash

# Git Bisect Usage:
#   $ git bisect start [BAD] [GOOD] --
#   $ git bisect run ../cdf5-test.sh
#
# Specifics:
#   $ git bisect start HEAD fab6669 --
#   $ git bisect run /g/cdf5-test.sh

function CLEANUP() {
    git clean -fd
    rm -rf libsrc/putget.c libsrc/attr.c libsrc/ncx.c nc_test/test_get.c nc_test/test_put.c
    git stash
}


function CHKERR() {
    
    RES=$?

    if [ $RES -ne 0 ]; then
	popd
	CLEANUP
	exit 1
    fi
}
	

###
# Assume the project has already been configured
# with the appropriate flags.
###
BISECTLOG="/g/bisect-log.txt"
pushd build
cmake .. -DCMAKE_PREFIX_PATH=/c/share/VS12/x64 -DENABLE_DAP=OFF  -G"Visual Studio 12 Win64"
cmake --build . --config Debug --target clean 
cmake --build . --config Debug ; CHKERR 

putlib -xd
cd nctest
cd Debug
./nctest.exe ; CHKERR
exit 0
