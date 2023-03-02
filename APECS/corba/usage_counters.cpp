/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "yacc_counters.h"
#include "cid_counters.h"

void bufncat(), bufputc();

struct usage_db {
    char *text;
    int  number;
} usage_table[] = {
"STATUS? ", STATUS, 
"STATE? ", STATE, 
"RESET ",RESET,
"GPSMINUSFMOUT?",GPSMINUSFMOUT,
"GPSMINUSMASER?",GPSMINUSMASER,
"CONFIGURE", CONFIGURE,
NULL, 0
};

void usage(int token)
{
  int i;
  for (i = 0; usage_table[i].number > 0; i++) {
    if (usage_table[i].number == token) {
      bufncat(usage_table[i].text, strlen(usage_table[i].text));
      bufputc('\n');
      break;
    }
  }
}
