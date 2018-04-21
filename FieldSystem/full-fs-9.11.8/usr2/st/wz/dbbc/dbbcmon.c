/*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command called by  stqkr.c
**********************************************************
DBBCmon=bnn,[u/l] set the Digital to Analog Channel source
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
#define DEBUG 0	/* additional prinf for debug mode when 1 */

extern struct stcom *st;
extern struct fscom *fs;

/*****************************/
void dbbcmon(command,itask,ip)     /*For Base Band Converter 1-16*/
/****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
                                      /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int iBuf_len;
      int ilast, ierr=0, ind,i,i0,i1,i2,i3,i4,itmp, count;
      int istrcmp=0;
      float f1,f2;
      double ftmp=0;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char buf[BUFSIZE];
      char ifbuf[BUFSIZE], inbuf[BUFSIZE];
      void skd_run(), skd_par();     	 /* program scheduling utilities */

      strcpy(outbuf,command->name);
      
      if (command->argv[0] == NULL || command->equal != '=' )
            /* command is only 'dbbcmon' or 'dbbcmon=' without arguments */   
         {
           goto dbbc_cn;
         } 
/* if we get this far it is a dbbcnn set-up command so parse it */
/*parse:*/
      ilast=0;                       
      out_recs=0;
      out_class=0;
      ptr=arg_next(command,&ilast);
      
      if (command->equal == '=') 
        strcat(outbuf,"=");
        
      /* check number of dbbcnn= commands, separated by ',' */ 
      while( ptr != NULL) {
 	out_recs++;
	ptr=arg_next(command,&ilast);
	
      }
      
     /* DBBCMON=bnn,[u/l] */       
      i = memcmp(command->argv[0],"b",1); /* check  string, first is "b" ? */
      if(i==0) /* it is b  command */
      {
        if (DEBUG)  
        printf("dbbcmon: i=%d command %s\n",i, command->argv[0]);
          
      }  
      else {

           logitf("dbbcmon: First char must be 'b'");
           goto error;
      }
        strcpy(buf,command->argv[0]);
        memset(buf,48,1);              /* "0" = 48 m*/
        if(DEBUG) printf("buf=%s\n",buf);
  
        /* check bbc #, allowed 1 - 16 DCC and 1 - 64 MCHM
         preliminary only 1 - 16 */
           i=sscanf (buf,"%d",&i2);/* convert to integer*/
           if (i != 1) {
             logitf("dbbcnn: not allowed input for dbbcmon=bnn");
           goto error;
           }  
           /* check,   input allowed? */
           if ((i2 >= 1) && (i2 <= 16)) /* nn=1-16*/
              {
              strcpy(outbuf,command->argv[0]);
              }
           else {
           /*   logitf("dbbcmon: allowed channel 1-16"); */
                z_logit("dbbcmon: allowed channel  1-16",-11,"ST");
           goto error;
           }
        /* Upper or lower side band? */   
             /* DBBCMON=bnn,[u/l] */  
       if (out_recs >= 2)  {     
          /* check complete string, is "u" ? */      
         i = strcmp(command->argv[1],"u");
          /* check complete string, is "l" ? */ 
         i1 = strcmp(command->argv[1],"l");
           if((i==0) || (i1==0)) /* it is u or l  command */
             {
             strcat(outbuf,",");
             strcat(outbuf,command->argv[1]);
             }   
           else {
              logitf("dbbcmon: only u or l allowed");
           goto error;
           }
        } 
 /* end of parsing dbbcmon*/
dbbc_cn:
           if (DEBUG)
             printf("send to DBBC: %s\n",outbuf);
           
           *(inbuf)='\0';
           i=strlen(outbuf);
           iBuf_len=256;
           /* client function */
           ierr=dbbc_com(iBuf_len, outbuf, inbuf);
           if (DEBUG)
              printf("dbbcmon: ierr=%d, Respons from Server= %s\n", ierr, inbuf);
           if (ierr != 0)
               goto error;
exit:	for (i=0;i<5;i++)  ip[i]=0;
        cls_snd(&ip[0],inbuf,strlen(inbuf),0,0);
        ip[1]=1;
        return;              
error:      
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"dbbcmon",7); 
      return;
}


