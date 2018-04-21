/* usr2/st/stqkr/mpoint.c 
 * --------------------mpoint.c ------------------------
 *
 * R. Zeitlhoefler, 02.05.1997.
 * last mod. 98_10_09, zei.
 * Diese Funktion wird von 'oprin' (ueber 'stqkr') mit dem command 'MPOINT'
 * aufgerufen.
 * Das 'SOURCE' Command hat vorher die Daten fuer rect/decl der vorletzten
 * 'source'im shared memory abgelegt.
 * 'MPOINT' sendet die Daten der vorletzten Quelle zur Antenne.
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
void mpoint (command,itask,ip)
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
        
    strcpy(output,command->name);
    strcpy(errbuf,command->name);
    strcat(output,"/ pointing commands to antenna for source: ");
    strncat(output, stm_addr->prior2_lsorna, 10);

    /* setup st-shared memory */
    setup_ids();
    fs = shm_addr;
    setup_st();
    st = stm_addr;
    
    if (debug)
    printf("mpoint:   debug = %d  \n",debug);  
    
    iaerr = antst(debug);  /* get antenna status in st-shm */
    if (iaerr<0)
       goto ANT_ERROR;
       
    /* antenne in local ? */
    locrem = (stm_addr->AN_mswel | stm_addr->AN_mswaz) & 0X0080;
    if (locrem == 0) {
      strcat(errbuf,"mpoint: !!! ANTENNA IN LOCAL !!!! \a\n");
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
       

    if ( stm_addr->prior2_ra50 == 0.0 || stm_addr->prior2_dec50 == 0.0 )  {
       strcat(errbuf,"/ Rect. und Decl. is NULL, NO SOURCE DATAS !\a\n");
       goto ERR_END;   
    }
    /* set prior source datats in fs-shm */
    
    shm_addr->radat = stm_addr->prior2_ra50;
    shm_addr->decdat = stm_addr->prior2_dec50;
    memcpy(shm_addr->lsorna, stm_addr->prior2_lsorna,10);
       
    iaerr = antso(debug);  /* send prior source data from  */
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
        strcat(errbuf," Antenna Communication Error \a\n");
        
      /* error return to stqkr */
      ERR_END:
      
      for (i=0;i<5;i++)
        ip[i] = 0;
      cls_snd(&ip[0],errbuf,strlen(errbuf),0,0);
      ip[1] = 1;
      return;                         
}
