 /* usr2/st/stqkr/counter2.c  (This program reads the counter53131A)
 *
 * --------------------counter2.c (new-counter2)------------------------
 * This program is part of stqkr and reads the datas from  HP Counter HP 53131A/132A.
 * The measurement mode is setted manually and stored in the counters memory #1.
 * See also manuals "Einfuehrungshandbuch HP 53131A" and "Programming Guide HP 53131A". 
 * R. Zeitlhoefler, oct 99.
 * mod. 00-03-11, round up of log entry mod. zei.
 * - used for reading the fmout-gps value.
 * - mod. changed  fmout-gps to gps-fmout, 03.11.2002, zei.
 */

 
# define CONTROLFILE "/usr2/st/control/st_ibad.ctl" 

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h> 
#include <math.h>

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"      /* shared memory pointer */

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_BUF   256

int gpib_io();   /*Function for  gpib_io via converter GPIB-232CT from
                 Naional Instruments , in 'st/antlib' */ 
int find_def(char *filename, char *s_search, char *s_token, int debug);                 
                 /*Function for getting tokens from control file, in 'st/antlib'*/
                 
/*--------------------------------------*/
void counter2(command,itask,ip)
/*--------------------------------------*/

struct cmd_ds *command;
long ip[5];
int itask;

{

    void setup_ids();
    struct fscom *fs;

    extern int ib_echo;   /* keine unmittelbaren Fehlermeldungen */
    int i, ioerr, ierr;
    int len = 255;
    int debug;
    int outlen;
    int mode;
    int IB_ADDR;
    double val;
    char output[MAX_BUF];
    char obuff[MAX_BUF];
    char ibuff[MAX_BUF];
    char token[20];
    char stmp[50];
    
    setup_ids();
    fs = shm_addr;
    
    /* get IB_ADDR from CONTROLFILE */
    debug=1;
    ierr = find_def(CONTROLFILE, "counter", token, debug);
    
    sscanf(token,"%d", &IB_ADDR);
    if (IB_ADDR <0 || IB_ADDR >31)  {
       sprintf (output,"gps-fmout ERROR, GPIB-ADDR %d out of range.\a",IB_ADDR);
       goto END;
    }    
    
    /* only local info with printf */
    printf("Station command from stqkr: 'counter2',IB_ADDR=%d, counter HP53131 A\n", IB_ADDR );
/*    strcpy(output,command->name); */
    /*strcpy(output,"fmout-gps");*/
    strcpy(output,"gps-fmout");
    strcat(output,"/");
    if (ierr!=0)  {
       strcat(output,"ERROR reading CONTROLFILE st_ibad.ctl .");
       goto END;
    }   
    ib_echo = 0;
    mode = 2; /* gpib_io(): Write to gpib ASCII characters, mode = 1 read ASCII char. */
  
    /* Write to counter *rcl 1      -> Recall Memory 1, measurement mode is manually composed
                                       and stored in memory 1 */
    strcpy (obuff, "*rcl 1\n");
    len=strlen(obuff);
    ioerr = gpib_io(mode, IB_ADDR, obuff, &len );
    
    if (ioerr < 0)  {
      strcat(output," GPIB-ERROR! \a\n");
      goto END;
    }
    
    /* Write to counter :read?       -> start measurement and query the result */
    
    strcpy (obuff, ":read?\n");
    len=strlen(obuff);
    ioerr = gpib_io(mode, IB_ADDR, obuff, &len );
    
    if (ioerr < 0)  {
      strcat(output," GPIB-ERROR! \a\n");
      goto END;
    }
  
    
    
    
    /* Read from counter           -> reads message from gpib device */
    
    mode=1;  /* read in ASCII mode */
    len=255; /* reads max 255 characters */   
    (char)*ibuff=0;
    
    ioerr = gpib_io(mode, IB_ADDR, ibuff, &len );
    
    if (ioerr < 0)  {
      strcat(output," read GPIB-ERROR! \a\n");
      goto END;
    }
  
    
   /*  printf("string counter2 = %s \n",ibuff);*/  
     
    /* The ':read?' command swithes to single measurement mode, therefore after reading rcl 1.
    It looks better when the actual measurements are displayed. */
       
    /* Write to counter :intit:cont on     -> Continue with measurements after reading */
   
   strcpy (obuff, ":init:cont on\n");
    len=strlen(obuff);
    /*gpib_io mode 2 -> write*/
    ioerr = gpib_io(2, IB_ADDR, obuff, &len );
    
    if (ioerr  < 0)  {
      strcat(output," GPIB-ERROR! \a\n");
      goto END;
    }
  
    sscanf(ibuff,"%lE",&val); 
    
      
    
/*    printf("  val=%.3E\n", val); */
      if (val >= 0.100000)
      	sprintf (stmp,"%.6E", val);
      else	 
        sprintf (stmp,"%.3E",val);
    
      strcat(output,stmp); 
      
     
END:                    /*  return to stqkr  */
    for (i=0;i<5;i++) ip[i]=0;
    cls_snd(&ip[0],output,strlen(output),0,0);
    ip[1]=1;
    return;
error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
       memcpy(ip+3,"st",2);
      return;
}

