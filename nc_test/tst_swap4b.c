/*! Test swap4b function.
 *
 * Test added July 27, 2015.
 *
 * This test confirms that the function ncx.c:swap4b() is
 * working properly. For more information regarding this test
 * and the circumstances prompting it, see
 * https://bugtracking.unidata.ucar.edu/browse/NCF-338
 */

#include "netcdf.h"

#define SWAP4(a) ( ((a) << 24) |                \
                   (((a) <<  8) & 0x00ff0000) | \
                   (((a) >>  8) & 0x0000ff00) | \
                   (((a) >> 24) & 0x000000ff) )

/* Taken from ncx.c */
void swap4b(void *dst, const void *src);

#include <stdio.h>

/*! The 'Old' swap4b algorithm.
 * The naive algorithm.  Replicated here to test
 * CPU bounds.
 */
void old_swap4b(void *dst, const void *src) {
  char *op = dst;
  const char *ip = src;
  op[0] = ip[3];
  op[1] = ip[2];
  op[2] = ip[1];
  op[3] = ip[0];
}

int main() {


  unsigned int first = 0x0000ffff;
  unsigned int check_first = 0xffff0000;

  unsigned int second = 0xffff0000;
  unsigned int check_second = 0x0000ffff;

  unsigned int third = 0x00abcdef;
  unsigned int check_third = 0xefcdab00;

  unsigned int rev_first = 0;
  unsigned int rev_second = 0;
  unsigned int rev_third = 0;

  printf("\n");
  printf("First:\t0x%08x...\n",first);
  printf("Second:\t0x%08x...\n",second);
  printf("Third:\t0x%08x...\n",third);

  printf("\nReversing Unsigned Integers with old swap4b\n");
  old_swap4b(&rev_first,&first);
  old_swap4b(&rev_second,&second);
  old_swap4b(&rev_third,&third);

  printf("1. Reversing 0x%08x: 0x%08x ...\t",first,rev_first);
  if(rev_first == check_first) {
    printf("Success!\n");
  } else {
    printf("Failure!\n");
    return -1;
  }

  printf("2. Reversing 0x%08x: 0x%08x ...\t",second,rev_second);
  if(rev_second == check_second) {
    printf("Success!\n");
  } else {
    printf("Failure!\n");
    return -1;
  }

  printf("3. Reversing 0x%08x: 0x%08x ...\t",third,rev_third);
  if(rev_third == check_third) {
    printf("Success!\n");
  } else {
    printf("Failure!\n");
    return -1;
  }

  /* Zero out results, check with bit-shifting swap4b. */
  rev_first = 0;
  rev_second = 0;
  rev_third = 0;

  printf("\nReversing Unsigned Integers with new swap4b\n");
  swap4b(&rev_first,&first);
  swap4b(&rev_second,&second);
  swap4b(&rev_third,&third);

  printf("1. Reversing 0x%08x: 0x%08x ...\t",first,rev_first);
  if(rev_first == check_first) {
    printf("Success!\n");
  } else {
    printf("Failure!\n");
    return -1;
  }

  printf("2. Reversing 0x%08x: 0x%08x ...\t",second,rev_second);
  if(rev_second == check_second) {
    printf("Success!\n");
  } else {
    printf("Failure!\n");
    return -1;
  }

  printf("3. Reversing 0x%08x: 0x%08x ...\t",third,rev_third);
  if(rev_third == check_third) {
    printf("Success!\n");
  } else {
    printf("Failure!\n");
    return -1;
  }

  printf("\nFinished.\n");
  return 0;

}
