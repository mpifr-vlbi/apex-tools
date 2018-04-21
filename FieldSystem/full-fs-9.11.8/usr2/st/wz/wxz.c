
/* usr2/st/stqkr/mxz.c
 * -----------------------mxz.c--------------------------------
 * last mod 12.10.98, zei.
 *       Version fuer stqkr, wxz als Snap Command        
 * RTW oct 03, 1996 R. Zeitlhoefler
 * Daten vom st-shared memory, 23.07.2009, Zei.
 *
 * Es werden die Werte aller Sensoren auf dem Terminal angezeigt.
 * Die Portparameter sind in usr2/st/control/serial.ctl
*/ 
/* Include files */
#include <stdio.h>
#include <stdlib.h>

# include <ctype.h>
# include <string.h>
# include <sys/types.h>
# include "/usr2/fs/include/params.h"
# include "/usr2/fs/include/fs_types.h"
# include "/usr2/fs/include/fscom.h"
# include "/usr2/fs/include/shm_addr.h"
# include "/usr2/st/include/stcom.h"
# include "/usr2/st/include/stm_addr.h"

 extern struct fscom *fs; 
 extern struct stcom *st;

/*--------------------------------------------------------------------*/
void wxz(command, itask, ip)      /* Aufruf ueber stqkr, Snap Command */
/*--------------------------------------------------------------------*/

  struct cmd_ds *command;
  long ip[5];
  int itask;
{ 

  int i;
  
  int it[6];
  char output[100]; 
  /*  rte Zeit  */
   /*  setup_ids();*/
     rte_time(it, &it[5]);
   

             printf("      WETTERDATEN von st-shm \n");
             printf("wxz at: %d/%d/%d:%d:%d  \n",it[5],it[4],it[3],it[2],it[1]);         
             printf("wind direction:  %d\n",st->st_directionwx);
             printf("wind velocity :  %5.1lf\n",st->st_speedwx);
             printf("temperature 1 :  %5.1lf\n",st->st_temp1wx);      
             printf("humidity 1    :  %5.1lf\n",st->st_humi1wx);
             printf("temperature 2 :  %5.1lf\n",st->st_temp2wx);
             printf("humidity 2    :  %5.1lf\n",st->st_humi2wx);
              
             printf("pressure      :  %5.1lf\n",st->st_preswx);
             
             printf(" \n");

  
  /* return to stqkr */
  strcpy (output, command->name);
  strcat (output, "/ st_cmd. wxz exec.");
  
  for ( i = 0; i < 5; i++)
    ip[i] = 0;
    
  cls_snd(&ip[0], output, strlen(output),0,0);   
    
  ip[i] = 1;
  return;
  
}



