#define DEBUG_MODE 1  // use localhost

/* Defined variables */
#define MINMODE 0  /* min,max modes for our operation */
#define MAXMODE 7

/* Include files */

#include <stdio.h>
#include <string.h>

#include "/usr2/fs/include/params.h" /* FS parameters            */
#include "/usr2/fs/include/fs_types.h" /* FS header files        */
#include "/usr2/fs/include/fscom.h"  /* FS shared mem. structure */
#include "/usr2/fs/include/shm_addr.h" /* FS shared mem. pointer */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "../apecs/apecs_iface.c" // TODO: use header file...

// APEX general control computer
#define APEXTRANSHOST    "10.0.2.165" // IP for apexdev2.apex-telescope.org
#define APEXTRANSPORT    22122        // apexdev2.apex-telescope.org:5000
#define APEXTRANSTIMEOUT 3000000      // 500e3 usec = 0.5s, 1e6 usec = 1s
#define ANTCNLOCALNAME   "mark5c1.apex-telescope.org"
#define ANTCNLOCALPORT   22122
#define DEBUGTRANSHOST   "10.0.2.95" //"127.0.0.1"

#if (DEBUG_MODE)
   #undef  APEXTRANSHOST
   #define APEXTRANSHOST DEBUGTRANSHOST
#endif

/* locals */
#define ANTCN_STR_LEN 1024
static char* apecs_trackmode_prev;
static char* apecs_trackmode;
static char* apecs_cmd;
static char* apecs_resp;

main()
{
  int ierr, nrec, nrecr;
  int dum = 0;
  int r1, r2;
  int imode,i,nchar;
  long ip[5], class, clasr;
  char buf[80], buf2[300];
  char buf3[300];
  char sorname[20];
  static char outra[15], outdec[15]; /*check len of these! */

  int n,iret,n1,n2,nepoch;
  apecs_fd_t* afd;

/* Allocations */
  apecs_trackmode_prev = malloc(ANTCN_STR_LEN);
  apecs_trackmode = malloc(ANTCN_STR_LEN);
  apecs_cmd = malloc(ANTCN_STR_LEN);
  apecs_resp = malloc(ANTCN_STR_LEN);
  memset(apecs_trackmode_prev, 0, ANTCN_STR_LEN);
  memset(apecs_trackmode, 0, ANTCN_STR_LEN);
  memset(apecs_cmd, 0, ANTCN_STR_LEN);
  memset(apecs_resp, 0, ANTCN_STR_LEN);

/* APECS hook-up */
  printf("Connecting: %s\n", APEXTRANSHOST);
  afd = apecs_connect(APEXTRANSHOST, APEXTRANSPORT, /*timeout usec=*/APEXTRANSTIMEOUT, ANTCNLOCALPORT);
  printf("Sending: execfile()\n");
  n = apecs_send(afd, "execfile('commands/vlbi_commands.apecs')\n", apecs_resp,ANTCN_STR_LEN);
  printf("Reply: %s\n", apecs_resp);

  while (1) {
      // send request and receive (hopefully) and "OK"
      sprintf(apecs_cmd, "vlbi_get_tracking_status('%s',%d)\n", ANTCNLOCALNAME, ANTCNLOCALPORT);
      printf("Sending: %s\n", apecs_cmd);
      n = apecs_send(afd, apecs_cmd, apecs_resp,ANTCN_STR_LEN);
      // if(strncasecmp(apecs_resp,"OK")||timeout) { /*try again N times?*/ }
      printf("Reply: %s\n", apecs_resp);

      // wait for second reply with the tracking information (arrives hopefully...)
      apecs_recv_req(afd, apecs_cmd, apecs_trackmode, ANTCN_STR_LEN);
      printf("Received: %s\n", apecs_trackmode);
  }
  return 0;
}

