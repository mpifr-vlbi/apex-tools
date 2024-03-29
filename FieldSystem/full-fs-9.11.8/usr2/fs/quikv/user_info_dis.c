/* S2 recorder user_info display */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"
#include "../include/shm_addr.h"

#define MAX_OUT 256

void user_info_dis(command,ip)
struct cmd_ds *command;
long ip[5];
{
      struct user_info_cmd lclc;
      int kcom, i, ierr, count, start;
      struct res_buf buffer;
      struct res_rec response;
      void get_res();
      char output[MAX_OUT];

      kcom= command->argv[0] != NULL &&
            *command->argv[0] == '?' && command->argv[1] == NULL;

      if ((!kcom) && command->equal == '=') {
         logrclmsg(output,command,ip);
         return;
      } else if (kcom){
	memcpy(&lclc,&shm_addr->user_info,sizeof(lclc));
      } else {
	opn_rclcn_res(&buffer,ip);
	
	for (i=0;i<4;i++) {
	  ierr=get_rclcn_user_info_read(&buffer,lclc.labels+i);
	  if(ierr!=0)
	    goto error;
	}

	ierr=get_rclcn_user_info_read(&buffer,lclc.field1);
	if(ierr!=0)
	  goto error;

	ierr=get_rclcn_user_info_read(&buffer,lclc.field2);
	if(ierr!=0)
	  goto error;

	ierr=get_rclcn_user_info_read(&buffer,lclc.field3);
	if(ierr!=0)
	  goto error;

	ierr=get_rclcn_user_info_read(&buffer,lclc.field4);
	if(ierr!=0)
	  goto error;

	clr_rclcn_res(&buffer);
      }


   /* format output buffer */

      strcpy(output,command->name);
      strcat(output,"/");
      start=strlen(output);

      for (i=0;i<5;i++) ip[i]=0;

      count=0;
      while( count>= 0) {
        if (count%3 != 0)
	  strcat(output,",");
	else
	  output[start]=0;
        count++;
        user_info_enc(output,&count,&lclc);
	if(count%3 == 0 && count > 0) {
	  cls_snd(&ip[0],output,strlen(output),0,0);
	  ip[1]++;
	}
      }


      return;

error:
      clr_rclcn_res(&buffer);
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"ru",2);
      return;
}
