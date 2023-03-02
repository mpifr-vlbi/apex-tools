/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "cid_counters.h"

void strsplit(char *str,char *str1,char *str2) 
{
    char vowel[2];

    strncpy(vowel, &str[3], 1);
    if ((int) strlen(str) <= 4) {
        strcpy(str1, str);
        strcpy(str2, "");
    } else if (strpbrk(vowel,"aeiouyAEIOUY") != 0) {
        strncpy(str1, str, 3);
        str1[3] = 0;
        strcpy(str2, &str[3]);
    } else {
        strncpy(str1, str, 4);
        str1[4] = 0;
        strcpy(str2, &str[4]);
    }
}
