 /* usr2/st/stqkr/cable  (station specific cable command)
 *
 * --------------------cable.c ------------------------
 * This program is part of stqkr and reads the datas from  HP Counter HP 53131A/132A.
 * The measurement mode is setted manually and stored in the counters memory #1.
 * See also manuals "Einfuehrungshandbuch HP 53131A" and "Programming Guide HP 53131A". 
 * R. Zeitlhoefler, oct 99.
 * Last mod. 99_10_12
 * - used for reading the cable value.
 */

/* counter address , other constants */
 
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

int gpib_io();   /* Funktion fuer gpib_io via converter GPIB-232CT von
                 Naional Instruments , in 'antlib.o' */ 
int find_def(char *filename, char *s_search, char *s_token, int debug);                 

/*--------------------------------------*/
void cable(command,itask,ip)
/*--------------------------------------*/

struct cmd_ds *command;
long ip[5];
int itask;

{

    void setup_ids();
    struct fscom *fs;

    extern int ib_echo;   /* keine unmittelbaren Fehlermeldungen */
    int ioerr, ierr, i;
    int len = 255;
    int debug;
    int outlen;
    int mode;
    int IB_ADDR;
    float val;
    char output[MAX_BUF];
    char obuff[MAX_BUF];
    char ibuff[MAX_BUF];
    char token[20];
    char stmp[50];
    
    setup_ids();
    fs = shm_addr;
    
    /* get IB_ADDR from CONTROLFILE */
    debug=1;
    ierr = find_def(CONTROLFILE, "cable", token, debug);
    
    sscanf(token,"%d", &IB_ADDR);
    if (IB_ADDR <0 || IB_ADDR > 31 ) {
       sprintf(output,"cable/  GPIB-ADDR %d out of range.\a",IB_ADDR);
       goto END;
    }   
           
    
    printf("Station command from stqkr: 'cable',IB_ADDR=%d, counter HP53131 A\n", IB_ADDR );
/*    strcpy(output,command->name); */
    strcpy(output,"cable");
    strcat(output,"/");
    if (ierr!=0)  {
       strcat(output,"ERROR reading CONTROLFILE st_ibad.ctl .");
       goto END;
    }   
    ib_echo = 0;
    mode = 2; /* gpib_io(): Write to gpib ASCII characters, mode = 1 read ASCII char. */
    
    /* Write to counter *rst       -> reset counter */
/*    strcpy (obuff, "*rst\n");
    len=strlen(obuff);
    ioerr = gpib_io(mode, IB_ADDR, obuff, &len );
    
    if (ioerr < 0)  {
      strcat(output," GPIB-ERROR! \a\n");
      goto END;
    }
  */
  
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
  
    
 /*    printf("cable: string ncable = %s \n",ibuff);  */
     
    /* The ':read?' command swithes to single measurement mode, therefore after reading rcl 1.
    It looks better when the measurements are displayed. */
       
    /* Write to counter :intit:cont on     -> Continue with measurements after reading */
   
   strcpy (obuff, ":init:cont on\n");
    len=strlen(obuff);
    /*gpib_io mode 2 -> write*/
    ioerr = gpib_io(2, IB_ADDR, obuff, &len );
    
    if (ioerr  < 0)  {
      strcat(output," GPIB-ERROR! \a\n");
      goto END;
    }
  
    sscanf(ibuff,"%E",&val); 
    shm_addr->cablev = val;   
    
      
    
/*    printf("cablev= %f  val=%.3E\n",shm_addr->cablev, val); */
  
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

