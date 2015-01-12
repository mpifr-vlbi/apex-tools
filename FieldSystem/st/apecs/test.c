#include <stdio.h>
#include <string.h>

#include "apecs_iface.c"

// APEX general control computer
#define APEXTRANSHOST    "10.0.2.170" // IP for observer3.apex-telescope.org
#define APEXTRANSPORT    22122        // apexdev2.apex-telescope.org:5000
#define APEXTRANSTIMEOUT 3000000      // 500e3 usec = 0.5s, 1e6 usec = 1s
#define ANTCNLOCALPORT   22122

// APEX string passing
#define ANTCN_STR_LEN 1024
static char* apecs_trackmode_prev;
static char* apecs_trackmode;
static char* apecs_cmd;
static char* apecs_resp;

int main(int argc, char** argv)
{
  apecs_fd_t* afd;
  int n;

  // alloc
  apecs_trackmode_prev = malloc(ANTCN_STR_LEN);
  apecs_trackmode = malloc(ANTCN_STR_LEN);
  apecs_cmd = malloc(ANTCN_STR_LEN);
  apecs_resp = malloc(ANTCN_STR_LEN);
  memset(apecs_trackmode_prev, 0, ANTCN_STR_LEN);
  memset(apecs_trackmode, 0, ANTCN_STR_LEN);
  memset(apecs_cmd, 0, ANTCN_STR_LEN);
  memset(apecs_resp, 0, ANTCN_STR_LEN);

  afd = apecs_connect(APEXTRANSHOST, APEXTRANSPORT, /*timeout usec=*/APEXTRANSTIMEOUT, ANTCNLOCALPORT);

  strcpy(apecs_cmd, "apexObsUtils.getMCPoint('ABM1:ANTMOUNT:mode')\n");
  n = apecs_send(afd, apecs_cmd, apecs_trackmode,ANTCN_STR_LEN);

}

