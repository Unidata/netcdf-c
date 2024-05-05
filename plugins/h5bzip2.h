#ifndef H5BZIP2_H
#define H5BZIP2_H

#include "bzlib.h"

#ifndef H5PLUGIN_DLL
#ifdef _MSC_VER
  #define H5PLUGIN_DLL __declspec(dllexport) extern
#else
  #define H5PLUGIN_DLL extern
#endif
#endif

/* use an integer greater than 256 to be id of the registered filter. */
#define H5Z_FILTER_BZIP2 307

/* declare the hdf5 interface */
H5PLUGIN_DLL H5PL_type_t H5PLget_plugin_type(void);
H5PLUGIN_DLL const void* H5PLget_plugin_info(void);
H5PLUGIN_DLL const H5Z_class2_t H5Z_BZIP2[1]; 

#endif /*H5BZIP2_H*/

