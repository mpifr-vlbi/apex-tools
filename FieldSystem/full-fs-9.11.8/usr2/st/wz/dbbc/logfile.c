/*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command called by  stqkr.c
***********************
DBBC Command logfile=on/off
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
void logfile(command,itask,ip)
/*****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
          /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int ilast, ierr, ind, i, i1, i2, count, ilen;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char inbuf[BUFSIZE];

      void skd_run(), skd_par();      /* program scheduling utilities */
      
      strcpy(outbuf,command->name);
      if (command->argv[0] == NULL || command->equal != '=' )
       /* get dbbcstatus, command is only 'logfile ' or logfile= */   
      {
            ilen=strlen(outbuf);
            /*dbbc_com() is client function*/
            ierr = dbbc_com(ilen, outbuf, inbuf);
            if (ierr == 0)
              { /* msg from dbbc in inbuf */
              logitf(inbuf);
              goto exit;
              }
            else
              {
              logitf("dbbcform Client communication ERROR");            
              goto error;
              }
    
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
      /* outbuf contains command */
      
      if (out_recs > 1) {
         logitf("dbbc: logfile Input ERROR: Only on or off  allowed");
         goto exit;
      }          
      /* then out_recs=1 */
      /* is command on/off?*/
      i1=strcmp(command->argv[0],"on");
      if (i1 != 0) {
          i2=strcmp(command->argv[0],"off");
          if (i2 !=0) {
             logitf("allowed dbbc commands: logfile=on or logfile=off");
             goto exit;
          } 
      }
      ilen=strlen(outbuf);
      ierr = dbbc_com(ilen, outbuf, inbuf);
      if (ierr !=0)
        {
        goto error;
        }
      logitf(inbuf);        

      
exit: for (i=0;i<5;i++) ip[i]=0;
      cls_snd(&ip[0],outbuf, strlen(outbuf),0,0);
      ip[1]=1;
      return;

error:
      ip[0]=0;
      ip[1]=1;
      ip[2]=ierr;
      memcpy(ip+3,"logfile",7);
      return;      
}

