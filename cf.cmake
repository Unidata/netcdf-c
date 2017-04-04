# Is netcdf-4 and/or DAP enabled?
NC4=1
DAP=1

# Is visual studio being used?
VS=yes
#CYGWIN=yes

if test "x$VS" = x ; then
#CC=mpicc
CC=gcc
fi

export CC

FLAGS="-DCMAKE_PREFIX_PATH=c:/tools/nccmake"
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=d:/ignore"

if test "x$DAP" = x ; then
FLAGS="$FLAGS -DENABLE_DAP=false"
else
FLAGS="$FLAGS -DENABLE_DAP=true"
fi
if test "x$NC4" = x ; then
FLAGS="$FLAGS -DENABLE_NETCDF_4=false"
fi
FLAGS="$FLAGS -DENABLE_CONVERSION_WARNINGS=false"
FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
FLAGS="$FLAGS -DENABLE_TESTS=true"
FLAGS="$FLAGS -DENABLE_EXAMPLES=false"
#FLAGS="$FLAGS -DENABLE_HDF4=true"

rm -fr build
mkdir build
cd build

cmake $FLAGS ..
# We must use Release config here because Debug will invoke a runtime dialog box.

# If missing, appears to default to Debug
VSCFG=RelWithDebInfo
CFG="--config $VSCFG"
export PATH="/cygdrive/d/git/dap4/build/liblib/${VSCFG}:$PATH"
cmake --build . ${CFG}
cmake --build . ${CFG} --target RUN_TESTS
