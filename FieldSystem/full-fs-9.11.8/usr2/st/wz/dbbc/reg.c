/*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command called by  stqkr.c
***********************
reg=  DBBC command
***********************
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>

#include "/usr2/fs//include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"         /* fs-shared memory definition */
#include "/usr2/fs/include/shm_addr.h"      /* fs-shared memory pointer */
#include "/usr2/st/include/stcom.h"         /* st-shared memory definition */
#include "/usr2/st/include/stm_addr.h"
#define BUFSIZE 256
#define DEBUG 0     /* additional prinf for debug mode when 1 */


/*****************************/
void reg (command,itask,ip)
/*****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
          /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int ilast, ierr, ind, i, count, ilen;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char inbuf[BUFSIZE];

      void skd_run(), skd_par();      /* program scheduling utilities */
      
      strcpy(outbuf,command->name);
      if (command->argv[0] == NULL || command->equal != '=' )
       /* get dbbcstatus, command is only 'dbbcform' or dbbcform= */   
      {
      logitf("dbbcstat: no core module selected");
      goto exit;
      } 
/* if we get this far it is a set-up command so parse it */
parse:
      ilast=0;                                      /* last argv examined */
      out_recs=0;
      out_class=0;
      ptr=arg_next(command,&ilast);
      
       /* command has passed the stqkr */
      if (command->equal == '=')
         strcat(outbuf,"="); 
     
      while( ptr != NULL) {  
	strcat(outbuf,ptr);
 	out_recs++;
	ptr=arg_next(command,&ilast);
	if (ptr != NULL)
	strcat(outbuf,",");
      } 
      /* outbuf contains complete command */
      /* currently there is no command parsing */            
      ilen=strlen(outbuf);
      ierr = dbbc_com(ilen, outbuf, inbuf);
      /* inbuf contains server msg */
      if (ierr !=0)
        {
        goto error;
        }
/*      logitf(inbuf);  */        

      
exit: for (i=0;i<5;i++) ip[i]=0;
/*      cls_snd(&ip[0],outbuf, strlen(outbuf),0,0);  */
      cls_snd(&ip[0],inbuf, strlen(inbuf),0,0);
      ip[1]=1;
      return;

error:
      ip[0]=0;
      ip[1]=1;
      ip[2]=ierr;
      memcpy(ip+3,"dbbcform",8);
      return;      
}

