 /*orig.  function handls mk5 SNAP commands 
mod for dbbc test, Zei 06/07/08
now: dbbc SNAP command called by  stqkr.c
***********************
DBBCnn=   'nn= base band converter 1-16'
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
#define DEBUG 0    		/* additional prinf for debug mode when 1 */
#define FREQmin 10.000000		/* DBBCnn min frequency */
#define FREQmax 2048.000000	/* DBBCnn max frequency */
extern struct stcom *st;
extern struct fscom *fs;

#include "simple_socket.h"

/*****************************/
void dbbcnn(command,itask,ip)     /*For Base Band Converter 1-16*/
/****************************/
struct cmd_ds *command;                /* parsed command structure */
int itask;                  
                                      /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int iBuf_len=BUFSIZE;
      int debug=1;
      int ilast, ierr=0, ind,i,i0,i1,i2,i3,i4,itmp, count;
      int istrcmp=0;
      float f1,f2;
      double ftmp=0;
      char *ptr;
      char *arg_next();
      int out_recs, out_class;
      char outbuf[BUFSIZE];
      char buf[BUFSIZE];
      char inbuf[BUFSIZE];
      void skd_run(), skd_par();     	 /* program scheduling utilities */
      strcpy(outbuf,command->name);
      
      if (command->argv[0] == NULL || command->equal != '=' )
            /* command is only 'dbbcnn' or 'dbbcnn=' without arguments */   
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
      
      /*---------------mch command ? ---------------------------------*/
      /* multi channel Eqi Spaced = 'mch'command? DBBCnn=mch,[2eK] */
       
     /* i=memcmp(command->argv[0],"mch", 3);*/ /*check only first 3 char*/
      i = strcmp(command->argv[0],"mch"); /* check complete string, is "mch" ? */
      if(i==0) /* it is mch command */
      {
        if (DEBUG)  
        printf("dbbc=mch command i=%d\n",i);
        
        strcat(outbuf,command->argv[0]);
        
        if (out_recs==1) {
          logitf("dbbcnn: No value for MultiChannelEquiSpace command");
           goto exit;
        }   
        if (out_recs!=2) { 
           logitf("dbbcnn: dbbc=mch more than one argument");    
           goto exit;
        } 
        
        /* check mch command range, allowed values 2,4,8,16,32,64 */
        
        i=sscanf (command->argv[1],"%d",&i2);/* convert to integer*/
        if (i != 1) {
           logitf ("dbbcnn: not allowed input for dbbcnn=mch");
           goto exit;
        }            
        /* check,   input allowed? */
        if (i2==2 || i2==4 || i2==8 || i2==16 || i2==32 || i2==64) {
           strcat(outbuf,",");
           strcat(outbuf,command->argv[1]);
           
        }
        else { 
 
           logitf("dbbcnn: allowed values for mch command:  2,4,8,16,32,64");
           goto exit;
        }      
            
      
      }  /* end of parsing dbbc=mch,xx command */

  
        /*---------------- DBBCnn=freq,IF,bwdU,bwdL,gainU,gainL,tpint -------------------*/
        /*---- check inputs, are they valid?---- */
 
        /*---- first check frequency range ----*/
        
        if (DEBUG) {
            printf("dbbcnn: dbbcnn, command [0]%s out_recs %d out_class %d  outbuf %s\n"\
           ,command->argv[0], out_recs, out_class,outbuf);
          }
        i=sscanf (command->argv[0],"%lf",&ftmp); 
            /* arg[0] contains char[]  freq */
      
        if (DEBUG)
            printf("i-sscanf=%d ftmp=%f\n",i,ftmp);
        if ((ftmp < FREQmin)||(ftmp > FREQmax)) {
           logitf("DBBCnn frequency out of range"); 
           goto exit;
        }   
        strcat(outbuf,command->argv[0]);
          
        /*--- check IF channel setting ---*/
  
        if (out_recs >= 2)   /* is value for dbbcnn IF definded or empty? */
        {       
            
              if ((command->argv[1] == NULL) && (out_recs >=3)) {
                 strcat(outbuf,",");
                 goto bwdU;
              }       
              i0 = strlen(command->argv[1]);            
              if (i0 !=2) {
                 logitf("DBBCnn: IF channal range incorrect");
                 
                  
                 goto exit;
              }              
               
              if(DEBUG) { 
               printf("dbbcnn:IF-setting=%s strlen=%d\n",command->argv[1],i0);
              }
             /* Check now, has  IF setting allowed commands? */
             /* allowed IF=i n (i=A or B or C or D, n=1 or 2 or 3 or 4), cmd in argv[1] */
               
                
             i0=(command->argv[1])[0]; /* i0 contains input channel, e,g, a=97, d=100 */
             i1=(command->argv[1])[1]; /* i1 contains 1 -4 (1=49 .. 4=52)*/
             
             if ((i0 >= 97)&&(i0 <=100)&&(i1 >= 49)&&(i1<=52)) {
             /* then allowed input */
               if (DEBUG)
                  printf("(argv[1])[0]=%d  (argv[1])[1]=%d\n",i0,i1);
               strcat(outbuf,",");
               strcat(outbuf,command->argv[1]);
               if (out_recs  ==2) 
                  goto dbbc_cn;              
             }   
             else { 
              logitf("DBBCnn: IF channal setting out of range"); 
              goto exit;
             }
        }
