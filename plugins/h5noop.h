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

/* use an integer greater than 256 to be id of the registered filter. */
#define H5Z_FILTER_NOOP 40000

/* declare the hdf5 interface */
DECLSPEC H5PL_type_t H5PLget_plugin_type(void);
DECLSPEC const void* H5PLget_plugin_info(void);
DECLSPEC const H5Z_class2_t H5Z_NOOP[1]; 

/* Declare filter specific functions */
DECLSPEC htri_t H5Z_noop_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
DECLSPEC size_t H5Z_filter_noop(unsigned flags,size_t cd_nelmts,const unsigned cd_values[],
                    size_t nbytes,size_t *buf_size,void**buf);

#endif /*H5MISC_H*/
