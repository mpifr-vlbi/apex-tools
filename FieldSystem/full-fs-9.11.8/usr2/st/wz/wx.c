/* usr2/st/stqkr/mx.c 
 * -------------------- mx.c ------------------------

 * wx - C version of the weather command 
 * mod. Zei, 18. oct. 96, mod 12.06.97, fuer Wettzell Wetterstation.
 * Original  Pablo de Vicente 2-Sept-1996 
*/ 

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"      /* shared memory pointer */

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_OUT   256

void wx(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    int ierr,i;
    char output[MAX_OUT];

    strcpy(output,command->name);
    strcat(output,"/");

    if(shm_addr->tempwx>99.9 | shm_addr->tempwx<-99.9) 
      strcat(output,"$$$$");
   else
      sprintf(output+strlen(output),"%4.1f",shm_addr->tempwx);

    strcat(output,",");



    if(shm_addr->preswx>1999. | shm_addr->preswx<0)
      strcat(output,"$$$$$");
    else
      sprintf(output+strlen(output),"%5.1f",shm_addr->preswx);



    strcat(output,",");
    
    
    if(shm_addr->humiwx>200.0 | shm_addr->humiwx<0)
      strcat(output,"$$");
    else
      sprintf(output+strlen(output),"%2.1f",shm_addr->humiwx);


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
