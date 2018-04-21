/* usr2/st/stqkr/onso.c 
 * -------------------- onso.c ------------------------
 * 
 * R. Zeitlhoefler, 02.05.1997.
 *
 * Funktion fuer 'stqkr.c'
 *
 * Diese Funktion wird von 'oprin' (ueber 'stqkr') mit dem command 'ONSO'
 * aufgerufen.
 * Beim Aufruf von 'antst()' wird der Antennenstatus abgefragt.
 * Der Antennenstatus wird in das 'shared memory' (usr2/st/include/stcom.h)
 * abgelegt.
 * 'ONSO'ist ein 'stationsspezifisches Snap-Command' das 'ONSOURCE' entspricht,
 * welches normalerweise ueber 'antcn()' bearbeitet wird. 
 * 'onso'schreibt in das fs->ionsor '1' wenn pointing ok, '0' pointing nicht ok.
*/ 

#define DEBUG 0
#define IB_ADDR 4 
#define IB_TEST 0
#define DELTA_DEVIATION_ONSOURCE 0.005
 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"
#include "/usr2/st/include/stcom.h"
#include "/usr2/st/include/stm_addr.h"

      /* shared memory pointer */

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_OUT 200

int antge();

/*----------------------------------------*/
void onso (command,itask,ip)
/*----------------------------------------*/
struct cmd_ds *command;
long ip[5];
int itask;
{
    int ierr, iaerr, len, i, ionsor;
    int locrem, ant_stop, az_stop, el_stop;
    int mswaz, mswel, mswazel;
    int debug = DEBUG;
    int ib_addr = IB_ADDR;
    int ib_test = IB_TEST;
    float max_dev = DELTA_DEVIATION_ONSOURCE;
    double dev_el;
    double dev_az;
    char output[MAX_OUT];
    char errbuf[MAX_OUT];
        
    strcpy(output,command->name);
    strcpy(errbuf,command->name);
  /*  strcat(output,"/ st-onsource command for:");
    strncat(output, fs->lsorna, 10);
  */
    /* setup st-shared memory */
    setup_ids();
    setup_st();
    st = stm_addr;
    
    if (debug)
    printf("onso: Address GPIB = %d  debug = %d  ib_test = %d\n",\
    ib_addr,debug,ib_test);  
    
    iaerr = antst(debug, ib_addr, ib_test);  /* get antenna status in st-shm */
    if (iaerr<0)
       goto ANT_ERROR;
    mswaz = stm_addr->AN_mswaz;
    mswel = stm_addr->AN_mswel;
      
    /* antenne in local ? */
    locrem = mswaz & 0X80;  /* loc/rem gilt fuer beide Achsen */
    if (locrem == 0) {
       strcat(errbuf,"/  ANTENNA IN LOCAL , not ONSOURCE ! \a\n");
       fs->ionsor = 0;
       goto ERR_END; 
    }
    if (debug)
       printf("mswaz = %X mswel = %X \n", mswaz, mswel);
    
    

    if( ( mswaz & 0X0F ) == 0X01)   {
        strcat (errbuf,"/ Azimuth is in STOP !\a\n");
        fs->ionsor = 0;
    }
    if ( (mswel & 0X0F) == 0x01)    {
        strcat (errbuf, "           / Elevation is in STOP !\a\n");
        fs->ionsor = 0;
        goto ERR_END;
    }    

    if ( fs->radat == 0.0 || fs->decdat == 0.0 )  {
       strcat(errbuf,"/ Rect. und Decl. ist NULL, KEINE SOURCEDATEN !\a\n");
       fs->ionsor = 0;
       goto ERR_END;   
    }

/* Check onsource status */

   dev_el = fabs(stm_addr->AN_dvel);
   dev_az = fabs(stm_addr->AN_dvaz);
   
   
   if ( (dev_el <= max_dev) && (dev_az <= max_dev))  {
      strcat (output, "/ Antenna ONSOURCE \n");
      fs->ionsor = 1;
      goto END;
   }   
   
   
   if ( (dev_el >= max_dev) && (dev_az >= max_dev) ) {
      strcat (errbuf,"/ Azimuth and Elevation not ONSOURCE !\a\n");   
      fs->ionsor = 0;
      goto ERR_END;
   }   
      
   if ( dev_el >= max_dev )  {
      strcat (errbuf, "/ Elevation not ONSOURCE ! \a\n");
      fs->ionsor = 0;
      goto ERR_END; 
   }
   
   if ( dev_az >= max_dev )  {
      strcat (errbuf, "/ Azimuth not ONSOURCE !  \a\n");
      fs->ionsor = 0;
   }
   
         
      
      
      
/*  return from class i/o */

END:
    for (i=0;i<5;i++) ip[i]=0;
    cls_snd(&ip[0],output,strlen(output),0,0);
    ip[1]=1;
    return;
/*error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"st",2);
*/      return;
      
      
ANT_ERROR:
      if (iaerr == -4)
         strcat(errbuf,"/ error -4, GPIB Converter TIME OUT ERROR !!! \a\n");
      else if(iaerr == -16016)
        strcat(errbuf,"/ error -16016, READ GPIB TIME OUT ERROR !!! \a\n");
      else if(iaerr == -16028)
        strcat(errbuf,"/ error -16028, WRONG GPIB ADDR. or ANT. CONTROLLER \
 NOT ACTIV ??? \a\n");
      else if((iaerr == -32392) || (iaerr == -32464))
        strcat(errbuf,"/ error , BUS CABLE CONNECTED ? \a\n");
      else 
        strcat(errbuf,"/ ERROR GPIB-I/O \a\n");
        
      /* error return to stqkr */
 ERR_END:
      
      for (i=0;i<5;i++)
        ip[i] = 0;
      cls_snd(&ip[0],errbuf,strlen(errbuf),0,0);
      ip[1] = 1;
      return;                         
}
