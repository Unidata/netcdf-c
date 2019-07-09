# Visual Studio

# Is netcdf-4 and/or DAP enabled?
NC4=1
DAP=1
#CDF5=1
#HDF4=1

#TR=--trace

NCC="c:/tools/nccmake"

export SETX=1

for arg in "$@" ; do
case "$arg" in
vs|VS) VS=1 ;;
linux|nix|l|x) unset VS ;;
nobuild|nb) NOBUILD=1 ;;
notest|nt) NOTEST=1 ;;
*) echo "Must specify env: vs|linux"; exit 1; ;;
esac
done

if test "x$VS" = x1 ; then
  if test "x$2" = xsetup ; then
    VSSETUP=1
  else
    unset VSSETUP
  fi
fi

#TESTSERVERS="localhost:8080,149.165.169.123:8080"

#export NCPATHDEBUG=1

if test "x$VSSETUP" = x1 ; then
CFG="Debug"
else
CFG="Release"
fi

FLAGS=

if test "x$VS" != x -a "x$INSTALL" != x ; then
<<<<<<< HEAD
FLAGS="-DCMAKE_PREFIX_PATH=${NCC}"
=======
FLAGS="$FLAGS -DCMAKE_PREFIX_PATH=${NCC}"
>>>>>>> master
fi
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=/tmp/netcdf"

if test "x$DAP" = x ; then
FLAGS="$FLAGS -DENABLE_DAP=false"
fi
if test "x$NC4" = x ; then
FLAGS="$FLAGS -DENABLE_NETCDF_4=false"
else
ignore=1
#FLAGS="-DHDF5_C_LIBRARY=${NCC}/lib/hdf5 -DHDF5_HL_LIBRARY=${NCC}/lib/hdf5_hl -DHDF5_INCLUDE_DIR=${NCC}/include"
fi
if test "x$CDF5" != x ; then
FLAGS="$FLAGS -DENABLE_CDF5=true"
fi
if test "x$HDF4" != x ; then
FLAGS="$FLAGS -DENABLE_HDF4=true"
fi

if test "x$TESTSERVERS" != x ; then
FLAGS="$FLAGS -DREMOTETESTSERVERS=${TESTSERVERS}"
fi

# Enables
FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
FLAGS="$FLAGS -DENABLE_LOGGING=true"
#FLAGS="$FLAGS -DENABLE_DOXYGEN=true -DENABLE_INTERNAL_DOCS=true"
#FLAGS="$FLAGS -DENABLE_LARGE_FILE_TESTS=true"
#FLAGS="$FLAGS -DENABLE_FILTER_TESTING=true"

# Disables
FLAGS="$FLAGS -DENABLE_EXAMPLES=false"
FLAGS="$FLAGS -DENABLE_CONVERSION_WARNINGS=false"
#FLAGS="$FLAGS -DENABLE_TESTS=false"
#FLAGS="$FLAGS -DENABLE_DISKLESS=false"
FLAGS="$FLAGS -DBUILD_UTILITIES=true"

# Withs
FLAGS="$FLAGS -DNCPROPERTIES_EXTRA=\"key1=value1|key2=value2\""

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
cmake ${TR} "$G" -DCMAKE_BUILD_TYPE=${CFG} $FLAGS ..
if test "x$NOBUILD" = x ; then
#cmake ${TR} --build . --config ${CFG} --target ZERO_CHECK
cmake ${TR} --build . --config ${CFG} --target ALL_BUILD
if test "x$NOTEST" = x ; then
cmake ${TR} --build . --config ${CFG} --target RUN_TESTS
fi
fi
else
# GCC
NCLIB="${NCLIB}/build/liblib"
#G="-GUnix Makefiles"
#T="--trace-expand"
cmake "${G}" $FLAGS ..
if test "x$NOBUILD" = x ; then
make all
make test
fi
fi
exit
