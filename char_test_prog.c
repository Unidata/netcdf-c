#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


int main(void) {
  int char_is_signed = 0;
  char thischar;
  if(CHAR_MAX < 255) {
    char_is_signed = 1;
  }

  printf("-------------------\n");
  printf("Unsigned\n");
  printf("-------------------\n");
  printf("UCHAR_MAX: %hhu\n",UCHAR_MAX);
  printf("Min unsigned char: %hhu\n",0);
  printf("Min UBYTE-1: %hhu\n",-1);
  printf("Max UBYTE: %hhu\n",UCHAR_MAX);
  printf("Max UBYTE+1: %hhu\n", UCHAR_MAX+1);

  if(char_is_signed == 1) {
   printf("-------------------\n");
   printf("Default char is signed\n");
   printf("-------------------\n");


   printf("CHAR_MAX: %hhd\n",CHAR_MAX);
   printf("Min char: %hhd\n",CHAR_MIN);
   printf("Min char-1: %hhd\n",CHAR_MIN-1);
   printf("Max char: %hhd\n",CHAR_MAX);
   printf("Max char+1: %hhd\n", CHAR_MAX+1);
} else {
 printf("-------------------\n");
   printf("Default char is unsigned\n");
   printf("-------------------\n");


   printf("CHAR_MAX: %hhu\n",CHAR_MAX);
   printf("Min char: %hhu\n",CHAR_MIN);
   printf("Min char-1: %hhu\n",CHAR_MIN-1);
   printf("Max char: %hhu\n",CHAR_MAX);
   printf("Max char+1: %hhu\n", CHAR_MAX+1);

  }

  printf("-------------------\n");
  printf("Signed\n");
  printf("-------------------\n");
  printf("SCHAR_MAX: %hhd\n",SCHAR_MAX);
  printf("Min char: %hhd\n",SCHAR_MIN);
  printf("Min char-1: %hhd\n",SCHAR_MIN-1);
  printf("Max char: %hhd\n",SCHAR_MAX);
  printf("Max char+1: %hhd\n", SCHAR_MAX+1);




}
