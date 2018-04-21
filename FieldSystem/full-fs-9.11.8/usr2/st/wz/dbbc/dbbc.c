/*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command dbbc called by  stqkr.c
***********************
DBBC=xxxxxxxx   
***********************
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>
#include <math.h>
#include <signal.h>

#include "/usr2/fs/include/dpi.h"
#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"         /* fs-shared memory definition */
#include "/usr2/fs/include/shm_addr.h"      /* fs-shared memory pointer */
#include "/usr2/st/include/stcom.h"         /* st-shared memory definition */
#include "/usr2/st/include/stm_addr.h"      /* st-shared memory pointer */
#define BUFSIZE 256
#define DEBUG 1    		/* additional prinf for debug mode when 1 */
extern struct stcom *st;
extern struct fscom *fs;

#include "simple_socket.h"

/*****************************/
void dbbc(command,itask,ip)     /* DBBC='dirct string'*/
/****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
                                      /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int i, ilast, ilen, ierr=0;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char buf[BUFSIZE];
      char inbuf[BUFSIZE];
      void skd_run(), skd_par();     	 /* program scheduling utilities */
      void logitf();
      if (DEBUG) printf("from dbbc.c\n");       
      strcpy(outbuf,command->name);
      
      if (command->argv[0] == NULL || command->equal != '=' )
      
      /*get dbbc status, FS-command is only 'dbbc'  without arguments */   
       {
         ilen=strlen(outbuf);
         if (DEBUG) printf("dbbc:outbuf %s ilen=%d \n", outbuf, ilen);
         
         /*dbbc_com() is the client function, uses functions in simple_sockt.c */
         ierr = dbbc_com(ilen, outbuf, inbuf);
         if (ierr == 0)
           {
             if (DEBUG) {
                printf("dbbc.c cmd=dbbc general status request\n");           
                printf("msg from DBBC:ierr=%d outbuf= %s inbuf= %s\n",ierr,outbuf, inbuf);
             }
             /* msg from dbbc received, doe something with status request
             and exit */
             logitf(inbuf);
             goto exit;
           }
         else
            {
            logitf("dbbc.c: Client communication ERROR \n");      
            goto error;
            }
        }    
/* if we get this far it is a dbbc=xxx... set-up command so parse it */
/*parse:*/
      ilast=0;                       
      out_recs=0;
      out_class=0;
      ptr=arg_next(command,&ilast);
      
      if (command->equal == '=') 
        strcat(outbuf,"=");
        
      /* check number of dbbcnn= commands, separated by ',' */ 
      while( ptr != NULL) {
	strcat(outbuf,ptr);
 	out_recs++;
	ptr=arg_next(command,&ilast);
	if (ptr != NULL)
	   strcat(outbuf,",");
	
      }
        ilen=strlen(outbuf);
         /*sent outbuf to DBBC*/
         if (DEBUG) printf("dbbc:outbuf %s\n", outbuf);
        ierr = dbbc_com(ilen, outbuf, inbuf); 
         if (ierr != 0)
           {
           
             goto error;
           }
           if (DEBUG)
               printf("dbbc.c: :ierr=%d outbuf= %s inbuf= %s\n",ierr,outbuf, inbuf);
           logitf(inbuf);
  
exit:
      for (i=0;i<5;i++) ip[i]=0;
      cls_snd(&ip[0],outbuf,strlen(outbuf),0,0);
      ip[1]=1;
      return;
error:
      if (DEBUG) printf("dbbc: ERROR, ierr=%d\n",ierr);
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"dbbc",4); 
      return;


  }
