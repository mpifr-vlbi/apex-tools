/* stqkr - C version of station command controller */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/fscom.h"
#include "../../fs/include/shm_addr.h"     /* shared memory pointer */

#include "../include/stparams.h"
#include "../include/stcom.h"

struct stcom *st;
struct fscom *fs;

#define MAX_BUF   257

main()
{
    long ip[5];
    long ipsave[5];
    long outclass;
    int isub,itask,idum,ierr,nchars,i;
    char buf[MAX_BUF];
    struct cmd_ds command;
    int cls_rcv(), cmd_parse();
    void skd_wait();
/*   char effstat[10]; *//* holds message >>RX etc. */

/* Set up IDs for shared memory, then assign the pointer to
 * "fs", for readability.
 */

  setup_ids();
  fs = shm_addr;
  setup_st();

loop:
      skd_wait("stqkr",ip,(unsigned) 0);
      if(ip[0]==0) {
        ierr=-1;
        goto error;
      }

      nchars=cls_rcv(ip[0],buf,MAX_BUF,&idum,&idum,0,0);
      if(nchars==MAX_BUF && buf[nchars-1] != '\0' ) { /*does it fit?*/
        ierr=-2;
        goto error;
      }
                                   /* null terminate to be sure */
      if(nchars < MAX_BUF && buf[nchars-1] != '\0') buf[nchars]='\0';

      if(0 != (ierr = cmd_parse(buf,&command))) { /* parse it */
        ierr=-3;
        goto error;
      } 
  

      isub = ip[1]/100;
      itask = ip[1] - 100*isub;

      switch (isub) {
//--------------------------------------------------------------------
         case 5:  /* DBBC Testversion, see usr2/control/stcmd.ctl, commands 5xx*/
                switch (itask) { 
                     case  0:  dbbcinit(&command,itask,ip); break;        
                     case 29:  dbbc_direct(&command,itask,ip); break;        
                     case 30:  dbbcrelink(&command,itask,ip); break; 
                     case 31:  dbbcclose(&command,itask,ip); break; 
             }                        
            break;
//--------------------------------------------------------------------
         default:
            ierr=-4;
            goto error;
      }
      goto loop;

error:
      for (i=0;i<5;i++) ip[i]=0;
      ip[2]=ierr;
      memcpy(ip+3,"s@",2);
      goto loop;
}
