/* user_device buffer parsing utilities */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <math.h>

#include "../include/macro.h"
#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"         /* shared memory definition */
#include "../include/shm_addr.h"      /* shared memory pointer */

static char *dev_key[ ]={"u1","u2","u3","u4","u5","u6"};
static char *sb_key[ ]={"unknown","usb","lsb"};
static char *pol_key[ ]={"unknown","rcp","lcp"};
static char *star_key[ ]={"*"};

#define DEV_KEY sizeof(dev_key)/sizeof( char *)
#define SB_KEY  sizeof(sb_key)/sizeof( char *)
#define POL_KEY sizeof(pol_key)/sizeof( char *)
#define STAR_KEY sizeof(star_key)/sizeof( char *)

int user_device_dec(lcl,count,ptr)
struct user_device_cmd *lcl;
int *count;
char *ptr;
{
    int ierr, arg_key(), arg_int();
    int len, dum, i;
    static int dev;

    ierr=0;
    if(ptr == NULL) ptr="";

    switch (*count) {
    case 1:
      ierr=arg_key(ptr,star_key,STAR_KEY,&dum,0,FALSE);
      if(ierr == 0 && dum == 0)
	ierr=-300;
      else {
	ierr=arg_key(ptr,dev_key,DEV_KEY,&dev,0,FALSE);
	if(ierr==0 && dev < 4) {
	  ierr=-200;
	} else if(ierr==-100) {
	  for (i=0;i<MAX_USER_DEV;i++) {
	    lcl->lo[i]=-1;
	    lcl->sideband[i]=0;
	  }
	  ierr=0;
	  *count=-1;
	}
      }
      break;
    case 2:
      ierr=arg_key(ptr,star_key,STAR_KEY,&dum,0,FALSE);
      if(ierr == 0 && dum == 0)
	ierr=-300;
      else
	ierr=arg_dble(ptr,&lcl->lo[dev],0.0,FALSE);
      break;
    case 3:
      ierr=arg_key(ptr,star_key,STAR_KEY,&dum,0,FALSE);
      if(ierr == 0 && dum == 0)
	ierr=-300;
      else
	ierr=arg_key(ptr,sb_key,SB_KEY,&lcl->sideband[dev],0,FALSE);
      break;
    case 4:
      ierr=arg_key(ptr,star_key,STAR_KEY,&dum,0,FALSE);
      if(ierr == 0 && dum == 0)
	ierr=-300;
      else
	ierr=arg_key(ptr,pol_key,POL_KEY,&lcl->pol[dev],0,FALSE);
      break;
    case 5:
      ierr=arg_key(ptr,star_key,STAR_KEY,&dum,0,FALSE);
      if(ierr == 0 && dum == 0)
	ierr=-300;
      else
	ierr=arg_dble(ptr,&lcl->center[dev],0.0,FALSE);
      break;
    default:
      *count=-1;
    }
    
    if(ierr!=0) ierr-=*count;
    if(*count>0) (*count)++;
    return ierr;
}

void user_device_enc(output,count,lcl)
char *output;
int *count;
struct user_device_cmd *lcl;
{
  int ivalue,idec,pos;
  static int idev;

  output=output+strlen(output);

  if(*count == 1)
    idev=0;
  else
    idev++;

  while(idev<MAX_USER_DEV && lcl->lo[idev] <0)
    idev++;
  if(idev >= MAX_USER_DEV) {
    if(*count==1)
      strcpy(output,"undefined");
    else
      *count=-1;
    return;
  }

  strcpy(output,dev_key[idev]);
  strcat(output,",");
  
  idec=16;
  if(lcl->lo[idev] >= 1.0)
    idec-=log10(lcl->lo[idev]);
  dble2str(output,lcl->lo[idev],35,idec);
  pos=strlen(output)-1;
  while(output[pos]=='0') {
    output[pos]='\0';
    pos=strlen(output)-1;
  }
  pos=strlen(output)-1;
  if(output[pos]=='.')
    output[pos]='\0';
  strcat(output,",");

  ivalue = lcl->sideband[idev];
  if (ivalue >=0 && ivalue <SB_KEY)
    strcat(output,sb_key[ivalue]);
  else
    strcat(output,BAD_VALUE);
  strcat(output,",");

  ivalue = lcl->pol[idev];
  if (ivalue >=0 && ivalue <POL_KEY)
    strcat(output,pol_key[ivalue]);
  else
    strcat(output,BAD_VALUE);
  strcat(output,",");

  idec=17;
  if(lcl->center[idev] >= 1.0)
    idec-=log10(lcl->center[idev]);
  dble2str(output,lcl->center[idev],35,idec);
  pos=strlen(output)-1;
  while(output[pos]=='0') {
    output[pos]='\0';
    pos=strlen(output)-1;
  }
  pos=strlen(output)-1;
  if(output[pos]=='.')
    output[pos]='\0';
  
  if(*count>0)
    *count++;
  return;
}
