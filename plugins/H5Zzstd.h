#ifndef H5ZSTD_H
#define H5ZSTD_H

#include "zstd.h"

#ifdef _MSC_VER
  #ifdef DLL_EXPORT /* define when building the library */
    #define DECLSPEC __declspec(dllexport)
  #else
    #define DECLSPEC __declspec(dllimport)
  #endif
#else
  #define DECLSPEC extern
#endif

/* declare the hdf5 interface */
DECLSPEC H5PL_type_t H5PLget_plugin_type(void);
DECLSPEC const void* H5PLget_plugin_info(void);
DECLSPEC const H5Z_class2_t H5Z_ZSTD[1]; 

#endif /*H5ZSTD_H*/

