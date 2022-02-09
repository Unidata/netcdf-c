#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
#include <windows.h>

int
main(int argc, char** argv)
{
   int acp = -1;
   const char* acpid = NULL;
   const char* lcall = NULL;
   const char* lcctype = NULL;
   char digits[16];

   switch (acp = GetACP()) {
   case 1252: acpid = "CP_1252"; break;
   case 65001: acpid = "CP_UTF8"; break;
   default:
	snprintf(digits,sizeof(digits),"%d",acp);
	acpid = digits;
	break;
   }

   lcall = setlocale(LC_ALL,NULL);
   lcctype = setlocale(LC_CTYPE,NULL);
   printf("ACP=%s locale: LC_ALL=%s LC_CTYPE=%s\n",acpid,lcall,lcctype);
   return 0;
}
