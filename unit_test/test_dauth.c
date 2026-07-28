/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
unit tests for libdispatch/dauth.c
*/

#include "../libdispatch/dauth.c" //To test static function setauthfield

#define STR_NULL(x) ((x == NULL) ? "NULL" : x)
#define AUTH_OFF {{0}}
#define AUTH_ON {.ssl = {.verifypeer = 1, .verifyhost = 2}}
#define AUTH_DEFAULT_SSL {.ssl = {.verifypeer = -1, .verifyhost = -1}}

#define AUTH_HOST_ON {.ssl = {.verifyhost = 1}}
#define AUTH_HOST_DEFAULT {.ssl = {.verifyhost = -1}}

#define AUTH_PEER_ON {.ssl = {.verifypeer = 1}}
#define AUTH_PEER_DEFAULT {.ssl = {.verifypeer = -1}}
#define AUTH_UNTOUCHED {{0xF}}

typedef struct Test {
  char *field;
  char *value;
  int code;
  NCauth initial;
  NCauth expected;
} Test;

static Test TESTS[] = {
  // SSL VALIDATE -> changes both verifypeer and verifyhost
  /* 1 */ {"HTTP.SSL.VALIDATE", "1",        NC_NOERR, AUTH_OFF, AUTH_ON},
  /* 2 */ {"HTTP.SSL.VALIDATE", "yes",      NC_NOERR, AUTH_OFF, AUTH_ON},
  /* 3 */ {"HTTP.SSL.VALIDATE", "YES",      NC_NOERR, AUTH_OFF, AUTH_ON},
  /* 4 */ {"HTTP.SSL.VALIDATE", "true",     NC_NOERR, AUTH_OFF, AUTH_ON},
  /* 5 */ {"HTTP.SSL.VALIDATE", "True",     NC_NOERR, AUTH_OFF, AUTH_ON},
  /* 6 */ {"HTTP.SSL.VALIDATE", "on",       NC_NOERR, AUTH_OFF, AUTH_ON},
  /* 7 */ {"HTTP.SSL.VALIDATE", "0",        NC_NOERR, AUTH_ON, AUTH_OFF},
  /* 8 */ {"HTTP.SSL.VALIDATE", "No",       NC_NOERR, AUTH_ON, AUTH_OFF},
  /* 9 */ {"HTTP.SSL.VALIDATE", "no",       NC_NOERR, AUTH_ON, AUTH_OFF},
  /*10 */ {"HTTP.SSL.VALIDATE", "false",    NC_NOERR, AUTH_ON, AUTH_OFF},
  /*11 */ {"HTTP.SSL.VALIDATE", "False",    NC_NOERR, AUTH_ON, AUTH_OFF},
  /*12 */ {"HTTP.SSL.VALIDATE", "off",      NC_NOERR, AUTH_ON, AUTH_OFF},
  /*13 */ {"HTTP.SSL.VALIDATE", "",       NC_ERCFILE, AUTH_ON, AUTH_DEFAULT_SSL},
  /*14 */ {"HTTP.SSL.VALIDATE", NULL,       NC_NOERR, AUTH_ON, AUTH_ON},
  // SSL VERIFY HOST
  /*15 */ {"HTTP.SSL.VERIFYHOST", "1",      NC_NOERR, AUTH_OFF, AUTH_HOST_ON},
  /*16 */ {"HTTP.SSL.VERIFYHOST", "true",   NC_NOERR, AUTH_OFF, AUTH_HOST_ON},
  /*17 */ {"HTTP.SSL.VERIFYHOST", "True",   NC_NOERR, AUTH_OFF, AUTH_HOST_ON},
  /*18 */ {"HTTP.SSL.VERIFYHOST", "0",      NC_NOERR, AUTH_HOST_ON, AUTH_OFF},
  /*19 */ {"HTTP.SSL.VERIFYHOST", "false",  NC_NOERR, AUTH_HOST_ON, AUTH_OFF},
  /*20 */ {"HTTP.SSL.VERIFYHOST", "False",  NC_NOERR, AUTH_HOST_ON, AUTH_OFF},
  /*21 */ {"HTTP.SSL.VERIFYHOST", "",     NC_ERCFILE, AUTH_OFF, AUTH_HOST_DEFAULT},
  /*22 */ {"HTTP.SSL.VERIFYHOST", NULL,     NC_NOERR, AUTH_HOST_ON, AUTH_HOST_ON},
  // SSL VERIFY PEER
  /*23 */ {"HTTP.SSL.VERIFYPEER", "1",     NC_NOERR, AUTH_OFF, AUTH_PEER_ON},
  /*24 */ {"HTTP.SSL.VERIFYPEER", "True",  NC_NOERR, AUTH_OFF, AUTH_PEER_ON},
  /*25 */ {"HTTP.SSL.VERIFYPEER", "0",     NC_NOERR, AUTH_PEER_ON, AUTH_OFF},
  /*26 */ {"HTTP.SSL.VERIFYPEER", "false", NC_NOERR, AUTH_PEER_ON, AUTH_OFF},
  /*27 */ {"HTTP.SSL.VERIFYPEER", "",    NC_ERCFILE, AUTH_OFF, AUTH_PEER_DEFAULT},
  /*28 */ {"HTTP.SSL.VERIFYPEER", NULL,    NC_NOERR, AUTH_PEER_ON, AUTH_PEER_ON},
  // Test invalid fields!
  /*29 */ {"HTTP.SSL.VALIDATE",   "BAD", NC_ERCFILE, AUTH_ON, AUTH_DEFAULT_SSL},
  /*30 */ {"HTTP.SSL.VERIFYPEER", "BAD", NC_ERCFILE, AUTH_PEER_ON, AUTH_PEER_DEFAULT},
  /*31 */ {"HTTP.SSL.VERIFYHOST", "BAD", NC_ERCFILE, AUTH_HOST_ON, AUTH_HOST_DEFAULT},
  // END
  { NULL, NULL, 0, {{0}}, {{0} } }
};

void auth_print(const NCauth *auth, FILE *f) {
  if (f == NULL) {
    f = stdout;
  }
  fprintf(f,
          "NCAuth:\n"
          "\tHTTP.SSL.VERIFYPEER:%d\n"
          "\tHTTP.SSL.VERIFYHOST:%d\n",
          auth->ssl.verifypeer, auth->ssl.verifyhost);
}

int main(int argc, char **argv) {
  Test *test;
  int failcount = 0;
  int index;
  NCauth testauth = {0};

  for (index = 0, test = TESTS; test->field; test++, index++) {
    int ret = 0;
    memcpy(&testauth, &test->initial, sizeof(test->initial));
    ret = setauthfield(&testauth, test->field, test->value);
    if (ret != test->code) {
      fprintf(
          stderr,
          "Failed test #%d, setauthfield returned %d instead of %d (%s = %s)\n",
          index + 1, ret, test->code, test->field, STR_NULL(test->value));
      failcount++;
    }
    if (memcmp(&testauth, &test->expected, sizeof(test->expected))) {
      fprintf(stderr, "Failed test #%d, %s = \"%s\" not set properly\n",
              index + 1, test->field, STR_NULL(test->value));
      fprintf(stderr, "Expected:\n");
      auth_print(&test->expected, stderr);
      fprintf(stderr, "Got:\n");
      auth_print(&testauth, stderr);
      failcount++;
    }
  }

  fprintf(stderr, "%s test_dauth\n", failcount > 0 ? "***FAIL" : "***PASS");
  return (failcount > 0 ? 1 : 0);
}
