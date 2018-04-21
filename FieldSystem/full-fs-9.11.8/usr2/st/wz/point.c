/* usr2/st/stqkr/point.c 
 * -------------------- point.c ------------------------
 *
 * R. Zeitlhoefler, 02.05.1997.
 * last mod. 99_01_27, zei.
 * Diese Funktion wird von 'oprin' (ueber 'stqkr') mit dem command 'POINT'
 * aufgerufen.
 * Das 'SOURCE' command hat vorher die daten fuer rect/decl
 * im st-shared memory abgelegt.
 * 'POINT' sendet Daten fuer das Pointing zur Antenne. 
*/ 

#define DEBUG 0

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"
#include "/usr2/st/include/stcom.h"
#include "/usr2/st/include/stm_addr.h"

      /* shared memory pointer */

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_OUT   256

int antge(), antti(), antmo(), anten(), antso(), antso_0(), antst();

/*----------------------------------------*/
void point (command,itask,ip)
/*----------------------------------------*/
struct cmd_ds *command;
long ip[5];
int itask;
{
    int ierr, iaerr, len, i;
    int locrem;
    int debug = DEBUG;
    char output[MAX_OUT];
    char errbuf[MAX_OUT];

    /* setup st-shared memory */
    setup_ids();
    setup_st();
    st = stm_addr;
        
    strcpy(output,command->name);
    strcpy(errbuf,command->name);
    strcat(output," pointing commands to antenna for source:");
    /* next mod. 99/01/27, zei */
    strncat(output,shm_addr->lsorna, 10); /*stm_addr->prior1_lsorna, 10);*/
    
    if (debug)
    printf("point:   debug = %d  \n",debug);  
    
    iaerr = antst(debug);  /* get antenna status in st-shm */
    if (iaerr<0)
       goto ANT_ERROR;
       
    /* antenne in local ? */
    locrem = (stm_addr->AN_mswel | stm_addr->AN_mswaz) & 0X0080;
    if (locrem == 0) {
      strcat(errbuf,"/ !!! ANTENNA IN LOCAL !!!! \a\n");
       goto ERR_END;
    }
    
    
    iaerr = antge(debug); /* send MJD to antenna */ 
    if (iaerr < 0)
       goto ANT_ERROR;
       
    iaerr = antti(debug); /* send Time sync */
    if (iaerr < 0)
       goto ANT_ERROR;
       
    iaerr = anten(debug); /* send met. data */
    if (iaerr < 0)
       goto ANT_ERROR;    
       

    if ( fs->radat == 0.0 || fs->decdat == 0.0 )  {
       strcat(errbuf,"/ Rect. und Decl. ist NULL, KEINE SOURCEDATEN !\a\n");
       goto ERR_END;   
    }
    /* antst() gets source datas from radat and decdat */
    
   /* fs->radat=stm_addr->prior1_ra50;
    fs->decdat=stm_addr->prior1_dec50;
    memcpy(fs->lsorna, stm_addr->prior1_lsorna, 10);
   mod. 99/01/27, zei */
          
    iaerr = antso(debug);  /* send source data from shm */
    if (iaerr < 0)
       goto ANT_ERROR;           
           
    iaerr = antmo(9,9,debug); /* send auto obser. */
    if (iaerr < 0)
       goto ANT_ERROR;

/*  return from class i/o */

END:
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
      
      
ANT_ERROR:
        strcat(errbuf,"/point: Serial I/O Error \a\n");
        
      /* error return to stqkr */
      
      ERR_END:
      
      for (i=0;i<5;i++)
        ip[i] = 0;
      cls_snd(&ip[0],errbuf,strlen(errbuf),0,0);
      ip[1] = 1;
      return;                         
}
