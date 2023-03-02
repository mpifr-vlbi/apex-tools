/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "yacc_maserhousing.h"
#include "cid_maserhousing.h"

void bufncat(), bufputc();

struct usage_db {
    char *text;
    int  number;
} usage_table[] = {
{"STATUS? ", STATUS}, 
{"TEMPERATURE? ", TEMPERATURE},
{"HUMIDITY? ", HUMIDITY},
{"CMDTEMPERATURE? ", CMDTEMPERATURE},
{"TEMPDUTYCYCLE? ", TEMPDUTYCYCLE},
{"CMDHUMIDITY? ", CMDHUMIDITY},
{"HUMMIDDUTYCYCLE? ", HUMIDDUTYCYCLE},
{NULL, 0}
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
