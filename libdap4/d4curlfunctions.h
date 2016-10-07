/*********************************************************************
  *   Copyright 2016, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *********************************************************************/

#ifndef D4CURLFUNCTIONS_H
#define D4CURLFUNCTIONS_H

enum CURLFLAGTYPE {CF_UNKNOWN=0,CF_OTHER=1,CF_STRING=2,CF_LONG=3};
struct CURLFLAG {
    const char* name;
    int flag;
    int value;
    enum CURLFLAGTYPE type;
};

extern ncerror ncd4_set_curlopt(NCD4INFO* state, int flag, void* value);

extern ncerror ncd4_set_flags_perfetch(NCD4INFO*);
extern ncerror ncd4_set_flags_perlink(NCD4INFO*);

extern ncerror ncd4_set_curlflag(NCD4INFO*,int);
extern ncerror dcd4_set_curlstate(NCD4INFO* state, int flag, void* value);

extern void ncd4_curl_debug(NCD4INFO* state);

extern struct CURLFLAG* ncd4_curlflagbyflag(int flag);
extern struct CURLFLAG* ncd4_curlflagbyname(const char* name);

#endif /*D4CURLFUNCTIONS_H*/
