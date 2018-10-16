 /*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command called by  stqkr.c
***********************
DBBCif(A,B,C,D)=   command for if modules 
***********************
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>
#include <math.h>

#include "/usr2/fs//include/params.h"
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
void dbbcifn(command,itask,ip)     /*used for dbbcif(A,B,C,D) commands*/
/****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
long ip[5];                           /* ipc parameters */
{
      int iBuf_len;
      int ilast, ierr=0, ind,i,i0,i1,itmp, count;
      int istrcmp=0;
      float f1,f2,f3;
      double ftmp=0;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char inbuf[BUFSIZE];
      char buf[BUFSIZE];
      void skd_run(), skd_par();     	 /* program scheduling utilities */

      strcpy(outbuf,command->name);
      
      if (command->argv[0] == NULL || command->equal != '=' )
            /* command is only 'dbbcnn' or 'dbbcnn=' without arguments */   
         {
           goto dbbc_cn;
         } 
/* if we get this far it is a DBBCIF(A,B,C,D) set-up command so parse it */
/*parse:*/
      ilast=0;                       
      out_recs=0;
      out_class=0;
      ptr=arg_next(command,&ilast);
      
      if (command->equal == '=') 
        strcat(outbuf,"=");
        
      /* check number of dbbcifn=arg0,arg1,... */ 
      while( ptr != NULL) {
	/*strcat(outbuf,ptr);*/
 	out_recs++;
	ptr=arg_next(command,&ilast);
	
      }
      /* parse input channel settings */
      if (out_recs >=1)
      {      
          i=sscanf (command->argv[0],"%d",&i1);/* convert to integer*/
          if (DEBUG) printf("i=%d, i1=%d out_recs=%d\n",i,i1,out_recs);
        /* check,   input allowed? */
        if (i1==1 || i1==2 || i1==3 || i1==4) {
           strcat(outbuf,command->argv[0]);
        }
        else { 
           logitf("dbbcifn: Only input channel 1,2,3,4 possible ");
           goto error;
        }   
      }      
      if (out_recs >=2 )
      { 
       /* gain IF, range -16dB to +16 dB, stepp 0.5 dB or AGC */
         i=sscanf(command->argv[1],"%f",&f1);
         if (DEBUG) printf ("i=%d f1=%f\n",i,f1);
            /* is value forIF range allowed? */
         if (i==1 && (( f1 >= -16) && (f1 <= 16)))
           {
              i=(int)(f1 / 0.5);
              f2=(float)  (i * 0.5);
              if(f1 != f2) printf ("dbbcifn: tpint allowed steps 0.5 dB\n");
              if (DEBUG) printf("dbbcifn: gain= %.2f f3=%f\n",f2);
              sprintf(buf,"%.2f",f2);
              strcat(outbuf,",");
              strcat(outbuf,buf);
              if (out_recs == 2)
                 goto dbbc_cn;
              
            /*  goto if_filter; */
           }
             /* check complete string, is "agc" ? */
            i = strcmp(command->argv[1],"agc");
           
            if(i==0) /* - is agc command */
            {
               strcat(outbuf,",");  
               strcat(outbuf,command->argv[1]);
             /*  goto if_filter;*/
            }
            else 
            {
            logitf("dbbcifn: Allowed gain range: -16 till  16");
            goto error;
            }   
         }
         
         if (out_recs == 2)
            goto dbbc_cn;
         
if_filter:    
         /* check filter settings */
      if (out_recs >=3)
      {      
          i=sscanf (command->argv[2],"%d",&i1);/* convert to integer*/
          if (DEBUG) printf("i=%d, i1=%d out_recs=%d\n",i,i1,out_recs);
        /* check,   input allowed? */
        if (i1==1 || i1==2 || i1==3 || i1==4) {
           strcat(outbuf,",");
           strcat(outbuf,command->argv[2]);
        }
        else { 
           logitf("dbbcifn: Only filter  1,2,3,4 possible ");
           goto error;
        }   
      }

       if (out_recs >= 4)
       {  
         logitf("dbbcifn: Command has more than 4 arguments");
         if (DEBUG)
         printf("IF module Configuration: outbuf: %s\n", outbuf);
         goto error;
       }    
   /* client function for dbbc connection */ 
dbbc_cn:   
      *(inbuf)='\0';
      i=strlen (outbuf);
      iBuf_len=256;
      ierr=dbbc_com( iBuf_len, outbuf, inbuf);
      if (ierr != 0)
          goto error;
      
   exit:
       for (i=0;i<5;i++) ip[i]=0;
       cls_snd(&ip[0],inbuf,strlen(inbuf),0,0);
       ip[1]=1;
       return;
   
   error:
      
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"dbbcifa",7); 
      return;


  }

