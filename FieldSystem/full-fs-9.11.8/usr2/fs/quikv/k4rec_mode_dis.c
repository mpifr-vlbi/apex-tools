/* k4 vc display */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"
#include "../include/shm_addr.h"

#define MAX_OUT 256

void k4rec_mode_dis(command,itask,ip)
struct cmd_ds *command;
int itask;
long ip[5];
{
  struct k4rec_mode_cmd lclc;
  struct k4rec_mode_mon lclm;
  int kcom, i, ierr, count, start;
  char output[MAX_OUT];

  kcom= command->argv[0] != NULL &&
    *command->argv[0] == '?' && command->argv[1] == NULL;

  if ((!kcom) && command->equal == '=') {
    if(ip[0]!=0) {
      cls_clr(ip[0]);
      ip[0]=0;
    }
    ip[1]=0;
    return;
  } else if (kcom){
    memcpy(&lclc,&shm_addr->k4rec_mode,sizeof(lclc));
  } else {
    k4rec_mode_res_q(&lclc,&lclm,ip);
    if(ip[1]!=0) {
      cls_clr(ip[0]);
      ip[0]=ip[1]=0;
    }
    if(ip[2]!=0) {
      ierr=ip[2];
      goto error;
    }
  }

   /* format output buffer */

  strcpy(output,command->name);
  strcat(output,"/");
  start=strlen(output);

  for (i=0;i<5;i++)
    ip[i]=0;

  count=0;
  while( count>= 0) {
    if (count != 0)
      strcat(output,",");
    count++;
    k4rec_mode_enc(output,&count,&lclc);
  }

  if(!kcom) {
    count=0;
    while( count>= 0) {
      if (count > 0) strcat(output,",");
      count++;
      k4rec_mode_mon(output,&count,&lclm);
    }
  }

  if(strlen(output)>0) output[strlen(output)-1]='\0';
  
  cls_snd(&ip[0],output,strlen(output),0,0);
  ip[1]++;
  
  return;
  
 error:
  ip[0]=0;
  ip[1]=0;
  ip[2]=ierr;
  memcpy(ip+3,"km",2);
  return;
}

