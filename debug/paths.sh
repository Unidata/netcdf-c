case "$1" in
vs|VS|v) VS=1 ;;
linux|nix|l|x) unset VS ;;
*) echo "Must specify env: vs|linux"; return; ;;
esac

NCLIB=`pwd`
NCLIB="${NCLIB}/build/liblib"

if test "x$VS" = x ; then
export DISTCHECK_CONFIGURE_FLAGS='--prefix /usr/local --enable-extreme-numbers --enable-logging --disable-parallel4 --enable-shared'
fi

if test "x$VS" != x ; then
PATH="/cygdrive/c/tools/CMake/bin:$PATH"
PATH="/cygdrive/c/tools/nccmake/bin:$PATH"
if test -e /cygdrive/c/tools/libaec ; then
PATH="$PATH:/cygdrive/c/tools/libaec:$PATH"
fi
fi

PATH="${NCLIB}:$PATH"
#PATH="/cygdrive/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin:$PATH"
export PATH
VS=
