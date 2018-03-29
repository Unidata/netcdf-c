# Visual Studio

# Is netcdf-4 and/or DAP enabled?
NC4=1
DAP=1
CDF5=1
#HDF4=1

case "$1" in
vs|VS) VS=1 ;;
linux|nix|l|x) unset VS ;;
*) echo "Must specify env: vs|linux"; exit 1; ;;
esac

if test "x$VS" = x1 ; then
  if test "x$2" = xsetup ; then
    VSSETUP=1
  else
    unset VSSETUP
  fi
fi

#export NCPATHDEBUG=1

if test "x$VSSETUP" = x1 ; then
CFG="Debug"
else
CFG="Release"
fi

if test "x$VS" != x -a "x$INSTALL" != x ; then
FLAGS="-DCMAKE_PREFIX_PATH=c:/tools/nccmake"
fi
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=/tmp/netcdf"

if test "x$DAP" = x ; then FLAGS="$FLAGS -DENABLE_DAP=false"; fi
if test "x$NC4" = x ; then FLAGS="$FLAGS -DENABLE_NETCDF_4=false"; fi
if test "x$CDF5" != x ; then FLAGS="$FLAGS -DENABLE_CDF5=true"; fi
if test "x$HDF4" != x ; then FLAGS="$FLAGS -DENABLE_HDF4=true"; fi
FLAGS="$FLAGS -DENABLE_CONVERSION_WARNINGS=false"
FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
FLAGS="$FLAGS -DENABLE_TESTS=true"
FLAGS="$FLAGS -DENABLE_EXAMPLES=false"
FLAGS="$FLAGS -DENABLE_DYNAMIC_LOADING=false"
FLAGS="$FLAGS -DENABLE_WINSOCK2=false"
#FLAGS="$FLAGS -DENABLE_LARGE_FILE_TESTS=true"
#FLAGS="$FLAGS -DENABLE_FILTER_TESTING=true"
FLAGS="$FLAGS -DENABLE_LIBRARY_FINALIZE=true"

rm -fr build
mkdir build
cd build

NCLIB=`pwd`

if test "x$VS" != x ; then
# Visual Studio
CFG="Release"
NCLIB="${NCLIB}/liblib"
export PATH="${NCLIB}:${PATH}"
#G=
cmake "$G" -DCMAKE_BUILD_TYPE=${CFG} $FLAGS ..
cmake --build . --config ${CFG}
cmake --build . --config ${CFG} --target RUN_TESTS
else
# GCC
NCLIB="${NCLIB}/build/liblib"
#G="-GUnix Makefiles"
#T="--trace-expand"
cmake "${G}" $FLAGS ..
make all
make test
fi
exit
