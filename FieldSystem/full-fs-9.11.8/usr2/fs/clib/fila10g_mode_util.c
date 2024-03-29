/* fila10g_mode commmand buffer parsing utilities */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"         /* shared memory definition */
#include "../include/shm_addr.h"      /* shared memory pointer */

static char *disk_key[ ]=         { "disk_record_ok" }; 

#define NSOURCE_KEY sizeof(source_key)/sizeof( char *)
#define NDISK_KEY sizeof(disk_key)/sizeof( char *)

char *m5trim();

int fila10g_mode_dec(lcl,count,ptr)
struct fila10g_mode_cmd *lcl;
int *count;
char *ptr;
{
    int ierr, i, arg_key();
    float sample;
    
    ierr=0;
    if(ptr == NULL) ptr="";

    switch (*count) {
    case 1:
      ierr=arg_int(ptr,&lcl->mask.mask ,0xffffffff,TRUE);
      m5state_init(&lcl->mask.state);
      if(ierr==0) {
	lcl->mask.state.known=1;
      } else {
	lcl->mask.state.error=1;
      } 
      break;
    case 2:
      lcl->decimate.decimate=0;
      ierr=arg_int(ptr,&lcl->decimate.decimate ,1,FALSE);
      m5state_init(&lcl->decimate.state);
      if(ierr == 0 && (lcl->decimate.decimate<1 ||
		       lcl->decimate.decimate>255))
	ierr=-200;
      if(ierr==0) {
	lcl->decimate.state.known=1;
      } else if(ierr!=-100) {
	lcl->decimate.state.error=1;
      } 
      if(ierr==-100)
	ierr=0;
      break;
    case 3:
      ierr=arg_float(ptr,&sample,0.0,FALSE);
      if(lcl->decimate.state.known != 0) {
	if(ierr != -100)
	  ierr=-300;
	else if(ierr == -100) {
	  ierr = 0;
	  break;
	}
      } else if(lcl->decimate.state.known == 0 && ierr == -100) {
	if(0 == shm_addr->m5b_crate)
	  ierr=-100;
	else {
	  sample=shm_addr->m5b_crate;
	  ierr=0;
	}
      }
      if(ierr == 0 ) {
	if(sample <= 0.124) {
	  ierr=-200;
	} else {
	  lcl->decimate.decimate=(shm_addr->m5b_crate/sample)+0.5;
	  if( fabs(lcl->decimate.decimate*sample-shm_addr->m5b_crate)/
	      shm_addr->m5b_crate > 0.001)
	    ierr=-210;
	  else if( lcl->decimate.decimate <1 ||
		   lcl->decimate.decimate >255) {
	    ierr=-210;
	  }
	}
      }
      if(ierr==0) {
	lcl->decimate.state.known=1;
      } else {
	lcl->decimate.state.error=1;
      } 
      break;
    case 4:
      ierr=arg_key(ptr,disk_key,NDISK_KEY,&lcl->disk.disk,-1,TRUE);
      m5state_init(&lcl->disk.state);
      if(ierr==0) {
	lcl->disk.state.known=1;
      } else {
	lcl->disk.state.error=1;
      } 
      break;
    default:
      *count=-1;
   }

   if(ierr!=0) ierr-=*count;
   if(*count>0) (*count)++;
   return ierr;
}

void fila10g_mode_enc(output,count,lclc)
char *output;
int *count;
struct fila10g_mode_cmd *lclc;
{

  int ivalue;

    output=output+strlen(output);

    switch (*count) {
    case 1:
      strcpy(output,"0x");
      m5sprintf(output+2,"%lx",&lclc->mask.mask,&lclc->mask.state);
      break;
    case 2:
      m5sprintf(output,"%d",&lclc->decimate.decimate,&lclc->decimate.state);
      break;
    case 3:
      if(shm_addr->fila10g_mode.decimate.state.known &&
	 shm_addr->fila10g_mode.decimate.decimate!=0) {
	sprintf(output,"(%.3f)",
		(float) (shm_addr->m5b_crate/
			 shm_addr->fila10g_mode.decimate.decimate) );
	m5state_encode(output,&lclc->decimate.state);
      }
      break;
    default:
      *count=-1;
   }

   if(*count>0) *count++;
   return;
}
void fila10g_mode_mon(output,count,lclm)
char *output;
int *count;
struct fila10g_mode_mon *lclm;
{

  int ivalue;

    output=output+strlen(output);

    switch (*count) {
    case 1:
      m5sprintf(output,"%d",&lclm->samplerate.samplerate,
		&lclm->samplerate.state);
      break;
    default:
      *count=-1;
   }

   if(*count>0) *count++;
   return;
}

vsi_bitmask_2_fila10g(ptr,lclc)
char *ptr;
struct fila10g_mode_cmd *lclc;
{

  sprintf(ptr,"fila10g=vsi_bitmask 0x%x",lclc->mask.mask);

  return;
}
vsi_samplerate_2_fila10g(ptr,lclc)
char *ptr;
struct fila10g_mode_cmd *lclc;
{
  sprintf(ptr,"fila10g=vsi_samplerate %d  %d",
	  (int) (shm_addr->m5b_crate*1.0e6+0.5),lclc->decimate.decimate);

  return;
}
vdif_frame_2_fila10g(ptr,lclc)
char *ptr;
struct fila10g_mode_cmd *lclc;
{
  int bits=0;
  int bitmask=lclc->mask.mask;
  int bits_p_chan = 0 ;
  int channels = 0;
  int i;
    
  for(i=0;i<32;i++) 
    if(bitmask & 0x1<<i)
      bits++;
  
  if((0xaaaaaaaa & bitmask) && (0x5555555 & bitmask))
    bits_p_chan = 2 ;
  else if(bitmask)
    bits_p_chan = 1 ;  
  
  if(bits_p_chan > 0)
    channels = bits/bits_p_chan;

  sprintf(ptr,"fila10g=vdif_frame %d  %d 8000", bits_p_chan,channels);

  return;
}

int fila10g_2_vsi_bitmask(ptr,lclc) /* return values:
				  *  0 == no error
				  *  0 != error
				  */
     char *ptr;           /* input buffer to be parsed */

     struct fila10g_mode_cmd *lclc;  /* result structure with parameters */
{
  char string[]= "VSI input bitmask :";

  m5state_init(&lclc->mask.state);

  ptr=strstr(ptr,string);
  if(ptr == NULL) {
    return -1;
  }

  if(m5sscanf(ptr+sizeof(string),"%lx",&lclc->mask.mask,&lclc->mask.state)) {
    return -1;
  }

  return 0;
}
int fila10g_2_vsi_samplerate(ptr,lclc,lclm) /* return values:
				  *  0 == no error
				  *  0 != error
				  */
     char *ptr;           /* input buffer to be parsed */

     struct fila10g_mode_cmd *lclc;  /* result structure with parameters */
     struct fila10g_mode_mon *lclm;  /* result structure with parameters */
{
  char string[]= "VSI sample rate :";

  m5state_init(&lclc->decimate.state);
  m5state_init(&lclm->samplerate.state);

  ptr=strstr(ptr,string);
  if(ptr == NULL) {
    return -1;
  }

  if(m5sscanf(ptr+sizeof(string),"%d",
	      &lclm->samplerate.samplerate,&lclm->samplerate.state)) {
    return -1;
  }

  return 0;
}
