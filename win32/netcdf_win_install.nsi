!define VERSION "3.6.0beta2"
!define INSTDIR "c:\WINNT\system32"
Name "netCDF ${VERSION}"
OutFile "netcdf-${VERSION}.exe"
LicenseData license.txt

BGGradient

Page license
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles


Section "netcdf"
  SetOutPath $INSTDIR
  File "NET\Debug\netcdf.dll"
SectionEnd

OutFile setup.exe







