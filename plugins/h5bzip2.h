#ifndef H5BZIP2_H
#define H5BZIP2_H

#include "bzlib.h"

#ifdef _MSC_VER
  #ifdef DLL_EXPORT /* define when building the library */
    #define DECLSPEC __declspec(dllexport)
  #else
    #define DECLSPEC __declspec(dllimport)
  #endif
#else
  #define DECLSPEC extern
#endif

/* use an integer greater than 256 to be id of the registered filter. */
#define H5Z_FILTER_BZIP2 307

/* declare the hdf5 interface */
DECLSPEC H5PL_type_t H5PLget_plugin_type(void);
DECLSPEC const void* H5PLget_plugin_info(void);
DECLSPEC const H5Z_class2_t H5Z_BZIP2[1]; 

#endif /*H5BZIP2_H*/

