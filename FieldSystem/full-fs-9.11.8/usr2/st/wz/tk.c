/* usr2/st/stqkr/tk.c
 *
 * ___________________________ TK.C __________________________________
 *
 * Function tk() - talking to antenna.
 * This programm sends data frames to the antenna.
 * It is useful for programm developement and works also in antenna=local mode
 * because no checks are made before sending a data frame.
 *  
 * 
 * 
 * 97_03_04, Zei, last mod: 98_10_23 zei.
 *
 */ 

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"

# define MAX_OUT 256

extern struct stcom *st;
extern struct fscom *fs;

/*-------------------------*/
void tk(command, itask, ip) 
/*-------------------------*/

struct cmd_ds *command;
long ip[5];
int itask;

{
  int i;
  int ierro;
  int antge(), antti(), antmo(), anten(), antso(), ant_azeloff();
  char output[MAX_OUT];
  char pcmd[200];
  int ierr;
  int igets = 0;
  int debug = 0;
 
  int mswel;		/* Mode Status Word EL */
  int mswaz;		/* Mode Status Word AZ */   

  
  START1:
   
    if(debug == 2)  
    {
     printf("\nANTENNA DATA TRANSFER BLOCKS, zei nov 96\n");
    }
 
  printf(" -------------------------------------------------\n");
  printf(" >>>       tk - talking to KRUPP antenna       <<<\n"); 
  printf(" -------------------------------------------------\n");
  printf(" For selection  click to 'login shell' !!!        \n");
  printf("                                                  \n");
  printf(" INI - send GE, TI and EN for INItialisation      \n");
  printf(" GE  - send GEneral conditions to ACU             \n");
  printf(" TI  - send TIme synchronisation                  \n");
  printf(" EN  - send ENviromental conditions (weather)     \n");
  printf(" MO  - send MOde (STOP, PRESET, RATE)             \n");
  printf(" PT  - send Program Track (AZ and EL values with actual time)\n");
  printf(" PRT - send Programm Track, man. input AZ, EL, Time \n");
  printf(" ST  - request ACU for Actual State Monitoring    \n");
  printf(" so  - send source datas from st_shm to ACU       \n");
/*  printf(" s0  - send src.cmd. '0' to clear obs. stack      \n");*/
  printf(" OFS - send az/el offsets, manual input           \n");
  printf(" D   - set debug mode. Actual debug mode = %d     \n", debug);
  printf(" ?   - show options                               \n");
  printf(" ex  - to EXIT, also 'a' for abort or '::'        \n");
  
  START2:
  printf("tk? ");  
 
  igets = *gets(pcmd);
  pcmd[0] = toupper(pcmd[0]);
  pcmd[1] = toupper(pcmd[1]);
  pcmd[2] = toupper(pcmd[2]);
   
  /* printf("igets = %d string pcmd: %s\n", igets, pcmd);*/

  if (pcmd[0] == 'A' || (pcmd[0] == 'E' && pcmd[1] == 'X') || (pcmd[0] == ':' &&
      pcmd[1] == ':'))
  goto END; 

  else if (memcmp(pcmd,"INI",3) == 0)
  {
      printf("sends GE, TI, EN to ACU \n"); 
      ierr = antge(debug);
      ierr = antti(debug);
      ierr = anten(debug);
  }
  else if (memcmp(pcmd,"GE",2) == 0)  
  {
      /* printf("send GEneral conditions \n"); */
      ierr = antge(debug);
  }
  else if (memcmp(pcmd, "TI",2) == 0 ) 
  {
      /* printf("send TIme synchronisation \n");*/
      ierr = antti(debug);
  }
  else if (memcmp(pcmd, "EN",2) == 0) 
  {
      /* printf("send ENvironmental conditions \n");*/
      ierr = anten(debug);
  }
  else if (memcmp(pcmd,"MO",2) == 0 ) 
  {
      /* printf("send MOde selection \n"); */
      mswel = 0;
      mswaz = 0; /* 0 for manual input */ 
      ierr = antmo(mswel, mswaz);
  }
  else if ( memcmp(pcmd,"ST",2) == 0 ) 
  {
      /* printf("request for actual STate monitoring  \n"); */
      /* debug = 1;   1 schaltet 'printf' fuer die Anzeige ein  */
      ierr = antst(1);
  }
  else if (memcmp(pcmd,"SO",2) == 0 ) 
  {
      printf("source datas from shared memory \n");
      printf("sende: antmo(STOP), antso(), antmo(AUTO-OBS.)\n"); 
    /*  ierr = antso_0(debug);	*/		/* clear observation stack  */
     
    /*  ierr = antmo(1, 1, debug);	*/	/* az/el in STOP mode       */
      ierr = antmo(9, 9, debug);
      ierr = antso(debug);      		/* source data from sh_mem. */
      ierr = antmo(9, 9, debug); 		/* az/el in AUTO-OBS. mode  */
  }   
 /* else if  (memcmp(pcmd,"S0",2) == 0 ) 
  {
      ierr = antso_0(debug);	
  }        */
  else if  ( memcmp(pcmd,"PT",2) == 0 ) 
  {
 
           
      ierr = antmo(3, 3, debug);		
      ierr = antpt(0.0, 0.0, 3, debug); 	/* PROGRAM TRACK 	*/
      printf("sende: antpt(), antmo(PROGRAM TRACK) \n");
     /* ierr = antmo(3, 3, debug);	*/	/* az/el PROG.TRACK md. */
  }  
  
  else if  (memcmp(pcmd,"PRT",3) == 0 ) 
  {
 
     /* printf("sende: antmo(STOP)\n");*/ 
    /*  ierr = antmo(1, 1, debug);*/		/* az/el in STOP mode	*/
      ierr = antmo(3, 3, debug);
      ierr = antptr(0.0, 0.0,-1, 3, debug); 	/* PROGRAM TRACK 	*/
      printf("sende: antmo(PROGRAM TRACK), antptr() \n");
    /*  ierr = antmo(3, 3, debug);*/		/* az/el PROG.TRACK md. */
  }  
  
  else if (memcmp(pcmd,"D",1) == 0)  
  {
      printf("Debug mode: 0 or 1 or 2 or (3 = without data transfer ) ?");
      scanf("%d", &debug);
  }  
    
  else if (memcmp(pcmd,"OFS",3) == 0 )
  {
      ierr = ant_azeloff(0.0000,0.0000,debug,1);
  }         

  else 
  {
      goto START1;
  }
  goto START2;
  
  END:     
  printf("!!!   Click to 'Operator Input' now   !!!\n");
      
  /* return to stqkr */
  strcpy(output, command->name);
  strcat(output,"/ direct antenna cmmand execution"); 
  for ( i = 0; i > 5; i++)  ip[i]=0;
  cls_snd(&ip[0], output, strlen(output), 0, 0);
  ip[1] = 1;
  return;

  error:
    ierro = 0;
    ip[0] = 0;
    ip[1] = 1;
    ip[2] = ierro;
    memcpy(ip+3,"st",2);
  /*printf("  END\n"); */
  
  return;

} /* End of function tk() */
