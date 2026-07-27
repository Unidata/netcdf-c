/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
unit tests for libdispatch/dauth.c
*/

#include "../libdispatch/dauth.c" //To test static function setauthfield

typedef struct Test {
  char *field;
  char *value;
  NCauth initial;
  NCauth expected;
} Test;

#define TEST_OK 0
#define TEST_ERROR 1

#define extract_auth_field(a, f) a.f

NCauth testauth = {0};
const NCauth auth_off = {0};
const NCauth auth_on = {.ssl = {.verifypeer = 1, .verifyhost = 2}};
const NCauth auth_default_ssl = {.ssl = {.verifypeer = -1, .verifyhost = -1}};

const NCauth auth_host_on = {.ssl = {.verifyhost = 1}};
const NCauth auth_host_default = {.ssl = {.verifyhost = -1}};

const NCauth auth_peer_on = {.ssl = {.verifypeer = 1}};
const NCauth auth_peer_default = {.ssl = {.verifypeer = -1}};

const NCauth untouched = {{0xF}};

static Test TESTS[] = {
  // SSL VALIDATE -> changes both verifypeer and verifyhost
  /* 1 */ {"HTTP.SSL.VALIDATE", "1",     auth_off, auth_on},
  /* 2 */ {"HTTP.SSL.VALIDATE", "yes",   auth_off, auth_on},
  /* 3 */ {"HTTP.SSL.VALIDATE", "YES",   auth_off, auth_on},
  /* 4 */ {"HTTP.SSL.VALIDATE", "true",  auth_off, auth_on},
  /* 5 */ {"HTTP.SSL.VALIDATE", "True",  auth_off, auth_on},
  /* 6 */ {"HTTP.SSL.VALIDATE", "0",     auth_on, auth_off},
  /* 7 */ {"HTTP.SSL.VALIDATE", "No",    auth_on, auth_off},
  /* 8 */ {"HTTP.SSL.VALIDATE", "no",    auth_on, auth_off},
  /* 9 */ {"HTTP.SSL.VALIDATE", "false", auth_on, auth_off},
  /*10 */ {"HTTP.SSL.VALIDATE", "False", auth_on, auth_off},
  /*11 */ {"HTTP.SSL.VALIDATE", "",      auth_on, auth_default_ssl},
  /*12 */ {"HTTP.SSL.VALIDATE", NULL, untouched, untouched},

  // SSL VERIFY HOST
  /*13 */ {"HTTP.SSL.VERIFYHOST", "1",     auth_off, auth_host_on},
  /*14 */ {"HTTP.SSL.VERIFYHOST", "true",  auth_off, auth_host_on},
  /*15 */ {"HTTP.SSL.VERIFYHOST", "True",  auth_off, auth_host_on},
  /*16 */ {"HTTP.SSL.VERIFYHOST", "0",     auth_host_on, auth_off},
  /*17 */ {"HTTP.SSL.VERIFYHOST", "false", auth_host_on, auth_off},
  /*18 */ {"HTTP.SSL.VERIFYHOST", "False", auth_host_on, auth_off},
  /*19 */ {"HTTP.SSL.VERIFYHOST", "",      auth_off, auth_host_default},
  /*20 */ {"HTTP.SSL.VERIFYHOST", NULL,    auth_on, auth_on},

  // SSL VERIFY PEER
  /*21 */ {"HTTP.SSL.VERIFYPEER", "1",     auth_off, auth_peer_on},
  /*22 */ {"HTTP.SSL.VERIFYPEER", "True",  auth_off, auth_peer_on},
  /*23 */ {"HTTP.SSL.VERIFYPEER", "0",     auth_peer_on, auth_off},
  /*24 */ {"HTTP.SSL.VERIFYPEER", "false", auth_peer_on, auth_off},
  /*25 */ {"HTTP.SSL.VERIFYPEER", "",      auth_off, auth_peer_default},
  /*26 */ {"HTTP.SSL.VERIFYPEER", NULL, untouched, untouched},
  // END
  { NULL, NULL, {{0}}, {{0} } }
};

void auth_print(const NCauth *auth, FILE *f) {
  if (f == NULL) {
    f = stdout;
  }
  fprintf(f, "NCAuth:\n");
  fprintf(f,
          "\tHTTP.SSL.VERIFYPEER:%d\n"
          "\tHTTP.SSL.VERIFYHOST:%d\n",
          auth->ssl.verifypeer, auth->ssl.verifyhost);
}

int main(int argc, char **argv) {
  Test *test;
  int failcount = 0;
  int index;

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