bwdU:       /*Setting of  upper side bandwith in MHz*/
        if (out_recs >= 2)   
        {   
            i=sscanf(command->argv[2],"%d",&i2);
            /* are values for bwdU allowed? */
            if (i2==2 || i2==4 || i2==8 || i2==16 || i2==32 )  {  
               strcat(outbuf,",");
               strcat(outbuf,command->argv[2]);
               goto bwdL;
            }  
            else { 
               logitf("dbbcnn: bwdU incorrect value");
               goto exit;
            }     
           /*    goto bwdL;*/
        }   
bwdL:       /*Setting of  lower side bandwith in MHz*/
        if (out_recs >= 4)
        {   
            i=sscanf(command->argv[3],"%d",&i2);
            /* are values for bwdU allowed? */
            if (i2==2 || i2==4 || i2==8 || i2==16 || i2==32 )  {  
               strcat(outbuf,",");
               strcat(outbuf,command->argv[3]);
               
               goto gainU;
            }  
            else { 
               logitf("dbbcnn: bwdL incorrect value");
               goto exit;
            }     
        }     
gainU:  /* gain USB, 0-48 stepp 1 or AGC */
        if (out_recs >= 5)
        {
           i=sscanf(command->argv[4],"%d",&i2);
            /* is value for gainU allowed? */
           if (i==1 && (( i2 >= 0) && (i2 <= 48)))
           {
               strcat(outbuf,",");
               strcat(outbuf,command->argv[4]);
               goto gainL;
           }
             /* check complete string, is "agc" ? */
            i = strcmp(command->argv[4],"agc");
           
            if(i==0) /* - is agc command */
            {
               if (DEBUG)  
                   printf("dbbc=agc command i=%d\n",i);
               strcat(outbuf,",");  
               strcat(outbuf,command->argv[4]);
               goto gainL;
            }        
               if (out_recs==5) {
               logitf("dbbcnn: No allowed value for gainU command");
               goto exit;
               }
        }
               
   gainL:  /* gain LSB, 0-48 stepp 1 or AGC */
        if (out_recs >= 6)
        {
           i=sscanf(command->argv[5],"%d",&i2);
            /* is value for gainU allowed? */
           if (i==1 && (( i2 >= 0) && (i2 <= 48)))
           {
               strcat(outbuf,",");
               strcat(outbuf,command->argv[5]);
               goto tpint;
           }
             /* check complete string, is "agc" ? */
            i = strcmp(command->argv[5],"agc");
           
            if(i==0) /* - is agc command */
            {
               if (DEBUG)  
                   printf("dbbc=LSB is agc command i=%d\n",i);
               strcat(outbuf,",");  
               strcat(outbuf,command->argv[5]);
               goto tpint;
            }        
               if (out_recs==6) {
               logitf("dbbcnn: No allowed value for gainL command");
               goto exit;
               }
        }             
tpint:  if (out_recs >= 7)  
        {   i=sscanf(command->argv[6],"%f",&f1);
            /* is value for tpint allowed? */
           if (i==1 && (( f1 >= 0.25 ) && (f1 <= 60)))
           {
              i=(int)(f1 / 0.25);
              f2=(float)  (i * 0.25);
              if(f1 != f2) printf ("tpint allowed steps 0.25\n");
              if (DEBUG) printf("Tpint: %.2f\n",f2);
              sprintf(buf,"%.2f",f2);
               strcat(outbuf,",");
               strcat(outbuf,buf);
               goto dbbc_cn;
           }
            logitf("tpint allowed range: 0.25 - 60, step 0.25");
         }
        
   if (out_recs >= 8)
   {
      printf("Command has more than 7 arguments\n");
      goto exit;
   }   
 
 dbbc_cn: /* connect dbbc, call client function */
   
   *(inbuf)='\0';
   i=strlen(outbuf);
   iBuf_len=256;
   /* client function */
   ierr=dbbc_com( iBuf_len, outbuf, inbuf);
   logitf(inbuf); 
      if (DEBUG)
      {
        printf( "dbbcnn: ierr from dbbc_com=%d \n", ierr);
        printf("dbbcnn: strlen(outbuf)=%d\n",i);
      }  

  
exit:
      
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"dbbcnn",6); 
      return;


  }

