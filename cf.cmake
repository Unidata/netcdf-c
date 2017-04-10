# Is netcdf-4 and/or DAP enabled?
NC4=1
DAP=1

#export CC=mpicc
export CC=gcc

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
FLAGS="$FLAGS -DENABLE_DYNAMIC_LOADING=false"

rm -fr build
mkdir build
cd build

NCLIB=`pwd`
NCLIB="${NCLIB}/build/liblib"

G="-GUnix Makefiles"
cmake "${G}" $FLAGS ..
#cmake "${G}" --build .

#cmake "${G}" --build . --target test
make all
export PATH="${NCLIB}:${PATH}"
make test
exit
