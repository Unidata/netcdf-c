/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
unit tests for libdispatch/dauth.c
*/

#include "../libdispatch/dauth.c" //To test static function setauthfield

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
  NCauth initial;
  NCauth expected;
} Test;

static Test TESTS[] = {
  // SSL VALIDATE -> changes both verifypeer and verifyhost
  /* 1 */ {"HTTP.SSL.VALIDATE", "1",       AUTH_OFF, AUTH_ON},
  /* 2 */ {"HTTP.SSL.VALIDATE", "yes",     AUTH_OFF, AUTH_ON},
  /* 3 */ {"HTTP.SSL.VALIDATE", "YES",     AUTH_OFF, AUTH_ON},
  /* 4 */ {"HTTP.SSL.VALIDATE", "true",    AUTH_OFF, AUTH_ON},
  /* 5 */ {"HTTP.SSL.VALIDATE", "True",    AUTH_OFF, AUTH_ON},
  /* 6 */ {"HTTP.SSL.VALIDATE", "0",       AUTH_ON, AUTH_OFF},
  /* 7 */ {"HTTP.SSL.VALIDATE", "No",      AUTH_ON, AUTH_OFF},
  /* 8 */ {"HTTP.SSL.VALIDATE", "no",      AUTH_ON, AUTH_OFF},
  /* 9 */ {"HTTP.SSL.VALIDATE", "false",   AUTH_ON, AUTH_OFF},
  /*10 */ {"HTTP.SSL.VALIDATE", "False",   AUTH_ON, AUTH_OFF},
  /*11 */ {"HTTP.SSL.VALIDATE", "",        AUTH_ON, AUTH_DEFAULT_SSL},
  /*12 */ {"HTTP.SSL.VALIDATE", NULL,      AUTH_ON, AUTH_ON},
  // SSL VERIFY HOST
  /*13 */ {"HTTP.SSL.VERIFYHOST", "1",     AUTH_OFF, AUTH_HOST_ON},
  /*14 */ {"HTTP.SSL.VERIFYHOST", "true",  AUTH_OFF, AUTH_HOST_ON},
  /*15 */ {"HTTP.SSL.VERIFYHOST", "True",  AUTH_OFF, AUTH_HOST_ON},
  /*16 */ {"HTTP.SSL.VERIFYHOST", "0",     AUTH_HOST_ON, AUTH_OFF},
  /*17 */ {"HTTP.SSL.VERIFYHOST", "false", AUTH_HOST_ON, AUTH_OFF},
  /*18 */ {"HTTP.SSL.VERIFYHOST", "False", AUTH_HOST_ON, AUTH_OFF},
  /*19 */ {"HTTP.SSL.VERIFYHOST", "",      AUTH_OFF, AUTH_HOST_DEFAULT},
  /*20 */ {"HTTP.SSL.VERIFYHOST", NULL,    AUTH_HOST_ON, AUTH_HOST_ON},
  // SSL VERIFY PEER
  /*21 */ {"HTTP.SSL.VERIFYPEER", "1",     AUTH_OFF, AUTH_PEER_ON},
  /*22 */ {"HTTP.SSL.VERIFYPEER", "True",  AUTH_OFF, AUTH_PEER_ON},
  /*23 */ {"HTTP.SSL.VERIFYPEER", "0",     AUTH_PEER_ON, AUTH_OFF},
  /*24 */ {"HTTP.SSL.VERIFYPEER", "false", AUTH_PEER_ON, AUTH_OFF},
  /*25 */ {"HTTP.SSL.VERIFYPEER", "",      AUTH_OFF, AUTH_PEER_DEFAULT},
  /*26 */ {"HTTP.SSL.VERIFYPEER", NULL,    AUTH_PEER_ON, AUTH_PEER_ON},
  // END
  { NULL, NULL, {{0}}, {{0} } }
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
    // setdefaults(&testauth);
    ret = setauthfield(&testauth, test->field, test->value);
    // auth_print(testauth, NULL);
    if (ret != NC_NOERR) {
      fprintf(stderr, "Set auth fail: %s\n", test->field);
      failcount++;
    } else {
      if (memcmp(&testauth, &test->expected, sizeof(test->expected))) {
        fprintf(stderr, "Failed test #%d, %s = \"%s\" not set properly\n",
                index + 1, test->field,
                (test->value == NULL) ? "NULL" : test->value);
        fprintf(stderr, "Expected:\n");
        auth_print(&test->expected, stderr);
        fprintf(stderr, "Got:\n");
        auth_print(&testauth, stderr);
        failcount++;
      }
    }
  }

  fprintf(stderr, "%s test_dauth\n", failcount > 0 ? "***FAIL" : "***PASS");
  return (failcount > 0 ? 1 : 0);
}
