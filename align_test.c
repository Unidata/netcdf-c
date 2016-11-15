#include <stdio.h>
#include <stdlib.h>

#define is_aligned(POINTER, BYTE_COUNT) \
  (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

typedef struct {
  float lat;
  float lon;
  char* stid;
  double time;
  float temp;
} obs_t;

int main (int argc, char **argv) {

  obs_t p;

  printf("IS_ALIGNED (p,4): %d\n",is_aligned(&p,4u));
  printf("IS_ALIGNED (p,8): %d\n",is_aligned(&p,8u));
  printf("IS_ALIGNED (p,16): %d\n",is_aligned(&p,16u));
  printf("IS_ALIGNED (p,32): %d\n",is_aligned(&p,32u));
  printf("IS_ALIGNED (p,64): %d\n",is_aligned(&p,64u));
  printf("Finished.\n");

}
