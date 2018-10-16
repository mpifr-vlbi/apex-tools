 /* usr2/st/stqkr/stqkr.c
 *  last mod. 25/07/08, zei. for DBBC tests.
 * stqkr - C version of station command controller
 * 
*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "/usr2/fs/include/params.h"	/*FS parameters */
#include "/usr2/fs/include/fs_types.h"	/*FS header files */
#include "/usr2/fs/include/fscom.h"     /*FS shared mem. structure */
#include "/usr2/st/include/stcom.h"	/*ST shared mem. structure */
#include "/usr2/st/include/stm_addr.h"  /*ST shared mem. pointer */
#include "/usr2/fs/include/shm_addr.h"  /*FS shared memory pointer */

struct stcom *st;
struct fscom *fs;

#define MAX_BUF   257

char old_Line[255]; int rh_to=5; /* ext var for relheiht.c */ 
main()
{
    long ip[5];
    int isub,itask,idum,ierr,nchars,i;
    char buf[MAX_BUF];
    struct cmd_ds command;
    int cls_rcv(), cmd_parse();
    void skd_wait();
   
    
/* Set up IDs for shared memory, then assign the pointer to
 * "fs", for readability.
 */

  setup_ids(); 
  fs = shm_addr;
  
  setup_st(); 
  st = stm_addr;
  loop:
      skd_wait("stqkr",ip,(unsigned) 0);
      if(ip[0]==0) {
        ierr=-1;
        goto error;
      }

      nchars=cls_rcv(ip[0],buf,MAX_BUF,&idum,&idum,0,0);
      if(nchars==MAX_BUF && buf[nchars-1] != '\0' ) { /*does it fit?*/
        ierr=-2;
        goto error;
      }
                                   /* null terminate to be sure */
      if(nchars < MAX_BUF && buf[nchars-1] != '\0') buf[nchars]='\0';

      if(0 != (ierr = cmd_parse(buf,&command))) { /* parse it */
        ierr=-3;
        goto error;
      } 
  

      isub = ip[1]/100;
      itask = ip[1] - 100*isub;
      
      
      switch (isub) {  
      case 1: 
              switch (itask) {    
                case 1:
                  wx(&command,itask,ip);
                  break; /* wx cmd in station has higher priority than wx cmd in fs */
                case 2:      
                  wxz(&command,itask,ip);
                  break;  /* other wx with more datas */
                  }
                  break;
         case 2:
                switch (itask) {
                  case 1:
                    cable(&command,itask,ip);
                    break;  /* HP 53132 counter */
                  case 2:      
                    counter2(&command,itask,ip);
                    break;  /* other Hp 531232 counter */  
                  case 3:      
                    relheight(&command,itask,ip); /* Telescope invar highth measure */     	
                    break; }
                    break;
         case 3:
                switch (itask) {
                   case 1: point (&command,itask,ip);    /* repeat last source command */
                   break;
                   case 2: mpoint(&command,itask,ip);    /* repeat source command bevore last source command */  
                   break; }
                   break;   
         case 4:
                switch (itask) {
                   case 1: /*tk(&command,itask,ip);*/        /* test functions for antenna control */
                   break; 
                   case 2:
                   ptlog(&command,itask,ip);     /* test, write pointing values in log-file */
                   break; }
                   break;
         case 5:  /* DBBC Testversion, see usr2/control/stcmd.ctl, commands 5xx*/
                switch (itask) { 
                case 0: /* command dbbc=' direct sring ' */
                  dbbc(&command,itask,ip);
                break;
                case 1: /* command dbbc01= ... */ 
                  dbbcnn(&command,itask,ip);
                break;
                case 2: /* command dbbc02= ... */
                  dbbcnn(&command,itask,ip);
                break;
                case 3: /* command dbbc03= ... */
                   dbbcnn(&command,itask,ip);
                break;
                case 4: /* command dbbc04= ... */
                   dbbcnn(&command,itask,ip);
                break;
                case 5: /* command dbbc05= ... */
                   dbbcnn(&command,itask,ip);
                break;
                 case 6: /* command dbbc06= ... */
                   dbbcnn(&command,itask,ip);
                break;
               case 7: /* command dbbc07= ... */
                   dbbcnn(&command,itask,ip);
                break;
               case 8: /* command dbbc08= ... */
                   dbbcnn(&command,itask,ip);
                break;
               case 9: /* command dbbc09= ... */
                   dbbcnn(&command,itask,ip);
                break;
                 case 10: /* command dbbc10= ... */
                   dbbcnn(&command,itask,ip);
                break;
                   case 11: /* command dbbc11= ... */
                   dbbcnn(&command,itask,ip);
                break;
                      case 12: /* command dbbc12= ... */
                   dbbcnn(&command,itask,ip);
                break;
                     case 13: /* command dbbc13= ... */
                   dbbcnn(&command,itask,ip);
                break;
                     case 14: /* command dbbc14= ... */
                   dbbcnn(&command,itask,ip);
                break;
                     case 15: /* command dbbc15= ... */
                   dbbcnn(&command,itask,ip);
                break;
                     case 16: /* command dbbc16= ... */
                   dbbcnn(&command,itask,ip);
                break;
                     case 20: /* command dbbcif ... */
                   dbbcif(&command,itask,ip);
                break;
                     case 21: /* command dbbcifa ... */
                   dbbcifn(&command,itask,ip);
                break;
                     case 22: /* command dbbcifb ... */
                   dbbcifn(&command,itask,ip);
                break;
                     case 23: /* command dbbcifc ... */
                   dbbcifn(&command,itask,ip);
                break;
                     case 24: /* command dbbcifd ... */
                   dbbcifn(&command,itask,ip);
                break; 
                     case 25: /* command dbbcform ... */
                   dbbcform(&command,itask,ip);
                break; 
                     case 26: /* command dbbcmon ... */
                   dbbcmon(&command,itask,ip);
                break;
                     case 27: /* command dbbc_cal_if */
                   dbbc_cal_if(&command,itask,ip);
                break;
                     case 28: /* command dbbc_cal_ch */
                   dbbc_cal_ch(&command,itask,ip);
                break;        
                     case 30: /* command for dbbc: logfile ... */
                   logfile(&command,itask,ip);
                break; 
                     case 31: /* command for dbbc: resetall ... */
                   resetall(&command,itask,ip);
                break; 
                     case 32: /* command for dbbc: clock ... */
                   clock(&command,itask,ip);
                break;
                     case 33: /* command for dbbc: dbbcstat */
                   dbbcstat(&command,itask,ip);
                break;
                     case 34: /* command for dbbc: reg */
                   reg(&command,itask,ip);
                break;               
             }                        
            break;
         case 6:
               switch (itask) {
                }  
            break;
         case 7:
            break;     
         case 8:
            break;
         case 9:
            break; 	                      
         default:
            ierr=-6;
            goto error;
      }
      goto loop;

    error:
      for (i=0;i<5;i++) ip[i]=0;
      ip[2]=ierr;
      memcpy(ip+3,"st",2);
      goto loop;
}
