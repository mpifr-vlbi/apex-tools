
/* usr2/st/stqkr/ptlog.c 
 * -------------------- ptlog.c ------------------------
 * ptlog.c (Poining-logit) is a test program for pointing tests.
 * The snap command ptlog writes az and el values into the log file.
 * It reads antenna status an compares AZ/EL nominal and true values.
 * The nominal values are calculatet by FS-program cnvrt() routine.
 * -under construction,   Zei.
 * last mod line 87, Aug 14,2007,zei.
*/ 

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"      /* shared memory pointer */
#include "/usr2/st/include/stcom.h"
#include "/usr2/st/include/stm_addr.h"
#define MAX_OUT  256

extern struct stcom *st;
extern struct fscom *fs;

void setup_st();
void setup_ids();

int antst();
/*-------------------------------*/
void ptlog(command,itask,ip)
/*-------------------------------*/
struct cmd_ds *command;
long ip[5];
int itask;

{
    int ierr;
    int i;
    int it[6];
    char output[MAX_OUT];
    char outbuf[256];
    double dazim;
    double delev;
    double daz;
    double del;
    


/* setup_ids();
 setup_st();*/
fs=shm_addr;

st=stm_addr;



    
    
    strcpy(output,command->name);
   
    /* get fs-time */
 
    rte_time(it,&it[5]);
    
    dazim=fs->ra50;
    delev=fs->dec50;
    
    /* covert FS-RA/Dec in AZ/EL values */
    z_cnvrt(1,fs->radat, fs->decdat,&dazim,&delev,it,fs->alat,fs->wlong);
    
    /* get antenna status and write to shared memory */
   ierr=antst(0);
   
    if (ierr < 0)
     {
       strcat(output,"/ can't read antenna status.");
       goto end;
       }
   delev=delev * M_1_PI * 180.0000;
   dazim=dazim * M_1_PI * 180.0000;    
   /* calculate delta AZ/EL */
   daz=dazim - st->AN_azim;
   del=delev - st->AN_elev;
   
   sprintf(outbuf,"/ FS-AZ:%.4f, AN-AZ:%.4f, Delta_AZ:%.4f"\
   "                 FS-EL:%.4f, AN-EL:%.4f, Delta_EL: %.4f\n",\
                    dazim,st->AN_azim,daz, delev,st->AN_elev,del);
   
   strcat(output,outbuf); 
end:   
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
