 /*--------------------- relheight.c----------------------------------------*/
/* R. Zeitlhoefler, 18. Mar, 1999.
 * last mod: apr 20, 1999 zei.
 * Reads the rel. height of the RTW 20m antenna from a file.
 * The datas of the hight meassurement system, received from a PC
 * with the program 'SAMBA' are in the file /home/share/stack.txt.
 *
 * 27.07.2004 After the installation of the new SUPERMICRO Field System PC
 *            (running since 07.06.2004) the data file with the rel. height
 *            values coming from the rel.hight measurement system via ftp
 *            to fspc directory /usr2/st/invar/stack.txt.
 *            R. Schatz               
 *
 * This programm reads the file /usr2/st/invar/stack.txt and puts
 * the last line into the log file.
 *
 * Argument 'debug' der Funktion: debug = 0  - keine 'printf'
 *                                debug = 1  - 'printf' Fehlermeldungen
 *                                debug = 2  - + Statusmeldungen
 * return Werte: 0  = ok.
 *              -1  = File open error.
 *               
 *              
*/

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <sys/types.h>

# include "/usr2/fs/include/params.h"
# include "/usr2/fs/include/fs_types.h"
# include "/usr2/fs/include/fscom.h"
# include "/usr2/fs/include/shm_addr.h"

# define DEBUG 0
# define MAXLINE 1000
# define DATAFILE "/usr2/st/invar/stack.txt"

extern struct stcom *st;
extern struct fscom *fs;
   
/*-----------------------------------------*/
  void relheight(command,itask,ip)
/*-----------------------------------------*/   
   
   
  struct cmd_ds *command;
  long ip[5];
  int itask;

   {
   
   unsigned long ul_Wert;
   int get_lastline();
   int ierr,iscmp;
   int debug = DEBUG;
   int i, len;
   extern int rh_to;  
                
   char Line[255];
   extern char old_Line[];
   char output[255];
     
   
   strcpy(output,command->name);
   strcat(output,"/T1,T2,T3,T4,L5/ ");
                    
   ierr = get_lastline(DATAFILE, Line , debug);
   
   if (ierr==-1)  {
      sprintf(output+strlen(output), "File open error in %s \n", DATAFILE);
      goto END;
   }
   
     if (debug) {
        printf("relheight: get_lastline: ierr = %d, Line = %s\n", ierr, Line);
        printf("relheight: old_Line: %s\n",old_Line);
        }     
        
   len = strlen(Line);
   
   iscmp = memcmp(old_Line,Line,len);
   
   if (debug)
      printf("relheight: memcmp_val: %d\n",iscmp);
      
   if (iscmp == 0) {
        if (debug)
           printf("rh_to=%d\n",rh_to);
        
        if (rh_to >0) {             
           rh_to-=1;
           goto NEXT;
        }        
          
        /* if 5 times no new value, then doe next */
        if (rh_to == 0) {  
   	  sprintf(output+strlen(output)," No new values for antenna heights");
   	  goto END;
 	}
   	
   }
   rh_to=5;
NEXT:   
   strcat(output,Line); 
   /* new datas, logit */
   	
   memcpy (old_Line,Line,len); /* save Line */
   
   END:
   len=strlen(output);
   
   if (debug)
         printf("relheight: end: output=%s  strlen=%d\n",output, len);
      
   /* return to stqkr */
   for (i=0;i<5;i++) ip[i]=0;
   cls_snd(&ip[0],output,len,0,0);
   ip[1]=1;
   return;
   
   ERROR: /* not used */
   ip[0]=0;
   ip[1]=0;
   ip[2]=ierr;
   memcpy(ip+3,"st",2);
   return;
    
    
  } 
  
  /*................ */
 
 /* function for relheight.c, reads last line from file ---------*/
 
 int get_lastline(char *filename, char *Line, int debug)
 /*--------------------------------------------------------------*/
{ 
  
  int ierr=0;
 
  unsigned long ulLinecount=0;            /* Schleifenzaehler */
  
  FILE *Samplefile;
  

    
 Samplefile = fopen( filename ,"r");

 if (Samplefile == NULL) {
     if (debug) 
       printf("relheight, get_lastline(): File open error, filename: %s\a\n", filename);
    return -1;
 }
 if (debug == 2)
    printf("relheight, get_lastline(): File %s is open. \n", filename);
    
     /*letzte Zeile von file einlesen mit fgets() in Line */ 
    
 while ( fgets( Line, MAXLINE, Samplefile) != NULL) {
  
     ulLinecount++; 
            
     if (debug==2)  {
        printf("relheight, get_lastline(): %ld Line \n%s\n",
        ulLinecount,Line);
     } 

 }  /* end of while */
                       
 fclose(Samplefile);
  
 return ierr;
 
} /* end of function */
