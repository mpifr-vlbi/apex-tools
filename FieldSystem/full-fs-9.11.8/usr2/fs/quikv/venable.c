/* vlba enable snap command */

#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>

#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"         /* shared memory definition */
#include "../include/shm_addr.h"      /* shared memory pointer */

void venable(command,itask,ip)
struct cmd_ds *command;                /* parsed command structure */
int itask;                            /* sub-task, ifd number +1  */
long ip[5];                           /* ipc parameters */
{
      int ilast, ierr, indx, ichold, i, count;
      char *ptr;
      struct req_rec request;      /* mcbcn request record */
      struct req_buf buffer;       /* mcbcn request buffer */
      struct venable_cmd lcl;      /* local instance of venable command struc */

      int vepro_dec();                 /* parsing utilities */
      char *arg_next();

      void ini_req(), add_req(), end_req(); /*mcbcn request utilities */
      void skd_run(), skd_par();      /* program scheduling utilities */

      ichold= -99;                    /* check vlaue holder */

      ini_req(&buffer);

      indx=itask-1;                    /* index for this module */

      if(indx == 0) 
	memcpy(request.device,"r1",2);
      else 
	memcpy(request.device,"r2",2);

      if (command->equal != '=') {            /* read module */
         request.type=1;
         request.addr=0x80; add_req(&buffer,&request);
         request.addr=0x81; add_req(&buffer,&request);
         goto mcbcn;
      } else if (command->argv[0]==NULL) goto parse;  /* simple equals */
        else if (command->argv[1]==NULL) /* special cases */
         if (*command->argv[0]=='?') {
            venable_dis(command,itask,ip,indx);
            return;
         } else if(0==strcmp(command->argv[0],ADDR_ST)) {
            ierr=-301;
            goto error;
         } else if(0==strcmp(command->argv[0],TEST)) {
            ierr=-301;
            goto error;
         } 

/* if we get this far it is a set-up command so parse it */

parse:
      ilast=0;                                      /* last argv examined */
      memcpy(&lcl,&shm_addr->venable[indx],sizeof(lcl));

      count=1;
      while( count>= 0) {
        ptr=arg_next(command,&ilast);
        ierr=venable_dec(&lcl,&count, ptr,indx);
        if(ierr !=0 ) goto error;
      }

/* all parameters parsed okay, update common */

      ichold=shm_addr->check.rec[indx];
      shm_addr->check.rec[indx]=0;
      memcpy(&shm_addr->venable[indx],&lcl,sizeof(lcl));
      
/* format buffers for mcbcn */
      
      request.type=0; 
      request.addr=0x80;
      venable80mc(&request.data,&lcl); add_req(&buffer,&request);
      request.addr=0x81;
      venable81mc(&request.data,&lcl); add_req(&buffer,&request);

mcbcn:
      end_req(ip,&buffer);
      skd_run("mcbcn",'w',ip);
      skd_par(ip);

      if (ichold != -99) {
         shm_addr->check.vkenable[indx] = TRUE;
         if (ichold >= 0)
            ichold=ichold % 1000 + 1;
         shm_addr->check.rec[indx]=ichold;
      }

      if(ip[2]<0) return;
      venable_dis(command,itask,ip,indx);
      return;

error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"ve",2);
      return;
}
