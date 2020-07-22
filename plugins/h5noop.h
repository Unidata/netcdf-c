#ifndef H5MISC_H
#define H5MISC_H

#ifdef _WIN32
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

#endif /*H5MISC_H*/
