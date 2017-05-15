# Visual Studio
VS=1

# Is netcdf-4 and/or DAP enabled?
NC4=1
DAP=1

if test "x$VS" != x ; then
FLAGS="-DCMAKE_PREFIX_PATH=c:/tools/nccmake"
fi
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
FLAGS="$FLAGS -DENABLE_DYNAMIC_LOADING=false"

rm -fr build
mkdir build
cd build

NCLIB=`pwd`

if test "x$VS" != x ; then
CMAKE=/cygdrive/c/tools/CMake/bin/cmake
# Visual Studio
#CFG="RelWithDebInfo"
CFG="Release"
NCLIB="${NCLIB}/build/liblib/$CFG"
export PATH="${NCLIB}:${PATH}"
$CMAKE $FLAGS ..
$CMAKE --build . --config ${CFG}
$CMAKE --build . --config ${CFG} --target RUN_TESTS
else
# GCC
CMAKE=/usr/bin/cmake.exe
NCLIB="${NCLIB}/build/liblib"
export PATH="${NCLIB}:${PATH}"
G="-GUnix Makefiles"
#$CMAKE "${G}" $FLAGS ..
$CMAKE $FLAGS ..
make all
make test
fi
exit
p
