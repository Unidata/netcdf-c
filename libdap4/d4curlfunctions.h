/*********************************************************************
  *   Copyright 2018, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *********************************************************************/

/** @file d4curlfunctions.h
 * @brief libcurl option management for the DAP4 client.
 *
 * Declares the CURLFLAG table type and the functions that apply
 * per-fetch and per-link CURLOPT settings to a DAP4 connection.
 * @author Dennis Heimbigner
 */

#ifndef D4CURLFUNCTIONS_H
#define D4CURLFUNCTIONS_H

/* Aliases to older libcurl option names */
#ifndef HAVE_CURLOPT_KEYPASSWD
#define CURLOPT_KEYPASSWD CURLOPT_SSLKEYPASSWD /**< Compatibility alias for older libcurl. */
#endif
#ifndef HAVE_CURLINFO_RESPONSE_CODE
#define CURLINFO_RESPONSE_CODE CURLINFO_HTTP_CODE /**< Compatibility alias for older libcurl. */
#endif

/** Classification of a CURLFLAG value type. */
enum CURLFLAGTYPE {CF_UNKNOWN=0, CF_OTHER=1, CF_STRING=2, CF_LONG=3};

/** Descriptor for one named libcurl option flag. */
struct CURLFLAG {
    const char* name;       /**< Human-readable option name string. */
    int flag;               /**< CURLOPT_* constant value. */
    int value;              /**< Default numeric value (for CF_LONG flags). */
    enum CURLFLAGTYPE type; /**< Value type of this option. */
};

/** Set a single CURLOPT on @p state->curl->curl. */
extern ncerror NCD4_set_curlopt(NCD4INFO* state, int flag, void* value);

/** Apply all per-fetch CURLOPT settings (called before each HTTP request). */
extern ncerror NCD4_set_flags_perfetch(NCD4INFO* state);
/** Apply all per-link CURLOPT settings (called once when the handle is opened). */
extern ncerror NCD4_set_flags_perlink(NCD4INFO* state);

/** Apply one named curl flag from the CURLFLAG table to @p state. */
extern ncerror NCD4_set_curlflag(NCD4INFO* state, int flag);

/** Enable verbose libcurl debug output for @p state. */
extern void NCD4_curl_debug(NCD4INFO* state);

/** Look up a CURLFLAG descriptor by option name string. */
extern struct CURLFLAG* NCD4_curlflagbyname(const char* name);
/** Configure the allowed protocols on @p state's curl handle. */
extern void NCD4_curl_protocols(NCD4INFO* state);
/** Read RC-file properties and apply them to @p state. */
extern ncerror NCD4_get_rcproperties(NCD4INFO* state);

#endif /*D4CURLFUNCTIONS_H*/
