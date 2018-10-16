 /*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command called by  stqkr.c
**********************************************************
DBBCIF reports settings of IF modules
**********************************************************
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
#define DEBUG 0    		/* additional prinf for debug mode when 1 */

extern struct stcom *st;
extern struct fscom *fs;

/*****************************/
void dbbcif(command,itask,ip)    
/****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
                                      /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int iBuf_len;
      int i, ierr, ilast;
      float f1,f2;
      double ftmp=0;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char inbuf[BUFSIZE];
      void skd_run(), skd_par();     	 /* program scheduling utilities */
      void logitf();
      
      strcpy(outbuf,command->name);
      
      if (command->argv[0] == NULL || command->equal != '=' )
            /* command is only 'dbbcif' without arguments */   
         {
           if (DEBUG) printf("dbbcif.c: outbuf %s \n",outbuf);
           goto dbbc_cn;
         } 
/* if we get this far it is  command so parse it */
/*parse:*/
      ilast=0;                       
      out_recs=0;
      out_class=0;
      ptr=arg_next(command,&ilast);
      
      if (command->equal == '=') 
        strcat(outbuf,"=");
        
      /* check number of arguments, separated by ',' */ 
      while( ptr != NULL) {
 	out_recs++;
	ptr=arg_next(command,&ilast);
	
      }
      if (out_recs >=1) {
         logitf("dbbcif: no arguments allowed,only dbbc status request");
         goto error;   
      }  
 /* end of parsing */

dbbc_cn:
      *(inbuf)='\0';
      i=strlen(outbuf);
      iBuf_len = BUFSIZE;
      ierr=dbbc_com( iBuf_len, outbuf, inbuf);
      if (DEBUG)
       printf("dbbcif:Respons from DBBC= %s\n", inbuf);
      if (ierr != 0)
          goto error; 
   exit:
      for (i=0;i<5;i++) ip[i]=0;
      cls_snd(&ip[0],inbuf, strlen(inbuf),0,0);
      ip[1]=1;
      return;
   error:   
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"dbbcif",6); 
      return;
}



