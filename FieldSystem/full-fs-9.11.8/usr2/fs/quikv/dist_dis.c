/* vlba ifd display */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"
#include "../include/shm_addr.h"

#define MAX_OUT 256

void dist_dis(command,itask,ip)
struct cmd_ds *command;
int itask;
long ip[5];
{
      struct dist_cmd lclc;
      struct dist_mon lclm;
      int ind,kcom,i,ich, ierr, count;
      struct res_buf buffer;
      struct res_rec response;
      void get_res();
      void mc01dist(), mc02dist(), mc04dist(), mc06dist(), mc07dist();
      char output[MAX_OUT];

      ind=itask-1;

      kcom= command->argv[0] != NULL &&
            *command->argv[0] == '?' && command->argv[1] == NULL;

      if ((!kcom) && command->equal == '=') {
         logmsg(output,command,ip);
         return;
      } else if(kcom)
         memcpy(&lclc,&shm_addr->dist[ind],sizeof(lclc));
      else {
         opn_res(&buffer,ip);
         get_res(&response, &buffer); mc01dist(&lclc, response.data);
         get_res(&response, &buffer); mc02dist(&lclc, response.data);
         get_res(&response, &buffer); mc04dist(&lclm, response.data);
         get_res(&response, &buffer); mc06dist(&lclm, response.data);
         get_res(&response, &buffer); mc07dist(&lclm, response.data);
         if(response.state == -1) {
            clr_res(&buffer);
            ierr=-401;
            goto error;
         }
         clr_res(&buffer);
      }

   /* format output buffer */

      strcpy(output,command->name);
      strcat(output,"/");

      count=0;
      while( count>= 0) {
        if (count > 0) strcat(output,",");
        count++;
        dist_enc(output,&count,&lclc);
      }

      if(!kcom) {
        count=0;
        while( count>= 0) {
        if (count > 0) strcat(output,",");
          count++;
          dist_mon(output,&count,&lclm);
        }
      }
      if(strlen(output)>0) output[strlen(output)-1]='\0';

      for (i=0;i<5;i++) ip[i]=0;
      cls_snd(&ip[0],output,strlen(output),0,0);
      ip[1]=1;

      return;

error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"vi",2);
      return;
}
