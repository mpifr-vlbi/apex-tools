/* dbbcrelink SNAP command */

#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"         /* shared memory definition */
#include "/usr2/fs/include/shm_addr.h"      /* shared memory pointer */

#include "/usr2/st/include/stparams.h"
#include "/usr2/st/include/stcom.h"
#include "/usr2/st/include/stm_addr.h"   /* Station shared mem. pointer*/

#define BUFSIZE 512

void dbbcrelink(command,itask,ip)
struct cmd_ds *command;                /* parsed command structure */
int itask;                            /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int ilast, ierr, ind, i, count;
      char *ptr;
      char *arg_next();

      void skd_run(), skd_par();      /* program scheduling utilities */

      if (command->equal == '=' ) {
	ierr=-301;
	goto error;
      }

/* if we get this far it is a set-up command so parse it */


dbbccn:
      printf("DEBUG in dbbcrelink\n");
      ip[0]=2;
      ip[1]=0;
      ip[2]=0;
      skd_run("dbbccn",'w',ip);
      skd_par(ip);

      /* allow schedule to continue if no error */
      if(ip[2]==0)
	shm_addr->KHALT=0;

      return;

error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"5l",2);
      return;
}
