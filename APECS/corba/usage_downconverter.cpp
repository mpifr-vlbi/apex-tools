/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "yacc_downconverter.h"
#include "cid_downconverter.h"

void bufncat(), bufputc();

struct usage_db {
    char *text;
    int  number;
} usage_table[] = {
"STATUS? ", STATUS, 
"STATE? ", STATE, 
"CMDCENTERFREQ? ",CMDCENTERFREQ,
"CMDSYNTHFREQ? ",CMDSYNTHFREQ,
"CMDATTEN? ",CMDATTEN,
"ATTEN? ",ATTEN,
"CENTERFREQ? ",CENTERFREQ,
"SYNTHFREQ? ",SYNTHFREQ,
"CMDMODE? ",CMDMODE,
"MODE? ",MODE,
"INPUT? ",INPUT,
"BANDWIDTH? ",BANDWIDTH,
"CMDBANDWIDTH? ",CMDBANDWIDTH,
"FREQAXISFLIPPED? ",FREQAXISFLIPPED,
"LOCK? ",LOCK,
"AUTOLEVEL ",AUTOLEVEL,
"RESET ",RESET,
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
