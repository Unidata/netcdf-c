set PATH=%PATH%;C:\msys64\usr\bin

mkdir %SRC_DIR%\build
cd %SRC_DIR%\build

cmake -G "Visual Studio 15 Win64" ^
      -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% ^
      -D BUILD_SHARED_LIBS=ON ^
      -D ENABLE_TESTS=ON ^
      -D ENABLE_HDF4=ON ^
      -D CMAKE_PREFIX_PATH=%LIBRARY_PREFIX% ^
      -D ZLIB_LIBRARY=%LIBRARY_LIB%\zlib.lib ^
      -D ZLIB_INCLUDE_DIR=%LIBRARY_INC% ^
      -D CMAKE_BUILD_TYPE=Release ^
      %SRC_DIR%
if errorlevel 1 exit 1

cmake --build . --config Release || exit 1

ctest || exit 1

nmake install || exit 1
