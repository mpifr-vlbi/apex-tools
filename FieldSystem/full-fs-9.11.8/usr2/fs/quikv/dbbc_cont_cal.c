/* dbbc_cont_cal snap commands */

#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>

#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"         /* shared memory definition */
#include "../include/shm_addr.h"      /* shared memory pointer */

#define BUFSIZE 512

void dbbc_cont_cal(command,ip)
struct cmd_ds *command;                /* parsed command structure */
long ip[5];                           /* ipc parameters */
{
      int ilast, ierr,count;
      char *ptr;
      struct dbbc_cont_cal_cmd lcl;  /* local instance of dbbc_cont_cal command struct */
      int out_recs, out_class;
      char outbuf[BUFSIZE];

      int dbbc_cont_cal_dec();               /* parsing utilities */
      char *arg_next();

      void dbbc_cont_cal_dis();
      void skd_run(), skd_par();      /* program scheduling utilities */

      int kdiff;

      if (command->equal != '=') {            /* read module */
	out_recs=0;
	out_class=0;

	strcpy(outbuf,"cont_cal");
	cls_snd(&out_class, outbuf, strlen(outbuf) , 0, 0);
	out_recs++;
         goto dbbcn;
      } else if (command->argv[0]==NULL) goto parse;  /* simple equals */
      else if (command->argv[1]==NULL) /* special cases */
	if (*command->argv[0]=='?') {
	  dbbc_cont_cal_dis(command,ip);
	  return;
	}

/* if we get this far it is a set-up command so parse it */

parse:
      ilast=0;                                      /* last argv examined */
      memcpy(&lcl,&shm_addr->dbbc_cont_cal,sizeof(lcl));

      count=1;
      while( count>= 0) {
        ptr=arg_next(command,&ilast);
        ierr=dbbc_cont_cal_dec(&lcl,&count, ptr, shm_addr->dbbccontcalpol);
        if(ierr !=0 ) goto error;
      }

      kdiff=memcmp(&shm_addr->dbbc_cont_cal,&lcl,sizeof(lcl));
      memcpy(&shm_addr->dbbc_cont_cal,&lcl,sizeof(lcl));

      if(kdiff)
	skd_run("tpicd",'w',ip);
 
/* format buffer for dbbcn */
      
      out_recs=0;
      out_class=0;

      dbbc_cont_cal_2_dbbc(outbuf,&lcl);
      cls_snd(&out_class, outbuf, strlen(outbuf) , 0, 0);
      out_recs++;

dbbcn:
      ip[0]=1;
      ip[1]=out_class;
      ip[2]=out_recs;
      skd_run("dbbcn",'w',ip);
      skd_par(ip);

      if(ip[2]<0) {
	if(ip[0]!=0) {
	  cls_clr(ip[0]);
	  ip[0]=ip[1]=0;
	}
	return;
      }

      dbbc_cont_cal_dis(command,ip);
      return;
      
error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"dd",2);
      return;
}
